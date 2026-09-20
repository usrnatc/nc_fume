#if !defined(__NC_TIME_H__)
#define __NC_TIME_H__

#include "nc_types.h"

struct TIME_INFORMATION;

extern TIME_INFORMATION TIME_INFO;

// @defines____________________________________________________________________
#define MICROSECONDS(X) (X)
#define MILLISECONDS(X) THOUSAND(X)
#define SECONDS(X)      MILLION(X)
#define MINUTES(X)      (X * SECONDS(60))
#define HOURS(X)        (X * MINUTES(60))

// @types______________________________________________________________________
typedef u64 WallClockTime;
typedef u64 PerfCounter;

typedef u32 WeekDayType;
enum : u32 {
    SUNDAY,
    MONDAY,
    TUESDAY,
    WEDNESDAY,
    THURSDAY,
    FRIDAY,
    SATURDAY,
    WEEKDAY_COUNT
};

typedef u32 MonthType;
enum : u32 {
    JANURARY,
    FEBRUARY,
    MARCH,
    APRIL,
    MAY,
    JUNE,
    JULY,
    AUGUST,
    SEPTEMBER,
    OCTOBER,
    NOVEMBER,
    DECEMBER,
    MONTH_COUNT
};

struct DateTime {
    u16 Year;
    u16 Month;
    u16 Day;
    u16 Hour;
    u16 Minute;
    u16 Second;
    u16 MSec;
    u16 USec;
    u16 WeekDay;
};

// @functions__________________________________________________________________
void TimeInit(void);
void SleepMSecs(u32 MSecs);
PerfCounter TimeGetTimestamp(void);
f64 TimeElapsedSec(PerfCounter Start, PerfCounter End);
f64 TimeElapsedMSec(PerfCounter Start, PerfCounter End);
f64 TimeElapsedUSec(PerfCounter Start, PerfCounter End);
WallClockTime TimeGetWallClockUTC(void);
void TimeWallClockToDateTime(WallClockTime Time, OUT DateTime* DTime);
WallClockTime TimeDateTimeToWallClock(DateTime* DTime);

#endif // __NC_TIME_H__
