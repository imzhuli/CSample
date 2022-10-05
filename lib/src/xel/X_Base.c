#include <xel/X_Base.h>
#include <stdio.h>
#include <stdarg.h>

#ifndef NDEBUG
void X_DbgInfo(const char * fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	fprintf(stdout, "Debug: ");
	vfprintf(stdout, fmt, va);
	fprintf(stdout, "\n");
	va_end(va);
}
void X_DbgError(const char * fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	fprintf(stdout, "Error: ");
	vfprintf(stdout, fmt, va);
	fprintf(stdout, "\n");
	va_end(va);
}
#endif

void X_RuntimeAssert(bool Assertion, const char * Message)
{
    if (!Assertion) {
        fprintf(stderr, "X_RuntimeAssert failed: %s", Message);
        abort();
    }
}

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
