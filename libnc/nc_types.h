#if !defined(__NC_TYPES_H__)
#define __NC_TYPES_H__

#include "nc_defines.h"

#if !defined(TRUE)
    #define TRUE 1
#endif

#if !defined(FALSE)
    #define FALSE 0
#endif

#if !defined(NULL)
    #if defined(NC_LANG_CPP)
        #if NC_LANG_CPP_VERSION > 11 && NC_LANG_CPP_VERSION != 98
            #define NULL nullptr
        #else
            #define NULL 0
        #endif
    #else
        #define NULL ((void*) 0)
    #endif
#endif

#if defined(NC_CC_MSVC)
    #if !NC_CC_MSVC_YEAR // (_MSC_VER < 1300)
        typedef unsigned char    u8;
        typedef signed   char    i8;
        typedef unsigned short   u16;
        typedef signed   short   i16;
        typedef unsigned int     u32;
        typedef signed   int     i32;
    #else
        typedef unsigned __int8  u8;
        typedef signed   __int8  i8;
        typedef unsigned __int16 u16;
        typedef signed   __int16 i16;
        typedef unsigned __int32 u32;
        typedef signed   __int32 i32;
    #endif

    typedef     unsigned __int64 u64;
    typedef     signed   __int64 i64;
    typedef     float            f32;
    typedef     double           f64;

#elif defined(NC_CC_GNU)
    typedef     __UINT8_TYPE__   u8;
    typedef     __UINT16_TYPE__  u16;
    typedef     __UINT32_TYPE__  u32;
    typedef     __UINT64_TYPE__  u64;
    typedef     __INT8_TYPE__    i8;
    typedef     __INT16_TYPE__   i16;
    typedef     __INT32_TYPE__   i32;
    typedef     __INT64_TYPE__   i64;
    typedef     float            f32;
    typedef     double           f64;
#else
    #include <stdint.h>

    typedef     uint8_t          u8;
    typedef     uint16_t         u16;
    typedef     uint32_t         u32;
    typedef     uint64_t         u64;
    typedef     int8_t           i8;
    typedef     int16_t          i16;
    typedef     int32_t          i32;
    typedef     int64_t          i64;
    typedef     float            f32;
    typedef     double           f64;
#endif

#if defined(NC_CPU_X86_64)
    typedef     u64              sz;
    typedef     i64              dptr;
    typedef     i64              iptr;
    typedef     u64              uptr;
#else
    typedef     u32              sz;
    typedef     i32              dptr;
    typedef     i32              iptr;
    typedef     u32              uptr;
#endif

typedef     unsigned short wchar;
typedef     u16  b16;
typedef     u32  b32;
typedef     void vproc(void);

#if defined(__SIZEOF_FLOAT128__) && __SIZEOF_FLOAT128__ == 16
    #define NC_FLOAT_128
    typedef __float128 f128;
#elif defined(__SIZEOF_LONG_DOUBLE__) && __SIZEOF_LONG_DOUBLE__ == 16
    #define NC_FLOAT_128
    typedef long double f128;
#endif

STATIC_ASSERT(sizeof(u8)  == sizeof(i8),   EightBitSizeCheck);
STATIC_ASSERT(sizeof(u16) == sizeof(i16),  SixteenBitSizeCheck);
STATIC_ASSERT(sizeof(u32) == sizeof(i32),  ThirtyTwoBitSizeCheck);
STATIC_ASSERT(sizeof(u64) == sizeof(i64),  SixtyFourBitSizeCheck);
STATIC_ASSERT(sizeof(u8)  == 1,            ByteSizeCheck);
STATIC_ASSERT(sizeof(u16) == 2,            ShortSizeCheck);
STATIC_ASSERT(sizeof(u32) == 4,            LongSizeCheck);
STATIC_ASSERT(sizeof(u64) == 8,            LongLongSizeCheck);
STATIC_ASSERT(sizeof(sz)  == sizeof(dptr), PointerSizeCheck);

#if defined(NC_FLOAT_128)
    STATIC_ASSERT(sizeof(f128) == 16, F128SizeCheck);
#endif

union u128 {
    u8  U8[16];
    u16 U16[8];
    u32 U32[4];
    u64 U64[2];
};

