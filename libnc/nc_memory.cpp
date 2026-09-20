#include "nc_memory.h"

MemInfo MEM_INFO = {};

#if defined(NC_OS_WIN) && !defined(NC_CC_CLANG)
    #pragma intrinsic(__movsb)
#endif

API_INTERNAL void*
MemCpy(
    void* _Dst, 
    void const* _Src, 
    u64 N
) {
#if defined(NC_OS_WIN) && !defined(NC_CC_CLANG)
    if (!_Dst)
        return NULL;

    __movsb((u8*) _Dst, (u8*) _Src, N);
#elif defined(NC_CPU_X86)
    if (!_Dst)
        return NULL;

    void* Dst = _Dst;

    __asm__ __volatile__(
        "rep movsb" 
        : "+D"(Dst), "+S"(_Src), "+c"(N) : : "memory"
    );
#elif defined(NC_CPU_ARM) || defined(NC_CPU_RISCV)
    u8* Src = (u8*) _Src;
    u8* Dst = (u8*) _Dst;

    for (u64 I = 0; I < N; I++)
        *Dst++ = *Src++;
#else
    u8* Dst = (u8*) _Dst;
    u8 const* Src = (u8 const*) _Src;
    u32 W;
    u32 X;

    if (!_Dst)
        return NULL;

    for (; (uptr) Src % 4 && N; N--)
        *Dst++ = *Src++;

    if ((uptr) Dst % 4 == 0) {
        for (; N >= 16; Src += 16, Dst += 16, N -= 16) {
            *(u32*) (Dst +  0) = *(u32*) (Src +  0);
            *(u32*) (Dst +  4) = *(u32*) (Src +  4);
            *(u32*) (Dst +  8) = *(u32*) (Src +  8);
            *(u32*) (Dst + 12) = *(u32*) (Src + 12);
        }

        if (N & 8) {
            *(u32*) (Dst + 0) = *(u32*) (Src + 0);
            *(u32*) (Dst + 4) = *(u32*) (Src + 4);
            Dst += 8;
            Src += 8;
        }

        if (N & 4) {
            *(u32*) (Dst + 0) = *(u32*) (Src + 0);
            Dst += 4;
            Src += 4;
        }

        if (N & 2) {
            *Dst++ = *Src++;
            *Dst++ = *Src++;
        }

        if (N & 1)
            *Dst = *Src;

        return _Dst;
    }

    if (N >= 32) {
    #if NC_BYTE_ORDER == NC_BYTE_ORDER_BIG
        #define LS(X, Y) (X) << (Y)
        #define RS(X, Y) (X) >> (Y)
    #else
        #define LS(X, Y) (X) >> (Y)
        #define RS(X, Y) (X) << (Y)
    #endif

        switch ((uptr) Dst % 4) {
            default: {} break;

            case 1: {
                W = *(u32*) Src;
                *Dst++ = *Src++;
                *Dst++ = *Src++;
                *Dst++ = *Src++;
                N -= 3;

                while (N > 16) {
                    X = *(u32*) (Src + 1);
                    *(u32*) (Dst + 0) = (LS(W, 24)) | (RS(X, 8));
                    W = *(u32*) (Src + 5);
                    *(u32*) (Dst + 4) = (LS(X, 24)) | (RS(W, 8));
                    X = *(u32*) (Src + 9);
                    *(u32*) (Dst + 8) = (LS(W, 24)) | (RS(X, 8));
                    W = *(u32*) (Src + 13);
                    *(u32*) (Dst + 12) = (LS(X, 24)) | (RS(W, 8));
                    Src += 16;
                    Dst += 16;
                    N -= 16;
                }
            } break;

            case 2: {
                W = *(u32*) Src;
                *Dst++ = *Src++;
                *Dst++ = *Src++;
                N -= 2;

                while (N > 17) {
                    X = *(u32*) (Src + 2);
                    *(u32*) (Dst + 0) = (LS(W, 16)) | (RS(X, 16));
                    W = *(u32*) (Src + 6);
                    *(u32*) (Dst + 4) = (LS(X, 16)) | (RS(W, 16));
                    X = *(u32*) (Src + 10);
                    *(u32*) (Dst + 8) = (LS(W, 16)) | (RS(X, 16));
                    W = *(u32*) (Src + 14);
                    *(u32*) (Dst + 12) = (LS(X, 16)) | (RS(W, 16));
                    Src += 16;
                    Dst += 16;
                    N -= 16;
                }
            } break;

            case 3: {
                W = *(u32*) Src;
                *Dst++ = *Src++;
                N -= 1;

                while (N > 18) {
                    X = *(u32*) (Src + 3);
                    *(u32*) (Dst + 0) = (LS(W, 8)) | (RS(X, 24));
                    W = *(u32*) (Src + 7);
                    *(u32*) (Dst + 4) = (LS(X, 8)) | (RS(W, 24));
                    X = *(u32*) (Src + 11);
                    *(u32*) (Dst + 8) = (LS(W, 8)) | (RS(X, 24));
                    W = *(u32*) (Src + 15);
                    *(u32*) (Dst + 12) = (LS(X, 8)) | (RS(W, 24));
                    Src += 16;
                    Dst += 16;
                    N -= 16;
                }
            } break;
        }

    #undef LS
    #undef RS

        if (N & 16) {
            *Dst++ = *Src++;
            *Dst++ = *Src++;
            *Dst++ = *Src++;
            *Dst++ = *Src++;
            *Dst++ = *Src++;
            *Dst++ = *Src++;
            *Dst++ = *Src++;
            *Dst++ = *Src++;
            *Dst++ = *Src++;
            *Dst++ = *Src++;
            *Dst++ = *Src++;
            *Dst++ = *Src++;
            *Dst++ = *Src++;
            *Dst++ = *Src++;
            *Dst++ = *Src++;
            *Dst++ = *Src++;
        }

        if (N & 8) {
            *Dst++ = *Src++;
            *Dst++ = *Src++;
            *Dst++ = *Src++;
            *Dst++ = *Src++;
            *Dst++ = *Src++;
            *Dst++ = *Src++;
            *Dst++ = *Src++;
            *Dst++ = *Src++;
        }

        if (N & 4) {
            *Dst++ = *Src++;
            *Dst++ = *Src++;
            *Dst++ = *Src++;
            *Dst++ = *Src++;
        }

        if (N & 2) {
            *Dst++ = *Src++;
            *Dst++ = *Src++;
        }

        if (N & 1) {
            *Dst++ = *Src++;
        }
    }
#endif

    return _Dst;
}

