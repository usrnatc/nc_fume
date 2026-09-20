#if !defined(__NC_ATOMICS_H__)
#define __NC_ATOMICS_H__

#include "nc_types.h"

#if defined(NC_CC_MSVC)
    #if defined(_M_AMD64) || defined(_M_X64)
        #define MemoryBarrier __faststorefence
    #elif defined(_M_IA64)
        #define MemoryBarrier __mf
    #else
        #if defined(NC_CPU_X86_32)
            forceinline VOID
            MemoryBarrier(VOID)
            {
                __asm {
                    xchg eax, eax
                }
            }
        #else
            #error MemoryBarrier not defined
        #endif
    #endif
#elif defined(NC_CC_CLANG) || defined(NC_CC_GNU)
    #define MemoryBarrier __sync_synchronize()
#endif

typedef enum MEMORY_ORDER {
    MEM_ORDER_RELAXED,      // no ordering constraints
    MEM_ORDER_CONSUME,      // prevent re-ordering of subsequent data-dependent ops
    MEM_ORDER_ACQUIRE,      // prevent memory ops that _follow_ from being re-ordered _before_
    MEM_ORDER_RELEASE,      // prevent memory ops that _precede_ from being re-ordered _after_
    MEM_ORDER_ACQ_REL,      // combines acquire and release semantics
    MEM_ORDER_SEQ_CST       // ensures single sequential order of ops
} MEMORY_ORDER;

#if defined(NC_CC_MSVC)
    typedef struct au8 au8;
    struct ALIGNAS(sizeof(u8)) au8 {
        volatile u8 V;
    };

    typedef struct ai8 ai8;
    struct ALIGNAS(sizeof(i8)) ai8 {
        volatile i8 V;
    };

    typedef struct au16 au16;
    struct ALIGNAS(sizeof(u16)) au16 {
        volatile u16 V;
    };

    typedef struct ai16 ai16;
    struct ALIGNAS(sizeof(i16)) ai16 {
        volatile i16 V;
    };

    typedef struct au32 au32;
    struct ALIGNAS(sizeof(u32)) au32 {
        volatile u32 V;
    };

    typedef struct ai32 ai32;
    struct ALIGNAS(sizeof(i32)) ai32 {
        volatile i32 V;
    };

    typedef struct au64 au64;
    struct ALIGNAS(sizeof(u64)) au64 {
        volatile u64 V;
    };

    typedef struct ai64 ai64;
    struct ALIGNAS(sizeof(i64)) ai64 {
        volatile i64 V;
    };
#elif defined(NC_CC_GNU) || defined(NC_CC_CLANG)
    typedef struct au8 au8;
    struct au8 {
        volatile u8 V;
    } __attribute__((aligned(sizeof(u8))));

    typedef struct ai8 ai8;
    struct ai8 {
        volatile i8 V;
    } __attribute__((aligned(sizeof(i8))));

    typedef struct au16 au16;
    struct au16 {
        volatile u16 V;
    } __attribute__((aligned(sizeof(u16))));

    typedef struct ai16 ai16;
    struct ai16 {
        volatile i16 V;
    } __attribute__((aligned(sizeof(i16))));

    typedef struct au32 au32;
    struct au32 {
        volatile u32 V;
    } __attribute__((aligned(sizeof(u32))));

    typedef struct ai32 ai32;
    struct ai32 {
        volatile i32 V;
    } __attribute__((aligned(sizeof(i32))));

    typedef struct au64 au64;
    struct au64 {
        volatile u64 V;
    } __attribute__((aligned(sizeof(u64))));

    typedef struct ai64 ai64;
    struct ai64 {
        volatile i64 V;
    } __attribute__((aligned(sizeof(i64))));
#endif

#if defined(NC_CPU_X86_64)
    typedef au64 aptr;
#elif defined(NC_CPU_X86_32)
    typedef au32 aptr;
#else
    #error Atomic ptr not defined for architecture
#endif

