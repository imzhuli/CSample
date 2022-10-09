#pragma once
#include "./X_Base.h"

X_CNAME_BEGIN

X_API uint64_t X_GetTimestamp();
X_API uint64_t X_GetTimestampMS();
X_API uint64_t X_GetTimestampUS();

typedef struct XelTimer XelTimer;
struct XelTimer
{
    uint64_t _MicroTimestamp;
};

X_STATIC_INLINE XelTimer XT_Now()
{
    XelTimer Now = { X_GetTimestampUS() };
    return Now;
}

X_STATIC_INLINE uint64_t XT_Elapsed(const XelTimer * Timer)
{
    uint64_t Now = X_GetTimestampUS();
    return (Now - Timer->_MicroTimestamp) * 1000000;
}

X_STATIC_INLINE uint64_t XT_ElapsedMS(const XelTimer * Timer)
{
    uint64_t Now = X_GetTimestampUS();
    return (Now - Timer->_MicroTimestamp) * 1000;
}

X_STATIC_INLINE uint64_t XT_ElapsedUS(const XelTimer * Timer)
{
    uint64_t Now = X_GetTimestampUS();
    return (Now - Timer->_MicroTimestamp);
}

X_STATIC_INLINE uint64_t XT_TagAndGetElapsed(XelTimer * Timer)
{
    uint64_t Now = X_GetTimestampUS();
    uint64_t Diff = (Now - Timer->_MicroTimestamp) * 1000000;
    Timer->_MicroTimestamp = Now;
    return Diff;
}

X_STATIC_INLINE uint64_t XT_TagAndGetElapsedMS(XelTimer * Timer)
{
    uint64_t Now = X_GetTimestampUS();
    uint64_t Diff = (Now - Timer->_MicroTimestamp) * 1000;
    Timer->_MicroTimestamp = Now;
    return Diff;
}

X_STATIC_INLINE uint64_t XT_TagAndGetElapsedUS(XelTimer * Timer)
{
    uint64_t Now = X_GetTimestampUS();
    uint64_t Diff = (Now - Timer->_MicroTimestamp);
    Timer->_MicroTimestamp = Now;
    return Diff;
}

X_STATIC_INLINE bool XT_TestAndTag(XelTimer *Timer, uint64_t Timeout)
{
    Timeout *= 1000000;
    uint64_t Now = X_GetTimestampUS();
    if (Now - Timer->_MicroTimestamp >= Timeout) {
        Timer->_MicroTimestamp = Now;
        return true;
    }
    return false;
}

X_STATIC_INLINE bool XT_TestAndTagMS(XelTimer *Timer, uint64_t Timeout)
{
    Timeout *= 1000;
    uint64_t Now = X_GetTimestampUS();
    if (Now - Timer->_MicroTimestamp >= Timeout) {
        Timer->_MicroTimestamp = Now;
        return true;
    }
    return false;
}

X_STATIC_INLINE bool XT_TestAndTagUS(XelTimer *Timer, uint64_t Timeout)
{
    uint64_t Now = X_GetTimestampUS();
    if (Now - Timer->_MicroTimestamp >= Timeout) {
        Timer->_MicroTimestamp = Now;
        return true;
    }
    return false;
}

X_STATIC_INLINE bool XT_Consume(XelTimer *Timer, uint64_t Timespan)
{
    uint64_t Target = Timer->_MicroTimestamp + Timespan * 1000000;
    uint64_t Now = X_GetTimestampUS();
    if ((int64_t)(Now - Target) >= 0) {
        Timer->_MicroTimestamp = Target;
        return true;
    }
    return false;
}

X_STATIC_INLINE bool XT_ConsumeMS(XelTimer *Timer, uint64_t Timespan)
{
    uint64_t Target = Timer->_MicroTimestamp + Timespan * 1000;
    uint64_t Now = X_GetTimestampUS();
    if ((int64_t)(Now - Target) >= 0) {
        Timer->_MicroTimestamp = Target;
        return true;
    }
    return false;
}

X_STATIC_INLINE bool XT_ConsumeUS(XelTimer *Timer, uint64_t Timespan)
{
    uint64_t Target = Timer->_MicroTimestamp + Timespan;
    uint64_t Now = X_GetTimestampUS();
    if ((int64_t)(Now - Target) >= 0) {
        Timer->_MicroTimestamp = Target;
        return true;
    }
    return false;
}

X_CNAME_END
