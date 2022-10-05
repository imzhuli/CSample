#include <xel/X_Chrono.h>

#ifdef X_SYSTEM_WINDOWS
    #define WIN32_LEAN_AND_MEAN
    #include <Windows.h>

    static const uint64_t EPOCH = ((uint64_t) 116444736000000000ULL);
    X_STATIC_INLINE uint64_t Internal_MicroTimestamp()
    {
        // Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
        // This magic number is the number of 100 nanosecond intervals since January 1, 1601 (UTC)
        // until 00:00:00 January 1, 1970
        SYSTEMTIME  system_time;
        FILETIME    file_time;
        uint64_t    time;

        GetSystemTime( &system_time );
        SystemTimeToFileTime( &system_time, &file_time );
        time =  ((uint64_t)file_time.dwLowDateTime )      ;
        time += ((uint64_t)file_time.dwHighDateTime) << 32;
        return ((time - EPOCH) / 10L);
    }

#else
    #include <sys/time.h>

    X_STATIC_INLINE uint64_t Internal_MicroTimestamp() {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return tv.tv_sec * 1000000 + tv.tv_usec;
    }
#endif

uint64_t X_GetTimestamp()
{
    return Internal_MicroTimestamp() / 1000000;
}

uint64_t X_GetTimestampMS()
{
    return Internal_MicroTimestamp() / 1000;
}

uint64_t X_GetTimestampUS()
{
    return Internal_MicroTimestamp();
}
