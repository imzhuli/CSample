#pragma once
#include <xel/X_Base.h>
#include <xel/X_Byte.h>
#include <stdbool.h>
#include <stdio.h>
#include <errno.h>
#include "./X_IO.h"
#include "./X_Packet.h"
#include "./X_PacketBuffer.h"

X_CNAME_BEGIN

/* link */
typedef enum {
    XLS_Idle = 0,
    XLS_Connecting,
    XLS_Connected,
    XEL_Closed,
} XelLinkStatus;

struct XelLinkCallbacks;

#define XLF_NONE      ((uint32_t)0x00000000)
#define XLF_ERROR     ((uint32_t)0x00000001)

typedef struct {
    XelLinkStatus               Status;
    uint32_t                    Flags;
    XelSocket                   SocketFd;
    XelUByte                    ReadBuffer[XelPacketMaxSize];
    size_t                      ReadBufferDataSize;
    XelWriteBufferChain         BufferChain;
    struct XelLinkCallbacks *   CallbacksPtr;
} XelLink;

static inline bool XL_IsWorking(const XelLink* LinkPtr) {
    return LinkPtr->Status == XLS_Connecting || LinkPtr->Status == XLS_Connected;
}

typedef struct XelLinkCallbacks {
    // system event callbacks
    bool (*OnReadEvent)(void * CtxPtr, XelLink* LinkPtr);
    bool (*OnWriteEvent)(void * CtxPtr, XelLink* LinkPtr);
    void (*OnErrorEvent)(void * CtxPtr, XelLink* LinkPtr);
    // callbacks during event procedures:
    void (*OnSetClose)(void * CtxPtr, XelLink* LinkPtr);
    void * CtxPtr;
} XelLinkCallbacks;
#define XEL_LINK_CALLBACK(LinkPtr, CallbackName) (*((LinkPtr)->CallbacksPtr->CallbackName))((LinkPtr)->CallbacksPtr->CtxPtr, (LinkPtr))

typedef bool XelPacketCallback(void * CtxPtr, const XelPacketHeader * HeaderPtr, const void * PayloadPtr, size_t PayloadSize);
X_PRIVATE bool XL_Connect(XelLink * LinkPtr, uint32_t SAddr, uint16_t Port);
X_PRIVATE bool XL_ReadRawData(XelLink * LinkPtr, void * DestBufferPtr, size_t * DestBufferSize);
X_PRIVATE bool XL_ReadPacketLoop(XelLink * LinkPtr, XelPacketCallback * CallbackPtr, void * CallbackCtxPtr);
X_PRIVATE bool XL_WriteRawData(XelLink * LinkPtr, const void * DataPtr, size_t Length);
X_PRIVATE void XL_SetError(XelLink* LinkPtr);
X_PRIVATE bool XL_IsError(XelLink* LinkPtr) { return LinkPtr->Flags & XLF_ERROR; }

X_PRIVATE bool XL_ReadEventCallback(void * CtxPtr, XelLink* LinkPtr);
X_PRIVATE bool XL_WriteEventCallback(void * CtxPtr, XelLink* LinkPtr);
X_PRIVATE void XL_ErrorEventCallback(void * CtxPtr, XelLink* LinkPtr);
X_PRIVATE void XL_OnSetClose(void * CtxPtr, XelLink* LinkPtr);


X_API bool XL_Init(XelLink * LinkPtr);
X_API void XL_Clean(XelLink * LinkPtr);
X_API bool XL_AppendData(XelLink * LinkPtr, const void * DataPtr, size_t DataSize);
X_API bool XL_FlushData(XelLink * LinkPtr);

X_CNAME_END
