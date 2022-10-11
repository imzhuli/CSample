#include <xel/X_Base.h>
#include <stdio.h>
#include <stdarg.h>

XelVariable XEL_VARIABLE_NONE = XEL_VARIABLE_INIT;

#ifndef NDEBUG
void X_DbgInfo(const char * fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	fprintf(stdout, "Debug-Info: ");
	vfprintf(stdout, fmt, va);
	fprintf(stdout, "\n");
	va_end(va);
}
void X_DbgError(const char * fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	fprintf(stdout, "Debug-Error: ");
	vfprintf(stdout, fmt, va);
	fprintf(stdout, "\n");
	va_end(va);
}
#endif

void X_RuntimeAssert(bool Assertion, const char * Message)
{
    if (!Assertion) {
        fprintf(stderr, "X_RuntimeAssert failed: %s\n", Message);
        abort();
    }
}

void X_FatalAbort(const char * fmt, ...)
{
    if (!fmt) {
        fprintf(stderr, "Program aborts on unknown fatal error\n");
    }
    else {
        va_list va;
        va_start(va, fmt);
        fprintf(stderr, "Error: ");
        vfprintf(stderr, fmt, va);
        fprintf(stderr, "\n");
        va_end(va);
    }
    abort();
}