API_INTERNAL void*
MemSet(
    void* _Dst, 
    u8 Value, 
    u64 N
) {
    u8* Dst = (u8*) _Dst;
    u64 K;
    u32 Value32 = ((u32) -1) / 255 * Value;

    if (!_Dst)
        return NULL;

    if (!N)
        return _Dst;

    if (!Value && N >= KB(128)) {                                               // NOTE: only use stosb for large blocks due to slow startup time
        MemZero(_Dst, N);

        return _Dst;
    }

    Dst[0] = Value;
    Dst[N - 1] = Value;

    if (N < 3)
        return _Dst;

    Dst[1] = Value;
    Dst[N - 2] = Value;
    Dst[2] = Value;
    Dst[N - 3] = Value;

    if (N < 7)
        return _Dst;

    Dst[3] = Value;
    Dst[N - 4] = Value;

    if (N < 9)
        return _Dst;

    K = -(iptr) Dst & 3;
    Dst += K;
    N -= K;
    N &= -4;
    *(u32*) (Dst + 0) = Value32;
    *(u32*) (Dst + N - 4) = Value32;

    if (N < 9)
        return _Dst;

    *(u32*) (Dst + 4) = Value32;
    *(u32*) (Dst + 8) = Value32;
    *(u32*) (Dst + N - 12) = Value32;
    *(u32*) (Dst + N - 8) = Value32;

    if (N < 25)
        return _Dst;

    *(u32*) (Dst + 12) = Value32;
    *(u32*) (Dst + 16) = Value32;
    *(u32*) (Dst + 20) = Value32;
    *(u32*) (Dst + 24) = Value32;
    *(u32*) (Dst + N - 28) = Value32;
    *(u32*) (Dst + N - 24) = Value32;
    *(u32*) (Dst + N - 20) = Value32;
    *(u32*) (Dst + N - 16) = Value32;
    K = 24 + ((uptr) Dst & 4);
    Dst += K;
    N -= K;

    {
        u64 Value64 = ((u64) Value32 << 32) | Value32;

        while (N > 31) {
            *(u64*) (Dst +  0) = Value64;
            *(u64*) (Dst +  8) = Value64;
            *(u64*) (Dst + 16) = Value64;
            *(u64*) (Dst + 24) = Value64;
            N -=32;
            Dst += 32;
        }
    }

    return _Dst;
}

