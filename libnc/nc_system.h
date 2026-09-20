#if !defined(__NC_SYSTEM_H__)
#define __NC_SYSTEM_H__

#include "nc_types.h"
#include "nc_arena.h"
#include "nc_string.h"
#include "nc_cli.h"

// @defines____________________________________________________________________
struct OSState;
struct OSEntity;
struct Arena;
struct Str8;
struct Handle;

typedef u64 WallClockTime;
typedef void ThreadFuncType(void* Param);

typedef u32 SystemAccessKind;
enum : u32 {
    SYS_ACCESS_READ        = (1 << 0),
    SYS_ACCESS_WRITE       = (1 << 1),
    SYS_ACCESS_EXECUTE     = (1 << 2),
    SYS_ACCESS_APPEND      = (1 << 3),
    SYS_ACCESS_SHARE_READ  = (1 << 4),
    SYS_ACCESS_SHARE_WRITE = (1 << 5),
    SYS_ACCESS_INHERITED   = (1 << 6)
};

enum EntityKind {
    ENTITY_KIND_NULL,
    ENTITY_KIND_THREAD,
    ENTITY_KIND_MUTEX,
    ENTITY_KIND_RWMUTEX,
    ENTITY_KIND_CONDVAR,
    ENTITY_KIND_BARRIER
};


extern void MainThreadBaseEntryPoint(int ArgC, char** ArgV);

// @types______________________________________________________________________
struct Handle {
    u64 V[1];
};

struct HandleNode {
    HandleNode* Next;
    Handle      Value;
};

struct HandleList {
    HandleNode* Head;
    HandleNode* Tail;
    u64         Count;
};

struct HandleArray {
    Handle* Data;
    u64     Count;
};

struct SystemProperties {
    Str8 MachineName;
    u64  AllocationGranularity;
    u64  LargePageSize;
    u64  PageSize;
    u32  LogicalProcessorCount;
};

struct ProcessProperties {
    Str8List ModuleLoadPaths;
    Str8List Environment;
    Str8     BinaryPath;
    Str8     InitialPath;
    Str8     UserProgramDataPath;
    u32      PID;
    b32      LargePagesAllowed;
};

// @runtime____________________________________________________________________
constexpr Handle EMPTY_HANDLE_VALUE = {};
extern OSState OS_STATE;
extern u64 GLOBAL_UPDATE_TICK_INDEX;
extern u64 ASYNC_THREADS_COUNT;
extern Handle ASYNC_TICK_BEGIN_COND_VAR;
extern Handle ASYNC_TICK_BEGIN_MTX;
extern Handle ASYNC_TICK_END_MTX;
extern b32 ASYNC_LOOP_REPEAT;
extern b32 ASYNC_LOOP_REPEAT_HIGH_PRIORITY;
extern b32 GLOBAL_ASYNC_EXIT;

// @functions__________________________________________________________________
b32 operator==(Handle HndlA, Handle HndlB);
b32 operator!=(Handle HndlA, Handle HndlB);
void ListPush(Arena* MemPool, HandleList* List, Handle Hndl);
HandleArray HandleArrayFromList(Arena* MemPool, HandleList* List);
OSEntity* EntityAlloc(EntityKind Kind);
void EntityDestroy(OSEntity* Entity);
Handle SharedMemoryAlloc(u64 Size, Str8 Name);
Handle SharedMemoryOpen(Str8 Name);
void SharedMemoryClose(Handle MemHandle);
void* SharedMemoryViewOpen(Handle MemHandle, r1u64 Range);
void SharedMemoryViewClose(Handle MemHandle, void* Ptr, r1u64 Range);
u64 GetUSecsResolution(void);
WallClockTime TimeNow(void);
u32 SleepMSecsFromEndTimeUSecs(u64 EndTimeUSecs);
void Abort(u32 Code);
void SysInit(void);
void EntryPoint(CommandLine* CLI);
SystemProperties* GetSystemProperties(void);
ProcessProperties* GetProcessProperties(void);
Str8 GetCurrentPath(Arena* MemPool);
u64 UpdateTickIndex(void);

#endif // __NC_SYSTEM_H__
