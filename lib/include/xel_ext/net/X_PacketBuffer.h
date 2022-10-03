#pragma once
#include "./X_Packet.h"

X_CNAME_BEGIN

/* WriteBuffer(Chain) */
typedef struct XelWriteBuffer {
    XelUByte                   Buffer[XelPacketMaxSize];
    size_t                     BufferDataSize;
    struct XelWriteBuffer *    NextPtr;
} XelWriteBuffer;

static inline void XWB_Init(XelWriteBuffer * BufferPtr) {
    BufferPtr->BufferDataSize = 0;
    BufferPtr->NextPtr = 0;
}

static inline void XWB_Clean(XelWriteBuffer * BufferPtr) {
    assert(!BufferPtr->NextPtr);
    BufferPtr->BufferDataSize = 0;
}

typedef struct {
    XelWriteBuffer * (*Alloc)(void *);
    void (*Free)(void *, XelWriteBuffer *);
    void * CtxPtr;
} XelWriteBuffer_Allocator;
X_PRIVATE XelWriteBuffer_Allocator * const XWB_DefaultAllocatorPtr;

typedef struct XelWriteBufferChain {
    XelWriteBuffer * FirstPtr;
    XelWriteBuffer * LastPtr;
    XelWriteBuffer_Allocator *  AllocatorPtr;
} XelWriteBufferChain;


X_STATIC_INLINE XelWriteBuffer * XWBC_Alloc(XelWriteBufferChain * ChainPtr)
{
    return ChainPtr->AllocatorPtr->Alloc(ChainPtr->AllocatorPtr->CtxPtr);
}

X_STATIC_INLINE void XWBC_Free(XelWriteBufferChain * ChainPtr, XelWriteBuffer * BufferPtr)
{
    ChainPtr->AllocatorPtr->Free(ChainPtr->AllocatorPtr->CtxPtr, BufferPtr);
}

X_STATIC_INLINE XelWriteBuffer * XWBC_Peek(XelWriteBufferChain * ChainPtr)
{
    return ChainPtr->FirstPtr;
}

X_STATIC_INLINE void XWBC_FreeFront(XelWriteBufferChain * ChainPtr)
{
    XelWriteBuffer * BufferPtr = ChainPtr->FirstPtr;
    assert(BufferPtr);

    if (BufferPtr == ChainPtr->LastPtr) {
        ChainPtr->FirstPtr = ChainPtr->LastPtr = NULL;
    } else {
        ChainPtr->FirstPtr = BufferPtr->NextPtr;
    }
    BufferPtr->NextPtr = NULL;
    XWBC_Free(ChainPtr, BufferPtr);
}

X_STATIC_INLINE XelWriteBufferChain XWBC_Init(XelWriteBuffer_Allocator * AllocatorPtr)
{
    if (!AllocatorPtr) {
        AllocatorPtr = XWB_DefaultAllocatorPtr;
    }
    XelWriteBufferChain Ret = {
        NULL, NULL, AllocatorPtr
    };
    return Ret;
}

X_STATIC_INLINE void XWBC_Clean(XelWriteBufferChain * ChainPtr)
{
    while(XWBC_Peek(ChainPtr)) {
        XWBC_FreeFront(ChainPtr);
    }
}

X_STATIC_INLINE void XWBC_Append(XelWriteBufferChain * ChainPtr, XelWriteBuffer * BufferPtr)
{
    assert(BufferPtr);
    assert(!BufferPtr->NextPtr);
    if (!ChainPtr->FirstPtr) {
        ChainPtr->LastPtr = ChainPtr->FirstPtr = BufferPtr;
    } else {
        ChainPtr->LastPtr = BufferPtr;
    }
}

X_CNAME_END
