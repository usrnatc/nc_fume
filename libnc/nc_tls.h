#if !defined(__NC_TLS_H__)
#define __NC_TLS_H__

#include "nc_types.h"
#include "nc_arena.h"
#include "nc_math.h"

struct ThreadLocalStorage {
    u32                      ASYNC_WORK_THREAD_DEPTH;
    u64                      ASYNC_WORK_THREAD_INDEX;
    b32                      ARENA_THREAD_SCRATCH_INIT;
    b32                      IS_ASYNC_THREAD;
    struct ThreadContext*    LOCAL_THREAD_CTX;
    struct UIState*          UI_STATE;
    Arena*                   ARENA_THREAD_SCRATCH_POOL[ARENA_SCRATCH_COUNT];
};

#if defined(NC_CC_MSVC)
    EXTERN_C_LINK unsigned long _tls_index;
    EXTERN_C_LINK DLL_IMPORT void* __stdcall TlsGetValue(unsigned long);            // TODO: can access directly from TEB and bypass kernel

    INTERNAL ThreadLocalStorage*
    GetTLS(void)
    {
        return (ThreadLocalStorage*) TlsGetValue(_tls_index);
    }
#elif defined(NC_CC_CLANG) || defined(NC_CC_GNU)
global THREAD_LOCAL ThreadLocalStorage* _tls_ptr;

INTERNAL ThreadLocalStorage*
GetTLS(void)
{
    if (!_tls_ptr) {
        _tls_ptr = (ThreadLocalStorage*) MemReserve(sizeof(ThreadLocalStorage));
        MemCommit(_tls_ptr, sizeof(*_tls_ptr));
    }

    return _tls_ptr;
}
#endif

#endif // __NC_TLS_H__
