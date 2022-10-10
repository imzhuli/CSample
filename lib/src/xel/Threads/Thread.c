#include <xel/Threads/X_Thread.h>
#include <inttypes.h>
#include <errno.h>

typedef struct XelThreadRoutineWrapper
{
    XelThreadRoutine Routine;
    void * ContextPtr;
} XelThreadRoutineWrapper;

static XelThreadRoutineWrapper * XTRW_New(XelThreadRoutine Routine, void * ContextPtr)
{
    XelThreadRoutineWrapper * WrapperPtr = (XelThreadRoutineWrapper*)malloc(sizeof(XelThreadRoutineWrapper));
    WrapperPtr->Routine = Routine;
    WrapperPtr->ContextPtr = ContextPtr;
    return WrapperPtr;
}

static void XTRW_Delete(XelThreadRoutineWrapper * WrapperPtr) 
{
    free(WrapperPtr);
}

#if defined(X_SYSTEM_WINDOWS)

#include <process.h>

static unsigned X_ThreadWrapperProc(void* ContextPtr) // work with _beginthreadex, not _beginthread
{
    XelThreadRoutineWrapper * WrapperPtr = (XelThreadRoutineWrapper *)ContextPtr;
    (*WrapperPtr->Routine)(WrapperPtr->ContextPtr);
    XTRW_Delete(WrapperPtr);
    return 0;
}


bool X_StartThread(XelThreadId * OutputThreadId, XelThreadRoutine Routine, void * ContextPtr)
{
	if (0 == (*OutputThreadId = (XelThreadId)_beginthreadex(NULL, 0, X_ThreadWrapperProc, XTRW_New(Routine, ContextPtr), 0, NULL))) {
		return false;
	}
	return true;
}

void X_DetachThread(XelThreadId ThreadId)
{
    if (!CloseHandle(ThreadId)) {
        X_FatalAbort("Fatal when detaching thread: %p", (void *)ThreadId);
    }
}

void X_JoinThread(XelThreadId ThreadId)
{
    if (WAIT_OBJECT_0 !=WaitForSingleObject( ThreadId, INFINITE )) {
        X_FatalAbort("Fatal when joining thread: %p", (void *)ThreadId);
    }
}

void X_SleepMS(size_t MS)
{
    Sleep((DWORD)MS);
}

bool X_InitMutex(XelMutex * MutexPtr)
{
    InitializeCriticalSection(&MutexPtr->_CriticalSection);
    return true;
}

void X_CleanMutex(XelMutex * MutexPtr)
{
    DeleteCriticalSection(&MutexPtr->_CriticalSection);
}

void X_LockMutex(XelMutex * MutexPtr)
{
    EnterCriticalSection(&MutexPtr->_CriticalSection);
}

bool X_TryLockMutex(XelMutex * MutexPtr)
{
    return TryEnterCriticalSection(&MutexPtr->_CriticalSection);
}   

void X_UnlockMutex(XelMutex * MutexPtr)
{
    LeaveCriticalSection(&MutexPtr->_CriticalSection);
}

bool X_InitConditionalVariable(XelConditionalVariable * CondPtr)
{
    InitializeConditionVariable(&CondPtr->_CondVar);
    return true;
}

void X_CleanConditionalVariable(XelConditionalVariable * CondPtr)
{
    // no explicit cleanup for CONDITION_VARIABLE type;
}

void X_NotifyConditionalVariable(XelConditionalVariable * CondPtr)
{
    WakeConditionVariable(&CondPtr->_CondVar);
}

void X_NotifyAllConditionalVariables(XelConditionalVariable * CondPtr)
{
    WakeAllConditionVariable(&CondPtr->_CondVar);
}

void X_WaitForConditionalVariable(XelConditionalVariable * CondPtr, XelMutex * MutexPtr)
{
    SleepConditionVariableCS(&CondPtr->_CondVar, &MutexPtr->_CriticalSection, INFINITE);
}

#elif defined(X_SYSTEM_LINUX) || defined(X_SYSTEM_MACOS) || defined(X_SYSTEM_IOS)
#include <time.h>

static void * X_ThreadWrapperProc(void* ContextPtr)
{
    XelThreadRoutineWrapper * WrapperPtr = (XelThreadRoutineWrapper *)ContextPtr;
    (*WrapperPtr->Routine)(WrapperPtr->ContextPtr);
    XTRW_Delete(WrapperPtr);
    return NULL;
}

bool X_StartThread(XelThreadId * OutputThreadId, XelThreadRoutine Routine, void * ContextPtr)
{
	if (0 != pthread_create(OutputThreadId, NULL, X_ThreadWrapperProc, XTRW_New(Routine, ContextPtr))) {
		return false;
	}
	return true;
}

