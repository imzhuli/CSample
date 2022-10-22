#pragma once
#include "./X_Base.h"
#include <string.h>
#include <stdint.h>

#if defined(__APPLE__)
	#include <architecture/byte_order.h>
	#include <libkern/OSByteOrder.h>
#elif defined(__ANDROID_API__)
    #include <endian.h>
    #include <byteswap.h>
#elif defined(_POSIX_SOURCE)
	#include <endian.h>
	#include <byteswap.h>
#elif defined(_MSC_VER)
	#include <stdlib.h>
#else
	#error "no supported byte order operations yet"
#endif

#if defined(__APPLE__)
	#define X_ByteSwap16 OSSwapInt16
	#define X_ByteSwap32 OSSwapInt32
	#define X_ByteSwap64 OSSwapInt64
#elif defined(__ANDROID_API__)
    #define X_ByteSwap16 __swap16
    #define X_ByteSwap32 __swap32
    #define X_ByteSwap64 __swap64
#elif defined(_POSIX_SOURCE)
	#define X_ByteSwap16 __bswap_16
	#define X_ByteSwap32 __bswap_32
	#define X_ByteSwap64 __bswap_64
#elif defined(_MSC_VER)
	#define X_ByteSwap16 _byteswap_ushort
	#define X_ByteSwap32 _byteswap_ulong
	#define X_ByteSwap64 _byteswap_uint64
#endif

X_CNAME_BEGIN
/****************************************
* test if local endian is little endian
*/
#if BYTE_ORDER == LITTLE_ENDIAN
	#define Xel_IS_CONSIST_LITTLE_ENDIAN    true
	#define Xel_IS_CONSIST_BIG_ENDIAN       false
	X_STATIC_INLINE uint8_t  X_LE8 (const uint8_t s)  { return s; }
	X_STATIC_INLINE uint16_t X_LE16(const uint16_t s) { return s; }
	X_STATIC_INLINE uint32_t X_LE32(const uint32_t s) { return s; }
	X_STATIC_INLINE uint64_t X_LE64(const uint64_t s) { return s; }
	X_STATIC_INLINE uint8_t  X_BE8 (const uint8_t s)  { return s; }
	X_STATIC_INLINE uint16_t X_BE16(const uint16_t s) { return X_ByteSwap16(s); }
	X_STATIC_INLINE uint32_t X_BE32(const uint32_t s) { return X_ByteSwap32(s); }
	X_STATIC_INLINE uint64_t X_BE64(const uint64_t s) { return X_ByteSwap64(s); }
#elif BYTE_ORDER == BIG_ENDIAN
	#define Xel_IS_CONSIST_LITTLE_ENDIAN    false
	#define Xel_IS_CONSIST_BIG_ENDIAN       true
	X_STATIC_INLINE uint8_t  X_LE8 (const uint8_t s)  { return s; }
	X_STATIC_INLINE uint16_t X_LE16(const uint16_t s) { return X_ByteSwap16(s); }
	X_STATIC_INLINE uint32_t X_LE32(const uint32_t s) { return X_ByteSwap32(s); }
	X_STATIC_INLINE uint64_t X_LE64(const uint64_t s) { return X_ByteSwap64(s); }
	X_STATIC_INLINE uint8_t  X_BE8 (const uint8_t s)  { return s; }
	X_STATIC_INLINE uint16_t X_BE16(const uint16_t s) { return s; }
	X_STATIC_INLINE uint32_t X_BE32(const uint32_t s) { return s; }
	X_STATIC_INLINE uint64_t X_BE64(const uint64_t s) { return s; }
#else
	#error("Mixed endian is not supported by Xel");
#endif

typedef union {
	uint8_t   _1;
	uint16_t  _2;
    uint32_t  _4;
	uint64_t  _8;
	XelUByte _[8];
} XelUBytePunning;

typedef struct {
	const XelUByte * Current;
	const XelUByte * Start;
} XelStreamReader;

typedef struct {
	XelUByte * Current;
	XelUByte * Start;
} XelStreamWriter;

