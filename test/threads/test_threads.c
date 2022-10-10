#include <xel/Threads/X_Thread.h>
#include <stdio.h>

XelMutex Mutex;
XelConditionalVariable CondVar;

XelAutoResetEvent Event;

void SubThread(void * ContextPtr)
{
    printf("SubThreadContextPtr=%p, routine=%p\n", ContextPtr, (void*)SubThread);
    X_SleepMS(100);
    do {
        X_LockMutex(&Mutex);        
        X_NotifyAllConditionalVariables(&CondVar);
        printf("SubThread notify\n");
        X_UnlockMutex(&Mutex);
    } while(false);

    X_SleepMS(100);
    X_NotifyAutoResetEvent(&Event);
    printf("SubThread exits\n");
}

int main(int argc, char * argv[])
{
    X_InitMutex(&Mutex);
    X_InitConditionalVariable(&CondVar);
    X_InitAutoResetEvent(&Event);

    XelThreadId ThreadId;
    if (!X_StartThread(&ThreadId, SubThread, SubThread)) {
        fprintf(stderr, "failed to create sub thread\n");
        exit(-1);
    }

    do {
        X_LockMutex(&Mutex);
        X_WaitForConditionalVariable(&CondVar, &Mutex);
        printf("MainThread CondVar notify received\n");
        X_UnlockMutex(&Mutex);
    } while(false);

    do {
        X_WaitForAutoResetEvent(&Event);
        printf("AutoResetEvent Triggered\n");
    } while(false);

    X_JoinThread(ThreadId);
    printf("SubThread joint\n");

    X_CleanAutoResetEvent(&Event);
    X_CleanConditionalVariable(&CondVar);
    X_CleanMutex(&Mutex);

    return 0;
}
