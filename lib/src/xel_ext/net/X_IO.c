#include <xel_ext/net/X_IO.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#if defined(X_SYSTEM_LINUX) ||defined(X_SYSTEM_MACOS) || defined(X_SYSTEM_IOS)
#include <fcntl.h>
#define LOOP_ONCE_MAX_EVENT_NUMBER  256
#endif

bool XIC_Init(XelIoContext * ContextPtr)
{
    XelIoContext InitValue = { .EventPoller = XelInvalidEventPoller,  .Reserved = {0} };
    #if defined(X_SYSTEM_LINUX)
        XelEventPoller Epoll = epoll_create1(EPOLL_CLOEXEC);
        if (Epoll == -1) {
            *ContextPtr = InitValue;
            return false;
        }
        InitValue.EventPoller = Epoll;
    #elif defined(X_SYSTEM_MACOS) || defined(X_SYSTEM_IOS)
        XelEventPoller Epoll = kqueue();
        if (Epoll == -1) {
            *ContextPtr = InitValue;
            return false;
        }
        InitValue.EventPoller = Epoll;
    #endif
    *ContextPtr = InitValue;
    XL_Init(&ContextPtr->UserEventList);
    return true;
}

void XIC_Clean(XelIoContext * ContextPtr)
{
    if (ContextPtr->EventPoller == XelInvalidEventPoller) {
        return;
    }

    XL_Clean(&ContextPtr->UserEventList);
    #if defined(X_SYSTEM_LINUX)
        close(ContextPtr->EventPoller);
    #elif defined(X_SYSTEM_MACOS) || defined(X_SYSTEM_IOS)
        close(ContextPtr->EventPoller);
    #endif
    ContextPtr->EventPoller = XelInvalidEventPoller;
}

static void XIC_UpdateEvents(XelIoContext * ContextPtr, XelIoEventBase * EventBasePtr)
{
#if defined(X_SYSTEM_LINUX)
    uint32_t InterestedEvents = EPOLLET;
    if (EventBasePtr->_EnableReadingEvent) {
        InterestedEvents |= EPOLLIN;
    }
    if (EventBasePtr->_EnableWritingEvent) {
        InterestedEvents |= EPOLLOUT;
    }
    if (X_UNLIKELY(EventBasePtr->_NativeRequiredEvents != InterestedEvents)) {
        struct epoll_event Events = { .events = InterestedEvents, .data = { .ptr = EventBasePtr } };
        if (-1 == epoll_ctl(ContextPtr->EventPoller, EPOLL_CTL_MOD, EventBasePtr->_IoHandle.FileDescriptor, &Events)) {
            X_DbgError("Failed to change event : errno=%i, %s", errno, strerror(errno));
        }
        EventBasePtr->_NativeRequiredEvents = InterestedEvents;
    }
#endif
}

void XIC_LoopOnce(XelIoContext * ContextPtr, int TimeoutMS)
{
    assert(!ContextPtr->ProcessingTargerPtr);
    XelListForwardIterator Iter = XL_Begin(&ContextPtr->UserEventList);
    XelListForwardIterator IterEnd = XL_End(&ContextPtr->UserEventList);
    while(!XLFI_IsEqual(Iter, IterEnd)) {
        XelIoUserEvent * EventNodePtr = X_Entry(XLFI_Get(Iter), XelIoUserEvent, UserEventNode);
        XIUE_Detach(EventNodePtr);
        EventNodePtr->UserEventProc(EventNodePtr);
        Iter = XLFI_Next(Iter);
    }

#if defined(X_SYSTEM_LINUX)
    struct epoll_event Events[LOOP_ONCE_MAX_EVENT_NUMBER];
    int Total = epoll_wait(ContextPtr->EventPoller, Events, LOOP_ONCE_MAX_EVENT_NUMBER, TimeoutMS);
    for(int i = 0 ; i < Total; ++i)
    {
        struct epoll_event * EventPtr = &Events[i];
        XelIoEventBase * EventBasePtr = (XelIoEventBase *)EventPtr->data.ptr;
        XelIoEventCallback Callback = EventBasePtr->_EventCallback;
        ContextPtr->ProcessingTargerPtr = EventBasePtr;
        if (EventPtr->events & (EPOLLERR | EPOLLHUP)) {
            XIEB_Unbind(EventBasePtr);
            if (Callback) {
                Callback(EventBasePtr, XIET_Err);
            }
            continue;
        }
        if (EventPtr->events & EPOLLIN) {
            Callback(EventBasePtr, XIET_In);
            // unbind called during procedure:
            if (!EventBasePtr->_IoContextPtr) {
                continue;
            }
        }
        if (EventPtr->events & EPOLLOUT) {
            EventBasePtr->_EnableWritingEvent = false;
            Callback(EventBasePtr, XIET_Out);
            // unbind called during procedure:
            if (!EventBasePtr->_IoContextPtr) {
                continue;
            }
        }
        // update event flags:
        XIC_UpdateEvents(ContextPtr, EventBasePtr);
    }
#else
    X_FatalAbort("Not implemented");
#endif

    ContextPtr->ProcessingTargerPtr = NULL;
}