/* StreamReader */
X_STATIC_INLINE XelStreamReader XSR(const void * SourcePtr) {
	XelStreamReader Ctx;
	Ctx.Current = Ctx.Start = (const XelUByte *)SourcePtr;
	return Ctx;
}
X_STATIC_INLINE void XSR_Rewind(XelStreamReader * CtxPtr) {
	CtxPtr->Current = CtxPtr->Start;
}
X_STATIC_INLINE XelUByte XSR_B(XelStreamReader * CtxPtr) {
	return (*(CtxPtr->Current++));
}
X_STATIC_INLINE uint8_t XSR_1(XelStreamReader * CtxPtr) {
	return (uint8_t)(*(CtxPtr->Current++));
}
X_STATIC_INLINE uint16_t XSR_2(XelStreamReader * CtxPtr) {
	XelUBytePunning Punning;
	Punning._[0] = (*(CtxPtr->Current++));
	Punning._[1] = (*(CtxPtr->Current++));
	return X_BE16(Punning._2);
}
X_STATIC_INLINE uint32_t XSR_4(XelStreamReader * CtxPtr) {
	XelUBytePunning Punning;
	Punning._[0] = (*(CtxPtr->Current++));
	Punning._[1] = (*(CtxPtr->Current++));
	Punning._[2] = (*(CtxPtr->Current++));
	Punning._[3] = (*(CtxPtr->Current++));
	return X_BE32(Punning._4);
}
X_STATIC_INLINE uint64_t XSR_8(XelStreamReader * CtxPtr) {
	XelUBytePunning Punning;
	Punning._[0] = (*(CtxPtr->Current++));
	Punning._[1] = (*(CtxPtr->Current++));
	Punning._[2] = (*(CtxPtr->Current++));
	Punning._[3] = (*(CtxPtr->Current++));
	Punning._[4] = (*(CtxPtr->Current++));
	Punning._[5] = (*(CtxPtr->Current++));
	Punning._[6] = (*(CtxPtr->Current++));
	Punning._[7] = (*(CtxPtr->Current++));
	return X_BE64(Punning._8);
}
X_STATIC_INLINE uint8_t XSR_1L(XelStreamReader * CtxPtr) {
	return (uint8_t)(*(CtxPtr->Current++));
}
X_STATIC_INLINE uint16_t XSR_2L(XelStreamReader * CtxPtr) {
	XelUBytePunning Punning;
	Punning._[0] = (*(CtxPtr->Current++));
	Punning._[1] = (*(CtxPtr->Current++));
	return X_LE16(Punning._2);
}
X_STATIC_INLINE uint32_t XSR_4L(XelStreamReader * CtxPtr) {
	XelUBytePunning Punning;
	Punning._[0] = (*(CtxPtr->Current++));
	Punning._[1] = (*(CtxPtr->Current++));
	Punning._[2] = (*(CtxPtr->Current++));
	Punning._[3] = (*(CtxPtr->Current++));
	return X_LE32(Punning._4);
}
X_STATIC_INLINE uint64_t XSR_8L(XelStreamReader * CtxPtr) {
	XelUBytePunning Punning;
	Punning._[0] = (*(CtxPtr->Current++));
	Punning._[1] = (*(CtxPtr->Current++));
	Punning._[2] = (*(CtxPtr->Current++));
	Punning._[3] = (*(CtxPtr->Current++));
	Punning._[4] = (*(CtxPtr->Current++));
	Punning._[5] = (*(CtxPtr->Current++));
	Punning._[6] = (*(CtxPtr->Current++));
	Punning._[7] = (*(CtxPtr->Current++));
	return X_LE64(Punning._8);
}
X_STATIC_INLINE void XSR_Raw(XelStreamReader * CtxPtr, void * DestPtr, size_t Length) {
	memcpy(DestPtr, CtxPtr->Current, Length);
	CtxPtr->Current += Length;
}
X_STATIC_INLINE void XSR_Skip(XelStreamReader * CtxPtr, ptrdiff_t Length) {
	CtxPtr->Current += Length;
}
X_STATIC_INLINE const void * XSR_Ptr(XelStreamReader * CtxPtr) {
	return CtxPtr->Current;
}
X_STATIC_INLINE ptrdiff_t XSR_Offset(XelStreamReader * CtxPtr) {
	return (CtxPtr->Current - CtxPtr->Start);
}

