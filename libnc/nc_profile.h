#if !defined(__NC_PROFILE_H__)
#define __NC_PROFILE_H__

#include "nc_types.h"
#include "nc_time.h"
#include "nc_string.h"

// @defines____________________________________________________________________
#define PROFILE(X) for (ProfileState GLUE(__ProfState, __LINE__) = ProfileLoopBegin(STRINGIFY(X)); GLUE(__ProfState, __LINE__).Active; GLUE(__ProfState, __LINE__).Active = ProfileLoopEnd(&GLUE(__ProfState, __LINE__)))

// @types______________________________________________________________________
struct ProfileState {
    Str8        Name;
    PerfCounter Start;
    b32         Active;
};

// @functions__________________________________________________________________
ProfileState ProfileLoopBegin(char const* TimerName);
b32 ProfileLoopEnd(ProfileState* State);

#endif // __NC_PROFILE_H__
