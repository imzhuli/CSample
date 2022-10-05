#include <xel/X_Base.h>
#include <stdio.h>

void X_FatalAbort(const char * Reason)
{
    if (Reason) {
        fprintf(stderr, "Program aborts on fatal error: %s\n", Reason);
    }
    else {
        fprintf(stderr, "Program aborts on unknown fatal error\n");
    }
    abort();
}
