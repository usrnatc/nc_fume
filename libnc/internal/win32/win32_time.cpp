#include "win32_platform.h"
#include "../nc_time.h"

struct TIME_INFORMATION {
    i64 PerformanceFrequency;
};

TIME_INFORMATION TIME_INFO = {};

void 
TimeInit(void)
{
    LARGE_INTEGER Frequency;

    if (UNLIKELY(!QueryPerformanceFrequency(&Frequency))) {
        ASSERT_ALWAYS(!"QueryPerformanceFrequency call failed");
    }

    TIME_INFO.PerformanceFrequency = (i64) Frequency.QuadPart;
}

PerfCounter
TimeGetTimestamp(void)
{
    LARGE_INTEGER Result;

    QueryPerformanceCounter(&Result);

    return (PerfCounter) Result.QuadPart;
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
    FILETIME FTime;

    GetSystemTimeAsFileTime(&FTime);

    return ((u64) FTime.dwHighDateTime << 32) | FTime.dwLowDateTime;
}

void
TimeWallClockToDateTime(WallClockTime Time, OUT DateTime* DTime)
{
    FILETIME FTime;
    SYSTEMTIME STime;
    u64 Ticks100NSecs = Time;

    FTime.dwLowDateTime = (u32) (Time & U32_MAX);
    FTime.dwHighDateTime = (u32) (Time >> 32);

    if (FileTimeToSystemTime(&FTime, &STime)) {
        DTime->Year = STime.wYear;
        DTime->Month = STime.wMonth;
        DTime->Day = STime.wDay;
        DTime->Hour = STime.wHour;
        DTime->Minute = STime.wMinute;
        DTime->Second = STime.wSecond;
        DTime->MSec = STime.wMilliseconds;
        DTime->WeekDay = STime.wDayOfWeek;
        DTime->USec = (u16) ((Ticks100NSecs / 10) % 1000);
    }
}

WallClockTime
TimeDateTimeToWallClock(DateTime* DTime)
{
    SYSTEMTIME STime = {};
    FILETIME FTime;

    STime.wYear = DTime->Year;
    STime.wMonth = DTime->Month;
    STime.wDay = DTime->Day;
    STime.wHour = DTime->Hour;
    STime.wMinute = DTime->Minute;
    STime.wSecond = DTime->Second;
    STime.wMilliseconds = DTime->MSec;
    STime.wDayOfWeek = DTime->WeekDay;

    if (SystemTimeToFileTime(&STime, &FTime)) {
        u64 Result = ((u64) FTime.dwHighDateTime << 32) | FTime.dwLowDateTime;

        Result += (u64) DTime->USec * 10;

        return Result;
    }

    return 0;
}

void
SleepMSecs(u32 MSecs)
{
    Sleep(MSecs);
}