// utils

void XS_SetNonBlocking(XelSocket Sock)
{
    bool blocking = false;
#if defined(X_SYSTEM_WINDOWS)
    unsigned long mode = blocking ? 0 : 1;
    ioctlsocket(Sock, FIONBIO, &mode);
#else
    int flags = fcntl(Sock, F_GETFL, 0);
    flags = blocking ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
    fcntl(Sock, F_SETFL, flags);
#endif
}

// Event base support

bool XIEB_Init(XelIoEventBase * EventBasePtr)
{
    XelIoEventBase InitObject = { NULL };
    *EventBasePtr = InitObject;
    return true;
}

void XIEB_Clean(XelIoEventBase * EventBasePtr)
{
    assert(!EventBasePtr->_IoContextPtr);
    XelIoEventBase CleanObject = { NULL };
    *EventBasePtr = CleanObject;
}

bool XIEB_Bind(XelIoContext * IoContextPtr, XelIoEventBase * EventBasePtr, XelIoHandle IoHandle, XelIoEventCallback Callback)
{
    assert(!EventBasePtr->_IoContextPtr);
    assert(EventBasePtr->_IoHandle.IoType == XIT_Unknown);
    assert(Callback);
#if defined(X_SYSTEM_LINUX)
    assert(!EventBasePtr->_NativeRequiredEvents);
    uint32_t InterestedEvents = EPOLLET;
    if (EventBasePtr->_EnableReadingEvent) {
        InterestedEvents |= EPOLLIN;
    }
    if (EventBasePtr->_EnableWritingEvent) {
        InterestedEvents |= EPOLLOUT;
    }
    struct epoll_event Events = { .events = InterestedEvents, .data = { .ptr = EventBasePtr } };
    if (-1 == epoll_ctl(IoContextPtr->EventPoller, EPOLL_CTL_ADD, IoHandle.FileDescriptor, &Events)) {
        return false;
    }
    EventBasePtr->_IoContextPtr = IoContextPtr;
    EventBasePtr->_EventCallback = Callback;
    EventBasePtr->_IoHandle = IoHandle;
    EventBasePtr->_NativeRequiredEvents = InterestedEvents;
#else
    X_FatalAbort("not implemented");
#endif
    return true;
}

void XIEB_Unbind(XelIoEventBase * EventBasePtr)
{
    XelIoContext * IoContextPtr = EventBasePtr->_IoContextPtr;
    if (!IoContextPtr) { // duplicate unbind is allowed
        return;
    }
#if defined(X_SYSTEM_LINUX)
    epoll_ctl(IoContextPtr->EventPoller, EPOLL_CTL_DEL, EventBasePtr->_IoHandle.FileDescriptor, NULL);
    EventBasePtr->_NativeRequiredEvents = 0;
#else
    X_FatalAbort("Not implemented");
#endif
    EventBasePtr->_IoContextPtr = NULL;
    EventBasePtr->_IoHandle = XIH_None();
    EventBasePtr->_EventCallback = NULL;
}

void XIEB_ResumeReading(XelIoEventBase * EventBasePtr)
{
    EventBasePtr->_EnableReadingEvent = true;
    if (!EventBasePtr->_IoContextPtr || XIC_IsProcessing(EventBasePtr->_IoContextPtr, EventBasePtr)) {
        return;
    }
    XIC_UpdateEvents(EventBasePtr->_IoContextPtr, EventBasePtr);
}

X_API void XIEB_SuspendReading(XelIoEventBase * EventBasePtr)
{
    EventBasePtr->_EnableReadingEvent = false;
    if (!EventBasePtr->_IoContextPtr || XIC_IsProcessing(EventBasePtr->_IoContextPtr, EventBasePtr)) {
        return;
    }
    XIC_UpdateEvents(EventBasePtr->_IoContextPtr, EventBasePtr);
}

X_API void XIEB_MarkWriting(XelIoEventBase * EventBasePtr)
{
    EventBasePtr->_EnableWritingEvent = true;
    if (!EventBasePtr->_IoContextPtr || XIC_IsProcessing(EventBasePtr->_IoContextPtr, EventBasePtr)) {
        return;
    }
    XIC_UpdateEvents(EventBasePtr->_IoContextPtr, EventBasePtr);
}