#if defined(NC_LANG_CPP)
    internal b32
    operator==(u128 ValA, u128 ValB)
    {
        return (ValA.U64[0] == ValB.U64[0] && ValA.U64[1] == ValB.U64[1]);
    }

    internal b32
    operator!=(u128 ValA, u128 ValB)
    {
        return !(ValA == ValB);
    }

    internal u128
    operator+(u128 ValA, u128 ValB)
    {
        u128 Result = {};

        Result.U64[1] = ValA.U64[1] + ValB.U64[1];
        Result.U64[0] = (
            ValA.U64[0] + 
            ValB.U64[0] + 
            (Result.U64[1] < ValA.U64[1])
        );

        return Result;
    }

    internal u128
    operator>>(u128 Value, u32 Shift)
    {
        ASSERT_ALWAYS(Shift >= 64 && Shift < 128);

        u128 Result = {};

        Result.U64[1] = Value.U64[0] >> (Shift - 64);

        return Result;
    }
#endif

union u256 {
    u8   U8[32];
    u16  U16[16];
    u32  U32[8];
    u64  U64[4];
    u128 U128[2];
};

#if defined(NC_LANG_CPP)
    internal b32
    operator==(u256 ValA, u256 ValB)
    {
        return (ValA.U128[0] == ValB.U128[0] && ValA.U128[1] == ValB.U128[1]);
    }

    internal b32
    operator!=(u256 ValA, u256 ValB)
    {
        return !(ValA == ValB);
    }
#endif

union u512 {
    u8   U8[64];
    u16  U16[32];
    u32  U32[16];
    u64  U64[8];
    u128 U128[4];
    u256 U256[2];
};

#if defined(NC_LANG_CPP)
    internal b32
    operator==(u512 ValA, u512 ValB)
    {
        return (ValA.U256[0] == ValB.U256[0] && ValA.U256[1] == ValB.U256[1]);
    }

    internal b32
    operator!=(u512 ValA, u512 ValB)
    {
        return !(ValA == ValB);
    }
#endif

#define BYTE_SIZE       8
#define WIDE_BYTE_SIZE 16

#define NC_NAN     (0.0 / 0.0)
#define NC_INF     (1.0 / 0.0)
#define NC_NEG_INF (-1.0 / 0.0)

#define NC_PI32 3.14159265359f

#define U8_MIN 0U
#define U8_MAX 0xFFU
#define I8_MIN (-0x7F - 1)
#define I8_MAX 0x7F
#define U16_MIN 0U
#define U16_MAX 0xFFFFU
#define I16_MIN (-0x7FFF - 1)
#define I16_MAX 0x7FFF
#define U32_MIN 0U
#define U32_MAX 0xFFFFFFFFU
#define I32_MIN (-0x7FFFFFFF - 1)
#define I32_MAX 0x7FFFFFFF
#define U64_MIN 0ULL
#define U64_MAX 0xFFFFFFFFFFFFFFFFULL
#define I64_MIN (-0x7FFFFFFFFFFFFFFFLL - 1)
#define I64_MAX 0x7FFFFFFFFFFFFFFFLL

#if defined(NC_CPU_X86_32)
    #define SZ_MIN U32_MIN
    #define SZ_MAX U32_MAX
#elif defined(NC_CPU_X86_64)
    #define SZ_MIN U64_MIN
    #define SZ_MAX U64_MAX
#else
    #error Limits not defined for architecture
#endif

#define F32_MIN 1.17549435E-38f
#define F32_MAX 3.40282347E+38f
#define F64_MIN 2.2250738585072014E-308
#define F64_MAX 1.7976931348623157E+308

#if defined(NC_FLOAT_128)
    #define F128_MIN 1.18973149536E-4932
    #define F128_MAX 1.18973E+4932
#endif

#define KB(X) (((u64) (X)) << 10)
#define MB(X) (((u64) (X)) << 20)
#define GB(X) (((u64) (X)) << 30)
#define TB(X) (((u64) (X)) << 40)

#define THOUSAND(X) ((X) * 1000)
#define MILLION(X)  ((X) * 1000000)
#define BILLION(X)  ((X) * 1000000000)

#if defined(NC_CPU_X86_64)
    #define INT_FROM_PTR(Ptr) ((u64) (Ptr))
#elif defined(NC_CPU_X86_32)
    #define INT_FROM_PTR(Ptr) ((u32) (Ptr))
#endif

#define PTR_FROM_INT(Int) ((void*) (Int))

#if defined(NC_CC_MSVC)
    #include <intrin.h>

    #pragma intrinsic(_tzcnt_u32)
    #pragma intrinsic(_tzcnt_u64)
    #pragma intrinsic(_blsr_u32)
    #pragma intrinsic(_blsr_u64)

    internal force_inline u16
    SwapByteOrder(u16 Val)
    {
        return _byteswap_ushort(Val);
    }

    internal force_inline u32
    SwapByteOrder(u32 Val)
    {
        return _byteswap_ulong(Val);
    }

    internal force_inline u64
    SwapByteOrder(u64 Val)
    {
        return _byteswap_uint64(Val);
    }

    #define Ctz32(X) _tzcnt_u32(X)
    #define Ctz64(X) _tzcnt_u64(X)
    #define ResetLow32(X) _blsr_u32(X)
    #define ResetLow64(X) _blsr_u64(X)
