#pragma once
#include <xel/X_Common.h>

#ifdef X_SYSTEM_WINDOWS
#define WIN32_LEAN_AND_MEAN 
#include <windows.h>
#include <windef.h>
#include <ws2def.h>
#include <winsock2.h>
    typedef SOCKET XelSocket;
    #define XelInvalidSocket           (INVALID_SOCKET)
    #define XelCloseSocket(sockfd)     closesocket((sockfd))
#else
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
    typedef int XelSocket;
    #define XelInvalidSocket           ((XelSocket) -1)
    #define XelCloseSocket(sockfd)     close((sockfd))
#endif

X_CNAME_BEGIN

typedef struct XelIoContext XelIoContext;
struct XelIoContext {
    unsigned char Reserved [16];
};

static inline void XIC_Init(XelIoContext * ContextPtr)
{
    XelIoContext InitValue = { .Reserved = {0}};
    *ContextPtr = InitValue;
}


X_CNAME_END
