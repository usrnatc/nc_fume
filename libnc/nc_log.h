#if !defined(__NC_LOG_H__)
#define __NC_LOG_H__

#include "nc_types.h"
#include "nc_string.h"
#include "nc_ring.h"
#include "nc_thread.h"
#include "nc_time.h"

struct LogEndpoint;
struct LogEntry;

// @examples___________________________________________________________________
//
// socket______________________________________________________________________
// SocketInfo LogSockAndAddress = {
//     EMPTY_HANDLE_VALUE,
//     {}
// };
//
// void
// AppRegisterLogEndpoints(void)
// {
//     LogSockAndAddress.Socket = SocketOpen(
//         SOCKET_TCP,                                  // or SOCKET_UDP
//         FALSE
//     );
//     LogSockAndAddress.Address = SockAddrLocal(8888); // localhost:8888
//     SocketSetFlag(
//         LogSockAndAddress.Socket,
//         SOCKET_FLAG_NONBLOCKING | SOCKET_FLAG_NODELAY
//     );
//     SocketConnect(LogSockAndAddress.Socket, LogSockAndAddress.Address);
//
//     LogEndpointParams LogSockParams = {
//         "SOCKET"_s8,
//         LOG_ENDPOINT_FUNC_NAME(Socket),
//         LOG_ENDPOINT_FLUSH_FUNC_NAME(Socket),
//         &LogSockAndAddress,
//         sizeof(LogSockAndAddress),
//         LOG_LEVEL_KIND_NORMAL
//     };
//
//     LogRegisterEndpoint(LogSockParams);
// }
//
// AppUnregisterLogEndpoints(void)
// {
//     if (LogSockAndAddress.Socket != EMPTY_HANDLE_VALUE)
//         SocketClose(LogSockAndAddress.Socket);
// }
//
// file________________________________________________________________________
// Handle LogFile = EMPTY_HANDLE_VALUE;
//
// void
// AppRegisterLogEndpoints(void)
// {
//     LogFile = SystemOpenFile(
//         SYS_ACCESS_APPEND | SYS_ACCESS_WRITE,
//         "nc_log.txt"_s8
//     );
//
//     LogEndpointParams LogFileParams = {
//         "FILE"_s8,
//         LOG_ENDPOINT_FUNC_NAME(File),
//         LOG_ENDPOINT_FLUSH_FUNC_NAME(File),
//         &LogFile,
//         sizeof(LogFile),
//         LOG_LEVEL_KIND_NORMAL
//     };
//
//     LogRegisterEndpoint(LogFileParams);
// }
//
// AppUnregisterLogEndpoints(void)
// {
//     if (LogFile != EMPTY_HANDLE_VALUE)
//         SystemCloseFile(LogFile);
// }
//
// console_____________________________________________________________________
// void
// AppRegisterLogEndpoints(void)
// {
//     LogEndpointParams LogConsoleParams = {
//         "CONSOLE"_s8,
//         LOG_ENDPOINT_FUNC_NAME(Console),
//         LOG_ENDPOINT_FLUSH_FUNC_NAME(Console),
//         NULL,
//         0,
//         LOG_LEVEL_KIND_NORMAL,
//     };
//
//     LogRegisterEndpoint(LogConsoleParams);
// }

// @defines____________________________________________________________________
typedef u64 LogEndpointID;

#define LOG_MAX_ACTIVE_ENTRIES        256
#define LOG_MAX_REGISTERED_ENDPOINTS   64
#define LOG_MAX_STAGE_SIZE           1024
#define LOG_SCOPE_MAX_DEPTH          U8_MAX

#define LOG_COLOUR_NORMAL  "\x1B[0m"
#define LOG_COLOUR_RED     "\x1B[31m"
#define LOG_COLOUR_GREEN   "\x1B[32m"
#define LOG_COLOUR_YELLOW  "\x1B[33m"
#define LOG_COLOUR_BLUE    "\x1B[34m"
#define LOG_COLOUR_MAGENTA "\x1B[35m"
#define LOG_COLOUR_CYAN    "\x1B[36m"
#define LOG_COLOUR_WHITE   "\x1B[37m"
#define LOG_COLOUR_RESET   "\033[0m"