API_INTERNAL b32
MemCmp(
    void const* Ptr1, 
    void const* Ptr2, 
    u64 Size
) {
    u8 const* A = (u8 const*) Ptr1;
    u8 const* B = (u8 const*) Ptr2;

    b32 Result = TRUE;

    for(u64 I = 0; I < Size; ++I) {
        if(A[I] != B[I]) {
            Result = FALSE;
            break;
        }
    }

    return Result;
}

API_INTERNAL void*
MemMove(
    void* _Dst,
    void const* _Src,
    u64 N
) {
    if (!_Dst || !N)
        return _Dst;

    u8* Dst = (u8*)_Dst;
    u8 const* Src = (u8 const*)_Src;

    if (Dst <= Src || Dst >= Src + N)
        return MemCpy(_Dst, _Src, N);

    Dst += N - 1;
    Src += N - 1;

#if defined(NC_CPU_X86) && !defined(NC_OS_WIN)
    // NOTE(nathan): reverse direction in case regions overlap
    //             : may incur performance cost as CPU cannot prefetch
    __asm__ __volatile__(
        "std\n\t"
        "rep movsb\n\t"
        "cld"
        : "+D"(Dst), "+S"(Src), "+c"(N)
        : 
        : "memory"
    );
#else
    if (N >= 32) {
        while (N && ((uptr) Dst % 4)) {
            *--Dst = *--Src;
            N--;
        }

        while (N >= 16) {
            Dst -= 16;
            Src -= 16;
            
            u32* Dst32 = (u32*)Dst;
            u32 const* Src32 = (u32 const*)Src;

            Dst32[3] = Src32[3];
            Dst32[2] = Src32[2];
            Dst32[1] = Src32[1];
            Dst32[0] = Src32[0];
            N -= 16;
        }
        
        while (N >= 4) {
            Dst -= 4;
            Src -= 4;
            *(u32*) Dst = *(u32*) Src;
            N -= 4;
        }
    }

    while (N--)
        *--Dst = *--Src;
#endif

    return _Dst;
}

API_INTERNAL b32
MemIsZero(void* _Ptr, u64 Size)
{
    b32 Result = TRUE;
    u64 Extra = (Size & 0x7);
    u64 ByteCount = (Size >> 3);
    u64* Ptr64 = (u64*) _Ptr;

    if (Result) {
        for (
            u64 Index = 0; 
            Index < ByteCount; 
            ++Index, ++Ptr64
        ) {
            if (*Ptr64) {
                Result = FALSE;
                goto DONE;
            }
        }
    }

    if (Result) {
        u8* Ptr8 = (u8*) Ptr64;

        for (u64 Index = 0; Index < Extra; ++Index, ++Ptr8) {
            if (*Ptr8) {
                Result = FALSE;
                goto DONE;
            }
        }
    }

DONE:
    return Result;
}
