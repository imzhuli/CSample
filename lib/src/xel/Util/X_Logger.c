#include <xel/Util/X_Logger.h>
#include <xel/Threads/X_Thread.h>
#include <xel/X_String.h>
#include <stdio.h>
#include <stdarg.h>

static XelString      LogFilename = NULL;
static XelLoggerLevel LogLevel = XLL_NONE;
static FILE *         LogFilePtr = NULL;
static XelMutex       LogMutex;

static bool CheckAndOpenLogfile_NoThreadSafety()
{
    if (LogFilePtr) {
        return false;
    }
    if (!LogFilename) {
        return false;
    }
    LogFilePtr = fopen(XS_GetData(LogFilename), "ab");
    return LogFilePtr;
}

static void Log(XelLoggerLevel DesiredLogLevel, const char * fmt, va_list Vars)
{
    if (LogLevel < DesiredLogLevel) {
        return;
    }
    X_LockMutex(&LogMutex);
    if (!CheckAndOpenLogfile_NoThreadSafety()) {
        X_UnlockMutex(&LogMutex);
        return;
    }

    vfprintf(LogFilePtr, fmt, Vars);
    fputc('\n', LogFilePtr);
    fflush(LogFilePtr);   

    X_UnlockMutex(&LogMutex);
}
bool X_InitStdLogger(XelLoggerLevel NewLogLevel)
{
    if (!X_InitMutex(&LogMutex)) {
        return false;
    }
    X_LockMutex(&LogMutex);
    LogFilePtr = stdout;
    LogLevel = NewLogLevel;
    X_UnlockMutex(&LogMutex);
    return true;
}

bool X_InitLogger(const char * Filename, XelLoggerLevel NewLogLevel)
{
    if (!X_InitMutex(&LogMutex)) {
        return false;
    }
    X_LockMutex(&LogMutex);
    if (Filename) {
        LogFilename = XS_NewString(Filename);
        if (!LogFilename) {
            X_UnlockMutex(&LogMutex);
            X_CleanMutex(&LogMutex);
            return false;
        }
    }
    LogLevel = NewLogLevel;
    X_UnlockMutex(&LogMutex);
    return true;
}

void X_CleanLogger()
{
    X_LockMutex(&LogMutex);
    if (LogFilePtr && LogFilePtr != stdout) {
        fclose(LogFilePtr);
    }
    LogFilePtr = NULL;
    XS_Delete(LogFilename);
    LogFilename = NULL;
    LogLevel = XLL_NONE;
    X_UnlockMutex(&LogMutex);
    X_CleanMutex(&LogMutex);
}

X_PRINTF_LIKE(1,2) void X_LogE(const char * fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    Log(XLL_ERROR, fmt, va);
    va_end(va);
}

X_PRINTF_LIKE(1,2) void X_LogW(const char * fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    Log(XLL_WARNING, fmt, va);
    va_end(va);
}

X_PRINTF_LIKE(1,2) void X_LogI(const char * fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    Log(XLL_INFO, fmt, va);
    va_end(va);
}

X_PRINTF_LIKE(1,2) void X_LogD(const char * fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    Log(XLL_DEBUG, fmt, va);
    va_end(va);
}

X_PRINTF_LIKE(1,2) void X_LogV(const char * fmt, ...)
{    
    va_list va;
    va_start(va, fmt);
    Log(XLL_VERBOSE, fmt, va);
    va_end(va);
}

