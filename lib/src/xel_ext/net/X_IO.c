#include <xel_ext/net/X_IO.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#if defined(X_SYSTEM_LINUX) ||defined(X_SYSTEM_MACOS) || defined(X_SYSTEM_IOS)
#include <fcntl.h>
#endif

#define LOOP_ONCE_MAX_EVENT_NUMBER  256

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
    return true;
}

void XIC_Clean(XelIoContext * ContextPtr)
{
    if (ContextPtr->EventPoller == XelInvalidEventPoller) {
        return;
    }
    #if defined(X_SYSTEM_LINUX)
        close(ContextPtr->EventPoller);
    #elif defined(X_SYSTEM_MACOS) || defined(X_SYSTEM_IOS)
        close(ContextPtr->EventPoller);
    #endif

    ContextPtr->EventPoller = XelInvalidEventPoller;
}

void XIC_LoopOnce(XelIoContext * ContextPtr, int TimeoutMS)
{
#if defined(X_SYSTEM_LINUX)
    XelEventPoller EventPoller = ContextPtr->EventPoller;
    struct epoll_event Events[LOOP_ONCE_MAX_EVENT_NUMBER];
    int Total = epoll_wait(ContextPtr->EventPoller, Events, LOOP_ONCE_MAX_EVENT_NUMBER, TimeoutMS);
    for(int i = 0 ; i < Total; ++i) 
    {
        struct epoll_event * EventPtr = &Events[i];
        XelIoEventBase * EventBasePtr = (XelIoEventBase *)EventPtr->data.ptr;
        if (EventPtr->events & (EPOLLERR | EPOLLHUP)) {
            XelIoEventCallback Callback = EventBasePtr->_ErrorEventCallback;
            XIEB_Unbind(EventBasePtr);
            if (Callback) {
                Callback(EventBasePtr, XIET_Err);
            }
            continue;
        }
        if (EventPtr->events & EPOLLIN) {
            XelIoEventCallback Callback = EventBasePtr->_InEventCallback;
            assert(Callback);            
            Callback(EventBasePtr, XIET_In);            
            // unbind called during procedure:
            if (!EventBasePtr->_IoContextPtr) {
                continue;
            }
        }
        if (EventPtr->events & EPOLLOUT) {
            XelIoEventCallback Callback = EventBasePtr->_OutEventCallback;
            assert(Callback);            
            Callback(EventBasePtr, XIET_Out);
            // unbind called during procedure:
            if (!EventBasePtr->_IoContextPtr) {
                continue;
            }
        }
        // update event flags:
        struct epoll_event Events = { .events = EPOLLET };
        if (X_LIKELY(EventBasePtr->_InEventCallback)) {
            Events.events |= EPOLLIN;
        }
        if (X_UNLIKELY(EventBasePtr->_OutEventCallback)) {
            Events.events |= EPOLLOUT;
        }
        if (X_UNLIKELY(EventBasePtr->_NativeRequiredEvents != Events.events)) {
            EventBasePtr->_NativeRequiredEvents = Events.events;
            Events.data.ptr = EventBasePtr;
            epoll_ctl(EventPoller, EPOLL_CTL_MOD, EventBasePtr->_IoHandle.FileDescriptor, &Events);
        }
    }
#else
    X_FatalAbort("Not implemented");
#endif
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
    if (EventBasePtr->_IoContextPtr) {
        XIEB_Unbind(EventBasePtr);
    }
    XelIoEventBase CleanObject = { NULL };  
    *EventBasePtr = CleanObject; 
}

void XIEB_Bind(XelIoContext * IoContextPtr, XelIoEventBase * EventBasePtr)
{
    assert(!EventBasePtr->_IoContextPtr);
#if defined(X_SYSTEM_LINUX)
    assert(!EventBasePtr->_NativeRequiredEvents);
    struct epoll_event Events = { .events = EPOLLET };
    if (EventBasePtr->_InEventCallback) {
        Events.events |= EPOLLIN;
    }
    if (EventBasePtr->_OutEventCallback) {
        Events.events |= EPOLLOUT;
    }
    EventBasePtr->_NativeRequiredEvents = Events.events;
    Events.data.ptr = EventBasePtr;
    if (-1 == epoll_ctl(IoContextPtr->EventPoller, EPOLL_CTL_ADD, EventBasePtr->_IoHandle.FileDescriptor, &Events)) {
        X_FatalAbort(strerror(errno));
    }
#endif
    EventBasePtr->_IoContextPtr = IoContextPtr;
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
    EventBasePtr->_InEventCallback = NULL;
    EventBasePtr->_OutEventCallback = NULL;
    EventBasePtr->_ErrorEventCallback = NULL;
}

void XIEB_ResumeReading(XelIoEventBase * EventBasePtr, XelIoEventCallback Callback)
{
    assert(Callback);
    EventBasePtr->_InEventCallback = Callback;
}

X_API void XIEB_SuspendReading(XelIoEventBase * EventBasePtr)
{
    EventBasePtr->_InEventCallback = NULL;
}

X_API void XIEB_MarkWriting(XelIoEventBase * EventBasePtr, XelIoEventCallback Callback)
{
    assert(Callback);
    EventBasePtr->_OutEventCallback = Callback;
}

void XIEB_SetErrorCallback(XelIoEventBase * EventBasePtr, XelIoEventCallback Callback)
{
    EventBasePtr->_ErrorEventCallback = Callback;
}
