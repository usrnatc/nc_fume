#if !defined(__NC_SIMD_H__)
#define __NC_SIMD_H__

// TODO(nathan): Add these:
//             : Sqrt
//             : Cbrt
//             : FMod
//             : Pow
//             : Ceil
//             : Floor
//             : Round
//             : Sin
//             : Cos
//             : Tan
//             : ArcTan
//             : ArcTan2
//             : Lerp
//             : Dot
//             : LengthSq
//             : Length
//             : Norm
//             : Cross
//             : Translation
//             : Scaler
//             : Perspective
//             : Orthographic
//             : LookAt
//             : Rotation
//             : Inverse
//             : DeRotate
//             : Transpose
//             : Shift
//             : Widen
//             : Centre
//             : InRange
//             : Union
//             : Intersect
//             : Clamp
//             : HSVFromRGB
//             : RGBFromHSV
//             : HSVAFromRGBA
//             : RGBAFromHSVA
//             : LinearFromSRGB
//             : LinearFromSRGBA

// TODO(nathan): Add some string utilities

// TODO(nathan): Some of these operations could do with manual ASM 
//             : optimisations as they may not produce optimal code in DEBUG
//             : nor in RELEASE

// TODO(nathan): Try to cut down on use of division operations as they are
//             : very expensive

// FIXME(nathan): Comparisons are returning their base type, this will be
//              : problematic for code that is to be cross-arch as we will
//              : still need to conditionally compile any code that uses them

#include "nc_types.h"

#if defined(NC_CC_MSVC)
    #include <intrin.h>

    #define NC_CPU_PREFETCH(X) _mm_prefetch((char const*) (X), _MM_HINT_T0)
#elif defined(NC_CC_GNU) || defined(NC_CC_CLANG)
    #include <x86intrin.h>

    #if IS_BUILTIN(prefetch)
        #define NC_CPU_PREFETCH(X) __builtin_prefetch((X), 0, 3)
    #else
        #define NC_CPU_PREFETCH(X)
    #endif
#endif

#if defined(NC_CPU_AVX512F) ||  defined(NC_CPU_AVX2)
    #include <immintrin.h>
#endif

#if defined(NC_CPU_SSE4_2)
    #include <nmmintrin.h>
#endif

#if defined(NC_CPU_SSE)
    #include <xmmintrin.h>
#endif

#if defined(NC_SPU_NEON)
    #include <arm_neon.h>
#endif

// NOTE(nathan): for some unknown reason, GCC does not have _mm*_and_epi32 so
//             : these have been added, the same implementation as clang
#if NC_SIMD_LEVEL >= 2
    typedef __m128i v128;
    typedef __m128  v128f;
    typedef __m128d v128d;

    #if defined(NC_CC_GNU) && !defined(NC_CC_CLANG)
        INTERNAL __attribute__((gnu_inline)) v128
        _mm_and_epi32(v128 ValA, v128 ValB)
        {
            return (v128) ((__v4su) ValA & (__v4su) ValB);
        }
    #endif
#endif

#if NC_SIMD_LEVEL >= 6
    typedef __m256i v256;
    typedef __m256  v256f;
    typedef __m256d v256d;

    #if defined(NC_CC_GNU) && !defined(NC_CC_CLANG)
        INTERNAL __attribute__((gnu_inline)) v256
        _mm256_and_epi32(v256 ValA, v256 ValB)
        {
            return (v256) ((__v8su) ValA & (__v8su) ValB);
        }
    #endif
#endif

#if NC_SIMD_LEVEL >= 8
    typedef __m512i v512;
    typedef __m512  v512f;
    typedef __m512d v512d;
#endif

#if !NC_SIMD_LEVEL >= 8
    union zv2f32 { 
        struct { 
            v512f X;
            v512f Y; 
        };

        v512f V[2];
    };

    union zv2i32 {
        struct {
            v512 X;
            v512 Y; 
        };

        v512 V[2];
    };

    union zv2f64 {
        struct {
            v512d X;
            v512d Y; 
        };

        v512d V[2];
    };

    union zv2i16 {
        struct {
            v512 X;
            v512 Y; 
        };

        v512  V[2];
    };

    union zv3f32 {
        struct {
            v512f X;
            v512f Y;
            v512f Z;
        };

        v512f V[3];
    };

    union zv3i32 {
        struct {
            v512 X;
            v512 Y;
            v512 Z;
        };

        v512  V[3];
    };

    union zv3f64 {
        struct {
            v512d X;
            v512d Y;
            v512d Z;
        };

        v512d V[3];
    };

    union zv4f32 {
        struct {
            v512f X;
            v512f Y;
            v512f Z;
            v512f W;
        };

        v512f V[4];
    };

    union zv4i32 {
        struct {
            v512 X;
            v512 Y;
            v512 Z;
            v512 W;
        };

        v512  V[4];
    };

    union zv4f64 {
        struct {
            v512d X;
            v512d Y; 
            v512d Z;
            v512d W;
        };

        v512d V[4];
    };

    union zr1i32 {
        struct {
            v512 Min;
            v512 Max;
        };

        v512  V[2];
    };

    union zr1f32 {
        struct {
            v512f Min;
            v512f Max;
        };

        v512f V[2];
    };

    union zr1f64 {
        struct {
            v512d Min;
            v512d Max;
        };

        v512d V[2];
    };

    union zr2i32 {
        struct {
            zv2i32 Min;
            zv2i32 Max;
        };

        struct {
            zv2i32 Point0;
            zv2i32 Point1;
        };

        zv2i32 V[2];
    };

    union zr2f32 {
        struct {
            zv2f32 Min;
            zv2f32 Max;
        };

        struct {
            zv2f32 Point0;
            zv2f32 Point1;
        };

        zv2f32 V[2];
    };

    union zr2f64 {
        struct {
            zv2f64 Min;
            zv2f64 Max;
        };

        struct {
            zv2f64 Point0;
            zv2f64 Point1;
        };

        zv2f64 V[2];
    };


    union zr3f32 {
        struct {
            zv3f32 Min;
            zv3f32 Max;
        };

        struct {
            zv3f32 Point0;
            zv3f32 Point1;
        };

        zv3f32 V[2];
    };

    union zr3i32 {
        struct {
            zv3i32 Min;
            zv3i32 Max;
        };

        struct {
            zv3i32 Point0;
            zv3i32 Point1;
        };

        zv3i32 V[2];
    };

    union zr3f64 {
        struct {
            zv3f64 Min;
            zv3f64 Max;
        };

        struct {
            zv3f64 Point0;
            zv3f64 Point1;
        };

        zv3f64 V[2];
    };

    INTERNAL zv2f32 
    operator+(zv2f32 VecA, zv2f32 VecB) 
    {
        zv2f32 Result = {};

        Result.X = _mm512_add_ps(VecA.X, VecB.X);
        Result.Y = _mm512_add_ps(VecA.Y, VecB.Y);

        return Result;
    }

    INTERNAL zv2f32 
    operator-(zv2f32 VecA, zv2f32 VecB) 
    {
        zv2f32 Result = {};

        Result.X = _mm512_sub_ps(VecA.X, VecB.X);
        Result.Y = _mm512_sub_ps(VecA.Y, VecB.Y);

        return Result;
    }

    INTERNAL zv2f32 
    operator*(zv2f32 VecA, zv2f32 VecB) 
    {
        zv2f32 Result = {};

        Result.X = _mm512_mul_ps(VecA.X, VecB.X);
        Result.Y = _mm512_mul_ps(VecA.Y, VecB.Y);

        return Result;
    }

    INTERNAL zv2f32 
    operator*(zv2f32 Vec, v512f Value) 
    {
        zv2f32 Result = {};

        Result.X = _mm512_mul_ps(Vec.X, Value);
        Result.Y = _mm512_mul_ps(Vec.Y, Value);

        return Result;
    }

    INTERNAL zv2f32 
    operator*(zv2f32 Vec, f32 Value) 
    {
        zv2f32 Result = {};
        v512f Scale = _mm512_set1_ps(Value);

        Result.X = _mm512_mul_ps(Vec.X, Scale);
        Result.Y = _mm512_mul_ps(Vec.Y, Scale);

        return Result;
    }

    INTERNAL zv2f32 
    operator/(zv2f32 VecA, zv2f32 VecB) 
    {
        zv2f32 Result = {};

        Result.X = _mm512_div_ps(VecA.X, VecB.X);
        Result.Y = _mm512_div_ps(VecA.Y, VecB.Y);

        return Result;
    }

    INTERNAL zv2f32 
    operator/(zv2f32 Vec, v512f Value) 
    {
        zv2f32 Result = {};

        Result.X = _mm512_div_ps(Vec.X, Value);
        Result.Y = _mm512_div_ps(Vec.Y, Value);

        return Result;
    }

    INTERNAL zv2f32 
    operator/(zv2f32 Vec, f32 Value) 
    {
        v512f ZValue = _mm512_set1_ps(Value);

        return Vec / ZValue;
    }

    INTERNAL zv2f32 
    operator&(zv2f32 VecA, zv2f32 VecB) 
    {
        zv2f32 Result = {};

        Result.X = _mm512_and_ps(VecA.X, VecB.X);
        Result.Y = _mm512_and_ps(VecA.Y, VecB.Y);

        return Result;
    }

    INTERNAL zv2f32 
    operator|(zv2f32 VecA, zv2f32 VecB) 
    {
        zv2f32 Result = {};

        Result.X = _mm512_or_ps(VecA.X, VecB.X);
        Result.Y = _mm512_or_ps(VecA.Y, VecB.Y);

        return Result;
    }

    INTERNAL zv2f32 
    operator^(zv2f32 VecA, zv2f32 VecB) 
    {
        zv2f32 Result = {};

        Result.X = _mm512_xor_ps(VecA.X, VecB.X);
        Result.Y = _mm512_xor_ps(VecA.Y, VecB.Y);

        return Result;
    }

    INTERNAL v512f
    operator==(zv2f32 VecA, zv2f32 VecB) 
    {
        __mmask16 MaskX = _mm512_cmp_ps_mask(VecA.X, VecB.X, _CMP_EQ_OQ);
        __mmask16 MaskY = _mm512_cmp_ps_mask(VecA.Y, VecB.Y, _CMP_EQ_OQ);
        __mmask16 ResultMask = _mm512_kand(MaskX, MaskY);
        v512f Result = _mm512_mask_mov_ps(
            _mm512_setzero_ps(),
            ResultMask,
            _mm512_set1_ps(1.0f)
        );

        return Result;
    }

    INTERNAL v512f 
    operator!=(zv2f32 VecA, zv2f32 VecB) 
    {
        __mmask16 MaskX = _mm512_cmp_ps_mask(VecA.X, VecB.X, _CMP_NEQ_OQ);
        __mmask16 MaskY = _mm512_cmp_ps_mask(VecA.Y, VecB.Y, _CMP_NEQ_OQ);
        __mmask16 ResultMask = _mm512_kand(MaskX, MaskY);
        v512f Result = _mm512_mask_mov_ps(
            _mm512_setzero_ps(),
            ResultMask,
            _mm512_set1_ps(1.0f)
        );

        return Result;
    }

    INTERNAL v512f
    operator<(zv2f32 VecA, zv2f32 VecB) 
    {
        __mmask16 MaskX = _mm512_cmp_ps_mask(VecA.X, VecB.X, _CMP_LT_OQ);
        __mmask16 MaskY = _mm512_cmp_ps_mask(VecA.Y, VecB.Y, _CMP_LT_OQ);
        __mmask16 ResultMask = _mm512_kand(MaskX, MaskY);
        v512f Result = _mm512_mask_mov_ps(
            _mm512_setzero_ps(),
            ResultMask,
            _mm512_set1_ps(1.0f)
        );

        return Result;
    }

    INTERNAL v512f
    operator>(zv2f32 VecA, zv2f32 VecB) 
    {
        __mmask16 MaskX = _mm512_cmp_ps_mask(VecA.X, VecB.X, _CMP_GT_OQ);
        __mmask16 MaskY = _mm512_cmp_ps_mask(VecA.Y, VecB.Y, _CMP_GT_OQ);
        __mmask16 ResultMask = _mm512_kand(MaskX, MaskY);
        v512f Result = _mm512_mask_mov_ps(
            _mm512_setzero_ps(),
            ResultMask,
            _mm512_set1_ps(1.0f)
        );

        return Result;
    }

    INTERNAL v512f
    operator<=(zv2f32 VecA, zv2f32 VecB) 
    {
        __mmask16 MaskX = _mm512_cmp_ps_mask(VecA.X, VecB.X, _CMP_LE_OQ);
        __mmask16 MaskY = _mm512_cmp_ps_mask(VecA.Y, VecB.Y, _CMP_LE_OQ);
        __mmask16 ResultMask = _mm512_kand(MaskX, MaskY);
        v512f Result = _mm512_mask_mov_ps(
            _mm512_setzero_ps(),
            ResultMask,
            _mm512_set1_ps(1.0f)
        );

        return Result;
    }

    INTERNAL v512f
    operator>=(zv2f32 VecA, zv2f32 VecB) 
    {
        __mmask16 MaskX = _mm512_cmp_ps_mask(VecA.X, VecB.X, _CMP_GE_OQ);
        __mmask16 MaskY = _mm512_cmp_ps_mask(VecA.Y, VecB.Y, _CMP_GE_OQ);
        __mmask16 ResultMask = _mm512_kand(MaskX, MaskY);
        v512f Result = _mm512_mask_mov_ps(
            _mm512_set1_ps(0.0f),
            ResultMask,
            _mm512_set1_ps(1.0f)
        );

        return Result;
    }

    INTERNAL zv2f32 
    operator%(zv2f32 VecA, zv2f32 VecB) 
    {
        zv2f32 Result = {};
        v512f DivX = _mm512_div_ps(VecA.X, VecB.X);
        v512f DivY = _mm512_div_ps(VecA.Y, VecB.Y);
        v512f FloorX = _mm512_roundscale_ps(DivX, _MM_FROUND_FLOOR);                // TODO: compare against _mm512_floor_ps and 
        v512f FloorY = _mm512_roundscale_ps(DivY, _MM_FROUND_FLOOR);                //     : _mm512_roundscale_round_ps

        Result.X = _mm512_fnmadd_ps(FloorX, VecB.X, VecA.X);
        Result.Y = _mm512_fnmadd_ps(FloorY, VecB.Y, VecA.Y);

        return Result;
    }

    INTERNAL v512f
    Dot(zv2f32 VecA, zv2f32 VecB)
    {
        v512f Result = {};

        Result = _mm512_fmadd_ps(
            VecA.X,
            VecB.X,
            _mm512_mul_ps(VecA.Y, VecB.Y)
        );

        return Result;
    }

    INTERNAL v512f
    LengthSq(zv2f32 Vec)
    {
        return Dot(Vec, Vec);
    }

    INTERNAL v512f
    Length(zv2f32 Vec)
    {
        return _mm512_sqrt_ps(LengthSq(Vec));
    }

    INTERNAL zv2f32
    Norm(zv2f32 Vec)
    {
        v512f Scale = _mm512_rsqrt14_ps(LengthSq(Vec));

        return Vec * Scale;
    }

    INTERNAL zv2f32
    Lerp(zv2f32 VecA, zv2f32 VecB, v512f T)
    {
        zv2f32 Result = {};

        Result.X = _mm512_fmadd_ps(
            T,
            VecB.X,
            _mm512_fnmadd_ps(T, VecA.X, VecA.X)
        );
        Result.Y = _mm512_fmadd_ps(
            T,
            VecB.Y,
            _mm512_fnmadd_ps(T, VecA.Y, VecA.Y)
        );

        return Result;
    }

    INTERNAL zv2f32
    Lerp(zv2f32 VecA, zv2f32 VecB, f32 T)
    {
        v512f ZT = _mm512_set1_ps(T);

        return Lerp(VecA, VecB, ZT);
    }

    INTERNAL zv2i32 
    operator+(zv2i32 VecA, zv2i32 VecB) 
    {
        zv2i32 Result = {};

        Result.X = _mm512_add_epi32(VecA.X, VecB.X);
        Result.Y = _mm512_add_epi32(VecA.Y, VecB.Y);

        return Result;
    }

    INTERNAL zv2i32 
    operator-(zv2i32 VecA, zv2i32 VecB) 
    {
        zv2i32 Result = {};

        Result.X = _mm512_sub_epi32(VecA.X, VecB.X);
        Result.Y = _mm512_sub_epi32(VecA.Y, VecB.Y);

        return Result;
    }

    INTERNAL zv2i32 
    operator*(zv2i32 VecA, zv2i32 VecB) 
    {
        zv2i32 Result = {};

        Result.X = _mm512_mullo_epi32(VecA.X, VecB.X);
        Result.Y = _mm512_mullo_epi32(VecA.Y, VecB.Y);

        return Result;
    }

    INTERNAL zv2i32
    operator*(zv2i32 Vec, v512 Value)
    {
        zv2i32 Result = {};

        Result.X = _mm512_mullo_epi32(Vec.X, Value);
        Result.Y = _mm512_mullo_epi32(Vec.Y, Value);

        return Result;
    }

    INTERNAL zv2i32
    operator*(zv2i32 Vec, i32 Value)
    {
        v512 ZValue = _mm512_set1_epi32(Value);

        return Vec * ZValue;
    }

    INTERNAL zv2i32 
    operator/(zv2i32 VecA, zv2i32 VecB)
    {
        zv2i32 Result = {};
        v512f FloatAX = _mm512_cvtepi32_ps(VecA.X);
        v512f FloatAY = _mm512_cvtepi32_ps(VecA.Y);
        v512f FloatBX = _mm512_cvtepi32_ps(VecB.X);
        v512f FloatBY = _mm512_cvtepi32_ps(VecB.Y);
        v512f QuotientX = _mm512_div_ps(FloatAX, FloatBX);
        v512f QuotientY = _mm512_div_ps(FloatAY, FloatBY);

        Result.X = _mm512_cvtps_epi32(QuotientX);
        Result.Y = _mm512_cvtps_epi32(QuotientY);

        return Result;
    }

    INTERNAL zv2i32 
    operator&(zv2i32 VecA, zv2i32 VecB) 
    {
        zv2i32 Result = {};

        Result.X = _mm512_and_epi32(VecA.X, VecB.X);
        Result.Y = _mm512_and_epi32(VecA.Y, VecB.Y);

        return Result;
    }

    INTERNAL zv2i32 
    operator|(zv2i32 VecA, zv2i32 VecB) 
    {
        zv2i32 Result = {};

        Result.X = _mm512_or_epi32(VecA.X, VecB.X);
        Result.Y = _mm512_or_epi32(VecA.Y, VecB.Y);

        return Result;
    }

    INTERNAL zv2i32 
    operator^(zv2i32 VecA, zv2i32 VecB) 
    {
        zv2i32 Result = {};

        Result.X = _mm512_xor_epi32(VecA.X, VecB.X);
        Result.Y = _mm512_xor_epi32(VecA.Y, VecB.Y);

        return Result;
    }

    INTERNAL zv2i32 
    operator~(zv2i32 VecA)
    {
        zv2i32 Result = {};

        Result.X = _mm512_ternarylogic_epi32(VecA.X, VecA.X, VecA.X, 0x55);
        Result.Y = _mm512_ternarylogic_epi32(VecA.Y, VecA.Y, VecA.Y, 0x55);

        return Result;
    }

    INTERNAL v512
    operator==(zv2i32 VecA, zv2i32 VecB) 
    {
        __mmask16 MaskX = _mm512_cmp_epi32_mask(VecA.X, VecB.X, _MM_CMPINT_EQ);
        __mmask16 MaskY = _mm512_cmp_epi32_mask(VecA.Y, VecB.Y, _MM_CMPINT_EQ);
        __mmask16 ResultMask = _mm512_kand(MaskX, MaskY);
        v512 Result = _mm512_mask_mov_epi32(
            _mm512_setzero_epi32(),
            ResultMask,
            _mm512_set1_epi32(~0)
        );

        return Result;
    }

    INTERNAL v512
    operator!=(zv2i32 VecA, zv2i32 VecB) 
    {
        __mmask16 MaskX = _mm512_cmp_epi32_mask(VecA.X, VecB.X, _MM_CMPINT_NE);
        __mmask16 MaskY = _mm512_cmp_epi32_mask(VecA.Y, VecB.Y, _MM_CMPINT_NE);
        __mmask16 ResultMask = _mm512_kand(MaskX, MaskY);
        v512 Result = _mm512_mask_mov_epi32(
            _mm512_setzero_epi32(),
            ResultMask,
            _mm512_set1_epi32(~0)
        );

        return Result;
    }

    INTERNAL v512
    operator<(zv2i32 VecA, zv2i32 VecB) 
    {
        __mmask16 MaskX = _mm512_cmp_epi32_mask(VecA.X, VecB.X, _MM_CMPINT_LT);
        __mmask16 MaskY = _mm512_cmp_epi32_mask(VecA.Y, VecB.Y, _MM_CMPINT_LT);
        __mmask16 ResultMask = _mm512_kand(MaskX, MaskY);
        v512 Result = _mm512_mask_mov_epi32(
            _mm512_setzero_epi32(),
            ResultMask,
            _mm512_set1_epi32(~0)
        );

        return Result;
    }

    INTERNAL v512
    operator>(zv2i32 VecA, zv2i32 VecB) 
    {
        __mmask16 MaskX = _mm512_cmp_epi32_mask(VecA.X, VecB.X, _MM_CMPINT_GT);
        __mmask16 MaskY = _mm512_cmp_epi32_mask(VecA.Y, VecB.Y, _MM_CMPINT_GT);
        __mmask16 ResultMask = _mm512_kand(MaskX, MaskY);
        v512 Result = _mm512_mask_mov_epi32(
            _mm512_setzero_epi32(),
            ResultMask,
            _mm512_set1_epi32(~0)
        );

        return Result;
    }

    INTERNAL v512
    operator<=(zv2i32 VecA, zv2i32 VecB) 
    {
        __mmask16 MaskX = _mm512_cmp_epi32_mask(VecA.X, VecB.X, _MM_CMPINT_LE);
        __mmask16 MaskY = _mm512_cmp_epi32_mask(VecA.Y, VecB.Y, _MM_CMPINT_LE);
        __mmask16 ResultMask = _mm512_kand(MaskX, MaskY);
        v512 Result = _mm512_mask_mov_epi32(
            _mm512_setzero_epi32(),
            ResultMask,
            _mm512_set1_epi32(~0)
        );

        return Result;
    }

    INTERNAL v512
    operator>=(zv2i32 VecA, zv2i32 VecB) 
    {
        __mmask16 MaskX = _mm512_cmp_epi32_mask(VecA.X, VecB.X, _MM_CMPINT_GE);
        __mmask16 MaskY = _mm512_cmp_epi32_mask(VecA.Y, VecB.Y, _MM_CMPINT_GE);
        __mmask16 ResultMask = _mm512_kand(MaskX, MaskY);
        v512 Result = _mm512_mask_mov_epi32(
            _mm512_setzero_epi32(),
            ResultMask,
            _mm512_set1_epi32(~0)
        );

        return Result;
    }

    INTERNAL zv2i32 
    operator<<(zv2i32 VecA, i32 Shift)
    {
        zv2i32 Result = {};

        Result.X = _mm512_slli_epi32(VecA.X, Shift);
        Result.Y = _mm512_slli_epi32(VecA.Y, Shift);

        return Result;
    }

    INTERNAL zv2i32 
    operator>>(zv2i32 VecA, i32 Shift)
    {
        zv2i32 Result = {};

        Result.X = _mm512_srli_epi32(VecA.X, Shift);
        Result.Y = _mm512_srli_epi32(VecA.Y, Shift);

        return Result;
    }

    INTERNAL v512
    Dot(zv2i32 VecA, zv2i32 VecB)
    {
        v512 Result = {};

        Result = _mm512_add_epi32(
            _mm512_mul_epi32(VecA.X, VecB.X),
            _mm512_mul_epi32(VecA.Y, VecB.Y)
        );

        return Result;
    }

    INTERNAL v512
    LengthSq(zv2i32 Vec)
    {
        return Dot(Vec, Vec);
    }

    INTERNAL v512
    Length(zv2i32 Vec)
    {
        v512f LenSq = _mm512_cvtepi32_ps(LengthSq(Vec));
        
        return _mm512_cvtps_epi32(
            _mm512_sqrt_ps(LenSq)
        );
    }

    INTERNAL zv2i32
    Norm(zv2i32 Vec)
    {
        v512 Scale = _mm512_cvtps_epi32(
            _mm512_rsqrt14_ps(
                _mm512_cvtepi32_ps(
                    LengthSq(Vec)
                )
            )
        );

        return Vec * Scale;
    }

    INTERNAL zv2i32
    Lerp(zv2i32 VecA, zv2i32 VecB, v512 T)
    {
        zv2i32 Result = {};
        v512f AX = _mm512_cvtepi32_ps(VecA.X);
        v512f AY = _mm512_cvtepi32_ps(VecA.Y);
        v512f BX = _mm512_cvtepi32_ps(VecB.X);
        v512f BY = _mm512_cvtepi32_ps(VecB.Y);
        v512f ZT = _mm512_cvtepi32_ps(T);
        v512f ResultX = _mm512_fmadd_ps(
            ZT,
            BX,
            _mm512_fnmadd_ps(ZT, AX, AX)
        );
        v512f ResultY = _mm512_fmadd_ps(
            ZT,
            BY,
            _mm512_fnmadd_ps(ZT, AY, AY)
        );

        Result.X = _mm512_cvtps_epi32(ResultX);
        Result.Y = _mm512_cvtps_epi32(ResultY);

        return Result;
    }

    INTERNAL zv2i32
    Lerp(zv2i32 VecA, zv2i32 VecB, i32 T)
    {
        v512 ZT = _mm512_set1_epi32(T);

        return Lerp(VecA, VecB, ZT);
    }

    INTERNAL zv2f64 
    operator+(zv2f64 VecA, zv2f64 VecB) 
    {
        zv2f64 Result = {};

        Result.X = _mm512_add_pd(VecA.X, VecB.X);
        Result.Y = _mm512_add_pd(VecA.Y, VecB.Y);

        return Result;
    }

    INTERNAL zv2f64 
    operator-(zv2f64 VecA, zv2f64 VecB) 
    {
        zv2f64 Result = {};

        Result.X = _mm512_sub_pd(VecA.X, VecB.X);
        Result.Y = _mm512_sub_pd(VecA.Y, VecB.Y);

        return Result;
    }

    INTERNAL zv2f64 
    operator*(zv2f64 VecA, zv2f64 VecB) 
    {
        zv2f64 Result = {};

        Result.X = _mm512_mul_pd(VecA.X, VecB.X);
        Result.Y = _mm512_mul_pd(VecA.Y, VecB.Y);

        return Result;
    }

    INTERNAL zv2f64 
    operator*(zv2f64 Vec, v512d Value) 
    {
        zv2f64 Result = {};

        Result.X = _mm512_mul_pd(Vec.X, Value);
        Result.Y = _mm512_mul_pd(Vec.Y, Value);

        return Result;
    }

    INTERNAL zv2f64 
    operator*(zv2f64 Vec, f64 Value) 
    {
        v512d ZValue = _mm512_set1_pd(Value);

        return Vec * ZValue;
    }

    INTERNAL zv2f64 
    operator/(zv2f64 VecA, zv2f64 VecB) 
    {
        zv2f64 Result = {};

        Result.X = _mm512_div_pd(VecA.X, VecB.X);
        Result.Y = _mm512_div_pd(VecA.Y, VecB.Y);

        return Result;
    }

    INTERNAL zv2f64 
    operator/(zv2f64 Vec, v512d Value) 
    {
        zv2f64 Result = {};

        Result.X = _mm512_div_pd(Vec.X, Value);
        Result.Y = _mm512_div_pd(Vec.Y, Value);

        return Result;
    }

    INTERNAL zv2f64 
    operator/(zv2f64 Vec, f64 Value) 
    {
        v512d ZValue = _mm512_set1_pd(Value);

        return Vec / ZValue;
    }

    INTERNAL zv2f64 
    operator&(zv2f64 VecA, zv2f64 VecB) 
    {
        zv2f64 Result = {};

        Result.X = _mm512_and_pd(VecA.X, VecB.X);
        Result.Y = _mm512_and_pd(VecA.Y, VecB.Y);

        return Result;
    }

    INTERNAL zv2f64 
    operator|(zv2f64 VecA, zv2f64 VecB) 
    {
        zv2f64 Result = {};

        Result.X = _mm512_or_pd(VecA.X, VecB.X);
        Result.Y = _mm512_or_pd(VecA.Y, VecB.Y);

        return Result;
    }

    INTERNAL zv2f64 
    operator^(zv2f64 VecA, zv2f64 VecB) 
    {
        zv2f64 Result = {};

        Result.X = _mm512_xor_pd(VecA.X, VecB.X);
        Result.Y = _mm512_xor_pd(VecA.Y, VecB.Y);

        return Result;
    }

    INTERNAL v512d
    operator==(zv2f64 VecA, zv2f64 VecB) 
    {
        __mmask8 MaskX = _mm512_cmp_pd_mask(VecA.X, VecB.X, _CMP_EQ_OQ);
        __mmask8 MaskY = _mm512_cmp_pd_mask(VecA.Y, VecB.Y, _CMP_EQ_OQ);
        __mmask8 ResultMask = _kand_mask8(MaskX, MaskY);
        v512d Result = _mm512_mask_mov_pd(
            _mm512_setzero_pd(),
            ResultMask,
            _mm512_set1_pd(1.0)
        );

        return Result;
    }

    INTERNAL v512d
    operator!=(zv2f64 VecA, zv2f64 VecB) 
    {
        __mmask8 MaskX = _mm512_cmp_pd_mask(VecA.X, VecB.X, _CMP_NEQ_OQ);
        __mmask8 MaskY = _mm512_cmp_pd_mask(VecA.Y, VecB.Y, _CMP_NEQ_OQ);
        __mmask8 ResultMask = _kand_mask8(MaskX, MaskY);
        v512d Result = _mm512_mask_mov_pd(
            _mm512_setzero_pd(),
            ResultMask,
            _mm512_set1_pd(1.0)
        );

        return Result;
    }

    INTERNAL v512d
    operator<(zv2f64 VecA, zv2f64 VecB) 
    {
        __mmask8 MaskX = _mm512_cmp_pd_mask(VecA.X, VecB.X, _CMP_LT_OQ);
        __mmask8 MaskY = _mm512_cmp_pd_mask(VecA.Y, VecB.Y, _CMP_LT_OQ);
        __mmask8 ResultMask = _kand_mask8(MaskX, MaskY);
        v512d Result = _mm512_mask_mov_pd(
            _mm512_setzero_pd(),
            ResultMask,
            _mm512_set1_pd(1.0)
        );

        return Result;
    }

    INTERNAL v512d
    operator>(zv2f64 VecA, zv2f64 VecB) 
    {
        __mmask8 MaskX = _mm512_cmp_pd_mask(VecA.X, VecB.X, _CMP_GT_OQ);
        __mmask8 MaskY = _mm512_cmp_pd_mask(VecA.Y, VecB.Y, _CMP_GT_OQ);
        __mmask8 ResultMask = _kand_mask8(MaskX, MaskY);
        v512d Result = _mm512_mask_mov_pd(
            _mm512_setzero_pd(),
            ResultMask,
            _mm512_set1_pd(1.0)
        );

        return Result;
    }

    INTERNAL v512d
    operator<=(zv2f64 VecA, zv2f64 VecB) 
    {
        __mmask8 MaskX = _mm512_cmp_pd_mask(VecA.X, VecB.X, _CMP_LE_OQ);
        __mmask8 MaskY = _mm512_cmp_pd_mask(VecA.Y, VecB.Y, _CMP_LE_OQ);
        __mmask8 ResultMask = _kand_mask8(MaskX, MaskY);
        v512d Result = _mm512_mask_mov_pd(
            _mm512_setzero_pd(),
            ResultMask,
            _mm512_set1_pd(1.0)
        );

        return Result;
    }

    INTERNAL v512d
    operator>=(zv2f64 VecA, zv2f64 VecB) 
    {
        __mmask8 MaskX = _mm512_cmp_pd_mask(VecA.X, VecB.X, _CMP_GE_OQ);
        __mmask8 MaskY = _mm512_cmp_pd_mask(VecA.Y, VecB.Y, _CMP_GE_OQ);
        __mmask8 ResultMask = _kand_mask8(MaskX, MaskY);
        v512d Result = _mm512_mask_mov_pd(
            _mm512_setzero_pd(),
            ResultMask,
            _mm512_set1_pd(1.0)
        );

        return Result;
    }

    INTERNAL zv2f64 
    operator%(zv2f64 VecA, zv2f64 VecB) 
    {
        zv2f64 Result = {};
        v512d DivX = _mm512_div_pd(VecA.X, VecB.X);
        v512d DivY = _mm512_div_pd(VecA.Y, VecB.Y);
        v512d FloorX = _mm512_roundscale_pd(DivX, _MM_FROUND_FLOOR);                // TODO: compare against _mm512_floor_pd and 
        v512d FloorY = _mm512_roundscale_pd(DivY, _MM_FROUND_FLOOR);                //     : _mm512_roundscale_round_pd

        Result.X = _mm512_fnmadd_pd(FloorX, VecB.X, VecA.X);
        Result.Y = _mm512_fnmadd_pd(FloorY, VecB.Y, VecA.Y);

        return Result;
    }

    INTERNAL v512d
    Dot(zv2f64 VecA, zv2f64 VecB)
    {
        v512d Result = {};

        Result = _mm512_fmadd_pd(
            VecA.X,
            VecB.X,
            _mm512_mul_pd(VecA.Y, VecB.Y)
        );

        return Result;
    }

    INTERNAL v512d
    LengthSq(zv2f64 Vec)
    {
        return Dot(Vec, Vec);
    }

    INTERNAL v512d
    Length(zv2f64 Vec)
    {
        return _mm512_sqrt_pd(LengthSq(Vec));
    }

    INTERNAL zv2f64
    Norm(zv2f64 Vec)
    {
        v512d Scale = _mm512_rsqrt14_pd(LengthSq(Vec));

        return Vec * Scale;
    }

    INTERNAL zv2f64
    Lerp(zv2f64 VecA, zv2f64 VecB, v512d T)
    {
        zv2f64 Result = {};

        Result.X = _mm512_fmadd_pd(
            T,
            VecB.X,
            _mm512_fnmadd_pd(T, VecA.X, VecA.X)
        );
        Result.Y = _mm512_fmadd_pd(
            T,
            VecB.Y,
            _mm512_fnmadd_pd(T, VecA.Y, VecA.Y)
        );

        return Result;
    }

    INTERNAL zv2f64
    Lerp(zv2f64 VecA, zv2f64 VecB, f64 T)
    {
        v512d ZT = _mm512_set1_pd(T);

        return Lerp(VecA, VecB, ZT);
    }

    INTERNAL zv3f32 
    operator+(zv3f32 VecA, zv3f32 VecB) 
    {
        zv3f32 Result = {};

        Result.X = _mm512_add_ps(VecA.X, VecB.X);
        Result.Y = _mm512_add_ps(VecA.Y, VecB.Y);
        Result.Z = _mm512_add_ps(VecA.Z, VecB.Z);

        return Result;
    }

    INTERNAL zv3f32 
    operator-(zv3f32 VecA, zv3f32 VecB) 
    {
        zv3f32 Result = {};

        Result.X = _mm512_sub_ps(VecA.X, VecB.X);
        Result.Y = _mm512_sub_ps(VecA.Y, VecB.Y);
        Result.Z = _mm512_sub_ps(VecA.Z, VecB.Z);

        return Result;
    }

    INTERNAL zv3f32 
    operator*(zv3f32 VecA, zv3f32 VecB) 
    {
        zv3f32 Result = {};

        Result.X = _mm512_mul_ps(VecA.X, VecB.X);
        Result.Y = _mm512_mul_ps(VecA.Y, VecB.Y);
        Result.Z = _mm512_mul_ps(VecA.Z, VecB.Z);

        return Result;
    }

    INTERNAL zv3f32 
    operator*(zv3f32 Vec, v512f Value) 
    {
        zv3f32 Result = {};

        Result.X = _mm512_mul_ps(Vec.X, Value);
        Result.Y = _mm512_mul_ps(Vec.Y, Value);
        Result.Z = _mm512_mul_ps(Vec.Z, Value);

        return Result;
    }

    INTERNAL zv3f32 
    operator*(zv3f32 Vec, f32 Value) 
    {
        v512f ZValue = _mm512_set1_ps(Value);

        return Vec * ZValue;
    }

    INTERNAL zv3f32 
    operator/(zv3f32 VecA, zv3f32 VecB) 
    {
        zv3f32 Result = {};

        Result.X = _mm512_div_ps(VecA.X, VecB.X);
        Result.Y = _mm512_div_ps(VecA.Y, VecB.Y);
        Result.Z = _mm512_div_ps(VecA.Z, VecB.Z);

        return Result;
    }

    INTERNAL zv3f32
    operator/(zv3f32 Vec, v512f Value) 
    {
        zv3f32 Result = {};

        Result.X = _mm512_div_ps(Vec.X, Value);
        Result.Y = _mm512_div_ps(Vec.Y, Value);
        Result.Z = _mm512_div_ps(Vec.Z, Value);

        return Result;
    }

    INTERNAL zv3f32
    operator/(zv3f32 Vec, f32 Value) 
    {
        v512f ZValue = _mm512_set1_ps(Value);

        return Vec / ZValue;
    }

    INTERNAL zv3f32 
    operator&(zv3f32 VecA, zv3f32 VecB) 
    {
        zv3f32 Result = {};

        Result.X = _mm512_and_ps(VecA.X, VecB.X);
        Result.Y = _mm512_and_ps(VecA.Y, VecB.Y);
        Result.Z = _mm512_and_ps(VecA.Z, VecB.Z);

        return Result;
    }

    INTERNAL zv3f32 
    operator|(zv3f32 VecA, zv3f32 VecB) 
    {
        zv3f32 Result = {};

        Result.X = _mm512_or_ps(VecA.X, VecB.X);
        Result.Y = _mm512_or_ps(VecA.Y, VecB.Y);
        Result.Z = _mm512_or_ps(VecA.Z, VecB.Z);

        return Result;
    }

    INTERNAL zv3f32 
    operator^(zv3f32 VecA, zv3f32 VecB) 
    {
        zv3f32 Result = {};

        Result.X = _mm512_xor_ps(VecA.X, VecB.X);
        Result.Y = _mm512_xor_ps(VecA.Y, VecB.Y);
        Result.Z = _mm512_xor_ps(VecA.Z, VecB.Z);

        return Result;
    }

    INTERNAL v512f
    operator==(zv3f32 VecA, zv3f32 VecB) 
    {
        __mmask16 MaskX = _mm512_cmp_ps_mask(VecA.X, VecB.X, _CMP_EQ_OQ);
        __mmask16 MaskY = _mm512_cmp_ps_mask(VecA.Y, VecB.Y, _CMP_EQ_OQ);
        __mmask16 MaskZ = _mm512_cmp_ps_mask(VecA.Z, VecB.Z, _CMP_EQ_OQ);
        __mmask16 ResultMask = _mm512_kand(_mm512_kand(MaskX, MaskY), MaskZ);
        v512f Result = _mm512_mask_mov_ps(
            _mm512_setzero_ps(),
            ResultMask,
            _mm512_set1_ps(1.0f)
        );

        return Result;
    }

    INTERNAL v512f 
    operator!=(zv3f32 VecA, zv3f32 VecB) 
    {
        __mmask16 MaskX = _mm512_cmp_ps_mask(VecA.X, VecB.X, _CMP_NEQ_OQ);
        __mmask16 MaskY = _mm512_cmp_ps_mask(VecA.Y, VecB.Y, _CMP_NEQ_OQ);
        __mmask16 MaskZ = _mm512_cmp_ps_mask(VecA.Z, VecB.Z, _CMP_NEQ_OQ);
        __mmask16 ResultMask = _mm512_kand(_mm512_kand(MaskX, MaskY), MaskZ);
        v512f Result = _mm512_mask_mov_ps(
            _mm512_setzero_ps(),
            ResultMask,
            _mm512_set1_ps(1.0f)
        );

        return Result;
    }

    INTERNAL v512f 
    operator<(zv3f32 VecA, zv3f32 VecB) 
    {
        __mmask16 MaskX = _mm512_cmp_ps_mask(VecA.X, VecB.X, _CMP_LT_OQ);
        __mmask16 MaskY = _mm512_cmp_ps_mask(VecA.Y, VecB.Y, _CMP_LT_OQ);
        __mmask16 MaskZ = _mm512_cmp_ps_mask(VecA.Z, VecB.Z, _CMP_LT_OQ);
        __mmask16 ResultMask = _mm512_kand(_mm512_kand(MaskX, MaskY), MaskZ);
        v512f Result = _mm512_mask_mov_ps(
            _mm512_setzero_ps(),
            ResultMask,
            _mm512_set1_ps(1.0f)
        );

        return Result;
    }

    INTERNAL v512f 
    operator>(zv3f32 VecA, zv3f32 VecB) 
    {
        __mmask16 MaskX = _mm512_cmp_ps_mask(VecA.X, VecB.X, _CMP_GT_OQ);
        __mmask16 MaskY = _mm512_cmp_ps_mask(VecA.Y, VecB.Y, _CMP_GT_OQ);
        __mmask16 MaskZ = _mm512_cmp_ps_mask(VecA.Z, VecB.Z, _CMP_GT_OQ);
        __mmask16 ResultMask = _mm512_kand(_mm512_kand(MaskX, MaskY), MaskZ);
        v512f Result = _mm512_mask_mov_ps(
            _mm512_setzero_ps(),
            ResultMask,
            _mm512_set1_ps(1.0f)
        );

        return Result;
    }

    INTERNAL v512f 
    operator<=(zv3f32 VecA, zv3f32 VecB) 
    {
        __mmask16 MaskX = _mm512_cmp_ps_mask(VecA.X, VecB.X, _CMP_LE_OQ);
        __mmask16 MaskY = _mm512_cmp_ps_mask(VecA.Y, VecB.Y, _CMP_LE_OQ);
        __mmask16 MaskZ = _mm512_cmp_ps_mask(VecA.Z, VecB.Z, _CMP_LE_OQ);
        __mmask16 ResultMask = _mm512_kand(_mm512_kand(MaskX, MaskY), MaskZ);
        v512f Result = _mm512_mask_mov_ps(
            _mm512_setzero_ps(),
            ResultMask,
            _mm512_set1_ps(1.0f)
        );

        return Result;
    }

    INTERNAL v512f 
    operator>=(zv3f32 VecA, zv3f32 VecB) 
    {
        __mmask16 MaskX = _mm512_cmp_ps_mask(VecA.X, VecB.X, _CMP_GE_OQ);
        __mmask16 MaskY = _mm512_cmp_ps_mask(VecA.Y, VecB.Y, _CMP_GE_OQ);
        __mmask16 MaskZ = _mm512_cmp_ps_mask(VecA.Z, VecB.Z, _CMP_GE_OQ);
        __mmask16 ResultMask = _mm512_kand(_mm512_kand(MaskX, MaskY), MaskZ);
        v512f Result = _mm512_mask_mov_ps(
            _mm512_setzero_ps(),
            ResultMask,
            _mm512_set1_ps(1.0f)
        );

        return Result;
    }

    INTERNAL zv3f32 
    operator%(zv3f32 VecA, zv3f32 VecB) 
    {
        zv3f32 Result = {};
        v512f DivX = _mm512_div_ps(VecA.X, VecB.X);
        v512f DivY = _mm512_div_ps(VecA.Y, VecB.Y);
        v512f DivZ = _mm512_div_ps(VecA.Z, VecB.Z);
        v512f FloorX = _mm512_roundscale_ps(DivX, _MM_FROUND_FLOOR);                // TODO: compare against _mm512_floor_ps and 
        v512f FloorY = _mm512_roundscale_ps(DivY, _MM_FROUND_FLOOR);                //     : _mm512_roundscale_round_ps
        v512f FloorZ = _mm512_roundscale_ps(DivZ, _MM_FROUND_FLOOR);

        Result.X = _mm512_fnmadd_ps(FloorX, VecB.X, VecA.X);
        Result.Y = _mm512_fnmadd_ps(FloorY, VecB.Y, VecA.Y);
        Result.Z = _mm512_fnmadd_ps(FloorZ, VecB.Z, VecA.Z);

        return Result;
    }

    INTERNAL v512f
    Dot(zv3f32 VecA, zv3f32 VecB)
    {
        v512f Result = {};

        Result = _mm512_fmadd_ps(
            VecA.X,
            VecB.X,
            _mm512_fmadd_ps(
                VecA.Y,
                VecB.Y,
                _mm512_mul_ps(VecA.Z, VecB.Z)
            )
        );

        return Result;
    }

    INTERNAL v512f
    LengthSq(zv3f32 Vec)
    {
        return Dot(Vec, Vec);
    }

    INTERNAL v512f
    Length(zv3f32 Vec)
    {
        return _mm512_sqrt_ps(LengthSq(Vec));
    }

    INTERNAL zv3f32
    Norm(zv3f32 Vec)
    {
        v512f Scale = _mm512_rsqrt14_ps(LengthSq(Vec));

        return Vec * Scale;
    }

    INTERNAL zv3f32
    Lerp(zv3f32 VecA, zv3f32 VecB, v512f T)
    {
        zv3f32 Result = {};

        Result.X = _mm512_fmadd_ps(
            T, 
            VecB.X, 
            _mm512_fnmadd_ps(T, VecA.X, VecA.X)
        );
        Result.Y = _mm512_fmadd_ps(
            T, 
            VecB.Y, 
            _mm512_fnmadd_ps(T, VecA.Y, VecA.Y)
        );
        Result.Z = _mm512_fmadd_ps(
            T, 
            VecB.Z, 
            _mm512_fnmadd_ps(T, VecA.Z, VecA.Z)
        );

        return Result;
    }

    INTERNAL zv3f32
    Lerp(zv3f32 VecA, zv3f32 VecB, f32 T)
    {
        v512f ZT = _mm512_set1_ps(T);

        return Lerp(VecA, VecB, ZT);
    }

    INTERNAL zv3f32
    Cross(zv3f32 VecA, zv3f32 VecB)
    {
        zv3f32 Result = {};

        Result.X = _mm512_fmsub_ps(
            VecA.Y,
            VecB.Z,
            _mm512_mul_ps(VecA.Z, VecB.Y)
        );
        Result.Y = _mm512_fmsub_ps(
            VecA.Z,
            VecB.X,
            _mm512_mul_ps(VecA.X, VecB.Z)
        );
        Result.Z = _mm512_fmsub_ps(
            VecA.X,
            VecB.Y,
            _mm512_mul_ps(VecA.Y, VecB.X)
        );

        return Result;
    }

    INTERNAL zv3i32 
    operator+(zv3i32 VecA, zv3i32 VecB) 
    {
        zv3i32 Result = {};

        Result.X = _mm512_add_epi32(VecA.X, VecB.X);
        Result.Y = _mm512_add_epi32(VecA.Y, VecB.X);
        Result.Z = _mm512_add_epi32(VecA.Z, VecB.X);

        return Result;
    }

    INTERNAL zv3i32 
    operator-(zv3i32 VecA, zv3i32 VecB) 
    {
        zv3i32 Result = {};

        Result.X = _mm512_sub_epi32(VecA.X, VecB.X);
        Result.Y = _mm512_sub_epi32(VecA.Y, VecB.X);
        Result.Z = _mm512_sub_epi32(VecA.Z, VecB.X);

        return Result;
    }

    INTERNAL zv3i32 
    operator*(zv3i32 VecA, zv3i32 VecB) 
    {
        zv3i32 Result = {};

        Result.X = _mm512_mullo_epi32(VecA.X, VecB.X);
        Result.Y = _mm512_mullo_epi32(VecA.Y, VecB.X);
        Result.Z = _mm512_mullo_epi32(VecA.Z, VecB.X);

        return Result;
    }

    INTERNAL zv3i32 
    operator*(zv3i32 Vec, v512 Value) 
    {
        zv3i32 Result = {};

        Result.X = _mm512_mullo_epi32(Vec.X, Value);
        Result.Y = _mm512_mullo_epi32(Vec.Y, Value);
        Result.Z = _mm512_mullo_epi32(Vec.Z, Value);

        return Result;
    }

    INTERNAL zv3i32 
    operator*(zv3i32 Vec, i32 Value) 
    {
        v512 ZValue = _mm512_set1_epi32(Value);

        return Vec * ZValue;
    }

    INTERNAL zv3i32 
    operator/(zv3i32 VecA, zv3i32 VecB) 
    {
        zv3i32 Result = {};
        v512f FloatAX = _mm512_cvtepi32_ps(VecA.X);
        v512f FloatAY = _mm512_cvtepi32_ps(VecA.Y);
        v512f FloatAZ = _mm512_cvtepi32_ps(VecA.Z);
        v512f FloatBX = _mm512_cvtepi32_ps(VecB.X);
        v512f FloatBY = _mm512_cvtepi32_ps(VecB.Y);
        v512f FloatBZ = _mm512_cvtepi32_ps(VecB.Z);
        v512f QuotientX = _mm512_div_ps(FloatAX, FloatBX);
        v512f QuotientY = _mm512_div_ps(FloatAY, FloatBY);
        v512f QuotientZ = _mm512_div_ps(FloatAZ, FloatBZ);

        Result.X = _mm512_cvtps_epi32(QuotientX);
        Result.Y = _mm512_cvtps_epi32(QuotientY);
        Result.Z = _mm512_cvtps_epi32(QuotientZ);

        return Result;
    }

    INTERNAL zv3i32 
    operator&(zv3i32 VecA, zv3i32 VecB) 
    {
        zv3i32 Result = {};

        Result.X = _mm512_and_epi32(VecA.X, VecB.X);
        Result.Y = _mm512_and_epi32(VecA.Y, VecB.X);
        Result.Z = _mm512_and_epi32(VecA.Z, VecB.X);

        return Result;
    }

    INTERNAL zv3i32 
    operator|(zv3i32 VecA, zv3i32 VecB) 
    {
        zv3i32 Result = {};

        Result.X = _mm512_or_epi32(VecA.X, VecB.X);
        Result.Y = _mm512_or_epi32(VecA.Y, VecB.X);
        Result.Z = _mm512_or_epi32(VecA.Z, VecB.X);

        return Result;
    }

    INTERNAL zv3i32 
    operator^(zv3i32 VecA, zv3i32 VecB) 
    {
        zv3i32 Result = {};

        Result.X = _mm512_xor_epi32(VecA.X, VecB.X);
        Result.Y = _mm512_xor_epi32(VecA.Y, VecB.X);
        Result.Z = _mm512_xor_epi32(VecA.Z, VecB.X);

        return Result;
    }

    INTERNAL v512
    operator==(zv3i32 VecA, zv3i32 VecB) 
    {
        __mmask16 MaskX = _mm512_cmp_epi32_mask(VecA.X, VecB.X, _MM_CMPINT_EQ);
        __mmask16 MaskY = _mm512_cmp_epi32_mask(VecA.Y, VecB.Y, _MM_CMPINT_EQ);
        __mmask16 MaskZ = _mm512_cmp_epi32_mask(VecA.Z, VecB.Z, _MM_CMPINT_EQ);
        __mmask16 ResultMask = _mm512_kand(_mm512_kand(MaskX, MaskY), MaskZ);
        v512 Result = _mm512_mask_mov_epi32(
            _mm512_setzero_epi32(),
            ResultMask,
            _mm512_set1_epi32(~0)
        );

        return Result;
    }

    INTERNAL v512
    operator!=(zv3i32 VecA, zv3i32 VecB) 
    {
        __mmask16 MaskX = _mm512_cmp_epi32_mask(VecA.X, VecB.X, _MM_CMPINT_NE);
        __mmask16 MaskY = _mm512_cmp_epi32_mask(VecA.Y, VecB.Y, _MM_CMPINT_NE);
        __mmask16 MaskZ = _mm512_cmp_epi32_mask(VecA.Z, VecB.Z, _MM_CMPINT_NE);
        __mmask16 ResultMask = _mm512_kand(_mm512_kand(MaskX, MaskY), MaskZ);
        v512 Result = _mm512_mask_mov_epi32(
            _mm512_setzero_epi32(),
            ResultMask,
            _mm512_set1_epi32(~0)
        );

        return Result;
    }

    INTERNAL v512
    operator<(zv3i32 VecA, zv3i32 VecB) 
    {
        __mmask16 MaskX = _mm512_cmp_epi32_mask(VecA.X, VecB.X, _MM_CMPINT_LT);
        __mmask16 MaskY = _mm512_cmp_epi32_mask(VecA.Y, VecB.Y, _MM_CMPINT_LT);
        __mmask16 MaskZ = _mm512_cmp_epi32_mask(VecA.Z, VecB.Z, _MM_CMPINT_LT);
        __mmask16 ResultMask = _mm512_kand(_mm512_kand(MaskX, MaskY), MaskZ);
        v512 Result = _mm512_mask_mov_epi32(
            _mm512_setzero_epi32(),
            ResultMask,
            _mm512_set1_epi32(~0)
        );

        return Result;
    }

    INTERNAL v512
    operator>(zv3i32 VecA, zv3i32 VecB) 
    {
        __mmask16 MaskX = _mm512_cmp_epi32_mask(VecA.X, VecB.X, _MM_CMPINT_GT);
        __mmask16 MaskY = _mm512_cmp_epi32_mask(VecA.Y, VecB.Y, _MM_CMPINT_GT);
        __mmask16 MaskZ = _mm512_cmp_epi32_mask(VecA.Z, VecB.Z, _MM_CMPINT_GT);
        __mmask16 ResultMask = _mm512_kand(_mm512_kand(MaskX, MaskY), MaskZ);
        v512 Result = _mm512_mask_mov_epi32(
            _mm512_setzero_epi32(),
            ResultMask,
            _mm512_set1_epi32(~0)
        );

        return Result;
    }

    INTERNAL v512
    operator<=(zv3i32 VecA, zv3i32 VecB) 
    {
        __mmask16 MaskX = _mm512_cmp_epi32_mask(VecA.X, VecB.X, _MM_CMPINT_LE);
        __mmask16 MaskY = _mm512_cmp_epi32_mask(VecA.Y, VecB.Y, _MM_CMPINT_LE);
        __mmask16 MaskZ = _mm512_cmp_epi32_mask(VecA.Z, VecB.Z, _MM_CMPINT_LE);
        __mmask16 ResultMask = _mm512_kand(_mm512_kand(MaskX, MaskY), MaskZ);
        v512 Result = _mm512_mask_mov_epi32(
            _mm512_setzero_epi32(),
            ResultMask,
            _mm512_set1_epi32(~0)
        );

        return Result;
    }

    INTERNAL v512
    operator>=(zv3i32 VecA, zv3i32 VecB) 
    {
        __mmask16 MaskX = _mm512_cmp_epi32_mask(VecA.X, VecB.X, _MM_CMPINT_GE);
        __mmask16 MaskY = _mm512_cmp_epi32_mask(VecA.Y, VecB.Y, _MM_CMPINT_GE);
        __mmask16 MaskZ = _mm512_cmp_epi32_mask(VecA.Z, VecB.Z, _MM_CMPINT_GE);
        __mmask16 ResultMask = _mm512_kand(_mm512_kand(MaskX, MaskY), MaskZ);
        v512 Result = _mm512_mask_mov_epi32(
            _mm512_setzero_epi32(),
            ResultMask,
            _mm512_set1_epi32(~0)
        );

        return Result;
    }

    INTERNAL zv3i32 
    operator<<(zv3i32 VecA, i32 Shift)
    {
        zv3i32 Result = {};

        Result.X = _mm512_slli_epi32(VecA.X, Shift);
        Result.Y = _mm512_slli_epi32(VecA.Y, Shift);
        Result.Z = _mm512_slli_epi32(VecA.Z, Shift);

        return Result;
    }

    INTERNAL zv3i32 
    operator>>(zv3i32 VecA, i32 Shift)
    {
        zv3i32 Result = {};

        Result.X = _mm512_srli_epi32(VecA.X, Shift);
        Result.Y = _mm512_srli_epi32(VecA.Y, Shift);
        Result.Z = _mm512_srli_epi32(VecA.Z, Shift);

        return Result;
    }

    INTERNAL v512
    Dot(zv3i32 VecA, zv3i32 VecB)
    {
        v512 Result = {};

        Result = _mm512_add_epi32(
            _mm512_add_epi32(
                _mm512_mullo_epi32(VecA.X, VecB.X),
                _mm512_mullo_epi32(VecA.Y, VecB.Y)
            ),
            _mm512_mullo_epi32(VecA.Z, VecB.Z)
        );

        return Result;
    }

    INTERNAL v512
    LengthSq(zv3i32 Vec)
    {
        return Dot(Vec, Vec);
    }

    INTERNAL v512
    Length(zv3i32 Vec)
    {
        v512f LenSq = _mm512_cvtepi32_ps(LengthSq(Vec));
        
        return _mm512_cvtps_epi32(
            _mm512_sqrt_ps(LenSq)
        );
    }

    INTERNAL zv3i32
    Norm(zv3i32 Vec)
    {
        v512 Scale = _mm512_cvtps_epi32(
            _mm512_rcp14_ps(
                _mm512_cvtepi32_ps(
                    Length(Vec)
                )
            )
        );

        return Vec * Scale;
    }

    INTERNAL zv3i32
    Lerp(zv3i32 VecA, zv3i32 VecB, v512 T)
    {
        zv3i32 Result = {};
        v512f ZT = _mm512_cvtepi32_ps(T);
        v512f ResultX = _mm512_fmadd_ps(
            ZT,
            _mm512_cvtepi32_ps(VecB.X),
            _mm512_fnmadd_ps(
                ZT, 
                _mm512_cvtepi32_ps(VecA.X), 
                _mm512_cvtepi32_ps(VecA.X)
            )
        );
        v512f ResultY = _mm512_fmadd_ps(
            ZT,
            _mm512_cvtepi32_ps(VecB.Y),
            _mm512_fnmadd_ps(
                ZT, 
                _mm512_cvtepi32_ps(VecA.Y), 
                _mm512_cvtepi32_ps(VecA.Y)
            )
        );
        v512f ResultZ = _mm512_fmadd_ps(
            ZT,
            _mm512_cvtepi32_ps(VecB.Z),
            _mm512_fnmadd_ps(
                ZT, 
                _mm512_cvtepi32_ps(VecA.Z), 
                _mm512_cvtepi32_ps(VecA.Z)
            )
        );

        Result.X = _mm512_cvtps_epi32(ResultX);
        Result.Y = _mm512_cvtps_epi32(ResultY);
        Result.Z = _mm512_cvtps_epi32(ResultZ);

        return Result;
    }

    INTERNAL zv3i32
    Lerp(zv3i32 VecA, zv3i32 VecB, i32 T)
    {
        v512 ZT = _mm512_set1_epi32(T);

        return Lerp(VecA, VecB, ZT);
    }

    INTERNAL zv3i32
    Cross(zv3i32 VecA, zv3i32 VecB)
    {
        zv3i32 Result = {};

        Result.X = _mm512_sub_epi32(
            _mm512_mullo_epi32(VecA.Y, VecB.Z),
            _mm512_mullo_epi32(VecA.Z, VecB.Y)
        );
        Result.Y = _mm512_sub_epi32(
            _mm512_mullo_epi32(VecA.Z, VecB.X),
            _mm512_mullo_epi32(VecA.X, VecB.Z)
        );
        Result.Z = _mm512_sub_epi32(
            _mm512_mullo_epi32(VecA.X, VecB.Y),
            _mm512_mullo_epi32(VecA.Y, VecB.X)
        );

        return Result;
    }

    INTERNAL zv3f64 
    operator+(zv3f64 VecA, zv3f64 VecB) 
    {
        zv3f64 Result = {};

        Result.X = _mm512_add_pd(VecA.X, VecB.X);
        Result.Y = _mm512_add_pd(VecA.Y, VecB.Y);
        Result.Z = _mm512_add_pd(VecA.Z, VecB.Z);

        return Result;
    }

    INTERNAL zv3f64 
    operator-(zv3f64 VecA, zv3f64 VecB) 
    {
        zv3f64 Result = {};

        Result.X = _mm512_sub_pd(VecA.X, VecB.X);
        Result.Y = _mm512_sub_pd(VecA.Y, VecB.Y);
        Result.Z = _mm512_sub_pd(VecA.Z, VecB.Z);

        return Result;
    }

    INTERNAL zv3f64 
    operator*(zv3f64 VecA, zv3f64 VecB) 
    {
        zv3f64 Result = {};

        Result.X = _mm512_mul_pd(VecA.X, VecB.X);
        Result.Y = _mm512_mul_pd(VecA.Y, VecB.Y);
        Result.Z = _mm512_mul_pd(VecA.Z, VecB.Z);

        return Result;
    }

    INTERNAL zv3f64 
    operator*(zv3f64 Vec, v512d Value) 
    {
        zv3f64 Result = {};

        Result.X = _mm512_mul_pd(Vec.X, Value);
        Result.Y = _mm512_mul_pd(Vec.Y, Value);
        Result.Z = _mm512_mul_pd(Vec.Z, Value);

        return Result;
    }

    INTERNAL zv3f64 
    operator*(zv3f64 Vec, f64 Value) 
    {
        v512d ZValue = _mm512_set1_pd(Value);

        return Vec * ZValue;
    }

    INTERNAL zv3f64 
    operator/(zv3f64 VecA, zv3f64 VecB) 
    {
        zv3f64 Result = {};

        Result.X = _mm512_div_pd(VecA.X, VecB.X);
        Result.Y = _mm512_div_pd(VecA.Y, VecB.Y);
        Result.Z = _mm512_div_pd(VecA.Z, VecB.Z);

        return Result;
    }

    INTERNAL zv3f64
    operator/(zv3f64 Vec, v512d Value) 
    {
        zv3f64 Result = {};

        Result.X = _mm512_div_pd(Vec.X, Value);
        Result.Y = _mm512_div_pd(Vec.Y, Value);
        Result.Z = _mm512_div_pd(Vec.Z, Value);

        return Result;
    }

    INTERNAL zv3f64
    operator/(zv3f64 Vec, f64 Value) 
    {
        v512d ZValue = _mm512_set1_pd(Value);

        return Vec / ZValue;
    }

    INTERNAL zv3f64 
    operator&(zv3f64 VecA, zv3f64 VecB) 
    {
        zv3f64 Result = {};

        Result.X = _mm512_and_pd(VecA.X, VecB.X);
        Result.Y = _mm512_and_pd(VecA.Y, VecB.Y);
        Result.Z = _mm512_and_pd(VecA.Z, VecB.Z);

        return Result;
    }

    INTERNAL zv3f64 
    operator|(zv3f64 VecA, zv3f64 VecB) 
    {
        zv3f64 Result = {};

        Result.X = _mm512_or_pd(VecA.X, VecB.X);
        Result.Y = _mm512_or_pd(VecA.Y, VecB.Y);
        Result.Z = _mm512_or_pd(VecA.Z, VecB.Z);

        return Result;
    }

    INTERNAL zv3f64 
    operator^(zv3f64 VecA, zv3f64 VecB) 
    {
        zv3f64 Result = {};

        Result.X = _mm512_xor_pd(VecA.X, VecB.X);
        Result.Y = _mm512_xor_pd(VecA.Y, VecB.Y);
        Result.Z = _mm512_xor_pd(VecA.Z, VecB.Z);

        return Result;
    }

    INTERNAL v512d
    operator==(zv3f64 VecA, zv3f64 VecB) 
    {
        __mmask8 MaskX = _mm512_cmp_pd_mask(VecA.X, VecB.X, _CMP_EQ_OQ);
        __mmask8 MaskY = _mm512_cmp_pd_mask(VecA.Y, VecB.Y, _CMP_EQ_OQ);
        __mmask8 MaskZ = _mm512_cmp_pd_mask(VecA.Z, VecB.Z, _CMP_EQ_OQ);
        __mmask8 ResultMask = _kand_mask8(_kand_mask8(MaskX, MaskY), MaskZ);
        v512d Result = _mm512_mask_mov_pd(
            _mm512_setzero_pd(),
            ResultMask,
            _mm512_set1_pd(1.0)
        );

        return Result;
    }

    INTERNAL v512d
    operator!=(zv3f64 VecA, zv3f64 VecB) 
    {
        __mmask8 MaskX = _mm512_cmp_pd_mask(VecA.X, VecB.X, _CMP_NEQ_OQ);
        __mmask8 MaskY = _mm512_cmp_pd_mask(VecA.Y, VecB.Y, _CMP_NEQ_OQ);
        __mmask8 MaskZ = _mm512_cmp_pd_mask(VecA.Z, VecB.Z, _CMP_NEQ_OQ);
        __mmask8 ResultMask = _kand_mask8(_kand_mask8(MaskX, MaskY), MaskZ);
        v512d Result = _mm512_mask_mov_pd(
            _mm512_setzero_pd(),
            ResultMask,
            _mm512_set1_pd(1.0)
        );

        return Result;
    }

    INTERNAL v512d
    operator<(zv3f64 VecA, zv3f64 VecB) 
    {
        __mmask8 MaskX = _mm512_cmp_pd_mask(VecA.X, VecB.X, _CMP_LT_OQ);
        __mmask8 MaskY = _mm512_cmp_pd_mask(VecA.Y, VecB.Y, _CMP_LT_OQ);
        __mmask8 MaskZ = _mm512_cmp_pd_mask(VecA.Z, VecB.Z, _CMP_LT_OQ);
        __mmask8 ResultMask = _kand_mask8(_kand_mask8(MaskX, MaskY), MaskZ);
        v512d Result = _mm512_mask_mov_pd(
            _mm512_setzero_pd(),
            ResultMask,
            _mm512_set1_pd(1.0)
        );

        return Result;
    }

    INTERNAL v512d
    operator>(zv3f64 VecA, zv3f64 VecB) 
    {
        __mmask8 MaskX = _mm512_cmp_pd_mask(VecA.X, VecB.X, _CMP_GT_OQ);
        __mmask8 MaskY = _mm512_cmp_pd_mask(VecA.Y, VecB.Y, _CMP_GT_OQ);
        __mmask8 MaskZ = _mm512_cmp_pd_mask(VecA.Z, VecB.Z, _CMP_GT_OQ);
        __mmask8 ResultMask = _kand_mask8(_kand_mask8(MaskX, MaskY), MaskZ);
        v512d Result = _mm512_mask_mov_pd(
            _mm512_setzero_pd(),
            ResultMask,
            _mm512_set1_pd(1.0)
        );

        return Result;
    }

    INTERNAL v512d
    operator<=(zv3f64 VecA, zv3f64 VecB) 
    {
        __mmask8 MaskX = _mm512_cmp_pd_mask(VecA.X, VecB.X, _CMP_LE_OQ);
        __mmask8 MaskY = _mm512_cmp_pd_mask(VecA.Y, VecB.Y, _CMP_LE_OQ);
        __mmask8 MaskZ = _mm512_cmp_pd_mask(VecA.Z, VecB.Z, _CMP_LE_OQ);
        __mmask8 ResultMask = _kand_mask8(_kand_mask8(MaskX, MaskY), MaskZ);
        v512d Result = _mm512_mask_mov_pd(
            _mm512_setzero_pd(),
            ResultMask,
            _mm512_set1_pd(1.0)
        );

        return Result;
    }

    INTERNAL v512d
    operator>=(zv3f64 VecA, zv3f64 VecB) 
    {
        __mmask8 MaskX = _mm512_cmp_pd_mask(VecA.X, VecB.X, _CMP_GE_OQ);
        __mmask8 MaskY = _mm512_cmp_pd_mask(VecA.Y, VecB.Y, _CMP_GE_OQ);
        __mmask8 MaskZ = _mm512_cmp_pd_mask(VecA.Z, VecB.Z, _CMP_GE_OQ);
        __mmask8 ResultMask = _kand_mask8(_kand_mask8(MaskX, MaskY), MaskZ);
        v512d Result = _mm512_mask_mov_pd(
            _mm512_setzero_pd(),
            ResultMask,
            _mm512_set1_pd(1.0)
        );

        return Result;
    }

    INTERNAL v512d
    Dot(zv3f64 VecA, zv3f64 VecB)
    {
        v512d Result = {};

        Result = _mm512_fmadd_pd(
            VecA.X,
            VecB.X,
            _mm512_fmadd_pd(
                VecA.Y,
                VecB.Y,
                _mm512_mul_pd(VecA.Z, VecB.Z)
            )
        );

        return Result;
    }

    INTERNAL v512d
    LengthSq(zv3f64 Vec)
    {
        return Dot(Vec, Vec);
    }

    INTERNAL v512d
    Length(zv3f64 Vec)
    {
        return _mm512_sqrt_pd(LengthSq(Vec));
    }

    INTERNAL zv3f64
    Norm(zv3f64 Vec)
    {
        v512d Scale = _mm512_rsqrt14_pd(LengthSq(Vec));

        return Vec * Scale;
    }

    INTERNAL zv3f64
    Lerp(zv3f64 VecA, zv3f64 VecB, v512d T)
    {
        zv3f64 Result = {};

        Result.X = _mm512_fmadd_pd(T, VecB.X, _mm512_fnmadd_pd(T, VecA.X, VecA.X));
        Result.Y = _mm512_fmadd_pd(T, VecB.Y, _mm512_fnmadd_pd(T, VecA.Y, VecA.Y));
        Result.Z = _mm512_fmadd_pd(T, VecB.Z, _mm512_fnmadd_pd(T, VecA.Z, VecA.Z));

        return Result;
    }

    INTERNAL zv3f64
    Lerp(zv3f64 VecA, zv3f64 VecB, f64 T)
    {
        v512d ZT = _mm512_set1_pd(T);

        return Lerp(VecA, VecB, ZT);
    }

    INTERNAL zv3f64
    Cross(zv3f64 VecA, zv3f64 VecB)
    {
        zv3f64 Result = {};

        Result.X = _mm512_fmsub_pd(
            VecA.Y,
            VecB.Z,
            _mm512_mul_pd(VecA.Z, VecB.Y)
        );
        Result.Y = _mm512_fmsub_pd(
            VecA.Z,
            VecB.X,
            _mm512_mul_pd(VecA.X, VecB.Z)
        );
        Result.Z = _mm512_fmsub_pd(
            VecA.X,
            VecB.Y,
            _mm512_mul_pd(VecA.Y, VecB.X)
        );

        return Result;
    }

    INTERNAL zv4f32 
    operator+(zv4f32 VecA, zv4f32 VecB) 
    {
        zv4f32 Result = {};

        Result.X = _mm512_add_ps(VecA.X, VecB.X);
        Result.Y = _mm512_add_ps(VecA.Y, VecB.Y);
        Result.Z = _mm512_add_ps(VecA.Z, VecB.Z);
        Result.W = _mm512_add_ps(VecA.W, VecB.W);

        return Result;
    }

    INTERNAL zv4f32 
    operator-(zv4f32 VecA, zv4f32 VecB) 
    {
        zv4f32 Result = {};

        Result.X = _mm512_sub_ps(VecA.X, VecB.X);
        Result.Y = _mm512_sub_ps(VecA.Y, VecB.Y);
        Result.Z = _mm512_sub_ps(VecA.Z, VecB.Z);
        Result.W = _mm512_sub_ps(VecA.W, VecB.W);

        return Result;
    }

    INTERNAL zv4f32 
    operator*(zv4f32 VecA, zv4f32 VecB) 
    {
        zv4f32 Result = {};

        Result.X = _mm512_mul_ps(VecA.X, VecB.X);
        Result.Y = _mm512_mul_ps(VecA.Y, VecB.Y);
        Result.Z = _mm512_mul_ps(VecA.Z, VecB.Z);
        Result.W = _mm512_mul_ps(VecA.W, VecB.W);

        return Result;
    }

    INTERNAL zv4f32 
    operator*(zv4f32 Vec, v512f Value) 
    {
        zv4f32 Result = {};

        Result.X = _mm512_mul_ps(Vec.X, Value);
        Result.Y = _mm512_mul_ps(Vec.Y, Value);
        Result.Z = _mm512_mul_ps(Vec.Z, Value);
        Result.W = _mm512_mul_ps(Vec.W, Value);

        return Result;
    }

    INTERNAL zv4f32 
    operator*(zv4f32 Vec, f32 Value) 
    {
        v512f ZValue = _mm512_set1_ps(Value);

        return Vec * ZValue;
    }

    INTERNAL zv4f32 
    operator/(zv4f32 VecA, zv4f32 VecB) 
    {
        zv4f32 Result = {};

        Result.X = _mm512_div_ps(VecA.X, VecB.X);
        Result.Y = _mm512_div_ps(VecA.Y, VecB.Y);
        Result.Z = _mm512_div_ps(VecA.Z, VecB.Z);
        Result.W = _mm512_div_ps(VecA.W, VecB.W);

        return Result;
    }

    INTERNAL zv4f32 
    operator&(zv4f32 VecA, zv4f32 VecB) 
    {
        zv4f32 Result = {};

        Result.X = _mm512_and_ps(VecA.X, VecB.X);
        Result.Y = _mm512_and_ps(VecA.Y, VecB.Y);
        Result.Z = _mm512_and_ps(VecA.Z, VecB.Z);
        Result.W = _mm512_and_ps(VecA.W, VecB.W);

        return Result;
    }

    INTERNAL zv4f32 
    operator|(zv4f32 VecA, zv4f32 VecB) 
    {
        zv4f32 Result = {};

        Result.X = _mm512_or_ps(VecA.X, VecB.X);
        Result.Y = _mm512_or_ps(VecA.Y, VecB.Y);
        Result.Z = _mm512_or_ps(VecA.Z, VecB.Z);
        Result.W = _mm512_or_ps(VecA.W, VecB.W);

        return Result;
    }

    INTERNAL zv4f32 
    operator^(zv4f32 VecA, zv4f32 VecB) 
    {
        zv4f32 Result = {};

        Result.X = _mm512_xor_ps(VecA.X, VecB.X);
        Result.Y = _mm512_xor_ps(VecA.Y, VecB.Y);
        Result.Z = _mm512_xor_ps(VecA.Z, VecB.Z);
        Result.W = _mm512_xor_ps(VecA.W, VecB.W);

        return Result;
    }

    INTERNAL v512f
    operator==(zv4f32 VecA, zv4f32 VecB) 
    {
        __mmask16 MaskX = _mm512_cmp_ps_mask(VecA.X, VecB.X, _CMP_EQ_OQ);
        __mmask16 MaskY = _mm512_cmp_ps_mask(VecA.Y, VecB.Y, _CMP_EQ_OQ);
        __mmask16 MaskZ = _mm512_cmp_ps_mask(VecA.Z, VecB.Z, _CMP_EQ_OQ);
        __mmask16 MaskW = _mm512_cmp_ps_mask(VecA.W, VecB.W, _CMP_EQ_OQ);
        __mmask16 ResultMask = _mm512_kand(
            _mm512_kand(MaskX, MaskY), 
            _mm512_kand(MaskZ, MaskW)
        );
        v512f Result = _mm512_mask_mov_ps(
            _mm512_setzero_ps(),
            ResultMask,
            _mm512_set1_ps(1.0f)
        );

        return Result;
    }

    INTERNAL v512f
    operator!=(zv4f32 VecA, zv4f32 VecB) 
    {
        __mmask16 MaskX = _mm512_cmp_ps_mask(VecA.X, VecB.X, _CMP_NEQ_OQ);
        __mmask16 MaskY = _mm512_cmp_ps_mask(VecA.Y, VecB.Y, _CMP_NEQ_OQ);
        __mmask16 MaskZ = _mm512_cmp_ps_mask(VecA.Z, VecB.Z, _CMP_NEQ_OQ);
        __mmask16 MaskW = _mm512_cmp_ps_mask(VecA.W, VecB.W, _CMP_NEQ_OQ);
        __mmask16 ResultMask = _mm512_kand(
            _mm512_kand(MaskX, MaskY), 
            _mm512_kand(MaskZ, MaskW)
        );
        v512f Result = _mm512_mask_mov_ps(
            _mm512_setzero_ps(),
            ResultMask,
            _mm512_set1_ps(1.0f)
        );

        return Result;
    }

    INTERNAL v512f
    operator<(zv4f32 VecA, zv4f32 VecB) 
    {
        __mmask16 MaskX = _mm512_cmp_ps_mask(VecA.X, VecB.X, _CMP_LT_OQ);
        __mmask16 MaskY = _mm512_cmp_ps_mask(VecA.Y, VecB.Y, _CMP_LT_OQ);
        __mmask16 MaskZ = _mm512_cmp_ps_mask(VecA.Z, VecB.Z, _CMP_LT_OQ);
        __mmask16 MaskW = _mm512_cmp_ps_mask(VecA.W, VecB.W, _CMP_LT_OQ);
        __mmask16 ResultMask = _mm512_kand(
            _mm512_kand(MaskX, MaskY), 
            _mm512_kand(MaskZ, MaskW)
        );
        v512f Result = _mm512_mask_mov_ps(
            _mm512_setzero_ps(),
            ResultMask,
            _mm512_set1_ps(1.0f)
        );

        return Result;
    }

    INTERNAL v512f
    operator>(zv4f32 VecA, zv4f32 VecB) 
    {
        __mmask16 MaskX = _mm512_cmp_ps_mask(VecA.X, VecB.X, _CMP_GT_OQ);
        __mmask16 MaskY = _mm512_cmp_ps_mask(VecA.Y, VecB.Y, _CMP_GT_OQ);
        __mmask16 MaskZ = _mm512_cmp_ps_mask(VecA.Z, VecB.Z, _CMP_GT_OQ);
        __mmask16 MaskW = _mm512_cmp_ps_mask(VecA.W, VecB.W, _CMP_GT_OQ);
        __mmask16 ResultMask = _mm512_kand(
            _mm512_kand(MaskX, MaskY), 
            _mm512_kand(MaskZ, MaskW)
        );
        v512f Result = _mm512_mask_mov_ps(
            _mm512_setzero_ps(),
            ResultMask,
            _mm512_set1_ps(1.0f)
        );

        return Result;
    }

    INTERNAL v512f
    operator<=(zv4f32 VecA, zv4f32 VecB) 
    {
        __mmask16 MaskX = _mm512_cmp_ps_mask(VecA.X, VecB.X, _CMP_LE_OQ);
        __mmask16 MaskY = _mm512_cmp_ps_mask(VecA.Y, VecB.Y, _CMP_LE_OQ);
        __mmask16 MaskZ = _mm512_cmp_ps_mask(VecA.Z, VecB.Z, _CMP_LE_OQ);
        __mmask16 MaskW = _mm512_cmp_ps_mask(VecA.W, VecB.W, _CMP_LE_OQ);
        __mmask16 ResultMask = _mm512_kand(
            _mm512_kand(MaskX, MaskY), 
            _mm512_kand(MaskZ, MaskW)
        );
        v512f Result = _mm512_mask_mov_ps(
            _mm512_setzero_ps(),
            ResultMask,
            _mm512_set1_ps(1.0f)
        );

        return Result;
    }

    INTERNAL v512f
    operator>=(zv4f32 VecA, zv4f32 VecB) 
    {
        __mmask16 MaskX = _mm512_cmp_ps_mask(VecA.X, VecB.X, _CMP_GE_OQ);
        __mmask16 MaskY = _mm512_cmp_ps_mask(VecA.Y, VecB.Y, _CMP_GE_OQ);
        __mmask16 MaskZ = _mm512_cmp_ps_mask(VecA.Z, VecB.Z, _CMP_GE_OQ);
        __mmask16 MaskW = _mm512_cmp_ps_mask(VecA.W, VecB.W, _CMP_GE_OQ);
        __mmask16 ResultMask = _mm512_kand(
            _mm512_kand(MaskX, MaskY), 
            _mm512_kand(MaskZ, MaskW)
        );
        v512f Result = _mm512_mask_mov_ps(
            _mm512_setzero_ps(),
            ResultMask,
            _mm512_set1_ps(1.0f)
        );

        return Result;
    }

    INTERNAL v512f
    Dot(zv4f32 VecA, zv4f32 VecB)
    {
        v512f Result = {};

        Result = _mm512_fmadd_ps(
            VecA.X,
            VecB.X,
            _mm512_fmadd_ps(
                VecA.Y,
                VecB.Y,
                _mm512_fmadd_ps(
                    VecA.Z,
                    VecB.Z,
                    _mm512_mul_ps(VecA.W, VecB.W)
                )
            )
        );

        return Result;
    }

    INTERNAL v512f
    LengthSq(zv4f32 Vec)
    {
        return Dot(Vec, Vec);
    }

    INTERNAL v512f
    Length(zv4f32 Vec)
    {
        return _mm512_sqrt_ps(LengthSq(Vec));
    }

    INTERNAL zv4f32
    Norm(zv4f32 Vec)
    {
        v512f Scale = _mm512_rsqrt14_ps(LengthSq(Vec));

        return Vec * Scale;
    }

    INTERNAL zv4f32
    Lerp(zv4f32 VecA, zv4f32 VecB, v512f T)
    {
        zv4f32 Result = {};

        Result.X = _mm512_fmadd_ps(T, VecB.X, _mm512_fnmadd_ps(T, VecA.X, VecA.X));
        Result.Y = _mm512_fmadd_ps(T, VecB.Y, _mm512_fnmadd_ps(T, VecA.Y, VecA.Y));
        Result.Z = _mm512_fmadd_ps(T, VecB.Z, _mm512_fnmadd_ps(T, VecA.Z, VecA.Z));
        Result.W = _mm512_fmadd_ps(T, VecB.W, _mm512_fnmadd_ps(T, VecA.W, VecA.W));

        return Result;
    }

    INTERNAL zv4f32
    Lerp(zv4f32 VecA, zv4f32 VecB, f32 T)
    {
        v512f ZT = _mm512_set1_ps(T);

        return Lerp(VecA, VecB, ZT);
    }

    INTERNAL zv4i32 
    operator+(zv4i32 VecA, zv4i32 VecB) 
    {
        zv4i32 Result = {};

        Result.X = _mm512_add_epi32(VecA.X, VecB.X);
        Result.Y = _mm512_add_epi32(VecA.Y, VecB.Y);
        Result.Z = _mm512_add_epi32(VecA.Z, VecB.Z);
        Result.W = _mm512_add_epi32(VecA.W, VecB.W);

        return Result;
    }

    INTERNAL zv4i32 
    operator-(zv4i32 VecA, zv4i32 VecB) 
    {
        zv4i32 Result = {};

        Result.X = _mm512_sub_epi32(VecA.X, VecB.X);
        Result.Y = _mm512_sub_epi32(VecA.Y, VecB.Y);
        Result.Z = _mm512_sub_epi32(VecA.Z, VecB.Z);
        Result.W = _mm512_sub_epi32(VecA.W, VecB.W);

        return Result;
    }

    INTERNAL zv4i32 
    operator*(zv4i32 VecA, zv4i32 VecB) 
    {
        zv4i32 Result = {};

        Result.X = _mm512_mullo_epi32(VecA.X, VecB.X);
        Result.Y = _mm512_mullo_epi32(VecA.Y, VecB.Y);
        Result.Z = _mm512_mullo_epi32(VecA.Z, VecB.Z);
        Result.W = _mm512_mullo_epi32(VecA.W, VecB.W);

        return Result;
    }

    INTERNAL zv4i32 
    operator*(zv4i32 Vec, v512 Value) 
    {
        zv4i32 Result = {};

        Result.X = _mm512_mullo_epi32(Vec.X, Value);
        Result.Y = _mm512_mullo_epi32(Vec.Y, Value);
        Result.Z = _mm512_mullo_epi32(Vec.Z, Value);
        Result.W = _mm512_mullo_epi32(Vec.W, Value);

        return Result;
    }

    INTERNAL zv4i32 
    operator*(zv4i32 Vec, i32 Value) 
    {
        v512 ZValue = _mm512_set1_epi32(Value);

        return Vec * ZValue;
    }

    INTERNAL zv4i32 
    operator/(zv4i32 VecA, zv4i32 VecB) 
    {
        zv4i32 Result = {};
        v512f FloatAX = _mm512_cvtepi32_ps(VecA.X);
        v512f FloatAY = _mm512_cvtepi32_ps(VecA.Y);
        v512f FloatAZ = _mm512_cvtepi32_ps(VecA.Z);
        v512f FloatAW = _mm512_cvtepi32_ps(VecA.W);
        v512f FloatBX = _mm512_cvtepi32_ps(VecB.X);
        v512f FloatBY = _mm512_cvtepi32_ps(VecB.Y);
        v512f FloatBZ = _mm512_cvtepi32_ps(VecB.Z);
        v512f FloatBW = _mm512_cvtepi32_ps(VecB.W);
        v512f QuotientX = _mm512_div_ps(FloatAX, FloatBX);
        v512f QuotientY = _mm512_div_ps(FloatAY, FloatBY);
        v512f QuotientZ = _mm512_div_ps(FloatAZ, FloatBZ);
        v512f QuotientW = _mm512_div_ps(FloatAW, FloatBW);

        Result.X = _mm512_cvtps_epi32(QuotientX);
        Result.Y = _mm512_cvtps_epi32(QuotientY);
        Result.Z = _mm512_cvtps_epi32(QuotientZ);
        Result.W = _mm512_cvtps_epi32(QuotientW);

        return Result;
    }

    INTERNAL zv4i32 
    operator&(zv4i32 VecA, zv4i32 VecB) 
    {
        zv4i32 Result = {};

        Result.X = _mm512_and_epi32(VecA.X, VecB.X);
        Result.Y = _mm512_and_epi32(VecA.Y, VecB.Y);
        Result.Z = _mm512_and_epi32(VecA.Z, VecB.Z);
        Result.W = _mm512_and_epi32(VecA.W, VecB.W);

        return Result;
    }

    INTERNAL zv4i32 
    operator|(zv4i32 VecA, zv4i32 VecB) 
    {
        zv4i32 Result = {};

        Result.X = _mm512_or_epi32(VecA.X, VecB.X);
        Result.Y = _mm512_or_epi32(VecA.Y, VecB.Y);
        Result.Z = _mm512_or_epi32(VecA.Z, VecB.Z);
        Result.W = _mm512_or_epi32(VecA.W, VecB.W);

        return Result;
    }

    INTERNAL zv4i32 
    operator^(zv4i32 VecA, zv4i32 VecB) 
    {
        zv4i32 Result = {};

        Result.X = _mm512_xor_epi32(VecA.X, VecB.X);
        Result.Y = _mm512_xor_epi32(VecA.Y, VecB.Y);
        Result.Z = _mm512_xor_epi32(VecA.Z, VecB.Z);
        Result.W = _mm512_xor_epi32(VecA.W, VecB.W);

        return Result;
    }

    INTERNAL v512 
    operator==(zv4i32 VecA, zv4i32 VecB) 
    {
        __mmask16 MaskX = _mm512_cmp_epi32_mask(VecA.X, VecB.X, _MM_CMPINT_EQ);
        __mmask16 MaskY = _mm512_cmp_epi32_mask(VecA.Y, VecB.Y, _MM_CMPINT_EQ);
        __mmask16 MaskZ = _mm512_cmp_epi32_mask(VecA.Z, VecB.Z, _MM_CMPINT_EQ);
        __mmask16 MaskW = _mm512_cmp_epi32_mask(VecA.W, VecB.W, _MM_CMPINT_EQ);
        __mmask16 ResultMask = _mm512_kand(
            _mm512_kand(MaskX, MaskY), 
            _mm512_kand(MaskZ, MaskW)
        );
        v512 Result = _mm512_mask_mov_epi32(
            _mm512_setzero_epi32(),
            ResultMask,
            _mm512_set1_epi32(~0)
        );

        return Result;
    }

    INTERNAL v512 
    operator!=(zv4i32 VecA, zv4i32 VecB) 
    {
        __mmask16 MaskX = _mm512_cmp_epi32_mask(VecA.X, VecB.X, _MM_CMPINT_NE);
        __mmask16 MaskY = _mm512_cmp_epi32_mask(VecA.Y, VecB.Y, _MM_CMPINT_NE);
        __mmask16 MaskZ = _mm512_cmp_epi32_mask(VecA.Z, VecB.Z, _MM_CMPINT_NE);
        __mmask16 MaskW = _mm512_cmp_epi32_mask(VecA.W, VecB.W, _MM_CMPINT_NE);
        __mmask16 ResultMask = _mm512_kand(
            _mm512_kand(MaskX, MaskY), 
            _mm512_kand(MaskZ, MaskW)
        );
        v512 Result = _mm512_mask_mov_epi32(
            _mm512_setzero_epi32(),
            ResultMask,
            _mm512_set1_epi32(~0)
        );

        return Result;
    }

    INTERNAL v512 
    operator<(zv4i32 VecA, zv4i32 VecB) 
    {
        __mmask16 MaskX = _mm512_cmp_epi32_mask(VecA.X, VecB.X, _MM_CMPINT_LT);
        __mmask16 MaskY = _mm512_cmp_epi32_mask(VecA.Y, VecB.Y, _MM_CMPINT_LT);
        __mmask16 MaskZ = _mm512_cmp_epi32_mask(VecA.Z, VecB.Z, _MM_CMPINT_LT);
        __mmask16 MaskW = _mm512_cmp_epi32_mask(VecA.W, VecB.W, _MM_CMPINT_LT);
        __mmask16 ResultMask = _mm512_kand(
            _mm512_kand(MaskX, MaskY), 
            _mm512_kand(MaskZ, MaskW)
        );
        v512 Result = _mm512_mask_mov_epi32(
            _mm512_setzero_epi32(),
            ResultMask,
            _mm512_set1_epi32(~0)
        );

        return Result;
    }

    INTERNAL v512 
    operator>(zv4i32 VecA, zv4i32 VecB) 
    {
        __mmask16 MaskX = _mm512_cmp_epi32_mask(VecA.X, VecB.X, _MM_CMPINT_GT);
        __mmask16 MaskY = _mm512_cmp_epi32_mask(VecA.Y, VecB.Y, _MM_CMPINT_GT);
        __mmask16 MaskZ = _mm512_cmp_epi32_mask(VecA.Z, VecB.Z, _MM_CMPINT_GT);
        __mmask16 MaskW = _mm512_cmp_epi32_mask(VecA.W, VecB.W, _MM_CMPINT_GT);
        __mmask16 ResultMask = _mm512_kand(
            _mm512_kand(MaskX, MaskY), 
            _mm512_kand(MaskZ, MaskW)
        );
        v512 Result = _mm512_mask_mov_epi32(
            _mm512_setzero_epi32(),
            ResultMask,
            _mm512_set1_epi32(~0)
        );

        return Result;
    }

    INTERNAL v512 
    operator<=(zv4i32 VecA, zv4i32 VecB) 
    {
        __mmask16 MaskX = _mm512_cmp_epi32_mask(VecA.X, VecB.X, _MM_CMPINT_LE);
        __mmask16 MaskY = _mm512_cmp_epi32_mask(VecA.Y, VecB.Y, _MM_CMPINT_LE);
        __mmask16 MaskZ = _mm512_cmp_epi32_mask(VecA.Z, VecB.Z, _MM_CMPINT_LE);
        __mmask16 MaskW = _mm512_cmp_epi32_mask(VecA.W, VecB.W, _MM_CMPINT_LE);
        __mmask16 ResultMask = _mm512_kand(
            _mm512_kand(MaskX, MaskY), 
            _mm512_kand(MaskZ, MaskW)
        );
        v512 Result = _mm512_mask_mov_epi32(
            _mm512_setzero_epi32(),
            ResultMask,
            _mm512_set1_epi32(~0)
        );

        return Result;
    }

    INTERNAL v512 
    operator>=(zv4i32 VecA, zv4i32 VecB) 
    {
        __mmask16 MaskX = _mm512_cmp_epi32_mask(VecA.X, VecB.X, _MM_CMPINT_GE);
        __mmask16 MaskY = _mm512_cmp_epi32_mask(VecA.Y, VecB.Y, _MM_CMPINT_GE);
        __mmask16 MaskZ = _mm512_cmp_epi32_mask(VecA.Z, VecB.Z, _MM_CMPINT_GE);
        __mmask16 MaskW = _mm512_cmp_epi32_mask(VecA.W, VecB.W, _MM_CMPINT_GE);
        __mmask16 ResultMask = _mm512_kand(
            _mm512_kand(MaskX, MaskY), 
            _mm512_kand(MaskZ, MaskW)
        );
        v512 Result = _mm512_mask_mov_epi32(
            _mm512_setzero_epi32(),
            ResultMask,
            _mm512_set1_epi32(~0)
        );

        return Result;
    }

    INTERNAL v512
    Dot(zv4i32 VecA, zv4i32 VecB)
    {
        v512 Result = {};

        Result = _mm512_add_epi32(
            _mm512_add_epi32(
                _mm512_mullo_epi32(VecA.X, VecB.X),
                _mm512_mullo_epi32(VecA.Y, VecB.Y)
            ),
            _mm512_add_epi32(
                _mm512_mullo_epi32(VecA.Z, VecB.Z),
                _mm512_mullo_epi32(VecA.W, VecB.W)
            )
        );

        return Result;
    }

    INTERNAL v512
    LengthSq(zv4i32 Vec)
    {
        return Dot(Vec, Vec);
    }

    INTERNAL v512
    Length(zv4i32 Vec)
    {
        v512f LenSq = _mm512_cvtepi32_ps(LengthSq(Vec));
        
        return _mm512_cvtps_epi32(
            _mm512_sqrt_ps(LenSq)
        );
    }

    INTERNAL zv4i32
    Norm(zv4i32 Vec)
    {
        v512 Scale = _mm512_cvtps_epi32(
            _mm512_rsqrt14_ps(
                _mm512_cvtepi32_ps(
                    LengthSq(Vec)
                )
            )
        );

        return Vec * Scale;
    }

    INTERNAL zv4i32
    Lerp(zv4i32 VecA, zv4i32 VecB, v512 T)
    {
        zv4i32 Result = {};
        v512f ZT = _mm512_cvtepi32_ps(T);
        
        v512f ResultX = _mm512_fmadd_ps(
            ZT,
            _mm512_cvtepi32_ps(VecB.X),
            _mm512_fnmadd_ps(ZT, _mm512_cvtepi32_ps(VecA.X), _mm512_cvtepi32_ps(VecA.X))
        );
        v512f ResultY = _mm512_fmadd_ps(
            ZT,
            _mm512_cvtepi32_ps(VecB.Y),
            _mm512_fnmadd_ps(ZT, _mm512_cvtepi32_ps(VecA.Y), _mm512_cvtepi32_ps(VecA.Y))
        );
        v512f ResultZ = _mm512_fmadd_ps(
            ZT,
            _mm512_cvtepi32_ps(VecB.Z),
            _mm512_fnmadd_ps(ZT, _mm512_cvtepi32_ps(VecA.Z), _mm512_cvtepi32_ps(VecA.Z))
        );
        v512f ResultW = _mm512_fmadd_ps(
            ZT,
            _mm512_cvtepi32_ps(VecB.W),
            _mm512_fnmadd_ps(ZT, _mm512_cvtepi32_ps(VecA.W), _mm512_cvtepi32_ps(VecA.W))
        );

        Result.X = _mm512_cvtps_epi32(ResultX);
        Result.Y = _mm512_cvtps_epi32(ResultY);
        Result.Z = _mm512_cvtps_epi32(ResultZ);
        Result.W = _mm512_cvtps_epi32(ResultW);

        return Result;
    }

    INTERNAL zv4i32
    Lerp(zv4i32 VecA, zv4i32 VecB, i32 T)
    {
        v512 ZT = _mm512_set1_epi32(T);

        return Lerp(VecA, VecB, ZT);
    }

    INTERNAL zv4f64 
    operator+(zv4f64 VecA, zv4f64 VecB) 
    {
        zv4f64 Result = {};

        Result.X = _mm512_add_pd(VecA.X, VecB.X);
        Result.Y = _mm512_add_pd(VecA.Y, VecB.Y);
        Result.Z = _mm512_add_pd(VecA.Z, VecB.Z);
        Result.W = _mm512_add_pd(VecA.W, VecB.W);

        return Result;
    }

    INTERNAL zv4f64 
    operator-(zv4f64 VecA, zv4f64 VecB) 
    {
        zv4f64 Result = {};

        Result.X = _mm512_sub_pd(VecA.X, VecB.X);
        Result.Y = _mm512_sub_pd(VecA.Y, VecB.Y);
        Result.Z = _mm512_sub_pd(VecA.Z, VecB.Z);
        Result.W = _mm512_sub_pd(VecA.W, VecB.W);

        return Result;
    }

    INTERNAL zv4f64 
    operator*(zv4f64 VecA, zv4f64 VecB) 
    {
        zv4f64 Result = {};

        Result.X = _mm512_mul_pd(VecA.X, VecB.X);
        Result.Y = _mm512_mul_pd(VecA.Y, VecB.Y);
        Result.Z = _mm512_mul_pd(VecA.Z, VecB.Z);
        Result.W = _mm512_mul_pd(VecA.W, VecB.W);

        return Result;
    }

    INTERNAL zv4f64
    operator*(zv4f64 Vec, v512d Value) 
    {
        zv4f64 Result = {};

        Result.X = _mm512_mul_pd(Vec.X, Value);
        Result.Y = _mm512_mul_pd(Vec.Y, Value);
        Result.Z = _mm512_mul_pd(Vec.Z, Value);
        Result.W = _mm512_mul_pd(Vec.W, Value);

        return Result;
    }

    INTERNAL zv4f64
    operator*(zv4f64 Vec, f64 Value) 
    {
        v512d ZValue = _mm512_set1_pd(Value);

        return Vec * ZValue;
    }

    INTERNAL zv4f64 
    operator/(zv4f64 VecA, zv4f64 VecB) 
    {
        zv4f64 Result = {};

        Result.X = _mm512_div_pd(VecA.X, VecB.X);
        Result.Y = _mm512_div_pd(VecA.Y, VecB.Y);
        Result.Z = _mm512_div_pd(VecA.Z, VecB.Z);
        Result.W = _mm512_div_pd(VecA.W, VecB.W);

        return Result;
    }

    INTERNAL zv4f64 
    operator&(zv4f64 VecA, zv4f64 VecB) 
    {
        zv4f64 Result = {};

        Result.X = _mm512_and_pd(VecA.X, VecB.X);
        Result.Y = _mm512_and_pd(VecA.Y, VecB.Y);
        Result.Z = _mm512_and_pd(VecA.Z, VecB.Z);
        Result.W = _mm512_and_pd(VecA.W, VecB.W);

        return Result;
    }

    INTERNAL zv4f64 
    operator|(zv4f64 VecA, zv4f64 VecB) 
    {
        zv4f64 Result = {};

        Result.X = _mm512_or_pd(VecA.X, VecB.X);
        Result.Y = _mm512_or_pd(VecA.Y, VecB.Y);
        Result.Z = _mm512_or_pd(VecA.Z, VecB.Z);
        Result.W = _mm512_or_pd(VecA.W, VecB.W);

        return Result;
    }

    INTERNAL zv4f64 
    operator^(zv4f64 VecA, zv4f64 VecB) 
    {
        zv4f64 Result = {};

        Result.X = _mm512_xor_pd(VecA.X, VecB.X);
        Result.Y = _mm512_xor_pd(VecA.Y, VecB.Y);
        Result.Z = _mm512_xor_pd(VecA.Z, VecB.Z);
        Result.W = _mm512_xor_pd(VecA.W, VecB.W);

        return Result;
    }

    INTERNAL v512d 
    operator==(zv4f64 VecA, zv4f64 VecB) 
    {
        __mmask8 MaskX = _mm512_cmp_pd_mask(VecA.X, VecB.X, _CMP_EQ_OQ);
        __mmask8 MaskY = _mm512_cmp_pd_mask(VecA.Y, VecB.Y, _CMP_EQ_OQ);
        __mmask8 MaskZ = _mm512_cmp_pd_mask(VecA.Z, VecB.Z, _CMP_EQ_OQ);
        __mmask8 MaskW = _mm512_cmp_pd_mask(VecA.W, VecB.W, _CMP_EQ_OQ);
        __mmask8 ResultMask = _kand_mask8(
            _kand_mask8(MaskX, MaskY), 
            _kand_mask8(MaskZ, MaskW)
        );
        v512d Result = _mm512_mask_mov_pd(
            _mm512_setzero_pd(),
            ResultMask,
            _mm512_set1_pd(1.0)
        );

        return Result;
    }

    INTERNAL v512d 
    operator!=(zv4f64 VecA, zv4f64 VecB) 
    {
        __mmask8 MaskX = _mm512_cmp_pd_mask(VecA.X, VecB.X, _CMP_NEQ_OQ);
        __mmask8 MaskY = _mm512_cmp_pd_mask(VecA.Y, VecB.Y, _CMP_NEQ_OQ);
        __mmask8 MaskZ = _mm512_cmp_pd_mask(VecA.Z, VecB.Z, _CMP_NEQ_OQ);
        __mmask8 MaskW = _mm512_cmp_pd_mask(VecA.W, VecB.W, _CMP_NEQ_OQ);
        __mmask8 ResultMask = _kand_mask8(
            _kand_mask8(MaskX, MaskY), 
            _kand_mask8(MaskZ, MaskW)
        );
        v512d Result = _mm512_mask_mov_pd(
            _mm512_setzero_pd(),
            ResultMask,
            _mm512_set1_pd(1.0)
        );

        return Result;
    }

    INTERNAL v512d 
    operator<(zv4f64 VecA, zv4f64 VecB) 
    {
        __mmask8 MaskX = _mm512_cmp_pd_mask(VecA.X, VecB.X, _CMP_LT_OQ);
        __mmask8 MaskY = _mm512_cmp_pd_mask(VecA.Y, VecB.Y, _CMP_LT_OQ);
        __mmask8 MaskZ = _mm512_cmp_pd_mask(VecA.Z, VecB.Z, _CMP_LT_OQ);
        __mmask8 MaskW = _mm512_cmp_pd_mask(VecA.W, VecB.W, _CMP_LT_OQ);
        __mmask8 ResultMask = _kand_mask8(
            _kand_mask8(MaskX, MaskY), 
            _kand_mask8(MaskZ, MaskW)
        );
        v512d Result = _mm512_mask_mov_pd(
            _mm512_setzero_pd(),
            ResultMask,
            _mm512_set1_pd(1.0)
        );

        return Result;
    }

    INTERNAL v512d 
    operator>(zv4f64 VecA, zv4f64 VecB) 
    {
        __mmask8 MaskX = _mm512_cmp_pd_mask(VecA.X, VecB.X, _CMP_GT_OQ);
        __mmask8 MaskY = _mm512_cmp_pd_mask(VecA.Y, VecB.Y, _CMP_GT_OQ);
        __mmask8 MaskZ = _mm512_cmp_pd_mask(VecA.Z, VecB.Z, _CMP_GT_OQ);
        __mmask8 MaskW = _mm512_cmp_pd_mask(VecA.W, VecB.W, _CMP_GT_OQ);
        __mmask8 ResultMask = _kand_mask8(
            _kand_mask8(MaskX, MaskY), 
            _kand_mask8(MaskZ, MaskW)
        );
        v512d Result = _mm512_mask_mov_pd(
            _mm512_setzero_pd(),
            ResultMask,
            _mm512_set1_pd(1.0)
        );

        return Result;
    }

    INTERNAL v512d 
    operator<=(zv4f64 VecA, zv4f64 VecB) 
    {
        __mmask8 MaskX = _mm512_cmp_pd_mask(VecA.X, VecB.X, _CMP_LE_OQ);
        __mmask8 MaskY = _mm512_cmp_pd_mask(VecA.Y, VecB.Y, _CMP_LE_OQ);
        __mmask8 MaskZ = _mm512_cmp_pd_mask(VecA.Z, VecB.Z, _CMP_LE_OQ);
        __mmask8 MaskW = _mm512_cmp_pd_mask(VecA.W, VecB.W, _CMP_LE_OQ);
        __mmask8 ResultMask = _kand_mask8(
            _kand_mask8(MaskX, MaskY), 
            _kand_mask8(MaskZ, MaskW)
        );
        v512d Result = _mm512_mask_mov_pd(
            _mm512_setzero_pd(),
            ResultMask,
            _mm512_set1_pd(1.0)
        );

        return Result;
    }

    INTERNAL v512d 
    operator>=(zv4f64 VecA, zv4f64 VecB) 
    {
        __mmask8 MaskX = _mm512_cmp_pd_mask(VecA.X, VecB.X, _CMP_GE_OQ);
        __mmask8 MaskY = _mm512_cmp_pd_mask(VecA.Y, VecB.Y, _CMP_GE_OQ);
        __mmask8 MaskZ = _mm512_cmp_pd_mask(VecA.Z, VecB.Z, _CMP_GE_OQ);
        __mmask8 MaskW = _mm512_cmp_pd_mask(VecA.W, VecB.W, _CMP_GE_OQ);
        __mmask8 ResultMask = _kand_mask8(
            _kand_mask8(MaskX, MaskY), 
            _kand_mask8(MaskZ, MaskW)
        );
        v512d Result = _mm512_mask_mov_pd(
            _mm512_setzero_pd(),
            ResultMask,
            _mm512_set1_pd(1.0)
        );

        return Result;
    }

    INTERNAL v512d
    Dot(zv4f64 VecA, zv4f64 VecB)
    {
        v512d Result = {};

        Result = _mm512_fmadd_pd(
            VecA.X,
            VecB.X,
            _mm512_fmadd_pd(
                VecA.Y,
                VecB.Y,
                _mm512_fmadd_pd(
                    VecA.Z,
                    VecB.Z,
                    _mm512_mul_pd(VecA.W, VecB.W)
                )
            )
        );

        return Result;
    }

    INTERNAL v512d
    LengthSq(zv4f64 Vec)
    {
        return Dot(Vec, Vec);
    }

    INTERNAL v512d
    Length(zv4f64 Vec)
    {
        return _mm512_sqrt_pd(LengthSq(Vec));
    }

    INTERNAL zv4f64
    Norm(zv4f64 Vec)
    {
        v512d Scale = _mm512_rsqrt14_pd(LengthSq(Vec));

        return Vec * Scale;
    }

    INTERNAL zv4f64
    Lerp(zv4f64 VecA, zv4f64 VecB, v512d T)
    {
        zv4f64 Result = {};

        Result.X = _mm512_fmadd_pd(T, VecB.X, _mm512_fnmadd_pd(T, VecA.X, VecA.X));
        Result.Y = _mm512_fmadd_pd(T, VecB.Y, _mm512_fnmadd_pd(T, VecA.Y, VecA.Y));
        Result.Z = _mm512_fmadd_pd(T, VecB.Z, _mm512_fnmadd_pd(T, VecA.Z, VecA.Z));
        Result.W = _mm512_fmadd_pd(T, VecB.W, _mm512_fnmadd_pd(T, VecA.W, VecA.W));

        return Result;
    }

    INTERNAL zv4f64
    Lerp(zv4f64 VecA, zv4f64 VecB, f64 T)
    {
        v512d ZT = _mm512_set1_pd(T);

        return Lerp(VecA, VecB, ZT);
    }

    INTERNAL zr1i32
    Rng(v512 Min, v512 Max)
    {
        zr1i32 Result = {};

        Result.Min = Min;
        Result.Max = Max;

        return Result;
    }

    INTERNAL zr1i32
    Shift(zr1i32 Range, v512 Val)
    {
        zr1i32 Result = {};

        Result.Min = _mm512_add_epi32(Range.Min, Val);
        Result.Max = _mm512_add_epi32(Range.Max, Val);

        return Result;
    }

    INTERNAL zr1i32
    Widen(zr1i32 Range, v512 Val)
    {
        zr1i32 Result = {};

        Result.Min = _mm512_sub_epi32(Range.Min, Val);
        Result.Max = _mm512_add_epi32(Range.Max, Val);

        return Result;
    }

    INTERNAL v512
    Centre(zr1i32 Range)
    {
        v512f Float = _mm512_cvtepi32_ps(
            _mm512_add_epi32(Range.Min, Range.Max)
        );
        v512f Quotient = _mm512_div_ps(Float, _mm512_set1_ps(2.0f));

        return _mm512_cvtps_epi32(Quotient);
    }

    INTERNAL v512
    InRange(zr1i32 Range, v512 Val)
    {
        __mmask16 MinMask = _mm512_cmp_epi32_mask(Range.Min, Val, _MM_CMPINT_GE);
        __mmask16 MaxMask = _mm512_cmp_epi32_mask(Val, Range.Max, _MM_CMPINT_LT);
        __mmask16 ResultMask = _mm512_kand(MinMask, MaxMask);

        return _mm512_mask_mov_epi32(
            _mm512_setzero_epi32(),
            ResultMask,
            _mm512_set1_epi32(1)
        );
    }

    INTERNAL v512
    Length(zr1i32 Range)
    {
        return _mm512_sub_epi32(Range.Max, Range.Min);
    }

    INTERNAL zr1i32
    Union(zr1i32 RangeA, zr1i32 RangeB)
    {
        zr1i32 Result = {};

        Result.Min = _mm512_min_epi32(RangeA.Min, RangeB.Min);
        Result.Max = _mm512_max_epi32(RangeA.Max, RangeB.Max);

        return Result;
    }

    INTERNAL zr1i32
    Intersect(zr1i32 RangeA, zr1i32 RangeB)
    {
        zr1i32 Result = {};

        Result.Min = _mm512_max_epi32(RangeA.Min, RangeB.Min);
        Result.Max = _mm512_min_epi32(RangeA.Max, RangeB.Max);

        return Result;
    }

    INTERNAL v512
    Clamp(zr1i32 Range, v512 Val)
    {
        Val = _mm512_max_epi32(Range.Min, Val);
        Val = _mm512_min_epi32(Range.Max, Val);

        return Val;
    }

    INTERNAL zr1f32
    Rng(v512f Min, v512f Max)
    {
        zr1f32 Result = {};

        Result.Min = Min;
        Result.Max = Max;

        return Result;
    }

    INTERNAL zr1f32
    Shift(zr1f32 Range, v512f Val)
    {
        zr1f32 Result = {};

        Result.Min = _mm512_add_ps(Range.Min, Val);
        Result.Max = _mm512_add_ps(Range.Max, Val);

        return Result;
    }

    INTERNAL zr1f32
    Widen(zr1f32 Range, v512f Val)
    {
        zr1f32 Result = {};

        Result.Min = _mm512_sub_ps(Range.Min, Val);
        Result.Max = _mm512_add_ps(Range.Max, Val);

        return Result;
    }

    INTERNAL v512f
    Centre(zr1f32 Range)
    {
        return _mm512_mul_ps(
            _mm512_add_ps(Range.Min, Range.Max), 
            _mm512_set1_ps(0.5f)
        );
    }

    INTERNAL v512f
    InRange(zr1f32 Range, v512f Val)
    {
        __mmask16 MinMask = _mm512_cmp_ps_mask(Range.Min, Val, _CMP_GE_OQ);
        __mmask16 MaxMask = _mm512_cmp_ps_mask(Val, Range.Max, _CMP_LT_OQ);
        __mmask16 ResultMask = _mm512_kand(MinMask, MaxMask);

        return _mm512_mask_mov_ps(
            _mm512_setzero_ps(),
            ResultMask,
            _mm512_set1_ps(1.0f)
        );
    }

    INTERNAL v512f
    Length(zr1f32 Range)
    {
        return _mm512_sub_ps(Range.Max, Range.Min);
    }

    INTERNAL zr1f32
    Union(zr1f32 RangeA, zr1f32 RangeB)
    {
        zr1f32 Result = {};

        Result.Min = _mm512_min_ps(RangeA.Min, RangeB.Min);
        Result.Max = _mm512_max_ps(RangeA.Max, RangeB.Max);

        return Result;
    }

    INTERNAL zr1f32
    Intersect(zr1f32 RangeA, zr1f32 RangeB)
    {
        zr1f32 Result = {};

        Result.Min = _mm512_max_ps(RangeA.Min, RangeB.Min);
        Result.Max = _mm512_min_ps(RangeA.Max, RangeB.Max);

        return Result;
    }

    INTERNAL v512f
    Clamp(zr1f32 Range, v512f Val)
    {
        Val = _mm512_max_ps(Range.Min, Val);
        Val = _mm512_min_ps(Range.Max, Val);

        return Val;
    }

    INTERNAL zr1f64
    Rng(v512d Min, v512d Max)
    {
        zr1f64 Result = {};

        Result.Min = Min;
        Result.Max = Max;

        return Result;
    }

    INTERNAL zr1f64
    Shift(zr1f64 Range, v512d Val)
    {
        zr1f64 Result = {};

        Result.Min = _mm512_add_pd(Range.Min, Val);
        Result.Max = _mm512_add_pd(Range.Max, Val);

        return Result;
    }

    INTERNAL zr1f64
    Widen(zr1f64 Range, v512d Val)
    {
        zr1f64 Result = {};

        Result.Min = _mm512_sub_pd(Range.Min, Val);
        Result.Max = _mm512_add_pd(Range.Max, Val);

        return Result;
    }

    INTERNAL v512d
    Centre(zr1f64 Range)
    {
        return _mm512_mul_pd(
            _mm512_add_pd(Range.Min, Range.Max), 
            _mm512_set1_pd(0.5)
        );
    }

    INTERNAL v512d
    InRange(zr1f64 Range, v512d Val)
    {
        __mmask8 MinMask = _mm512_cmp_pd_mask(Range.Min, Val, _CMP_GE_OQ);
        __mmask8 MaxMask = _mm512_cmp_pd_mask(Val, Range.Max, _CMP_LT_OQ);
        __mmask8 ResultMask = _kand_mask8(MinMask, MaxMask);

        return _mm512_mask_mov_pd(
            _mm512_setzero_pd(),
            ResultMask,
            _mm512_set1_pd(1.0)
        );
    }

    INTERNAL v512d
    Length(zr1f64 Range)
    {
        return _mm512_sub_pd(Range.Max, Range.Min);
    }

    INTERNAL zr1f64
    Union(zr1f64 RangeA, zr1f64 RangeB)
    {
        zr1f64 Result = {};

        Result.Min = _mm512_min_pd(RangeA.Min, RangeB.Min);
        Result.Max = _mm512_max_pd(RangeA.Max, RangeB.Max);

        return Result;
    }

    INTERNAL zr1f64
    Intersect(zr1f64 RangeA, zr1f64 RangeB)
    {
        zr1f64 Result = {};

        Result.Min = _mm512_max_pd(RangeA.Min, RangeB.Min);
        Result.Max = _mm512_min_pd(RangeA.Max, RangeB.Max);

        return Result;
    }

    INTERNAL v512d
    Clamp(zr1f64 Range, v512d Val)
    {
        Val = _mm512_max_pd(Range.Min, Val);
        Val = _mm512_min_pd(Range.Max, Val);

        return Val;
    }

    INTERNAL zr2f32
    Rng(zv2f32 Min, zv2f32 Max)
    {
        zr2f32 Result = {};

        Result.Min = Min;
        Result.Max = Max;

        return Result;
    }

    INTERNAL zr2f32
    Shift(zr2f32 Range, zv2f32 Val)
    {
        zr2f32 Result = {};

        Result.Min = Range.Min + Val;
        Result.Max = Range.Max + Val;

        return Result;
    }

    INTERNAL zr2f32
    Widen(zr2f32 Range, zv2f32 Val)
    {
        zr2f32 Result = {};

        Result.Min = Range.Min - Val;
        Result.Max = Range.Max + Val;

        return Result;
    }

    INTERNAL zv2f32
    Centre(zr2f32 Range)
    {
        return (Range.Min + Range.Max) / 2.0f;
    }

    INTERNAL v512f
    InRange(zr2f32 Range, zv2f32 Val)
    {
        return _mm512_and_ps(
            Val >= Range.Min,
            Val < Range.Max
        );
    }

    INTERNAL zv2f32
    Length(zr2f32 Range)
    {
        return Range.Max - Range.Min;
    }

    INTERNAL zr2f32
    Union(zr2f32 RangeA, zr2f32 RangeB)
    {
        zr2f32 Result = {};

        Result.Min.X = _mm512_min_ps(RangeA.Min.X, RangeB.Min.X);
        Result.Min.Y = _mm512_min_ps(RangeA.Min.Y, RangeB.Min.Y);
        Result.Max.X = _mm512_max_ps(RangeA.Max.X, RangeB.Max.X);
        Result.Max.Y = _mm512_max_ps(RangeA.Max.Y, RangeB.Max.Y);

        return Result;
    }

    INTERNAL zr2f32
    Intersect(zr2f32 RangeA, zr2f32 RangeB)
    {
        zr2f32 Result = {};

        Result.Min.X = _mm512_max_ps(RangeA.Min.X, RangeB.Min.X);
        Result.Min.Y = _mm512_max_ps(RangeA.Min.Y, RangeB.Min.Y);
        Result.Max.X = _mm512_min_ps(RangeA.Max.X, RangeB.Max.X);
        Result.Max.Y = _mm512_min_ps(RangeA.Max.Y, RangeB.Max.Y);

        return Result;
    }

    INTERNAL zv2f32
    Clamp(zr2f32 Range, zv2f32 Val)
    {
        Val.X = _mm512_max_ps(Range.Min.X, Val.X);
        Val.Y = _mm512_max_ps(Range.Min.Y, Val.Y);
        Val.X = _mm512_min_ps(Range.Max.X, Val.X);
        Val.Y = _mm512_min_ps(Range.Max.Y, Val.Y);

        return Val;
    }

    INTERNAL zr2f64
    Rng(zv2f64 Min, zv2f64 Max)
    {
        zr2f64 Result = {};

        Result.Min = Min;
        Result.Max = Max;

        return Result;
    }

    INTERNAL zr2f64
    Shift(zr2f64 Range, zv2f64 Val)
    {
        zr2f64 Result = {};

        Result.Min = Range.Min + Val;
        Result.Max = Range.Max + Val;

        return Result;
    }

    INTERNAL zr2f64
    Widen(zr2f64 Range, zv2f64 Val)
    {
        zr2f64 Result = {};

        Result.Min = Range.Min - Val;
        Result.Max = Range.Max + Val;

        return Result;
    }

    INTERNAL zv2f64
    Centre(zr2f64 Range)
    {
        return (Range.Min + Range.Max) / 2.0;
    }

    INTERNAL v512d
    InRange(zr2f64 Range, zv2f64 Val)
    {
        return _mm512_and_pd(
            Val >= Range.Min,
            Val < Range.Max
        );
    }

    INTERNAL zv2f64
    Length(zr2f64 Range)
    {
        return Range.Max - Range.Min;
    }

    INTERNAL zr2f64
    Union(zr2f64 RangeA, zr2f64 RangeB)
    {
        zr2f64 Result = {};

        Result.Min.X = _mm512_min_pd(RangeA.Min.X, RangeB.Min.X);
        Result.Min.Y = _mm512_min_pd(RangeA.Min.Y, RangeB.Min.Y);
        Result.Max.X = _mm512_max_pd(RangeA.Max.X, RangeB.Max.X);
        Result.Max.Y = _mm512_max_pd(RangeA.Max.Y, RangeB.Max.Y);

        return Result;
    }

    INTERNAL zr2f64
    Intersect(zr2f64 RangeA, zr2f64 RangeB)
    {
        zr2f64 Result = {};

        Result.Min.X = _mm512_max_pd(RangeA.Min.X, RangeB.Min.X);
        Result.Min.Y = _mm512_max_pd(RangeA.Min.Y, RangeB.Min.Y);
        Result.Max.X = _mm512_min_pd(RangeA.Max.X, RangeB.Max.X);
        Result.Max.Y = _mm512_min_pd(RangeA.Max.Y, RangeB.Max.Y);

        return Result;
    }

    INTERNAL zv2f64
    Clamp(zr2f64 Range, zv2f64 Val)
    {
        Val.X = _mm512_max_pd(Range.Min.X, Val.X);
        Val.Y = _mm512_max_pd(Range.Min.Y, Val.Y);
        Val.X = _mm512_min_pd(Range.Max.X, Val.X);
        Val.Y = _mm512_min_pd(Range.Max.Y, Val.Y);

        return Val;
    }

    INTERNAL zr3f64
    Rng(zv3f64 Min, zv3f64 Max)
    {
        zr3f64 Result = {};

        Result.Min = Min;
        Result.Max = Max;

        return Result;
    }

    INTERNAL zr3f64
    Shift(zr3f64 Range, zv3f64 Val)
    {
        zr3f64 Result = {};

        Result.Min = Range.Min + Val;
        Result.Max = Range.Max + Val;

        return Result;
    }

    INTERNAL zr3f64
    Widen(zr3f64 Range, zv3f64 Val)
    {
        zr3f64 Result = {};

        Result.Min = Range.Min - Val;
        Result.Max = Range.Max + Val;

        return Result;
    }

    INTERNAL zv3f64
    Centre(zr3f64 Range)
    {
        return (Range.Min + Range.Max) / 2.0;
    }

    INTERNAL v512d
    InRange(zr3f64 Range, zv3f64 Val)
    {
        return _mm512_and_pd(
            Val >= Range.Min,
            Val < Range.Max
        );
    }

    INTERNAL zv3f64
    Length(zr3f64 Range)
    {
        return Range.Max - Range.Min;
    }

    INTERNAL zr3f64
    Union(zr3f64 RangeA, zr3f64 RangeB)
    {
        zr3f64 Result = {};

        Result.Min.X = _mm512_min_pd(RangeA.Min.X, RangeB.Min.X);
        Result.Min.Y = _mm512_min_pd(RangeA.Min.Y, RangeB.Min.Y);
        Result.Min.Z = _mm512_min_pd(RangeA.Min.Z, RangeB.Min.Z);
        Result.Max.X = _mm512_max_pd(RangeA.Max.X, RangeB.Max.X);
        Result.Max.Y = _mm512_max_pd(RangeA.Max.Y, RangeB.Max.Y);
        Result.Max.Z = _mm512_max_pd(RangeA.Max.Z, RangeB.Max.Z);

        return Result;
    }

    INTERNAL zr3f64
    Intersect(zr3f64 RangeA, zr3f64 RangeB)
    {
        zr3f64 Result = {};

        Result.Min.X = _mm512_max_pd(RangeA.Min.X, RangeB.Min.X);
        Result.Min.Y = _mm512_max_pd(RangeA.Min.Y, RangeB.Min.Y);
        Result.Min.Z = _mm512_max_pd(RangeA.Min.Z, RangeB.Min.Z);
        Result.Max.X = _mm512_min_pd(RangeA.Max.X, RangeB.Max.X);
        Result.Max.Y = _mm512_min_pd(RangeA.Max.Y, RangeB.Max.Y);
        Result.Max.Z = _mm512_min_pd(RangeA.Max.Z, RangeB.Max.Z);

        return Result;
    }

    INTERNAL zv3f64
    Clamp(zr3f64 Range, zv3f64 Val)
    {
        Val.X = _mm512_max_pd(Range.Min.X, Val.X);
        Val.Y = _mm512_max_pd(Range.Min.Y, Val.Y);
        Val.Z = _mm512_max_pd(Range.Min.Z, Val.Z);
        Val.X = _mm512_min_pd(Range.Max.X, Val.X);
        Val.Y = _mm512_min_pd(Range.Max.Y, Val.Y);
        Val.Z = _mm512_min_pd(Range.Max.Z, Val.Z);

        return Val;
    }

    INTERNAL zr3f32
    Rng(zv3f32 Min, zv3f32 Max)
    {
        zr3f32 Result = {};

        Result.Min = Min;
        Result.Max = Max;

        return Result;
    }

    INTERNAL zr3f32
    Shift(zr3f32 Range, zv3f32 Val)
    {
        zr3f32 Result = {};

        Result.Min = Range.Min + Val;
        Result.Max = Range.Max + Val;

        return Result;
    }

    INTERNAL zr3f32
    Widen(zr3f32 Range, zv3f32 Val)
    {
        zr3f32 Result = {};

        Result.Min = Range.Min - Val;
        Result.Max = Range.Max + Val;

        return Result;
    }

    INTERNAL zv3f32
    Centre(zr3f32 Range)
    {
        return (Range.Min + Range.Max) / 2.0f;
    }

    INTERNAL v512f
    InRange(zr3f32 Range, zv3f32 Val)
    {
        return _mm512_and_ps(
            Val >= Range.Min,
            Val < Range.Max
        );
    }

    INTERNAL zv3f32
    Length(zr3f32 Range)
    {
        return Range.Max - Range.Min;
    }

    INTERNAL zr3f32
    Union(zr3f32 RangeA, zr3f32 RangeB)
    {
        zr3f32 Result = {};

        Result.Min.X = _mm512_min_ps(RangeA.Min.X, RangeB.Min.X);
        Result.Min.Y = _mm512_min_ps(RangeA.Min.Y, RangeB.Min.Y);
        Result.Min.Z = _mm512_min_ps(RangeA.Min.Z, RangeB.Min.Z);
        Result.Max.X = _mm512_max_ps(RangeA.Max.X, RangeB.Max.X);
        Result.Max.Y = _mm512_max_ps(RangeA.Max.Y, RangeB.Max.Y);
        Result.Max.Z = _mm512_max_ps(RangeA.Max.Z, RangeB.Max.Z);

        return Result;
    }

    INTERNAL zr3f32
    Intersect(zr3f32 RangeA, zr3f32 RangeB)
    {
        zr3f32 Result = {};

        Result.Min.X = _mm512_max_ps(RangeA.Min.X, RangeB.Min.X);
        Result.Min.Y = _mm512_max_ps(RangeA.Min.Y, RangeB.Min.Y);
        Result.Min.Z = _mm512_max_ps(RangeA.Min.Z, RangeB.Min.Z);
        Result.Max.X = _mm512_min_ps(RangeA.Max.X, RangeB.Max.X);
        Result.Max.Y = _mm512_min_ps(RangeA.Max.Y, RangeB.Max.Y);
        Result.Max.Z = _mm512_min_ps(RangeA.Max.Z, RangeB.Max.Z);

        return Result;
    }

    INTERNAL zv3f32
    Clamp(zr3f32 Range, zv3f32 Val)
    {
        Val.X = _mm512_max_ps(Range.Min.X, Val.X);
        Val.Y = _mm512_max_ps(Range.Min.Y, Val.Y);
        Val.Z = _mm512_max_ps(Range.Min.Z, Val.Z);
        Val.X = _mm512_min_ps(Range.Max.X, Val.X);
        Val.Y = _mm512_min_ps(Range.Max.Y, Val.Y);
        Val.Z = _mm512_min_ps(Range.Max.Z, Val.Z);

        return Val;
    }
#elif NC_SIMD_LEVEL >= 6
    union zv2f32 { 
        struct { 
            v256f X;
            v256f Y; 
        };

        v256f V[2];
    };

    union zv2i32 {
        struct {
            v256 X;
            v256 Y; 
        };

        v256 V[2];
    };

    union zv2f64 {
        struct {
            v256d X;
            v256d Y; 
        };

        v256d V[2];
    };

    union zv3f32 {
        struct {
            v256f X;
            v256f Y;
            v256f Z;
        };

        v256f V[3];
    };

    union zv3i32 {
        struct {
            v256 X;
            v256 Y;
            v256 Z;
        };

        v256  V[3];
    };

    union zv3f64 {
        struct {
            v256d X;
            v256d Y;
            v256d Z;
        };

        v256d V[3];
    };

    union zv4f32 {
        struct {
            v256f X;
            v256f Y;
            v256f Z;
            v256f W;
        };

        v256f V[4];
    };

    union zv4i32 {
        struct {
            v256 X;
            v256 Y;
            v256 Z;
            v256 W;
        };

        v256  V[4];
    };

    union zv4f64 {
        struct {
            v256d X;
            v256d Y; 
            v256d Z;
            v256d W;
        };

        v256d V[4];
    };

    union zr1i32 {
        struct {
            v256 Min;
            v256 Max;
        };

        v256  V[2];
    };

    union zr1f32 {
        struct {
            v256f Min;
            v256f Max;
        };

        v256f V[2];
    };

    union zr1f64 {
        struct {
            v256d Min;
            v256d Max;
        };

        v256d V[2];
    };

    union zr2i32 {
        struct {
            zv2i32 Min;
            zv2i32 Max;
        };

        struct {
            zv2i32 Point0;
            zv2i32 Point1;
        };

        zv2i32 V[2];
    };

    union zr2f32 {
        struct {
            zv2f32 Min;
            zv2f32 Max;
        };

        struct {
            zv2f32 Point0;
            zv2f32 Point1;
        };

        zv2f32 V[2];
    };

    union zr2f64 {
        struct {
            zv2f64 Min;
            zv2f64 Max;
        };

        struct {
            zv2f64 Point0;
            zv2f64 Point1;
        };

        zv2f64 V[2];
    };


    union zr3f32 {
        struct {
            zv3f32 Min;
            zv3f32 Max;
        };

        struct {
            zv3f32 Point0;
            zv3f32 Point1;
        };

        zv3f32 V[2];
    };

    union zr3i32 {
        struct {
            zv3i32 Min;
            zv3i32 Max;
        };

        struct {
            zv3i32 Point0;
            zv3i32 Point1;
        };

        zv3i32 V[2];
    };

    union zr3f64 {
        struct {
            zv3f64 Min;
            zv3f64 Max;
        };

        struct {
            zv3f64 Point0;
            zv3f64 Point1;
        };

        zv3f64 V[2];
    };

    INTERNAL zv2f32 
    operator+(zv2f32 VecA, zv2f32 VecB) 
    {
        zv2f32 Result = {};

        Result.X = _mm256_add_ps(VecA.X, VecB.X);
        Result.Y = _mm256_add_ps(VecA.Y, VecB.Y);

        return Result;
    }

    INTERNAL zv2f32 
    operator-(zv2f32 VecA, zv2f32 VecB) 
    {
        zv2f32 Result = {};

        Result.X = _mm256_sub_ps(VecA.X, VecB.X);
        Result.Y = _mm256_sub_ps(VecA.Y, VecB.Y);

        return Result;
    }

    INTERNAL zv2f32 
    operator*(zv2f32 VecA, zv2f32 VecB) 
    {
        zv2f32 Result = {};

        Result.X = _mm256_mul_ps(VecA.X, VecB.X);
        Result.Y = _mm256_mul_ps(VecA.Y, VecB.Y);

        return Result;
    }

    INTERNAL zv2f32 
    operator*(zv2f32 Vec, v256f Value) 
    {
        zv2f32 Result = {};

        Result.X = _mm256_mul_ps(Vec.X, Value);
        Result.Y = _mm256_mul_ps(Vec.Y, Value);

        return Result;
    }

    INTERNAL zv2f32 
    operator*(zv2f32 Vec, f32 Value) 
    {
        zv2f32 Result = {};
        v256f Scale = _mm256_set1_ps(Value);

        Result.X = _mm256_mul_ps(Vec.X, Scale);
        Result.Y = _mm256_mul_ps(Vec.Y, Scale);

        return Result;
    }

    INTERNAL zv2f32 
    operator/(zv2f32 VecA, zv2f32 VecB) 
    {
        zv2f32 Result = {};

        Result.X = _mm256_div_ps(VecA.X, VecB.X);
        Result.Y = _mm256_div_ps(VecA.Y, VecB.Y);

        return Result;
    }

    INTERNAL zv2f32 
    operator/(zv2f32 Vec, v256f Value) 
    {
        zv2f32 Result = {};

        Result.X = _mm256_div_ps(Vec.X, Value);
        Result.Y = _mm256_div_ps(Vec.Y, Value);

        return Result;
    }

    INTERNAL zv2f32 
    operator/(zv2f32 Vec, f32 Value) 
    {
        v256f ZValue = _mm256_set1_ps(Value);

        return Vec / ZValue;
    }

    INTERNAL zv2f32 
    operator&(zv2f32 VecA, zv2f32 VecB) 
    {
        zv2f32 Result = {};

        Result.X = _mm256_and_ps(VecA.X, VecB.X);
        Result.Y = _mm256_and_ps(VecA.Y, VecB.Y);

        return Result;
    }

    INTERNAL zv2f32 
    operator|(zv2f32 VecA, zv2f32 VecB) 
    {
        zv2f32 Result = {};

        Result.X = _mm256_or_ps(VecA.X, VecB.X);
        Result.Y = _mm256_or_ps(VecA.Y, VecB.Y);

        return Result;
    }

    INTERNAL zv2f32 
    operator^(zv2f32 VecA, zv2f32 VecB) 
    {
        zv2f32 Result = {};

        Result.X = _mm256_xor_ps(VecA.X, VecB.X);
        Result.Y = _mm256_xor_ps(VecA.Y, VecB.Y);

        return Result;
    }

    INTERNAL v256f
    operator==(zv2f32 VecA, zv2f32 VecB) 
    {
        __mmask8 MaskX = _mm256_cmp_ps_mask(VecA.X, VecB.X, _CMP_EQ_OQ);
        __mmask8 MaskY = _mm256_cmp_ps_mask(VecA.Y, VecB.Y, _CMP_EQ_OQ);
        __mmask8 ResultMask = _kand_mask8(MaskX, MaskY);
        v256f Result = _mm256_mask_mov_ps(
            _mm256_setzero_ps(),
            ResultMask,
            _mm256_set1_ps(1.0f)
        );

        return Result;
    }

    INTERNAL v256f 
    operator!=(zv2f32 VecA, zv2f32 VecB) 
    {
        __mmask8 MaskX = _mm256_cmp_ps_mask(VecA.X, VecB.X, _CMP_NEQ_OQ);
        __mmask8 MaskY = _mm256_cmp_ps_mask(VecA.Y, VecB.Y, _CMP_NEQ_OQ);
        __mmask8 ResultMask = _kand_mask8(MaskX, MaskY);
        v256f Result = _mm256_mask_mov_ps(
            _mm256_setzero_ps(),
            ResultMask,
            _mm256_set1_ps(1.0f)
        );

        return Result;
    }

    INTERNAL v256f
    operator<(zv2f32 VecA, zv2f32 VecB) 
    {
        __mmask8 MaskX = _mm256_cmp_ps_mask(VecA.X, VecB.X, _CMP_LT_OQ);
        __mmask8 MaskY = _mm256_cmp_ps_mask(VecA.Y, VecB.Y, _CMP_LT_OQ);
        __mmask8 ResultMask = _kand_mask8(MaskX, MaskY);
        v256f Result = _mm256_mask_mov_ps(
            _mm256_setzero_ps(),
            ResultMask,
            _mm256_set1_ps(1.0f)
        );

        return Result;
    }

    INTERNAL v256f
    operator>(zv2f32 VecA, zv2f32 VecB) 
    {
        __mmask8 MaskX = _mm256_cmp_ps_mask(VecA.X, VecB.X, _CMP_GT_OQ);
        __mmask8 MaskY = _mm256_cmp_ps_mask(VecA.Y, VecB.Y, _CMP_GT_OQ);
        __mmask8 ResultMask = _kand_mask8(MaskX, MaskY);
        v256f Result = _mm256_mask_mov_ps(
            _mm256_setzero_ps(),
            ResultMask,
            _mm256_set1_ps(1.0f)
        );

        return Result;
    }

    INTERNAL v256f
    operator<=(zv2f32 VecA, zv2f32 VecB) 
    {
        __mmask8 MaskX = _mm256_cmp_ps_mask(VecA.X, VecB.X, _CMP_LE_OQ);
        __mmask8 MaskY = _mm256_cmp_ps_mask(VecA.Y, VecB.Y, _CMP_LE_OQ);
        __mmask8 ResultMask = _kand_mask8(MaskX, MaskY);
        v256f Result = _mm256_mask_mov_ps(
            _mm256_setzero_ps(),
            ResultMask,
            _mm256_set1_ps(1.0f)
        );

        return Result;
    }

    INTERNAL v256f
    operator>=(zv2f32 VecA, zv2f32 VecB) 
    {
        __mmask8 MaskX = _mm256_cmp_ps_mask(VecA.X, VecB.X, _CMP_GE_OQ);
        __mmask8 MaskY = _mm256_cmp_ps_mask(VecA.Y, VecB.Y, _CMP_GE_OQ);
        __mmask8 ResultMask = _kand_mask8(MaskX, MaskY);
        v256f Result = _mm256_mask_mov_ps(
            _mm256_set1_ps(0.0f),
            ResultMask,
            _mm256_set1_ps(1.0f)
        );

        return Result;
    }

    INTERNAL zv2f32 
    operator%(zv2f32 VecA, zv2f32 VecB) 
    {
        zv2f32 Result = {};
        v256f DivX = _mm256_div_ps(VecA.X, VecB.X);
        v256f DivY = _mm256_div_ps(VecA.Y, VecB.Y);
        v256f FloorX = _mm256_roundscale_ps(DivX, _MM_FROUND_FLOOR);                // TODO: compare against _mm256_floor_ps and 
        v256f FloorY = _mm256_roundscale_ps(DivY, _MM_FROUND_FLOOR);                //     : _mm256_roundscale_round_ps

        Result.X = _mm256_fnmadd_ps(FloorX, VecB.X, VecA.X);
        Result.Y = _mm256_fnmadd_ps(FloorY, VecB.Y, VecA.Y);

        return Result;
    }

    INTERNAL v256f
    Dot(zv2f32 VecA, zv2f32 VecB)
    {
        v256f Result = {};

        Result = _mm256_fmadd_ps(
            VecA.X,
            VecB.X,
            _mm256_mul_ps(VecA.Y, VecB.Y)
        );

        return Result;
    }

    INTERNAL v256f
    LengthSq(zv2f32 Vec)
    {
        return Dot(Vec, Vec);
    }

    INTERNAL v256f
    Length(zv2f32 Vec)
    {
        return _mm256_sqrt_ps(LengthSq(Vec));
    }

    INTERNAL zv2f32
    Norm(zv2f32 Vec)
    {
        v256f Scale = _mm256_rsqrt14_ps(LengthSq(Vec));

        return Vec * Scale;
    }

    INTERNAL zv2f32
    Lerp(zv2f32 VecA, zv2f32 VecB, v256f T)
    {
        zv2f32 Result = {};

        Result.X = _mm256_fmadd_ps(
            T,
            VecB.X,
            _mm256_fnmadd_ps(T, VecA.X, VecA.X)
        );
        Result.Y = _mm256_fmadd_ps(
            T,
            VecB.Y,
            _mm256_fnmadd_ps(T, VecA.Y, VecA.Y)
        );

        return Result;
    }

    INTERNAL zv2f32
    Lerp(zv2f32 VecA, zv2f32 VecB, f32 T)
    {
        v256f ZT = _mm256_set1_ps(T);

        return Lerp(VecA, VecB, ZT);
    }

    INTERNAL zv2i32 
    operator+(zv2i32 VecA, zv2i32 VecB) 
    {
        zv2i32 Result = {};

        Result.X = _mm256_add_epi32(VecA.X, VecB.X);
        Result.Y = _mm256_add_epi32(VecA.Y, VecB.Y);

        return Result;
    }

    INTERNAL zv2i32 
    operator-(zv2i32 VecA, zv2i32 VecB) 
    {
        zv2i32 Result = {};

        Result.X = _mm256_sub_epi32(VecA.X, VecB.X);
        Result.Y = _mm256_sub_epi32(VecA.Y, VecB.Y);

        return Result;
    }

    INTERNAL zv2i32 
    operator*(zv2i32 VecA, zv2i32 VecB) 
    {
        zv2i32 Result = {};

        Result.X = _mm256_mullo_epi32(VecA.X, VecB.X);
        Result.Y = _mm256_mullo_epi32(VecA.Y, VecB.Y);

        return Result;
    }

    INTERNAL zv2i32
    operator*(zv2i32 Vec, v256 Value)
    {
        zv2i32 Result = {};

        Result.X = _mm256_mullo_epi32(Vec.X, Value);
        Result.Y = _mm256_mullo_epi32(Vec.Y, Value);

        return Result;
    }

    INTERNAL zv2i32
    operator*(zv2i32 Vec, i32 Value)
    {
        v256 ZValue = _mm256_set1_epi32(Value);

        return Vec * ZValue;
    }

    INTERNAL zv2i32 
    operator/(zv2i32 VecA, zv2i32 VecB)
    {
        zv2i32 Result = {};
        v256f FloatAX = _mm256_cvtepi32_ps(VecA.X);
        v256f FloatAY = _mm256_cvtepi32_ps(VecA.Y);
        v256f FloatBX = _mm256_cvtepi32_ps(VecB.X);
        v256f FloatBY = _mm256_cvtepi32_ps(VecB.Y);
        v256f QuotientX = _mm256_div_ps(FloatAX, FloatBX);
        v256f QuotientY = _mm256_div_ps(FloatAY, FloatBY);

        Result.X = _mm256_cvtps_epi32(QuotientX);
        Result.Y = _mm256_cvtps_epi32(QuotientY);

        return Result;
    }

    INTERNAL zv2i32 
    operator&(zv2i32 VecA, zv2i32 VecB) 
    {
        zv2i32 Result = {};

        Result.X = _mm256_and_epi32(VecA.X, VecB.X);
        Result.Y = _mm256_and_epi32(VecA.Y, VecB.Y);

        return Result;
    }

    INTERNAL zv2i32 
    operator|(zv2i32 VecA, zv2i32 VecB) 
    {
        zv2i32 Result = {};

        Result.X = _mm256_or_epi32(VecA.X, VecB.X);
        Result.Y = _mm256_or_epi32(VecA.Y, VecB.Y);

        return Result;
    }

    INTERNAL zv2i32 
    operator^(zv2i32 VecA, zv2i32 VecB) 
    {
        zv2i32 Result = {};

        Result.X = _mm256_xor_epi32(VecA.X, VecB.X);
        Result.Y = _mm256_xor_epi32(VecA.Y, VecB.Y);

        return Result;
    }

    INTERNAL zv2i32 
    operator~(zv2i32 VecA)
    {
        zv2i32 Result = {};

        Result.X = _mm256_ternarylogic_epi32(VecA.X, VecA.X, VecA.X, 0x55);
        Result.Y = _mm256_ternarylogic_epi32(VecA.Y, VecA.Y, VecA.Y, 0x55);

        return Result;
    }

    INTERNAL v256
    operator==(zv2i32 VecA, zv2i32 VecB) 
    {
        __mmask8 MaskX = _mm256_cmp_epi32_mask(VecA.X, VecB.X, _MM_CMPINT_EQ);
        __mmask8 MaskY = _mm256_cmp_epi32_mask(VecA.Y, VecB.Y, _MM_CMPINT_EQ);
        __mmask8 ResultMask = _kand_mask8(MaskX, MaskY);
        v256 Result = _mm256_mask_mov_epi32(
            _mm256_setzero_si256(),
            ResultMask,
            _mm256_set1_epi32(~0)
        );

        return Result;
    }

    INTERNAL v256
    operator!=(zv2i32 VecA, zv2i32 VecB) 
    {
        __mmask8 MaskX = _mm256_cmp_epi32_mask(VecA.X, VecB.X, _MM_CMPINT_NE);
        __mmask8 MaskY = _mm256_cmp_epi32_mask(VecA.Y, VecB.Y, _MM_CMPINT_NE);
        __mmask8 ResultMask = _kand_mask8(MaskX, MaskY);
        v256 Result = _mm256_mask_mov_epi32(
            _mm256_setzero_si256(),
            ResultMask,
            _mm256_set1_epi32(~0)
        );

        return Result;
    }

    INTERNAL v256
    operator<(zv2i32 VecA, zv2i32 VecB) 
    {
        __mmask8 MaskX = _mm256_cmp_epi32_mask(VecA.X, VecB.X, _MM_CMPINT_LT);
        __mmask8 MaskY = _mm256_cmp_epi32_mask(VecA.Y, VecB.Y, _MM_CMPINT_LT);
        __mmask8 ResultMask = _kand_mask8(MaskX, MaskY);
        v256 Result = _mm256_mask_mov_epi32(
            _mm256_setzero_si256(),
            ResultMask,
            _mm256_set1_epi32(~0)
        );

        return Result;
    }

    INTERNAL v256
    operator>(zv2i32 VecA, zv2i32 VecB) 
    {
        __mmask8 MaskX = _mm256_cmp_epi32_mask(VecA.X, VecB.X, _MM_CMPINT_GT);
        __mmask8 MaskY = _mm256_cmp_epi32_mask(VecA.Y, VecB.Y, _MM_CMPINT_GT);
        __mmask8 ResultMask = _kand_mask8(MaskX, MaskY);
        v256 Result = _mm256_mask_mov_epi32(
            _mm256_setzero_si256(),
            ResultMask,
            _mm256_set1_epi32(~0)
        );

        return Result;
    }

    INTERNAL v256
    operator<=(zv2i32 VecA, zv2i32 VecB) 
    {
        __mmask8 MaskX = _mm256_cmp_epi32_mask(VecA.X, VecB.X, _MM_CMPINT_LE);
        __mmask8 MaskY = _mm256_cmp_epi32_mask(VecA.Y, VecB.Y, _MM_CMPINT_LE);
        __mmask8 ResultMask = _kand_mask8(MaskX, MaskY);
        v256 Result = _mm256_mask_mov_epi32(
            _mm256_setzero_si256(),
            ResultMask,
            _mm256_set1_epi32(~0)
        );

        return Result;
    }

    INTERNAL v256
    operator>=(zv2i32 VecA, zv2i32 VecB) 
    {
        __mmask8 MaskX = _mm256_cmp_epi32_mask(VecA.X, VecB.X, _MM_CMPINT_GE);
        __mmask8 MaskY = _mm256_cmp_epi32_mask(VecA.Y, VecB.Y, _MM_CMPINT_GE);
        __mmask8 ResultMask = _kand_mask8(MaskX, MaskY);
        v256 Result = _mm256_mask_mov_epi32(
            _mm256_setzero_si256(),
            ResultMask,
            _mm256_set1_epi32(~0)
        );

        return Result;
    }

    INTERNAL zv2i32 
    operator<<(zv2i32 VecA, i32 Shift)
    {
        zv2i32 Result = {};

        Result.X = _mm256_slli_epi32(VecA.X, Shift);
        Result.Y = _mm256_slli_epi32(VecA.Y, Shift);

        return Result;
    }

    INTERNAL zv2i32 
    operator>>(zv2i32 VecA, i32 Shift)
    {
        zv2i32 Result = {};

        Result.X = _mm256_srli_epi32(VecA.X, Shift);
        Result.Y = _mm256_srli_epi32(VecA.Y, Shift);

        return Result;
    }

    INTERNAL v256
    Dot(zv2i32 VecA, zv2i32 VecB)
    {
        v256 Result = {};

        Result = _mm256_add_epi32(
            _mm256_mul_epi32(VecA.X, VecB.X),
            _mm256_mul_epi32(VecA.Y, VecB.Y)
        );

        return Result;
    }

    INTERNAL v256
    LengthSq(zv2i32 Vec)
    {
        return Dot(Vec, Vec);
    }

    INTERNAL v256
    Length(zv2i32 Vec)
    {
        v256f LenSq = _mm256_cvtepi32_ps(LengthSq(Vec));
        
        return _mm256_cvtps_epi32(
            _mm256_sqrt_ps(LenSq)
        );
    }

    INTERNAL zv2i32
    Norm(zv2i32 Vec)
    {
        v256 Scale = _mm256_cvtps_epi32(
            _mm256_rsqrt14_ps(
                _mm256_cvtepi32_ps(
                    LengthSq(Vec)
                )
            )
        );

        return Vec * Scale;
    }

    INTERNAL zv2i32
    Lerp(zv2i32 VecA, zv2i32 VecB, v256 T)
    {
        zv2i32 Result = {};
        v256f AX = _mm256_cvtepi32_ps(VecA.X);
        v256f AY = _mm256_cvtepi32_ps(VecA.Y);
        v256f BX = _mm256_cvtepi32_ps(VecB.X);
        v256f BY = _mm256_cvtepi32_ps(VecB.Y);
        v256f ZT = _mm256_cvtepi32_ps(T);
        v256f ResultX = _mm256_fmadd_ps(
            ZT,
            BX,
            _mm256_fnmadd_ps(ZT, AX, AX)
        );
        v256f ResultY = _mm256_fmadd_ps(
            ZT,
            BY,
            _mm256_fnmadd_ps(ZT, AY, AY)
        );

        Result.X = _mm256_cvtps_epi32(ResultX);
        Result.Y = _mm256_cvtps_epi32(ResultY);

        return Result;
    }

    INTERNAL zv2i32
    Lerp(zv2i32 VecA, zv2i32 VecB, i32 T)
    {
        v256 ZT = _mm256_set1_epi32(T);

        return Lerp(VecA, VecB, ZT);
    }

    INTERNAL zv2f64 
    operator+(zv2f64 VecA, zv2f64 VecB) 
    {
        zv2f64 Result = {};

        Result.X = _mm256_add_pd(VecA.X, VecB.X);
        Result.Y = _mm256_add_pd(VecA.Y, VecB.Y);

        return Result;
    }

    INTERNAL zv2f64 
    operator-(zv2f64 VecA, zv2f64 VecB) 
    {
        zv2f64 Result = {};

        Result.X = _mm256_sub_pd(VecA.X, VecB.X);
        Result.Y = _mm256_sub_pd(VecA.Y, VecB.Y);

        return Result;
    }

    INTERNAL zv2f64 
    operator*(zv2f64 VecA, zv2f64 VecB) 
    {
        zv2f64 Result = {};

        Result.X = _mm256_mul_pd(VecA.X, VecB.X);
        Result.Y = _mm256_mul_pd(VecA.Y, VecB.Y);

        return Result;
    }

    INTERNAL zv2f64 
    operator*(zv2f64 Vec, v256d Value) 
    {
        zv2f64 Result = {};

        Result.X = _mm256_mul_pd(Vec.X, Value);
        Result.Y = _mm256_mul_pd(Vec.Y, Value);

        return Result;
    }

    INTERNAL zv2f64 
    operator*(zv2f64 Vec, f64 Value) 
    {
        v256d ZValue = _mm256_set1_pd(Value);

        return Vec * ZValue;
    }

    INTERNAL zv2f64 
    operator/(zv2f64 VecA, zv2f64 VecB) 
    {
        zv2f64 Result = {};

        Result.X = _mm256_div_pd(VecA.X, VecB.X);
        Result.Y = _mm256_div_pd(VecA.Y, VecB.Y);

        return Result;
    }

    INTERNAL zv2f64 
    operator/(zv2f64 Vec, v256d Value) 
    {
        zv2f64 Result = {};

        Result.X = _mm256_div_pd(Vec.X, Value);
        Result.Y = _mm256_div_pd(Vec.Y, Value);

        return Result;
    }

    INTERNAL zv2f64 
    operator/(zv2f64 Vec, f64 Value) 
    {
        v256d ZValue = _mm256_set1_pd(Value);

        return Vec / ZValue;
    }

    INTERNAL zv2f64 
    operator&(zv2f64 VecA, zv2f64 VecB) 
    {
        zv2f64 Result = {};

        Result.X = _mm256_and_pd(VecA.X, VecB.X);
        Result.Y = _mm256_and_pd(VecA.Y, VecB.Y);

        return Result;
    }

    INTERNAL zv2f64 
    operator|(zv2f64 VecA, zv2f64 VecB) 
    {
        zv2f64 Result = {};

        Result.X = _mm256_or_pd(VecA.X, VecB.X);
        Result.Y = _mm256_or_pd(VecA.Y, VecB.Y);

        return Result;
    }

    INTERNAL zv2f64 
    operator^(zv2f64 VecA, zv2f64 VecB) 
    {
        zv2f64 Result = {};

        Result.X = _mm256_xor_pd(VecA.X, VecB.X);
        Result.Y = _mm256_xor_pd(VecA.Y, VecB.Y);

        return Result;
    }

    INTERNAL v256d
    operator==(zv2f64 VecA, zv2f64 VecB) 
    {
        __mmask8 MaskX = _mm256_cmp_pd_mask(VecA.X, VecB.X, _CMP_EQ_OQ);
        __mmask8 MaskY = _mm256_cmp_pd_mask(VecA.Y, VecB.Y, _CMP_EQ_OQ);
        __mmask8 ResultMask = _kand_mask8(MaskX, MaskY);
        v256d Result = _mm256_mask_mov_pd(
            _mm256_setzero_pd(),
            ResultMask,
            _mm256_set1_pd(1.0)
        );

        return Result;
    }

    INTERNAL v256d
    operator!=(zv2f64 VecA, zv2f64 VecB) 
    {
        __mmask8 MaskX = _mm256_cmp_pd_mask(VecA.X, VecB.X, _CMP_NEQ_OQ);
        __mmask8 MaskY = _mm256_cmp_pd_mask(VecA.Y, VecB.Y, _CMP_NEQ_OQ);
        __mmask8 ResultMask = _kand_mask8(MaskX, MaskY);
        v256d Result = _mm256_mask_mov_pd(
            _mm256_setzero_pd(),
            ResultMask,
            _mm256_set1_pd(1.0)
        );

        return Result;
    }

    INTERNAL v256d
    operator<(zv2f64 VecA, zv2f64 VecB) 
    {
        __mmask8 MaskX = _mm256_cmp_pd_mask(VecA.X, VecB.X, _CMP_LT_OQ);
        __mmask8 MaskY = _mm256_cmp_pd_mask(VecA.Y, VecB.Y, _CMP_LT_OQ);
        __mmask8 ResultMask = _kand_mask8(MaskX, MaskY);
        v256d Result = _mm256_mask_mov_pd(
            _mm256_setzero_pd(),
            ResultMask,
            _mm256_set1_pd(1.0)
        );

        return Result;
    }

    INTERNAL v256d
    operator>(zv2f64 VecA, zv2f64 VecB) 
    {
        __mmask8 MaskX = _mm256_cmp_pd_mask(VecA.X, VecB.X, _CMP_GT_OQ);
        __mmask8 MaskY = _mm256_cmp_pd_mask(VecA.Y, VecB.Y, _CMP_GT_OQ);
        __mmask8 ResultMask = _kand_mask8(MaskX, MaskY);
        v256d Result = _mm256_mask_mov_pd(
            _mm256_setzero_pd(),
            ResultMask,
            _mm256_set1_pd(1.0)
        );

        return Result;
    }

    INTERNAL v256d
    operator<=(zv2f64 VecA, zv2f64 VecB) 
    {
        __mmask8 MaskX = _mm256_cmp_pd_mask(VecA.X, VecB.X, _CMP_LE_OQ);
        __mmask8 MaskY = _mm256_cmp_pd_mask(VecA.Y, VecB.Y, _CMP_LE_OQ);
        __mmask8 ResultMask = _kand_mask8(MaskX, MaskY);
        v256d Result = _mm256_mask_mov_pd(
            _mm256_setzero_pd(),
            ResultMask,
            _mm256_set1_pd(1.0)
        );

        return Result;
    }

    INTERNAL v256d
    operator>=(zv2f64 VecA, zv2f64 VecB) 
    {
        __mmask8 MaskX = _mm256_cmp_pd_mask(VecA.X, VecB.X, _CMP_GE_OQ);
        __mmask8 MaskY = _mm256_cmp_pd_mask(VecA.Y, VecB.Y, _CMP_GE_OQ);
        __mmask8 ResultMask = _kand_mask8(MaskX, MaskY);
        v256d Result = _mm256_mask_mov_pd(
            _mm256_setzero_pd(),
            ResultMask,
            _mm256_set1_pd(1.0)
        );

        return Result;
    }

    INTERNAL zv2f64 
    operator%(zv2f64 VecA, zv2f64 VecB) 
    {
        zv2f64 Result = {};
        v256d DivX = _mm256_div_pd(VecA.X, VecB.X);
        v256d DivY = _mm256_div_pd(VecA.Y, VecB.Y);
        v256d FloorX = _mm256_roundscale_pd(DivX, _MM_FROUND_FLOOR);                // TODO: compare against _mm256_floor_pd and 
        v256d FloorY = _mm256_roundscale_pd(DivY, _MM_FROUND_FLOOR);                //     : _mm256_roundscale_round_pd

        Result.X = _mm256_fnmadd_pd(FloorX, VecB.X, VecA.X);
        Result.Y = _mm256_fnmadd_pd(FloorY, VecB.Y, VecA.Y);

        return Result;
    }

    INTERNAL v256d
    Dot(zv2f64 VecA, zv2f64 VecB)
    {
        v256d Result = {};

        Result = _mm256_fmadd_pd(
            VecA.X,
            VecB.X,
            _mm256_mul_pd(VecA.Y, VecB.Y)
        );

        return Result;
    }

    INTERNAL v256d
    LengthSq(zv2f64 Vec)
    {
        return Dot(Vec, Vec);
    }

    INTERNAL v256d
    Length(zv2f64 Vec)
    {
        return _mm256_sqrt_pd(LengthSq(Vec));
    }

    INTERNAL zv2f64
    Norm(zv2f64 Vec)
    {
        v256d Scale = _mm256_rsqrt14_pd(LengthSq(Vec));

        return Vec * Scale;
    }

    INTERNAL zv2f64
    Lerp(zv2f64 VecA, zv2f64 VecB, v256d T)
    {
        zv2f64 Result = {};

        Result.X = _mm256_fmadd_pd(
            T,
            VecB.X,
            _mm256_fnmadd_pd(T, VecA.X, VecA.X)
        );
        Result.Y = _mm256_fmadd_pd(
            T,
            VecB.Y,
            _mm256_fnmadd_pd(T, VecA.Y, VecA.Y)
        );

        return Result;
    }

    INTERNAL zv2f64
    Lerp(zv2f64 VecA, zv2f64 VecB, f64 T)
    {
        v256d ZT = _mm256_set1_pd(T);

        return Lerp(VecA, VecB, ZT);
    }

    INTERNAL zv3f32 
    operator+(zv3f32 VecA, zv3f32 VecB) 
    {
        zv3f32 Result = {};

        Result.X = _mm256_add_ps(VecA.X, VecB.X);
        Result.Y = _mm256_add_ps(VecA.Y, VecB.Y);
        Result.Z = _mm256_add_ps(VecA.Z, VecB.Z);

        return Result;
    }

    INTERNAL zv3f32 
    operator-(zv3f32 VecA, zv3f32 VecB) 
    {
        zv3f32 Result = {};

        Result.X = _mm256_sub_ps(VecA.X, VecB.X);
        Result.Y = _mm256_sub_ps(VecA.Y, VecB.Y);
        Result.Z = _mm256_sub_ps(VecA.Z, VecB.Z);

        return Result;
    }

    INTERNAL zv3f32 
    operator*(zv3f32 VecA, zv3f32 VecB) 
    {
        zv3f32 Result = {};

        Result.X = _mm256_mul_ps(VecA.X, VecB.X);
        Result.Y = _mm256_mul_ps(VecA.Y, VecB.Y);
        Result.Z = _mm256_mul_ps(VecA.Z, VecB.Z);

        return Result;
    }

    INTERNAL zv3f32 
    operator*(zv3f32 Vec, v256f Value) 
    {
        zv3f32 Result = {};

        Result.X = _mm256_mul_ps(Vec.X, Value);
        Result.Y = _mm256_mul_ps(Vec.Y, Value);
        Result.Z = _mm256_mul_ps(Vec.Z, Value);

        return Result;
    }

    INTERNAL zv3f32 
    operator*(zv3f32 Vec, f32 Value) 
    {
        v256f ZValue = _mm256_set1_ps(Value);

        return Vec * ZValue;
    }

    INTERNAL zv3f32 
    operator/(zv3f32 VecA, zv3f32 VecB) 
    {
        zv3f32 Result = {};

        Result.X = _mm256_div_ps(VecA.X, VecB.X);
        Result.Y = _mm256_div_ps(VecA.Y, VecB.Y);
        Result.Z = _mm256_div_ps(VecA.Z, VecB.Z);

        return Result;
    }

    INTERNAL zv3f32
    operator/(zv3f32 Vec, v256f Value) 
    {
        zv3f32 Result = {};

        Result.X = _mm256_div_ps(Vec.X, Value);
        Result.Y = _mm256_div_ps(Vec.Y, Value);
        Result.Z = _mm256_div_ps(Vec.Z, Value);

        return Result;
    }

    INTERNAL zv3f32
    operator/(zv3f32 Vec, f32 Value) 
    {
        v256f ZValue = _mm256_set1_ps(Value);

        return Vec / ZValue;
    }

    INTERNAL zv3f32 
    operator&(zv3f32 VecA, zv3f32 VecB) 
    {
        zv3f32 Result = {};

        Result.X = _mm256_and_ps(VecA.X, VecB.X);
        Result.Y = _mm256_and_ps(VecA.Y, VecB.Y);
        Result.Z = _mm256_and_ps(VecA.Z, VecB.Z);

        return Result;
    }

    INTERNAL zv3f32 
    operator|(zv3f32 VecA, zv3f32 VecB) 
    {
        zv3f32 Result = {};

        Result.X = _mm256_or_ps(VecA.X, VecB.X);
        Result.Y = _mm256_or_ps(VecA.Y, VecB.Y);
        Result.Z = _mm256_or_ps(VecA.Z, VecB.Z);

        return Result;
    }

    INTERNAL zv3f32 
    operator^(zv3f32 VecA, zv3f32 VecB) 
    {
        zv3f32 Result = {};

        Result.X = _mm256_xor_ps(VecA.X, VecB.X);
        Result.Y = _mm256_xor_ps(VecA.Y, VecB.Y);
        Result.Z = _mm256_xor_ps(VecA.Z, VecB.Z);

        return Result;
    }

    INTERNAL v256f
    operator==(zv3f32 VecA, zv3f32 VecB) 
    {
        __mmask8 MaskX = _mm256_cmp_ps_mask(VecA.X, VecB.X, _CMP_EQ_OQ);
        __mmask8 MaskY = _mm256_cmp_ps_mask(VecA.Y, VecB.Y, _CMP_EQ_OQ);
        __mmask8 MaskZ = _mm256_cmp_ps_mask(VecA.Z, VecB.Z, _CMP_EQ_OQ);
        __mmask8 ResultMask = _kand_mask8(_kand_mask8(MaskX, MaskY), MaskZ);
        v256f Result = _mm256_mask_mov_ps(
            _mm256_setzero_ps(),
            ResultMask,
            _mm256_set1_ps(1.0f)
        );

        return Result;
    }

    INTERNAL v256f 
    operator!=(zv3f32 VecA, zv3f32 VecB) 
    {
        __mmask8 MaskX = _mm256_cmp_ps_mask(VecA.X, VecB.X, _CMP_NEQ_OQ);
        __mmask8 MaskY = _mm256_cmp_ps_mask(VecA.Y, VecB.Y, _CMP_NEQ_OQ);
        __mmask8 MaskZ = _mm256_cmp_ps_mask(VecA.Z, VecB.Z, _CMP_NEQ_OQ);
        __mmask8 ResultMask = _kand_mask8(_kand_mask8(MaskX, MaskY), MaskZ);
        v256f Result = _mm256_mask_mov_ps(
            _mm256_setzero_ps(),
            ResultMask,
            _mm256_set1_ps(1.0f)
        );

        return Result;
    }

    INTERNAL v256f 
    operator<(zv3f32 VecA, zv3f32 VecB) 
    {
        __mmask8 MaskX = _mm256_cmp_ps_mask(VecA.X, VecB.X, _CMP_LT_OQ);
        __mmask8 MaskY = _mm256_cmp_ps_mask(VecA.Y, VecB.Y, _CMP_LT_OQ);
        __mmask8 MaskZ = _mm256_cmp_ps_mask(VecA.Z, VecB.Z, _CMP_LT_OQ);
        __mmask8 ResultMask = _kand_mask8(_kand_mask8(MaskX, MaskY), MaskZ);
        v256f Result = _mm256_mask_mov_ps(
            _mm256_setzero_ps(),
            ResultMask,
            _mm256_set1_ps(1.0f)
        );

        return Result;
    }

    INTERNAL v256f 
    operator>(zv3f32 VecA, zv3f32 VecB) 
    {
        __mmask8 MaskX = _mm256_cmp_ps_mask(VecA.X, VecB.X, _CMP_GT_OQ);
        __mmask8 MaskY = _mm256_cmp_ps_mask(VecA.Y, VecB.Y, _CMP_GT_OQ);
        __mmask8 MaskZ = _mm256_cmp_ps_mask(VecA.Z, VecB.Z, _CMP_GT_OQ);
        __mmask8 ResultMask = _kand_mask8(_kand_mask8(MaskX, MaskY), MaskZ);
        v256f Result = _mm256_mask_mov_ps(
            _mm256_setzero_ps(),
            ResultMask,
            _mm256_set1_ps(1.0f)
        );

        return Result;
    }

    INTERNAL v256f 
    operator<=(zv3f32 VecA, zv3f32 VecB) 
    {
        __mmask8 MaskX = _mm256_cmp_ps_mask(VecA.X, VecB.X, _CMP_LE_OQ);
        __mmask8 MaskY = _mm256_cmp_ps_mask(VecA.Y, VecB.Y, _CMP_LE_OQ);
        __mmask8 MaskZ = _mm256_cmp_ps_mask(VecA.Z, VecB.Z, _CMP_LE_OQ);
        __mmask8 ResultMask = _kand_mask8(_kand_mask8(MaskX, MaskY), MaskZ);
        v256f Result = _mm256_mask_mov_ps(
            _mm256_setzero_ps(),
            ResultMask,
            _mm256_set1_ps(1.0f)
        );

        return Result;
    }

    INTERNAL v256f 
    operator>=(zv3f32 VecA, zv3f32 VecB) 
    {
        __mmask8 MaskX = _mm256_cmp_ps_mask(VecA.X, VecB.X, _CMP_GE_OQ);
        __mmask8 MaskY = _mm256_cmp_ps_mask(VecA.Y, VecB.Y, _CMP_GE_OQ);
        __mmask8 MaskZ = _mm256_cmp_ps_mask(VecA.Z, VecB.Z, _CMP_GE_OQ);
        __mmask8 ResultMask = _kand_mask8(_kand_mask8(MaskX, MaskY), MaskZ);
        v256f Result = _mm256_mask_mov_ps(
            _mm256_setzero_ps(),
            ResultMask,
            _mm256_set1_ps(1.0f)
        );

        return Result;
    }

    INTERNAL zv3f32 
    operator%(zv3f32 VecA, zv3f32 VecB) 
    {
        zv3f32 Result = {};
        v256f DivX = _mm256_div_ps(VecA.X, VecB.X);
        v256f DivY = _mm256_div_ps(VecA.Y, VecB.Y);
        v256f DivZ = _mm256_div_ps(VecA.Z, VecB.Z);
        v256f FloorX = _mm256_roundscale_ps(DivX, _MM_FROUND_FLOOR);                // TODO: compare against _mm256_floor_ps and 
        v256f FloorY = _mm256_roundscale_ps(DivY, _MM_FROUND_FLOOR);                //     : _mm256_roundscale_round_ps
        v256f FloorZ = _mm256_roundscale_ps(DivZ, _MM_FROUND_FLOOR);

        Result.X = _mm256_fnmadd_ps(FloorX, VecB.X, VecA.X);
        Result.Y = _mm256_fnmadd_ps(FloorY, VecB.Y, VecA.Y);
        Result.Z = _mm256_fnmadd_ps(FloorZ, VecB.Z, VecA.Z);

        return Result;
    }

    INTERNAL v256f
    Dot(zv3f32 VecA, zv3f32 VecB)
    {
        v256f Result = {};

        Result = _mm256_fmadd_ps(
            VecA.X,
            VecB.X,
            _mm256_fmadd_ps(
                VecA.Y,
                VecB.Y,
                _mm256_mul_ps(VecA.Z, VecB.Z)
            )
        );

        return Result;
    }

    INTERNAL v256f
    LengthSq(zv3f32 Vec)
    {
        return Dot(Vec, Vec);
    }

    INTERNAL v256f
    Length(zv3f32 Vec)
    {
        return _mm256_sqrt_ps(LengthSq(Vec));
    }

    INTERNAL zv3f32
    Norm(zv3f32 Vec)
    {
        v256f Scale = _mm256_rsqrt14_ps(LengthSq(Vec));

        return Vec * Scale;
    }

    INTERNAL zv3f32
    Lerp(zv3f32 VecA, zv3f32 VecB, v256f T)
    {
        zv3f32 Result = {};

        Result.X = _mm256_fmadd_ps(
            T, 
            VecB.X, 
            _mm256_fnmadd_ps(T, VecA.X, VecA.X)
        );
        Result.Y = _mm256_fmadd_ps(
            T, 
            VecB.Y, 
            _mm256_fnmadd_ps(T, VecA.Y, VecA.Y)
        );
        Result.Z = _mm256_fmadd_ps(
            T, 
            VecB.Z, 
            _mm256_fnmadd_ps(T, VecA.Z, VecA.Z)
        );

        return Result;
    }

    INTERNAL zv3f32
    Lerp(zv3f32 VecA, zv3f32 VecB, f32 T)
    {
        v256f ZT = _mm256_set1_ps(T);

        return Lerp(VecA, VecB, ZT);
    }

    INTERNAL zv3f32
    Cross(zv3f32 VecA, zv3f32 VecB)
    {
        zv3f32 Result = {};

        Result.X = _mm256_fmsub_ps(
            VecA.Y,
            VecB.Z,
            _mm256_mul_ps(VecA.Z, VecB.Y)
        );
        Result.Y = _mm256_fmsub_ps(
            VecA.Z,
            VecB.X,
            _mm256_mul_ps(VecA.X, VecB.Z)
        );
        Result.Z = _mm256_fmsub_ps(
            VecA.X,
            VecB.Y,
            _mm256_mul_ps(VecA.Y, VecB.X)
        );

        return Result;
    }

    INTERNAL zv3i32 
    operator+(zv3i32 VecA, zv3i32 VecB) 
    {
        zv3i32 Result = {};

        Result.X = _mm256_add_epi32(VecA.X, VecB.X);
        Result.Y = _mm256_add_epi32(VecA.Y, VecB.X);
        Result.Z = _mm256_add_epi32(VecA.Z, VecB.X);

        return Result;
    }

    INTERNAL zv3i32 
    operator-(zv3i32 VecA, zv3i32 VecB) 
    {
        zv3i32 Result = {};

        Result.X = _mm256_sub_epi32(VecA.X, VecB.X);
        Result.Y = _mm256_sub_epi32(VecA.Y, VecB.X);
        Result.Z = _mm256_sub_epi32(VecA.Z, VecB.X);

        return Result;
    }

    INTERNAL zv3i32 
    operator*(zv3i32 VecA, zv3i32 VecB) 
    {
        zv3i32 Result = {};

        Result.X = _mm256_mullo_epi32(VecA.X, VecB.X);
        Result.Y = _mm256_mullo_epi32(VecA.Y, VecB.X);
        Result.Z = _mm256_mullo_epi32(VecA.Z, VecB.X);

        return Result;
    }

    INTERNAL zv3i32 
    operator*(zv3i32 Vec, v256 Value) 
    {
        zv3i32 Result = {};

        Result.X = _mm256_mullo_epi32(Vec.X, Value);
        Result.Y = _mm256_mullo_epi32(Vec.Y, Value);
        Result.Z = _mm256_mullo_epi32(Vec.Z, Value);

        return Result;
    }

    INTERNAL zv3i32 
    operator*(zv3i32 Vec, i32 Value) 
    {
        v256 ZValue = _mm256_set1_epi32(Value);

        return Vec * ZValue;
    }

    INTERNAL zv3i32 
    operator/(zv3i32 VecA, zv3i32 VecB) 
    {
        zv3i32 Result = {};
        v256f FloatAX = _mm256_cvtepi32_ps(VecA.X);
        v256f FloatAY = _mm256_cvtepi32_ps(VecA.Y);
        v256f FloatAZ = _mm256_cvtepi32_ps(VecA.Z);
        v256f FloatBX = _mm256_cvtepi32_ps(VecB.X);
        v256f FloatBY = _mm256_cvtepi32_ps(VecB.Y);
        v256f FloatBZ = _mm256_cvtepi32_ps(VecB.Z);
        v256f QuotientX = _mm256_div_ps(FloatAX, FloatBX);
        v256f QuotientY = _mm256_div_ps(FloatAY, FloatBY);
        v256f QuotientZ = _mm256_div_ps(FloatAZ, FloatBZ);

        Result.X = _mm256_cvtps_epi32(QuotientX);
        Result.Y = _mm256_cvtps_epi32(QuotientY);
        Result.Z = _mm256_cvtps_epi32(QuotientZ);

        return Result;
    }

    INTERNAL zv3i32 
    operator&(zv3i32 VecA, zv3i32 VecB) 
    {
        zv3i32 Result = {};

        Result.X = _mm256_and_epi32(VecA.X, VecB.X);
        Result.Y = _mm256_and_epi32(VecA.Y, VecB.X);
        Result.Z = _mm256_and_epi32(VecA.Z, VecB.X);

        return Result;
    }

    INTERNAL zv3i32 
    operator|(zv3i32 VecA, zv3i32 VecB) 
    {
        zv3i32 Result = {};

        Result.X = _mm256_or_epi32(VecA.X, VecB.X);
        Result.Y = _mm256_or_epi32(VecA.Y, VecB.X);
        Result.Z = _mm256_or_epi32(VecA.Z, VecB.X);

        return Result;
    }

    INTERNAL zv3i32 
    operator^(zv3i32 VecA, zv3i32 VecB) 
    {
        zv3i32 Result = {};

        Result.X = _mm256_xor_epi32(VecA.X, VecB.X);
        Result.Y = _mm256_xor_epi32(VecA.Y, VecB.X);
        Result.Z = _mm256_xor_epi32(VecA.Z, VecB.X);

        return Result;
    }

    INTERNAL v256
    operator==(zv3i32 VecA, zv3i32 VecB) 
    {
        __mmask8 MaskX = _mm256_cmp_epi32_mask(VecA.X, VecB.X, _MM_CMPINT_EQ);
        __mmask8 MaskY = _mm256_cmp_epi32_mask(VecA.Y, VecB.Y, _MM_CMPINT_EQ);
        __mmask8 MaskZ = _mm256_cmp_epi32_mask(VecA.Z, VecB.Z, _MM_CMPINT_EQ);
        __mmask8 ResultMask = _kand_mask8(_kand_mask8(MaskX, MaskY), MaskZ);
        v256 Result = _mm256_mask_mov_epi32(
            _mm256_setzero_si256(),
            ResultMask,
            _mm256_set1_epi32(~0)
        );

        return Result;
    }

    INTERNAL v256
    operator!=(zv3i32 VecA, zv3i32 VecB) 
    {
        __mmask8 MaskX = _mm256_cmp_epi32_mask(VecA.X, VecB.X, _MM_CMPINT_NE);
        __mmask8 MaskY = _mm256_cmp_epi32_mask(VecA.Y, VecB.Y, _MM_CMPINT_NE);
        __mmask8 MaskZ = _mm256_cmp_epi32_mask(VecA.Z, VecB.Z, _MM_CMPINT_NE);
        __mmask8 ResultMask = _kand_mask8(_kand_mask8(MaskX, MaskY), MaskZ);
        v256 Result = _mm256_mask_mov_epi32(
            _mm256_setzero_si256(),
            ResultMask,
            _mm256_set1_epi32(~0)
        );

        return Result;
    }

    INTERNAL v256
    operator<(zv3i32 VecA, zv3i32 VecB) 
    {
        __mmask8 MaskX = _mm256_cmp_epi32_mask(VecA.X, VecB.X, _MM_CMPINT_LT);
        __mmask8 MaskY = _mm256_cmp_epi32_mask(VecA.Y, VecB.Y, _MM_CMPINT_LT);
        __mmask8 MaskZ = _mm256_cmp_epi32_mask(VecA.Z, VecB.Z, _MM_CMPINT_LT);
        __mmask8 ResultMask = _kand_mask8(_kand_mask8(MaskX, MaskY), MaskZ);
        v256 Result = _mm256_mask_mov_epi32(
            _mm256_setzero_si256(),
            ResultMask,
            _mm256_set1_epi32(~0)
        );

        return Result;
    }

    INTERNAL v256
    operator>(zv3i32 VecA, zv3i32 VecB) 
    {
        __mmask8 MaskX = _mm256_cmp_epi32_mask(VecA.X, VecB.X, _MM_CMPINT_GT);
        __mmask8 MaskY = _mm256_cmp_epi32_mask(VecA.Y, VecB.Y, _MM_CMPINT_GT);
        __mmask8 MaskZ = _mm256_cmp_epi32_mask(VecA.Z, VecB.Z, _MM_CMPINT_GT);
        __mmask8 ResultMask = _kand_mask8(_kand_mask8(MaskX, MaskY), MaskZ);
        v256 Result = _mm256_mask_mov_epi32(
            _mm256_setzero_si256(),
            ResultMask,
            _mm256_set1_epi32(~0)
        );

        return Result;
    }

    INTERNAL v256
    operator<=(zv3i32 VecA, zv3i32 VecB) 
    {
        __mmask8 MaskX = _mm256_cmp_epi32_mask(VecA.X, VecB.X, _MM_CMPINT_LE);
        __mmask8 MaskY = _mm256_cmp_epi32_mask(VecA.Y, VecB.Y, _MM_CMPINT_LE);
        __mmask8 MaskZ = _mm256_cmp_epi32_mask(VecA.Z, VecB.Z, _MM_CMPINT_LE);
        __mmask8 ResultMask = _kand_mask8(_kand_mask8(MaskX, MaskY), MaskZ);
        v256 Result = _mm256_mask_mov_epi32(
            _mm256_setzero_si256(),
            ResultMask,
            _mm256_set1_epi32(~0)
        );

        return Result;
    }

    INTERNAL v256
    operator>=(zv3i32 VecA, zv3i32 VecB) 
    {
        __mmask8 MaskX = _mm256_cmp_epi32_mask(VecA.X, VecB.X, _MM_CMPINT_GE);
        __mmask8 MaskY = _mm256_cmp_epi32_mask(VecA.Y, VecB.Y, _MM_CMPINT_GE);
        __mmask8 MaskZ = _mm256_cmp_epi32_mask(VecA.Z, VecB.Z, _MM_CMPINT_GE);
        __mmask8 ResultMask = _kand_mask8(_kand_mask8(MaskX, MaskY), MaskZ);
        v256 Result = _mm256_mask_mov_epi32(
            _mm256_setzero_si256(),
            ResultMask,
            _mm256_set1_epi32(~0)
        );

        return Result;
    }

    INTERNAL zv3i32 
    operator<<(zv3i32 VecA, i32 Shift)
    {
        zv3i32 Result = {};

        Result.X = _mm256_slli_epi32(VecA.X, Shift);
        Result.Y = _mm256_slli_epi32(VecA.Y, Shift);
        Result.Z = _mm256_slli_epi32(VecA.Z, Shift);

        return Result;
    }

    INTERNAL zv3i32 
    operator>>(zv3i32 VecA, i32 Shift)
    {
        zv3i32 Result = {};

        Result.X = _mm256_srli_epi32(VecA.X, Shift);
        Result.Y = _mm256_srli_epi32(VecA.Y, Shift);
        Result.Z = _mm256_srli_epi32(VecA.Z, Shift);

        return Result;
    }

    INTERNAL v256
    Dot(zv3i32 VecA, zv3i32 VecB)
    {
        v256 Result = {};

        Result = _mm256_add_epi32(
            _mm256_add_epi32(
                _mm256_mullo_epi32(VecA.X, VecB.X),
                _mm256_mullo_epi32(VecA.Y, VecB.Y)
            ),
            _mm256_mullo_epi32(VecA.Z, VecB.Z)
        );

        return Result;
    }

    INTERNAL v256
    LengthSq(zv3i32 Vec)
    {
        return Dot(Vec, Vec);
    }

    INTERNAL v256
    Length(zv3i32 Vec)
    {
        v256f LenSq = _mm256_cvtepi32_ps(LengthSq(Vec));
        
        return _mm256_cvtps_epi32(
            _mm256_sqrt_ps(LenSq)
        );
    }

    INTERNAL zv3i32
    Norm(zv3i32 Vec)
    {
        v256 Scale = _mm256_cvtps_epi32(
            _mm256_rsqrt14_ps(
                _mm256_cvtepi32_ps(
                    Length(Vec)
                )
            )
        );

        return Vec * Scale;
    }

    INTERNAL zv3i32
    Lerp(zv3i32 VecA, zv3i32 VecB, v256 T)
    {
        zv3i32 Result = {};
        v256f AX = _mm256_cvtepi32_ps(VecA.X);
        v256f AY = _mm256_cvtepi32_ps(VecA.Y);
        v256f AZ = _mm256_cvtepi32_ps(VecA.Z);
        v256f BX = _mm256_cvtepi32_ps(VecB.X);
        v256f BY = _mm256_cvtepi32_ps(VecB.Y);
        v256f BZ = _mm256_cvtepi32_ps(VecB.Z);
        v256f ZT = _mm256_cvtepi32_ps(T);
        v256f ResultX = _mm256_fmadd_ps(
            ZT,
            BX,
            _mm256_fnmadd_ps(
                ZT, 
                AX,
                AX
            )
        );
        v256f ResultY = _mm256_fmadd_ps(
            ZT,
            BY,
            _mm256_fnmadd_ps(
                ZT, 
                AY,
                AY
            )
        );
        v256f ResultZ = _mm256_fmadd_ps(
            ZT,
            BZ,
            _mm256_fnmadd_ps(
                ZT, 
                AZ,
                AZ
            )
        );

        Result.X = _mm256_cvtps_epi32(ResultX);
        Result.Y = _mm256_cvtps_epi32(ResultY);
        Result.Z = _mm256_cvtps_epi32(ResultZ);

        return Result;
    }

    INTERNAL zv3i32
    Lerp(zv3i32 VecA, zv3i32 VecB, i32 T)
    {
        v256 ZT = _mm256_set1_epi32(T);

        return Lerp(VecA, VecB, ZT);
    }

    INTERNAL zv3i32
    Cross(zv3i32 VecA, zv3i32 VecB)
    {
        zv3i32 Result = {};

        Result.X = _mm256_sub_epi32(
            _mm256_mullo_epi32(VecA.Y, VecB.Z),
            _mm256_mullo_epi32(VecA.Z, VecB.Y)
        );
        Result.Y = _mm256_sub_epi32(
            _mm256_mullo_epi32(VecA.Z, VecB.X),
            _mm256_mullo_epi32(VecA.X, VecB.Z)
        );
        Result.Z = _mm256_sub_epi32(
            _mm256_mullo_epi32(VecA.X, VecB.Y),
            _mm256_mullo_epi32(VecA.Y, VecB.X)
        );

        return Result;
    }

    INTERNAL zv3f64 
    operator+(zv3f64 VecA, zv3f64 VecB) 
    {
        zv3f64 Result = {};

        Result.X = _mm256_add_pd(VecA.X, VecB.X);
        Result.Y = _mm256_add_pd(VecA.Y, VecB.Y);
        Result.Z = _mm256_add_pd(VecA.Z, VecB.Z);

        return Result;
    }

    INTERNAL zv3f64 
    operator-(zv3f64 VecA, zv3f64 VecB) 
    {
        zv3f64 Result = {};

        Result.X = _mm256_sub_pd(VecA.X, VecB.X);
        Result.Y = _mm256_sub_pd(VecA.Y, VecB.Y);
        Result.Z = _mm256_sub_pd(VecA.Z, VecB.Z);

        return Result;
    }

    INTERNAL zv3f64 
    operator*(zv3f64 VecA, zv3f64 VecB) 
    {
        zv3f64 Result = {};

        Result.X = _mm256_mul_pd(VecA.X, VecB.X);
        Result.Y = _mm256_mul_pd(VecA.Y, VecB.Y);
        Result.Z = _mm256_mul_pd(VecA.Z, VecB.Z);

        return Result;
    }

    INTERNAL zv3f64 
    operator*(zv3f64 Vec, v256d Value) 
    {
        zv3f64 Result = {};

        Result.X = _mm256_mul_pd(Vec.X, Value);
        Result.Y = _mm256_mul_pd(Vec.Y, Value);
        Result.Z = _mm256_mul_pd(Vec.Z, Value);

        return Result;
    }

    INTERNAL zv3f64 
    operator*(zv3f64 Vec, f64 Value) 
    {
        v256d ZValue = _mm256_set1_pd(Value);

        return Vec * ZValue;
    }

    INTERNAL zv3f64 
    operator/(zv3f64 VecA, zv3f64 VecB) 
    {
        zv3f64 Result = {};

        Result.X = _mm256_div_pd(VecA.X, VecB.X);
        Result.Y = _mm256_div_pd(VecA.Y, VecB.Y);
        Result.Z = _mm256_div_pd(VecA.Z, VecB.Z);

        return Result;
    }

    INTERNAL zv3f64
    operator/(zv3f64 Vec, v256d Value) 
    {
        zv3f64 Result = {};

        Result.X = _mm256_div_pd(Vec.X, Value);
        Result.Y = _mm256_div_pd(Vec.Y, Value);
        Result.Z = _mm256_div_pd(Vec.Z, Value);

        return Result;
    }

    INTERNAL zv3f64
    operator/(zv3f64 Vec, f64 Value) 
    {
        v256d ZValue = _mm256_set1_pd(Value);

        return Vec / ZValue;
    }

    INTERNAL zv3f64 
    operator&(zv3f64 VecA, zv3f64 VecB) 
    {
        zv3f64 Result = {};

        Result.X = _mm256_and_pd(VecA.X, VecB.X);
        Result.Y = _mm256_and_pd(VecA.Y, VecB.Y);
        Result.Z = _mm256_and_pd(VecA.Z, VecB.Z);

        return Result;
    }

    INTERNAL zv3f64 
    operator|(zv3f64 VecA, zv3f64 VecB) 
    {
        zv3f64 Result = {};

        Result.X = _mm256_or_pd(VecA.X, VecB.X);
        Result.Y = _mm256_or_pd(VecA.Y, VecB.Y);
        Result.Z = _mm256_or_pd(VecA.Z, VecB.Z);

        return Result;
    }

    INTERNAL zv3f64 
    operator^(zv3f64 VecA, zv3f64 VecB) 
    {
        zv3f64 Result = {};

        Result.X = _mm256_xor_pd(VecA.X, VecB.X);
        Result.Y = _mm256_xor_pd(VecA.Y, VecB.Y);
        Result.Z = _mm256_xor_pd(VecA.Z, VecB.Z);

        return Result;
    }

    INTERNAL v256d
    operator==(zv3f64 VecA, zv3f64 VecB) 
    {
        __mmask8 MaskX = _mm256_cmp_pd_mask(VecA.X, VecB.X, _CMP_EQ_OQ);
        __mmask8 MaskY = _mm256_cmp_pd_mask(VecA.Y, VecB.Y, _CMP_EQ_OQ);
        __mmask8 MaskZ = _mm256_cmp_pd_mask(VecA.Z, VecB.Z, _CMP_EQ_OQ);
        __mmask8 ResultMask = _kand_mask8(_kand_mask8(MaskX, MaskY), MaskZ);
        v256d Result = _mm256_mask_mov_pd(
            _mm256_setzero_pd(),
            ResultMask,
            _mm256_set1_pd(1.0)
        );

        return Result;
    }

    INTERNAL v256d
    operator!=(zv3f64 VecA, zv3f64 VecB) 
    {
        __mmask8 MaskX = _mm256_cmp_pd_mask(VecA.X, VecB.X, _CMP_NEQ_OQ);
        __mmask8 MaskY = _mm256_cmp_pd_mask(VecA.Y, VecB.Y, _CMP_NEQ_OQ);
        __mmask8 MaskZ = _mm256_cmp_pd_mask(VecA.Z, VecB.Z, _CMP_NEQ_OQ);
        __mmask8 ResultMask = _kand_mask8(_kand_mask8(MaskX, MaskY), MaskZ);
        v256d Result = _mm256_mask_mov_pd(
            _mm256_setzero_pd(),
            ResultMask,
            _mm256_set1_pd(1.0)
        );

        return Result;
    }

    INTERNAL v256d
    operator<(zv3f64 VecA, zv3f64 VecB) 
    {
        __mmask8 MaskX = _mm256_cmp_pd_mask(VecA.X, VecB.X, _CMP_LT_OQ);
        __mmask8 MaskY = _mm256_cmp_pd_mask(VecA.Y, VecB.Y, _CMP_LT_OQ);
        __mmask8 MaskZ = _mm256_cmp_pd_mask(VecA.Z, VecB.Z, _CMP_LT_OQ);
        __mmask8 ResultMask = _kand_mask8(_kand_mask8(MaskX, MaskY), MaskZ);
        v256d Result = _mm256_mask_mov_pd(
            _mm256_setzero_pd(),
            ResultMask,
            _mm256_set1_pd(1.0)
        );

        return Result;
    }

    INTERNAL v256d
    operator>(zv3f64 VecA, zv3f64 VecB) 
    {
        __mmask8 MaskX = _mm256_cmp_pd_mask(VecA.X, VecB.X, _CMP_GT_OQ);
        __mmask8 MaskY = _mm256_cmp_pd_mask(VecA.Y, VecB.Y, _CMP_GT_OQ);
        __mmask8 MaskZ = _mm256_cmp_pd_mask(VecA.Z, VecB.Z, _CMP_GT_OQ);
        __mmask8 ResultMask = _kand_mask8(_kand_mask8(MaskX, MaskY), MaskZ);
        v256d Result = _mm256_mask_mov_pd(
            _mm256_setzero_pd(),
            ResultMask,
            _mm256_set1_pd(1.0)
        );

        return Result;
    }

    INTERNAL v256d
    operator<=(zv3f64 VecA, zv3f64 VecB) 
    {
        __mmask8 MaskX = _mm256_cmp_pd_mask(VecA.X, VecB.X, _CMP_LE_OQ);
        __mmask8 MaskY = _mm256_cmp_pd_mask(VecA.Y, VecB.Y, _CMP_LE_OQ);
        __mmask8 MaskZ = _mm256_cmp_pd_mask(VecA.Z, VecB.Z, _CMP_LE_OQ);
        __mmask8 ResultMask = _kand_mask8(_kand_mask8(MaskX, MaskY), MaskZ);
        v256d Result = _mm256_mask_mov_pd(
            _mm256_setzero_pd(),
            ResultMask,
            _mm256_set1_pd(1.0)
        );

        return Result;
    }

    INTERNAL v256d
    operator>=(zv3f64 VecA, zv3f64 VecB) 
    {
        __mmask8 MaskX = _mm256_cmp_pd_mask(VecA.X, VecB.X, _CMP_GE_OQ);
        __mmask8 MaskY = _mm256_cmp_pd_mask(VecA.Y, VecB.Y, _CMP_GE_OQ);
        __mmask8 MaskZ = _mm256_cmp_pd_mask(VecA.Z, VecB.Z, _CMP_GE_OQ);
        __mmask8 ResultMask = _kand_mask8(_kand_mask8(MaskX, MaskY), MaskZ);
        v256d Result = _mm256_mask_mov_pd(
            _mm256_setzero_pd(),
            ResultMask,
            _mm256_set1_pd(1.0)
        );

        return Result;
    }

    INTERNAL v256d
    Dot(zv3f64 VecA, zv3f64 VecB)
    {
        v256d Result = {};

        Result = _mm256_fmadd_pd(
            VecA.X,
            VecB.X,
            _mm256_fmadd_pd(
                VecA.Y,
                VecB.Y,
                _mm256_mul_pd(VecA.Z, VecB.Z)
            )
        );

        return Result;
    }

    INTERNAL v256d
    LengthSq(zv3f64 Vec)
    {
        return Dot(Vec, Vec);
    }

    INTERNAL v256d
    Length(zv3f64 Vec)
    {
        return _mm256_sqrt_pd(LengthSq(Vec));
    }

    INTERNAL zv3f64
    Norm(zv3f64 Vec)
    {
        v256d Scale = _mm256_rsqrt14_pd(LengthSq(Vec));

        return Vec * Scale;
    }

    INTERNAL zv3f64
    Lerp(zv3f64 VecA, zv3f64 VecB, v256d T)
    {
        zv3f64 Result = {};

        Result.X = _mm256_fmadd_pd(T, VecB.X, _mm256_fnmadd_pd(T, VecA.X, VecA.X));
        Result.Y = _mm256_fmadd_pd(T, VecB.Y, _mm256_fnmadd_pd(T, VecA.Y, VecA.Y));
        Result.Z = _mm256_fmadd_pd(T, VecB.Z, _mm256_fnmadd_pd(T, VecA.Z, VecA.Z));

        return Result;
    }

    INTERNAL zv3f64
    Lerp(zv3f64 VecA, zv3f64 VecB, f64 T)
    {
        v256d ZT = _mm256_set1_pd(T);

        return Lerp(VecA, VecB, ZT);
    }

    INTERNAL zv3f64
    Cross(zv3f64 VecA, zv3f64 VecB)
    {
        zv3f64 Result = {};

        Result.X = _mm256_fmsub_pd(
            VecA.Y,
            VecB.Z,
            _mm256_mul_pd(VecA.Z, VecB.Y)
        );
        Result.Y = _mm256_fmsub_pd(
            VecA.Z,
            VecB.X,
            _mm256_mul_pd(VecA.X, VecB.Z)
        );
        Result.Z = _mm256_fmsub_pd(
            VecA.X,
            VecB.Y,
            _mm256_mul_pd(VecA.Y, VecB.X)
        );

        return Result;
    }

    INTERNAL zv4f32 
    operator+(zv4f32 VecA, zv4f32 VecB) 
    {
        zv4f32 Result = {};

        Result.X = _mm256_add_ps(VecA.X, VecB.X);
        Result.Y = _mm256_add_ps(VecA.Y, VecB.Y);
        Result.Z = _mm256_add_ps(VecA.Z, VecB.Z);
        Result.W = _mm256_add_ps(VecA.W, VecB.W);

        return Result;
    }

    INTERNAL zv4f32 
    operator-(zv4f32 VecA, zv4f32 VecB) 
    {
        zv4f32 Result = {};

        Result.X = _mm256_sub_ps(VecA.X, VecB.X);
        Result.Y = _mm256_sub_ps(VecA.Y, VecB.Y);
        Result.Z = _mm256_sub_ps(VecA.Z, VecB.Z);
        Result.W = _mm256_sub_ps(VecA.W, VecB.W);

        return Result;
    }

    INTERNAL zv4f32 
    operator*(zv4f32 VecA, zv4f32 VecB) 
    {
        zv4f32 Result = {};

        Result.X = _mm256_mul_ps(VecA.X, VecB.X);
        Result.Y = _mm256_mul_ps(VecA.Y, VecB.Y);
        Result.Z = _mm256_mul_ps(VecA.Z, VecB.Z);
        Result.W = _mm256_mul_ps(VecA.W, VecB.W);

        return Result;
    }

    INTERNAL zv4f32 
    operator*(zv4f32 Vec, v256f Value) 
    {
        zv4f32 Result = {};

        Result.X = _mm256_mul_ps(Vec.X, Value);
        Result.Y = _mm256_mul_ps(Vec.Y, Value);
        Result.Z = _mm256_mul_ps(Vec.Z, Value);
        Result.W = _mm256_mul_ps(Vec.W, Value);

        return Result;
    }

    INTERNAL zv4f32 
    operator*(zv4f32 Vec, f32 Value) 
    {
        v256f ZValue = _mm256_set1_ps(Value);

        return Vec * ZValue;
    }

    INTERNAL zv4f32 
    operator/(zv4f32 VecA, zv4f32 VecB) 
    {
        zv4f32 Result = {};

        Result.X = _mm256_div_ps(VecA.X, VecB.X);
        Result.Y = _mm256_div_ps(VecA.Y, VecB.Y);
        Result.Z = _mm256_div_ps(VecA.Z, VecB.Z);
        Result.W = _mm256_div_ps(VecA.W, VecB.W);

        return Result;
    }

    INTERNAL zv4f32 
    operator&(zv4f32 VecA, zv4f32 VecB) 
    {
        zv4f32 Result = {};

        Result.X = _mm256_and_ps(VecA.X, VecB.X);
        Result.Y = _mm256_and_ps(VecA.Y, VecB.Y);
        Result.Z = _mm256_and_ps(VecA.Z, VecB.Z);
        Result.W = _mm256_and_ps(VecA.W, VecB.W);

        return Result;
    }

    INTERNAL zv4f32 
    operator|(zv4f32 VecA, zv4f32 VecB) 
    {
        zv4f32 Result = {};

        Result.X = _mm256_or_ps(VecA.X, VecB.X);
        Result.Y = _mm256_or_ps(VecA.Y, VecB.Y);
        Result.Z = _mm256_or_ps(VecA.Z, VecB.Z);
        Result.W = _mm256_or_ps(VecA.W, VecB.W);

        return Result;
    }

    INTERNAL zv4f32 
    operator^(zv4f32 VecA, zv4f32 VecB) 
    {
        zv4f32 Result = {};

        Result.X = _mm256_xor_ps(VecA.X, VecB.X);
        Result.Y = _mm256_xor_ps(VecA.Y, VecB.Y);
        Result.Z = _mm256_xor_ps(VecA.Z, VecB.Z);
        Result.W = _mm256_xor_ps(VecA.W, VecB.W);

        return Result;
    }

    INTERNAL v256f
    operator==(zv4f32 VecA, zv4f32 VecB) 
    {
        __mmask8 MaskX = _mm256_cmp_ps_mask(VecA.X, VecB.X, _CMP_EQ_OQ);
        __mmask8 MaskY = _mm256_cmp_ps_mask(VecA.Y, VecB.Y, _CMP_EQ_OQ);
        __mmask8 MaskZ = _mm256_cmp_ps_mask(VecA.Z, VecB.Z, _CMP_EQ_OQ);
        __mmask8 MaskW = _mm256_cmp_ps_mask(VecA.W, VecB.W, _CMP_EQ_OQ);
        __mmask8 ResultMask = _kand_mask8(
            _kand_mask8(MaskX, MaskY), 
            _kand_mask8(MaskZ, MaskW)
        );
        v256f Result = _mm256_mask_mov_ps(
            _mm256_setzero_ps(),
            ResultMask,
            _mm256_set1_ps(1.0f)
        );

        return Result;
    }

    INTERNAL v256f
    operator!=(zv4f32 VecA, zv4f32 VecB) 
    {
        __mmask8 MaskX = _mm256_cmp_ps_mask(VecA.X, VecB.X, _CMP_NEQ_OQ);
        __mmask8 MaskY = _mm256_cmp_ps_mask(VecA.Y, VecB.Y, _CMP_NEQ_OQ);
        __mmask8 MaskZ = _mm256_cmp_ps_mask(VecA.Z, VecB.Z, _CMP_NEQ_OQ);
        __mmask8 MaskW = _mm256_cmp_ps_mask(VecA.W, VecB.W, _CMP_NEQ_OQ);
        __mmask8 ResultMask = _kand_mask8(
            _kand_mask8(MaskX, MaskY), 
            _kand_mask8(MaskZ, MaskW)
        );
        v256f Result = _mm256_mask_mov_ps(
            _mm256_setzero_ps(),
            ResultMask,
            _mm256_set1_ps(1.0f)
        );

        return Result;
    }

    INTERNAL v256f
    operator<(zv4f32 VecA, zv4f32 VecB) 
    {
        __mmask8 MaskX = _mm256_cmp_ps_mask(VecA.X, VecB.X, _CMP_LT_OQ);
        __mmask8 MaskY = _mm256_cmp_ps_mask(VecA.Y, VecB.Y, _CMP_LT_OQ);
        __mmask8 MaskZ = _mm256_cmp_ps_mask(VecA.Z, VecB.Z, _CMP_LT_OQ);
        __mmask8 MaskW = _mm256_cmp_ps_mask(VecA.W, VecB.W, _CMP_LT_OQ);
        __mmask8 ResultMask = _kand_mask8(
            _kand_mask8(MaskX, MaskY), 
            _kand_mask8(MaskZ, MaskW)
        );
        v256f Result = _mm256_mask_mov_ps(
            _mm256_setzero_ps(),
            ResultMask,
            _mm256_set1_ps(1.0f)
        );

        return Result;
    }

    INTERNAL v256f
    operator>(zv4f32 VecA, zv4f32 VecB) 
    {
        __mmask8 MaskX = _mm256_cmp_ps_mask(VecA.X, VecB.X, _CMP_GT_OQ);
        __mmask8 MaskY = _mm256_cmp_ps_mask(VecA.Y, VecB.Y, _CMP_GT_OQ);
        __mmask8 MaskZ = _mm256_cmp_ps_mask(VecA.Z, VecB.Z, _CMP_GT_OQ);
        __mmask8 MaskW = _mm256_cmp_ps_mask(VecA.W, VecB.W, _CMP_GT_OQ);
        __mmask8 ResultMask = _kand_mask8(
            _kand_mask8(MaskX, MaskY), 
            _kand_mask8(MaskZ, MaskW)
        );
        v256f Result = _mm256_mask_mov_ps(
            _mm256_setzero_ps(),
            ResultMask,
            _mm256_set1_ps(1.0f)
        );

        return Result;
    }

    INTERNAL v256f
    operator<=(zv4f32 VecA, zv4f32 VecB) 
    {
        __mmask8 MaskX = _mm256_cmp_ps_mask(VecA.X, VecB.X, _CMP_LE_OQ);
        __mmask8 MaskY = _mm256_cmp_ps_mask(VecA.Y, VecB.Y, _CMP_LE_OQ);
        __mmask8 MaskZ = _mm256_cmp_ps_mask(VecA.Z, VecB.Z, _CMP_LE_OQ);
        __mmask8 MaskW = _mm256_cmp_ps_mask(VecA.W, VecB.W, _CMP_LE_OQ);
        __mmask8 ResultMask = _kand_mask8(
            _kand_mask8(MaskX, MaskY), 
            _kand_mask8(MaskZ, MaskW)
        );
        v256f Result = _mm256_mask_mov_ps(
            _mm256_setzero_ps(),
            ResultMask,
            _mm256_set1_ps(1.0f)
        );

        return Result;
    }

    INTERNAL v256f
    operator>=(zv4f32 VecA, zv4f32 VecB) 
    {
        __mmask8 MaskX = _mm256_cmp_ps_mask(VecA.X, VecB.X, _CMP_GE_OQ);
        __mmask8 MaskY = _mm256_cmp_ps_mask(VecA.Y, VecB.Y, _CMP_GE_OQ);
        __mmask8 MaskZ = _mm256_cmp_ps_mask(VecA.Z, VecB.Z, _CMP_GE_OQ);
        __mmask8 MaskW = _mm256_cmp_ps_mask(VecA.W, VecB.W, _CMP_GE_OQ);
        __mmask8 ResultMask = _kand_mask8(
            _kand_mask8(MaskX, MaskY), 
            _kand_mask8(MaskZ, MaskW)
        );
        v256f Result = _mm256_mask_mov_ps(
            _mm256_setzero_ps(),
            ResultMask,
            _mm256_set1_ps(1.0f)
        );

        return Result;
    }

    INTERNAL v256f
    Dot(zv4f32 VecA, zv4f32 VecB)
    {
        v256f Result = {};

        Result = _mm256_fmadd_ps(
            VecA.X,
            VecB.X,
            _mm256_fmadd_ps(
                VecA.Y,
                VecB.Y,
                _mm256_fmadd_ps(
                    VecA.Z,
                    VecB.Z,
                    _mm256_mul_ps(VecA.W, VecB.W)
                )
            )
        );

        return Result;
    }

    INTERNAL v256f
    LengthSq(zv4f32 Vec)
    {
        return Dot(Vec, Vec);
    }

    INTERNAL v256f
    Length(zv4f32 Vec)
    {
        return _mm256_sqrt_ps(LengthSq(Vec));
    }

    INTERNAL zv4f32
    Norm(zv4f32 Vec)
    {
        v256f Scale = _mm256_rsqrt14_ps(LengthSq(Vec));

        return Vec * Scale;
    }

    INTERNAL zv4f32
    Lerp(zv4f32 VecA, zv4f32 VecB, v256f T)
    {
        zv4f32 Result = {};

        Result.X = _mm256_fmadd_ps(T, VecB.X, _mm256_fnmadd_ps(T, VecA.X, VecA.X));
        Result.Y = _mm256_fmadd_ps(T, VecB.Y, _mm256_fnmadd_ps(T, VecA.Y, VecA.Y));
        Result.Z = _mm256_fmadd_ps(T, VecB.Z, _mm256_fnmadd_ps(T, VecA.Z, VecA.Z));
        Result.W = _mm256_fmadd_ps(T, VecB.W, _mm256_fnmadd_ps(T, VecA.W, VecA.W));

        return Result;
    }

    INTERNAL zv4f32
    Lerp(zv4f32 VecA, zv4f32 VecB, f32 T)
    {
        v256f ZT = _mm256_set1_ps(T);

        return Lerp(VecA, VecB, ZT);
    }

    INTERNAL zv4i32 
    operator+(zv4i32 VecA, zv4i32 VecB) 
    {
        zv4i32 Result = {};

        Result.X = _mm256_add_epi32(VecA.X, VecB.X);
        Result.Y = _mm256_add_epi32(VecA.Y, VecB.Y);
        Result.Z = _mm256_add_epi32(VecA.Z, VecB.Z);
        Result.W = _mm256_add_epi32(VecA.W, VecB.W);

        return Result;
    }

    INTERNAL zv4i32 
    operator-(zv4i32 VecA, zv4i32 VecB) 
    {
        zv4i32 Result = {};

        Result.X = _mm256_sub_epi32(VecA.X, VecB.X);
        Result.Y = _mm256_sub_epi32(VecA.Y, VecB.Y);
        Result.Z = _mm256_sub_epi32(VecA.Z, VecB.Z);
        Result.W = _mm256_sub_epi32(VecA.W, VecB.W);

        return Result;
    }

    INTERNAL zv4i32 
    operator*(zv4i32 VecA, zv4i32 VecB) 
    {
        zv4i32 Result = {};

        Result.X = _mm256_mullo_epi32(VecA.X, VecB.X);
        Result.Y = _mm256_mullo_epi32(VecA.Y, VecB.Y);
        Result.Z = _mm256_mullo_epi32(VecA.Z, VecB.Z);
        Result.W = _mm256_mullo_epi32(VecA.W, VecB.W);

        return Result;
    }

    INTERNAL zv4i32 
    operator*(zv4i32 Vec, v256 Value) 
    {
        zv4i32 Result = {};

        Result.X = _mm256_mullo_epi32(Vec.X, Value);
        Result.Y = _mm256_mullo_epi32(Vec.Y, Value);
        Result.Z = _mm256_mullo_epi32(Vec.Z, Value);
        Result.W = _mm256_mullo_epi32(Vec.W, Value);

        return Result;
    }

    INTERNAL zv4i32 
    operator*(zv4i32 Vec, i32 Value) 
    {
        v256 ZValue = _mm256_set1_epi32(Value);

        return Vec * ZValue;
    }

    INTERNAL zv4i32 
    operator/(zv4i32 VecA, zv4i32 VecB) 
    {
        zv4i32 Result = {};
        v256f FloatAX = _mm256_cvtepi32_ps(VecA.X);
        v256f FloatAY = _mm256_cvtepi32_ps(VecA.Y);
        v256f FloatAZ = _mm256_cvtepi32_ps(VecA.Z);
        v256f FloatAW = _mm256_cvtepi32_ps(VecA.W);
        v256f FloatBX = _mm256_cvtepi32_ps(VecB.X);
        v256f FloatBY = _mm256_cvtepi32_ps(VecB.Y);
        v256f FloatBZ = _mm256_cvtepi32_ps(VecB.Z);
        v256f FloatBW = _mm256_cvtepi32_ps(VecB.W);
        v256f QuotientX = _mm256_div_ps(FloatAX, FloatBX);
        v256f QuotientY = _mm256_div_ps(FloatAY, FloatBY);
        v256f QuotientZ = _mm256_div_ps(FloatAZ, FloatBZ);
        v256f QuotientW = _mm256_div_ps(FloatAW, FloatBW);

        Result.X = _mm256_cvtps_epi32(QuotientX);
        Result.Y = _mm256_cvtps_epi32(QuotientY);
        Result.Z = _mm256_cvtps_epi32(QuotientZ);
        Result.W = _mm256_cvtps_epi32(QuotientW);

        return Result;
    }

    INTERNAL zv4i32 
    operator&(zv4i32 VecA, zv4i32 VecB) 
    {
        zv4i32 Result = {};

        Result.X = _mm256_and_epi32(VecA.X, VecB.X);
        Result.Y = _mm256_and_epi32(VecA.Y, VecB.Y);
        Result.Z = _mm256_and_epi32(VecA.Z, VecB.Z);
        Result.W = _mm256_and_epi32(VecA.W, VecB.W);

        return Result;
    }

    INTERNAL zv4i32 
    operator|(zv4i32 VecA, zv4i32 VecB) 
    {
        zv4i32 Result = {};

        Result.X = _mm256_or_epi32(VecA.X, VecB.X);
        Result.Y = _mm256_or_epi32(VecA.Y, VecB.Y);
        Result.Z = _mm256_or_epi32(VecA.Z, VecB.Z);
        Result.W = _mm256_or_epi32(VecA.W, VecB.W);

        return Result;
    }

    INTERNAL zv4i32 
    operator^(zv4i32 VecA, zv4i32 VecB) 
    {
        zv4i32 Result = {};

        Result.X = _mm256_xor_epi32(VecA.X, VecB.X);
        Result.Y = _mm256_xor_epi32(VecA.Y, VecB.Y);
        Result.Z = _mm256_xor_epi32(VecA.Z, VecB.Z);
        Result.W = _mm256_xor_epi32(VecA.W, VecB.W);

        return Result;
    }

    INTERNAL v256 
    operator==(zv4i32 VecA, zv4i32 VecB) 
    {
        __mmask8 MaskX = _mm256_cmp_epi32_mask(VecA.X, VecB.X, _MM_CMPINT_EQ);
        __mmask8 MaskY = _mm256_cmp_epi32_mask(VecA.Y, VecB.Y, _MM_CMPINT_EQ);
        __mmask8 MaskZ = _mm256_cmp_epi32_mask(VecA.Z, VecB.Z, _MM_CMPINT_EQ);
        __mmask8 MaskW = _mm256_cmp_epi32_mask(VecA.W, VecB.W, _MM_CMPINT_EQ);
        __mmask8 ResultMask = _kand_mask8(
            _kand_mask8(MaskX, MaskY), 
            _kand_mask8(MaskZ, MaskW)
        );
        v256 Result = _mm256_mask_mov_epi32(
            _mm256_setzero_si256(),
            ResultMask,
            _mm256_set1_epi32(~0)
        );

        return Result;
    }

    INTERNAL v256 
    operator!=(zv4i32 VecA, zv4i32 VecB) 
    {
        __mmask8 MaskX = _mm256_cmp_epi32_mask(VecA.X, VecB.X, _MM_CMPINT_NE);
        __mmask8 MaskY = _mm256_cmp_epi32_mask(VecA.Y, VecB.Y, _MM_CMPINT_NE);
        __mmask8 MaskZ = _mm256_cmp_epi32_mask(VecA.Z, VecB.Z, _MM_CMPINT_NE);
        __mmask8 MaskW = _mm256_cmp_epi32_mask(VecA.W, VecB.W, _MM_CMPINT_NE);
        __mmask8 ResultMask = _kand_mask8(
            _kand_mask8(MaskX, MaskY), 
            _kand_mask8(MaskZ, MaskW)
        );
        v256 Result = _mm256_mask_mov_epi32(
            _mm256_setzero_si256(),
            ResultMask,
            _mm256_set1_epi32(~0)
        );

        return Result;
    }

    INTERNAL v256 
    operator<(zv4i32 VecA, zv4i32 VecB) 
    {
        __mmask8 MaskX = _mm256_cmp_epi32_mask(VecA.X, VecB.X, _MM_CMPINT_LT);
        __mmask8 MaskY = _mm256_cmp_epi32_mask(VecA.Y, VecB.Y, _MM_CMPINT_LT);
        __mmask8 MaskZ = _mm256_cmp_epi32_mask(VecA.Z, VecB.Z, _MM_CMPINT_LT);
        __mmask8 MaskW = _mm256_cmp_epi32_mask(VecA.W, VecB.W, _MM_CMPINT_LT);
        __mmask8 ResultMask = _kand_mask8(
            _kand_mask8(MaskX, MaskY), 
            _kand_mask8(MaskZ, MaskW)
        );
        v256 Result = _mm256_mask_mov_epi32(
            _mm256_setzero_si256(),
            ResultMask,
            _mm256_set1_epi32(~0)
        );

        return Result;
    }

    INTERNAL v256 
    operator>(zv4i32 VecA, zv4i32 VecB) 
    {
        __mmask8 MaskX = _mm256_cmp_epi32_mask(VecA.X, VecB.X, _MM_CMPINT_GT);
        __mmask8 MaskY = _mm256_cmp_epi32_mask(VecA.Y, VecB.Y, _MM_CMPINT_GT);
        __mmask8 MaskZ = _mm256_cmp_epi32_mask(VecA.Z, VecB.Z, _MM_CMPINT_GT);
        __mmask8 MaskW = _mm256_cmp_epi32_mask(VecA.W, VecB.W, _MM_CMPINT_GT);
        __mmask8 ResultMask = _kand_mask8(
            _kand_mask8(MaskX, MaskY), 
            _kand_mask8(MaskZ, MaskW)
        );
        v256 Result = _mm256_mask_mov_epi32(
            _mm256_setzero_si256(),
            ResultMask,
            _mm256_set1_epi32(~0)
        );

        return Result;
    }

    INTERNAL v256 
    operator<=(zv4i32 VecA, zv4i32 VecB) 
    {
        __mmask8 MaskX = _mm256_cmp_epi32_mask(VecA.X, VecB.X, _MM_CMPINT_LE);
        __mmask8 MaskY = _mm256_cmp_epi32_mask(VecA.Y, VecB.Y, _MM_CMPINT_LE);
        __mmask8 MaskZ = _mm256_cmp_epi32_mask(VecA.Z, VecB.Z, _MM_CMPINT_LE);
        __mmask8 MaskW = _mm256_cmp_epi32_mask(VecA.W, VecB.W, _MM_CMPINT_LE);
        __mmask8 ResultMask = _kand_mask8(
            _kand_mask8(MaskX, MaskY), 
            _kand_mask8(MaskZ, MaskW)
        );
        v256 Result = _mm256_mask_mov_epi32(
            _mm256_setzero_si256(),
            ResultMask,
            _mm256_set1_epi32(~0)
        );

        return Result;
    }

    INTERNAL v256 
    operator>=(zv4i32 VecA, zv4i32 VecB) 
    {
        __mmask8 MaskX = _mm256_cmp_epi32_mask(VecA.X, VecB.X, _MM_CMPINT_GE);
        __mmask8 MaskY = _mm256_cmp_epi32_mask(VecA.Y, VecB.Y, _MM_CMPINT_GE);
        __mmask8 MaskZ = _mm256_cmp_epi32_mask(VecA.Z, VecB.Z, _MM_CMPINT_GE);
        __mmask8 MaskW = _mm256_cmp_epi32_mask(VecA.W, VecB.W, _MM_CMPINT_GE);
        __mmask8 ResultMask = _kand_mask8(
            _kand_mask8(MaskX, MaskY), 
            _kand_mask8(MaskZ, MaskW)
        );
        v256 Result = _mm256_mask_mov_epi32(
            _mm256_setzero_si256(),
            ResultMask,
            _mm256_set1_epi32(~0)
        );

        return Result;
    }

    INTERNAL v256
    Dot(zv4i32 VecA, zv4i32 VecB)
    {
        v256 Result = {};

        Result = _mm256_add_epi32(
            _mm256_add_epi32(
                _mm256_mullo_epi32(VecA.X, VecB.X),
                _mm256_mullo_epi32(VecA.Y, VecB.Y)
            ),
            _mm256_add_epi32(
                _mm256_mullo_epi32(VecA.Z, VecB.Z),
                _mm256_mullo_epi32(VecA.W, VecB.W)
            )
        );

        return Result;
    }

    INTERNAL v256
    LengthSq(zv4i32 Vec)
    {
        return Dot(Vec, Vec);
    }

    INTERNAL v256
    Length(zv4i32 Vec)
    {
        v256f LenSq = _mm256_cvtepi32_ps(LengthSq(Vec));
        
        return _mm256_cvtps_epi32(
            _mm256_sqrt_ps(LenSq)
        );
    }

    INTERNAL zv4i32
    Norm(zv4i32 Vec)
    {
        v256 Scale = _mm256_cvtps_epi32(
            _mm256_rsqrt14_ps(
                _mm256_cvtepi32_ps(
                    LengthSq(Vec)
                )
            )
        );

        return Vec * Scale;
    }

    INTERNAL zv4i32
    Lerp(zv4i32 VecA, zv4i32 VecB, v256 T)
    {
        zv4i32 Result = {};
        v256f AX = _mm256_cvtepi32_ps(VecA.X);
        v256f AY = _mm256_cvtepi32_ps(VecA.Y);
        v256f AZ = _mm256_cvtepi32_ps(VecA.Z);
        v256f AW = _mm256_cvtepi32_ps(VecA.W);
        v256f BX = _mm256_cvtepi32_ps(VecB.X);
        v256f BY = _mm256_cvtepi32_ps(VecB.Y);
        v256f BZ = _mm256_cvtepi32_ps(VecB.Z);
        v256f BW = _mm256_cvtepi32_ps(VecB.W);
        v256f ZT = _mm256_cvtepi32_ps(T);
        v256f ResultX = _mm256_fmadd_ps(
            ZT,
            BX,
            _mm256_fnmadd_ps(
                ZT,
                AX,
                AX
            )
        );
        v256f ResultY = _mm256_fmadd_ps(
            ZT,
            BY,
            _mm256_fnmadd_ps(
                ZT,
                AY,
                AY
            )
        );
        v256f ResultZ = _mm256_fmadd_ps(
            ZT,
            BZ,
            _mm256_fnmadd_ps(
                ZT,
                AZ,
                AZ
            )
        );
        v256f ResultW = _mm256_fmadd_ps(
            ZT,
            BW,
            _mm256_fnmadd_ps(
                ZT,
                AW,
                AW
            )
        );

        Result.X = _mm256_cvtps_epi32(ResultX);
        Result.Y = _mm256_cvtps_epi32(ResultY);
        Result.Z = _mm256_cvtps_epi32(ResultZ);
        Result.W = _mm256_cvtps_epi32(ResultW);

        return Result;
    }

    INTERNAL zv4i32
    Lerp(zv4i32 VecA, zv4i32 VecB, i32 T)
    {
        v256 ZT = _mm256_set1_epi32(T);

        return Lerp(VecA, VecB, ZT);
    }

    INTERNAL zv4f64 
    operator+(zv4f64 VecA, zv4f64 VecB) 
    {
        zv4f64 Result = {};

        Result.X = _mm256_add_pd(VecA.X, VecB.X);
        Result.Y = _mm256_add_pd(VecA.Y, VecB.Y);
        Result.Z = _mm256_add_pd(VecA.Z, VecB.Z);
        Result.W = _mm256_add_pd(VecA.W, VecB.W);

        return Result;
    }

    INTERNAL zv4f64 
    operator-(zv4f64 VecA, zv4f64 VecB) 
    {
        zv4f64 Result = {};

        Result.X = _mm256_sub_pd(VecA.X, VecB.X);
        Result.Y = _mm256_sub_pd(VecA.Y, VecB.Y);
        Result.Z = _mm256_sub_pd(VecA.Z, VecB.Z);
        Result.W = _mm256_sub_pd(VecA.W, VecB.W);

        return Result;
    }

    INTERNAL zv4f64 
    operator*(zv4f64 VecA, zv4f64 VecB) 
    {
        zv4f64 Result = {};

        Result.X = _mm256_mul_pd(VecA.X, VecB.X);
        Result.Y = _mm256_mul_pd(VecA.Y, VecB.Y);
        Result.Z = _mm256_mul_pd(VecA.Z, VecB.Z);
        Result.W = _mm256_mul_pd(VecA.W, VecB.W);

        return Result;
    }

    INTERNAL zv4f64
    operator*(zv4f64 Vec, v256d Value) 
    {
        zv4f64 Result = {};

        Result.X = _mm256_mul_pd(Vec.X, Value);
        Result.Y = _mm256_mul_pd(Vec.Y, Value);
        Result.Z = _mm256_mul_pd(Vec.Z, Value);
        Result.W = _mm256_mul_pd(Vec.W, Value);

        return Result;
    }

    INTERNAL zv4f64
    operator*(zv4f64 Vec, f64 Value) 
    {
        v256d ZValue = _mm256_set1_pd(Value);

        return Vec * ZValue;
    }

    INTERNAL zv4f64 
    operator/(zv4f64 VecA, zv4f64 VecB) 
    {
        zv4f64 Result = {};

        Result.X = _mm256_div_pd(VecA.X, VecB.X);
        Result.Y = _mm256_div_pd(VecA.Y, VecB.Y);
        Result.Z = _mm256_div_pd(VecA.Z, VecB.Z);
        Result.W = _mm256_div_pd(VecA.W, VecB.W);

        return Result;
    }

    INTERNAL zv4f64 
    operator&(zv4f64 VecA, zv4f64 VecB) 
    {
        zv4f64 Result = {};

        Result.X = _mm256_and_pd(VecA.X, VecB.X);
        Result.Y = _mm256_and_pd(VecA.Y, VecB.Y);
        Result.Z = _mm256_and_pd(VecA.Z, VecB.Z);
        Result.W = _mm256_and_pd(VecA.W, VecB.W);

        return Result;
    }

    INTERNAL zv4f64 
    operator|(zv4f64 VecA, zv4f64 VecB) 
    {
        zv4f64 Result = {};

        Result.X = _mm256_or_pd(VecA.X, VecB.X);
        Result.Y = _mm256_or_pd(VecA.Y, VecB.Y);
        Result.Z = _mm256_or_pd(VecA.Z, VecB.Z);
        Result.W = _mm256_or_pd(VecA.W, VecB.W);

        return Result;
    }

    INTERNAL zv4f64 
    operator^(zv4f64 VecA, zv4f64 VecB) 
    {
        zv4f64 Result = {};

        Result.X = _mm256_xor_pd(VecA.X, VecB.X);
        Result.Y = _mm256_xor_pd(VecA.Y, VecB.Y);
        Result.Z = _mm256_xor_pd(VecA.Z, VecB.Z);
        Result.W = _mm256_xor_pd(VecA.W, VecB.W);

        return Result;
    }

    INTERNAL v256d 
    operator==(zv4f64 VecA, zv4f64 VecB) 
    {
        __mmask8 MaskX = _mm256_cmp_pd_mask(VecA.X, VecB.X, _CMP_EQ_OQ);
        __mmask8 MaskY = _mm256_cmp_pd_mask(VecA.Y, VecB.Y, _CMP_EQ_OQ);
        __mmask8 MaskZ = _mm256_cmp_pd_mask(VecA.Z, VecB.Z, _CMP_EQ_OQ);
        __mmask8 MaskW = _mm256_cmp_pd_mask(VecA.W, VecB.W, _CMP_EQ_OQ);
        __mmask8 ResultMask = _kand_mask8(
            _kand_mask8(MaskX, MaskY), 
            _kand_mask8(MaskZ, MaskW)
        );
        v256d Result = _mm256_mask_mov_pd(
            _mm256_setzero_pd(),
            ResultMask,
            _mm256_set1_pd(1.0)
        );

        return Result;
    }

    INTERNAL v256d 
    operator!=(zv4f64 VecA, zv4f64 VecB) 
    {
        __mmask8 MaskX = _mm256_cmp_pd_mask(VecA.X, VecB.X, _CMP_NEQ_OQ);
        __mmask8 MaskY = _mm256_cmp_pd_mask(VecA.Y, VecB.Y, _CMP_NEQ_OQ);
        __mmask8 MaskZ = _mm256_cmp_pd_mask(VecA.Z, VecB.Z, _CMP_NEQ_OQ);
        __mmask8 MaskW = _mm256_cmp_pd_mask(VecA.W, VecB.W, _CMP_NEQ_OQ);
        __mmask8 ResultMask = _kand_mask8(
            _kand_mask8(MaskX, MaskY), 
            _kand_mask8(MaskZ, MaskW)
        );
        v256d Result = _mm256_mask_mov_pd(
            _mm256_setzero_pd(),
            ResultMask,
            _mm256_set1_pd(1.0)
        );

        return Result;
    }

    INTERNAL v256d 
    operator<(zv4f64 VecA, zv4f64 VecB) 
    {
        __mmask8 MaskX = _mm256_cmp_pd_mask(VecA.X, VecB.X, _CMP_LT_OQ);
        __mmask8 MaskY = _mm256_cmp_pd_mask(VecA.Y, VecB.Y, _CMP_LT_OQ);
        __mmask8 MaskZ = _mm256_cmp_pd_mask(VecA.Z, VecB.Z, _CMP_LT_OQ);
        __mmask8 MaskW = _mm256_cmp_pd_mask(VecA.W, VecB.W, _CMP_LT_OQ);
        __mmask8 ResultMask = _kand_mask8(
            _kand_mask8(MaskX, MaskY), 
            _kand_mask8(MaskZ, MaskW)
        );
        v256d Result = _mm256_mask_mov_pd(
            _mm256_setzero_pd(),
            ResultMask,
            _mm256_set1_pd(1.0)
        );

        return Result;
    }

    INTERNAL v256d 
    operator>(zv4f64 VecA, zv4f64 VecB) 
    {
        __mmask8 MaskX = _mm256_cmp_pd_mask(VecA.X, VecB.X, _CMP_GT_OQ);
        __mmask8 MaskY = _mm256_cmp_pd_mask(VecA.Y, VecB.Y, _CMP_GT_OQ);
        __mmask8 MaskZ = _mm256_cmp_pd_mask(VecA.Z, VecB.Z, _CMP_GT_OQ);
        __mmask8 MaskW = _mm256_cmp_pd_mask(VecA.W, VecB.W, _CMP_GT_OQ);
        __mmask8 ResultMask = _kand_mask8(
            _kand_mask8(MaskX, MaskY), 
            _kand_mask8(MaskZ, MaskW)
        );
        v256d Result = _mm256_mask_mov_pd(
            _mm256_setzero_pd(),
            ResultMask,
            _mm256_set1_pd(1.0)
        );

        return Result;
    }

    INTERNAL v256d 
    operator<=(zv4f64 VecA, zv4f64 VecB) 
    {
        __mmask8 MaskX = _mm256_cmp_pd_mask(VecA.X, VecB.X, _CMP_LE_OQ);
        __mmask8 MaskY = _mm256_cmp_pd_mask(VecA.Y, VecB.Y, _CMP_LE_OQ);
        __mmask8 MaskZ = _mm256_cmp_pd_mask(VecA.Z, VecB.Z, _CMP_LE_OQ);
        __mmask8 MaskW = _mm256_cmp_pd_mask(VecA.W, VecB.W, _CMP_LE_OQ);
        __mmask8 ResultMask = _kand_mask8(
            _kand_mask8(MaskX, MaskY), 
            _kand_mask8(MaskZ, MaskW)
        );
        v256d Result = _mm256_mask_mov_pd(
            _mm256_setzero_pd(),
            ResultMask,
            _mm256_set1_pd(1.0)
        );

        return Result;
    }

    INTERNAL v256d 
    operator>=(zv4f64 VecA, zv4f64 VecB) 
    {
        __mmask8 MaskX = _mm256_cmp_pd_mask(VecA.X, VecB.X, _CMP_GE_OQ);
        __mmask8 MaskY = _mm256_cmp_pd_mask(VecA.Y, VecB.Y, _CMP_GE_OQ);
        __mmask8 MaskZ = _mm256_cmp_pd_mask(VecA.Z, VecB.Z, _CMP_GE_OQ);
        __mmask8 MaskW = _mm256_cmp_pd_mask(VecA.W, VecB.W, _CMP_GE_OQ);
        __mmask8 ResultMask = _kand_mask8(
            _kand_mask8(MaskX, MaskY), 
            _kand_mask8(MaskZ, MaskW)
        );
        v256d Result = _mm256_mask_mov_pd(
            _mm256_setzero_pd(),
            ResultMask,
            _mm256_set1_pd(1.0)
        );

        return Result;
    }

    INTERNAL v256d
    Dot(zv4f64 VecA, zv4f64 VecB)
    {
        v256d Result = {};

        Result = _mm256_fmadd_pd(
            VecA.X,
            VecB.X,
            _mm256_fmadd_pd(
                VecA.Y,
                VecB.Y,
                _mm256_fmadd_pd(
                    VecA.Z,
                    VecB.Z,
                    _mm256_mul_pd(VecA.W, VecB.W)
                )
            )
        );

        return Result;
    }

    INTERNAL v256d
    LengthSq(zv4f64 Vec)
    {
        return Dot(Vec, Vec);
    }

    INTERNAL v256d
    Length(zv4f64 Vec)
    {
        return _mm256_sqrt_pd(LengthSq(Vec));
    }

    INTERNAL zv4f64
    Norm(zv4f64 Vec)
    {
        v256d Scale = _mm256_rsqrt14_pd(LengthSq(Vec));

        return Vec * Scale;
    }

    INTERNAL zv4f64
    Lerp(zv4f64 VecA, zv4f64 VecB, v256d T)
    {
        zv4f64 Result = {};

        Result.X = _mm256_fmadd_pd(T, VecB.X, _mm256_fnmadd_pd(T, VecA.X, VecA.X));
        Result.Y = _mm256_fmadd_pd(T, VecB.Y, _mm256_fnmadd_pd(T, VecA.Y, VecA.Y));
        Result.Z = _mm256_fmadd_pd(T, VecB.Z, _mm256_fnmadd_pd(T, VecA.Z, VecA.Z));
        Result.W = _mm256_fmadd_pd(T, VecB.W, _mm256_fnmadd_pd(T, VecA.W, VecA.W));

        return Result;
    }

    INTERNAL zv4f64
    Lerp(zv4f64 VecA, zv4f64 VecB, f64 T)
    {
        v256d ZT = _mm256_set1_pd(T);

        return Lerp(VecA, VecB, ZT);
    }

    INTERNAL zr1i32
    Rng(v256 Min, v256 Max)
    {
        zr1i32 Result = {};

        Result.Min = Min;
        Result.Max = Max;

        return Result;
    }

    INTERNAL zr1i32
    Shift(zr1i32 Range, v256 Val)
    {
        zr1i32 Result = {};

        Result.Min = _mm256_add_epi32(Range.Min, Val);
        Result.Max = _mm256_add_epi32(Range.Max, Val);

        return Result;
    }

    INTERNAL zr1i32
    Widen(zr1i32 Range, v256 Val)
    {
        zr1i32 Result = {};

        Result.Min = _mm256_sub_epi32(Range.Min, Val);
        Result.Max = _mm256_add_epi32(Range.Max, Val);

        return Result;
    }

    INTERNAL v256
    Centre(zr1i32 Range)
    {
        v256f Float = _mm256_cvtepi32_ps(
            _mm256_add_epi32(Range.Min, Range.Max)
        );
        v256f Quotient = _mm256_div_ps(Float, _mm256_set1_ps(2.0f));

        return _mm256_cvtps_epi32(Quotient);
    }

    INTERNAL v256
    InRange(zr1i32 Range, v256 Val)
    {
        __mmask8 MinMask = _mm256_cmp_epi32_mask(Range.Min, Val, _MM_CMPINT_GE);
        __mmask8 MaxMask = _mm256_cmp_epi32_mask(Val, Range.Max, _MM_CMPINT_LT);
        __mmask8 ResultMask = _kand_mask8(MinMask, MaxMask);

        return _mm256_mask_mov_epi32(
            _mm256_setzero_si256(),
            ResultMask,
            _mm256_set1_epi32(1)
        );
    }

    INTERNAL v256
    Length(zr1i32 Range)
    {
        return _mm256_sub_epi32(Range.Max, Range.Min);
    }

    INTERNAL zr1i32
    Union(zr1i32 RangeA, zr1i32 RangeB)
    {
        zr1i32 Result = {};

        Result.Min = _mm256_min_epi32(RangeA.Min, RangeB.Min);
        Result.Max = _mm256_max_epi32(RangeA.Max, RangeB.Max);

        return Result;
    }

    INTERNAL zr1i32
    Intersect(zr1i32 RangeA, zr1i32 RangeB)
    {
        zr1i32 Result = {};

        Result.Min = _mm256_max_epi32(RangeA.Min, RangeB.Min);
        Result.Max = _mm256_min_epi32(RangeA.Max, RangeB.Max);

        return Result;
    }

    INTERNAL v256
    Clamp(zr1i32 Range, v256 Val)
    {
        Val = _mm256_max_epi32(Range.Min, Val);
        Val = _mm256_min_epi32(Range.Max, Val);

        return Val;
    }

    INTERNAL zr1f32
    Rng(v256f Min, v256f Max)
    {
        zr1f32 Result = {};

        Result.Min = Min;
        Result.Max = Max;

        return Result;
    }

    INTERNAL zr1f32
    Shift(zr1f32 Range, v256f Val)
    {
        zr1f32 Result = {};

        Result.Min = _mm256_add_ps(Range.Min, Val);
        Result.Max = _mm256_add_ps(Range.Max, Val);

        return Result;
    }

    INTERNAL zr1f32
    Widen(zr1f32 Range, v256f Val)
    {
        zr1f32 Result = {};

        Result.Min = _mm256_sub_ps(Range.Min, Val);
        Result.Max = _mm256_add_ps(Range.Max, Val);

        return Result;
    }

    INTERNAL v256f
    Centre(zr1f32 Range)
    {
        return _mm256_mul_ps(
            _mm256_add_ps(Range.Min, Range.Max), 
            _mm256_set1_ps(0.5f)
        );
    }

    INTERNAL v256f
    InRange(zr1f32 Range, v256f Val)
    {
        __mmask8 MinMask = _mm256_cmp_ps_mask(Range.Min, Val, _CMP_GE_OQ);
        __mmask8 MaxMask = _mm256_cmp_ps_mask(Val, Range.Max, _CMP_LT_OQ);
        __mmask8 ResultMask = _kand_mask8(MinMask, MaxMask);

        return _mm256_mask_mov_ps(
            _mm256_setzero_ps(),
            ResultMask,
            _mm256_set1_ps(1.0f)
        );
    }

    INTERNAL v256f
    Length(zr1f32 Range)
    {
        return _mm256_sub_ps(Range.Max, Range.Min);
    }

    INTERNAL zr1f32
    Union(zr1f32 RangeA, zr1f32 RangeB)
    {
        zr1f32 Result = {};

        Result.Min = _mm256_min_ps(RangeA.Min, RangeB.Min);
        Result.Max = _mm256_max_ps(RangeA.Max, RangeB.Max);

        return Result;
    }

    INTERNAL zr1f32
    Intersect(zr1f32 RangeA, zr1f32 RangeB)
    {
        zr1f32 Result = {};

        Result.Min = _mm256_max_ps(RangeA.Min, RangeB.Min);
        Result.Max = _mm256_min_ps(RangeA.Max, RangeB.Max);

        return Result;
    }

    INTERNAL v256f
    Clamp(zr1f32 Range, v256f Val)
    {
        Val = _mm256_max_ps(Range.Min, Val);
        Val = _mm256_min_ps(Range.Max, Val);

        return Val;
    }

    INTERNAL zr1f64
    Rng(v256d Min, v256d Max)
    {
        zr1f64 Result = {};

        Result.Min = Min;
        Result.Max = Max;

        return Result;
    }

    INTERNAL zr1f64
    Shift(zr1f64 Range, v256d Val)
    {
        zr1f64 Result = {};

        Result.Min = _mm256_add_pd(Range.Min, Val);
        Result.Max = _mm256_add_pd(Range.Max, Val);

        return Result;
    }

    INTERNAL zr1f64
    Widen(zr1f64 Range, v256d Val)
    {
        zr1f64 Result = {};

        Result.Min = _mm256_sub_pd(Range.Min, Val);
        Result.Max = _mm256_add_pd(Range.Max, Val);

        return Result;
    }

    INTERNAL v256d
    Centre(zr1f64 Range)
    {
        return _mm256_mul_pd(
            _mm256_add_pd(Range.Min, Range.Max), 
            _mm256_set1_pd(0.5)
        );
    }

    INTERNAL v256d
    InRange(zr1f64 Range, v256d Val)
    {
        __mmask8 MinMask = _mm256_cmp_pd_mask(Range.Min, Val, _CMP_GE_OQ);
        __mmask8 MaxMask = _mm256_cmp_pd_mask(Val, Range.Max, _CMP_LT_OQ);
        __mmask8 ResultMask = _kand_mask8(MinMask, MaxMask);

        return _mm256_mask_mov_pd(
            _mm256_setzero_pd(),
            ResultMask,
            _mm256_set1_pd(1.0)
        );
    }

    INTERNAL v256d
    Length(zr1f64 Range)
    {
        return _mm256_sub_pd(Range.Max, Range.Min);
    }

    INTERNAL zr1f64
    Union(zr1f64 RangeA, zr1f64 RangeB)
    {
        zr1f64 Result = {};

        Result.Min = _mm256_min_pd(RangeA.Min, RangeB.Min);
        Result.Max = _mm256_max_pd(RangeA.Max, RangeB.Max);

        return Result;
    }

    INTERNAL zr1f64
    Intersect(zr1f64 RangeA, zr1f64 RangeB)
    {
        zr1f64 Result = {};

        Result.Min = _mm256_max_pd(RangeA.Min, RangeB.Min);
        Result.Max = _mm256_min_pd(RangeA.Max, RangeB.Max);

        return Result;
    }

    INTERNAL v256d
    Clamp(zr1f64 Range, v256d Val)
    {
        Val = _mm256_max_pd(Range.Min, Val);
        Val = _mm256_min_pd(Range.Max, Val);

        return Val;
    }

    INTERNAL zr2f32
    Rng(zv2f32 Min, zv2f32 Max)
    {
        zr2f32 Result = {};

        Result.Min = Min;
        Result.Max = Max;

        return Result;
    }

    INTERNAL zr2f32
    Shift(zr2f32 Range, zv2f32 Val)
    {
        zr2f32 Result = {};

        Result.Min = Range.Min + Val;
        Result.Max = Range.Max + Val;

        return Result;
    }

    INTERNAL zr2f32
    Widen(zr2f32 Range, zv2f32 Val)
    {
        zr2f32 Result = {};

        Result.Min = Range.Min - Val;
        Result.Max = Range.Max + Val;

        return Result;
    }

    INTERNAL zv2f32
    Centre(zr2f32 Range)
    {
        return (Range.Min + Range.Max) / 2.0f;
    }

    INTERNAL v256f
    InRange(zr2f32 Range, zv2f32 Val)
    {
        return _mm256_and_ps(
            Val >= Range.Min,
            Val < Range.Max
        );
    }

    INTERNAL zv2f32
    Length(zr2f32 Range)
    {
        return Range.Max - Range.Min;
    }

    INTERNAL zr2f32
    Union(zr2f32 RangeA, zr2f32 RangeB)
    {
        zr2f32 Result = {};

        Result.Min.X = _mm256_min_ps(RangeA.Min.X, RangeB.Min.X);
        Result.Min.Y = _mm256_min_ps(RangeA.Min.Y, RangeB.Min.Y);
        Result.Max.X = _mm256_max_ps(RangeA.Max.X, RangeB.Max.X);
        Result.Max.Y = _mm256_max_ps(RangeA.Max.Y, RangeB.Max.Y);

        return Result;
    }

    INTERNAL zr2f32
    Intersect(zr2f32 RangeA, zr2f32 RangeB)
    {
        zr2f32 Result = {};

        Result.Min.X = _mm256_max_ps(RangeA.Min.X, RangeB.Min.X);
        Result.Min.Y = _mm256_max_ps(RangeA.Min.Y, RangeB.Min.Y);
        Result.Max.X = _mm256_min_ps(RangeA.Max.X, RangeB.Max.X);
        Result.Max.Y = _mm256_min_ps(RangeA.Max.Y, RangeB.Max.Y);

        return Result;
    }

    INTERNAL zv2f32
    Clamp(zr2f32 Range, zv2f32 Val)
    {
        Val.X = _mm256_max_ps(Range.Min.X, Val.X);
        Val.Y = _mm256_max_ps(Range.Min.Y, Val.Y);
        Val.X = _mm256_min_ps(Range.Max.X, Val.X);
        Val.Y = _mm256_min_ps(Range.Max.Y, Val.Y);

        return Val;
    }

    INTERNAL zr2f64
    Rng(zv2f64 Min, zv2f64 Max)
    {
        zr2f64 Result = {};

        Result.Min = Min;
        Result.Max = Max;

        return Result;
    }

    INTERNAL zr2f64
    Shift(zr2f64 Range, zv2f64 Val)
    {
        zr2f64 Result = {};

        Result.Min = Range.Min + Val;
        Result.Max = Range.Max + Val;

        return Result;
    }

    INTERNAL zr2f64
    Widen(zr2f64 Range, zv2f64 Val)
    {
        zr2f64 Result = {};

        Result.Min = Range.Min - Val;
        Result.Max = Range.Max + Val;

        return Result;
    }

    INTERNAL zv2f64
    Centre(zr2f64 Range)
    {
        return (Range.Min + Range.Max) / 2.0;
    }

    INTERNAL v256d
    InRange(zr2f64 Range, zv2f64 Val)
    {
        return _mm256_and_pd(
            Val >= Range.Min,
            Val < Range.Max
        );
    }

    INTERNAL zv2f64
    Length(zr2f64 Range)
    {
        return Range.Max - Range.Min;
    }

    INTERNAL zr2f64
    Union(zr2f64 RangeA, zr2f64 RangeB)
    {
        zr2f64 Result = {};

        Result.Min.X = _mm256_min_pd(RangeA.Min.X, RangeB.Min.X);
        Result.Min.Y = _mm256_min_pd(RangeA.Min.Y, RangeB.Min.Y);
        Result.Max.X = _mm256_max_pd(RangeA.Max.X, RangeB.Max.X);
        Result.Max.Y = _mm256_max_pd(RangeA.Max.Y, RangeB.Max.Y);

        return Result;
    }

    INTERNAL zr2f64
    Intersect(zr2f64 RangeA, zr2f64 RangeB)
    {
        zr2f64 Result = {};

        Result.Min.X = _mm256_max_pd(RangeA.Min.X, RangeB.Min.X);
        Result.Min.Y = _mm256_max_pd(RangeA.Min.Y, RangeB.Min.Y);
        Result.Max.X = _mm256_min_pd(RangeA.Max.X, RangeB.Max.X);
        Result.Max.Y = _mm256_min_pd(RangeA.Max.Y, RangeB.Max.Y);

        return Result;
    }

    INTERNAL zv2f64
    Clamp(zr2f64 Range, zv2f64 Val)
    {
        Val.X = _mm256_max_pd(Range.Min.X, Val.X);
        Val.Y = _mm256_max_pd(Range.Min.Y, Val.Y);
        Val.X = _mm256_min_pd(Range.Max.X, Val.X);
        Val.Y = _mm256_min_pd(Range.Max.Y, Val.Y);

        return Val;
    }

    INTERNAL zr3f64
    Rng(zv3f64 Min, zv3f64 Max)
    {
        zr3f64 Result = {};

        Result.Min = Min;
        Result.Max = Max;

        return Result;
    }

    INTERNAL zr3f64
    Shift(zr3f64 Range, zv3f64 Val)
    {
        zr3f64 Result = {};

        Result.Min = Range.Min + Val;
        Result.Max = Range.Max + Val;

        return Result;
    }

    INTERNAL zr3f64
    Widen(zr3f64 Range, zv3f64 Val)
    {
        zr3f64 Result = {};

        Result.Min = Range.Min - Val;
        Result.Max = Range.Max + Val;

        return Result;
    }

    INTERNAL zv3f64
    Centre(zr3f64 Range)
    {
        return (Range.Min + Range.Max) / 2.0;
    }

    INTERNAL v256d
    InRange(zr3f64 Range, zv3f64 Val)
    {
        return _mm256_and_pd(
            Val >= Range.Min,
            Val < Range.Max
        );
    }

    INTERNAL zv3f64
    Length(zr3f64 Range)
    {
        return Range.Max - Range.Min;
    }

    INTERNAL zr3f64
    Union(zr3f64 RangeA, zr3f64 RangeB)
    {
        zr3f64 Result = {};

        Result.Min.X = _mm256_min_pd(RangeA.Min.X, RangeB.Min.X);
        Result.Min.Y = _mm256_min_pd(RangeA.Min.Y, RangeB.Min.Y);
        Result.Min.Z = _mm256_min_pd(RangeA.Min.Z, RangeB.Min.Z);
        Result.Max.X = _mm256_max_pd(RangeA.Max.X, RangeB.Max.X);
        Result.Max.Y = _mm256_max_pd(RangeA.Max.Y, RangeB.Max.Y);
        Result.Max.Z = _mm256_max_pd(RangeA.Max.Z, RangeB.Max.Z);

        return Result;
    }

    INTERNAL zr3f64
    Intersect(zr3f64 RangeA, zr3f64 RangeB)
    {
        zr3f64 Result = {};

        Result.Min.X = _mm256_max_pd(RangeA.Min.X, RangeB.Min.X);
        Result.Min.Y = _mm256_max_pd(RangeA.Min.Y, RangeB.Min.Y);
        Result.Min.Z = _mm256_max_pd(RangeA.Min.Z, RangeB.Min.Z);
        Result.Max.X = _mm256_min_pd(RangeA.Max.X, RangeB.Max.X);
        Result.Max.Y = _mm256_min_pd(RangeA.Max.Y, RangeB.Max.Y);
        Result.Max.Z = _mm256_min_pd(RangeA.Max.Z, RangeB.Max.Z);

        return Result;
    }

    INTERNAL zv3f64
    Clamp(zr3f64 Range, zv3f64 Val)
    {
        Val.X = _mm256_max_pd(Range.Min.X, Val.X);
        Val.Y = _mm256_max_pd(Range.Min.Y, Val.Y);
        Val.Z = _mm256_max_pd(Range.Min.Z, Val.Z);
        Val.X = _mm256_min_pd(Range.Max.X, Val.X);
        Val.Y = _mm256_min_pd(Range.Max.Y, Val.Y);
        Val.Z = _mm256_min_pd(Range.Max.Z, Val.Z);

        return Val;
    }

    INTERNAL zr3f32
    Rng(zv3f32 Min, zv3f32 Max)
    {
        zr3f32 Result = {};

        Result.Min = Min;
        Result.Max = Max;

        return Result;
    }

    INTERNAL zr3f32
    Shift(zr3f32 Range, zv3f32 Val)
    {
        zr3f32 Result = {};

        Result.Min = Range.Min + Val;
        Result.Max = Range.Max + Val;

        return Result;
    }

    INTERNAL zr3f32
    Widen(zr3f32 Range, zv3f32 Val)
    {
        zr3f32 Result = {};

        Result.Min = Range.Min - Val;
        Result.Max = Range.Max + Val;

        return Result;
    }

    INTERNAL zv3f32
    Centre(zr3f32 Range)
    {
        return (Range.Min + Range.Max) / 2.0f;
    }

    INTERNAL v256f
    InRange(zr3f32 Range, zv3f32 Val)
    {
        return _mm256_and_ps(
            Val >= Range.Min,
            Val < Range.Max
        );
    }

    INTERNAL zv3f32
    Length(zr3f32 Range)
    {
        return Range.Max - Range.Min;
    }

    INTERNAL zr3f32
    Union(zr3f32 RangeA, zr3f32 RangeB)
    {
        zr3f32 Result = {};

        Result.Min.X = _mm256_min_ps(RangeA.Min.X, RangeB.Min.X);
        Result.Min.Y = _mm256_min_ps(RangeA.Min.Y, RangeB.Min.Y);
        Result.Min.Z = _mm256_min_ps(RangeA.Min.Z, RangeB.Min.Z);
        Result.Max.X = _mm256_max_ps(RangeA.Max.X, RangeB.Max.X);
        Result.Max.Y = _mm256_max_ps(RangeA.Max.Y, RangeB.Max.Y);
        Result.Max.Z = _mm256_max_ps(RangeA.Max.Z, RangeB.Max.Z);

        return Result;
    }

    INTERNAL zr3f32
    Intersect(zr3f32 RangeA, zr3f32 RangeB)
    {
        zr3f32 Result = {};

        Result.Min.X = _mm256_max_ps(RangeA.Min.X, RangeB.Min.X);
        Result.Min.Y = _mm256_max_ps(RangeA.Min.Y, RangeB.Min.Y);
        Result.Min.Z = _mm256_max_ps(RangeA.Min.Z, RangeB.Min.Z);
        Result.Max.X = _mm256_min_ps(RangeA.Max.X, RangeB.Max.X);
        Result.Max.Y = _mm256_min_ps(RangeA.Max.Y, RangeB.Max.Y);
        Result.Max.Z = _mm256_min_ps(RangeA.Max.Z, RangeB.Max.Z);

        return Result;
    }

    INTERNAL zv3f32
    Clamp(zr3f32 Range, zv3f32 Val)
    {
        Val.X = _mm256_max_ps(Range.Min.X, Val.X);
        Val.Y = _mm256_max_ps(Range.Min.Y, Val.Y);
        Val.Z = _mm256_max_ps(Range.Min.Z, Val.Z);
        Val.X = _mm256_min_ps(Range.Max.X, Val.X);
        Val.Y = _mm256_min_ps(Range.Max.Y, Val.Y);
        Val.Z = _mm256_min_ps(Range.Max.Z, Val.Z);

        return Val;
    }
#elif NC_SIMD_LEVEL >= 2
    union zv2f32 { 
        struct { 
            v128f X;
            v128f Y; 
        };

        v128f V[2];
    };

    union zv2i32 {
        struct {
            v128 X;
            v128 Y; 
        };

        v128 V[2];
    };

    union zv2f64 {
        struct {
            v128d X;
            v128d Y; 
        };

        v128d V[2];
    };

    union zv3f32 {
        struct {
            v128f X;
            v128f Y;
            v128f Z;
        };

        v128f V[3];
    };

    union zv3i32 {
        struct {
            v128 X;
            v128 Y;
            v128 Z;
        };

        v128  V[3];
    };

    union zv3f64 {
        struct {
            v128d X;
            v128d Y;
            v128d Z;
        };

        v128d V[3];
    };

    union zv4f32 {
        struct {
            v128f X;
            v128f Y;
            v128f Z;
            v128f W;
        };

        v128f V[4];
    };

    union zv4i32 {
        struct {
            v128 X;
            v128 Y;
            v128 Z;
            v128 W;
        };

        v128  V[4];
    };

    union zv4f64 {
        struct {
            v128d X;
            v128d Y; 
            v128d Z;
            v128d W;
        };

        v128d V[4];
    };

    union zr1i32 {
        struct {
            v128 Min;
            v128 Max;
        };

        v128  V[2];
    };

    union zr1f32 {
        struct {
            v128f Min;
            v128f Max;
        };

        v128f V[2];
    };

    union zr1f64 {
        struct {
            v128d Min;
            v128d Max;
        };

        v128d V[2];
    };

    union zr2i32 {
        struct {
            zv2i32 Min;
            zv2i32 Max;
        };

        struct {
            zv2i32 Point0;
            zv2i32 Point1;
        };

        zv2i32 V[2];
    };

    union zr2f32 {
        struct {
            zv2f32 Min;
            zv2f32 Max;
        };

        struct {
            zv2f32 Point0;
            zv2f32 Point1;
        };

        zv2f32 V[2];
    };

    union zr2f64 {
        struct {
            zv2f64 Min;
            zv2f64 Max;
        };

        struct {
            zv2f64 Point0;
            zv2f64 Point1;
        };

        zv2f64 V[2];
    };


    union zr3f32 {
        struct {
            zv3f32 Min;
            zv3f32 Max;
        };

        struct {
            zv3f32 Point0;
            zv3f32 Point1;
        };

        zv3f32 V[2];
    };

    union zr3i32 {
        struct {
            zv3i32 Min;
            zv3i32 Max;
        };

        struct {
            zv3i32 Point0;
            zv3i32 Point1;
        };

        zv3i32 V[2];
    };

    union zr3f64 {
        struct {
            zv3f64 Min;
            zv3f64 Max;
        };

        struct {
            zv3f64 Point0;
            zv3f64 Point1;
        };

        zv3f64 V[2];
    };

    INTERNAL zv2f32 
    operator+(zv2f32 VecA, zv2f32 VecB) 
    {
        zv2f32 Result = {};

        Result.X = _mm_add_ps(VecA.X, VecB.X);
        Result.Y = _mm_add_ps(VecA.Y, VecB.Y);

        return Result;
    }

    INTERNAL zv2f32 
    operator-(zv2f32 VecA, zv2f32 VecB) 
    {
        zv2f32 Result = {};

        Result.X = _mm_sub_ps(VecA.X, VecB.X);
        Result.Y = _mm_sub_ps(VecA.Y, VecB.Y);

        return Result;
    }

    INTERNAL zv2f32 
    operator*(zv2f32 VecA, zv2f32 VecB) 
    {
        zv2f32 Result = {};

        Result.X = _mm_mul_ps(VecA.X, VecB.X);
        Result.Y = _mm_mul_ps(VecA.Y, VecB.Y);

        return Result;
    }

    INTERNAL zv2f32 
    operator*(zv2f32 Vec, v128f Value) 
    {
        zv2f32 Result = {};

        Result.X = _mm_mul_ps(Vec.X, Value);
        Result.Y = _mm_mul_ps(Vec.Y, Value);

        return Result;
    }

    INTERNAL zv2f32 
    operator*(zv2f32 Vec, f32 Value) 
    {
        zv2f32 Result = {};
        v128f Scale = _mm_set1_ps(Value);

        Result.X = _mm_mul_ps(Vec.X, Scale);
        Result.Y = _mm_mul_ps(Vec.Y, Scale);

        return Result;
    }

    INTERNAL zv2f32 
    operator/(zv2f32 VecA, zv2f32 VecB) 
    {
        zv2f32 Result = {};

        Result.X = _mm_div_ps(VecA.X, VecB.X);
        Result.Y = _mm_div_ps(VecA.Y, VecB.Y);

        return Result;
    }

    INTERNAL zv2f32 
    operator/(zv2f32 Vec, v128f Value) 
    {
        zv2f32 Result = {};

        Result.X = _mm_div_ps(Vec.X, Value);
        Result.Y = _mm_div_ps(Vec.Y, Value);

        return Result;
    }

    INTERNAL zv2f32 
    operator/(zv2f32 Vec, f32 Value) 
    {
        v128f ZValue = _mm_set1_ps(Value);

        return Vec / ZValue;
    }

    INTERNAL zv2f32 
    operator&(zv2f32 VecA, zv2f32 VecB) 
    {
        zv2f32 Result = {};

        Result.X = _mm_and_ps(VecA.X, VecB.X);
        Result.Y = _mm_and_ps(VecA.Y, VecB.Y);

        return Result;
    }

    INTERNAL zv2f32 
    operator|(zv2f32 VecA, zv2f32 VecB) 
    {
        zv2f32 Result = {};

        Result.X = _mm_or_ps(VecA.X, VecB.X);
        Result.Y = _mm_or_ps(VecA.Y, VecB.Y);

        return Result;
    }

    INTERNAL zv2f32 
    operator^(zv2f32 VecA, zv2f32 VecB) 
    {
        zv2f32 Result = {};

        Result.X = _mm_xor_ps(VecA.X, VecB.X);
        Result.Y = _mm_xor_ps(VecA.Y, VecB.Y);

        return Result;
    }

    INTERNAL v128f
    operator==(zv2f32 VecA, zv2f32 VecB) 
    {
        return _mm_and_ps(
            _mm_cmpeq_ps(VecA.X, VecB.X), 
            _mm_cmpeq_ps(VecA.Y, VecB.Y)
        );
    }

    INTERNAL v128f 
    operator!=(zv2f32 VecA, zv2f32 VecB) 
    {
        return _mm_and_ps(
            _mm_cmpneq_ps(VecA.X, VecB.X), 
            _mm_cmpneq_ps(VecA.Y, VecB.Y)
        );
    }

    INTERNAL v128f
    operator<(zv2f32 VecA, zv2f32 VecB) 
    {
        return _mm_and_ps(
            _mm_cmplt_ps(VecA.X, VecB.X), 
            _mm_cmplt_ps(VecA.Y, VecB.Y)
        );
    }

    INTERNAL v128f
    operator>(zv2f32 VecA, zv2f32 VecB) 
    {
        return _mm_and_ps(
            _mm_cmpgt_ps(VecA.X, VecB.X), 
            _mm_cmpgt_ps(VecA.Y, VecB.Y)
        );
    }

    INTERNAL v128f
    operator<=(zv2f32 VecA, zv2f32 VecB) 
    {
        return _mm_and_ps(
            _mm_cmple_ps(VecA.X, VecB.X), 
            _mm_cmple_ps(VecA.Y, VecB.Y)
        );
    }

    INTERNAL v128f
    operator>=(zv2f32 VecA, zv2f32 VecB) 
    {
        return _mm_and_ps(
            _mm_cmpge_ps(VecA.X, VecB.X), 
            _mm_cmpge_ps(VecA.Y, VecB.Y)
        );
    }

    INTERNAL zv2f32 
    operator%(zv2f32 VecA, zv2f32 VecB) 
    {
        zv2f32 Result = {};
        v128f DivX = _mm_div_ps(VecA.X, VecB.X);
        v128f DivY = _mm_div_ps(VecA.Y, VecB.Y);
        v128f FloorX = _mm_roundscale_ps(DivX, _MM_FROUND_FLOOR);                // TODO: compare against _mm_floor_ps and 
        v128f FloorY = _mm_roundscale_ps(DivY, _MM_FROUND_FLOOR);                //     : _mm_roundscale_round_ps

        Result.X = _mm_fnmadd_ps(FloorX, VecB.X, VecA.X);
        Result.Y = _mm_fnmadd_ps(FloorY, VecB.Y, VecA.Y);

        return Result;
    }

    INTERNAL v128f
    Dot(zv2f32 VecA, zv2f32 VecB)
    {
        v128f Result = {};

        Result = _mm_fmadd_ps(
            VecA.X,
            VecB.X,
            _mm_mul_ps(VecA.Y, VecB.Y)
        );

        return Result;
    }

    INTERNAL v128f
    LengthSq(zv2f32 Vec)
    {
        return Dot(Vec, Vec);
    }

    INTERNAL v128f
    Length(zv2f32 Vec)
    {
        return _mm_sqrt_ps(LengthSq(Vec));
    }

    INTERNAL zv2f32
    Norm(zv2f32 Vec)
    {
        v128f Scale = _mm_rsqrt_ps(LengthSq(Vec));

        return Vec * Scale;
    }

    INTERNAL zv2f32
    Lerp(zv2f32 VecA, zv2f32 VecB, v128f T)
    {
        zv2f32 Result = {};

        Result.X = _mm_fmadd_ps(
            T,
            VecB.X,
            _mm_fnmadd_ps(T, VecA.X, VecA.X)
        );
        Result.Y = _mm_fmadd_ps(
            T,
            VecB.Y,
            _mm_fnmadd_ps(T, VecA.Y, VecA.Y)
        );

        return Result;
    }

    INTERNAL zv2f32
    Lerp(zv2f32 VecA, zv2f32 VecB, f32 T)
    {
        v128f ZT = _mm_set1_ps(T);

        return Lerp(VecA, VecB, ZT);
    }

    INTERNAL zv2i32 
    operator+(zv2i32 VecA, zv2i32 VecB) 
    {
        zv2i32 Result = {};

        Result.X = _mm_add_epi32(VecA.X, VecB.X);
        Result.Y = _mm_add_epi32(VecA.Y, VecB.Y);

        return Result;
    }

    INTERNAL zv2i32 
    operator-(zv2i32 VecA, zv2i32 VecB) 
    {
        zv2i32 Result = {};

        Result.X = _mm_sub_epi32(VecA.X, VecB.X);
        Result.Y = _mm_sub_epi32(VecA.Y, VecB.Y);

        return Result;
    }

    INTERNAL zv2i32 
    operator*(zv2i32 VecA, zv2i32 VecB) 
    {
        zv2i32 Result = {};

        Result.X = _mm_mullo_epi32(VecA.X, VecB.X);
        Result.Y = _mm_mullo_epi32(VecA.Y, VecB.Y);

        return Result;
    }

    INTERNAL zv2i32
    operator*(zv2i32 Vec, v128 Value)
    {
        zv2i32 Result = {};

        Result.X = _mm_mullo_epi32(Vec.X, Value);
        Result.Y = _mm_mullo_epi32(Vec.Y, Value);

        return Result;
    }

    INTERNAL zv2i32
    operator*(zv2i32 Vec, i32 Value)
    {
        v128 ZValue = _mm_set1_epi32(Value);

        return Vec * ZValue;
    }

    INTERNAL zv2i32 
    operator/(zv2i32 VecA, zv2i32 VecB)
    {
        zv2i32 Result = {};
        v128f FloatAX = _mm_cvtepi32_ps(VecA.X);
        v128f FloatAY = _mm_cvtepi32_ps(VecA.Y);
        v128f FloatBX = _mm_cvtepi32_ps(VecB.X);
        v128f FloatBY = _mm_cvtepi32_ps(VecB.Y);
        v128f QuotientX = _mm_div_ps(FloatAX, FloatBX);
        v128f QuotientY = _mm_div_ps(FloatAY, FloatBY);

        Result.X = _mm_cvtps_epi32(QuotientX);
        Result.Y = _mm_cvtps_epi32(QuotientY);

        return Result;
    }

    INTERNAL zv2i32 
    operator&(zv2i32 VecA, zv2i32 VecB) 
    {
        zv2i32 Result = {};

        Result.X = _mm_and_epi32(VecA.X, VecB.X);
        Result.Y = _mm_and_epi32(VecA.Y, VecB.Y);

        return Result;
    }

    INTERNAL zv2i32 
    operator|(zv2i32 VecA, zv2i32 VecB) 
    {
        zv2i32 Result = {};

        Result.X = _mm_or_epi32(VecA.X, VecB.X);
        Result.Y = _mm_or_epi32(VecA.Y, VecB.Y);

        return Result;
    }

    INTERNAL zv2i32 
    operator^(zv2i32 VecA, zv2i32 VecB) 
    {
        zv2i32 Result = {};

        Result.X = _mm_xor_epi32(VecA.X, VecB.X);
        Result.Y = _mm_xor_epi32(VecA.Y, VecB.Y);

        return Result;
    }

    INTERNAL zv2i32 
    operator~(zv2i32 VecA)
    {
        zv2i32 Result = {};

        Result.X = _mm_ternarylogic_epi32(VecA.X, VecA.X, VecA.X, 0x55);
        Result.Y = _mm_ternarylogic_epi32(VecA.Y, VecA.Y, VecA.Y, 0x55);

        return Result;
    }

    INTERNAL v128
    operator==(zv2i32 VecA, zv2i32 VecB) 
    {
        return _mm_and_epi32(
            _mm_cmpeq_epi32(VecA.X, VecB.X),
            _mm_cmpeq_epi32(VecA.Y, VecB.Y)
        );
    }

    INTERNAL v128
    operator!=(zv2i32 VecA, zv2i32 VecB) 
    {
        return _mm_and_si128(
            _mm_xor_si128(
                _mm_cmpeq_epi32(VecA.X, VecB.X),
                _mm_set1_epi32(0xFFFFFFFF)
            ),
            _mm_xor_si128(
                _mm_cmpeq_epi32(VecA.Y, VecB.Y),
                _mm_set1_epi32(0xFFFFFFFF)
            )
        );
    }

    INTERNAL v128
    operator<(zv2i32 VecA, zv2i32 VecB) 
    {
        return _mm_and_epi32(
            _mm_cmplt_epi32(VecA.X, VecB.X),
            _mm_cmplt_epi32(VecA.Y, VecB.Y)
        );
    }

    INTERNAL v128
    operator>(zv2i32 VecA, zv2i32 VecB) 
    {
        return _mm_and_epi32(
            _mm_cmpgt_epi32(VecA.X, VecB.X),
            _mm_cmpgt_epi32(VecA.Y, VecB.Y)
        );
    }

    INTERNAL v128
    operator<=(zv2i32 VecA, zv2i32 VecB) 
    {
        return _mm_and_epi32(
            VecA < VecB,
            VecA == VecB
        );
    }

    INTERNAL v128
    operator>=(zv2i32 VecA, zv2i32 VecB) 
    {
        return _mm_xor_si128(
            VecA < VecB,
            _mm_set1_epi32(0xFFFFFFFF)
        );
    }

    INTERNAL zv2i32 
    operator<<(zv2i32 VecA, i32 Shift)
    {
        zv2i32 Result = {};

        Result.X = _mm_slli_epi32(VecA.X, Shift);
        Result.Y = _mm_slli_epi32(VecA.Y, Shift);

        return Result;
    }

    INTERNAL zv2i32 
    operator>>(zv2i32 VecA, i32 Shift)
    {
        zv2i32 Result = {};

        Result.X = _mm_srli_epi32(VecA.X, Shift);
        Result.Y = _mm_srli_epi32(VecA.Y, Shift);

        return Result;
    }

    INTERNAL v128
    Dot(zv2i32 VecA, zv2i32 VecB)
    {
        v128 Result = {};

        Result = _mm_add_epi32(
            _mm_mul_epi32(VecA.X, VecB.X),
            _mm_mul_epi32(VecA.Y, VecB.Y)
        );

        return Result;
    }

    INTERNAL v128
    LengthSq(zv2i32 Vec)
    {
        return Dot(Vec, Vec);
    }

    INTERNAL v128
    Length(zv2i32 Vec)
    {
        v128f LenSq = _mm_cvtepi32_ps(LengthSq(Vec));
        
        return _mm_cvtps_epi32(
            _mm_sqrt_ps(LenSq)
        );
    }

    INTERNAL zv2i32
    Norm(zv2i32 Vec)
    {
        v128 Scale = _mm_cvtps_epi32(
            _mm_rsqrt14_ps(
                _mm_cvtepi32_ps(
                    LengthSq(Vec)
                )
            )
        );

        return Vec * Scale;
    }

    INTERNAL zv2i32
    Lerp(zv2i32 VecA, zv2i32 VecB, v128 T)
    {
        zv2i32 Result = {};
        v128f AX = _mm_cvtepi32_ps(VecA.X);
        v128f AY = _mm_cvtepi32_ps(VecA.Y);
        v128f BX = _mm_cvtepi32_ps(VecB.X);
        v128f BY = _mm_cvtepi32_ps(VecB.Y);
        v128f ZT = _mm_cvtepi32_ps(T);
        v128f ResultX = _mm_fmadd_ps(
            ZT,
            BX,
            _mm_fnmadd_ps(ZT, AX, AX)
        );
        v128f ResultY = _mm_fmadd_ps(
            ZT,
            BY,
            _mm_fnmadd_ps(ZT, AY, AY)
        );

        Result.X = _mm_cvtps_epi32(ResultX);
        Result.Y = _mm_cvtps_epi32(ResultY);

        return Result;
    }

    INTERNAL zv2i32
    Lerp(zv2i32 VecA, zv2i32 VecB, i32 T)
    {
        v128 ZT = _mm_set1_epi32(T);

        return Lerp(VecA, VecB, ZT);
    }

    INTERNAL zv2f64 
    operator+(zv2f64 VecA, zv2f64 VecB) 
    {
        zv2f64 Result = {};

        Result.X = _mm_add_pd(VecA.X, VecB.X);
        Result.Y = _mm_add_pd(VecA.Y, VecB.Y);

        return Result;
    }

    INTERNAL zv2f64 
    operator-(zv2f64 VecA, zv2f64 VecB) 
    {
        zv2f64 Result = {};

        Result.X = _mm_sub_pd(VecA.X, VecB.X);
        Result.Y = _mm_sub_pd(VecA.Y, VecB.Y);

        return Result;
    }

    INTERNAL zv2f64 
    operator*(zv2f64 VecA, zv2f64 VecB) 
    {
        zv2f64 Result = {};

        Result.X = _mm_mul_pd(VecA.X, VecB.X);
        Result.Y = _mm_mul_pd(VecA.Y, VecB.Y);

        return Result;
    }

    INTERNAL zv2f64 
    operator*(zv2f64 Vec, v128d Value) 
    {
        zv2f64 Result = {};

        Result.X = _mm_mul_pd(Vec.X, Value);
        Result.Y = _mm_mul_pd(Vec.Y, Value);

        return Result;
    }

    INTERNAL zv2f64 
    operator*(zv2f64 Vec, f64 Value) 
    {
        v128d ZValue = _mm_set1_pd(Value);

        return Vec * ZValue;
    }

    INTERNAL zv2f64 
    operator/(zv2f64 VecA, zv2f64 VecB) 
    {
        zv2f64 Result = {};

        Result.X = _mm_div_pd(VecA.X, VecB.X);
        Result.Y = _mm_div_pd(VecA.Y, VecB.Y);

        return Result;
    }

    INTERNAL zv2f64 
    operator/(zv2f64 Vec, v128d Value) 
    {
        zv2f64 Result = {};

        Result.X = _mm_div_pd(Vec.X, Value);
        Result.Y = _mm_div_pd(Vec.Y, Value);

        return Result;
    }

    INTERNAL zv2f64 
    operator/(zv2f64 Vec, f64 Value) 
    {
        v128d ZValue = _mm_set1_pd(Value);

        return Vec / ZValue;
    }

    INTERNAL zv2f64 
    operator&(zv2f64 VecA, zv2f64 VecB) 
    {
        zv2f64 Result = {};

        Result.X = _mm_and_pd(VecA.X, VecB.X);
        Result.Y = _mm_and_pd(VecA.Y, VecB.Y);

        return Result;
    }

    INTERNAL zv2f64 
    operator|(zv2f64 VecA, zv2f64 VecB) 
    {
        zv2f64 Result = {};

        Result.X = _mm_or_pd(VecA.X, VecB.X);
        Result.Y = _mm_or_pd(VecA.Y, VecB.Y);

        return Result;
    }

    INTERNAL zv2f64 
    operator^(zv2f64 VecA, zv2f64 VecB) 
    {
        zv2f64 Result = {};

        Result.X = _mm_xor_pd(VecA.X, VecB.X);
        Result.Y = _mm_xor_pd(VecA.Y, VecB.Y);

        return Result;
    }

    INTERNAL v128d
    operator==(zv2f64 VecA, zv2f64 VecB) 
    {
        return _mm_and_pd(
            _mm_cmpeq_pd(VecA.X, VecB.X),
            _mm_cmpeq_pd(VecA.Y, VecB.Y)
        );
    }

    INTERNAL v128d
    operator!=(zv2f64 VecA, zv2f64 VecB) 
    {
        return _mm_and_pd(
            _mm_cmpneq_pd(VecA.X, VecB.X),
            _mm_cmpneq_pd(VecA.Y, VecB.Y)
        );
    }

    INTERNAL v128d
    operator<(zv2f64 VecA, zv2f64 VecB) 
    {
        return _mm_and_pd(
            _mm_cmplt_pd(VecA.X, VecB.X),
            _mm_cmplt_pd(VecA.Y, VecB.Y)
        );
    }

    INTERNAL v128d
    operator>(zv2f64 VecA, zv2f64 VecB) 
    {
        return _mm_and_pd(
            _mm_cmpgt_pd(VecA.X, VecB.X),
            _mm_cmpgt_pd(VecA.Y, VecB.Y)
        );
    }

    INTERNAL v128d
    operator<=(zv2f64 VecA, zv2f64 VecB) 
    {
        return _mm_and_pd(
            _mm_cmple_pd(VecA.X, VecB.X),
            _mm_cmple_pd(VecA.Y, VecB.Y)
        );
    }

    INTERNAL v128d
    operator>=(zv2f64 VecA, zv2f64 VecB) 
    {
        return _mm_and_pd(
            _mm_cmpge_pd(VecA.X, VecB.X),
            _mm_cmpge_pd(VecA.Y, VecB.Y)
        );
    }

    INTERNAL zv2f64 
    operator%(zv2f64 VecA, zv2f64 VecB) 
    {
        zv2f64 Result = {};
        v128d DivX = _mm_div_pd(VecA.X, VecB.X);
        v128d DivY = _mm_div_pd(VecA.Y, VecB.Y);
        v128d FloorX = _mm_roundscale_pd(DivX, _MM_FROUND_FLOOR);                // TODO: compare against _mm_floor_pd and 
        v128d FloorY = _mm_roundscale_pd(DivY, _MM_FROUND_FLOOR);                //     : _mm_roundscale_round_pd

        Result.X = _mm_fnmadd_pd(FloorX, VecB.X, VecA.X);
        Result.Y = _mm_fnmadd_pd(FloorY, VecB.Y, VecA.Y);

        return Result;
    }

    INTERNAL v128d
    Dot(zv2f64 VecA, zv2f64 VecB)
    {
        v128d Result = {};

        Result = _mm_fmadd_pd(
            VecA.X,
            VecB.X,
            _mm_mul_pd(VecA.Y, VecB.Y)
        );

        return Result;
    }

    INTERNAL v128d
    LengthSq(zv2f64 Vec)
    {
        return Dot(Vec, Vec);
    }

    INTERNAL v128d
    Length(zv2f64 Vec)
    {
        return _mm_sqrt_pd(LengthSq(Vec));
    }

    INTERNAL zv2f64
    Norm(zv2f64 Vec)
    {
        v128d Scale = _mm_rsqrt14_pd(LengthSq(Vec));

        return Vec * Scale;
    }

    INTERNAL zv2f64
    Lerp(zv2f64 VecA, zv2f64 VecB, v128d T)
    {
        zv2f64 Result = {};

        Result.X = _mm_fmadd_pd(
            T,
            VecB.X,
            _mm_fnmadd_pd(T, VecA.X, VecA.X)
        );
        Result.Y = _mm_fmadd_pd(
            T,
            VecB.Y,
            _mm_fnmadd_pd(T, VecA.Y, VecA.Y)
        );

        return Result;
    }

    INTERNAL zv2f64
    Lerp(zv2f64 VecA, zv2f64 VecB, f64 T)
    {
        v128d ZT = _mm_set1_pd(T);

        return Lerp(VecA, VecB, ZT);
    }

    INTERNAL zv3f32 
    operator+(zv3f32 VecA, zv3f32 VecB) 
    {
        zv3f32 Result = {};

        Result.X = _mm_add_ps(VecA.X, VecB.X);
        Result.Y = _mm_add_ps(VecA.Y, VecB.Y);
        Result.Z = _mm_add_ps(VecA.Z, VecB.Z);

        return Result;
    }

    INTERNAL zv3f32 
    operator-(zv3f32 VecA, zv3f32 VecB) 
    {
        zv3f32 Result = {};

        Result.X = _mm_sub_ps(VecA.X, VecB.X);
        Result.Y = _mm_sub_ps(VecA.Y, VecB.Y);
        Result.Z = _mm_sub_ps(VecA.Z, VecB.Z);

        return Result;
    }

    INTERNAL zv3f32 
    operator*(zv3f32 VecA, zv3f32 VecB) 
    {
        zv3f32 Result = {};

        Result.X = _mm_mul_ps(VecA.X, VecB.X);
        Result.Y = _mm_mul_ps(VecA.Y, VecB.Y);
        Result.Z = _mm_mul_ps(VecA.Z, VecB.Z);

        return Result;
    }

    INTERNAL zv3f32 
    operator*(zv3f32 Vec, v128f Value) 
    {
        zv3f32 Result = {};

        Result.X = _mm_mul_ps(Vec.X, Value);
        Result.Y = _mm_mul_ps(Vec.Y, Value);
        Result.Z = _mm_mul_ps(Vec.Z, Value);

        return Result;
    }

    INTERNAL zv3f32 
    operator*(zv3f32 Vec, f32 Value) 
    {
        v128f ZValue = _mm_set1_ps(Value);

        return Vec * ZValue;
    }

    INTERNAL zv3f32 
    operator/(zv3f32 VecA, zv3f32 VecB) 
    {
        zv3f32 Result = {};

        Result.X = _mm_div_ps(VecA.X, VecB.X);
        Result.Y = _mm_div_ps(VecA.Y, VecB.Y);
        Result.Z = _mm_div_ps(VecA.Z, VecB.Z);

        return Result;
    }

    INTERNAL zv3f32
    operator/(zv3f32 Vec, v128f Value) 
    {
        zv3f32 Result = {};

        Result.X = _mm_div_ps(Vec.X, Value);
        Result.Y = _mm_div_ps(Vec.Y, Value);
        Result.Z = _mm_div_ps(Vec.Z, Value);

        return Result;
    }

    INTERNAL zv3f32
    operator/(zv3f32 Vec, f32 Value) 
    {
        v128f ZValue = _mm_set1_ps(Value);

        return Vec / ZValue;
    }

    INTERNAL zv3f32 
    operator&(zv3f32 VecA, zv3f32 VecB) 
    {
        zv3f32 Result = {};

        Result.X = _mm_and_ps(VecA.X, VecB.X);
        Result.Y = _mm_and_ps(VecA.Y, VecB.Y);
        Result.Z = _mm_and_ps(VecA.Z, VecB.Z);

        return Result;
    }

    INTERNAL zv3f32 
    operator|(zv3f32 VecA, zv3f32 VecB) 
    {
        zv3f32 Result = {};

        Result.X = _mm_or_ps(VecA.X, VecB.X);
        Result.Y = _mm_or_ps(VecA.Y, VecB.Y);
        Result.Z = _mm_or_ps(VecA.Z, VecB.Z);

        return Result;
    }

    INTERNAL zv3f32 
    operator^(zv3f32 VecA, zv3f32 VecB) 
    {
        zv3f32 Result = {};

        Result.X = _mm_xor_ps(VecA.X, VecB.X);
        Result.Y = _mm_xor_ps(VecA.Y, VecB.Y);
        Result.Z = _mm_xor_ps(VecA.Z, VecB.Z);

        return Result;
    }

    INTERNAL v128f
    operator==(zv3f32 VecA, zv3f32 VecB) 
    {
        return _mm_and_ps(
            _mm_and_ps(
                _mm_cmpeq_ps(VecA.X, VecB.X),
                _mm_cmpeq_ps(VecA.Y, VecB.Y)
            ),
            _mm_cmpeq_ps(VecA.Z, VecB.Z)
        );
    }

    INTERNAL v128f 
    operator!=(zv3f32 VecA, zv3f32 VecB) 
    {
        return _mm_and_ps(
            _mm_and_ps(
                _mm_cmpneq_ps(VecA.X, VecB.X),
                _mm_cmpneq_ps(VecA.Y, VecB.Y)
            ),
            _mm_cmpneq_ps(VecA.Z, VecB.Z)
        );
    }

    INTERNAL v128f 
    operator<(zv3f32 VecA, zv3f32 VecB) 
    {
        return _mm_and_ps(
            _mm_and_ps(
                _mm_cmplt_ps(VecA.X, VecB.X),
                _mm_cmplt_ps(VecA.Y, VecB.Y)
            ),
            _mm_cmplt_ps(VecA.Z, VecB.Z)
        );
    }

    INTERNAL v128f 
    operator>(zv3f32 VecA, zv3f32 VecB) 
    {
        return _mm_and_ps(
            _mm_and_ps(
                _mm_cmpgt_ps(VecA.X, VecB.X),
                _mm_cmpgt_ps(VecA.Y, VecB.Y)
            ),
            _mm_cmpgt_ps(VecA.Z, VecB.Z)
        );
    }

    INTERNAL v128f 
    operator<=(zv3f32 VecA, zv3f32 VecB) 
    {
        return _mm_and_ps(
            _mm_and_ps(
                _mm_cmple_ps(VecA.X, VecB.X),
                _mm_cmple_ps(VecA.Y, VecB.Y)
            ),
            _mm_cmple_ps(VecA.Z, VecB.Z)
        );
    }

    INTERNAL v128f 
    operator>=(zv3f32 VecA, zv3f32 VecB) 
    {
        return _mm_and_ps(
            _mm_and_ps(
                _mm_cmpge_ps(VecA.X, VecB.X),
                _mm_cmpge_ps(VecA.Y, VecB.Y)
            ),
            _mm_cmpge_ps(VecA.Z, VecB.Z)
        );
    }

    INTERNAL zv3f32 
    operator%(zv3f32 VecA, zv3f32 VecB) 
    {
        zv3f32 Result = {};
        v128f DivX = _mm_div_ps(VecA.X, VecB.X);
        v128f DivY = _mm_div_ps(VecA.Y, VecB.Y);
        v128f DivZ = _mm_div_ps(VecA.Z, VecB.Z);
        v128f FloorX = _mm_roundscale_ps(DivX, _MM_FROUND_FLOOR);                // TODO: compare against _mm_floor_ps and 
        v128f FloorY = _mm_roundscale_ps(DivY, _MM_FROUND_FLOOR);                //     : _mm_roundscale_round_ps
        v128f FloorZ = _mm_roundscale_ps(DivZ, _MM_FROUND_FLOOR);

        Result.X = _mm_fnmadd_ps(FloorX, VecB.X, VecA.X);
        Result.Y = _mm_fnmadd_ps(FloorY, VecB.Y, VecA.Y);
        Result.Z = _mm_fnmadd_ps(FloorZ, VecB.Z, VecA.Z);

        return Result;
    }

    INTERNAL v128f
    Dot(zv3f32 VecA, zv3f32 VecB)
    {
        v128f Result = {};

        Result = _mm_fmadd_ps(
            VecA.X,
            VecB.X,
            _mm_fmadd_ps(
                VecA.Y,
                VecB.Y,
                _mm_mul_ps(VecA.Z, VecB.Z)
            )
        );

        return Result;
    }

    INTERNAL v128f
    LengthSq(zv3f32 Vec)
    {
        return Dot(Vec, Vec);
    }

    INTERNAL v128f
    Length(zv3f32 Vec)
    {
        return _mm_sqrt_ps(LengthSq(Vec));
    }

    INTERNAL zv3f32
    Norm(zv3f32 Vec)
    {
        v128f Scale = _mm_rsqrt14_ps(LengthSq(Vec));

        return Vec * Scale;
    }

    INTERNAL zv3f32
    Lerp(zv3f32 VecA, zv3f32 VecB, v128f T)
    {
        zv3f32 Result = {};

        Result.X = _mm_fmadd_ps(
            T, 
            VecB.X, 
            _mm_fnmadd_ps(T, VecA.X, VecA.X)
        );
        Result.Y = _mm_fmadd_ps(
            T, 
            VecB.Y, 
            _mm_fnmadd_ps(T, VecA.Y, VecA.Y)
        );
        Result.Z = _mm_fmadd_ps(
            T, 
            VecB.Z, 
            _mm_fnmadd_ps(T, VecA.Z, VecA.Z)
        );

        return Result;
    }

    INTERNAL zv3f32
    Lerp(zv3f32 VecA, zv3f32 VecB, f32 T)
    {
        v128f ZT = _mm_set1_ps(T);

        return Lerp(VecA, VecB, ZT);
    }

    INTERNAL zv3f32
    Cross(zv3f32 VecA, zv3f32 VecB)
    {
        zv3f32 Result = {};

        Result.X = _mm_fmsub_ps(
            VecA.Y,
            VecB.Z,
            _mm_mul_ps(VecA.Z, VecB.Y)
        );
        Result.Y = _mm_fmsub_ps(
            VecA.Z,
            VecB.X,
            _mm_mul_ps(VecA.X, VecB.Z)
        );
        Result.Z = _mm_fmsub_ps(
            VecA.X,
            VecB.Y,
            _mm_mul_ps(VecA.Y, VecB.X)
        );

        return Result;
    }

    INTERNAL zv3i32 
    operator+(zv3i32 VecA, zv3i32 VecB) 
    {
        zv3i32 Result = {};

        Result.X = _mm_add_epi32(VecA.X, VecB.X);
        Result.Y = _mm_add_epi32(VecA.Y, VecB.X);
        Result.Z = _mm_add_epi32(VecA.Z, VecB.X);

        return Result;
    }

    INTERNAL zv3i32 
    operator-(zv3i32 VecA, zv3i32 VecB) 
    {
        zv3i32 Result = {};

        Result.X = _mm_sub_epi32(VecA.X, VecB.X);
        Result.Y = _mm_sub_epi32(VecA.Y, VecB.X);
        Result.Z = _mm_sub_epi32(VecA.Z, VecB.X);

        return Result;
    }

    INTERNAL zv3i32 
    operator*(zv3i32 VecA, zv3i32 VecB) 
    {
        zv3i32 Result = {};

        Result.X = _mm_mullo_epi32(VecA.X, VecB.X);
        Result.Y = _mm_mullo_epi32(VecA.Y, VecB.X);
        Result.Z = _mm_mullo_epi32(VecA.Z, VecB.X);

        return Result;
    }

    INTERNAL zv3i32 
    operator*(zv3i32 Vec, v128 Value) 
    {
        zv3i32 Result = {};

        Result.X = _mm_mullo_epi32(Vec.X, Value);
        Result.Y = _mm_mullo_epi32(Vec.Y, Value);
        Result.Z = _mm_mullo_epi32(Vec.Z, Value);

        return Result;
    }

    INTERNAL zv3i32 
    operator*(zv3i32 Vec, i32 Value) 
    {
        v128 ZValue = _mm_set1_epi32(Value);

        return Vec * ZValue;
    }

    INTERNAL zv3i32 
    operator/(zv3i32 VecA, zv3i32 VecB) 
    {
        zv3i32 Result = {};
        v128f FloatAX = _mm_cvtepi32_ps(VecA.X);
        v128f FloatAY = _mm_cvtepi32_ps(VecA.Y);
        v128f FloatAZ = _mm_cvtepi32_ps(VecA.Z);
        v128f FloatBX = _mm_cvtepi32_ps(VecB.X);
        v128f FloatBY = _mm_cvtepi32_ps(VecB.Y);
        v128f FloatBZ = _mm_cvtepi32_ps(VecB.Z);
        v128f QuotientX = _mm_div_ps(FloatAX, FloatBX);
        v128f QuotientY = _mm_div_ps(FloatAY, FloatBY);
        v128f QuotientZ = _mm_div_ps(FloatAZ, FloatBZ);

        Result.X = _mm_cvtps_epi32(QuotientX);
        Result.Y = _mm_cvtps_epi32(QuotientY);
        Result.Z = _mm_cvtps_epi32(QuotientZ);

        return Result;
    }

    INTERNAL zv3i32 
    operator&(zv3i32 VecA, zv3i32 VecB) 
    {
        zv3i32 Result = {};

        Result.X = _mm_and_epi32(VecA.X, VecB.X);
        Result.Y = _mm_and_epi32(VecA.Y, VecB.X);
        Result.Z = _mm_and_epi32(VecA.Z, VecB.X);

        return Result;
    }

    INTERNAL zv3i32 
    operator|(zv3i32 VecA, zv3i32 VecB) 
    {
        zv3i32 Result = {};

        Result.X = _mm_or_epi32(VecA.X, VecB.X);
        Result.Y = _mm_or_epi32(VecA.Y, VecB.X);
        Result.Z = _mm_or_epi32(VecA.Z, VecB.X);

        return Result;
    }

    INTERNAL zv3i32 
    operator^(zv3i32 VecA, zv3i32 VecB) 
    {
        zv3i32 Result = {};

        Result.X = _mm_xor_epi32(VecA.X, VecB.X);
        Result.Y = _mm_xor_epi32(VecA.Y, VecB.X);
        Result.Z = _mm_xor_epi32(VecA.Z, VecB.X);

        return Result;
    }

    INTERNAL v128
    operator==(zv3i32 VecA, zv3i32 VecB) 
    {
        return _mm_and_epi32(
            _mm_and_epi32(
                _mm_cmpeq_epi32(VecA.X, VecB.X),
                _mm_cmpeq_epi32(VecA.Y, VecB.Y)
            ),
            _mm_cmpeq_epi32(VecA.Z, VecB.Z)
        );
    }

    INTERNAL v128
    operator!=(zv3i32 VecA, zv3i32 VecB) 
    {
        return _mm_xor_si128(
            VecA == VecB,
            _mm_set1_epi32(0xFFFFFFFF)
        );
    }

    INTERNAL v128
    operator<(zv3i32 VecA, zv3i32 VecB) 
    {
        return _mm_and_epi32(
            _mm_and_epi32(
                _mm_cmplt_epi32(VecA.X, VecB.X),
                _mm_cmplt_epi32(VecA.Y, VecB.Y)
            ),
            _mm_cmplt_epi32(VecA.Z, VecB.Z)
        );
    }

    INTERNAL v128
    operator>(zv3i32 VecA, zv3i32 VecB) 
    {
        return _mm_and_epi32(
            _mm_and_epi32(
                _mm_cmpgt_epi32(VecA.X, VecB.X),
                _mm_cmpgt_epi32(VecA.Y, VecB.Y)
            ),
            _mm_cmpgt_epi32(VecA.Z, VecB.Z)
        );
    }

    INTERNAL v128
    operator<=(zv3i32 VecA, zv3i32 VecB) 
    {
        return _mm_xor_si128(
            VecA > VecB,
            _mm_set1_epi32(0xFFFFFFFF)
        );
    }

    INTERNAL v128
    operator>=(zv3i32 VecA, zv3i32 VecB) 
    {
        return _mm_xor_si128(
            VecA < VecB,
            _mm_set1_epi32(0xFFFFFFFF)
        );
    }

    INTERNAL zv3i32 
    operator<<(zv3i32 VecA, i32 Shift)
    {
        zv3i32 Result = {};

        Result.X = _mm_slli_epi32(VecA.X, Shift);
        Result.Y = _mm_slli_epi32(VecA.Y, Shift);
        Result.Z = _mm_slli_epi32(VecA.Z, Shift);

        return Result;
    }

    INTERNAL zv3i32 
    operator>>(zv3i32 VecA, i32 Shift)
    {
        zv3i32 Result = {};

        Result.X = _mm_srli_epi32(VecA.X, Shift);
        Result.Y = _mm_srli_epi32(VecA.Y, Shift);
        Result.Z = _mm_srli_epi32(VecA.Z, Shift);

        return Result;
    }

    INTERNAL v128
    Dot(zv3i32 VecA, zv3i32 VecB)
    {
        v128 Result = {};

        Result = _mm_add_epi32(
            _mm_add_epi32(
                _mm_mullo_epi32(VecA.X, VecB.X),
                _mm_mullo_epi32(VecA.Y, VecB.Y)
            ),
            _mm_mullo_epi32(VecA.Z, VecB.Z)
        );

        return Result;
    }

    INTERNAL v128
    LengthSq(zv3i32 Vec)
    {
        return Dot(Vec, Vec);
    }

    INTERNAL v128
    Length(zv3i32 Vec)
    {
        v128f LenSq = _mm_cvtepi32_ps(LengthSq(Vec));
        
        return _mm_cvtps_epi32(
            _mm_sqrt_ps(LenSq)
        );
    }

    INTERNAL zv3i32
    Norm(zv3i32 Vec)
    {
        v128 Scale = _mm_cvtps_epi32(
            _mm_rcp14_ps(
                _mm_cvtepi32_ps(
                    Length(Vec)
                )
            )
        );

        return Vec * Scale;
    }

    INTERNAL zv3i32
    Lerp(zv3i32 VecA, zv3i32 VecB, v128 T)
    {
        zv3i32 Result = {};
        v128f AX = _mm_cvtepi32_ps(VecA.X);
        v128f AY = _mm_cvtepi32_ps(VecA.Y);
        v128f AZ = _mm_cvtepi32_ps(VecA.Z);
        v128f BX = _mm_cvtepi32_ps(VecB.X);
        v128f BY = _mm_cvtepi32_ps(VecB.Y);
        v128f BZ = _mm_cvtepi32_ps(VecB.Z);
        v128f ZT = _mm_cvtepi32_ps(T);
        v128f ResultX = _mm_fmadd_ps(
            ZT,
            BX,
            _mm_fnmadd_ps(
                ZT, 
                AX,
                AX
            )
        );
        v128f ResultY = _mm_fmadd_ps(
            ZT,
            BY,
            _mm_fnmadd_ps(
                ZT, 
                AY,
                AY
            )
        );
        v128f ResultZ = _mm_fmadd_ps(
            ZT,
            BZ,
            _mm_fnmadd_ps(
                ZT, 
                AZ,
                AZ
            )
        );

        Result.X = _mm_cvtps_epi32(ResultX);
        Result.Y = _mm_cvtps_epi32(ResultY);
        Result.Z = _mm_cvtps_epi32(ResultZ);

        return Result;
    }

    INTERNAL zv3i32
    Lerp(zv3i32 VecA, zv3i32 VecB, i32 T)
    {
        v128 ZT = _mm_set1_epi32(T);

        return Lerp(VecA, VecB, ZT);
    }

    INTERNAL zv3i32
    Cross(zv3i32 VecA, zv3i32 VecB)
    {
        zv3i32 Result = {};

        Result.X = _mm_sub_epi32(
            _mm_mullo_epi32(VecA.Y, VecB.Z),
            _mm_mullo_epi32(VecA.Z, VecB.Y)
        );
        Result.Y = _mm_sub_epi32(
            _mm_mullo_epi32(VecA.Z, VecB.X),
            _mm_mullo_epi32(VecA.X, VecB.Z)
        );
        Result.Z = _mm_sub_epi32(
            _mm_mullo_epi32(VecA.X, VecB.Y),
            _mm_mullo_epi32(VecA.Y, VecB.X)
        );

        return Result;
    }

    INTERNAL zv3f64 
    operator+(zv3f64 VecA, zv3f64 VecB) 
    {
        zv3f64 Result = {};

        Result.X = _mm_add_pd(VecA.X, VecB.X);
        Result.Y = _mm_add_pd(VecA.Y, VecB.Y);
        Result.Z = _mm_add_pd(VecA.Z, VecB.Z);

        return Result;
    }

    INTERNAL zv3f64 
    operator-(zv3f64 VecA, zv3f64 VecB) 
    {
        zv3f64 Result = {};

        Result.X = _mm_sub_pd(VecA.X, VecB.X);
        Result.Y = _mm_sub_pd(VecA.Y, VecB.Y);
        Result.Z = _mm_sub_pd(VecA.Z, VecB.Z);

        return Result;
    }

    INTERNAL zv3f64 
    operator*(zv3f64 VecA, zv3f64 VecB) 
    {
        zv3f64 Result = {};

        Result.X = _mm_mul_pd(VecA.X, VecB.X);
        Result.Y = _mm_mul_pd(VecA.Y, VecB.Y);
        Result.Z = _mm_mul_pd(VecA.Z, VecB.Z);

        return Result;
    }

    INTERNAL zv3f64 
    operator*(zv3f64 Vec, v128d Value) 
    {
        zv3f64 Result = {};

        Result.X = _mm_mul_pd(Vec.X, Value);
        Result.Y = _mm_mul_pd(Vec.Y, Value);
        Result.Z = _mm_mul_pd(Vec.Z, Value);

        return Result;
    }

    INTERNAL zv3f64 
    operator*(zv3f64 Vec, f64 Value) 
    {
        v128d ZValue = _mm_set1_pd(Value);

        return Vec * ZValue;
    }

    INTERNAL zv3f64 
    operator/(zv3f64 VecA, zv3f64 VecB) 
    {
        zv3f64 Result = {};

        Result.X = _mm_div_pd(VecA.X, VecB.X);
        Result.Y = _mm_div_pd(VecA.Y, VecB.Y);
        Result.Z = _mm_div_pd(VecA.Z, VecB.Z);

        return Result;
    }

    INTERNAL zv3f64
    operator/(zv3f64 Vec, v128d Value) 
    {
        zv3f64 Result = {};

        Result.X = _mm_div_pd(Vec.X, Value);
        Result.Y = _mm_div_pd(Vec.Y, Value);
        Result.Z = _mm_div_pd(Vec.Z, Value);

        return Result;
    }

    INTERNAL zv3f64
    operator/(zv3f64 Vec, f64 Value) 
    {
        v128d ZValue = _mm_set1_pd(Value);

        return Vec / ZValue;
    }

    INTERNAL zv3f64 
    operator&(zv3f64 VecA, zv3f64 VecB) 
    {
        zv3f64 Result = {};

        Result.X = _mm_and_pd(VecA.X, VecB.X);
        Result.Y = _mm_and_pd(VecA.Y, VecB.Y);
        Result.Z = _mm_and_pd(VecA.Z, VecB.Z);

        return Result;
    }

    INTERNAL zv3f64 
    operator|(zv3f64 VecA, zv3f64 VecB) 
    {
        zv3f64 Result = {};

        Result.X = _mm_or_pd(VecA.X, VecB.X);
        Result.Y = _mm_or_pd(VecA.Y, VecB.Y);
        Result.Z = _mm_or_pd(VecA.Z, VecB.Z);

        return Result;
    }

    INTERNAL zv3f64 
    operator^(zv3f64 VecA, zv3f64 VecB) 
    {
        zv3f64 Result = {};

        Result.X = _mm_xor_pd(VecA.X, VecB.X);
        Result.Y = _mm_xor_pd(VecA.Y, VecB.Y);
        Result.Z = _mm_xor_pd(VecA.Z, VecB.Z);

        return Result;
    }

    INTERNAL v128d
    operator==(zv3f64 VecA, zv3f64 VecB) 
    {
        return _mm_and_pd(
            _mm_and_pd(
                _mm_cmpeq_pd(VecA.X, VecB.X),
                _mm_cmpeq_pd(VecA.Y, VecB.Y)
            ),
            _mm_cmpeq_pd(VecA.Z, VecB.Z)
        );
    }

    INTERNAL v128d
    operator!=(zv3f64 VecA, zv3f64 VecB) 
    {
        return _mm_and_pd(
            _mm_and_pd(
                _mm_cmpneq_pd(VecA.X, VecB.X),
                _mm_cmpneq_pd(VecA.Y, VecB.Y)
            ),
            _mm_cmpneq_pd(VecA.Z, VecB.Z)
        );
    }

    INTERNAL v128d
    operator<(zv3f64 VecA, zv3f64 VecB) 
    {
        return _mm_and_pd(
            _mm_and_pd(
                _mm_cmplt_pd(VecA.X, VecB.X),
                _mm_cmplt_pd(VecA.Y, VecB.Y)
            ),
            _mm_cmplt_pd(VecA.Z, VecB.Z)
        );
    }

    INTERNAL v128d
    operator>(zv3f64 VecA, zv3f64 VecB) 
    {
        return _mm_and_pd(
            _mm_and_pd(
                _mm_cmpgt_pd(VecA.X, VecB.X),
                _mm_cmpgt_pd(VecA.Y, VecB.Y)
            ),
            _mm_cmpgt_pd(VecA.Z, VecB.Z)
        );
    }

    INTERNAL v128d
    operator<=(zv3f64 VecA, zv3f64 VecB) 
    {
        return _mm_and_pd(
            _mm_and_pd(
                _mm_cmple_pd(VecA.X, VecB.X),
                _mm_cmple_pd(VecA.Y, VecB.Y)
            ),
            _mm_cmple_pd(VecA.Z, VecB.Z)
        );
    }

    INTERNAL v128d
    operator>=(zv3f64 VecA, zv3f64 VecB) 
    {
        return _mm_and_pd(
            _mm_and_pd(
                _mm_cmpge_pd(VecA.X, VecB.X),
                _mm_cmpge_pd(VecA.Y, VecB.Y)
            ),
            _mm_cmpge_pd(VecA.Z, VecB.Z)
        );
    }

    INTERNAL v128d
    Dot(zv3f64 VecA, zv3f64 VecB)
    {
        v128d Result = {};

        Result = _mm_fmadd_pd(
            VecA.X,
            VecB.X,
            _mm_fmadd_pd(
                VecA.Y,
                VecB.Y,
                _mm_mul_pd(VecA.Z, VecB.Z)
            )
        );

        return Result;
    }

    INTERNAL v128d
    LengthSq(zv3f64 Vec)
    {
        return Dot(Vec, Vec);
    }

    INTERNAL v128d
    Length(zv3f64 Vec)
    {
        return _mm_sqrt_pd(LengthSq(Vec));
    }

    INTERNAL zv3f64
    Norm(zv3f64 Vec)
    {
        v128d Scale = _mm_rsqrt14_pd(LengthSq(Vec));

        return Vec * Scale;
    }

    INTERNAL zv3f64
    Lerp(zv3f64 VecA, zv3f64 VecB, v128d T)
    {
        zv3f64 Result = {};

        Result.X = _mm_fmadd_pd(T, VecB.X, _mm_fnmadd_pd(T, VecA.X, VecA.X));
        Result.Y = _mm_fmadd_pd(T, VecB.Y, _mm_fnmadd_pd(T, VecA.Y, VecA.Y));
        Result.Z = _mm_fmadd_pd(T, VecB.Z, _mm_fnmadd_pd(T, VecA.Z, VecA.Z));

        return Result;
    }

    INTERNAL zv3f64
    Lerp(zv3f64 VecA, zv3f64 VecB, f64 T)
    {
        v128d ZT = _mm_set1_pd(T);

        return Lerp(VecA, VecB, ZT);
    }

    INTERNAL zv3f64
    Cross(zv3f64 VecA, zv3f64 VecB)
    {
        zv3f64 Result = {};

        Result.X = _mm_fmsub_pd(
            VecA.Y,
            VecB.Z,
            _mm_mul_pd(VecA.Z, VecB.Y)
        );
        Result.Y = _mm_fmsub_pd(
            VecA.Z,
            VecB.X,
            _mm_mul_pd(VecA.X, VecB.Z)
        );
        Result.Z = _mm_fmsub_pd(
            VecA.X,
            VecB.Y,
            _mm_mul_pd(VecA.Y, VecB.X)
        );

        return Result;
    }

    INTERNAL zv4f32 
    operator+(zv4f32 VecA, zv4f32 VecB) 
    {
        zv4f32 Result = {};

        Result.X = _mm_add_ps(VecA.X, VecB.X);
        Result.Y = _mm_add_ps(VecA.Y, VecB.Y);
        Result.Z = _mm_add_ps(VecA.Z, VecB.Z);
        Result.W = _mm_add_ps(VecA.W, VecB.W);

        return Result;
    }

    INTERNAL zv4f32 
    operator-(zv4f32 VecA, zv4f32 VecB) 
    {
        zv4f32 Result = {};

        Result.X = _mm_sub_ps(VecA.X, VecB.X);
        Result.Y = _mm_sub_ps(VecA.Y, VecB.Y);
        Result.Z = _mm_sub_ps(VecA.Z, VecB.Z);
        Result.W = _mm_sub_ps(VecA.W, VecB.W);

        return Result;
    }

    INTERNAL zv4f32 
    operator*(zv4f32 VecA, zv4f32 VecB) 
    {
        zv4f32 Result = {};

        Result.X = _mm_mul_ps(VecA.X, VecB.X);
        Result.Y = _mm_mul_ps(VecA.Y, VecB.Y);
        Result.Z = _mm_mul_ps(VecA.Z, VecB.Z);
        Result.W = _mm_mul_ps(VecA.W, VecB.W);

        return Result;
    }

    INTERNAL zv4f32 
    operator*(zv4f32 Vec, v128f Value) 
    {
        zv4f32 Result = {};

        Result.X = _mm_mul_ps(Vec.X, Value);
        Result.Y = _mm_mul_ps(Vec.Y, Value);
        Result.Z = _mm_mul_ps(Vec.Z, Value);
        Result.W = _mm_mul_ps(Vec.W, Value);

        return Result;
    }

    INTERNAL zv4f32 
    operator*(zv4f32 Vec, f32 Value) 
    {
        v128f ZValue = _mm_set1_ps(Value);

        return Vec * ZValue;
    }

    INTERNAL zv4f32 
    operator/(zv4f32 VecA, zv4f32 VecB) 
    {
        zv4f32 Result = {};

        Result.X = _mm_div_ps(VecA.X, VecB.X);
        Result.Y = _mm_div_ps(VecA.Y, VecB.Y);
        Result.Z = _mm_div_ps(VecA.Z, VecB.Z);
        Result.W = _mm_div_ps(VecA.W, VecB.W);

        return Result;
    }

    INTERNAL zv4f32 
    operator&(zv4f32 VecA, zv4f32 VecB) 
    {
        zv4f32 Result = {};

        Result.X = _mm_and_ps(VecA.X, VecB.X);
        Result.Y = _mm_and_ps(VecA.Y, VecB.Y);
        Result.Z = _mm_and_ps(VecA.Z, VecB.Z);
        Result.W = _mm_and_ps(VecA.W, VecB.W);

        return Result;
    }

    INTERNAL zv4f32 
    operator|(zv4f32 VecA, zv4f32 VecB) 
    {
        zv4f32 Result = {};

        Result.X = _mm_or_ps(VecA.X, VecB.X);
        Result.Y = _mm_or_ps(VecA.Y, VecB.Y);
        Result.Z = _mm_or_ps(VecA.Z, VecB.Z);
        Result.W = _mm_or_ps(VecA.W, VecB.W);

        return Result;
    }

    INTERNAL zv4f32 
    operator^(zv4f32 VecA, zv4f32 VecB) 
    {
        zv4f32 Result = {};

        Result.X = _mm_xor_ps(VecA.X, VecB.X);
        Result.Y = _mm_xor_ps(VecA.Y, VecB.Y);
        Result.Z = _mm_xor_ps(VecA.Z, VecB.Z);
        Result.W = _mm_xor_ps(VecA.W, VecB.W);

        return Result;
    }

    INTERNAL v128f
    operator==(zv4f32 VecA, zv4f32 VecB) 
    {
        return _mm_and_ps(
            _mm_and_ps(
                _mm_cmpeq_ps(VecA.X, VecB.X),
                _mm_cmpeq_ps(VecA.Y, VecB.Y)
            ),
            _mm_and_ps(
                _mm_cmpeq_ps(VecA.Z, VecB.Z),
                _mm_cmpeq_ps(VecA.W, VecB.W)
            )
        );
    }

    INTERNAL v128f
    operator!=(zv4f32 VecA, zv4f32 VecB) 
    {
        __mmask8 MaskX = _mm_cmp_ps_mask(VecA.X, VecB.X, _CMP_NEQ_OQ);
        __mmask8 MaskY = _mm_cmp_ps_mask(VecA.Y, VecB.Y, _CMP_NEQ_OQ);
        __mmask8 MaskZ = _mm_cmp_ps_mask(VecA.Z, VecB.Z, _CMP_NEQ_OQ);
        __mmask8 MaskW = _mm_cmp_ps_mask(VecA.W, VecB.W, _CMP_NEQ_OQ);
        __mmask8 ResultMask = _kand_mask8(
            _kand_mask8(MaskX, MaskY), 
            _kand_mask8(MaskZ, MaskW)
        );
        v128f Result = _mm_mask_mov_ps(
            _mm_setzero_ps(),
            ResultMask,
            _mm_set1_ps(1.0f)
        );

        return Result;
    }

    INTERNAL v128f
    operator<(zv4f32 VecA, zv4f32 VecB) 
    {
        __mmask8 MaskX = _mm_cmp_ps_mask(VecA.X, VecB.X, _CMP_LT_OQ);
        __mmask8 MaskY = _mm_cmp_ps_mask(VecA.Y, VecB.Y, _CMP_LT_OQ);
        __mmask8 MaskZ = _mm_cmp_ps_mask(VecA.Z, VecB.Z, _CMP_LT_OQ);
        __mmask8 MaskW = _mm_cmp_ps_mask(VecA.W, VecB.W, _CMP_LT_OQ);
        __mmask8 ResultMask = _kand_mask8(
            _kand_mask8(MaskX, MaskY), 
            _kand_mask8(MaskZ, MaskW)
        );
        v128f Result = _mm_mask_mov_ps(
            _mm_setzero_ps(),
            ResultMask,
            _mm_set1_ps(1.0f)
        );

        return Result;
    }

    INTERNAL v128f
    operator>(zv4f32 VecA, zv4f32 VecB) 
    {
        __mmask8 MaskX = _mm_cmp_ps_mask(VecA.X, VecB.X, _CMP_GT_OQ);
        __mmask8 MaskY = _mm_cmp_ps_mask(VecA.Y, VecB.Y, _CMP_GT_OQ);
        __mmask8 MaskZ = _mm_cmp_ps_mask(VecA.Z, VecB.Z, _CMP_GT_OQ);
        __mmask8 MaskW = _mm_cmp_ps_mask(VecA.W, VecB.W, _CMP_GT_OQ);
        __mmask8 ResultMask = _kand_mask8(
            _kand_mask8(MaskX, MaskY), 
            _kand_mask8(MaskZ, MaskW)
        );
        v128f Result = _mm_mask_mov_ps(
            _mm_setzero_ps(),
            ResultMask,
            _mm_set1_ps(1.0f)
        );

        return Result;
    }

    INTERNAL v128f
    operator<=(zv4f32 VecA, zv4f32 VecB) 
    {
        __mmask8 MaskX = _mm_cmp_ps_mask(VecA.X, VecB.X, _CMP_LE_OQ);
        __mmask8 MaskY = _mm_cmp_ps_mask(VecA.Y, VecB.Y, _CMP_LE_OQ);
        __mmask8 MaskZ = _mm_cmp_ps_mask(VecA.Z, VecB.Z, _CMP_LE_OQ);
        __mmask8 MaskW = _mm_cmp_ps_mask(VecA.W, VecB.W, _CMP_LE_OQ);
        __mmask8 ResultMask = _kand_mask8(
            _kand_mask8(MaskX, MaskY), 
            _kand_mask8(MaskZ, MaskW)
        );
        v128f Result = _mm_mask_mov_ps(
            _mm_setzero_ps(),
            ResultMask,
            _mm_set1_ps(1.0f)
        );

        return Result;
    }

    INTERNAL v128f
    operator>=(zv4f32 VecA, zv4f32 VecB) 
    {
        __mmask8 MaskX = _mm_cmp_ps_mask(VecA.X, VecB.X, _CMP_GE_OQ);
        __mmask8 MaskY = _mm_cmp_ps_mask(VecA.Y, VecB.Y, _CMP_GE_OQ);
        __mmask8 MaskZ = _mm_cmp_ps_mask(VecA.Z, VecB.Z, _CMP_GE_OQ);
        __mmask8 MaskW = _mm_cmp_ps_mask(VecA.W, VecB.W, _CMP_GE_OQ);
        __mmask8 ResultMask = _kand_mask8(
            _kand_mask8(MaskX, MaskY), 
            _kand_mask8(MaskZ, MaskW)
        );
        v128f Result = _mm_mask_mov_ps(
            _mm_setzero_ps(),
            ResultMask,
            _mm_set1_ps(1.0f)
        );

        return Result;
    }

    INTERNAL v128f
    Dot(zv4f32 VecA, zv4f32 VecB)
    {
        v128f Result = {};

        Result = _mm_fmadd_ps(
            VecA.X,
            VecB.X,
            _mm_fmadd_ps(
                VecA.Y,
                VecB.Y,
                _mm_fmadd_ps(
                    VecA.Z,
                    VecB.Z,
                    _mm_mul_ps(VecA.W, VecB.W)
                )
            )
        );

        return Result;
    }

    INTERNAL v128f
    LengthSq(zv4f32 Vec)
    {
        return Dot(Vec, Vec);
    }

    INTERNAL v128f
    Length(zv4f32 Vec)
    {
        return _mm_sqrt_ps(LengthSq(Vec));
    }

    INTERNAL zv4f32
    Norm(zv4f32 Vec)
    {
        v128f Scale = _mm_rsqrt14_ps(LengthSq(Vec));

        return Vec * Scale;
    }

    INTERNAL zv4f32
    Lerp(zv4f32 VecA, zv4f32 VecB, v128f T)
    {
        zv4f32 Result = {};

        Result.X = _mm_fmadd_ps(T, VecB.X, _mm_fnmadd_ps(T, VecA.X, VecA.X));
        Result.Y = _mm_fmadd_ps(T, VecB.Y, _mm_fnmadd_ps(T, VecA.Y, VecA.Y));
        Result.Z = _mm_fmadd_ps(T, VecB.Z, _mm_fnmadd_ps(T, VecA.Z, VecA.Z));
        Result.W = _mm_fmadd_ps(T, VecB.W, _mm_fnmadd_ps(T, VecA.W, VecA.W));

        return Result;
    }

    INTERNAL zv4f32
    Lerp(zv4f32 VecA, zv4f32 VecB, f32 T)
    {
        v128f ZT = _mm_set1_ps(T);

        return Lerp(VecA, VecB, ZT);
    }

    INTERNAL zv4i32 
    operator+(zv4i32 VecA, zv4i32 VecB) 
    {
        zv4i32 Result = {};

        Result.X = _mm_add_epi32(VecA.X, VecB.X);
        Result.Y = _mm_add_epi32(VecA.Y, VecB.Y);
        Result.Z = _mm_add_epi32(VecA.Z, VecB.Z);
        Result.W = _mm_add_epi32(VecA.W, VecB.W);

        return Result;
    }

    INTERNAL zv4i32 
    operator-(zv4i32 VecA, zv4i32 VecB) 
    {
        zv4i32 Result = {};

        Result.X = _mm_sub_epi32(VecA.X, VecB.X);
        Result.Y = _mm_sub_epi32(VecA.Y, VecB.Y);
        Result.Z = _mm_sub_epi32(VecA.Z, VecB.Z);
        Result.W = _mm_sub_epi32(VecA.W, VecB.W);

        return Result;
    }

    INTERNAL zv4i32 
    operator*(zv4i32 VecA, zv4i32 VecB) 
    {
        zv4i32 Result = {};

        Result.X = _mm_mullo_epi32(VecA.X, VecB.X);
        Result.Y = _mm_mullo_epi32(VecA.Y, VecB.Y);
        Result.Z = _mm_mullo_epi32(VecA.Z, VecB.Z);
        Result.W = _mm_mullo_epi32(VecA.W, VecB.W);

        return Result;
    }

    INTERNAL zv4i32 
    operator*(zv4i32 Vec, v128 Value) 
    {
        zv4i32 Result = {};

        Result.X = _mm_mullo_epi32(Vec.X, Value);
        Result.Y = _mm_mullo_epi32(Vec.Y, Value);
        Result.Z = _mm_mullo_epi32(Vec.Z, Value);
        Result.W = _mm_mullo_epi32(Vec.W, Value);

        return Result;
    }

    INTERNAL zv4i32 
    operator*(zv4i32 Vec, i32 Value) 
    {
        v128 ZValue = _mm_set1_epi32(Value);

        return Vec * ZValue;
    }

    INTERNAL zv4i32 
    operator/(zv4i32 VecA, zv4i32 VecB) 
    {
        zv4i32 Result = {};
        v128f FloatAX = _mm_cvtepi32_ps(VecA.X);
        v128f FloatAY = _mm_cvtepi32_ps(VecA.Y);
        v128f FloatAZ = _mm_cvtepi32_ps(VecA.Z);
        v128f FloatAW = _mm_cvtepi32_ps(VecA.W);
        v128f FloatBX = _mm_cvtepi32_ps(VecB.X);
        v128f FloatBY = _mm_cvtepi32_ps(VecB.Y);
        v128f FloatBZ = _mm_cvtepi32_ps(VecB.Z);
        v128f FloatBW = _mm_cvtepi32_ps(VecB.W);
        v128f QuotientX = _mm_div_ps(FloatAX, FloatBX);
        v128f QuotientY = _mm_div_ps(FloatAY, FloatBY);
        v128f QuotientZ = _mm_div_ps(FloatAZ, FloatBZ);
        v128f QuotientW = _mm_div_ps(FloatAW, FloatBW);

        Result.X = _mm_cvtps_epi32(QuotientX);
        Result.Y = _mm_cvtps_epi32(QuotientY);
        Result.Z = _mm_cvtps_epi32(QuotientZ);
        Result.W = _mm_cvtps_epi32(QuotientW);

        return Result;
    }

    INTERNAL zv4i32 
    operator&(zv4i32 VecA, zv4i32 VecB) 
    {
        zv4i32 Result = {};

        Result.X = _mm_and_epi32(VecA.X, VecB.X);
        Result.Y = _mm_and_epi32(VecA.Y, VecB.Y);
        Result.Z = _mm_and_epi32(VecA.Z, VecB.Z);
        Result.W = _mm_and_epi32(VecA.W, VecB.W);

        return Result;
    }

    INTERNAL zv4i32 
    operator|(zv4i32 VecA, zv4i32 VecB) 
    {
        zv4i32 Result = {};

        Result.X = _mm_or_epi32(VecA.X, VecB.X);
        Result.Y = _mm_or_epi32(VecA.Y, VecB.Y);
        Result.Z = _mm_or_epi32(VecA.Z, VecB.Z);
        Result.W = _mm_or_epi32(VecA.W, VecB.W);

        return Result;
    }

    INTERNAL zv4i32 
    operator^(zv4i32 VecA, zv4i32 VecB) 
    {
        zv4i32 Result = {};

        Result.X = _mm_xor_epi32(VecA.X, VecB.X);
        Result.Y = _mm_xor_epi32(VecA.Y, VecB.Y);
        Result.Z = _mm_xor_epi32(VecA.Z, VecB.Z);
        Result.W = _mm_xor_epi32(VecA.W, VecB.W);

        return Result;
    }

    INTERNAL v128 
    operator==(zv4i32 VecA, zv4i32 VecB) 
    {
        __mmask8 MaskX = _mm_cmp_epi32_mask(VecA.X, VecB.X, _MM_CMPINT_EQ);
        __mmask8 MaskY = _mm_cmp_epi32_mask(VecA.Y, VecB.Y, _MM_CMPINT_EQ);
        __mmask8 MaskZ = _mm_cmp_epi32_mask(VecA.Z, VecB.Z, _MM_CMPINT_EQ);
        __mmask8 MaskW = _mm_cmp_epi32_mask(VecA.W, VecB.W, _MM_CMPINT_EQ);
        __mmask8 ResultMask = _kand_mask8(
            _kand_mask8(MaskX, MaskY), 
            _kand_mask8(MaskZ, MaskW)
        );
        v128 Result = _mm_mask_mov_epi32(
            _mm_setzero_si128(),
            ResultMask,
            _mm_set1_epi32(~0)
        );

        return Result;
    }

    INTERNAL v128 
    operator!=(zv4i32 VecA, zv4i32 VecB) 
    {
        __mmask8 MaskX = _mm_cmp_epi32_mask(VecA.X, VecB.X, _MM_CMPINT_NE);
        __mmask8 MaskY = _mm_cmp_epi32_mask(VecA.Y, VecB.Y, _MM_CMPINT_NE);
        __mmask8 MaskZ = _mm_cmp_epi32_mask(VecA.Z, VecB.Z, _MM_CMPINT_NE);
        __mmask8 MaskW = _mm_cmp_epi32_mask(VecA.W, VecB.W, _MM_CMPINT_NE);
        __mmask8 ResultMask = _kand_mask8(
            _kand_mask8(MaskX, MaskY), 
            _kand_mask8(MaskZ, MaskW)
        );
        v128 Result = _mm_mask_mov_epi32(
            _mm_setzero_si128(),
            ResultMask,
            _mm_set1_epi32(~0)
        );

        return Result;
    }

    INTERNAL v128 
    operator<(zv4i32 VecA, zv4i32 VecB) 
    {
        __mmask8 MaskX = _mm_cmp_epi32_mask(VecA.X, VecB.X, _MM_CMPINT_LT);
        __mmask8 MaskY = _mm_cmp_epi32_mask(VecA.Y, VecB.Y, _MM_CMPINT_LT);
        __mmask8 MaskZ = _mm_cmp_epi32_mask(VecA.Z, VecB.Z, _MM_CMPINT_LT);
        __mmask8 MaskW = _mm_cmp_epi32_mask(VecA.W, VecB.W, _MM_CMPINT_LT);
        __mmask8 ResultMask = _kand_mask8(
            _kand_mask8(MaskX, MaskY), 
            _kand_mask8(MaskZ, MaskW)
        );
        v128 Result = _mm_mask_mov_epi32(
            _mm_setzero_si128(),
            ResultMask,
            _mm_set1_epi32(~0)
        );

        return Result;
    }

    INTERNAL v128 
    operator>(zv4i32 VecA, zv4i32 VecB) 
    {
        __mmask8 MaskX = _mm_cmp_epi32_mask(VecA.X, VecB.X, _MM_CMPINT_GT);
        __mmask8 MaskY = _mm_cmp_epi32_mask(VecA.Y, VecB.Y, _MM_CMPINT_GT);
        __mmask8 MaskZ = _mm_cmp_epi32_mask(VecA.Z, VecB.Z, _MM_CMPINT_GT);
        __mmask8 MaskW = _mm_cmp_epi32_mask(VecA.W, VecB.W, _MM_CMPINT_GT);
        __mmask8 ResultMask = _kand_mask8(
            _kand_mask8(MaskX, MaskY), 
            _kand_mask8(MaskZ, MaskW)
        );
        v128 Result = _mm_mask_mov_epi32(
            _mm_setzero_si128(),
            ResultMask,
            _mm_set1_epi32(~0)
        );

        return Result;
    }

    INTERNAL v128 
    operator<=(zv4i32 VecA, zv4i32 VecB) 
    {
        __mmask8 MaskX = _mm_cmp_epi32_mask(VecA.X, VecB.X, _MM_CMPINT_LE);
        __mmask8 MaskY = _mm_cmp_epi32_mask(VecA.Y, VecB.Y, _MM_CMPINT_LE);
        __mmask8 MaskZ = _mm_cmp_epi32_mask(VecA.Z, VecB.Z, _MM_CMPINT_LE);
        __mmask8 MaskW = _mm_cmp_epi32_mask(VecA.W, VecB.W, _MM_CMPINT_LE);
        __mmask8 ResultMask = _kand_mask8(
            _kand_mask8(MaskX, MaskY), 
            _kand_mask8(MaskZ, MaskW)
        );
        v128 Result = _mm_mask_mov_epi32(
            _mm_setzero_si128(),
            ResultMask,
            _mm_set1_epi32(~0)
        );

        return Result;
    }

    INTERNAL v128 
    operator>=(zv4i32 VecA, zv4i32 VecB) 
    {
        __mmask8 MaskX = _mm_cmp_epi32_mask(VecA.X, VecB.X, _MM_CMPINT_GE);
        __mmask8 MaskY = _mm_cmp_epi32_mask(VecA.Y, VecB.Y, _MM_CMPINT_GE);
        __mmask8 MaskZ = _mm_cmp_epi32_mask(VecA.Z, VecB.Z, _MM_CMPINT_GE);
        __mmask8 MaskW = _mm_cmp_epi32_mask(VecA.W, VecB.W, _MM_CMPINT_GE);
        __mmask8 ResultMask = _kand_mask8(
            _kand_mask8(MaskX, MaskY), 
            _kand_mask8(MaskZ, MaskW)
        );
        v128 Result = _mm_mask_mov_epi32(
            _mm_setzero_si128(),
            ResultMask,
            _mm_set1_epi32(~0)
        );

        return Result;
    }

    INTERNAL v128
    Dot(zv4i32 VecA, zv4i32 VecB)
    {
        v128 Result = {};

        Result = _mm_add_epi32(
            _mm_add_epi32(
                _mm_mullo_epi32(VecA.X, VecB.X),
                _mm_mullo_epi32(VecA.Y, VecB.Y)
            ),
            _mm_add_epi32(
                _mm_mullo_epi32(VecA.Z, VecB.Z),
                _mm_mullo_epi32(VecA.W, VecB.W)
            )
        );

        return Result;
    }

    INTERNAL v128
    LengthSq(zv4i32 Vec)
    {
        return Dot(Vec, Vec);
    }

    INTERNAL v128
    Length(zv4i32 Vec)
    {
        v128f LenSq = _mm_cvtepi32_ps(LengthSq(Vec));
        
        return _mm_cvtps_epi32(
            _mm_sqrt_ps(LenSq)
        );
    }

    INTERNAL zv4i32
    Norm(zv4i32 Vec)
    {
        v128 Scale = _mm_cvtps_epi32(
            _mm_rsqrt14_ps(
                _mm_cvtepi32_ps(
                    LengthSq(Vec)
                )
            )
        );

        return Vec * Scale;
    }

    INTERNAL zv4i32
    Lerp(zv4i32 VecA, zv4i32 VecB, v128 T)
    {
        zv4i32 Result = {};
        v128f ZT = _mm_cvtepi32_ps(T);
        
        v128f ResultX = _mm_fmadd_ps(
            ZT,
            _mm_cvtepi32_ps(VecB.X),
            _mm_fnmadd_ps(ZT, _mm_cvtepi32_ps(VecA.X), _mm_cvtepi32_ps(VecA.X))
        );
        v128f ResultY = _mm_fmadd_ps(
            ZT,
            _mm_cvtepi32_ps(VecB.Y),
            _mm_fnmadd_ps(ZT, _mm_cvtepi32_ps(VecA.Y), _mm_cvtepi32_ps(VecA.Y))
        );
        v128f ResultZ = _mm_fmadd_ps(
            ZT,
            _mm_cvtepi32_ps(VecB.Z),
            _mm_fnmadd_ps(ZT, _mm_cvtepi32_ps(VecA.Z), _mm_cvtepi32_ps(VecA.Z))
        );
        v128f ResultW = _mm_fmadd_ps(
            ZT,
            _mm_cvtepi32_ps(VecB.W),
            _mm_fnmadd_ps(ZT, _mm_cvtepi32_ps(VecA.W), _mm_cvtepi32_ps(VecA.W))
        );

        Result.X = _mm_cvtps_epi32(ResultX);
        Result.Y = _mm_cvtps_epi32(ResultY);
        Result.Z = _mm_cvtps_epi32(ResultZ);
        Result.W = _mm_cvtps_epi32(ResultW);

        return Result;
    }

    INTERNAL zv4i32
    Lerp(zv4i32 VecA, zv4i32 VecB, i32 T)
    {
        v128 ZT = _mm_set1_epi32(T);

        return Lerp(VecA, VecB, ZT);
    }

    INTERNAL zv4f64 
    operator+(zv4f64 VecA, zv4f64 VecB) 
    {
        zv4f64 Result = {};

        Result.X = _mm_add_pd(VecA.X, VecB.X);
        Result.Y = _mm_add_pd(VecA.Y, VecB.Y);
        Result.Z = _mm_add_pd(VecA.Z, VecB.Z);
        Result.W = _mm_add_pd(VecA.W, VecB.W);

        return Result;
    }

    INTERNAL zv4f64 
    operator-(zv4f64 VecA, zv4f64 VecB) 
    {
        zv4f64 Result = {};

        Result.X = _mm_sub_pd(VecA.X, VecB.X);
        Result.Y = _mm_sub_pd(VecA.Y, VecB.Y);
        Result.Z = _mm_sub_pd(VecA.Z, VecB.Z);
        Result.W = _mm_sub_pd(VecA.W, VecB.W);

        return Result;
    }

    INTERNAL zv4f64 
    operator*(zv4f64 VecA, zv4f64 VecB) 
    {
        zv4f64 Result = {};

        Result.X = _mm_mul_pd(VecA.X, VecB.X);
        Result.Y = _mm_mul_pd(VecA.Y, VecB.Y);
        Result.Z = _mm_mul_pd(VecA.Z, VecB.Z);
        Result.W = _mm_mul_pd(VecA.W, VecB.W);

        return Result;
    }

    INTERNAL zv4f64
    operator*(zv4f64 Vec, v128d Value) 
    {
        zv4f64 Result = {};

        Result.X = _mm_mul_pd(Vec.X, Value);
        Result.Y = _mm_mul_pd(Vec.Y, Value);
        Result.Z = _mm_mul_pd(Vec.Z, Value);
        Result.W = _mm_mul_pd(Vec.W, Value);

        return Result;
    }

    INTERNAL zv4f64
    operator*(zv4f64 Vec, f64 Value) 
    {
        v128d ZValue = _mm_set1_pd(Value);

        return Vec * ZValue;
    }

    INTERNAL zv4f64 
    operator/(zv4f64 VecA, zv4f64 VecB) 
    {
        zv4f64 Result = {};

        Result.X = _mm_div_pd(VecA.X, VecB.X);
        Result.Y = _mm_div_pd(VecA.Y, VecB.Y);
        Result.Z = _mm_div_pd(VecA.Z, VecB.Z);
        Result.W = _mm_div_pd(VecA.W, VecB.W);

        return Result;
    }

    INTERNAL zv4f64 
    operator&(zv4f64 VecA, zv4f64 VecB) 
    {
        zv4f64 Result = {};

        Result.X = _mm_and_pd(VecA.X, VecB.X);
        Result.Y = _mm_and_pd(VecA.Y, VecB.Y);
        Result.Z = _mm_and_pd(VecA.Z, VecB.Z);
        Result.W = _mm_and_pd(VecA.W, VecB.W);

        return Result;
    }

    INTERNAL zv4f64 
    operator|(zv4f64 VecA, zv4f64 VecB) 
    {
        zv4f64 Result = {};

        Result.X = _mm_or_pd(VecA.X, VecB.X);
        Result.Y = _mm_or_pd(VecA.Y, VecB.Y);
        Result.Z = _mm_or_pd(VecA.Z, VecB.Z);
        Result.W = _mm_or_pd(VecA.W, VecB.W);

        return Result;
    }

    INTERNAL zv4f64 
    operator^(zv4f64 VecA, zv4f64 VecB) 
    {
        zv4f64 Result = {};

        Result.X = _mm_xor_pd(VecA.X, VecB.X);
        Result.Y = _mm_xor_pd(VecA.Y, VecB.Y);
        Result.Z = _mm_xor_pd(VecA.Z, VecB.Z);
        Result.W = _mm_xor_pd(VecA.W, VecB.W);

        return Result;
    }

    INTERNAL v128d 
    operator==(zv4f64 VecA, zv4f64 VecB) 
    {
        __mmask8 MaskX = _mm_cmp_pd_mask(VecA.X, VecB.X, _CMP_EQ_OQ);
        __mmask8 MaskY = _mm_cmp_pd_mask(VecA.Y, VecB.Y, _CMP_EQ_OQ);
        __mmask8 MaskZ = _mm_cmp_pd_mask(VecA.Z, VecB.Z, _CMP_EQ_OQ);
        __mmask8 MaskW = _mm_cmp_pd_mask(VecA.W, VecB.W, _CMP_EQ_OQ);
        __mmask8 ResultMask = _kand_mask8(
            _kand_mask8(MaskX, MaskY), 
            _kand_mask8(MaskZ, MaskW)
        );
        v128d Result = _mm_mask_mov_pd(
            _mm_setzero_pd(),
            ResultMask,
            _mm_set1_pd(1.0)
        );

        return Result;
    }

    INTERNAL v128d 
    operator!=(zv4f64 VecA, zv4f64 VecB) 
    {
        __mmask8 MaskX = _mm_cmp_pd_mask(VecA.X, VecB.X, _CMP_NEQ_OQ);
        __mmask8 MaskY = _mm_cmp_pd_mask(VecA.Y, VecB.Y, _CMP_NEQ_OQ);
        __mmask8 MaskZ = _mm_cmp_pd_mask(VecA.Z, VecB.Z, _CMP_NEQ_OQ);
        __mmask8 MaskW = _mm_cmp_pd_mask(VecA.W, VecB.W, _CMP_NEQ_OQ);
        __mmask8 ResultMask = _kand_mask8(
            _kand_mask8(MaskX, MaskY), 
            _kand_mask8(MaskZ, MaskW)
        );
        v128d Result = _mm_mask_mov_pd(
            _mm_setzero_pd(),
            ResultMask,
            _mm_set1_pd(1.0)
        );

        return Result;
    }

    INTERNAL v128d 
    operator<(zv4f64 VecA, zv4f64 VecB) 
    {
        __mmask8 MaskX = _mm_cmp_pd_mask(VecA.X, VecB.X, _CMP_LT_OQ);
        __mmask8 MaskY = _mm_cmp_pd_mask(VecA.Y, VecB.Y, _CMP_LT_OQ);
        __mmask8 MaskZ = _mm_cmp_pd_mask(VecA.Z, VecB.Z, _CMP_LT_OQ);
        __mmask8 MaskW = _mm_cmp_pd_mask(VecA.W, VecB.W, _CMP_LT_OQ);
        __mmask8 ResultMask = _kand_mask8(
            _kand_mask8(MaskX, MaskY), 
            _kand_mask8(MaskZ, MaskW)
        );
        v128d Result = _mm_mask_mov_pd(
            _mm_setzero_pd(),
            ResultMask,
            _mm_set1_pd(1.0)
        );

        return Result;
    }

    INTERNAL v128d 
    operator>(zv4f64 VecA, zv4f64 VecB) 
    {
        __mmask8 MaskX = _mm_cmp_pd_mask(VecA.X, VecB.X, _CMP_GT_OQ);
        __mmask8 MaskY = _mm_cmp_pd_mask(VecA.Y, VecB.Y, _CMP_GT_OQ);
        __mmask8 MaskZ = _mm_cmp_pd_mask(VecA.Z, VecB.Z, _CMP_GT_OQ);
        __mmask8 MaskW = _mm_cmp_pd_mask(VecA.W, VecB.W, _CMP_GT_OQ);
        __mmask8 ResultMask = _kand_mask8(
            _kand_mask8(MaskX, MaskY), 
            _kand_mask8(MaskZ, MaskW)
        );
        v128d Result = _mm_mask_mov_pd(
            _mm_setzero_pd(),
            ResultMask,
            _mm_set1_pd(1.0)
        );

        return Result;
    }

    INTERNAL v128d 
    operator<=(zv4f64 VecA, zv4f64 VecB) 
    {
        __mmask8 MaskX = _mm_cmp_pd_mask(VecA.X, VecB.X, _CMP_LE_OQ);
        __mmask8 MaskY = _mm_cmp_pd_mask(VecA.Y, VecB.Y, _CMP_LE_OQ);
        __mmask8 MaskZ = _mm_cmp_pd_mask(VecA.Z, VecB.Z, _CMP_LE_OQ);
        __mmask8 MaskW = _mm_cmp_pd_mask(VecA.W, VecB.W, _CMP_LE_OQ);
        __mmask8 ResultMask = _kand_mask8(
            _kand_mask8(MaskX, MaskY), 
            _kand_mask8(MaskZ, MaskW)
        );
        v128d Result = _mm_mask_mov_pd(
            _mm_setzero_pd(),
            ResultMask,
            _mm_set1_pd(1.0)
        );

        return Result;
    }

    INTERNAL v128d 
    operator>=(zv4f64 VecA, zv4f64 VecB) 
    {
        __mmask8 MaskX = _mm_cmp_pd_mask(VecA.X, VecB.X, _CMP_GE_OQ);
        __mmask8 MaskY = _mm_cmp_pd_mask(VecA.Y, VecB.Y, _CMP_GE_OQ);
        __mmask8 MaskZ = _mm_cmp_pd_mask(VecA.Z, VecB.Z, _CMP_GE_OQ);
        __mmask8 MaskW = _mm_cmp_pd_mask(VecA.W, VecB.W, _CMP_GE_OQ);
        __mmask8 ResultMask = _kand_mask8(
            _kand_mask8(MaskX, MaskY), 
            _kand_mask8(MaskZ, MaskW)
        );
        v128d Result = _mm_mask_mov_pd(
            _mm_setzero_pd(),
            ResultMask,
            _mm_set1_pd(1.0)
        );

        return Result;
    }

    INTERNAL v128d
    Dot(zv4f64 VecA, zv4f64 VecB)
    {
        v128d Result = {};

        Result = _mm_fmadd_pd(
            VecA.X,
            VecB.X,
            _mm_fmadd_pd(
                VecA.Y,
                VecB.Y,
                _mm_fmadd_pd(
                    VecA.Z,
                    VecB.Z,
                    _mm_mul_pd(VecA.W, VecB.W)
                )
            )
        );

        return Result;
    }

    INTERNAL v128d
    LengthSq(zv4f64 Vec)
    {
        return Dot(Vec, Vec);
    }

    INTERNAL v128d
    Length(zv4f64 Vec)
    {
        return _mm_sqrt_pd(LengthSq(Vec));
    }

    INTERNAL zv4f64
    Norm(zv4f64 Vec)
    {
        v128d Scale = _mm_rsqrt14_pd(LengthSq(Vec));

        return Vec * Scale;
    }

    INTERNAL zv4f64
    Lerp(zv4f64 VecA, zv4f64 VecB, v128d T)
    {
        zv4f64 Result = {};

        Result.X = _mm_fmadd_pd(T, VecB.X, _mm_fnmadd_pd(T, VecA.X, VecA.X));
        Result.Y = _mm_fmadd_pd(T, VecB.Y, _mm_fnmadd_pd(T, VecA.Y, VecA.Y));
        Result.Z = _mm_fmadd_pd(T, VecB.Z, _mm_fnmadd_pd(T, VecA.Z, VecA.Z));
        Result.W = _mm_fmadd_pd(T, VecB.W, _mm_fnmadd_pd(T, VecA.W, VecA.W));

        return Result;
    }

    INTERNAL zv4f64
    Lerp(zv4f64 VecA, zv4f64 VecB, f64 T)
    {
        v128d ZT = _mm_set1_pd(T);

        return Lerp(VecA, VecB, ZT);
    }

    INTERNAL zr1i32
    Rng(v128 Min, v128 Max)
    {
        zr1i32 Result = {};

        Result.Min = Min;
        Result.Max = Max;

        return Result;
    }

    INTERNAL zr1i32
    Shift(zr1i32 Range, v128 Val)
    {
        zr1i32 Result = {};

        Result.Min = _mm_add_epi32(Range.Min, Val);
        Result.Max = _mm_add_epi32(Range.Max, Val);

        return Result;
    }

    INTERNAL zr1i32
    Widen(zr1i32 Range, v128 Val)
    {
        zr1i32 Result = {};

        Result.Min = _mm_sub_epi32(Range.Min, Val);
        Result.Max = _mm_add_epi32(Range.Max, Val);

        return Result;
    }

    INTERNAL v128
    Centre(zr1i32 Range)
    {
        v128f Float = _mm_cvtepi32_ps(
            _mm_add_epi32(Range.Min, Range.Max)
        );
        v128f Quotient = _mm_div_ps(Float, _mm_set1_ps(2.0f));

        return _mm_cvtps_epi32(Quotient);
    }

    INTERNAL v128
    InRange(zr1i32 Range, v128 Val)
    {
        __mmask8 MinMask = _mm_cmp_epi32_mask(Range.Min, Val, _MM_CMPINT_GE);
        __mmask8 MaxMask = _mm_cmp_epi32_mask(Val, Range.Max, _MM_CMPINT_LT);
        __mmask8 ResultMask = _kand_mask8(MinMask, MaxMask);

        return _mm_mask_mov_epi32(
            _mm_setzero_si128(),
            ResultMask,
            _mm_set1_epi32(1)
        );
    }

    INTERNAL v128
    Length(zr1i32 Range)
    {
        return _mm_sub_epi32(Range.Max, Range.Min);
    }

    INTERNAL zr1i32
    Union(zr1i32 RangeA, zr1i32 RangeB)
    {
        zr1i32 Result = {};

        Result.Min = _mm_min_epi32(RangeA.Min, RangeB.Min);
        Result.Max = _mm_max_epi32(RangeA.Max, RangeB.Max);

        return Result;
    }

    INTERNAL zr1i32
    Intersect(zr1i32 RangeA, zr1i32 RangeB)
    {
        zr1i32 Result = {};

        Result.Min = _mm_max_epi32(RangeA.Min, RangeB.Min);
        Result.Max = _mm_min_epi32(RangeA.Max, RangeB.Max);

        return Result;
    }

    INTERNAL v128
    Clamp(zr1i32 Range, v128 Val)
    {
        Val = _mm_max_epi32(Range.Min, Val);
        Val = _mm_min_epi32(Range.Max, Val);

        return Val;
    }

    INTERNAL zr1f32
    Rng(v128f Min, v128f Max)
    {
        zr1f32 Result = {};

        Result.Min = Min;
        Result.Max = Max;

        return Result;
    }

    INTERNAL zr1f32
    Shift(zr1f32 Range, v128f Val)
    {
        zr1f32 Result = {};

        Result.Min = _mm_add_ps(Range.Min, Val);
        Result.Max = _mm_add_ps(Range.Max, Val);

        return Result;
    }

    INTERNAL zr1f32
    Widen(zr1f32 Range, v128f Val)
    {
        zr1f32 Result = {};

        Result.Min = _mm_sub_ps(Range.Min, Val);
        Result.Max = _mm_add_ps(Range.Max, Val);

        return Result;
    }

    INTERNAL v128f
    Centre(zr1f32 Range)
    {
        return _mm_mul_ps(
            _mm_add_ps(Range.Min, Range.Max), 
            _mm_set1_ps(0.5f)
        );
    }

    INTERNAL v128f
    InRange(zr1f32 Range, v128f Val)
    {
        __mmask8 MinMask = _mm_cmp_ps_mask(Range.Min, Val, _CMP_GE_OQ);
        __mmask8 MaxMask = _mm_cmp_ps_mask(Val, Range.Max, _CMP_LT_OQ);
        __mmask8 ResultMask = _kand_mask8(MinMask, MaxMask);

        return _mm_mask_mov_ps(
            _mm_setzero_ps(),
            ResultMask,
            _mm_set1_ps(1.0f)
        );
    }

    INTERNAL v128f
    Length(zr1f32 Range)
    {
        return _mm_sub_ps(Range.Max, Range.Min);
    }

    INTERNAL zr1f32
    Union(zr1f32 RangeA, zr1f32 RangeB)
    {
        zr1f32 Result = {};

        Result.Min = _mm_min_ps(RangeA.Min, RangeB.Min);
        Result.Max = _mm_max_ps(RangeA.Max, RangeB.Max);

        return Result;
    }

    INTERNAL zr1f32
    Intersect(zr1f32 RangeA, zr1f32 RangeB)
    {
        zr1f32 Result = {};

        Result.Min = _mm_max_ps(RangeA.Min, RangeB.Min);
        Result.Max = _mm_min_ps(RangeA.Max, RangeB.Max);

        return Result;
    }

    INTERNAL v128f
    Clamp(zr1f32 Range, v128f Val)
    {
        Val = _mm_max_ps(Range.Min, Val);
        Val = _mm_min_ps(Range.Max, Val);

        return Val;
    }

    INTERNAL zr1f64
    Rng(v128d Min, v128d Max)
    {
        zr1f64 Result = {};

        Result.Min = Min;
        Result.Max = Max;

        return Result;
    }

    INTERNAL zr1f64
    Shift(zr1f64 Range, v128d Val)
    {
        zr1f64 Result = {};

        Result.Min = _mm_add_pd(Range.Min, Val);
        Result.Max = _mm_add_pd(Range.Max, Val);

        return Result;
    }

    INTERNAL zr1f64
    Widen(zr1f64 Range, v128d Val)
    {
        zr1f64 Result = {};

        Result.Min = _mm_sub_pd(Range.Min, Val);
        Result.Max = _mm_add_pd(Range.Max, Val);

        return Result;
    }

    INTERNAL v128d
    Centre(zr1f64 Range)
    {
        return _mm_mul_pd(
            _mm_add_pd(Range.Min, Range.Max), 
            _mm_set1_pd(0.5)
        );
    }

    INTERNAL v128d
    InRange(zr1f64 Range, v128d Val)
    {
        __mmask8 MinMask = _mm_cmp_pd_mask(Range.Min, Val, _CMP_GE_OQ);
        __mmask8 MaxMask = _mm_cmp_pd_mask(Val, Range.Max, _CMP_LT_OQ);
        __mmask8 ResultMask = _kand_mask8(MinMask, MaxMask);

        return _mm_mask_mov_pd(
            _mm_setzero_pd(),
            ResultMask,
            _mm_set1_pd(1.0)
        );
    }

    INTERNAL v128d
    Length(zr1f64 Range)
    {
        return _mm_sub_pd(Range.Max, Range.Min);
    }

    INTERNAL zr1f64
    Union(zr1f64 RangeA, zr1f64 RangeB)
    {
        zr1f64 Result = {};

        Result.Min = _mm_min_pd(RangeA.Min, RangeB.Min);
        Result.Max = _mm_max_pd(RangeA.Max, RangeB.Max);

        return Result;
    }

    INTERNAL zr1f64
    Intersect(zr1f64 RangeA, zr1f64 RangeB)
    {
        zr1f64 Result = {};

        Result.Min = _mm_max_pd(RangeA.Min, RangeB.Min);
        Result.Max = _mm_min_pd(RangeA.Max, RangeB.Max);

        return Result;
    }

    INTERNAL v128d
    Clamp(zr1f64 Range, v128d Val)
    {
        Val = _mm_max_pd(Range.Min, Val);
        Val = _mm_min_pd(Range.Max, Val);

        return Val;
    }

    INTERNAL zr2f32
    Rng(zv2f32 Min, zv2f32 Max)
    {
        zr2f32 Result = {};

        Result.Min = Min;
        Result.Max = Max;

        return Result;
    }

    INTERNAL zr2f32
    Shift(zr2f32 Range, zv2f32 Val)
    {
        zr2f32 Result = {};

        Result.Min = Range.Min + Val;
        Result.Max = Range.Max + Val;

        return Result;
    }

    INTERNAL zr2f32
    Widen(zr2f32 Range, zv2f32 Val)
    {
        zr2f32 Result = {};

        Result.Min = Range.Min - Val;
        Result.Max = Range.Max + Val;

        return Result;
    }

    INTERNAL zv2f32
    Centre(zr2f32 Range)
    {
        return (Range.Min + Range.Max) / 2.0f;
    }

    INTERNAL v128f
    InRange(zr2f32 Range, zv2f32 Val)
    {
        return _mm_and_ps(
            Val >= Range.Min,
            Val < Range.Max
        );
    }

    INTERNAL zv2f32
    Length(zr2f32 Range)
    {
        return Range.Max - Range.Min;
    }

    INTERNAL zr2f32
    Union(zr2f32 RangeA, zr2f32 RangeB)
    {
        zr2f32 Result = {};

        Result.Min.X = _mm_min_ps(RangeA.Min.X, RangeB.Min.X);
        Result.Min.Y = _mm_min_ps(RangeA.Min.Y, RangeB.Min.Y);
        Result.Max.X = _mm_max_ps(RangeA.Max.X, RangeB.Max.X);
        Result.Max.Y = _mm_max_ps(RangeA.Max.Y, RangeB.Max.Y);

        return Result;
    }

    INTERNAL zr2f32
    Intersect(zr2f32 RangeA, zr2f32 RangeB)
    {
        zr2f32 Result = {};

        Result.Min.X = _mm_max_ps(RangeA.Min.X, RangeB.Min.X);
        Result.Min.Y = _mm_max_ps(RangeA.Min.Y, RangeB.Min.Y);
        Result.Max.X = _mm_min_ps(RangeA.Max.X, RangeB.Max.X);
        Result.Max.Y = _mm_min_ps(RangeA.Max.Y, RangeB.Max.Y);

        return Result;
    }

    INTERNAL zv2f32
    Clamp(zr2f32 Range, zv2f32 Val)
    {
        Val.X = _mm_max_ps(Range.Min.X, Val.X);
        Val.Y = _mm_max_ps(Range.Min.Y, Val.Y);
        Val.X = _mm_min_ps(Range.Max.X, Val.X);
        Val.Y = _mm_min_ps(Range.Max.Y, Val.Y);

        return Val;
    }

    INTERNAL zr2f64
    Rng(zv2f64 Min, zv2f64 Max)
    {
        zr2f64 Result = {};

        Result.Min = Min;
        Result.Max = Max;

        return Result;
    }

    INTERNAL zr2f64
    Shift(zr2f64 Range, zv2f64 Val)
    {
        zr2f64 Result = {};

        Result.Min = Range.Min + Val;
        Result.Max = Range.Max + Val;

        return Result;
    }

    INTERNAL zr2f64
    Widen(zr2f64 Range, zv2f64 Val)
    {
        zr2f64 Result = {};

        Result.Min = Range.Min - Val;
        Result.Max = Range.Max + Val;

        return Result;
    }

    INTERNAL zv2f64
    Centre(zr2f64 Range)
    {
        return (Range.Min + Range.Max) / 2.0;
    }

    INTERNAL v128d
    InRange(zr2f64 Range, zv2f64 Val)
    {
        return _mm_and_pd(
            Val >= Range.Min,
            Val < Range.Max
        );
    }

    INTERNAL zv2f64
    Length(zr2f64 Range)
    {
        return Range.Max - Range.Min;
    }

    INTERNAL zr2f64
    Union(zr2f64 RangeA, zr2f64 RangeB)
    {
        zr2f64 Result = {};

        Result.Min.X = _mm_min_pd(RangeA.Min.X, RangeB.Min.X);
        Result.Min.Y = _mm_min_pd(RangeA.Min.Y, RangeB.Min.Y);
        Result.Max.X = _mm_max_pd(RangeA.Max.X, RangeB.Max.X);
        Result.Max.Y = _mm_max_pd(RangeA.Max.Y, RangeB.Max.Y);

        return Result;
    }

    INTERNAL zr2f64
    Intersect(zr2f64 RangeA, zr2f64 RangeB)
    {
        zr2f64 Result = {};

        Result.Min.X = _mm_max_pd(RangeA.Min.X, RangeB.Min.X);
        Result.Min.Y = _mm_max_pd(RangeA.Min.Y, RangeB.Min.Y);
        Result.Max.X = _mm_min_pd(RangeA.Max.X, RangeB.Max.X);
        Result.Max.Y = _mm_min_pd(RangeA.Max.Y, RangeB.Max.Y);

        return Result;
    }

    INTERNAL zv2f64
    Clamp(zr2f64 Range, zv2f64 Val)
    {
        Val.X = _mm_max_pd(Range.Min.X, Val.X);
        Val.Y = _mm_max_pd(Range.Min.Y, Val.Y);
        Val.X = _mm_min_pd(Range.Max.X, Val.X);
        Val.Y = _mm_min_pd(Range.Max.Y, Val.Y);

        return Val;
    }

    INTERNAL zr3f64
    Rng(zv3f64 Min, zv3f64 Max)
    {
        zr3f64 Result = {};

        Result.Min = Min;
        Result.Max = Max;

        return Result;
    }

    INTERNAL zr3f64
    Shift(zr3f64 Range, zv3f64 Val)
    {
        zr3f64 Result = {};

        Result.Min = Range.Min + Val;
        Result.Max = Range.Max + Val;

        return Result;
    }

    INTERNAL zr3f64
    Widen(zr3f64 Range, zv3f64 Val)
    {
        zr3f64 Result = {};

        Result.Min = Range.Min - Val;
        Result.Max = Range.Max + Val;

        return Result;
    }

    INTERNAL zv3f64
    Centre(zr3f64 Range)
    {
        return (Range.Min + Range.Max) / 2.0;
    }

    INTERNAL v128d
    InRange(zr3f64 Range, zv3f64 Val)
    {
        return _mm_and_pd(
            Val >= Range.Min,
            Val < Range.Max
        );
    }

    INTERNAL zv3f64
    Length(zr3f64 Range)
    {
        return Range.Max - Range.Min;
    }

    INTERNAL zr3f64
    Union(zr3f64 RangeA, zr3f64 RangeB)
    {
        zr3f64 Result = {};

        Result.Min.X = _mm_min_pd(RangeA.Min.X, RangeB.Min.X);
        Result.Min.Y = _mm_min_pd(RangeA.Min.Y, RangeB.Min.Y);
        Result.Min.Z = _mm_min_pd(RangeA.Min.Z, RangeB.Min.Z);
        Result.Max.X = _mm_max_pd(RangeA.Max.X, RangeB.Max.X);
        Result.Max.Y = _mm_max_pd(RangeA.Max.Y, RangeB.Max.Y);
        Result.Max.Z = _mm_max_pd(RangeA.Max.Z, RangeB.Max.Z);

        return Result;
    }

    INTERNAL zr3f64
    Intersect(zr3f64 RangeA, zr3f64 RangeB)
    {
        zr3f64 Result = {};

        Result.Min.X = _mm_max_pd(RangeA.Min.X, RangeB.Min.X);
        Result.Min.Y = _mm_max_pd(RangeA.Min.Y, RangeB.Min.Y);
        Result.Min.Z = _mm_max_pd(RangeA.Min.Z, RangeB.Min.Z);
        Result.Max.X = _mm_min_pd(RangeA.Max.X, RangeB.Max.X);
        Result.Max.Y = _mm_min_pd(RangeA.Max.Y, RangeB.Max.Y);
        Result.Max.Z = _mm_min_pd(RangeA.Max.Z, RangeB.Max.Z);

        return Result;
    }

    INTERNAL zv3f64
    Clamp(zr3f64 Range, zv3f64 Val)
    {
        Val.X = _mm_max_pd(Range.Min.X, Val.X);
        Val.Y = _mm_max_pd(Range.Min.Y, Val.Y);
        Val.Z = _mm_max_pd(Range.Min.Z, Val.Z);
        Val.X = _mm_min_pd(Range.Max.X, Val.X);
        Val.Y = _mm_min_pd(Range.Max.Y, Val.Y);
        Val.Z = _mm_min_pd(Range.Max.Z, Val.Z);

        return Val;
    }

    INTERNAL zr3f32
    Rng(zv3f32 Min, zv3f32 Max)
    {
        zr3f32 Result = {};

        Result.Min = Min;
        Result.Max = Max;

        return Result;
    }

    INTERNAL zr3f32
    Shift(zr3f32 Range, zv3f32 Val)
    {
        zr3f32 Result = {};

        Result.Min = Range.Min + Val;
        Result.Max = Range.Max + Val;

        return Result;
    }

    INTERNAL zr3f32
    Widen(zr3f32 Range, zv3f32 Val)
    {
        zr3f32 Result = {};

        Result.Min = Range.Min - Val;
        Result.Max = Range.Max + Val;

        return Result;
    }

    INTERNAL zv3f32
    Centre(zr3f32 Range)
    {
        return (Range.Min + Range.Max) / 2.0f;
    }

    INTERNAL v128f
    InRange(zr3f32 Range, zv3f32 Val)
    {
        return _mm_and_ps(
            Val >= Range.Min,
            Val < Range.Max
        );
    }

    INTERNAL zv3f32
    Length(zr3f32 Range)
    {
        return Range.Max - Range.Min;
    }

    INTERNAL zr3f32
    Union(zr3f32 RangeA, zr3f32 RangeB)
    {
        zr3f32 Result = {};

        Result.Min.X = _mm_min_ps(RangeA.Min.X, RangeB.Min.X);
        Result.Min.Y = _mm_min_ps(RangeA.Min.Y, RangeB.Min.Y);
        Result.Min.Z = _mm_min_ps(RangeA.Min.Z, RangeB.Min.Z);
        Result.Max.X = _mm_max_ps(RangeA.Max.X, RangeB.Max.X);
        Result.Max.Y = _mm_max_ps(RangeA.Max.Y, RangeB.Max.Y);
        Result.Max.Z = _mm_max_ps(RangeA.Max.Z, RangeB.Max.Z);

        return Result;
    }

    INTERNAL zr3f32
    Intersect(zr3f32 RangeA, zr3f32 RangeB)
    {
        zr3f32 Result = {};

        Result.Min.X = _mm_max_ps(RangeA.Min.X, RangeB.Min.X);
        Result.Min.Y = _mm_max_ps(RangeA.Min.Y, RangeB.Min.Y);
        Result.Min.Z = _mm_max_ps(RangeA.Min.Z, RangeB.Min.Z);
        Result.Max.X = _mm_min_ps(RangeA.Max.X, RangeB.Max.X);
        Result.Max.Y = _mm_min_ps(RangeA.Max.Y, RangeB.Max.Y);
        Result.Max.Z = _mm_min_ps(RangeA.Max.Z, RangeB.Max.Z);

        return Result;
    }

    INTERNAL zv3f32
    Clamp(zr3f32 Range, zv3f32 Val)
    {
        Val.X = _mm_max_ps(Range.Min.X, Val.X);
        Val.Y = _mm_max_ps(Range.Min.Y, Val.Y);
        Val.Z = _mm_max_ps(Range.Min.Z, Val.Z);
        Val.X = _mm_min_ps(Range.Max.X, Val.X);
        Val.Y = _mm_min_ps(Range.Max.Y, Val.Y);
        Val.Z = _mm_min_ps(Range.Max.Z, Val.Z);

        return Val;
    }
#else
    typedef zv2f32 v2f32;
    typedef zv2i32 v2i32;
    typedef zv2f64 v2f64;
    typedef zv3f32 v3f32;
    typedef zv3i32 v3i32;
    typedef zv3f64 v3f64;
    typedef zv4f32 v4f32;
    typedef zv4i32 v4i32;
    typedef zv4f64 v4f64;
    typedef zr1i32 r1i32;
    typedef zr1f32 r1f32;
    typedef zr1f64 r1f64;
    typedef zr2i32 r2i32;
    typedef zr2f32 r2f32;
    typedef zr2f64 r2f64;
    typedef zr3f32 r3f32;
    typedef zr3i32 r3i32;
    typedef zr3f64 r3f64;
#endif

INTERNAL zv2f32& 
operator+=(zv2f32& VecA, zv2f32 VecB)
{
    VecA = VecA + VecB;

    return VecA;
}

INTERNAL zv2f32& 
operator-=(zv2f32& VecA, zv2f32 VecB)
{
    VecA = VecA - VecB;

    return VecA;
}

INTERNAL zv2f32& 
operator*=(zv2f32& VecA, zv2f32 VecB)
{
    VecA = VecA * VecB;

    return VecA;
}

INTERNAL zv2f32& 
operator/=(zv2f32& VecA, zv2f32 VecB)
{
    VecA = VecA / VecB;

    return VecA;
}

INTERNAL zv2f32& 
operator^=(zv2f32& VecA, zv2f32 VecB)
{
    VecA = VecA ^ VecB;

    return VecA;
}

INTERNAL zv2i32& 
operator+=(zv2i32& VecA, zv2i32 VecB)
{
    VecA = VecA + VecB;

    return VecA;
}

INTERNAL zv2i32& 
operator-=(zv2i32& VecA, zv2i32 VecB)
{
    VecA = VecA - VecB;

    return VecA;
}

INTERNAL zv2i32& 
operator*=(zv2i32& VecA, zv2i32 VecB)
{
    VecA = VecA * VecB;

    return VecA;
}

INTERNAL zv2i32& 
operator/=(zv2i32& VecA, zv2i32 VecB)
{
    VecA = VecA / VecB;

    return VecA;
}

INTERNAL zv2f64& 
operator+=(zv2f64& VecA, zv2f64 VecB)
{
    VecA = VecA + VecB;

    return VecA;
}

INTERNAL zv2f64& 
operator-=(zv2f64& VecA, zv2f64 VecB)
{
    VecA = VecA - VecB;

    return VecA;
}

INTERNAL zv2f64& 
operator*=(zv2f64& VecA, zv2f64 VecB)
{
    VecA = VecA * VecB;

    return VecA;
}

INTERNAL zv2f64& 
operator/=(zv2f64& VecA, zv2f64 VecB)
{
    VecA = VecA / VecB;

    return VecA;
}

INTERNAL zv3f32& 
operator+=(zv3f32& VecA, zv3f32 VecB)
{
    VecA = VecA + VecB;

    return VecA;
}

INTERNAL zv3f32& 
operator-=(zv3f32& VecA, zv3f32 VecB)
{
    VecA = VecA - VecB;

    return VecA;
}

INTERNAL zv3f32& 
operator*=(zv3f32& VecA, zv3f32 VecB)
{
    VecA = VecA * VecB;

    return VecA;
}

INTERNAL zv3f32& 
operator/=(zv3f32& VecA, zv3f32 VecB)
{
    VecA = VecA / VecB;

    return VecA;
}

INTERNAL zv3i32& 
operator+=(zv3i32& VecA, zv3i32 VecB)
{
    VecA = VecA + VecB;

    return VecA;
}

INTERNAL zv3i32& 
operator-=(zv3i32& VecA, zv3i32 VecB)
{
    VecA = VecA - VecB;

    return VecA;
}

INTERNAL zv3i32& 
operator*=(zv3i32& VecA, zv3i32 VecB)
{
    VecA = VecA * VecB;

    return VecA;
}

INTERNAL zv3i32& 
operator/=(zv3i32& VecA, zv3i32 VecB)
{
    VecA = VecA / VecB;

    return VecA;
}

INTERNAL zv3f64& 
operator+=(zv3f64& VecA, zv3f64 VecB)
{
    VecA = VecA + VecB;

    return VecA;
}

INTERNAL zv3f64& 
operator-=(zv3f64& VecA, zv3f64 VecB)
{
    VecA = VecA - VecB;

    return VecA;
}

INTERNAL zv3f64& 
operator*=(zv3f64& VecA, zv3f64 VecB)
{
    VecA = VecA * VecB;

    return VecA;
}

INTERNAL zv3f64& 
operator/=(zv3f64& VecA, zv3f64 VecB)
{
    VecA = VecA / VecB;

    return VecA;
}

INTERNAL zv4f32& 
operator+=(zv4f32& VecA, zv4f32 VecB)
{
    VecA = VecA + VecB;

    return VecA;
}

INTERNAL zv4f32& 
operator-=(zv4f32& VecA, zv4f32 VecB)
{
    VecA = VecA - VecB;

    return VecA;
}

INTERNAL zv4f32& 
operator*=(zv4f32& VecA, zv4f32 VecB)
{
    VecA = VecA * VecB;

    return VecA;
}

INTERNAL zv4f32& 
operator/=(zv4f32& VecA, zv4f32 VecB)
{
    VecA = VecA / VecB;

    return VecA;
}

INTERNAL zv4i32& 
operator+=(zv4i32& VecA, zv4i32 VecB)
{
    VecA = VecA + VecB;

    return VecA;
}

INTERNAL zv4i32& 
operator-=(zv4i32& VecA, zv4i32 VecB)
{
    VecA = VecA - VecB;

    return VecA;
}

INTERNAL zv4i32& 
operator*=(zv4i32& VecA, zv4i32 VecB)
{
    VecA = VecA * VecB;

    return VecA;
}

INTERNAL zv4i32& 
operator/=(zv4i32& VecA, zv4i32 VecB)
{
    VecA = VecA / VecB;

    return VecA;
}

INTERNAL zv4f64& 
operator+=(zv4f64& VecA, zv4f64 VecB)
{
    VecA = VecA + VecB;

    return VecA;
}

INTERNAL zv4f64& 
operator-=(zv4f64& VecA, zv4f64 VecB)
{
    VecA = VecA - VecB;

    return VecA;
}

INTERNAL zv4f64& 
operator*=(zv4f64& VecA, zv4f64 VecB)
{
    VecA = VecA * VecB;

    return VecA;
}

INTERNAL zv4f64& 
operator/=(zv4f64& VecA, zv4f64 VecB)
{
    VecA = VecA / VecB;

    return VecA;
}

#endif // __NC_SIMD_H__
