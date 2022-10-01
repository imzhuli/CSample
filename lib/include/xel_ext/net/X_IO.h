#pragma once
#include <xel/X_Common.h>

#if defined(X_SYSTEM_WINDOWS)
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #include <windef.h>
    #include <ws2def.h>
    #include <winsock2.h>
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

extern bool XIC_Init(XelIoContext * ContextPtr);
extern void XIC_Clean(XelIoContext * ContextPtr);

X_CNAME_END
