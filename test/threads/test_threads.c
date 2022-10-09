#include <xel/Threads/X_Thread.h>
#include <stdio.h>

void SubThread(void * ContextPtr)
{
    printf("SubThreadContextPtr=%p, routine=%p\n", ContextPtr, (void*)SubThread);
    X_SleepMS(100);
    printf("SubThread exits\n");
}

int main(int argc, char * argv[])
{
    XelThreadId ThreadId;
    if (!X_StartThread(&ThreadId, SubThread, SubThread)) {
        fprintf(stderr, "failed to create sub thread\n");
        exit(-1);
    }

    X_JoinThread(ThreadId);
    printf("SubThread joint\n");
    return 0;
}
