#pragma once
#include <xel/X_Base.h>

#define XelPacketHeaderSize          ((size_t)(32))
#define XelPacketMagicMask           ((size_t)(0xFF000000))
#define XelPacketMagicValue          ((size_t)(0xCD000000))
#define XelPacketLengthMask          ((size_t)(0x00FFFFFF))
#define XelPacketMaxSize             ((size_t)(4096 & XelPacketLengthMask))
#define XelPacketMaxPayloadSize      ((size_t)(XelPacketMaxSize - XelPacketHeaderSize))

X_CNAME_BEGIN

typedef struct XelPacketHeader {
    uint32_t     PacketLength; // header size included, lower 24 bits as length, higher 8 bits as a magic check
    uint8_t      PackageSequenceId; // the index of the packet in a full package, (this is no typo)
    uint8_t      PackageSequenceTotalMax;
    uint16_t     CommandId;
    uint64_t     RequestId;
    XelUByte     TraceId[16]; // allow uuid
} XelPacketHeader;

X_API size_t XPH_Read(XelPacketHeader * HeaderPtr, const void * SourcePtr);
X_API void   XPH_Write(void * DestPtr, const XelPacketHeader * HeaderPtr);

X_CNAME_END
