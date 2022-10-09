#pragma once
#include "../X_Base.h"

typedef void (*XelThreadRoutine)(void * ContextPtr);

X_CNAME_BEGIN

#if defined(X_SYSTEM_WINDOWS)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <process.h>

typedef HANDLE XelThreadId;

#elif defined(X_SYSTEM_LINUX) || defined(X_SYSTEM_MACOS) || defined(X_SYSTEM_IOS)

#include <pthread.h>

typedef pthread_t XelThreadId;

#else
#error "unsupported platform"
#endif

X_API bool X_CreateThread(XelThreadId * OuputThreadId, XelThreadRoutine Routine, void * ContextPtr);
X_API void X_DetachThread(XelThreadId ThreadId);
X_API void X_JoinThread(XelThreadId ThreadId);
X_API void X_SleepMS(size_t MS);

X_CNAME_END
