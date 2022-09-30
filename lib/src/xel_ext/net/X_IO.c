#include <xel_ext/net/X_IO.h>

bool XIC_Init(XelIoContext * ContextPtr)
{
    XelIoContext InitValue = { .EventPoller = XelInvalidEventPoller,  .Reserved = {0} };
    #ifdef X_SYSTEM_LINUX
        XelEventPoller Epoll = epoll_create1(EPOLL_CLOEXEC);
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
    #ifdef X_SYSTEM_LINUX
        close(ContextPtr->EventPoller);
    #endif
    
    ContextPtr->EventPoller = XelInvalidEventPoller;
}

