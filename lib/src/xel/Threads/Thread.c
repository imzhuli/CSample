#include <xel/Threads/X_Thread.h>
#include <inttypes.h>

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

#include <synchapi.h>

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

#endif
