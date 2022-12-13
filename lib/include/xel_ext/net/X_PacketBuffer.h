#pragma once
#include "./X_Packet.h"
#include <string.h>

X_CNAME_BEGIN

/* WriteBuffer(Chain) */
typedef struct XelWriteBuffer {
    XelUByte                   Buffer[XelPacketMaxSize];
    size_t                     BufferDataSize;
    struct XelWriteBuffer *    NextPtr;
} XelWriteBuffer;

X_STATIC_INLINE void XWB_Init(XelWriteBuffer * BufferPtr) {
    BufferPtr->BufferDataSize = 0;
    BufferPtr->NextPtr = 0;
}

X_STATIC_INLINE void XWB_Clean(XelWriteBuffer * BufferPtr) {
    assert(!BufferPtr->NextPtr);
    BufferPtr->BufferDataSize = 0;
}

X_STATIC_INLINE size_t XWB_PushBack(XelWriteBuffer * BufferPtr, const void * DataPtr, size_t Size) {
    size_t RemainSize = sizeof(BufferPtr->Buffer) - BufferPtr->BufferDataSize;
    size_t PushSize = RemainSize <= Size ? RemainSize : Size;
    memcpy(BufferPtr->Buffer + BufferPtr->BufferDataSize, DataPtr, PushSize);
    BufferPtr->BufferDataSize += PushSize;
    return PushSize;
}

typedef struct {
    XelWriteBuffer * (*Alloc)(void *);
    void (*Free)(void *, XelWriteBuffer *);
    void * CtxPtr;
} XelWriteBuffer_Allocator;

X_PRIVATE XelWriteBuffer_Allocator * const XWB_DefaultAllocatorPtr;

typedef struct XelWriteBufferChain {
    XelWriteBuffer *            FirstPtr;
    XelWriteBuffer *            LastPtr;
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

X_STATIC_INLINE bool XWBC_IsEmpty(XelWriteBufferChain * ChainPtr)
{
    return !ChainPtr->FirstPtr;
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

X_STATIC_INLINE bool XWBC_Init(XelWriteBufferChain * ChainPtr, XelWriteBuffer_Allocator * AllocatorPtr)
{
    if (!AllocatorPtr) {
        AllocatorPtr = XWB_DefaultAllocatorPtr;
    }
    XelWriteBufferChain InitObject = {
        NULL, NULL, AllocatorPtr
    };
    *ChainPtr = InitObject;
    return true;
}

X_STATIC_INLINE void XWBC_Clean(XelWriteBufferChain * ChainPtr)
{
    while(XWBC_Peek(ChainPtr)) {
        XWBC_FreeFront(ChainPtr);
    }
}

X_STATIC_INLINE void XWBC_AppendBuffer(XelWriteBufferChain * ChainPtr, XelWriteBuffer * BufferPtr)
{
    assert(BufferPtr);
    assert(!BufferPtr->NextPtr);
    if (!ChainPtr->FirstPtr) {
        ChainPtr->LastPtr = ChainPtr->FirstPtr = BufferPtr;
    } else {
        ChainPtr->LastPtr->NextPtr = BufferPtr;
        ChainPtr->LastPtr = BufferPtr;
    }
}

X_STATIC_INLINE size_t XWBC_PushBack(XelWriteBufferChain * ChainPtr, const void * DataPtr_, size_t Size)
{
    size_t PushTotal = 0;
    XelUByte * DataPtr = (XelUByte*)DataPtr_;
    if (ChainPtr->LastPtr) {
        size_t PushOnceSize = XWB_PushBack(ChainPtr->LastPtr, DataPtr, Size);
        DataPtr += PushOnceSize;
        Size -= PushOnceSize;
        PushTotal += PushOnceSize;
    }
    while(Size) {
        XelWriteBuffer * NewWriteBuffer = XWBC_Alloc(ChainPtr);
        if (!NewWriteBuffer) {
            X_DbgError("Failed to alloc new wirte buffer");
            return PushTotal;
        }
        XWB_Init(NewWriteBuffer);
        size_t PushOnceSize = XWB_PushBack(NewWriteBuffer, DataPtr, Size);
        DataPtr += PushOnceSize;
        Size -= PushOnceSize;
        PushTotal += PushOnceSize;
        XWBC_AppendBuffer(ChainPtr, NewWriteBuffer);
    }
    return PushTotal;
}

X_CNAME_END