#if defined(NC_CPU_X86)
    #if defined(NC_CC_MSVC)
        #define CPUPause() _mm_pause()
    #elif defined(NC_CC_CLANG) || defined(NC_CC_GNU)                                // FIXME: check this isnt super slow (sched_yield ?)
        #define CPUPause() asm volatile("pause\n": : :"memory")
    #endif
#else
    #define CPUPause()
#endif

#if defined(NC_CC_MSVC)
    #define AtomicThreadFence(O) do {                      \
        if ((O) >= MEM_ORDER_ACQUIRE) _ReadWriteBarrier(); \
        if ((O) == MEM_ORDER_SEQ_CST) MemoryBarrier();     \
    } while (FALSE)

    #define AtomicLoadU8(X, O)  [&]() -> u8 { AtomicThreadFence(O); return (*(volatile u8*) (X)); }()
    #define AtomicLoadU16(X, O) [&]() -> u16 { AtomicThreadFence(O); return (*(volatile u16*) (X)); }()
    #define AtomicLoadU32(X, O) [&]() -> u32 { AtomicThreadFence(O); return (*(volatile u32*) (X)); }()
    #define AtomicLoadU64(X, O) (u64) _InterlockedCompareExchange64((volatile i64*) (X), 0, 0)
    
    #define AtomicStoreU8(X, Y, O)  do { *(volatile u8*) (X) = (Y); AtomicThreadFence(O); } while (FALSE)
    #define AtomicStoreU16(X, Y, O) do { *(volatile u16*) (X) = (Y); AtomicThreadFence(O); } while (FALSE)
    #define AtomicStoreU32(X, Y, O) do { *(volatile u32*) (X) = (Y); AtomicThreadFence(O); } while (FALSE)
    #define AtomicStoreU64(X, Y, O) _InterlockedExchange64((volatile i64*) (X), (i64) (Y))
    
    #define AtomicCASU8(X, E, N, SO, FO)  (u8)  _InterlockedCompareExchange8((volatile char*) (X), (char) (N), (char) (E))
    #define AtomicCASU16(X, E, N, SO, FO) (u16) _InterlockedCompareExchange16((volatile short*) (X), (short) (N), (short) (E))
    #define AtomicCASU32(X, E, N, SO, FO) (u32) _InterlockedCompareExchange((volatile long*) (X), (long) (N), (long) (E))
    #define AtomicCASU64(X, E, N, SO, FO) (u64) _InterlockedCompareExchange64((volatile i64*) (X), (i64) (N), (i64) (E))
    
    #define AtomicExchangeU8(X, Y, O)  (u8)  _InterlockedExchange8((volatile char*) (X), (char) (Y))
    #define AtomicExchangeU16(X, Y, O) (u16) _InterlockedExchange16((volatile short*) (X), (short) (Y))
    #define AtomicExchangeU32(X, Y, O) (u32) _InterlockedExchange((volatile long*) (X), (long) (Y))
    #define AtomicExchangeU64(X, Y, O) (u64) _InterlockedExchange64((volatile i64*) (X), (i64) (Y))

    INTERNAL u8
    __ATOMIC_FETCH_OP_U8(volatile u8* X, u8 Y, char OP)
    {
        u8 OldVal = *X;

        for (;;) {
            u8 NewVal;

            if (OP == '+')
                NewVal = OldVal + Y;
            else if (OP == '-')
                NewVal = OldVal - Y;
            else if (OP == '&')
                NewVal = OldVal & Y;
            else if (OP == '|')
                NewVal = OldVal | Y;
            else
                NewVal = OldVal ^ Y;

            if ((u8) _InterlockedCompareExchange8((volatile char*) X, (char) NewVal, (char) OldVal) == OldVal)
                return OldVal;

            CPUPause();
        }
    }

    INTERNAL u16
    __ATOMIC_FETCH_OP_U16(volatile u16* X, u16 Y, char OP)
    {
        u16 OldVal = *X;

        for (;;) {
            u16 NewVal;

            if (OP == '+')
                NewVal = OldVal + Y;
            else if (OP == '-')
                NewVal = OldVal - Y;
            else if (OP == '&')
                NewVal = OldVal & Y;
            else if (OP == '|')
                NewVal = OldVal | Y;
            else
                NewVal = OldVal ^ Y;

            if ((u16) _InterlockedCompareExchange16((volatile short*) X, (short) NewVal, (short) OldVal) == OldVal)
                return OldVal;

            CPUPause();
        }
    }

    #define AtomicFetchAddU8(X, Y, O)       __ATOMIC_FETCH_OP_U8((volatile u8*) X, Y, '+')
    #define AtomicFetchSubU8(X, Y, O)       __ATOMIC_FETCH_OP_U8((volatile u8*) X, Y, '-')
    #define AtomicFetchAndU8(X, Y, O)       __ATOMIC_FETCH_OP_U8((volatile u8*) X, Y, '&')
    #define AtomicFetchOrU8(X, Y, O)        __ATOMIC_FETCH_OP_U8((volatile u8*) X, Y, '|')
    #define AtomicFetchXorU8(X, Y, O)       __ATOMIC_FETCH_OP_U8((volatile u8*) X, Y, '^')
    #define AtomicFetchAddU16(X, Y, O)      __ATOMIC_FETCH_OP_U16((volatile u16*) X, Y, '+')
    #define AtomicFetchSubU16(X, Y, O)      __ATOMIC_FETCH_OP_U16((volatile u16*) X, Y, '-')
    #define AtomicFetchAndU16(X, Y, O)      __ATOMIC_FETCH_OP_U16((volatile u16*) X, Y, '&')
    #define AtomicFetchOrU16(X, Y, O)       __ATOMIC_FETCH_OP_U16((volatile u16*) X, Y, '|')
    #define AtomicFetchXorU16(X, Y, O)      __ATOMIC_FETCH_OP_U16((volatile u16*) X, Y, '^')
    #define AtomicFetchAddU32(X, Y, O) (u32)_InterlockedExchangeAdd((volatile long*)(X), (long)(Y))
    #define AtomicFetchSubU32(X, Y, O) (u32)_InterlockedExchangeAdd((volatile long*)(X), -(long)(Y))
    #define AtomicFetchAndU32(X, Y, O) (u32)_InterlockedAnd((volatile long*)(X), (long)(Y))
    #define AtomicFetchOrU32(X, Y, O)  (u32)_InterlockedOr((volatile long*)(X), (long)(Y))
    #define AtomicFetchXorU32(X, Y, O) (u32)_InterlockedXor((volatile long*)(X), (long)(Y))
    #define AtomicFetchAddU64(X, Y, O) (u64)_InterlockedExchangeAdd64((volatile i64*)(X), (i64)(Y))
    #define AtomicFetchSubU64(X, Y, O) (u64)_InterlockedExchangeAdd64((volatile i64*)(X), -(i64)(Y))
    #define AtomicFetchAndU64(X, Y, O) (u64)_InterlockedAnd64((volatile i64*)(X), (i64)(Y))
    #define AtomicFetchOrU64(X, Y, O)  (u64)_InterlockedOr64((volatile i64*)(X), (i64)(Y))
    #define AtomicFetchXorU64(X, Y, O) (u64)_InterlockedXor64((volatile i64*)(X), (i64)(Y))