void X_DetachThread(XelThreadId ThreadId)
{
    int Error = pthread_detach(ThreadId);
    if (Error) {
        X_FatalAbort("Fatal when detaching thread: %" PRIxPTR "", (uintptr_t)ThreadId);
    }
}

void X_JoinThread(XelThreadId ThreadId)
{
    void * IgnoredReturnValue;
    int Error = pthread_join(ThreadId, &IgnoredReturnValue);
    if (Error) {
        X_FatalAbort("Fatal when joining thread: %" PRIxPTR "", (uintptr_t)ThreadId);
    }
}

void X_SleepMS(size_t MS)
{
    struct timespec Timeout = {
        MS / 1000,
        (MS % 1000) * 1000 * 1000,
    };
    while(-1 == nanosleep(&Timeout, &Timeout)) {
        X_Pass();
    }
}

bool X_InitMutex(XelMutex * MutexPtr)
{
    return 0 == pthread_mutex_init(&MutexPtr->_Mutex, NULL);
}

void X_CleanMutex(XelMutex * MutexPtr)
{
    X_RuntimeAssert(0 == pthread_mutex_destroy(&MutexPtr->_Mutex), "pthread_mutex_destroy should return 0");
}

void X_LockMutex(XelMutex * MutexPtr)
{
    int Result = pthread_mutex_lock(&MutexPtr->_Mutex);
    if (Result) {
        switch (Result) {
            case EINVAL:
                X_DbgError(
                    "pthread_mutex_lock failed (EINVAL), possible reasons are: \n%s",
                    "\t1. The value specified by mutex does not refer to an initialized mutex object.\n"
                    "\t2. The mutex was created with the protocol attribute having the value PTHREAD_PRIO_PROTECT and the calling thread's priority is higher than the mutex's current priority ceiling"
                    );
                break;
            case EAGAIN: 
                X_DbgError(
                    "pthread_mutex_lock failed (EAGAIN), possible reasons are: \n%s", 
                    "\tThe mutex could not be acquired because the maximum number of recursive locks for mutex has been exceeded."
                    );
                break;
            case ENOTRECOVERABLE:
                X_DbgError(
                    "pthread_mutex_lock failed (ENOTRECOVERABLE), possible reasons are: \n%s", 
                    "\tThe state protected by the mutex is not recoverable."
                    );
                break;
            case EOWNERDEAD:
                X_DbgError(
                    "pthread_mutex_lock failed (EBUSY), possible reasons are: \n%s", 
                    "\tThe mutex is a robust mutex and the previous owning thread terminated while holding the mutex lock. The mutex lock shall be acquired by the calling thread and it is up to the new owner to make the state consistent."
                    );
                break;
            case EDEADLK:
                X_DbgError(
                    "pthread_mutex_lock failed (EDEADLK), possible reasons are: \n%s", 
                    "\t1. The mutex type is PTHREAD_MUTEX_ERRORCHECK and the current thread already owns the mutex.\n"
                    "\t2. A deadlock condition was detected."
                    );
                break;
            default:
                X_Pass();
        }
        X_FatalAbort("pthread_mutex_lock failed, errno=%i", Result);
    }
}

bool X_TryLockMutex(XelMutex * MutexPtr)
{
    int Result = pthread_mutex_trylock(&MutexPtr->_Mutex);
    if (Result) {
        switch (Result) {
            case EBUSY:
                return false;
            case EAGAIN: 
                X_DbgError(
                    "pthread_mutex_trylock failed (EAGAIN), possible reasons are: \n%s", 
                    "\tThe mutex could not be acquired because the maximum number of recursive locks for mutex has been exceeded."
                    );
                    break;
            case EINVAL:
                X_DbgError(
                    "pthread_mutex_trylock failed (EINVAL), possible reasons are: \n%s",
                    "\t1. The value specified by mutex does not refer to an initialized mutex object.\n"
                    "\t2. The mutex was created with the protocol attribute having the value PTHREAD_PRIO_PROTECT and the calling thread's priority is higher than the mutex's current priority ceiling"
                    );
                    break;
            case ENOTRECOVERABLE:
                X_DbgError(
                    "pthread_mutex_trylock failed (ENOTRECOVERABLE), possible reasons are: \n%s", 
                    "\tThe state protected by the mutex is not recoverable."
                    );
                    break;
            case EOWNERDEAD:
                X_DbgError(
                    "pthread_mutex_trylock failed (EBUSY), possible reasons are: \n%s", 
                    "\tThe mutex is a robust mutex and the previous owning thread terminated while holding the mutex lock. The mutex lock shall be acquired by the calling thread and it is up to the new owner to make the state consistent."
                    );
                    break;
            default:
                X_Pass();
        }
        X_FatalAbort("pthread_mutex_trylock failed, errno=%i", Result);
        return false;
    }
    return true;
}

