#pragma once

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
	//define something for Windows (32-bit and 64-bit, this part is common)
	#pragma warning (disable:4180)
	#pragma warning (disable:4200)
	#pragma warning (disable:4819)
	#define X_SYSTEM_WINDOWS
	#ifdef _WIN64
		//define something for Windows (64-bit only)
	  	#define X_SYSTEM_WIN64
	#else
		//define something for Windows (32-bit only)
		#define X_SYSTEM_WIN32
	#endif
#elif __APPLE__
	#define X_SYSTEM_APPLE
	#include <TargetConditionals.h>
	#if TARGET_IPHONE_SIMULATOR
		// iOS Simulator
		#define X_SYSTEM_IPHONE_SIMULATOR
	#elif TARGET_OS_IPHONE
		// iOS device
		#define X_SYSTEM_IPHONE
	#elif TARGET_OS_MAC
		// Other kinds of Mac OS
		#define X_SYSTEM_MACOS
	#else
	#	error "Unknown Apple platform"
	#endif
#elif defined(__ANDROID_API__)
	#define X_SYSTEM_LINUX
	#define X_SYSTEM_ANDROID
#elif __linux__
	// linux
	#define X_SYSTEM_LINUX
	#ifdef __FreeBSD__
		#define X_SYSTEM_FREEBSD
	#endif
#elif __unix__ // all unices not caught above
	// Unix
	#error "unsupported unix"
#elif defined(_POSIX_VERSION)
	// POSIX
	#error "unsupported posix"
#else
	#error "Unknown system type"
#endif

#if defined(X_SYSTEM_WINDOWS) || defined(X_SYSTEM_LINUX) || defined(X_SYSTEM_MACOS)
#ifndef X_SYSTEM_ANDROID
	#define X_SYSTEM_DESKTOP
#endif
#endif

#if defined(X_SYSTEM_IPHONE_SIMULATOR) || defined(X_SYSTEM_IPHONE)
	#define X_SYSTEM_IOS
#endif

#if defined(X_SYSTEM_IOS) || defined(X_SYSTEM_MACOS)
	#define X_SYSTEM_DARWIN
#endif

#if defined(_MSC_VER)

	#ifndef NOMINMAX
		#define NOMINMAX
	#endif

	#define X_LIKELY(x)                    (x)
	#define X_UNLIKELY(x)                  (x)
	#define X_PRINTF_LIKE(a, b)

	#define X_INLINE                       __forceinline
	#define X_STATIC_INLINE                static __forceinline

	#define X_PRIVATE                      extern
	#define X_PRIVATE_MEMBER
	#define X_PRIVATE_STATIC_MEMBER        static
	#define X_PRIVATE_CONSTEXPR            constexpr
	#define X_PRIVATE_STATIC_CONSTEXPR     static constexpr
	#define X_PRIVATE_INLINE               __forceinline

	#define X_EXPORT                       __declspec(dllexport) extern
	#define X_EXPORT_MEMBER                __declspec(dllexport)
	#define X_EXPORT_STATIC_MEMBER         __declspec(dllexport) static
	#define X_IMPORT                       __declspec(dllimport) extern
	#define X_IMPORT_MEMBER                __declspec(dllimport)
	#define X_IMPORT_STATIC_MEMBER         __declspec(dllimport) static

#elif defined(__clang__) || defined(__GNUC__)

	#define X_LIKELY(x)                    __builtin_expect(!!(x), 1)
	#define X_UNLIKELY(x)                  __builtin_expect(!!(x), 0)
	#define X_PRINTF_LIKE(f, a)            __attribute__((format(printf, f, a)))

	#define X_INLINE                       __attribute__((always_inline)) inline
	#define X_STATIC_INLINE                __attribute__((always_inline)) static inline

	#define X_PRIVATE                      __attribute__((__visibility__("hidden"))) extern
	#define X_PRIVATE_MEMBER               __attribute__((__visibility__("hidden")))
	#define X_PRIVATE_STATIC_MEMBER        __attribute__((__visibility__("hidden"))) static
	#define X_PRIVATE_CONSTEXPR            __attribute__((__visibility__("hidden"))) constexpr
	#define X_PRIVATE_STATIC_CONSTEXPR     __attribute__((__visibility__("hidden"))) static constexpr
	#define X_PRIVATE_INLINE               __attribute__((always_inline)) __attribute__((__visibility__("hidden"))) inline

	#define X_EXPORT                       __attribute__((__visibility__("default"))) extern
	#define X_EXPORT_MEMBER                __attribute__((__visibility__("default")))
	#define X_EXPORT_STATIC_MEMBER         __attribute__((__visibility__("default"))) static
	#define X_IMPORT                       extern
	#define X_IMPORT_MEMBER
	#define X_IMPORT_STATIC_MEMBER         static

#else
	#error "Unsupported compiler"
#endif

#define X_EXTERN             extern
#define X_MEMBER
#define X_STATIC_MEMBER      static
#if defined(X_OPTION_STATIC)
	#if defined(X_OPTION_EXPORT_API)
		#error X_OPTION_STATIC is used with X_OPTION_EXPORT_API
	#endif
	#define X_API                      X_EXTERN
	#define X_API_MEMBER               X_MEMBER
	#define X_API_STATIC_MEMBER        X_STATIC_MEMBER