#elif defined(NC_CC_GNU) || defined(NC_CC_CLANG)
    global int __MEMORY_ORDERING_MAP[] = {
        __ATOMIC_RELAXED,
        __ATOMIC_CONSUME,
        __ATOMIC_ACQUIRE,
        __ATOMIC_RELEASE,
        __ATOMIC_ACQ_REL,
        __ATOMIC_SEQ_CST
    };

    #define AtomicThreadFence(O)              __atomic_thread_fence(__MEMORY_ORDERING_MAP[O])
    #define __ATOMIC_LOAD(T, X, O)            __atomic_load_n((volatile T*) (X), __MEMORY_ORDERING_MAP[O])
    #define __ATOMIC_STORE(T, X, Y, O)        __atomic_store_n((volatile T*) (X), (T) (Y), __MEMORY_ORDERING_MAP[O])
    #define __ATOMIC_CAS(T, X, E, N, SO, FO)  ({ T _e = (T) (E); __atomic_compare_exchange_n((volatile T*) (X), &_e, (T) (N), 0, __MEMORY_ORDERING_MAP[SO], __MEMORY_ORDERING_MAP[FO]); _e; })
    #define __ATOMIC_EXCHANGE(T, X, Y, O)     __atomic_exchange_n((volatile T*) (X), (T) (Y), __MEMORY_ORDERING_MAP[O])
    #define __ATOMIC_FETCH_OP(T, OP, X, Y, O) __atomic_fetch_##OP((volatile T*) (X), (T) (Y), __MEMORY_ORDERING_MAP[O])
    #define __ATOMIC_OP_FETCH(T, OP, X, Y, O) __atomic_##OP##_fetch((volatile T*) (X), (T) (Y), __MEMORY_ORDERING_MAP[O])

    #define AtomicLoadU8(X, O)            __ATOMIC_LOAD(u8, X, O)
    #define AtomicLoadU16(X, O)           __ATOMIC_LOAD(u16, X, O)
    #define AtomicLoadU32(X, O)           __ATOMIC_LOAD(u32, X, O)
    #define AtomicLoadU64(X, O)           __ATOMIC_LOAD(u64, X, O)

    #define AtomicStoreU8(X, Y, O)        __ATOMIC_STORE(u8, X, Y, O)
    #define AtomicStoreU16(X, Y, O)       __ATOMIC_STORE(u16, X, Y, O)
    #define AtomicStoreU32(X, Y, O)       __ATOMIC_STORE(u32, X, Y, O)
    #define AtomicStoreU64(X, Y, O)       __ATOMIC_STORE(u64, X, Y, O)

    #define AtomicCASU8(X, E, N, SO, FO)  __ATOMIC_CAS(u8, X, E, N, SO, FO)
    #define AtomicCASU16(X, E, N, SO, FO) __ATOMIC_CAS(u16, X, E, N, SO, FO)
    #define AtomicCASU32(X, E, N, SO, FO) __ATOMIC_CAS(u32, X, E, N, SO, FO)
    #define AtomicCASU64(X, E, N, SO, FO) __ATOMIC_CAS(u64, X, E, N, SO, FO)

    #define AtomicExchangeU8(X, Y, O)     __ATOMIC_EXCHANGE(u8, X, Y, O)
    #define AtomicExchangeU16(X, Y, O)    __ATOMIC_EXCHANGE(u16, X, Y, O)
    #define AtomicExchangeU32(X, Y, O)    __ATOMIC_EXCHANGE(u32, X, Y, O)
    #define AtomicExchangeU64(X, Y, O)    __ATOMIC_EXCHANGE(u64, X, Y, O)

    #define AtomicFetchAddU8(X, Y, O)     __ATOMIC_FETCH_OP(u8, add, X, Y, O)
    #define AtomicFetchAddU16(X, Y, O)    __ATOMIC_FETCH_OP(u16, add, X, Y, O)
    #define AtomicFetchAddU32(X, Y, O)    __ATOMIC_FETCH_OP(u32, add, X, Y, O)
    #define AtomicFetchAddU64(X, Y, O)    __ATOMIC_FETCH_OP(u64, add, X, Y, O)

    #define AtomicFetchSubU8(X, Y, O)     __ATOMIC_FETCH_OP(u8, sub, X, Y, O)
    #define AtomicFetchSubU16(X, Y, O)    __ATOMIC_FETCH_OP(u16, sub, X, Y, O)
    #define AtomicFetchSubU32(X, Y, O)    __ATOMIC_FETCH_OP(u32, sub, X, Y, O)
    #define AtomicFetchSubU64(X, Y, O)    __ATOMIC_FETCH_OP(u64, sub, X, Y, O)

    #define AtomicFetchAndU8(X, Y, O)     __ATOMIC_FETCH_OP(u8, and, X, Y, O)
    #define AtomicFetchAndU16(X, Y, O)    __ATOMIC_FETCH_OP(u16, and, X, Y, O)
    #define AtomicFetchAndU32(X, Y, O)    __ATOMIC_FETCH_OP(u32, and, X, Y, O)
    #define AtomicFetchAndU64(X, Y, O)    __ATOMIC_FETCH_OP(u64, and, X, Y, O)

    #define AtomicFetchOrU8(X, Y, O)      __ATOMIC_FETCH_OP(u8, or, X, Y, O)
    #define AtomicFetchOrU16(X, Y, O)     __ATOMIC_FETCH_OP(u16, or, X, Y, O)
    #define AtomicFetchOrU32(X, Y, O)     __ATOMIC_FETCH_OP(u32, or, X, Y, O)
    #define AtomicFetchOrU64(X, Y, O)     __ATOMIC_FETCH_OP(u64, or, X, Y, O)

    #define AtomicFetchXorU8(X, Y, O)     __ATOMIC_FETCH_OP(u8, xor, X, Y, O)
    #define AtomicFetchXorU16(X, Y, O)    __ATOMIC_FETCH_OP(u16, xor, X, Y, O)
    #define AtomicFetchXorU32(X, Y, O)    __ATOMIC_FETCH_OP(u32, xor, X, Y, O)
    #define AtomicFetchXorU64(X, Y, O)    __ATOMIC_FETCH_OP(u64, xor, X, Y, O)
