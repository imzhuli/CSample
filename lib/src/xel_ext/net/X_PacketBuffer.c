#include <xel_ext/net/X_PacketBuffer.h>

/* Allocator */
static XelWriteBuffer * XWB_DefaultAlloc(void * CtxPtr)
{
    return (XelWriteBuffer*)malloc(sizeof(XelWriteBuffer));
}

static void XWB_DefaultFree(void * CtxPtr, XelWriteBuffer * BufferPtr)
{
    free(BufferPtr);
}

static XelWriteBuffer_Allocator XWB_DefaultAllocator = {
    &XWB_DefaultAlloc,
    &XWB_DefaultFree,
    NULL
};

XelWriteBuffer_Allocator * const XWB_DefaultAllocatorPtr = &XWB_DefaultAllocator;
