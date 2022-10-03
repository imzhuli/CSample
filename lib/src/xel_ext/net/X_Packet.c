#include <xel_ext/net/X_Packet.h>
#include <xel/X_Byte.h>

X_STATIC_INLINE uint32_t MakeHeaderLength(uint32_t PacketLength) {
    assert(PacketLength <= XelPacketMaxSize);
    return PacketLength | XelPacketMagicValue;
}

X_STATIC_INLINE bool CheckPackageLength(uint32_t PacketLength) {
    return (PacketLength & XelPacketMagicMask) == XelPacketMagicValue
        && (PacketLength & XelPacketLengthMask) <= XelPacketMaxSize;
}

size_t XPH_Read(XelPacketHeader * HeaderPtr, const void * SourcePtr)
{
    XelStreamReader Ctx = XSR(SourcePtr);
    HeaderPtr->PacketLength = XSR_4L(&Ctx);
    if (!CheckPackageLength(HeaderPtr->PacketLength)) {
        return 0;
    }
    HeaderPtr->PacketLength &= XelPacketLengthMask;
    HeaderPtr->PackageSequenceId        = XSR_1L(&Ctx);
    HeaderPtr->PackageSequenceTotalMax  = XSR_1L(&Ctx);
    HeaderPtr->CommandId                = XSR_2L(&Ctx);
    HeaderPtr->RequestId                = XSR_8L(&Ctx);
    XSR_Raw(&Ctx, HeaderPtr->TraceId, 16);
    return HeaderPtr->PacketLength;
}

void XLH_Write(void * DestPtr, const XelPacketHeader * HeaderPtr)
{
    XelStreamWriter Ctx = XSW(DestPtr);
    XSW_4L(&Ctx, MakeHeaderLength(HeaderPtr->PacketLength));
    XSW_1L(&Ctx, HeaderPtr->PackageSequenceId);
    XSW_1L(&Ctx, HeaderPtr->PackageSequenceTotalMax);
    XSW_2L(&Ctx, HeaderPtr->CommandId);
    XSW_8L(&Ctx, HeaderPtr->RequestId);
    XSW_Raw(&Ctx, HeaderPtr->TraceId, 16);
}