#else
    #error Atomic intrinsics not defined for compiler
#endif

#define AtomicAddFetchU8(X, Y, O)  (AtomicFetchAddU8(X, Y, O) + (Y))
#define AtomicAddFetchU16(X, Y, O) (AtomicFetchAddU16(X, Y, O) + (Y))
#define AtomicAddFetchU32(X, Y, O) (AtomicFetchAddU32(X, Y, O) + (Y))
#define AtomicAddFetchU64(X, Y, O) (AtomicFetchAddU64(X, Y, O) + (Y))

#define AtomicSubFetchU8(X, Y, O)  (AtomicFetchSubU8(X, Y, O) - (Y))
#define AtomicSubFetchU16(X, Y, O) (AtomicFetchSubU16(X, Y, O) - (Y))
#define AtomicSubFetchU32(X, Y, O) (AtomicFetchSubU32(X, Y, O) - (Y))
#define AtomicSubFetchU64(X, Y, O) (AtomicFetchSubU64(X, Y, O) - (Y))

#define AtomicAndFetchU8(X, Y, O)  (AtomicFetchAndU8(X, Y, O) & (Y))
#define AtomicAndFetchU16(X, Y, O) (AtomicFetchAndU16(X, Y, O) & (Y))
#define AtomicAndFetchU32(X, Y, O) (AtomicFetchAndU32(X, Y, O) & (Y))
#define AtomicAndFetchU64(X, Y, O) (AtomicFetchAndU64(X, Y, O) & (Y))

