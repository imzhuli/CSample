#pragma once
#include "../X_Base.h"

typedef void (*XelThreadRoutine)(void * ContextPtr);

X_CNAME_BEGIN

#if defined(X_SYSTEM_WINDOWS)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <synchapi.h>

typedef HANDLE XelThreadId;

struct XelMutex
{
    CRITICAL_SECTION _CriticalSection;
};

struct XelConditionalVariable
{
    CONDITION_VARIABLE _CondVar;
};

#elif defined(X_SYSTEM_LINUX) || defined(X_SYSTEM_MACOS) || defined(X_SYSTEM_IOS)

#include <pthread.h>

typedef pthread_t XelThreadId;

struct XelMutex
{
    pthread_mutex_t _Mutex;
};

struct XelConditionalVariable
{
    pthread_cond_t _Cond;
    bool           _StopWaiting;
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

typedef struct XelConditionalVariable XelConditionalVariable;
X_API bool X_InitConditionalVariable(XelConditionalVariable * CondPtr);
X_API void X_CleanConditionalVariable(XelConditionalVariable * CondPtr);
X_API void X_NotifyConditionalVariable(XelConditionalVariable * CondPtr);
X_API void X_NotifyAllConditionalVariables(XelConditionalVariable * CondPtr);
X_API void X_WaitForConditionalVariable(XelConditionalVariable * CondPtr, XelMutex * MutexPtr);

struct XelAutoResetEvent
{
    struct XelMutex                 _Mutex;
    struct XelConditionalVariable   _CondVar;
    bool                            _HasEvent;
};
typedef struct XelAutoResetEvent XelAutoResetEvent;
X_API bool X_InitAutoResetEvent(XelAutoResetEvent * EventPtr);
X_API void X_CleanAutoResetEvent(XelAutoResetEvent * EventPtr);
X_API void X_WaitForAutoResetEvent(XelAutoResetEvent * EventPtr);
X_API void X_NotifyAutoResetEvent(XelAutoResetEvent * EventPtr);

X_CNAME_END
