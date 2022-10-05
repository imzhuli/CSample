#include <xel_ext/net/X_Link.h>

bool XL_AppendData(XelLink * LinkPtr, const void * DataPtr, size_t DataSize)
{
    return false;
}

bool XL_FlushData(XelLink * LinkPtr)
{
    assert(LinkPtr->Status == XLS_Connected);
    XelWriteBufferChain * ChainPtr = &LinkPtr->BufferChain;
    while(true) {
        XelWriteBuffer * BufferPtr = XWBC_Peek(ChainPtr);
        if (!BufferPtr) {
            return true;
        }
        ssize_t WB = send(LinkPtr->SocketFd, BufferPtr->Buffer, (send_len_t)BufferPtr->BufferDataSize, XelNoWriteSignal);
        if (WB == BufferPtr->BufferDataSize) {
            XWBC_FreeFront(ChainPtr);
            continue;
        }
        if (WB < 0) {
            if (errno != EAGAIN) {
                XL_SetError(LinkPtr);
                return false;
            }
            break;
        }
        if ((size_t)WB < BufferPtr->BufferDataSize) {
            BufferPtr->BufferDataSize -= WB;
            memmove(BufferPtr->Buffer, BufferPtr->Buffer + WB, BufferPtr->BufferDataSize);
            break;
        }
    }
    return true;
}

/* Link */
 bool XL_Connect(XelLink * LinkPtr, uint32_t SAddr, uint16_t Port)
 {
    assert(LinkPtr->Status == XLS_Idle);
    assert(LinkPtr->SocketFd == XelInvalidSocket);

    LinkPtr->SocketFd = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if (LinkPtr->SocketFd == XelInvalidSocket) {
        return false;
    }
    struct sockaddr_in TargetAddr;
    memset(&TargetAddr, 0, sizeof(TargetAddr));
    TargetAddr.sin_family = AF_INET;
    TargetAddr.sin_addr.s_addr = SAddr;
    TargetAddr.sin_port = htons(Port);
    if (0 == connect(LinkPtr->SocketFd, (struct sockaddr*)&TargetAddr, sizeof(TargetAddr))) {
        LinkPtr->Status = XLS_Connected;
        return true;
    }
    if (errno != EAGAIN) {
        XelCloseSocket(LinkPtr->SocketFd);
        LinkPtr->SocketFd = XelInvalidSocket;
        return false;
    }
    LinkPtr->Status = XLS_Connecting;
    return true;
 }

bool XL_ReadRawData(XelLink * LinkPtr, void * DestBufferPtr, size_t * DestBufferSize)
{
    assert(XL_IsWorking(LinkPtr));
    assert(DestBufferPtr && DestBufferSize && *DestBufferSize);

    ssize_t Rb = recv(LinkPtr->SocketFd, DestBufferPtr, (recv_len_t)*DestBufferSize, 0);
    if (Rb == 0) {
        XEL_LINK_CALLBACK(LinkPtr, OnSetClose);
        return false;
    }
    if (Rb < 0) {
        if (errno == EAGAIN) {
            *DestBufferSize = 0;
            return true;
        }
        XL_SetError(LinkPtr);
        return false;
    }
    *DestBufferSize = 0;
    return true;
}

bool XL_ReadPacketLoop(XelLink * LinkPtr, XelPacketCallback * CallbackPtr, void * CallbackCtxPtr)
{
    assert(XL_IsWorking(LinkPtr));
    assert(CallbackPtr);

    ssize_t Rb = recv(LinkPtr->SocketFd, LinkPtr->ReadBuffer + LinkPtr->ReadBufferDataSize, (recv_len_t)(XelPacketMaxSize - LinkPtr->ReadBufferDataSize), 0);
    if (Rb == 0) { XEL_LINK_CALLBACK(LinkPtr, OnSetClose); return false; }
    if (Rb < 0) { return errno == EAGAIN; }
    LinkPtr->ReadBufferDataSize += Rb;

    XelUByte * StartPtr = LinkPtr->ReadBuffer;
    size_t RemainSize   = LinkPtr->ReadBufferDataSize;
    while(true) {
        if (RemainSize < XelPacketHeaderSize) {
            break;
        }
        XelPacketHeader Header;
        memset(&Header, 0, sizeof(Header));
        if (!XPH_Read(&Header, StartPtr)) {
            XL_SetError(LinkPtr);
            return false;
        }
        if (RemainSize < Header.PacketLength) {
            break;
        }
        if (!(*CallbackPtr)(CallbackCtxPtr, &Header, StartPtr + XelPacketHeaderSize, Header.PacketLength - XelPacketHeaderSize)) {
            XL_SetError(LinkPtr);
            return false;
        }
        StartPtr    += Header.PacketLength;
        RemainSize  -= Header.PacketLength;
    }
    if (RemainSize && StartPtr != LinkPtr->ReadBuffer) {
        memmove(LinkPtr->ReadBuffer, StartPtr, RemainSize);
        LinkPtr->ReadBufferDataSize = RemainSize;
    }
    return true;
}

bool XL_WriteRawData(XelLink * LinkPtr, const void * _DataPtr, size_t Length)
{
    // XS_PrintHexShow(stdout, _DataPtr, Length, true);
    if (!XL_IsWorking(LinkPtr)) {
        return false;
    }

    const XelUByte * DataPtr = _DataPtr;
    XelWriteBufferChain * ChainPtr = &LinkPtr->BufferChain;
    if (LinkPtr->Status == XLS_Connecting || XWBC_Peek(ChainPtr)) {
        return XL_AppendData(LinkPtr, DataPtr, Length);
    }
    ssize_t WB = send(LinkPtr->SocketFd, DataPtr, (send_len_t)Length, XelNoWriteSignal);
    if (WB < 0) {
        if (errno != EAGAIN) {
            XL_SetError(LinkPtr);
            return false;
        }
        WB = 0;
    }
    DataPtr += WB;
    Length -= WB;
    return XL_AppendData(LinkPtr, DataPtr, Length);;
}

bool XL_ReadEventCallback(void * CtxPtr, XelLink* LinkPtr)
{
    // Do Nothing!
    return false;
}

void XL_ErrorEventCallback(void * CtxPtr, XelLink* LinkPtr)
{
    XL_SetError(LinkPtr);
}

bool XL_WriteEventCallback(void * CtxPtr, XelLink* LinkPtr)
{
    if (LinkPtr->Status == XLS_Connecting) {
        LinkPtr->Status = XLS_Connected;
    }
    return XL_FlushData(LinkPtr);
}

void XL_OnSetClose(void * CtxPtr, XelLink* LinkPtr)
{
    LinkPtr->Status = XEL_Closed;
}

void XL_SetError(XelLink* LinkPtr)
{
    LinkPtr->Flags |= XLF_ERROR;
    XEL_LINK_CALLBACK(LinkPtr, OnSetClose);
}