#define AtomicOrFetchU8(X, Y, O)  (AtomicFetchOrU8(X, Y, O) | (Y))
#define AtomicOrFetchU16(X, Y, O) (AtomicFetchOrU16(X, Y, O) | (Y))
#define AtomicOrFetchU32(X, Y, O) (AtomicFetchOrU32(X, Y, O) | (Y))
#define AtomicOrFetchU64(X, Y, O) (AtomicFetchOrU64(X, Y, O) | (Y))

#define AtomicXorFetchU8(X, Y, O)  (AtomicFetchXorU8(X, Y, O) ^ (Y))
#define AtomicXorFetchU16(X, Y, O) (AtomicFetchXorU16(X, Y, O) ^ (Y))
#define AtomicXorFetchU32(X, Y, O) (AtomicFetchXorU32(X, Y, O) ^ (Y))
#define AtomicXorFetchU64(X, Y, O) (AtomicFetchXorU64(X, Y, O) ^ (Y))

#define AtomicIncFetchU8(X, O)  AtomicAddFetchU8(X, 1, O)
#define AtomicIncFetchU16(X, O) AtomicAddFetchU16(X, 1, O)
#define AtomicIncFetchU32(X, O) AtomicAddFetchU32(X, 1, O)
#define AtomicIncFetchU64(X, O) AtomicAddFetchU64(X, 1, O)