void X_UnlockMutex(XelMutex * MutexPtr)
{
    int Result = pthread_mutex_unlock(&MutexPtr->_Mutex);
    if (Result) {
        switch (Result) {
            case EPERM:
                X_DbgError(
                    "pthread_mutex_unlock failed, possible reasons are: \n%s",
                    "\tThe mutex type is PTHREAD_MUTEX_ERRORCHECK or PTHREAD_MUTEX_RECURSIVE, or the mutex is a robust mutex, and the current thread does not own the mutex."
                    );
                    break;
            default:
                X_Pass();
        }
        X_FatalAbort("pthread_mutex_unlock failed, errno=%i", Result);
    }
}

bool X_InitConditionalVariable(XelConditionalVariable * CondPtr)
{
    return 0 == pthread_cond_init(&CondPtr->_CondVar, NULL);
}

void X_CleanConditionalVariable(XelConditionalVariable * CondPtr)
{ 
    pthread_cond_destroy(&CondPtr->_CondVar);
}

void X_NotifyConditionalVariable(XelConditionalVariable * CondPtr)
{
    X_RuntimeAssert(0 == pthread_cond_signal(&CondPtr->_CondVar), "The value cond should refer to an initialized condition variable.");
}

void X_NotifyAllConditionalVariables(XelConditionalVariable * CondPtr)
{
    X_RuntimeAssert(0 == pthread_cond_broadcast(&CondPtr->_CondVar), "The value cond should refer to an initialized condition variable.");
}

void X_WaitForConditionalVariable(XelConditionalVariable * CondPtr, XelMutex * MutexPtr)
{
    X_RuntimeAssert(0 == pthread_cond_wait(&CondPtr->_CondVar, &MutexPtr->_Mutex), "The value cond & mutex should refer to valid object, correctly owned.");
}

#endif

// spinlock :

void X_InitSpinlock(XelSpinlock * LockPtr)
{
#if defined(XEL_LACK_ATOMIC)
    X_InitMutex(&LockPtr->_Mutex);
#else
    atomic_flag InitValue = ATOMIC_FLAG_INIT;
    LockPtr->_Flag = InitValue;
#endif
}

void X_CleanSpinlock(XelSpinlock * LockPtr)
{
#if defined(XEL_LACK_ATOMIC)
    X_CleanMutex(&LockPtr->_Mutex);
#else
    X_Pass();
#endif
}

void X_AcquireSpinlock(XelSpinlock * LockPtr)
{
#if defined(XEL_LACK_ATOMIC)
    X_LockMutex(&LockPtr->_Mutex);
#else
    while(!atomic_flag_test_and_set(&LockPtr->_Flag)) {
        X_Pass();
    }
#endif
}

X_API void X_ReleaseSpinlock(XelSpinlock * LockPtr)
{
#if defined(XEL_LACK_ATOMIC)
    X_UnlockMutex(&LockPtr->_Mutex);
#else
    atomic_flag_clear(&LockPtr->_Flag);
#endif
}


// AutoResetEvent

bool X_InitAutoResetEvent(XelAutoResetEvent * EventPtr)
{
    if (!X_InitMutex(&EventPtr->_Mutex)) {
        return false;
    }
    if (!X_InitConditionalVariable(&EventPtr->_CondVar)) {
        X_CleanMutex(&EventPtr->_Mutex);
        return false;
    }
    EventPtr->_HasEvent = false;
    return true;
}

void X_CleanAutoResetEvent(XelAutoResetEvent * EventPtr)
{
    X_CleanConditionalVariable(&EventPtr->_CondVar);
    X_CleanMutex(&EventPtr->_Mutex);
    EventPtr->_HasEvent = false;
}

void X_WaitForAutoResetEventAndLock(XelAutoResetEvent * EventPtr)
{
    X_LockMutex(&EventPtr->_Mutex);
    while (!EventPtr->_HasEvent) {
        X_WaitForConditionalVariable(&EventPtr->_CondVar, &EventPtr->_Mutex);
    }
}

void X_UnlockAutoResetEvent(XelAutoResetEvent * EventPtr)
{
    EventPtr->_HasEvent = false;
    X_UnlockMutex(&EventPtr->_Mutex);
}

void X_PrepareAutoResetEvent(XelAutoResetEvent * EventPtr)
{
    X_LockMutex(&EventPtr->_Mutex);
}

void X_CancelAutoResetEvent(XelAutoResetEvent * EventPtr)
{
    X_LockMutex(&EventPtr->_Mutex);
}

void X_FireAutoResetEvent(XelAutoResetEvent * EventPtr)
{
    if(EventPtr->_HasEvent) {
        X_UnlockMutex(&EventPtr->_Mutex);
        return;
    }
    EventPtr->_HasEvent = true;
    X_UnlockMutex(&EventPtr->_Mutex);
    X_NotifyConditionalVariable(&EventPtr->_CondVar);
}

