#include <xel/Util/X_Logger.h>
#include <xel/Threads/X_Thread.h>
#include <xel/X_String.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

static XelString      LogFilename = NULL;
static XelLoggerLevel LogLevel = XLL_NONE;
static FILE *         LogFilePtr = NULL;
static XelMutex       LogMutex;

static bool CheckAndOpenLogfile_NoThreadSafety()
{
    if (LogFilePtr) {
        return true;
    }
    if (!LogFilename) {
        return false;
    }
    LogFilePtr = fopen(XS_GetData(LogFilename), "ab");
    return LogFilePtr;
}

static const char LogLevelHint[] =
{
    '-',  // silent
    'E',
    'W', 
    'I',
    'D',
    'V'
};

static void Log(XelLoggerLevel DesiredLogLevel, const char * fmt, va_list Vars)
{
    if (LogLevel < DesiredLogLevel) {
        return;
    }
    struct tm brokenTime;
    time_t now = time(NULL);
    X_LocalTime(&now, &brokenTime);

    X_LockMutex(&LogMutex);
    do {
        if (!CheckAndOpenLogfile_NoThreadSafety()) {
            X_UnlockMutex(&LogMutex);
            return;
        }
        fprintf(LogFilePtr, "%c-%02d%02d%02d:%02d%02d%02d ", LogLevelHint[(size_t)DesiredLogLevel],
            brokenTime.tm_year + 1900 - 2000, brokenTime.tm_mon + 1, brokenTime.tm_mday,
            brokenTime.tm_hour, brokenTime.tm_min, brokenTime.tm_sec
        );
        vfprintf(LogFilePtr, fmt, Vars);
        fputc('\n', LogFilePtr);
        fflush(LogFilePtr);   
    } while(false);
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

