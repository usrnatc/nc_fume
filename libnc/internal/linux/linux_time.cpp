#include "linux_platform.h"
#include "../../nc_time.h"

struct TIME_INFORMATION {
    i64 PerformanceFrequency;
};
 
TIME_INFORMATION TIME_INFO = {};

void 
TimeInit(void)
{
    timespec Probe = {};

    if (UNLIKELY(clock_gettime(CLOCK_MONOTONIC_RAW, &Probe) != 0)) {
        ASSERT_ALWAYS(!"clock_gettime(CLOCK_MONOTONIC) call failed");
    }
 
    TIME_INFO.PerformanceFrequency = 1000000000LL;
}

INTERNAL DateTime
TimeTMToDateTime(tm TM, u32 MSecs)
{
    DateTime Result = {};

    Result.Second = TM.tm_sec;
    Result.Minute = TM.tm_min;
    Result.Hour = TM.tm_hour;
    Result.Day = TM.tm_mday - 1;
    Result.Month = TM.tm_mon;
    Result.Year = TM.tm_year + 1900;
    Result.MSec = MSecs;

    return Result;
}

INTERNAL tm
TimeDateTimeToTM(DateTime Time)
{
    tm Result = {};

    Result.tm_sec = Time.Second;
    Result.tm_min = Time.Minute;
    Result.tm_hour = Time.Hour;
    Result.tm_mday = Time.Day + 1;
    Result.tm_mon = Time.Month;
    Result.tm_year = Time.Year - 1900;

    return Result;
}

INTERNAL timespec
TimeDateTimeToTimeSpec(DateTime Time)
{
    tm TM = TimeDateTimeToTM(Time);
    time_t Seconds = timegm(&TM);
    timespec Result = {};

    Result.tv_sec = Seconds;

    return Result;
}

INTERNAL WallClockTime
TimeTimeSpecToWallClockTime(timespec TimeSpec)
{
    WallClockTime Result = 0;

    {
        tm TM = {};

        gmtime_r(&TimeSpec.tv_sec, &TM);

        DateTime Time = TimeTMToDateTime(TM, TimeSpec.tv_nsec / MILLION(1));
        
        Result = TimeDateTimeToWallClock(&Time);
    }

    return Result;
}

void 
SleepMSecs(u32 MSecs)
{
    usleep(MSecs * THOUSAND(1));
}

PerfCounter 
TimeGetTimestamp(void)
{
    timespec Now = {};
 
    clock_gettime(CLOCK_MONOTONIC_RAW, &Now);
 
    return (PerfCounter) (
        (u64) Now.tv_sec * (u64) 1000000000LL +
        (u64) Now.tv_nsec
    );
}

f64 
TimeElapsedSec(PerfCounter Start, PerfCounter End)
{
    ASSERT_ALWAYS(TIME_INFO.PerformanceFrequency > 0);

    i64 Delta = (i64) (End - Start);

    return (f64) Delta / (f64) TIME_INFO.PerformanceFrequency;
}

WallClockTime 
TimeGetWallClockUTC(void)
{
    timespec Now = {};
 
    clock_gettime(CLOCK_REALTIME, &Now);
 
    return TimeTimeSpecToWallClockTime(Now);
}

void 
TimeWallClockToDateTime(WallClockTime Time, OUT DateTime* DTime)
{
    DateTime TimeDate = {};

    TimeWallClockToDateTime(Time, &TimeDate);

    timespec TimeSpec = TimeDateTimeToTimeSpec(TimeDate);
    u64 Ticks100NSecs = Time;
    tm TM = {};
 
    if (gmtime_r(&TimeSpec.tv_sec, &TM)) {
        *DTime = TimeTMToDateTime(TM, (u32) (TimeSpec.tv_nsec / MILLION(1)));
 
        DTime->USec = (u16) ((Ticks100NSecs / 10) % 1000);
    }
}

WallClockTime 
TimeDateTimeToWallClock(DateTime* DTime)
{
    WallClockTime Result = 0;

    Result += DTime->Year;
    Result *= 12;
    Result += DTime->Month;
    Result *= 31;
    Result += DTime->Day;
    Result *= 24;
    Result += DTime->Hour;
    Result *= 60;
    Result += DTime->Minute;
    Result *= 61;
    Result += DTime->Second;
    Result *= 1000;
    Result += DTime->MSec;

    return Result;
}