#if defined(NC_DEBUG)
    #define LogNormal(...)           __LogBase(LOG_ENDPOINT_SELECT_KIND_ALL, EMPTY_LOG_ENDPOINT_ID_VALUE, __FILE__, __LINE__, LOG_LEVEL_KIND_NORMAL,  __VA_ARGS__)
    #define LogSuccess(...)          __LogBase(LOG_ENDPOINT_SELECT_KIND_ALL, EMPTY_LOG_ENDPOINT_ID_VALUE, __FILE__, __LINE__, LOG_LEVEL_KIND_SUCCESS, __VA_ARGS__)
    #define LogInfo(...)             __LogBase(LOG_ENDPOINT_SELECT_KIND_ALL, EMPTY_LOG_ENDPOINT_ID_VALUE, __FILE__, __LINE__, LOG_LEVEL_KIND_INFO,    __VA_ARGS__)
    #define LogWarn(...)             __LogBase(LOG_ENDPOINT_SELECT_KIND_ALL, EMPTY_LOG_ENDPOINT_ID_VALUE, __FILE__, __LINE__, LOG_LEVEL_KIND_WARN,    __VA_ARGS__)
    #define LogError(...)            __LogBase(LOG_ENDPOINT_SELECT_KIND_ALL, EMPTY_LOG_ENDPOINT_ID_VALUE, __FILE__, __LINE__, LOG_LEVEL_KIND_ERROR,   __VA_ARGS__)
    #define LogPanic(...)            __LogBase(LOG_ENDPOINT_SELECT_KIND_ALL, EMPTY_LOG_ENDPOINT_ID_VALUE, __FILE__, __LINE__, LOG_LEVEL_KIND_PANIC,   __VA_ARGS__)
    #define LogTo(ID, Severity, ...) __LogBase(LOG_ENDPOINT_SELECT_KIND_ID, Hash((ID)), __FILE__, __LINE__, (Severity), __VA_ARGS__)
#else
    #define LogNormal(...)
    #define LogSuccess(...)
    #define LogInfo(...)
    #define LogWarn(...)
    #define LogError(...)
    #define LogPanic(...)
    #define LogTo(ID, Severity, ...)
#endif

#define LOG_ENDPOINT_FUNC_SIG(X)  void X(LogEndpoint* Endpoint, LogEntry* Entry)
#define LOG_ENDPOINT_FUNC_NAME(X) GLUE(__LogEndpointFunc, X)
#define LOG_ENDPOINT_FUNC_DEF(X)  LOG_ENDPOINT_FUNC_SIG(LOG_ENDPOINT_FUNC_NAME(X))

typedef LOG_ENDPOINT_FUNC_SIG(LogEndpointFuncKind);

#define LOG_ENDPOINT_FLUSH_FUNC_SIG(X)  void X(LogEndpoint* Endpoint)
#define LOG_ENDPOINT_FLUSH_FUNC_NAME(X) GLUE(__LogEndpointFlushFunc, X)
#define LOG_ENDPOINT_FLUSH_FUNC_DEF(X)  LOG_ENDPOINT_FLUSH_FUNC_SIG(LOG_ENDPOINT_FLUSH_FUNC_NAME(X))

typedef LOG_ENDPOINT_FLUSH_FUNC_SIG(LogEndpointFlushFuncKind);

typedef u8 LogLevelKind;
enum : u8 {
    LOG_LEVEL_KIND_NORMAL,
    LOG_LEVEL_KIND_SUCCESS,
    LOG_LEVEL_KIND_INFO,
    LOG_LEVEL_KIND_WARN,
    LOG_LEVEL_KIND_ERROR,
    LOG_LEVEL_KIND_PANIC,
    LOG_LEVEL_KIND_COUNT
};

typedef u8 LogEndpointSelectKind;
enum : u8 {
    LOG_ENDPOINT_SELECT_KIND_ID,
    LOG_ENDPOINT_SELECT_KIND_ALL
};

// @types______________________________________________________________________
struct LogLevelKindInfo {
    Str8  Prefix;
    char* ColourPrefix;
};

