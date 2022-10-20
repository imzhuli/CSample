#pragma once
#include "../X_Base.h"

typedef enum XelLoggerLevel
{
    XLL_NONE,
    XLL_ERROR,
    XLL_WARNING,
    XLL_INFO,
    XLL_DEBUG,
    XLL_VERBOSE,
} XelLoggerLevel;

X_API bool X_InitStdLogger(XelLoggerLevel LogLevel);
X_API bool X_InitLogger(const char * filename, XelLoggerLevel LogLevel);
X_API void X_CleanLogger();
X_API X_PRINTF_LIKE(1,2) void X_LogE(const char * fmt, ...);
X_API X_PRINTF_LIKE(1,2) void X_LogW(const char * fmt, ...);
X_API X_PRINTF_LIKE(1,2) void X_LogI(const char * fmt, ...);
X_API X_PRINTF_LIKE(1,2) void X_LogD(const char * fmt, ...);
X_API X_PRINTF_LIKE(1,2) void X_LogV(const char * fmt, ...);

#ifndef NDEBUG
#define X_LogDD X_LogD
#define X_LogDE X_LogE
#else
#define X_LogDD(fmt, ...)
#define X_LogDE(fmt, ...)
#endif