#define AtomicFetchIncU8(X, O)  AtomicFetchAddU8(X, 1, O)
#define AtomicFetchIncU16(X, O) AtomicFetchAddU16(X, 1, O)
#define AtomicFetchIncU32(X, O) AtomicFetchAddU32(X, 1, O)
#define AtomicFetchIncU64(X, O) AtomicFetchAddU64(X, 1, O)

#define AtomicDecFetchU8(X, O)  AtomicSubFetchU8(X, 1, O)
#define AtomicDecFetchU16(X, O) AtomicSubFetchU16(X, 1, O)
#define AtomicDecFetchU32(X, O) AtomicSubFetchU32(X, 1, O)
#define AtomicDecFetchU64(X, O) AtomicSubFetchU64(X, 1, O)

#define AtomicFetchDecU8(X, O)  AtomicFetchSubU8(X, 1, O)
#define AtomicFetchDecU16(X, O) AtomicFetchSubU16(X, 1, O)
#define AtomicFetchDecU32(X, O) AtomicFetchSubU32(X, 1, O)
#define AtomicFetchDecU64(X, O) AtomicFetchSubU64(X, 1, O)

#define AtomicLoadI64(X, O)           (i64) AtomicLoadU64((au64*) (X), O)
#define AtomicStoreI64(X, Y, O)             AtomicStoreU64((au64*) (X), (u64) (Y), O)
#define AtomicCASI64(X, E, N, SO, FO) (i64) AtomicCASU64((au64*) (X), (u64) (E), (u64) (N), SO, FO)
#define AtomicExchangeI64(X, Y, O)    (i64) AtomicExchangeU64((au64*) (X), (u64) (Y), O)
#define AtomicAddFetchI64(X, Y, O)    (i64) AtomicAddFetchU64((au64*) (X), (u64) (Y), O)
#define AtomicSubFetchI64(X, Y, O)    (i64) AtomicSubFetchU64((au64*) (X), (u64) (Y), O)
#define AtomicFetchAddI64(X, Y, O)    (i64) AtomicFetchAddU64((au64*) (X), (u64) (Y), O)
#define AtomicFetchSubI64(X, Y, O)    (i64) AtomicFetchSubU64((au64*) (X), (u64) (Y), O)

#if defined(NC_CPU_X86_64)
    #define AtomicLoadPtr(X, O)           (void*) AtomicLoadU64((au64*) (X), O)
    #define AtomicStorePtr(X, Y, O)               AtomicStoreU64((au64*) (X), (u64) (Y), O)
    #define AtomicCASPtr(X, E, N, SO, FO) (void*) AtomicCASU64((au64*) (X), (u64) (E), (u64) (N), SO, FO)
    #define AtomicExchangePtr(X, Y, O)    (void*) AtomicExchangeU64((au64*) (X), (u64) (Y), O)
#elif defined(NC_CPU_X86_32)
    #define AtomicLoadPtr(X, O)           (void*) AtomicLoadU32((au32*) (X), O)
    #define AtomicStorePtr(X, Y, O)               AtomicStoreU32((au32*) (X), (u32) (Y), O)
    #define AtomicCASPtr(X, E, N, SO, FO) (void*) AtomicCASU32((au32*) (X), (u32) (E), (u32) (N), SO, FO)
    #define AtomicExchangePtr(X, Y, O)    (void*) AtomicExchangeU32((au32*) (X), (u32) (Y), O)
#endif

#endif // __NC_ATOMICS_H__