struct LogEntryHeader {
    LogEndpointID         EndpointID;
    WallClockTime         TimeStamp;
    u8*                   FileName;
    u32                   MsgSize;
    u32                   Line;
    u16                   FileNameSize;
    LogEndpointSelectKind EndpointSelect;
    LogLevelKind          Severity;
    u8                    __Padding;
};

struct LogEntry {
    LogEntryHeader Header;
    Str8           FileName;
    Str8           Msg;
};

struct LogEndpointParams {
    Str8                      Name;
    LogEndpointFuncKind*      EndpointFunc;
    LogEndpointFlushFuncKind* FlushFunc;
    void*                     UserData;
    u64                       UserDataSize;
    LogLevelKind              MinSeverity;
};

struct LogEndpoint {
    LogEndpoint*              Next;
    LogEndpoint*              Prev;
    LogEndpointID             ID;
    Str8                      Name;
    Str8List                  PendingWrites;
    LogEndpointFuncKind*      EndpointFunc;
    LogEndpointFlushFuncKind* FlushFunc;
    void*                     UserData;
    LogLevelKind              MinSeverity;
    u64                       DroppedCount;
};

struct LogState {
    Arena*        MemPool;
    Arena*        FrameMemPool;
    SafeByteRing* SafeRing;
    Handle        Mtx;
    Handle        Thread;
    LogEndpoint*  HeadEndpoint;
    LogEndpoint*  TailEndpoint;
    u64           DroppedCount;
    u64           ActiveEntriesCount;
    LogEntry      ActiveEntries[LOG_MAX_ACTIVE_ENTRIES];
    LogEndpoint*  RegisteredEndpoints[LOG_MAX_REGISTERED_ENDPOINTS];
    u64           RegisteredEndpointsCount;
    b32           ThreadIsRunning;
    b32           ThreadShouldExit;
};

// @runtime____________________________________________________________________
extern LogLevelKindInfo LOG_LEVEL_KIND_INFO_TABLE[LOG_LEVEL_KIND_COUNT];
extern LogState* LOG_STATE;
extern LogEndpoint* const EMPTY_LOG_ENDPOINT_VALUE;

constexpr LogEndpointID EMPTY_LOG_ENDPOINT_ID_VALUE = 0;

// @functions__________________________________________________________________
void LogInit(u64 RingSize = MB(1), b32 RunAsync = TRUE);
void LogShutdown(void);
LogEndpoint* LogRegisterEndpoint(LogEndpointParams Params);
LogEndpoint* LogRegisterEndpoint(LogEndpointParams Params, char* Fmt, ...);
void LogSetLevelKindInfo(LogLevelKind Severity, Str8 Prefix, char* ColourPrefix);
void __LogBase(LogEndpointSelectKind EndpointSelect, LogEndpointID EndpointID, char const* FileName, int Line, LogLevelKind Severity, Str8 Msg);
void __LogBase(LogEndpointSelectKind EndpointSelect, LogEndpointID EndpointID, char const* FileName, int Line, LogLevelKind Severity, char* Fmt, ...);
void __LogBase(LogEndpointSelectKind EndpointSelect, LogEndpointID EndpointID, char const* FileName, int Line, LogLevelKind Severity, Str16 Msg);
void __LogBase(LogEndpointSelectKind EndpointSelect, LogEndpointID EndpointID, char const* FileName, int Line, LogLevelKind Severity, wchar* Fmt, ...);
void LogAsyncTick(void);
void LogThreadEntryPoint(void* Params);
Str8 LogFormatEntry(Arena* MemPool, LogEntry* Entry, b32 DoColour);

LOG_ENDPOINT_FUNC_DEF(Null);
LOG_ENDPOINT_FUNC_DEF(Console);
LOG_ENDPOINT_FUNC_DEF(Socket);
LOG_ENDPOINT_FUNC_DEF(File);

LOG_ENDPOINT_FLUSH_FUNC_DEF(Null);
LOG_ENDPOINT_FLUSH_FUNC_DEF(Console);
LOG_ENDPOINT_FLUSH_FUNC_DEF(File);
LOG_ENDPOINT_FLUSH_FUNC_DEF(Socket);

#endif // __NC_LOG_H__
