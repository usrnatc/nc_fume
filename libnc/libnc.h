#if !defined(__LIBNC_H__)
#define __LIBNC_H__

/*
 *      TODO:
*_____________________________________________________________________________*
 *      [ ]     TUI system:
 *               - panels
 *               - tabs
 *               - ui components
 *                  > buttons
 *                  > text input
 *                  > toggles
 *                  > watch cells
 *                  > tables
 *                  > scroll regions
 *                  > animations
 *                  > 
 *               - embedded terminal
*_____________________________________________________________________________*
 */

#include "nc_defines.h"
#include "nc_types.h"
#include "nc_system.h"
#include "nc_memory.h"

#if defined(NC_CC_GNU)
    #include "linux/linux_system.cpp"
#endif

#include "nc_tls.h"
#include "nc_math.h"
#include "nc_arena.h"
#include "nc_string.h"
#include "nc_time.h"
#include "nc_ring.h"
#include "nc_print.h"
#include "nc_log.h"
#include "nc_profile.h"
#include "nc_atomics.h"
#include "nc_cli.h"
#include "nc_file.h"
#include "nc_proc.h"
#include "nc_simd.h"
#include "nc_sync.h"
#include "nc_thread.h"
#include "nc_object_cache.h"
#include "nc_content.h"
#include "nc_file_stream.h"
#include "nc_base64.h"
#include "nc_sock.h"
#include "nc_http.h"
#include "nc_metadesk.h"

#include "nc_math.cpp"
#include "nc_memory.cpp"
#include "nc_arena.cpp"
#include "nc_string.cpp"
#include "nc_system.cpp"
#include "nc_time.cpp"
#include "nc_ring.cpp"
#include "nc_log.cpp"
#include "nc_profile.cpp"
#include "nc_cli.cpp"
#include "nc_file.cpp"
#include "nc_proc.cpp"
#include "nc_thread.cpp"
#include "nc_object_cache.cpp"
#include "nc_content.cpp"
#include "nc_file_stream.cpp"
#include "nc_base64.cpp"
#include "nc_http.cpp"
#include "nc_metadesk.cpp"

#if defined(NC_OS_WIN)
    #include "win32/win32_platform.h"
    #include "win32/win32_system.cpp"
    #include "win32/win32_memory.cpp"
    #include "win32/win32_file.cpp"
    #include "win32/win32_proc.cpp"
    #include "win32/win32_sync.cpp"
    #include "win32/win32_thread.cpp"
    #include "win32/win32_time.cpp"
    #include "win32/win32_print.cpp"
    #include "win32/win32_sock.cpp"
    #include "win32/win32_http.cpp"
    #include "win32/win32_entry.cpp"
#elif defined(NC_OS_LINUX)
    #include "linux/linux_platform.h"
    #include "linux/linux_memory.cpp"
    #include "linux/linux_time.cpp"
    #include "linux/linux_file.cpp"
    #include "linux/linux_proc.cpp"
    #include "linux/linux_sync.cpp"
    #include "linux/linux_thread.cpp"
    #include "linux/linux_print.cpp"
    #include "linux/linux_sock.cpp"
    #include "linux/linux_entry.cpp"
#elif defined(NC_OS_MAC)
    #include "macos/macos_system.cpp"
    #include "macos/macos_memory.cpp"
    #include "macos/macos_file.cpp"
    #include "macos/macos_keyboard.h"
    #include "macos/macos_net.cpp"
    #include "macos/macos_proc.cpp"
    #include "macos/macos_sync.cpp"
    #include "macos/macos_thread.cpp"
    #include "macos/macos_time.cpp"
    #include "macos/macos_window.cpp"
    #include "macos/macos_print.cpp"
    #include "macos/macos_sock.cpp"
#endif

#include "rad_lzb_simple/rad_lzb_simple.h"
#include "rad_lzb_simple/rad_lzb_simple.c"

#if defined(NC_DEBUG)
    void
    AppRegisterLogEndpoints(void)
    {
        LogEndpointParams LogConsoleParams = {
            "CONSOLE"_s8,
            LOG_ENDPOINT_FUNC_NAME(Console),
            LOG_ENDPOINT_FLUSH_FUNC_NAME(Console),
            NULL,
            0,
            LOG_LEVEL_KIND_NORMAL,
        };

        LogRegisterEndpoint(LogConsoleParams);
    }

    void
    AppUnregisterLogEndpoints(void)
    {}
#endif


void
MainThreadBaseEntryPoint(int ArgC, char** ArgV)
{
    ThreadSetName("[MAIN]"_s8);

    TempArena Scratch = GetScratch(NULL, 0);
    Str8List CLIStrings = StrListFromCLIArgs(Scratch.MemPool, ArgC, ArgV);
    CommandLine CLI = CommandLineFromStrList(Scratch.MemPool, CLIStrings);

    LogInit();

#if defined(NC_DEBUG)
    AppRegisterLogEndpoints();
#endif

    EntryPoint(&CLI);

#if defined(NC_DEBUG)
    AppUnregisterLogEndpoints();
#endif

    ReleaseScratch(Scratch);
}

// INTERNAL b32
// Update(void)
// {
//     b32 Result = FALSE;

//     AtomicIncFetchU64(&GLOBAL_UPDATE_TICK_INDEX, MEM_ORDER_SEQ_CST);

//     return Result;
// }
#else
    #error libnc.h should only be included once in any file of your choosing
#endif // __LIBNC_H__
