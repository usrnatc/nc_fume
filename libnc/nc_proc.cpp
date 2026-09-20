#include "nc_proc.h"

API_INTERNAL Str8List
StringListFromCommandLineArgs(Arena* MemPool, int ArgC, char** ArgV)
{
    Str8List Result = {};

    for (int I = 0; I < ArgC; ++I) {
        Str8 String = CStr8(ArgV[I]);

        ListPush(MemPool, &Result, String);
    }

    return Result;
}
