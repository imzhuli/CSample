#pragma once
#include <xel/X_Base.h>

#if defined(X_SYSTEM_WINDOWS)
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #include <windef.h>
    #include <ws2def.h>
    #include <winsock2.h>
    #include <Ws2tcpip.h>
        typedef HANDLE XelEventPoller;
        #define XelInvalidEventPoller      (INVALID_HANDLE_VALUE)
        typedef SOCKET XelSocket;
        #define XelInvalidSocket           (INVALID_SOCKET)
        #define XelCloseSocket(sockfd)     closesocket((sockfd))
#elif defined(X_SYSTEM_LINUX)
    #include <sys/epoll.h>
        typedef int XelEventPoller; // epoll
        #define XelInvalidEventPoller      ((XelEventPoller)-1)
    #include <sys/socket.h>
    #include <sys/types.h>
    #include <unistd.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
        typedef int XelSocket;
        #define XelInvalidSocket           ((XelSocket)-1)
        #define XelCloseSocket(sockfd)     close((sockfd))
#elif defined(X_SYSTEM_MACOS) || defined(X_SYSTEM_IOS)
    #include <sys/event.h>
        typedef int XelEventPoller; // kqueue
        #define XelInvalidEventPoller      ((XelEventPoller)-1)
    #include <sys/socket.h>
    #include <sys/types.h>
    #include <unistd.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
        typedef int XelSocket;
        #define XelInvalidSocket           ((XelSocket)-1)
        #define XelCloseSocket(sockfd)     close((sockfd))
#else
    #error unsupported system type
#endif

X_CNAME_BEGIN

typedef struct XelIoContext XelIoContext;
struct XelIoContext {
    XelEventPoller    EventPoller;
    unsigned char     Reserved [16];
};

typedef struct XelIoEventBase XelIoEventBase;
struct XelIoEventBase {
    void *            UserContextPtr;
    unsigned char     Reserved [16];
};

X_API bool XIC_Init(XelIoContext * ContextPtr);
X_API void XIC_Clean(XelIoContext * ContextPtr);
X_API void XIC_LoopOnce(uint32_t TimeoutMS);

X_API void XS_SetNonBlocking(XelSocket Sock);

X_STATIC_INLINE bool XIEB_Init(XelIoEventBase * EventBasePtr)  { XelIoEventBase InitObject = { NULL };  *EventBasePtr = InitObject; return true; }
X_STATIC_INLINE void XIEB_Clean(XelIoEventBase * EventBasePtr) {}

X_CNAME_END
