#pragma once
#include <xel/X_Base.h>
#include <xel/X_List.h>

#if defined(X_SYSTEM_WINDOWS)
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #include <windef.h>
    #include <ws2def.h>
    #include <winsock2.h>
    #include <Ws2tcpip.h>

    typedef SSIZE_T  ssize_t;
    typedef int      send_len_t;
    typedef int      recv_len_t;
    typedef HANDLE XelEventPoller;
    #define XelInvalidEventPoller      (INVALID_HANDLE_VALUE)
    typedef XelVariable                XelNativeEventType
    typedef SOCKET XelSocket;
    #define XelInvalidSocket           (INVALID_SOCKET)
    #define XelCloseSocket(sockfd)     closesocket((sockfd))

#elif defined(X_SYSTEM_LINUX)
    #include <sys/epoll.h>
    #include <sys/socket.h>
    #include <sys/types.h>
    #include <unistd.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>

    typedef size_t                     send_len_t;
    typedef size_t                     recv_len_t;
    typedef int                        XelEventPoller;        // epoll
    #define XelInvalidEventPoller      ((XelEventPoller)-1)
    typedef enum EPOLL_EVENTS          XelNativeEventType;    // EPOLLIN EPOLLOUT EPOLLERR ...
    typedef int XelSocket;
    #define XelInvalidSocket           ((XelSocket)-1)
    #define XelCloseSocket(sockfd)     close((sockfd))

#elif defined(X_SYSTEM_MACOS) || defined(X_SYSTEM_IOS)
    #include <sys/event.h>
    #include <sys/socket.h>
    #include <sys/types.h>
    #include <unistd.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>

    typedef size_t                     send_len_t;
    typedef size_t                     recv_len_t;
    typedef int                        XelEventPoller;        // kqueue
    #define XelInvalidEventPoller      ((XelEventPoller)-1)
    typedef int                        XelSocket;
    #define XelInvalidSocket           ((XelSocket)-1)
    #define XelCloseSocket(sockfd)     close((sockfd))

#else
    #error unsupported system type
#endif

#ifdef MSG_NOSIGNAL
    #define XelNoWriteSignal       MSG_NOSIGNAL
#else
    #define XelNoWriteSignal       0
#endif
#ifndef SOCK_CLOEXEC
    #define SOCK_CLOEXEC           0
#endif

X_CNAME_BEGIN

enum XelIoType {
    XIT_Unknown,
    XIT_File,
    XIT_Socket,
};
enum XelIoEventType {
    XIET_Err,
    XIET_In,
    XIET_Out,
};
typedef enum     XelIoType        XelIoType;
typedef enum     XelIoEventType   XelIoEventType;
typedef struct   XelIoHandle      XelIoHandle;
typedef struct   XelIoContext     XelIoContext;
typedef struct   XelIoEventBase   XelIoEventBase;

typedef void (*XelIoEventCallback)(XelIoEventBase * IoEventBasePtr, XelIoEventType IoEventType);

struct XelIoHandle
{
    XelIoType         IoType;
    union {
        XelVariable   _;
        int           FileDescriptor;
    };
};

struct XelIoContext {
    XelEventPoller    EventPoller;
    unsigned char     Reserved [16];
};

struct XelIoEventBase {
    // managed by the following functions
    XelIoContext *        _IoContextPtr;
    XelIoHandle           _IoHandle;
    XelNativeEventType    _NativeRequiredEvents;
    XelIoEventCallback    _InEventCallback;
    XelIoEventCallback    _OutEventCallback;
    XelIoEventCallback    _ErrorEventCallback;
    unsigned char         _Reserved [16];
};

X_API bool XIC_Init(XelIoContext * ContextPtr);
X_API void XIC_Clean(XelIoContext * ContextPtr);
X_API void XIC_LoopOnce(XelIoContext * ContextPtr, int TimeoutMS);

X_API void XS_SetNonBlocking(XelSocket Sock);

X_API bool XIEB_Init(XelIoEventBase * EventBasePtr);
X_API void XIEB_Clean(XelIoEventBase * EventBasePtr);
X_API void XIEB_Bind(XelIoContext * IoContextPtr, XelIoEventBase * EventBasePtr);
X_API void XIEB_Unbind(XelIoEventBase * EventBasePtr);
X_API void XIEB_ResumeReading(XelIoEventBase * EventBasePtr, XelIoEventCallback Callback);
X_API void XIEB_SuspendReading(XelIoEventBase * EventBasePtr);
X_API void XIEB_MarkWriting(XelIoEventBase * EventBasePtr, XelIoEventCallback Callback);
X_API void XIEB_SetErrorCallback(XelIoEventBase * EventBasePtr, XelIoEventCallback Callback);

X_CNAME_END
