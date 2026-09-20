#if !defined(__NC_PRINT_H__)
#define __NC_PRINT_H__

#include "nc_types.h"
#include "nc_system.h"

// @defines____________________________________________________________________
extern u32 PRINT_STDOUT_FILENO;
extern u32 PRINT_STDERR_FILENO;
extern u32 PRINT_STDIN_FILENO;

struct Arena;
struct Str8;

// @types______________________________________________________________________
struct IOHandles {
    Handle Out;
    Handle Err;
    Handle In;
};

// @runtime____________________________________________________________________
extern IOHandles IO_HANDLES;

// @functions__________________________________________________________________
void PrintInit(void);
void PrintRelease(void);
void PrintOut(Str8 Msg);
void PrintOut(char* Fmt, ...);
void PrintErr(Str8 Msg);
void PrintErr(char* Fmt, ...);
Str8 UserIn(Arena* MemPool, Str8 Prompt);
Str8 UserIn(Arena* MemPool);
void SetStdOut(i32 NewStdOut);
void SetStdErr(i32 NewStdErr);
void SetStdIn(i32 NewStdIn);
Handle GetStdOut(void);
Handle GetStdErr(void);
Handle GetStdIn(void);

#endif // __NC_PRINT_H__