#else
	#if defined(X_OPTION_EXPORT_API)
		#define X_API                  X_EXPORT
		#define X_API_MEMBER           X_EXPORT_MEMBER
		#define X_API_STATIC_MEMBER    X_EXPORT_STATIC_MEMBER
	#else
		#define X_API                  X_IMPORT
		#define X_API_MEMBER           X_IMPORT_MEMBER
		#define X_API_STATIC_MEMBER    X_IMPORT_STATIC_MEMBER
	#endif
#endif

#ifndef __cplusplus
#define X_CNAME
#define X_CNAME_BEGIN
#define X_CNAME_END
#else
#define X_CNAME extern "C"
#define X_CNAME_BEGIN extern "C" {
#define X_CNAME_END }
#endif

#define X_MAKE_STRING(s) #s
#define X_EXPAND_STRING(s) X_MAKE_STRING(s)
#define X_EXPECT(x) do { bool test=(bool)(x); if(!test) { throw #x; } } while(0)

#if defined(_MSC_VER)
#	define X_AlignedAlloc                      _aligned_malloc
#	define X_AlignedFree                       _aligned_free
#   define X_LocalTime(Timestamp, ST)          localtime_s((ST), (Timestamp));
#else
#	define X_AlignedAlloc(size, alignment)     aligned_alloc(alignment, size)
#	define X_AlignedFree                       free
#   define X_LocalTime(Timestamp, ST)          localtime_r((Timestamp), (ST));
#endif

#if defined(X_SYSTEM_ANDROID)
#	define _X_ANDROID_API__FUNC_NAME_JOIN(PackageName, ClassFuncName) \
		Java_ ## PackageName ## _ ## ClassFuncName
#	define X_ANDROID_API__FUNC_NAME(PackageName, ClassFuncName) \
		_X_ANDROID_API__FUNC_NAME_JOIN(PackageName, ClassFuncName)

#if defined(ANDROID_PACKAGE_CNAME)
#	define _X_ANDROID_API(ClassFuncName, ReturnType) \
		X_CNAME JNIEXPORT ReturnType JNICALL X_ANDROID_API__FUNC_NAME(ANDROID_PACKAGE_CNAME, ClassFuncName)
#	define X_ANDROID_API(ClassName, FuncName, ReturnType) _X_ANDROID_API(ClassName ## _ ## FuncName, ReturnType)
#	endif
#endif

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

X_CNAME_BEGIN

typedef unsigned char XelUByte;

typedef union XelVariable XelVariable;
union XelVariable
{
	XelUByte                      B[8];

	void *                        Ptr;
	const void *                  ConstPtr;

	ptrdiff_t                     Offset;
	size_t                        Size;

	int                           I;
	unsigned int                  U;
	int8_t                        I8;
	int16_t                       I16;
	int32_t                       I32;
	int64_t                       I64;
	uint8_t                       U8;
	uint16_t                      U16;
	uint32_t                      U32;
	uint64_t                      U64;

	struct { int32_t  x, y; }     IV2;
	struct { uint32_t x, y; }     UV32;
};
#define XEL_VARIABLE_INIT {{0}}
X_API XelVariable XEL_VARIABLE_NONE;
X_STATIC_INLINE XelVariable XV_None() { XelVariable InitObject = {{0}}; return InitObject; }

typedef struct XelOptionalVariable XelOptionalVariable;
struct XelOptionalVariable
{
	bool          HasValue;
	XelVariable   Value;
};
X_STATIC_INLINE XelOptionalVariable XOV_NoValue()                   { XelOptionalVariable InitObject = { false }; return InitObject; }
X_STATIC_INLINE XelOptionalVariable XOV_Value(XelVariable Value)    { XelOptionalVariable InitObject = { true, Value }; return InitObject; }

#define X_AddressOf(T) (&(T))
X_STATIC_INLINE void * X_Cast(void * MemberPtr, size_t MemberOffset)
{
	if (!MemberPtr) {
		return NULL;
	}
    return (void*)((unsigned char*)MemberPtr - MemberOffset);
}
#define X_Entry(_MemberPtr, Type, Member) ((Type*)(X_Cast((_MemberPtr), offsetof(Type, Member))))

X_API void X_RuntimeAssert(bool Assertion, const char * Message);
X_STATIC_INLINE void X_Pass() {}

#define X_RTA(x) X_RuntimeAssert((x), #x);
X_API X_PRINTF_LIKE(1, 2) void X_FatalAbort(const char * fmt, ...);

#ifndef NDEBUG
X_API X_PRINTF_LIKE(1,2) void X_DbgInfo(const char * fmt, ...);
X_API X_PRINTF_LIKE(1,2) void X_DbgError(const char * fmt, ...);
#else
#define X_DbgInfo(fmt, ...)
#define X_DbgError(fmt, ...)
#endif

typedef struct XelDataView XelDataView;
struct XelDataView {
    const void * DataPtr;
    size_t       Size;
};

typedef struct XelBufferView XelBufferView;
struct XelBufferView
{
	XelUByte *   DataPtr;
	size_t       Size;
};

#define XEL_HANDLE struct{void*__PlaceHolder__;}*

#ifndef NDEBUG
#define X_DEBUG_PRINTF printf
#define X_DEBUG_FPRINTF fprintf
#else
#define X_DEBUG_PRINTF(...) X_Pass()
#define X_DEBUG_FPRINTF(...) X_Pass()
#endif

X_CNAME_END
