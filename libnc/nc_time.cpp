#include "nc_time.h"

f64
TimeElapsedMSec(PerfCounter Start, PerfCounter End)
{
    return TimeElapsedSec(Start, End) * THOUSAND(1);
}

f64
TimeElapsedUSec(PerfCounter Start, PerfCounter End)
{
    return TimeElapsedSec(Start, End) * MILLION(1);
}
