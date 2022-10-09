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

struct XelMutex
{
    pthread_mutex_t _Mutex;
};

#else
#error "unsupported platform"
#endif

X_API bool X_StartThread(XelThreadId * OutputThreadId, XelThreadRoutine Routine, void * ContextPtr);
X_API void X_DetachThread(XelThreadId ThreadId);
X_API void X_JoinThread(XelThreadId ThreadId);
X_API void X_SleepMS(size_t MS);

typedef struct XelMutex XelMutex;
X_API bool X_InitMutex(XelMutex * MutexPtr);
X_API void X_CleanMutex(XelMutex * MutexPtr);

X_API bool X_LockMutex(XelMutex * MutexPtr);
X_API bool X_TryLockMutex(XelMutex * MutexPtr);
X_API bool X_UnlockMutex(XelMutex * MutexPtr);

X_CNAME_END
