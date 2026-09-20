#include "nc_profile.h"
#include "nc_log.h"

INTERNAL ProfileState 
ProfileLoopBegin(char const* TimerName)
{
    ProfileState Result = {};

    Result.Name = Str((char*) TimerName);
    Result.Active = TRUE;
    Result.Start = TimeGetTimestamp();

    return Result;
}

b32 
ProfileLoopEnd(ProfileState* State)
{
    PerfCounter End = TimeGetTimestamp();
    f64 ElapsedMSecs = TimeElapsedMSec(State->Start, End);

    LogInfo("[PROFILE] %S: %.4lf ms", PRINT_STR(State->Name), ElapsedMSecs);

    return FALSE;
}