#elif defined(NC_CC_GNU) || defined(NC_CC_CLANG)
    internal force_inline u16
    SwapByteOrder(u16 Val)
    {
        return __builtin_bswap16(Val);
    }

    internal force_inline u32
    SwapByteOrder(u32 Val)
    {
        return __builtin_bswap32(Val);
    }

    internal force_inline u64
    SwapByteOrder(u64 Val)
    {
        return __builtin_bswap64(Val);
    }

    #define Ctz32(X) __builtin_ctz(X)
    #define Ctz64(X) __builtin_ctz(X)
    #define ResetLow32(X) _blsr_u32(X)
    #define ResetLow64(X) _blsr_u64(X)
#else
    internal force_inline u16
    SwapByteOrder(u16 Val)
    {
        return ((Val >> 8) | (Val << 8));
    }

    internal force_inline u32
    SwapByteOrder(u32 Val)
    {
        return (
            (Val >> 24) |
            (Val << 24) |
            ((Val & 0x00FF0000U) >> 8) | 
            ((Val & 0x0000FF00U) << 8)
        );
    }

    internal force_inline u64
    SwapByteOrder(u64 Val)
    {
        return (
            (Val >> 56) |
            (Val << 56) |
            ((Val & 0x00FF000000000000ULL) >> 40) | 
            ((Val & 0x000000000000FF00ULL) << 40) |
            ((Val & 0x0000FF0000000000ULL) >> 24) | 
            ((Val & 0x0000000000FF0000ULL) << 24) |
            ((Val & 0x000000FF00000000ULL) >>  8) | 
            ((Val & 0x00000000FF000000ULL) <<  8)
        );
    }

    // TODO: these
    #define Ctz32(...)
    #define Ctz64(...)
#endif

#if defined(NC_CC_MSVC)
    // NOTE: see win32/win32_intrin.asm
    EXTERN_C_LINK_BEGIN
    extern u64 BitScanForward64(u64 Val);
    extern u64 BitScanReverse64(u64 Val);
    extern u64 PopCount64(u64 Val);
    EXTERN_C_LINK_END

    internal force_inline u64
    NumSetBits(u64 Val)
    {
        return PopCount64(Val);
    }
#elif defined(NC_CC_GNU) || defined(NC_CC_CLANG)
    internal force_inline u64
    NumSetBits(u64 Val)
    {
        return __builtin_popcountll(Val);
    }
#else
    internal force_inline u64
    NumSetBits(u64 Val)
    {
        u64 Result = 0;

        while (Val) {
            Result += (Val & 1);
            Val >>= 1;
        }

        return Result;
    }
#endif

INTERNAL u32
CtzBytes32(u32 Val)
{
    if (Val & 0x000000FFU)
        return 0;

    if (Val & 0x0000FF00U)
        return 1;

    if (Val & 0x00FF0000U)
        return 2;

    return 3;
}

INTERNAL u64
CtzBytes64(u64 Val)
{
    u32 Lo = (u32) Val;

    if (Lo)
        return CtzBytes32(Lo);

    return 4 + CtzBytes32((u32) (Val >> 32));
}

enum DimensionKind {
    DIM_X,
    DIM_Y,
    DIM_Z,
    DIM_W
};

typedef i32 SideKind;
enum : i32 {
    SIDE_INVALID = -1,
    SIDE_MIN,
    SIDE_MAX,
    SIDE_COUNT
};

#define FLIP_SIDE(X) ((SideKind) (!(X)))

typedef i32 Axis2D;
enum : i32 {
    AXIS_2D_INVALID = -1,
    AXIS_2D_X,
    AXIS_2D_Y,
    AXIS_2D_COUNT
};

#define FLIP_AXIS(X) ((Axis2D) (!(X)))

enum Corner {
    CORNER_INVALID = -1,
    CORNER_00 = 0,
    CORNER_01 = 1,
    CORNER_10 = 2,
    CORNER_11 = 3,
    CORNER_COUNT
};

enum Direction2D {
    DIR_2D_INVALID = -1,
    DIR_2D_LEFT,
    DIR_2D_UP,
    DIR_2D_RIGHT,
    DIR_2D_DOWN,
    DIR_2D_COUNT
};

#define Axis2DFromDir2D(X) (((X) & 1) ? AXIS_2D_Y : AXIS_2D_X)
#define SideFromDir2D(X) (((X) < DIR_2D_RIGHT) ? SIDE_MIN : SIDE_MAX)

#endif // __NC_TYPES_H__