/* StreamWriter */
X_STATIC_INLINE XelStreamWriter XSW(void * SourcePtr) {
	XelStreamWriter Ctx;
	Ctx.Current = Ctx.Start = (XelUByte *)SourcePtr;
	return Ctx;
}
X_STATIC_INLINE void XSW_Rewind(XelStreamWriter * CtxPtr) {
	CtxPtr->Current = CtxPtr->Start;
}
X_STATIC_INLINE void XSW_B(XelStreamWriter * CtxPtr, XelUByte Value) {
	(*(CtxPtr->Current++)) = Value;
}
X_STATIC_INLINE void XSW_1(XelStreamWriter * CtxPtr, uint8_t Value) {
	(*(CtxPtr->Current++)) = (XelUByte)Value;
}
X_STATIC_INLINE void XSW_2(XelStreamWriter * CtxPtr, uint16_t Value) {
	XelUBytePunning Punning;
	Punning._2 = X_BE16(Value);
	(*(CtxPtr->Current++)) = Punning._[0];
	(*(CtxPtr->Current++)) = Punning._[1];
}
X_STATIC_INLINE void XSW_4(XelStreamWriter * CtxPtr, uint32_t Value) {
	XelUBytePunning Punning;
	Punning._4 = X_BE32(Value);
	(*(CtxPtr->Current++)) = Punning._[0];
	(*(CtxPtr->Current++)) = Punning._[1];
	(*(CtxPtr->Current++)) = Punning._[2];
	(*(CtxPtr->Current++)) = Punning._[3];
}
X_STATIC_INLINE void XSW_8(XelStreamWriter * CtxPtr, uint64_t Value) {
	XelUBytePunning Punning;
	Punning._8 = X_BE64(Value);
	(*(CtxPtr->Current++)) = Punning._[0];
	(*(CtxPtr->Current++)) = Punning._[1];
	(*(CtxPtr->Current++)) = Punning._[2];
	(*(CtxPtr->Current++)) = Punning._[3];
	(*(CtxPtr->Current++)) = Punning._[4];
	(*(CtxPtr->Current++)) = Punning._[5];
	(*(CtxPtr->Current++)) = Punning._[6];
	(*(CtxPtr->Current++)) = Punning._[7];
}
X_STATIC_INLINE void XSW_1L(XelStreamWriter * CtxPtr, uint8_t Value) {
	(*(CtxPtr->Current++)) = (XelUByte)Value;
}
X_STATIC_INLINE void XSW_2L(XelStreamWriter * CtxPtr, uint16_t Value) {
	XelUBytePunning Punning;
	Punning._2 = X_LE16(Value);
	(*(CtxPtr->Current++)) = Punning._[0];
	(*(CtxPtr->Current++)) = Punning._[1];
}
X_STATIC_INLINE void XSW_4L(XelStreamWriter * CtxPtr, uint32_t Value) {
	XelUBytePunning Punning;
	Punning._4 = X_LE32(Value);
	(*(CtxPtr->Current++)) = Punning._[0];
	(*(CtxPtr->Current++)) = Punning._[1];
	(*(CtxPtr->Current++)) = Punning._[2];
	(*(CtxPtr->Current++)) = Punning._[3];
}
X_STATIC_INLINE void XSW_8L(XelStreamWriter * CtxPtr, uint64_t Value) {
	XelUBytePunning Punning;
	Punning._8 = X_LE64(Value);
	(*(CtxPtr->Current++)) = Punning._[0];
	(*(CtxPtr->Current++)) = Punning._[1];
	(*(CtxPtr->Current++)) = Punning._[2];
	(*(CtxPtr->Current++)) = Punning._[3];
	(*(CtxPtr->Current++)) = Punning._[4];
	(*(CtxPtr->Current++)) = Punning._[5];
	(*(CtxPtr->Current++)) = Punning._[6];
	(*(CtxPtr->Current++)) = Punning._[7];
}
X_STATIC_INLINE void XSW_Raw(XelStreamWriter * CtxPtr, const void * SourcePtr, size_t Length) {
	memcpy(CtxPtr->Current, SourcePtr, Length);
	CtxPtr->Current += Length;
}
X_STATIC_INLINE void XSW_Skip(XelStreamWriter * CtxPtr, ptrdiff_t Length) {
	CtxPtr->Current += Length;
}
X_STATIC_INLINE void * XSW_Ptr(XelStreamWriter * CtxPtr) {
	return CtxPtr->Current;
}
X_STATIC_INLINE ptrdiff_t XSW_Offset(XelStreamWriter * CtxPtr) {
	return (CtxPtr->Current - CtxPtr->Start);
}

X_CNAME_END