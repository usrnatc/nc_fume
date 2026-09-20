#include "nc_math.h"
#include "nc_memory.h"
#include "nc_arena.h"
#include "nc_simd.h"

API_INTERNAL f32
_INF32(void)
{
    union {
        u32 U;
        f32 F;
    } X;

    X.U = 0x7F800000;

    return X.F;
}

INTERNAL u64
Pow10(u64 Val)
{
    ASSERT_ALWAYS(Val < 20);

    if (!Val)
        return 1;

    u64 Result = 1;

    for (u64 I = 0; I < Val; ++I)
        Result = (Result << 3ULL) + (Result << 1ULL);

    return Result;
}

INTERNAL u32
SafeTruncateU64(u64 Value)
{
    if (Value > U32_MAX)
        return U32_MAX;

    return (u32) Value;
}

INTERNAL u16
SafeTruncateU32(u32 Value)
{
    if (Value > U16_MAX)
        return U16_MAX;

    return (u16) Value;
}

INTERNAL u8
SafeTruncateU16(u16 Value)
{
    if (Value > U8_MAX)
        return U8_MAX;

    return (u8) Value;
}

INTERNAL i32
SafeTruncateI64(i64 Value)
{
    if (Value > I32_MAX)
        return I32_MAX;

    return (i32) Value;
}

INTERNAL i16
SafeTruncateI32(i32 Value)
{
    if (Value > I16_MAX)
        return I16_MAX;

    return (i16) Value;
}

INTERNAL i8
SafeTruncateI16(i16 Value)
{
    if (Value > I8_MAX)
        return I8_MAX;

    return (i8) Value;
}

// f32_________________________________________________________________________
API_INTERNAL f32
Cbrt(f32 Val)
{
    union {
        f32 F;
        u32 U;
    } V = { Val };

    u32 U_Val = V.U;
    u32 Sign = U_Val & 0x80000000;

    U_Val &= 0x7FFFFFFF;

    if (U_Val >= 0x7F800000)
        return Val + Val;

    if (U_Val < 0x00800000) {
        if (U_Val == 0)
            return Val;

        V.F = Val * 0x1.0p24f;
        U_Val = V.U & 0x7FFFFFFF;
        U_Val = U_Val / 3 + 0x2651067F;
    } else {
        U_Val = U_Val / 3 + 0x2A51067F;
    }

    V.U = Sign | U_Val;

    f32 T = V.F;
    f32 S = T * T;
    f32 R = Val / S;
    f32 W = T + T;

    R = (R - T) / (W + R);
    T = T + T * R;
    S = T * T;
    R = Val / S;
    W = T + T;
    R = (R - T) / (W + R);
    T = T + T * R;

    return T;
}

API_INTERNAL f32
FMod(f32 ValA, f32 ValB)
{
    f32 Div = ValA / ValB;
    f32 Trunc = RoundF32(Div);
    f32 Mul = Trunc * ValB;
    f32 Sub = ValA - Mul;

    return Sub;
}

API_INTERNAL f32 
Pow(f32 Val, f32 Exp)
{
    if (Exp == 1.0f)
        return Val;

    if (Exp == 0.0f)
        return 1.0f;

    f32 Result = 0.0f;

#if NC_SIMD_LEVEL >= 6 && !defined(NC_SLOW_FLOATS)
    Result = AVXPowF32(Val, Exp);
#elif NC_SIMD_LEVEL >= 2 && !defined(NC_SLOW_FLOATS)
    Result = SSEPowF32(Val, Exp);
#else
    Result = PowF32(Val, Exp);
#endif

    return Result;
}

API_INTERNAL f32 
Exp(f32 Val)
{
    f32 Result = 0.0f;

    if (Val == 1.0f)
        return (f32) NC_EULER;

    if (Val == 0.0f)
        return 1.0f;

#if NC_SIMD_LEVEL >= 6 && !defined(NC_SLOW_FLOATS)
    Result = AVXExpF32(Val);
#elif NC_SIMD_LEVEL >= 2 && !defined(NC_SLOW_FLOATS)
    Result = SSEExpF32(Val);
#else
    Result = ExpF32(Val);
#endif

    return Result;
}

API_INTERNAL f32 
Ln(f32 Val)
{
    f32 Result = 0.0f;

#if NC_SIMD_LEVEL >= 6 && !defined(NC_SLOW_FLOATS)
    Result = AVXLnF32(Val);
#elif NC_SIMD_LEVEL >= 2 && !defined(NC_SLOW_FLOATS)
    Result = SSELnF32(Val);
#else
    Result = LnF32(Val);
#endif

    return Result;
}

API_INTERNAL f32 
Log10(f32 Val)
{
    f32 Result = 0.0f;

#if NC_SIMD_LEVEL >= 6 && !defined(NC_SLOW_FLOATS)
    Result = AVXLog10F32(Val);
#elif NC_SIMD_LEVEL >= 2 && !defined(NC_SLOW_FLOATS)
    Result = SSELog10F32(Val);
#else
    Result = Log10F32(Val);
#endif

    return Result;
}

API_INTERNAL f32 
Log2(f32 Val)
{
    f32 Result = 0.0f;

#if NC_SIMD_LEVEL >= 6 && !defined(NC_SLOW_FLOATS)
    Result = AVXLog2F32(Val);
#elif NC_SIMD_LEVEL >= 2 && !defined(NC_SLOW_FLOATS)
    Result = SSELog2F32(Val);
#else
    Result = Log2F32(Val);
#endif

    return Result;
}

API_INTERNAL f32 
InvSqrt(f32 Val)
{
    f32 Result = 0.0f;

#if NC_SIMD_LEVEL >= 6 && !defined(NC_SLOW_FLOATS)
    Result = AVXInvSqrtF32(Val);
#elif NC_SIMD_LEVEL >= 2 && !defined(NC_SLOW_FLOATS)
    Result = SSEInvSqrtF32(Val);
#else
    Result = InvSqrtF32(Val);
#endif

    return Result;
}

API_INTERNAL f32 
Sin(f32 Val)
{
    f32 Result = 0.0f;

#if NC_SIMD_LEVEL >= 6 && !defined(NC_SLOW_FLOATS)
    Result = AVXSinF32(Val);
#elif NC_SIMD_LEVEL >= 2 && !defined(NC_SLOW_FLOATS)
    Result = SSESinF32(Val);
#else
    Result = SinF32(Val);
#endif

    return Result;
}

API_INTERNAL f32 
Cos(f32 Val)
{
    f32 Result = 0.0f;

#if NC_SIMD_LEVEL >= 6 && !defined(NC_SLOW_FLOATS)
    Result = AVXCosF32(Val);
#elif NC_SIMD_LEVEL >= 2 && !defined(NC_SLOW_FLOATS)
    Result = SSECosF32(Val);
#else
    Result = CosF32(Val);
#endif

    return Result;
}

API_INTERNAL f32 
Tan(f32 Val)
{
    f32 Result = 0.0f;

#if NC_SIMD_LEVEL >= 6 && !defined(NC_SLOW_FLOATS)
    Result = AVXTanF32(Val);
#elif NC_SIMD_LEVEL >= 2 && !defined(NC_SLOW_FLOATS)
    Result = SSETanF32(Val);
#else
    Result = TanF32(Val);
#endif

    return Result;
}

API_INTERNAL f32 
ArcSin(f32 Val)
{
    f32 Result = 0.0f;

#if NC_SIMD_LEVEL >= 6 && !defined(NC_SLOW_FLOATS)
    Result = AVXArcSinF32(Val);
#elif NC_SIMD_LEVEL >= 2 && !defined(NC_SLOW_FLOATS)
    Result = SSEArcSinF32(Val);
#else
    Result = ArcSinF32(Val);
#endif

    return Result;
}

API_INTERNAL f32 
ArcCos(f32 Val)
{
    f32 Result = 0.0f;

#if NC_SIMD_LEVEL >= 6 && !defined(NC_SLOW_FLOATS)
    Result = AVXArcCosF32(Val);
#elif NC_SIMD_LEVEL >= 2 && !defined(NC_SLOW_FLOATS)
    Result = SSEArcCosF32(Val);
#else
    Result = ArcCosF32(Val);
#endif

    return Result;
}

API_INTERNAL f32 
ArcTan(f32 Val)
{
    f32 Result = 0.0f;

#if NC_SIMD_LEVEL >= 6 && !defined(NC_SLOW_FLOATS)
    Result = AVXArcTanF32(Val);
#elif NC_SIMD_LEVEL >= 2 && !defined(NC_SLOW_FLOATS)
    Result = SSEArcTanF32(Val);
#else
    Result = ArcTanF32(Val);
#endif

    return Result;
}

API_INTERNAL f32 
ArcTan2(f32 ValA, f32 ValB)
{
    f32 Result = 0.0f;

#if NC_SIMD_LEVEL >= 6 && !defined(NC_SLOW_FLOATS)
    Result = AVXArcTan2F32(ValA, ValB);
#elif NC_SIMD_LEVEL >= 2 && !defined(NC_SLOW_FLOATS)
    Result = SSEArcTan2F32(ValA, ValB);
#else
    Result = ArcTan2F32(ValA, ValB);
#endif

    return Result;
}

// f64_________________________________________________________________________
API_INTERNAL f64
Cbrt(f64 Val)
{
    union {
        f64 F;
        u64 U;
    } V = {
        Val
    };

    f64 R;
    f64 S;
    f64 T;
    f64 W;
    u32 HX = V.U >> 32 & 0x7FFFFFFF;

    if (HX >= 0x7FF00000)
        return Val + Val;

    if (HX < 0x00100000) {
        V.F = Val * 0x1p54;
        HX = V.U >> 32 & 0x7FFFFFFF;

        if (!HX)
            return Val;

        HX = HX / 3 + 696219795;
    } else {
        HX = HX / 3 + 715094163;
    }

    V.U &= 1ULL << 63;
    V.U |= (u64) HX << 32;
    T = V.F;
    R = (T * T) * (T / Val);
    T = T * ((1.87595182427177009643 + R * (-1.88497979543377169875 + R * 1.621429720105354466140)) + ((R * R) * R) * (-0.758397934778766047437 + R * 0.145996192886612446982));
    V.F = T;
    V.U = (V.U + 0x80000000) & 0xFFFFFFFFC0000000ULL;
    T = V.F;
    S = T * T;
    R = Val / S;
    W = T + T;
    R = (R - T) / (W + R);
    T = T + T * R;

    return T;
}

API_INTERNAL f64
FMod(f64 ValA, f64 ValB)
{
    v128d VecA = _mm_set_sd(ValA);
    v128d VecB = _mm_set_sd(ValB);
    v128d Div = _mm_div_sd(VecA, VecB);
    v128d Trunc = _mm_round_sd(Div, Div, _MM_FROUND_TRUNC);
    v128d Mul = _mm_mul_sd(Trunc, VecB);
    v128d Sub = _mm_sub_sd(VecA, Mul);

    return _mm_cvtsd_f64(Sub);
}

API_INTERNAL f64
FModCycle(f64 X, f64 Y)
{
    if (Y == 0.0)
        return 0.0;

    f64 N = FloorF64(X / Y);

    return X - N * Y;
}

API_INTERNAL f64 
Pow(f64 Val, f64 Exp)
{
    if (Exp == 1.0)
        return Val;

    if (Exp == 0.0)
        return 1.0;

    f64 Result = 0.0;

#if NC_SIMD_LEVEL >= 6 && !defined(NC_SLOW_FLOATS)
    Result = AVXPowF64(Val, Exp);
#elif NC_SIMD_LEVEL >= 2 && !defined(NC_SLOW_FLOATS)
    Result = SSEPowF64(Val, Exp);
#else
    Result = PowF64(Val, Exp);
#endif

    return Result;
}

API_INTERNAL f64 
Exp(f64 Val)
{
    f64 Result = 0.0;

    if (Val == 1.0)
        return NC_EULER;

    if (Val == 0.0)
        return 1.0;

#if NC_SIMD_LEVEL >= 6 && !defined(NC_SLOW_FLOATS)
    Result = AVXExpF64(Val);
#elif NC_SIMD_LEVEL >= 2 && !defined(NC_SLOW_FLOATS)
    Result = SSEExpF64(Val);
#else
    Result = ExpF64(Val);
#endif

    return Result;
}

API_INTERNAL f64 
Ln(f64 Val)
{
    f64 Result = 0.0;

#if NC_SIMD_LEVEL >= 6 && !defined(NC_SLOW_FLOATS)
    Result = AVXLnF64(Val);
#elif NC_SIMD_LEVEL >= 2 && !defined(NC_SLOW_FLOATS)
    Result = SSELnF64(Val);
#else
    Result = LnF64(Val);
#endif

    return Result;
}

API_INTERNAL f64 
Log10(f64 Val)
{
    f64 Result = 0.0;

#if NC_SIMD_LEVEL >= 6 && !defined(NC_SLOW_FLOATS)
    Result = AVXLog10F64(Val);
#elif NC_SIMD_LEVEL >= 2 && !defined(NC_SLOW_FLOATS)
    Result = SSELog10F64(Val);
#else
    Result = Log10F64(Val);
#endif

    return Result;
}

API_INTERNAL f64 
Log2(f64 Val)
{
    f64 Result = 0.0;

#if NC_SIMD_LEVEL >= 6 && !defined(NC_SLOW_FLOATS)
    Result = AVXLog2F64(Val);
#elif NC_SIMD_LEVEL >= 2 && !defined(NC_SLOW_FLOATS)
    Result = SSELog2F64(Val);
#else
    Result = Log2F64(Val);
#endif

    return Result;
}

API_INTERNAL f64 
InvSqrt(f64 Val)
{
    f64 Result = 0.0;

#if NC_SIMD_LEVEL >= 6 && !defined(NC_SLOW_FLOATS)
    Result = AVXInvSqrtF64(Val);
#elif NC_SIMD_LEVEL >= 2 && !defined(NC_SLOW_FLOATS)
    Result = SSEInvSqrtF64(Val);
#else
    Result = InvSqrtF64(Val);
#endif

    return Result;
}

API_INTERNAL f64 
Sin(f64 Val)
{
    f64 Result = 0.0;

#if NC_SIMD_LEVEL >= 6 && !defined(NC_SLOW_FLOATS)
    Result = AVXSinF64(Val);
#elif NC_SIMD_LEVEL >= 2 && !defined(NC_SLOW_FLOATS)
    Result = SSESinF64(Val);
#else
    Result = SinF64(Val);
#endif

    return Result;
}

API_INTERNAL f64 
Cos(f64 Val)
{
    f64 Result = 0.0;

#if NC_SIMD_LEVEL >= 6 && !defined(NC_SLOW_FLOATS)
    Result = AVXCosF64(Val);
#elif NC_SIMD_LEVEL >= 2 && !defined(NC_SLOW_FLOATS)
    Result = SSECosF64(Val);
#else
    Result = CosF64(Val);
#endif

    return Result;
}

API_INTERNAL f64 
Tan(f64 Val)
{
    f64 Result = 0.0;

#if NC_SIMD_LEVEL >= 6 && !defined(NC_SLOW_FLOATS)
    Result = AVXTanF64(Val);
#elif NC_SIMD_LEVEL >= 2 && !defined(NC_SLOW_FLOATS)
    Result = SSETanF64(Val);
#else
    Result = TanF64(Val);
#endif

    return Result;
}

API_INTERNAL f64 
ArcSin(f64 Val)
{
    f64 Result = 0.0;

#if NC_SIMD_LEVEL >= 6 && !defined(NC_SLOW_FLOATS)
    Result = AVXArcSinF64(Val);
#elif NC_SIMD_LEVEL >= 2 && !defined(NC_SLOW_FLOATS)
    Result = SSEArcSinF64(Val);
#else
    Result = ArcSinF64(Val);
#endif

    return Result;
}

API_INTERNAL f64 
ArcCos(f64 Val)
{
    f64 Result = 0.0;

#if NC_SIMD_LEVEL >= 6 && !defined(NC_SLOW_FLOATS)
    Result = AVXArcCosF64(Val);
#elif NC_SIMD_LEVEL >= 2 && !defined(NC_SLOW_FLOATS)
    Result = SSEArcCosF64(Val);
#else
    Result = ArcCosF64(Val);
#endif

    return Result;
}

API_INTERNAL f64 
ArcTan(f64 Val)
{
    f64 Result = 0.0;

#if NC_SIMD_LEVEL >= 6 && !defined(NC_SLOW_FLOATS)
    Result = AVXArcTanF64(Val);
#elif NC_SIMD_LEVEL >= 2 && !defined(NC_SLOW_FLOATS)
    Result = SSEArcTanF64(Val);
#else
    Result = ArcTanF64(Val);
#endif

    return Result;
}

API_INTERNAL f64 
ArcTan2(f64 ValA, f64 ValB)
{
    f64 Result = 0.0;

#if NC_SIMD_LEVEL >= 6 && !defined(NC_SLOW_FLOATS)
    Result = AVXArcTan2F64(ValA, ValB);
#elif NC_SIMD_LEVEL >= 2 && !defined(NC_SLOW_FLOATS)
    Result = SSEArcTan2F64(ValA, ValB);
#else
    Result = ArcTan2F64(ValA, ValB);
#endif

    return Result;
}

// v2f32_______________________________________________________________________
API_INTERNAL v2f32
Vec(f32 X, f32 Y)
{
    v2f32 Result = {};

    Result.X = X;
    Result.Y = Y;

    return Result;
}

#if defined(NC_LANG_CPP)
API_INTERNAL v2f32
operator+(v2f32 VecA, v2f32 VecB)
{
    v2f32 Result = {};

    Result.X = VecA.X + VecB.X;
    Result.Y = VecA.Y + VecB.Y;

    return Result;
}

API_INTERNAL v2f32&
operator+=(v2f32& VecA, v2f32 VecB)
{
    VecA = VecA + VecB;

    return VecA;
}

API_INTERNAL v2f32
operator-(v2f32 VecA, v2f32 VecB)
{
    v2f32 Result = {};

    Result.X = VecA.X - VecB.X;
    Result.Y = VecA.Y - VecB.Y;

    return Result;
}

API_INTERNAL v2f32&
operator-=(v2f32& VecA, v2f32 VecB)
{
    VecA = VecA - VecB;

    return VecA;
}

API_INTERNAL v2f32
operator*(v2f32 VecA, v2f32 VecB)
{
    v2f32 Result = {};

    Result.X = VecA.X * VecB.X;
    Result.Y = VecA.Y * VecB.Y;

    return Result;
}

API_INTERNAL v2f32
operator*(v2f32 Vec, f32 Scale)
{
    v2f32 Result = {};

    Result.X = Vec.X * Scale;
    Result.Y = Vec.Y * Scale;

    return Result;
}

API_INTERNAL v2f32&
operator*=(v2f32& VecA, v2f32 VecB)
{
    VecA = VecA * VecB;

    return VecA;
}

API_INTERNAL v2f32&
operator*=(v2f32& VecA, f32 Scale)
{
    VecA = VecA * Scale;

    return VecA;
}

API_INTERNAL v2f32
operator/(v2f32 VecA, v2f32 VecB)
{
    v2f32 Result = {};

    Result.X = VecA.X / VecB.X;
    Result.Y = VecA.Y / VecB.Y;

    return Result;
}

API_INTERNAL v2f32&
operator/=(v2f32& VecA, v2f32 VecB)
{
    VecA = VecA / VecB;

    return VecA;
}
#endif

API_INTERNAL f32
Dot(v2f32 VecA, v2f32 VecB)
{
    return (VecA.X * VecB.X + VecA.Y * VecB.Y);
}

API_INTERNAL f32
LengthSq(v2f32 Vec)
{
    return Dot(Vec, Vec);
}

API_INTERNAL f32
Length(v2f32 Vec)
{
    return SqrtF32(LengthSq(Vec));
}

API_INTERNAL v2f32 
Norm(v2f32 Vec)
{
    return (Vec * (1.0f / Length(Vec)));
}

API_INTERNAL v2f32
Lerp(v2f32 VecA, v2f32 VecB, f32 T)
{
    v2f32 Result = {};

    Result.X = LerpF32(T, VecA.X, VecB.X);
    Result.Y = LerpF32(T, VecA.Y, VecB.Y);

    return Result;
}

// v2f64_______________________________________________________________________
API_INTERNAL v2f64
Vec(f64 X, f64 Y)
{
    v2f64 Result = {};

    Result.X = X;
    Result.Y = Y;

    return Result;
}

#if defined(NC_LANG_CPP)
API_INTERNAL v2f64
operator+(v2f64 VecA, v2f64 VecB)
{
    v2f64 Result = {};

    Result.X = VecA.X + VecB.X;
    Result.Y = VecA.Y + VecB.Y;

    return Result;
}

API_INTERNAL v2f64&
operator+=(v2f64& VecA, v2f64 VecB)
{
    VecA = VecA + VecB;

    return VecA;
}

API_INTERNAL v2f64
operator-(v2f64 VecA, v2f64 VecB)
{
    v2f64 Result = {};

    Result.X = VecA.X - VecB.X;
    Result.Y = VecA.Y - VecB.Y;

    return Result;
}

API_INTERNAL v2f64&
operator-=(v2f64& VecA, v2f64 VecB)
{
    VecA = VecA - VecB;

    return VecA;
}

API_INTERNAL v2f64
operator*(v2f64 VecA, v2f64 VecB)
{
    v2f64 Result = {};

    Result.X = VecA.X * VecB.X;
    Result.Y = VecA.Y * VecB.Y;

    return Result;
}

API_INTERNAL v2f64
operator*(v2f64 Vec, f64 Scale)
{
    v2f64 Result = {};

    Result.X = Vec.X * Scale;
    Result.Y = Vec.Y * Scale;

    return Result;
}

API_INTERNAL v2f64&
operator*=(v2f64& VecA, v2f64 VecB)
{
    VecA = VecA * VecB;

    return VecA;
}

API_INTERNAL v2f64&
operator*=(v2f64& VecA, f64 Scale)
{
    VecA = VecA * Scale;

    return VecA;
}

API_INTERNAL v2f64
operator/(v2f64 VecA, v2f64 VecB)
{
    v2f64 Result = {};

    Result.X = VecA.X / VecB.X;
    Result.Y = VecA.Y / VecB.Y;

    return Result;
}

API_INTERNAL v2f64&
operator/=(v2f64& VecA, v2f64 VecB)
{
    VecA = VecA / VecB;

    return VecA;
}
#endif

API_INTERNAL f64
Dot(v2f64 VecA, v2f64 VecB)
{
    return (VecA.X * VecB.X + VecA.Y * VecB.Y);
}

API_INTERNAL f64
LengthSq(v2f64 Vec)
{
    return Dot(Vec, Vec);
}

API_INTERNAL f64
Length(v2f64 Vec)
{
    return SqrtF64(LengthSq(Vec));
}

API_INTERNAL v2f64 
Norm(v2f64 Vec)
{
    return (Vec * (1.0f / Length(Vec)));
}

API_INTERNAL v2f64
Lerp(v2f64 VecA, v2f64 VecB, f64 T)
{
    v2f64 Result = {};

    Result.X = LerpF64(T, VecA.X, VecB.X);
    Result.Y = LerpF64(T, VecA.Y, VecB.Y);

    return Result;
}

// v2i64_______________________________________________________________________
API_INTERNAL v2i64
Vec(i64 X, i64 Y)
{
    v2i64 Result = {};

    Result.X = X;
    Result.Y = Y;

    return Result;
}

API_INTERNAL v2i64
operator+(v2i64 VecA, v2i64 VecB)
{
    v2i64 Result = {};

    Result.X = VecA.X + VecB.X;
    Result.Y = VecA.Y + VecB.Y;

    return Result;
}

API_INTERNAL v2i64
operator-(v2i64 VecA, v2i64 VecB)
{
    v2i64 Result = {};

    Result.X = VecA.X - VecB.X;
    Result.Y = VecA.Y - VecB.Y;

    return Result;
}

API_INTERNAL v2i64
operator*(v2i64 VecA, v2i64 VecB)
{
    v2i64 Result = {};

    Result.X = VecA.X * VecB.X;
    Result.Y = VecA.Y * VecB.Y;

    return Result;
}

API_INTERNAL v2i64
operator*(v2i64 Vec, i64 Scale)
{
    v2i64 Result = {};

    Result.X = Vec.X * Scale;
    Result.Y = Vec.Y * Scale;

    return Result;
}

API_INTERNAL v2i64
operator/(v2i64 VecA, v2i64 VecB)
{
    v2i64 Result = {};

    Result.X = VecA.X / VecB.X;
    Result.Y = VecA.Y / VecB.Y;

    return Result;
}

API_INTERNAL i64
Dot(v2i64 VecA, v2i64 VecB)
{
    return (VecA.X * VecB.X + VecA.Y * VecB.Y);
}

API_INTERNAL i64
LengthSq(v2i64 Vec)
{
    return Dot(Vec, Vec);
}

API_INTERNAL i64
Length(v2i64 Vec)
{
    return (i64) SqrtF64((f64) LengthSq(Vec));
}

API_INTERNAL v2i64 
Norm(v2i64 Vec)
{
    return (Vec * (i64) (1.0f / Length(Vec)));
}

API_INTERNAL v2i64
Lerp(v2i64 VecA, v2i64 VecB, f32 T)
{
    v2i64 Result = {};

    LerpV2I64(&Result, &VecA, &VecB, T);

    return Result;
}

// v2i32_______________________________________________________________________
API_INTERNAL v2i32
Vec(i32 X, i32 Y)
{
    v2i32 Result = {};

    Result.X = X;
    Result.Y = Y;

    return Result;
}

API_INTERNAL v2i32
operator+(v2i32 VecA, v2i32 VecB)
{
    v2i32 Result = {};

    Result.X = VecA.X + VecB.X;
    Result.Y = VecA.Y + VecB.Y;

    return Result;
}

API_INTERNAL v2i32
operator-(v2i32 VecA, v2i32 VecB)
{
    v2i32 Result = {};

    Result.X = VecA.X - VecB.X;
    Result.Y = VecA.Y - VecB.Y;

    return Result;
}

API_INTERNAL v2i32
operator*(v2i32 VecA, v2i32 VecB)
{
    v2i32 Result = {};

    Result.X = VecA.X * VecB.X;
    Result.Y = VecA.Y * VecB.Y;

    return Result;
}

API_INTERNAL v2i32
operator*(v2i32 Vec, i32 Scale)
{
    v2i32 Result = {};

    Result.X = Vec.X * Scale;
    Result.Y = Vec.Y * Scale;

    return Result;
}

API_INTERNAL v2i32
operator/(v2i32 VecA, v2i32 VecB)
{
    v2i32 Result = {};

    Result.X = VecA.X / VecB.X;
    Result.Y = VecA.Y / VecB.Y;

    return Result;
}

API_INTERNAL i32
Dot(v2i32 VecA, v2i32 VecB)
{
    return (VecA.X * VecB.X + VecA.Y * VecB.Y);
}

API_INTERNAL i32
LengthSq(v2i32 Vec)
{
    return Dot(Vec, Vec);
}

API_INTERNAL i32
Length(v2i32 Vec)
{
    return (i32) SqrtF32((f32) LengthSq(Vec));
}

API_INTERNAL v2i32 
Norm(v2i32 Vec)
{
    return (Vec * (i32) (1.0f / Length(Vec)));
}

API_INTERNAL v2i32
Lerp(v2i32 VecA, v2i32 VecB, f32 T)
{
    v2i32 Result = LerpV2I32(VecA, VecB, T);

    return Result;
}

// v2i16_______________________________________________________________________
API_INTERNAL v2i16
Vec(i16 X, i16 Y)
{
    v2i16 Result = {};

    Result.X = X;
    Result.Y = Y;

    return Result;
}

API_INTERNAL v2i16
operator+(v2i16 VecA, v2i16 VecB)
{
    v2i16 Result = {};

    Result.X = VecA.X + VecB.X;
    Result.Y = VecA.Y + VecB.Y;

    return Result;
}

API_INTERNAL v2i16&
operator+=(v2i16& VecA, v2i16 VecB)
{
    VecA = VecA + VecB;

    return VecA;
}

API_INTERNAL v2i16
operator-(v2i16 VecA, v2i16 VecB)
{
    v2i16 Result = {};

    Result.X = VecA.X - VecB.X;
    Result.Y = VecA.Y - VecB.Y;

    return Result;
}
API_INTERNAL v2i16&
operator-=(v2i16& VecA, v2i16 VecB)
{
    VecA = VecA - VecB;

    return VecA;
}

API_INTERNAL v2i16
operator*(v2i16 VecA, v2i16 VecB)
{
    v2i16 Result = {};

    Result.X = VecA.X * VecB.X;
    Result.Y = VecA.Y * VecB.Y;

    return Result;
}

API_INTERNAL v2i16
operator*(v2i16 Vec, i16 Scale)
{
    v2i16 Result = {};

    Result.X = Vec.X * Scale;
    Result.Y = Vec.Y * Scale;

    return Result;
}

API_INTERNAL v2i16&
operator*=(v2i16& VecA, v2i16 VecB)
{
    VecA = VecA * VecB;

    return VecA;
}

API_INTERNAL v2i16&
operator*=(v2i16& VecA, i16 Scale)
{
    VecA = VecA * Scale;

    return VecA;
}

API_INTERNAL v2i16
operator/(v2i16 VecA, v2i16 VecB)
{
    v2i16 Result = {};

    Result.X = VecA.X / VecB.X;
    Result.Y = VecA.Y / VecB.Y;

    return Result;
}

API_INTERNAL v2i16&
operator/=(v2i16& VecA, v2i16 VecB)
{
    VecA = VecA / VecB;

    return VecA;
}

API_INTERNAL i16
Dot(v2i16 VecA, v2i16 VecB)
{
    return (VecA.X * VecB.X + VecA.Y * VecB.Y);
}

API_INTERNAL i16
LengthSq(v2i16 Vec)
{
    return Dot(Vec, Vec);
}

API_INTERNAL i16
Length(v2i16 Vec)
{
    return (i16) SqrtF32((f32) LengthSq(Vec));
}

API_INTERNAL v2i16 
Norm(v2i16 Vec)
{
    return (Vec * (i16) (1.0f / Length(Vec)));
}

API_INTERNAL v2i16
Lerp(v2i16 VecA, v2i16 VecB, f32 T)
{
    v2i16 Result = LerpV2I16(VecA, VecB, T);

    return Result;
}

// v3f32_______________________________________________________________________
API_INTERNAL v3f32
Vec(f32 X, f32 Y, f32 Z)
{
    v3f32 Result = {};

    Result.X = X;
    Result.Y = Y;
    Result.Z = Z;

    return Result;
}

API_INTERNAL v3f32
operator+(v3f32 VecA, v3f32 VecB)
{
    v3f32 Result = {};

    Result.X = VecA.X + VecB.X;
    Result.Y = VecA.Y + VecB.Y;
    Result.Z = VecA.Z + VecB.Z;

    return Result;
}

API_INTERNAL v3f32
operator-(v3f32 VecA, v3f32 VecB)
{
    v3f32 Result = {};

    Result.X = VecA.X - VecB.X;
    Result.Y = VecA.Y - VecB.Y;
    Result.Z = VecA.Z - VecB.Z;

    return Result;
}

API_INTERNAL v3f32
operator*(v3f32 VecA, v3f32 VecB)
{
    v3f32 Result = {};

    Result.X = VecA.X * VecB.X;
    Result.Y = VecA.Y * VecB.Y;
    Result.Z = VecA.Z * VecB.Z;

    return Result;
}

API_INTERNAL v3f32
operator*(v3f32 Vec, f32 Scale)
{
    v3f32 Result = {};

    Result.X = Vec.X * Scale;
    Result.Y = Vec.Y * Scale;
    Result.Z = Vec.Z * Scale;

    return Result;
}

API_INTERNAL v3f32
operator/(v3f32 VecA, v3f32 VecB)
{
    v3f32 Result = {};

    Result.X = VecA.X / VecB.X;
    Result.Y = VecA.Y / VecB.Y;
    Result.Z = VecA.Z / VecB.Z;

    return Result;
}


API_INTERNAL b32
operator<(v3f32 VecA, v3f32 VecB)
{
    return (
        VecA.X < VecB.X &&
        VecA.Y < VecB.Y &&
        VecA.Z < VecB.Z
    );
}

API_INTERNAL b32
operator>(v3f32 VecA, v3f32 VecB)
{

    return (
        VecA.X > VecB.X &&
        VecA.Y > VecB.Y &&
        VecA.Z > VecB.Z
    );
}


API_INTERNAL v3f32& 
operator+=(v3f32& VecA, v3f32 VecB)
{
    VecA = VecA + VecB;

    return VecA;
}

API_INTERNAL f32
Dot(v3f32 VecA, v3f32 VecB)
{
    return (VecA.X * VecB.X + VecA.Y * VecB.Y + VecA.Z * VecB.Z);
}

API_INTERNAL f32
LengthSq(v3f32 Vec)
{
    return Dot(Vec, Vec);
}

API_INTERNAL f32
Length(v3f32 Vec)
{
    return SqrtF32(LengthSq(Vec));
}

API_INTERNAL v3f32
Norm(v3f32 Vec)
{
    return (Vec * (1.0f / Length(Vec)));
}

API_INTERNAL v3f32
Lerp(v3f32 VecA, v3f32 VecB, f32 T)
{
    v3f32 Result = {};

    Result.X = LerpF32(T, VecA.X, VecB.X);
    Result.Y = LerpF32(T, VecA.Y, VecB.Y);
    Result.Z = LerpF32(T, VecA.Z, VecB.Z);

    return Result;
}

API_INTERNAL v3f32
Cross(v3f32 VecA, v3f32 VecB)
{
    v3f32 Result = {};

    Result.X = VecA.Y * VecB.Z - VecA.Z * VecB.Y;
    Result.Y = VecA.Z * VecB.X - VecA.X * VecB.Z;
    Result.Z = VecA.X * VecB.Y - VecA.Y * VecB.X;

    return Result;
}

// v3f64_______________________________________________________________________
API_INTERNAL v3f64
Vec(f64 X, f64 Y, f64 Z)
{
    v3f64 Result = {};

    Result.X = X;
    Result.Y = Y;
    Result.Z = Z;

    return Result;
}

API_INTERNAL v3f64
operator+(v3f64 VecA, v3f64 VecB)
{
    v3f64 Result = {};

    Result.X = VecA.X + VecB.X;
    Result.Y = VecA.Y + VecB.Y;
    Result.Z = VecA.Z + VecB.Z;

    return Result;
}

API_INTERNAL v3f64
operator-(v3f64 VecA, v3f64 VecB)
{
    v3f64 Result = {};

    Result.X = VecA.X - VecB.X;
    Result.Y = VecA.Y - VecB.Y;
    Result.Z = VecA.Z - VecB.Z;

    return Result;
}

API_INTERNAL v3f64
operator*(v3f64 VecA, v3f64 VecB)
{
    v3f64 Result = {};

    Result.X = VecA.X * VecB.X;
    Result.Y = VecA.Y * VecB.Y;
    Result.Z = VecA.Z * VecB.Z;

    return Result;
}

API_INTERNAL v3f64
operator*(v3f64 Vec, f64 Scale)
{
    v3f64 Result = {};

    Result.X = Vec.X * Scale;
    Result.Y = Vec.Y * Scale;
    Result.Z = Vec.Z * Scale;

    return Result;
}

API_INTERNAL v3f64
operator/(v3f64 VecA, v3f64 VecB)
{
    v3f64 Result = {};

    Result.X = VecA.X / VecB.X;
    Result.Y = VecA.Y / VecB.Y;
    Result.Z = VecA.Z / VecB.Z;

    return Result;
}

API_INTERNAL f64
Dot(v3f64 VecA, v3f64 VecB)
{
    return (VecA.X * VecB.X + VecA.Y * VecB.Y + VecA.Z * VecB.Z);
}

API_INTERNAL f64
LengthSq(v3f64 Vec)
{
    return Dot(Vec, Vec);
}

API_INTERNAL f64
Length(v3f64 Vec)
{
    return SqrtF64(LengthSq(Vec));
}

API_INTERNAL v3f64
Norm(v3f64 Vec)
{
    return (Vec * (1.0f / Length(Vec)));
}

API_INTERNAL v3f64
Lerp(v3f64 VecA, v3f64 VecB, f64 T)
{
    v3f64 Result = {};

    Result.X = LerpF64(T, VecA.X, VecB.X);
    Result.Y = LerpF64(T, VecA.Y, VecB.Y);
    Result.Z = LerpF64(T, VecA.Z, VecB.Z);

    return Result;
}

API_INTERNAL v3f64
Cross(v3f64 VecA, v3f64 VecB)
{
    v3f64 Result = {};

    Result.X = VecA.Y * VecB.Z - VecA.Z * VecB.Y;
    Result.Y = VecA.Z * VecB.X - VecA.X * VecB.Z;
    Result.Z = VecA.X * VecB.Y - VecA.Y * VecB.X;

    return Result;
}

// v3i32_______________________________________________________________________
API_INTERNAL v3i32
Vec(i32 X, i32 Y, i32 Z)
{
    v3i32 Result = {};

    Result.X = X;
    Result.Y = Y;
    Result.Z = Z;

    return Result;
}

API_INTERNAL v3i32
operator+(v3i32 VecA, v3i32 VecB)
{
    v3i32 Result = {};

    Result.X = VecA.X + VecB.X;
    Result.Y = VecA.Y + VecB.Y;
    Result.Z = VecA.Z + VecB.Z;

    return Result;
}

API_INTERNAL v3i32
operator-(v3i32 VecA, v3i32 VecB)
{
    v3i32 Result = {};

    Result.X = VecA.X - VecB.X;
    Result.Y = VecA.Y - VecB.Y;
    Result.Z = VecA.Z - VecB.Z;

    return Result;
}

API_INTERNAL v3i32
operator*(v3i32 VecA, v3i32 VecB)
{
    v3i32 Result = {};

    Result.X = VecA.X * VecB.X;
    Result.Y = VecA.Y * VecB.Y;
    Result.Z = VecA.Z * VecB.Z;

    return Result;
}

API_INTERNAL v3i32
operator*(v3i32 Vec, i32 Scale)
{
    v3i32 Result = {};

    Result.X = Vec.X * Scale;
    Result.Y = Vec.Y * Scale;
    Result.Z = Vec.Z * Scale;

    return Result;
}

API_INTERNAL v3i32
operator/(v3i32 VecA, v3i32 VecB)
{
    v3i32 Result = {};

    Result.X = VecA.X / VecB.X;
    Result.Y = VecA.Y / VecB.Y;
    Result.Z = VecA.Z / VecB.Z;

    return Result;
}

API_INTERNAL i32
Dot(v3i32 VecA, v3i32 VecB)
{
    return (VecA.X * VecB.X + VecA.Y * VecB.Y + VecA.Z * VecB.Z);
}

API_INTERNAL i32
LengthSq(v3i32 Vec)
{
    return Dot(Vec, Vec);
}

API_INTERNAL i32
Length(v3i32 Vec)
{
    return (i32) SqrtF32((f32) LengthSq(Vec));
}

API_INTERNAL v3i32
Norm(v3i32 Vec)
{
    return (Vec * (1.0f / Length(Vec)));
}

API_INTERNAL v3i32
Lerp(v3i32 VecA, v3i32 VecB, f32 T)
{
    v4i32 ResultWide = {};
    v4i32 VecAWide = Vec(VecA.X, VecA.Y, VecA.Z, 0);
    v4i32 VecBWide = Vec(VecB.X, VecB.Y, VecB.Z, 0);

    LerpV4I32(&ResultWide, &VecAWide, &VecBWide, T);

    v3i32 Result = Vec(ResultWide.X, ResultWide.Y, ResultWide.Z);

    return Result;
}

API_INTERNAL v3i32
Cross(v3i32 VecA, v3i32 VecB)
{
    v3i32 Result = {};

    Result.X = VecA.Y * VecB.Z - VecA.Z * VecB.Y;
    Result.Y = VecA.Z * VecB.X - VecA.X * VecB.Z;
    Result.Z = VecA.X * VecB.Y - VecA.Y * VecB.X;

    return Result;
}

// v4f32_______________________________________________________________________
API_INTERNAL v4f32
Vec(f32 X, f32 Y, f32 Z, f32 W)
{
    v4f32 Result = {};

    Result.X = X;
    Result.Y = Y;
    Result.Z = Z;
    Result.W = W;

    return Result;
}

API_INTERNAL v4f32
operator+(v4f32 VecA, v4f32 VecB)
{
    v4f32 Result = {};

    Result.X = VecA.X + VecB.X;
    Result.Y = VecA.Y + VecB.Y;
    Result.Z = VecA.Z + VecB.Z;
    Result.W = VecA.W + VecB.W;

    return Result;
}

API_INTERNAL v4f32
operator-(v4f32 VecA, v4f32 VecB)
{
    v4f32 Result = {};

    Result.X = VecA.X - VecB.X;
    Result.Y = VecA.Y - VecB.Y;
    Result.Z = VecA.Z - VecB.Z;
    Result.W = VecA.W - VecB.W;

    return Result;
}

API_INTERNAL v4f32
operator*(v4f32 VecA, v4f32 VecB)
{
    v4f32 Result = {};

    Result.X = VecA.X * VecB.X;
    Result.Y = VecA.Y * VecB.Y;
    Result.Z = VecA.Z * VecB.Z;
    Result.W = VecA.W * VecB.W;

    return Result;
}

API_INTERNAL v4f32
operator*(v4f32 Vec, f32 Scale)
{
    v4f32 Result = {};

    Result.X = Vec.X * Scale;
    Result.Y = Vec.Y * Scale;
    Result.Z = Vec.Z * Scale;
    Result.W = Vec.W * Scale;

    return Result;
}

API_INTERNAL v4f32
operator/(v4f32 VecA, v4f32 VecB)
{
    v4f32 Result = {};

    Result.X = VecA.X / VecB.X;
    Result.Y = VecA.Y / VecB.Y;
    Result.Z = VecA.Z / VecB.Z;
    Result.W = VecA.W / VecB.W;

    return Result;
}

API_INTERNAL v4f32
operator/(v4f32 Vec, f32 Value)
{
    v4f32 Result = {};

    Result.X = Vec.X / Value;
    Result.Y = Vec.Y / Value;
    Result.Z = Vec.Z / Value;
    Result.W = Vec.W / Value;

    return Result;
}

API_INTERNAL f32
Dot(v4f32 VecA, v4f32 VecB)
{
    return (VecA.X * VecB.X + VecA.Y * VecB.Y + VecA.Z * VecB.Z + VecA.W * VecB.W);
}

API_INTERNAL f32
LengthSq(v4f32 Vec)
{
    return Dot(Vec, Vec);
}

API_INTERNAL f32
Length(v4f32 Vec)
{
    return SqrtF32(LengthSq(Vec));
}

API_INTERNAL v4f32
Norm(v4f32 Vec)
{
    return (Vec * (1.0f / Length(Vec)));
}

API_INTERNAL v4f32
Lerp(v4f32 VecA, v4f32 VecB, f32 T)
{
    v4f32 Result = {};

    Result.X = LerpF32(T, VecA.X, VecB.X);
    Result.Y = LerpF32(T, VecA.Y, VecB.Y);
    Result.Z = LerpF32(T, VecA.Z, VecB.Z);
    Result.W = LerpF32(T, VecA.W, VecB.W);

    return Result;
}

// v4f64_______________________________________________________________________
API_INTERNAL v4f64
Vec(f64 X, f64 Y, f64 Z, f64 W)
{
    v4f64 Result = {};

    Result.X = X;
    Result.Y = Y;
    Result.Z = Z;
    Result.W = W;

    return Result;
}

API_INTERNAL v4f64
operator+(v4f64 VecA, v4f64 VecB)
{
    v4f64 Result = {};

    Result.X = VecA.X + VecB.X;
    Result.Y = VecA.Y + VecB.Y;
    Result.Z = VecA.Z + VecB.Z;
    Result.W = VecA.W + VecB.W;

    return Result;
}

API_INTERNAL v4f64
operator-(v4f64 VecA, v4f64 VecB)
{
    v4f64 Result = {};

    Result.X = VecA.X - VecB.X;
    Result.Y = VecA.Y - VecB.Y;
    Result.Z = VecA.Z - VecB.Z;
    Result.W = VecA.W - VecB.W;

    return Result;
}

API_INTERNAL v4f64
operator*(v4f64 VecA, v4f64 VecB)
{
    v4f64 Result = {};

    Result.X = VecA.X * VecB.X;
    Result.Y = VecA.Y * VecB.Y;
    Result.Z = VecA.Z * VecB.Z;
    Result.W = VecA.W * VecB.W;

    return Result;
}

API_INTERNAL v4f64
operator*(v4f64 Vec, f64 Scale)
{
    v4f64 Result = {};

    Result.X = Vec.X * Scale;
    Result.Y = Vec.Y * Scale;
    Result.Z = Vec.Z * Scale;
    Result.W = Vec.W * Scale;

    return Result;
}

API_INTERNAL v4f64
operator/(v4f64 VecA, v4f64 VecB)
{
    v4f64 Result = {};

    Result.X = VecA.X / VecB.X;
    Result.Y = VecA.Y / VecB.Y;
    Result.Z = VecA.Z / VecB.Z;
    Result.W = VecA.W / VecB.W;

    return Result;
}

API_INTERNAL f64
Dot(v4f64 VecA, v4f64 VecB)
{
    return (VecA.X * VecB.X + VecA.Y * VecB.Y + VecA.Z * VecB.Z + VecA.W * VecB.W);
}

API_INTERNAL f64
LengthSq(v4f64 Vec)
{
    return Dot(Vec, Vec);
}

API_INTERNAL f64
Length(v4f64 Vec)
{
    return SqrtF64(LengthSq(Vec));
}

API_INTERNAL v4f64
Norm(v4f64 Vec)
{
    return (Vec * (1.0f / Length(Vec)));
}

API_INTERNAL v4f64
Lerp(v4f64 VecA, v4f64 VecB, f64 T)
{
    v4f64 Result = {};

    Result.X = LerpF64(T, VecA.X, VecB.X);
    Result.Y = LerpF64(T, VecA.Y, VecB.Y);
    Result.Z = LerpF64(T, VecA.Z, VecB.Z);
    Result.W = LerpF64(T, VecA.W, VecB.W);

    return Result;
}

// v4i32_______________________________________________________________________
API_INTERNAL v4i32
Vec(i32 X, i32 Y, i32 Z, i32 W)
{
    v4i32 Result = {};

    Result.X = X;
    Result.Y = Y;
    Result.Z = Z;
    Result.W = W;

    return Result;
}

API_INTERNAL v4i32
operator+(v4i32 VecA, v4i32 VecB)
{
    v4i32 Result = {};

    Result.X = VecA.X + VecB.X;
    Result.Y = VecA.Y + VecB.Y;
    Result.Z = VecA.Z + VecB.Z;
    Result.W = VecA.W + VecB.W;

    return Result;
}

API_INTERNAL v4i32
operator-(v4i32 VecA, v4i32 VecB)
{
    v4i32 Result = {};

    Result.X = VecA.X - VecB.X;
    Result.Y = VecA.Y - VecB.Y;
    Result.Z = VecA.Z - VecB.Z;
    Result.W = VecA.W - VecB.W;

    return Result;
}

API_INTERNAL v4i32
operator*(v4i32 VecA, v4i32 VecB)
{
    v4i32 Result = {};

    Result.X = VecA.X * VecB.X;
    Result.Y = VecA.Y * VecB.Y;
    Result.Z = VecA.Z * VecB.Z;
    Result.W = VecA.W * VecB.W;

    return Result;
}

API_INTERNAL v4i32
operator*(v4i32 Vec, i32 Scale)
{
    v4i32 Result = {};

    Result.X = Vec.X * Scale;
    Result.Y = Vec.Y * Scale;
    Result.Z = Vec.Z * Scale;
    Result.W = Vec.W * Scale;

    return Result;
}

API_INTERNAL v4i32
operator/(v4i32 VecA, v4i32 VecB)
{
    v4i32 Result = {};

    Result.X = VecA.X / VecB.X;
    Result.Y = VecA.Y / VecB.Y;
    Result.Z = VecA.Z / VecB.Z;
    Result.W = VecA.W / VecB.W;

    return Result;
}

API_INTERNAL i32
Dot(v4i32 VecA, v4i32 VecB)
{
    return (VecA.X * VecB.X + VecA.Y * VecB.Y + VecA.Z * VecB.Z + VecA.W * VecB.W);
}

API_INTERNAL i32
LengthSq(v4i32 Vec)
{
    return Dot(Vec, Vec);
}

API_INTERNAL i32
Length(v4i32 Vec)
{
    return (i32) SqrtF32((f32) LengthSq(Vec));
}

API_INTERNAL v4i32
Norm(v4i32 Vec)
{
    return (Vec * (1.0f / Length(Vec)));
}

API_INTERNAL v4i32
Lerp(v4i32 VecA, v4i32 VecB, f32 T)
{
    v4i32 Result = {};

    LerpV4I32(&Result, &VecA, &VecB, T);

    return Result;
}

// v4u32_______________________________________________________________________
API_INTERNAL v4u32
Vec(u32 X, u32 Y, u32 Z, u32 W)
{
    v4u32 Result = {};

    Result.X = X;
    Result.Y = Y;
    Result.Z = Z;
    Result.W = W;

    return Result;
}

API_INTERNAL v4u32
operator+(v4u32 VecA, v4u32 VecB)
{
    v4u32 Result = {};

    Result.X = VecA.X + VecB.X;
    Result.Y = VecA.Y + VecB.Y;
    Result.Z = VecA.Z + VecB.Z;
    Result.W = VecA.W + VecB.W;

    return Result;
}

API_INTERNAL v4u32
operator-(v4u32 VecA, v4u32 VecB)
{
    v4u32 Result = {};

    Result.X = VecA.X - VecB.X;
    Result.Y = VecA.Y - VecB.Y;
    Result.Z = VecA.Z - VecB.Z;
    Result.W = VecA.W - VecB.W;

    return Result;
}

API_INTERNAL v4u32
operator*(v4u32 VecA, v4u32 VecB)
{
    v4u32 Result = {};

    Result.X = VecA.X * VecB.X;
    Result.Y = VecA.Y * VecB.Y;
    Result.Z = VecA.Z * VecB.Z;
    Result.W = VecA.W * VecB.W;

    return Result;
}

API_INTERNAL v4u32
operator*(v4u32 Vec, u32 Scale)
{
    v4u32 Result = {};

    Result.X = Vec.X * Scale;
    Result.Y = Vec.Y * Scale;
    Result.Z = Vec.Z * Scale;
    Result.W = Vec.W * Scale;

    return Result;
}

API_INTERNAL v4u32
operator/(v4u32 VecA, v4u32 VecB)
{
    v4u32 Result = {};

    Result.X = VecA.X / VecB.X;
    Result.Y = VecA.Y / VecB.Y;
    Result.Z = VecA.Z / VecB.Z;
    Result.W = VecA.W / VecB.W;

    return Result;
}

API_INTERNAL u32
Dot(v4u32 VecA, v4u32 VecB)
{
    return (VecA.X * VecB.X + VecA.Y * VecB.Y + VecA.Z * VecB.Z + VecA.W * VecB.W);
}

API_INTERNAL u32
LengthSq(v4u32 Vec)
{
    return Dot(Vec, Vec);
}

API_INTERNAL u32
Length(v4u32 Vec)
{
    return (u32) SqrtF32((f32) LengthSq(Vec));
}

API_INTERNAL v4u32
Norm(v4u32 Vec)
{
    return (Vec * (1.0f / Length(Vec)));
}

API_INTERNAL v4u32
Lerp(v4u32 VecA, v4u32 VecB, f32 T)
{
    v4u32 Result = {};

    LerpV4I32(                                                                      // FIXME: This assumes that all in VecA and VecB are < 2^31
        (v4i32*) &Result,
        (v4i32*) &VecA,
        (v4i32*) &VecB,
        T
    );

    return Result;
}

// m3f32_______________________________________________________________________
API_INTERNAL m3f32
Mat3(f32 Diag)
{
    m3f32 Result = {};

    Result.V[0][0] = Diag;
    Result.V[1][1] = Diag;
    Result.V[2][2] = Diag;

    return Result;
}

API_INTERNAL m3f32
Translation(v2f32 Vec)
{
    m3f32 Result = Mat3(1.0f);

    Result.V[2][0] = Vec.X;
    Result.V[2][1] = Vec.Y;

    return Result;
}

API_INTERNAL m3f32
Scaler(v2f32 Vec)
{
    m3f32 Result = Mat3(1.0f);

    Result.V[0][0] = Vec.X;
    Result.V[1][1] = Vec.Y;

    return Result;
}

API_INTERNAL m3f32
operator*(m3f32 MatA, m3f32 MatB)
{
    m3f32 Result = {};

    for (u8 J = 0; J < 3; ++J)
        for (u8 I = 0; I < 3; ++I)
            Result.V[I][J] = (MatA.V[0][J] * MatB.V[I][0] + MatA.V[1][J] * MatB.V[I][1] + MatA.V[2][J] * MatB.V[I][2]);

    return Result;
}

// m3f64_______________________________________________________________________
API_INTERNAL m3f64
Mat3(f64 Diag)
{
    m3f64 Result = {};

    Result.V[0][0] = Diag;
    Result.V[1][1] = Diag;
    Result.V[2][2] = Diag;

    return Result;
}

API_INTERNAL m3f64
Translation(v2f64 Vec)
{
    m3f64 Result = Mat3(1.0);

    Result.V[2][0] = Vec.X;
    Result.V[2][1] = Vec.Y;

    return Result;
}

API_INTERNAL m3f64
Scaler(v2f64 Vec)
{
    m3f64 Result = Mat3(1.0);

    Result.V[0][0] = Vec.X;
    Result.V[1][1] = Vec.Y;

    return Result;
}

API_INTERNAL m3f64
operator*(m3f64 MatA, m3f64 MatB)
{
    m3f64 Result = {};

    for (u8 J = 0; J < 3; ++J)
        for (u8 I = 0; I < 3; ++I)
            Result.V[I][J] = (MatA.V[0][J] * MatB.V[I][0] + MatA.V[1][J] * MatB.V[I][1] + MatA.V[2][J] * MatB.V[I][2]);

    return Result;
}

// m4f32_______________________________________________________________________
API_INTERNAL m4f32
Mat4(f32 Diag)
{
    m4f32 Result = {};

    Result.V[0][0] = Diag;
    Result.V[1][1] = Diag;
    Result.V[2][2] = Diag;
    Result.V[3][3] = Diag;

    return Result;
}

API_INTERNAL m4f32
Translation(v3f32 Vec)
{
    m4f32 Result = Mat4(1.0f);

    Result.V[3][0] = Vec.X;
    Result.V[3][1] = Vec.Y;
    Result.V[3][2] = Vec.Z;

    return Result;
}

API_INTERNAL m4f32
Scaler(v3f32 Vec)
{
    m4f32 Result = Mat4(1.0f);

    Result.V[0][0] = Vec.X;
    Result.V[1][1] = Vec.Y;
    Result.V[2][2] = Vec.Z;

    return Result;
}

API_INTERNAL m4f32
Perspective(f32 FOV, f32 AspectRatio, f32 NearZ, f32 FarZ)
{
    m4f32 Result = Mat4(1.0f);
    f32 TanHalfFOV = Tan(FOV / 2.0f);

    Result.V[0][0] = 1.0f / TanHalfFOV;
    Result.V[1][1] = AspectRatio / TanHalfFOV;
    Result.V[2][3] = 1.0f;
    Result.V[2][2] = -(NearZ + FarZ) / (NearZ - FarZ);
    Result.V[3][2] = (2.0f * NearZ * FarZ) / (NearZ - FarZ);
    Result.V[3][3] = 0.0f;

    return Result;
}

API_INTERNAL m4f32
Orthographic(f32 Left, f32 Right, f32 Bottom, f32 Top, f32 NearZ, f32 FarZ)
{
    m4f32 Result = Mat4(1.0f);

    Result.V[0][0] = 2.0f / (Right - Left);
    Result.V[1][1] = 2.0f / (Top - Bottom);
    Result.V[2][2] = 2.0f / (FarZ - NearZ);
    Result.V[3][3] = 1.0f;
    Result.V[3][0] = (Left + Right) / (Left - Right);
    Result.V[3][1] = (Bottom + Top) / (Bottom - Top);
    Result.V[3][2] = (NearZ + FarZ) / (NearZ - FarZ);

    return Result;
}

API_INTERNAL m4f32
LookAt(v3f32 Eye, v3f32 Centre, v3f32 Up)
{
    m4f32 Result = {};
    v3f32 F = Norm(Eye - Centre);
    v3f32 S = Norm(Cross(F, Up));
    v3f32 U = Cross(S, F);

    Result.V[0][0] = S.X;
    Result.V[0][1] = U.X;
    Result.V[0][2] = -F.X;
    Result.V[0][3] = 0.0f;
    Result.V[1][0] = S.Y;
    Result.V[1][1] = U.Y;
    Result.V[1][2] = -F.Y;
    Result.V[1][3] = 0.0f;
    Result.V[2][0] = S.Z;
    Result.V[2][1] = U.Z;
    Result.V[2][2] = -F.Z;
    Result.V[2][3] = 0.0f;
    Result.V[3][0] = -Dot(S, Eye);
    Result.V[3][1] = -Dot(U, Eye);
    Result.V[3][2] = Dot(F, Eye);
    Result.V[3][3] = 1.0f;

    return Result;
}

API_INTERNAL m4f32
Rotation(v3f32 Axis, f32 Turns)
{
    m4f32 Result = Mat4(1.0f);

    Axis = Norm(Axis);

    f32 SinTheta = Sin(Turns);
    f32 CosTheta = Cos(Turns);
    f32 CosValue = 1.0f - CosTheta;

    Result.V[0][0] = (Axis.X * Axis.X * CosValue) + CosTheta;
    Result.V[0][1] = (Axis.X * Axis.Y * CosValue) + (Axis.Z * SinTheta);
    Result.V[0][2] = (Axis.X * Axis.Z * CosValue) - (Axis.Y * SinTheta);
    Result.V[1][0] = (Axis.Y * Axis.X * CosValue) - (Axis.Z * SinTheta);
    Result.V[1][1] = (Axis.Y * Axis.Y * CosValue) + CosTheta;
    Result.V[1][2] = (Axis.Y * Axis.Z * CosValue) + (Axis.X * SinTheta);
    Result.V[2][0] = (Axis.Z * Axis.X * CosValue) + (Axis.Y * SinTheta);
    Result.V[2][1] = (Axis.Z * Axis.Y * CosValue) - (Axis.X * SinTheta);
    Result.V[2][2] = (Axis.Z * Axis.Z * CosValue) + CosTheta;

    return Result;
}

API_INTERNAL m4f32
operator*(m4f32 MatA, m4f32 MatB)
{
    m4f32 Result = {};

    for (u8 I = 0; I < 4; ++I)
        for (u8 J = 0; J < 4; ++J)
            Result.V[I][J] = (MatA.V[I][0] * MatB.V[0][J] + MatA.V[I][1] * MatB.V[1][J] + MatA.V[I][2] * MatB.V[2][J] + MatA.V[I][3] * MatB.V[3][J]);

    return Result;
}

API_INTERNAL v4f32
operator*(m4f32 Mat, v4f32 Vector)
{
    return Vec(
        Mat.V[0][0] * Vector.X + Mat.V[0][1] * Vector.Y + Mat.V[0][2] * Vector.Z + Mat.V[0][3] * Vector.W,
        Mat.V[1][0] * Vector.X + Mat.V[1][1] * Vector.Y + Mat.V[1][2] * Vector.Z + Mat.V[1][3] * Vector.W,
        Mat.V[2][0] * Vector.X + Mat.V[2][1] * Vector.Y + Mat.V[2][2] * Vector.Z + Mat.V[2][3] * Vector.W,
        Mat.V[3][0] * Vector.X + Mat.V[3][1] * Vector.Y + Mat.V[3][2] * Vector.Z + Mat.V[3][3] * Vector.W
    );
}

API_INTERNAL m4f32
operator*(m4f32 Mat, f32 Scale)
{
    m4f32 Result = {};

    for (u8 J = 0; J < 4; ++J)
        for (u8 I = 0; I < 4; ++I)
            Result.V[I][J] *= Scale;

    return Result;
}

API_INTERNAL m4f32
Inverse(m4f32 Mat)
{
    f32 Coef00 = Mat.V[2][2] * Mat.V[3][3] - Mat.V[3][2] * Mat.V[2][3];
    f32 Coef02 = Mat.V[1][2] * Mat.V[3][3] - Mat.V[3][2] * Mat.V[1][3];
    f32 Coef03 = Mat.V[1][2] * Mat.V[2][3] - Mat.V[2][2] * Mat.V[1][3];
    f32 Coef04 = Mat.V[2][1] * Mat.V[3][3] - Mat.V[3][1] * Mat.V[2][3];
    f32 Coef06 = Mat.V[1][1] * Mat.V[3][3] - Mat.V[3][1] * Mat.V[1][3];
    f32 Coef07 = Mat.V[1][1] * Mat.V[2][3] - Mat.V[2][1] * Mat.V[1][3];
    f32 Coef08 = Mat.V[2][1] * Mat.V[3][2] - Mat.V[3][1] * Mat.V[2][2];
    f32 Coef10 = Mat.V[1][1] * Mat.V[3][2] - Mat.V[3][1] * Mat.V[1][2];
    f32 Coef11 = Mat.V[1][1] * Mat.V[2][2] - Mat.V[2][1] * Mat.V[1][2];
    f32 Coef12 = Mat.V[2][0] * Mat.V[3][3] - Mat.V[3][0] * Mat.V[2][3];
    f32 Coef14 = Mat.V[1][0] * Mat.V[3][3] - Mat.V[3][0] * Mat.V[1][3];
    f32 Coef15 = Mat.V[1][0] * Mat.V[2][3] - Mat.V[2][0] * Mat.V[1][3];
    f32 Coef16 = Mat.V[2][0] * Mat.V[3][2] - Mat.V[3][0] * Mat.V[2][2];
    f32 Coef18 = Mat.V[1][0] * Mat.V[3][2] - Mat.V[3][0] * Mat.V[1][2];
    f32 Coef19 = Mat.V[1][0] * Mat.V[2][2] - Mat.V[2][0] * Mat.V[1][2];
    f32 Coef20 = Mat.V[2][0] * Mat.V[3][1] - Mat.V[3][0] * Mat.V[2][1];
    f32 Coef22 = Mat.V[1][0] * Mat.V[3][1] - Mat.V[3][0] * Mat.V[1][1];
    f32 Coef23 = Mat.V[1][0] * Mat.V[2][1] - Mat.V[2][0] * Mat.V[1][1];
    v4f32 Fac0 = {Coef00, Coef00, Coef02, Coef03};
    v4f32 Fac1 = {Coef04, Coef04, Coef06, Coef07};
    v4f32 Fac2 = {Coef08, Coef08, Coef10, Coef11};
    v4f32 Fac3 = {Coef12, Coef12, Coef14, Coef15};
    v4f32 Fac4 = {Coef16, Coef16, Coef18, Coef19};
    v4f32 Fac5 = {Coef20, Coef20, Coef22, Coef23};
    v4f32 Vec0 = {Mat.V[1][0], Mat.V[0][0], Mat.V[0][0], Mat.V[0][0]};
    v4f32 Vec1 = {Mat.V[1][1], Mat.V[0][1], Mat.V[0][1], Mat.V[0][1]};
    v4f32 Vec2 = {Mat.V[1][2], Mat.V[0][2], Mat.V[0][2], Mat.V[0][2]};
    v4f32 Vec3 = {Mat.V[1][3], Mat.V[0][3], Mat.V[0][3], Mat.V[0][3]};
    v4f32 Inv0 = ((Vec1 * Fac0) - (Vec2 * Fac1)) + (Vec3 * Fac2);
    v4f32 Inv1 = ((Vec0 * Fac0) - (Vec2 * Fac3)) + (Vec3 * Fac4);
    v4f32 Inv2 = ((Vec0 * Fac1) - (Vec1 * Fac3)) + (Vec3 * Fac5);
    v4f32 Inv3 = ((Vec0 * Fac2) - (Vec1 * Fac4)) + (Vec2 * Fac5);
    v4f32 SignA = {+1.0f, -1.0f, +1.0f, -1.0f};
    v4f32 SignB = {-1.0f, +1.0f, -1.0f, +1.0f};
    m4f32 Inverse = {};

    for (u8 I = 0; I < 4; ++I) {
        Inverse.V[0][I] = Inv0.V[I] * SignA.V[I];
        Inverse.V[1][I] = Inv1.V[I] * SignB.V[I];
        Inverse.V[2][I] = Inv2.V[I] * SignA.V[I];
        Inverse.V[3][I] = Inv3.V[I] * SignB.V[I];
    }

    v4f32 Row0 = {Inverse.V[0][0], Inverse.V[1][0], Inverse.V[2][0], Inverse.V[3][0]};
    v4f32 Mat0 = {Mat.V[0][0], Mat.V[0][1], Mat.V[0][2], Mat.V[0][3]};
    v4f32 Dot0 = Mat0 * Row0;
    f32 Dot1 = (Dot0.X + Dot0.Y) + (Dot0.Z + Dot0.W);
    f32 OneOverDet = 1.0f / Dot1;

    return (Inverse * OneOverDet);
}

API_INTERNAL m4f32
DeRotate(m4f32 Mat)
{
    v3f32 Scale = Vec(
        Length(Vec(Mat.V[0][0], Mat.V[0][1], Mat.V[0][2])),
        Length(Vec(Mat.V[1][0], Mat.V[1][1], Mat.V[1][2])),
        Length(Vec(Mat.V[2][0], Mat.V[2][1], Mat.V[2][2]))
    );

    Mat.V[0][0] = Scale.X;
    Mat.V[1][0] = 0.0f;
    Mat.V[2][0] = 0.0f;
    Mat.V[0][1] = 0.0f;
    Mat.V[1][1] = Scale.Y;
    Mat.V[2][1] = 0.0f;
    Mat.V[0][2] = 0.0f;
    Mat.V[1][2] = 0.0f;
    Mat.V[2][2] = Scale.Z;

    return Mat;
}

API_INTERNAL m4f32
Transpose(m4f32 Mat)
{
    m4f32 Result = {
        {
            Mat.V[0][0], Mat.V[1][0], Mat.V[2][0], Mat.V[3][0],
            Mat.V[0][1], Mat.V[1][1], Mat.V[2][1], Mat.V[3][1],
            Mat.V[0][2], Mat.V[1][2], Mat.V[2][2], Mat.V[3][2],
            Mat.V[0][3], Mat.V[1][3], Mat.V[2][3], Mat.V[3][3]
        }
    };

    return Result;
}

// m4f64_______________________________________________________________________
API_INTERNAL m4f64
Mat4(f64 Diag)
{
    m4f64 Result = {};

    Result.V[0][0] = Diag;
    Result.V[1][1] = Diag;
    Result.V[2][2] = Diag;
    Result.V[3][3] = Diag;

    return Result;
}

API_INTERNAL m4f64
Translation(v3f64 Vec)
{
    m4f64 Result = Mat4(1.0);

    Result.V[3][0] = Vec.X;
    Result.V[3][1] = Vec.Y;
    Result.V[3][2] = Vec.Z;

    return Result;
}

API_INTERNAL m4f64
Scaler(v3f64 Vec)
{
    m4f64 Result = Mat4(1.0);

    Result.V[0][0] = Vec.X;
    Result.V[1][1] = Vec.Y;
    Result.V[2][2] = Vec.Z;

    return Result;
}

API_INTERNAL m4f64
Perspective(f64 FOV, f64 AspectRatio, f64 NearZ, f64 FarZ)
{
    m4f64 Result = Mat4(1.0);
    f64 TanHalfFOV = Tan(FOV / 2.0);

    Result.V[0][0] = 1.0 / TanHalfFOV;
    Result.V[1][1] = AspectRatio / TanHalfFOV;
    Result.V[2][3] = 1.0;
    Result.V[2][2] = -(NearZ + FarZ) / (NearZ - FarZ);
    Result.V[3][2] = (2.0f * NearZ * FarZ) / (NearZ - FarZ);
    Result.V[3][3] = 0.0;

    return Result;
}

API_INTERNAL m4f64
Orthographic(f64 Left, f64 Right, f64 Bottom, f64 Top, f64 NearZ, f64 FarZ)
{
    m4f64 Result = Mat4(1.0);

    Result.V[0][0] = 2.0 / (Right - Left);
    Result.V[1][1] = 2.0 / (Top - Bottom);
    Result.V[2][2] = 2.0 / (FarZ - NearZ);
    Result.V[3][3] = 1.0;
    Result.V[3][0] = (Left + Right) / (Left - Right);
    Result.V[3][1] = (Bottom + Top) / (Bottom - Top);
    Result.V[3][2] = (NearZ + FarZ) / (NearZ - FarZ);

    return Result;
}

API_INTERNAL m4f64
LookAt(v3f64 Eye, v3f64 Centre, v3f64 Up)
{
    m4f64 Result = {};
    v3f64 F = Norm(Eye - Centre);
    v3f64 S = Norm(Cross(F, Up));
    v3f64 U = Cross(S, F);

    Result.V[0][0] = S.X;
    Result.V[0][1] = U.X;
    Result.V[0][2] = -F.X;
    Result.V[0][3] = 0.0;
    Result.V[1][0] = S.Y;
    Result.V[1][1] = U.Y;
    Result.V[1][2] = -F.Y;
    Result.V[1][3] = 0.0;
    Result.V[2][0] = S.Z;
    Result.V[2][1] = U.Z;
    Result.V[2][2] = -F.Z;
    Result.V[2][3] = 0.0;
    Result.V[3][0] = -Dot(S, Eye);
    Result.V[3][1] = -Dot(U, Eye);
    Result.V[3][2] = Dot(F, Eye);
    Result.V[3][3] = 1.0;

    return Result;
}

API_INTERNAL m4f64
Rotation(v3f64 Axis, f64 Turns)
{
    m4f64 Result = Mat4(1.0);

    Axis = Norm(Axis);

    f64 SinTheta = Sin(Turns);
    f64 CosTheta = Cos(Turns);
    f64 CosValue = 1.0 - CosTheta;

    Result.V[0][0] = (Axis.X * Axis.X * CosValue) + CosTheta;
    Result.V[0][1] = (Axis.X * Axis.Y * CosValue) + (Axis.Z * SinTheta);
    Result.V[0][2] = (Axis.X * Axis.Z * CosValue) - (Axis.Y * SinTheta);
    Result.V[1][0] = (Axis.Y * Axis.X * CosValue) - (Axis.Z * SinTheta);
    Result.V[1][1] = (Axis.Y * Axis.Y * CosValue) + CosTheta;
    Result.V[1][2] = (Axis.Y * Axis.Z * CosValue) + (Axis.X * SinTheta);
    Result.V[2][0] = (Axis.Z * Axis.X * CosValue) + (Axis.Y * SinTheta);
    Result.V[2][1] = (Axis.Z * Axis.Y * CosValue) - (Axis.X * SinTheta);
    Result.V[2][2] = (Axis.Z * Axis.Z * CosValue) + CosTheta;

    return Result;
}

API_INTERNAL m4f64
operator*(m4f64 MatA, m4f64 MatB)
{
    m4f64 Result = {};

    for (u8 I = 0; I < 4; ++I)
        for (u8 J = 0; J < 4; ++J)
            Result.V[I][J] = (MatA.V[I][0] * MatB.V[0][J] + MatA.V[I][1] * MatB.V[1][J] + MatA.V[I][2] * MatB.V[2][J] + MatA.V[I][3] * MatB.V[3][J]);

    return Result;
}

API_INTERNAL m4f64
operator*(m4f64 Mat, f64 Scale)
{
    m4f64 Result = {};

    for (u8 J = 0; J < 4; ++J)
        for (u8 I = 0; I < 4; ++I)
            Result.V[I][J] *= Scale;

    return Result;
}

API_INTERNAL m4f64
Inverse(m4f64 Mat)
{
    f64 Coef00 = Mat.V[2][2] * Mat.V[3][3] - Mat.V[3][2] * Mat.V[2][3];
    f64 Coef02 = Mat.V[1][2] * Mat.V[3][3] - Mat.V[3][2] * Mat.V[1][3];
    f64 Coef03 = Mat.V[1][2] * Mat.V[2][3] - Mat.V[2][2] * Mat.V[1][3];
    f64 Coef04 = Mat.V[2][1] * Mat.V[3][3] - Mat.V[3][1] * Mat.V[2][3];
    f64 Coef06 = Mat.V[1][1] * Mat.V[3][3] - Mat.V[3][1] * Mat.V[1][3];
    f64 Coef07 = Mat.V[1][1] * Mat.V[2][3] - Mat.V[2][1] * Mat.V[1][3];
    f64 Coef08 = Mat.V[2][1] * Mat.V[3][2] - Mat.V[3][1] * Mat.V[2][2];
    f64 Coef10 = Mat.V[1][1] * Mat.V[3][2] - Mat.V[3][1] * Mat.V[1][2];
    f64 Coef11 = Mat.V[1][1] * Mat.V[2][2] - Mat.V[2][1] * Mat.V[1][2];
    f64 Coef12 = Mat.V[2][0] * Mat.V[3][3] - Mat.V[3][0] * Mat.V[2][3];
    f64 Coef14 = Mat.V[1][0] * Mat.V[3][3] - Mat.V[3][0] * Mat.V[1][3];
    f64 Coef15 = Mat.V[1][0] * Mat.V[2][3] - Mat.V[2][0] * Mat.V[1][3];
    f64 Coef16 = Mat.V[2][0] * Mat.V[3][2] - Mat.V[3][0] * Mat.V[2][2];
    f64 Coef18 = Mat.V[1][0] * Mat.V[3][2] - Mat.V[3][0] * Mat.V[1][2];
    f64 Coef19 = Mat.V[1][0] * Mat.V[2][2] - Mat.V[2][0] * Mat.V[1][2];
    f64 Coef20 = Mat.V[2][0] * Mat.V[3][1] - Mat.V[3][0] * Mat.V[2][1];
    f64 Coef22 = Mat.V[1][0] * Mat.V[3][1] - Mat.V[3][0] * Mat.V[1][1];
    f64 Coef23 = Mat.V[1][0] * Mat.V[2][1] - Mat.V[2][0] * Mat.V[1][1];
    v4f64 Fac0 = {Coef00, Coef00, Coef02, Coef03};
    v4f64 Fac1 = {Coef04, Coef04, Coef06, Coef07};
    v4f64 Fac2 = {Coef08, Coef08, Coef10, Coef11};
    v4f64 Fac3 = {Coef12, Coef12, Coef14, Coef15};
    v4f64 Fac4 = {Coef16, Coef16, Coef18, Coef19};
    v4f64 Fac5 = {Coef20, Coef20, Coef22, Coef23};
    v4f64 Vec0 = {Mat.V[1][0], Mat.V[0][0], Mat.V[0][0], Mat.V[0][0]};
    v4f64 Vec1 = {Mat.V[1][1], Mat.V[0][1], Mat.V[0][1], Mat.V[0][1]};
    v4f64 Vec2 = {Mat.V[1][2], Mat.V[0][2], Mat.V[0][2], Mat.V[0][2]};
    v4f64 Vec3 = {Mat.V[1][3], Mat.V[0][3], Mat.V[0][3], Mat.V[0][3]};
    v4f64 Inv0 = ((Vec1 * Fac0) - (Vec2 * Fac1)) + (Vec3 * Fac2);
    v4f64 Inv1 = ((Vec0 * Fac0) - (Vec2 * Fac3)) + (Vec3 * Fac4);
    v4f64 Inv2 = ((Vec0 * Fac1) - (Vec1 * Fac3)) + (Vec3 * Fac5);
    v4f64 Inv3 = ((Vec0 * Fac2) - (Vec1 * Fac4)) + (Vec2 * Fac5);
    v4f64 SignA = {+1.0, -1.0, +1.0, -1.0};
    v4f64 SignB = {-1.0, +1.0, -1.0, +1.0};
    m4f64 Inverse = {};

    for (u8 I = 0; I < 4; ++I) {
        Inverse.V[0][I] = Inv0.V[I] * SignA.V[I];
        Inverse.V[1][I] = Inv1.V[I] * SignB.V[I];
        Inverse.V[2][I] = Inv2.V[I] * SignA.V[I];
        Inverse.V[3][I] = Inv3.V[I] * SignB.V[I];
    }

    v4f64 Row0 = {Inverse.V[0][0], Inverse.V[1][0], Inverse.V[2][0], Inverse.V[3][0]};
    v4f64 Mat0 = {Mat.V[0][0], Mat.V[0][1], Mat.V[0][2], Mat.V[0][3]};
    v4f64 Dot0 = Mat0 * Row0;
    f64 Dot1 = (Dot0.X + Dot0.Y) + (Dot0.Z + Dot0.W);
    f64 OneOverDet = 1.0 / Dot1;

    return (Inverse * OneOverDet);
}

API_INTERNAL m4f64
DeRotate(m4f64 Mat)
{
    v3f64 Scale = Vec(
        Length(Vec(Mat.V[0][0], Mat.V[0][1], Mat.V[0][2])),
        Length(Vec(Mat.V[1][0], Mat.V[1][1], Mat.V[1][2])),
        Length(Vec(Mat.V[2][0], Mat.V[2][1], Mat.V[2][2]))
    );

    Mat.V[0][0] = Scale.X;
    Mat.V[1][0] = 0.0;
    Mat.V[2][0] = 0.0;
    Mat.V[0][1] = 0.0;
    Mat.V[1][1] = Scale.Y;
    Mat.V[2][1] = 0.0;
    Mat.V[0][2] = 0.0;
    Mat.V[1][2] = 0.0;
    Mat.V[2][2] = Scale.Z;

    return Mat;
}

API_INTERNAL m4f64
Transpose(m4f64 Mat)
{
    m4f64 Result = {
        {
            Mat.V[0][0], Mat.V[1][0], Mat.V[2][0], Mat.V[3][0],
            Mat.V[0][1], Mat.V[1][1], Mat.V[2][1], Mat.V[3][1],
            Mat.V[0][2], Mat.V[1][2], Mat.V[2][2], Mat.V[3][2],
            Mat.V[0][3], Mat.V[1][3], Mat.V[2][3], Mat.V[3][3]
        }
    };

    return Result;
}

// r1u32_______________________________________________________________________
API_INTERNAL r1u32
Rng(u32 Min, u32 Max)
{
    r1u32 Result = {};

    if (Min < Max) {
        Result.Min = Min;
        Result.Max = Max;
    } else {
        Result.Min = Max;
        Result.Max = Min;
    }

    return Result;
}

API_INTERNAL r1u32
Shift(r1u32 Range, u32 Val)
{
    r1u32 Result = {};

    Result.Min = Range.Min + Val;
    Result.Max = Range.Max + Val;

    return Result;
}

API_INTERNAL r1u32
Widen(r1u32 Range, u32 Val)
{
    r1u32 Result = {};

    Result.Min = Range.Min - Val;
    Result.Max = Range.Max + Val;

    return Result;
}

API_INTERNAL u32
Centre(r1u32 Range)
{
    return ((Range.Min + Range.Max) / 2);
}

API_INTERNAL b32
InRange(r1u32 Range, u32 Val)
{
    return (Val >= Range.Min && Val < Range.Max);
}

API_INTERNAL u32
Length(r1u32 Range)
{
    u32 Result = 0;

    if (Range.Min < Range.Max)
        Result = Range.Max - Range.Min;

    return Result;
}

API_INTERNAL r1u32
Union(r1u32 RangeA, r1u32 RangeB)
{
    r1u32 Result = {};

    Result.Min = MIN(RangeA.Min, RangeB.Min);
    Result.Max = MAX(RangeA.Max, RangeB.Max);

    return Result;
}

API_INTERNAL r1u32
Intersect(r1u32 RangeA, r1u32 RangeB)
{
    r1u32 Result = {};

    Result.Min = MAX(RangeA.Min, RangeB.Min);
    Result.Max = MIN(RangeA.Max, RangeB.Max);

    return Result;
}

API_INTERNAL u32
Clamp(r1u32 Range, u32 Val)
{
    return CLAMP(Range.Min, Val, Range.Max);
}

// r1i32_______________________________________________________________________
API_INTERNAL r1i32
Rng(i32 Min, i32 Max)
{
    r1i32 Result = {};

    if (Min < Max) {
        Result.Min = Min;
        Result.Max = Max;
    } else {
        Result.Min = Max;
        Result.Max = Min;
    }

    return Result;
}

API_INTERNAL r1i32
Shift(r1i32 Range, i32 Val)
{
    r1i32 Result = {};

    Result.Min = Range.Min + Val;
    Result.Max = Range.Max + Val;

    return Result;
}

API_INTERNAL r1i32
Widen(r1i32 Range, i32 Val)
{
    r1i32 Result = {};

    Result.Min = Range.Min - Val;
    Result.Max = Range.Max + Val;

    return Result;
}

API_INTERNAL i32
Centre(r1i32 Range)
{
    return ((Range.Min + Range.Max) / 2);
}

API_INTERNAL b32
InRange(r1i32 Range, i32 Val)
{
    return (Val >= Range.Min && Val < Range.Max);
}

API_INTERNAL i32
Length(r1i32 Range)
{
    i32 Result = 0;

    if (Range.Min < Range.Max)
        Result = Range.Max - Range.Min;

    return Result;
}

API_INTERNAL r1i32
Union(r1i32 RangeA, r1i32 RangeB)
{
    r1i32 Result = {};

    Result.Min = MIN(RangeA.Min, RangeB.Min);
    Result.Max = MAX(RangeA.Max, RangeB.Max);

    return Result;
}

API_INTERNAL r1i32
Intersect(r1i32 RangeA, r1i32 RangeB)
{
    r1i32 Result = {};

    Result.Min = MAX(RangeA.Min, RangeB.Min);
    Result.Max = MIN(RangeA.Max, RangeB.Max);

    return Result;
}

API_INTERNAL i32
Clamp(r1i32 Range, i32 Val)
{
    return CLAMP(Range.Min, Val, Range.Max);
}

// r1u64_______________________________________________________________________
API_INTERNAL r1u64
Rng(u64 Min, u64 Max)
{
    r1u64 Result = {};

    if (Min < Max) {
        Result.Min = Min;
        Result.Max = Max;
    } else {
        Result.Min = Max;
        Result.Max = Min;
    }

    return Result;
}

API_INTERNAL r1u64
Shift(r1u64 Range, u64 Val)
{
    r1u64 Result = {};

    Result.Min = Range.Min + Val;
    Result.Max = Range.Max + Val;

    return Result;
}

API_INTERNAL r1u64
Widen(r1u64 Range, u64 Val)
{
    r1u64 Result = {};

    Result.Min = Range.Min - Val;
    Result.Max = Range.Max + Val;

    return Result;
}

API_INTERNAL u64
Centre(r1u64 Range)
{
    return ((Range.Min + Range.Max) / 2);
}

API_INTERNAL b32
InRange(r1u64 Range, u64 Val)
{
    return (Val >= Range.Min && Val < Range.Max);
}

API_INTERNAL u64
Length(r1u64 Range)
{
    u64 Result = 0;

    if (Range.Min < Range.Max)
        Result = Range.Max - Range.Min;

    return Result;
}

API_INTERNAL r1u64
Union(r1u64 RangeA, r1u64 RangeB)
{
    r1u64 Result = {};

    Result.Min = MIN(RangeA.Min, RangeB.Min);
    Result.Max = MAX(RangeA.Max, RangeB.Max);

    return Result;
}

API_INTERNAL r1u64
Intersect(r1u64 RangeA, r1u64 RangeB)
{
    r1u64 Result = {};

    Result.Min = MAX(RangeA.Min, RangeB.Min);
    Result.Max = MIN(RangeA.Max, RangeB.Max);

    return Result;
}

API_INTERNAL u64
Clamp(r1u64 Range, u64 Val)
{
    return CLAMP(Range.Min, Val, Range.Max);
}

// r1i64_______________________________________________________________________
API_INTERNAL r1i64
Rng(i64 Min, i64 Max)
{
    r1i64 Result = {};

    if (Min < Max) {
        Result.Min = Min;
        Result.Max = Max;
    } else {
        Result.Min = Max;
        Result.Max = Min;
    }

    return Result;
}

API_INTERNAL r1i64
Shift(r1i64 Range, i64 Val)
{
    r1i64 Result = {};

    Result.Min = Range.Min + Val;
    Result.Max = Range.Max + Val;

    return Result;
}

API_INTERNAL r1i64
Widen(r1i64 Range, i64 Val)
{
    r1i64 Result = {};

    Result.Min = Range.Min - Val;
    Result.Max = Range.Max + Val;

    return Result;
}

API_INTERNAL i64
Centre(r1i64 Range)
{
    return ((Range.Min + Range.Max) / 2);
}

API_INTERNAL b32
InRange(r1i64 Range, i64 Val)
{
    return (Val >= Range.Min && Val < Range.Max);
}

API_INTERNAL i64
Length(r1i64 Range)
{
    i64 Result = 0;

    if (Range.Min < Range.Max)
        Result = Range.Max - Range.Min;

    return Result;
}

API_INTERNAL r1i64
Union(r1i64 RangeA, r1i64 RangeB)
{
    r1i64 Result = {};

    Result.Min = MIN(RangeA.Min, RangeB.Min);
    Result.Max = MAX(RangeA.Max, RangeB.Max);

    return Result;
}

API_INTERNAL r1i64
Intersect(r1i64 RangeA, r1i64 RangeB)
{
    r1i64 Result = {};

    Result.Min = MAX(RangeA.Min, RangeB.Min);
    Result.Max = MIN(RangeA.Max, RangeB.Max);

    return Result;
}

API_INTERNAL i64
Clamp(r1i64 Range, i64 Val)
{
    return CLAMP(Range.Min, Val, Range.Max);
}

// r1f32_______________________________________________________________________
API_INTERNAL r1f32
Rng(f32 Min, f32 Max)
{
    r1f32 Result = {};

    if (Min < Max) {
        Result.Min = Min;
        Result.Max = Max;
    } else {
        Result.Min = Max;
        Result.Max = Min;
    }

    return Result;
}

API_INTERNAL r1f32
Shift(r1f32 Range, f32 Val)
{
    r1f32 Result = {};

    Result.Min = Range.Min + Val;
    Result.Max = Range.Max + Val;

    return Result;
}

API_INTERNAL r1f32
Widen(r1f32 Range, f32 Val)
{
    r1f32 Result = {};

    Result.Min = Range.Min - Val;
    Result.Max = Range.Max + Val;

    return Result;
}

API_INTERNAL f32
Centre(r1f32 Range)
{
    return ((Range.Min + Range.Max) / 2.0f);
}

API_INTERNAL b32
InRange(r1f32 Range, f32 Val)
{
    return (Val >= Range.Min && Val < Range.Max);
}

API_INTERNAL f32
Length(r1f32 Range)
{
    f32 Result = 0.0f;

    if (Range.Min < Range.Max)
        Result = Range.Max - Range.Min;

    return Result;
}

API_INTERNAL r1f32
Union(r1f32 RangeA, r1f32 RangeB)
{
    r1f32 Result = {};

    Result.Min = MIN(RangeA.Min, RangeB.Min);
    Result.Max = MAX(RangeA.Max, RangeB.Max);

    return Result;
}

API_INTERNAL r1f32
Intersect(r1f32 RangeA, r1f32 RangeB)
{
    r1f32 Result = {};

    Result.Min = MAX(RangeA.Min, RangeB.Min);
    Result.Max = MIN(RangeA.Max, RangeB.Max);

    return Result;
}

API_INTERNAL f32
Clamp(r1f32 Range, f32 Val)
{
    return CLAMP(Range.Min, Val, Range.Max);
}

// r1f64_______________________________________________________________________
API_INTERNAL r1f64
Rng(f64 Min, f64 Max)
{
    r1f64 Result = {};

    if (Min < Max) {
        Result.Min = Min;
        Result.Max = Max;
    } else {
        Result.Min = Max;
        Result.Max = Min;
    }

    return Result;
}

API_INTERNAL r1f64
Shift(r1f64 Range, f64 Val)
{
    r1f64 Result = {};

    Result.Min = Range.Min + Val;
    Result.Max = Range.Max + Val;

    return Result;
}

API_INTERNAL r1f64
Widen(r1f64 Range, f64 Val)
{
    r1f64 Result = {};

    Result.Min = Range.Min - Val;
    Result.Max = Range.Max + Val;

    return Result;
}

API_INTERNAL f64
Centre(r1f64 Range)
{
    return ((Range.Min + Range.Max) / 2.0);
}

API_INTERNAL b32
InRange(r1f64 Range, f64 Val)
{
    return (Val >= Range.Min && Val < Range.Max);
}

API_INTERNAL f64
Length(r1f64 Range)
{
    f64 Result = 0.0;

    if (Range.Min < Range.Max)
        Result = Range.Max - Range.Min;

    return Result;
}

API_INTERNAL r1f64
Union(r1f64 RangeA, r1f64 RangeB)
{
    r1f64 Result = {};

    Result.Min = MIN(RangeA.Min, RangeB.Min);
    Result.Max = MAX(RangeA.Max, RangeB.Max);

    return Result;
}

API_INTERNAL r1f64
Intersect(r1f64 RangeA, r1f64 RangeB)
{
    r1f64 Result = {};

    Result.Min = MAX(RangeA.Min, RangeB.Min);
    Result.Max = MIN(RangeA.Max, RangeB.Max);

    return Result;
}

API_INTERNAL f64
Clamp(r1f64 Range, f64 Val)
{
    return CLAMP(Range.Min, Val, Range.Max);
}

// r2i16_______________________________________________________________________
API_INTERNAL r2i16
Rng(v2i16 Min, v2i16 Max)
{
    r2i16 Result = {};

    Result.Min.X = MIN(Min.X, Max.X);
    Result.Min.Y = MIN(Min.Y, Max.Y);
    Result.Max.X = MAX(Min.X, Max.X);
    Result.Max.Y = MAX(Min.Y, Max.Y);

    return Result;
}

API_INTERNAL r2i16
Rng(i16 MinX, i16 MinY, i16 MaxX, i16 MaxY)
{
    r2i16 Result = {};

    Result.Min.X = MIN(MinX, MaxX);
    Result.Min.Y = MIN(MinY, MaxY);
    Result.Max.X = MAX(MinX, MaxX);
    Result.Max.Y = MAX(MinY, MaxY);

    return Result;
}

API_INTERNAL r2i16
Shift(r2i16 Range, v2i16 Vec)
{
    r2i16 Result = {};

    Result.Min = Range.Min + Vec;
    Result.Max = Range.Max + Vec;

    return Result;
}

API_INTERNAL r2i16
Widen(r2i16 Range, v2i16 Vec)
{
    r2i16 Result = {};

    Result.Min = Range.Min - Vec;
    Result.Max = Range.Max + Vec;

    return Result;
}

API_INTERNAL v2i16
Centre(r2i16 Range)
{
    v2i16 Result = {};

    Result.X = ((Range.Min.X + Range.Max.X) / 2);
    Result.Y = ((Range.Min.Y + Range.Max.Y) / 2);

    return Result;
}

API_INTERNAL b32
InRange(r2i16 Range, v2i16 Vec)
{
    return (Vec.X >= Range.Min.X && Vec.X < Range.Max.X && Vec.Y >= Range.Min.Y && Vec.Y < Range.Max.Y);
}

API_INTERNAL v2i16
Length(r2i16 Range)
{
    v2i16 Result = {};

    if (Range.Min.X < Range.Max.X)
        Result.X = Range.Max.X - Range.Min.X;

    if (Range.Min.Y < Range.Max.Y)
        Result.Y = Range.Max.Y - Range.Min.Y;

    return Result;
}

API_INTERNAL r2i16
Union(r2i16 RangeA, r2i16 RangeB)
{
    r2i16 Result = {};

    Result.Point0.X = MIN(RangeA.Min.X, RangeB.Min.X);
    Result.Point0.Y = MIN(RangeA.Min.Y, RangeB.Min.Y);
    Result.Point1.X = MAX(RangeA.Max.X, RangeB.Max.X);
    Result.Point1.Y = MAX(RangeA.Max.Y, RangeB.Max.Y);

    return Result;
}

API_INTERNAL r2i16
Intersect(r2i16 RangeA, r2i16 RangeB)
{
    r2i16 Result = {};

    Result.Point0.X = MAX(RangeA.Min.X, RangeB.Min.X);
    Result.Point0.Y = MAX(RangeA.Min.Y, RangeB.Min.Y);
    Result.Point1.X = MIN(RangeA.Max.X, RangeB.Max.X);
    Result.Point1.Y = MIN(RangeA.Max.Y, RangeB.Max.Y);

    return Result;
}

API_INTERNAL v2i16
Clamp(r2i16 Range, v2i16 Vec)
{
    CLAMP(Range.Min.X, Vec.X, Range.Max.X);
    CLAMP(Range.Min.Y, Vec.Y, Range.Max.Y);

    return Vec;
}

// r2i32_______________________________________________________________________
API_INTERNAL r2i32
Rng(v2i32 Min, v2i32 Max)
{
    r2i32 Result = {};

    Result.Min.X = MIN(Min.X, Max.X);
    Result.Min.Y = MIN(Min.Y, Max.Y);
    Result.Max.X = MAX(Min.X, Max.X);
    Result.Max.Y = MAX(Min.Y, Max.Y);

    return Result;
}

API_INTERNAL r2i32
Rng(i32 MinX, i32 MinY, i32 MaxX, i32 MaxY)
{
    r2i32 Result = {};

    Result.Min.X = MIN(MinX, MaxX);
    Result.Min.Y = MIN(MinY, MaxY);
    Result.Max.X = MAX(MinX, MaxX);
    Result.Max.Y = MAX(MinY, MaxY);

    return Result;
}

API_INTERNAL r2i32
Shift(r2i32 Range, v2i32 Vec)
{
    r2i32 Result = {};

    Result.Min = Range.Min + Vec;
    Result.Max = Range.Max + Vec;

    return Result;
}

API_INTERNAL r2i32
Widen(r2i32 Range, v2i32 Vec)
{
    r2i32 Result = {};

    Result.Min = Range.Min - Vec;
    Result.Max = Range.Max + Vec;

    return Result;
}

API_INTERNAL v2i32
Centre(r2i32 Range)
{
    v2i32 Result = {};

    Result.X = ((Range.Min.X + Range.Max.X) / 2);
    Result.Y = ((Range.Min.Y + Range.Max.Y) / 2);

    return Result;
}

API_INTERNAL b32
InRange(r2i32 Range, v2i32 Vec)
{
    return (Vec.X >= Range.Min.X && Vec.X < Range.Max.X && Vec.Y >= Range.Min.Y && Vec.Y < Range.Max.Y);
}

API_INTERNAL v2i32
Length(r2i32 Range)
{
    v2i32 Result = {};

    if (Range.Min.X < Range.Max.X)
        Result.X = Range.Max.X - Range.Min.X;

    if (Range.Min.Y < Range.Max.Y)
        Result.Y = Range.Max.Y - Range.Min.Y;

    return Result;
}

API_INTERNAL r2i32
Union(r2i32 RangeA, r2i32 RangeB)
{
    r2i32 Result = {};

    Result.Point0.X = MIN(RangeA.Min.X, RangeB.Min.X);
    Result.Point0.Y = MIN(RangeA.Min.Y, RangeB.Min.Y);
    Result.Point1.X = MAX(RangeA.Max.X, RangeB.Max.X);
    Result.Point1.Y = MAX(RangeA.Max.Y, RangeB.Max.Y);

    return Result;
}

API_INTERNAL r2i32
Intersect(r2i32 RangeA, r2i32 RangeB)
{
    r2i32 Result = {};

    Result.Point0.X = MAX(RangeA.Min.X, RangeB.Min.X);
    Result.Point0.Y = MAX(RangeA.Min.Y, RangeB.Min.Y);
    Result.Point1.X = MIN(RangeA.Max.X, RangeB.Max.X);
    Result.Point1.Y = MIN(RangeA.Max.Y, RangeB.Max.Y);

    return Result;
}

API_INTERNAL v2i32
Clamp(r2i32 Range, v2i32 Vec)
{
    CLAMP(Range.Min.X, Vec.X, Range.Max.X);
    CLAMP(Range.Min.Y, Vec.Y, Range.Max.Y);

    return Vec;
}

// r2i64_______________________________________________________________________
API_INTERNAL r2i64
Rng(v2i64 Min, v2i64 Max)
{
    r2i64 Result = {};

    Result.Min.X = MIN(Min.X, Max.X);
    Result.Min.Y = MIN(Min.Y, Max.Y);
    Result.Max.X = MAX(Min.X, Max.X);
    Result.Max.Y = MAX(Min.Y, Max.Y);

    return Result;
}

API_INTERNAL r2i64
Rng(i64 MinX, i64 MinY, i64 MaxX, i64 MaxY)
{
    r2i64 Result = {};

    Result.Min.X = MIN(MinX, MaxX);
    Result.Min.Y = MIN(MinY, MaxY);
    Result.Max.X = MAX(MinX, MaxX);
    Result.Max.Y = MAX(MinY, MaxY);

    return Result;
}

API_INTERNAL r2i64
Shift(r2i64 Range, v2i64 Vec)
{
    r2i64 Result = {};

    Result.Min = Range.Min + Vec;
    Result.Max = Range.Max + Vec;

    return Result;
}

API_INTERNAL r2i64
Widen(r2i64 Range, v2i64 Vec)
{
    r2i64 Result = {};

    Result.Min = Range.Min - Vec;
    Result.Max = Range.Max + Vec;

    return Result;
}

API_INTERNAL v2i64
Centre(r2i64 Range)
{
    v2i64 Result = {};

    Result.X = ((Range.Min.X + Range.Max.X) / 2);
    Result.Y = ((Range.Min.Y + Range.Max.Y) / 2);

    return Result;
}

API_INTERNAL b32
InRange(r2i64 Range, v2i64 Vec)
{
    return (Vec.X >= Range.Min.X && Vec.X < Range.Max.X && Vec.Y >= Range.Min.Y && Vec.Y < Range.Max.Y);
}

API_INTERNAL v2i64
Length(r2i64 Range)
{
    v2i64 Result = {};

    if (Range.Min.X < Range.Max.X)
        Result.X = Range.Max.X - Range.Min.X;

    if (Range.Min.Y < Range.Max.Y)
        Result.Y = Range.Max.Y - Range.Min.Y;

    return Result;
}

API_INTERNAL r2i64
Union(r2i64 RangeA, r2i64 RangeB)
{
    r2i64 Result = {};

    Result.Point0.X = MIN(RangeA.Min.X, RangeB.Min.X);
    Result.Point0.Y = MIN(RangeA.Min.Y, RangeB.Min.Y);
    Result.Point1.X = MAX(RangeA.Max.X, RangeB.Max.X);
    Result.Point1.Y = MAX(RangeA.Max.Y, RangeB.Max.Y);

    return Result;
}

API_INTERNAL r2i64
Intersect(r2i64 RangeA, r2i64 RangeB)
{
    r2i64 Result = {};

    Result.Point0.X = MAX(RangeA.Min.X, RangeB.Min.X);
    Result.Point0.Y = MAX(RangeA.Min.Y, RangeB.Min.Y);
    Result.Point1.X = MIN(RangeA.Max.X, RangeB.Max.X);
    Result.Point1.Y = MIN(RangeA.Max.Y, RangeB.Max.Y);

    return Result;
}

API_INTERNAL v2i64
Clamp(r2i64 Range, v2i64 Vec)
{
    CLAMP(Range.Min.X, Vec.X, Range.Max.X);
    CLAMP(Range.Min.Y, Vec.Y, Range.Max.Y);

    return Vec;
}

// r2f32_______________________________________________________________________
API_INTERNAL r2f32
Rng(v2f32 Min, v2f32 Max)
{
    r2f32 Result = {};

    Result.Min.X = MIN(Min.X, Max.X);
    Result.Min.Y = MIN(Min.Y, Max.Y);
    Result.Max.X = MAX(Min.X, Max.X);
    Result.Max.Y = MAX(Min.Y, Max.Y);

    return Result;
}

API_INTERNAL r2f32
Rng(f32 MinX, f32 MinY, f32 MaxX, f32 MaxY)
{
    r2f32 Result = {};

    Result.Min.X = MIN(MinX, MaxX);
    Result.Min.Y = MIN(MinY, MaxY);
    Result.Max.X = MAX(MinX, MaxX);
    Result.Max.Y = MAX(MinY, MaxY);

    return Result;
}

API_INTERNAL r2f32
Shift(r2f32 Range, v2f32 Vec)
{
    r2f32 Result = {};

    Result.Min = Range.Min + Vec;
    Result.Max = Range.Max + Vec;

    return Result;
}

API_INTERNAL r2f32
Widen(r2f32 Range, v2f32 Vec)
{
    r2f32 Result = {};

    Result.Min = Range.Min - Vec;
    Result.Max = Range.Max + Vec;

    return Result;
}

API_INTERNAL r2f32
Widen(r2f32 Range, f32 Value)
{
    r2f32 Result = {};

    Result.Min.X = Range.Min.X - Value;
    Result.Min.Y = Range.Min.Y - Value;
    Result.Max.X = Range.Max.X + Value;
    Result.Max.Y = Range.Max.Y + Value;

    return Result;
}

API_INTERNAL v2f32
Centre(r2f32 Range)
{
    v2f32 Result = {};

    Result.X = ((Range.Min.X + Range.Max.X) / 2);
    Result.Y = ((Range.Min.Y + Range.Max.Y) / 2);

    return Result;
}

API_INTERNAL b32
InRange(r2f32 Range, v2f32 Vec)
{
    return (Vec.X >= Range.Min.X && Vec.X < Range.Max.X && Vec.Y >= Range.Min.Y && Vec.Y < Range.Max.Y);
}

API_INTERNAL v2f32
Length(r2f32 Range)
{
    v2f32 Result = {};

    if (Range.Min.X < Range.Max.X)
        Result.X = Range.Max.X - Range.Min.X;

    if (Range.Min.Y < Range.Max.Y)
        Result.Y = Range.Max.Y - Range.Min.Y;

    return Result;
}

API_INTERNAL r2f32
Union(r2f32 RangeA, r2f32 RangeB)
{
    r2f32 Result = {};

    Result.Point0.X = MIN(RangeA.Min.X, RangeB.Min.X);
    Result.Point0.Y = MIN(RangeA.Min.Y, RangeB.Min.Y);
    Result.Point1.X = MAX(RangeA.Max.X, RangeB.Max.X);
    Result.Point1.Y = MAX(RangeA.Max.Y, RangeB.Max.Y);

    return Result;
}

API_INTERNAL r2f32
Intersect(r2f32 RangeA, r2f32 RangeB)
{
    r2f32 Result = {};

    Result.Point0.X = MAX(RangeA.Min.X, RangeB.Min.X);
    Result.Point0.Y = MAX(RangeA.Min.Y, RangeB.Min.Y);
    Result.Point1.X = MIN(RangeA.Max.X, RangeB.Max.X);
    Result.Point1.Y = MIN(RangeA.Max.Y, RangeB.Max.Y);

    return Result;
}

API_INTERNAL v2f32
Clamp(r2f32 Range, v2f32 Vec)
{
    CLAMP(Range.Min.X, Vec.X, Range.Max.X);
    CLAMP(Range.Min.Y, Vec.Y, Range.Max.Y);

    return Vec;
}

// r2f64_______________________________________________________________________
API_INTERNAL r2f64
Rng(v2f64 Min, v2f64 Max)
{
    r2f64 Result = {};

    Result.Min.X = MIN(Min.X, Max.X);
    Result.Min.Y = MIN(Min.Y, Max.Y);
    Result.Max.X = MAX(Min.X, Max.X);
    Result.Max.Y = MAX(Min.Y, Max.Y);

    return Result;
}

API_INTERNAL r2f64
Rng(f64 MinX, f64 MinY, f64 MaxX, f64 MaxY)
{
    r2f64 Result = {};

    Result.Min.X = MIN(MinX, MaxX);
    Result.Min.Y = MIN(MinY, MaxY);
    Result.Max.X = MAX(MinX, MaxX);
    Result.Max.Y = MAX(MinY, MaxY);

    return Result;
}

API_INTERNAL r2f64
Shift(r2f64 Range, v2f64 Vec)
{
    r2f64 Result = {};

    Result.Min = Range.Min + Vec;
    Result.Max = Range.Max + Vec;

    return Result;
}

API_INTERNAL r2f64
Widen(r2f64 Range, v2f64 Vec)
{
    r2f64 Result = {};

    Result.Min = Range.Min - Vec;
    Result.Max = Range.Max + Vec;

    return Result;
}

API_INTERNAL r2f64
Widen(r2f64 Range, f64 Value)
{
    r2f64 Result = {};

    Result.Min.X = Range.Min.X - Value;
    Result.Min.Y = Range.Min.Y - Value;
    Result.Max.X = Range.Max.X + Value;
    Result.Max.Y = Range.Max.Y + Value;

    return Result;
}

API_INTERNAL v2f64
Centre(r2f64 Range)
{
    v2f64 Result = {};

    Result.X = ((Range.Min.X + Range.Max.X) / 2);
    Result.Y = ((Range.Min.Y + Range.Max.Y) / 2);

    return Result;
}

API_INTERNAL b32
InRange(r2f64 Range, v2f64 Vec)
{
    return (Vec.X >= Range.Min.X && Vec.X < Range.Max.X && Vec.Y >= Range.Min.Y && Vec.Y < Range.Max.Y);
}

API_INTERNAL v2f64
Length(r2f64 Range)
{
    v2f64 Result = {};

    if (Range.Min.X < Range.Max.X)
        Result.X = Range.Max.X - Range.Min.X;

    if (Range.Min.Y < Range.Max.Y)
        Result.Y = Range.Max.Y - Range.Min.Y;

    return Result;
}

API_INTERNAL r2f64
Union(r2f64 RangeA, r2f64 RangeB)
{
    r2f64 Result = {};

    Result.Point0.X = MIN(RangeA.Min.X, RangeB.Min.X);
    Result.Point0.Y = MIN(RangeA.Min.Y, RangeB.Min.Y);
    Result.Point1.X = MAX(RangeA.Max.X, RangeB.Max.X);
    Result.Point1.Y = MAX(RangeA.Max.Y, RangeB.Max.Y);

    return Result;
}

API_INTERNAL r2f64
Intersect(r2f64 RangeA, r2f64 RangeB)
{
    r2f64 Result = {};

    Result.Point0.X = MAX(RangeA.Min.X, RangeB.Min.X);
    Result.Point0.Y = MAX(RangeA.Min.Y, RangeB.Min.Y);
    Result.Point1.X = MIN(RangeA.Max.X, RangeB.Max.X);
    Result.Point1.Y = MIN(RangeA.Max.Y, RangeB.Max.Y);

    return Result;
}

API_INTERNAL v2f64
Clamp(r2f64 Range, v2f64 Vec)
{
    CLAMP(Range.Min.X, Vec.X, Range.Max.X);
    CLAMP(Range.Min.Y, Vec.Y, Range.Max.Y);

    return Vec;
}

// r3f32_______________________________________________________________________
API_INTERNAL r3f32
Rng(v3f32 Min, v3f32 Max)
{
    r3f32 Result = {};

    Result.Min.X = MIN(Min.X, Max.X);
    Result.Min.Y = MIN(Min.Y, Max.Y);
    Result.Min.Z = MIN(Min.Z, Max.Z);
    Result.Max.X = MAX(Min.X, Max.X);
    Result.Max.Y = MAX(Min.Y, Max.Y);
    Result.Max.Y = MAX(Min.Z, Max.Z);

    return Result;
}

API_INTERNAL r3f32
Rng(f32 MinX, f32 MinY, f32 MinZ, f32 MaxX, f32 MaxY, f32 MaxZ)
{
    r3f32 Result = {};

    Result.Min.X = MIN(MinX, MaxX);
    Result.Min.Y = MIN(MinY, MaxY);
    Result.Min.Z = MIN(MinZ, MaxZ);
    Result.Max.X = MAX(MinX, MaxX);
    Result.Max.Y = MAX(MinY, MaxY);
    Result.Max.Y = MAX(MinZ, MaxZ);

    return Result;
}

API_INTERNAL r3f32
Shift(r3f32 Range, v3f32 Vec)
{
    r3f32 Result = {};

    Result.Min = Range.Min + Vec;
    Result.Max = Range.Max + Vec;

    return Result;
}

API_INTERNAL r3f32
Widen(r3f32 Range, v3f32 Vec)
{
    r3f32 Result = {};

    Result.Min = Range.Min - Vec;
    Result.Max = Range.Max + Vec;

    return Result;
}

API_INTERNAL r3f32
Widen(r3f32 Range, f32 Value)
{
    r3f32 Result = {};

    Result.Min.X = Range.Min.X - Value;
    Result.Min.Y = Range.Min.Y - Value;
    Result.Min.Z = Range.Min.Z - Value;
    Result.Max.X = Range.Max.X + Value;
    Result.Max.Y = Range.Max.Y + Value;
    Result.Max.Z = Range.Max.Z + Value;

    return Result;
}

API_INTERNAL v3f32
Centre(r3f32 Range)
{
    v3f32 Result = {};

    Result.X = ((Range.Min.X + Range.Max.X) / 2.0f);
    Result.Y = ((Range.Min.Y + Range.Max.Y) / 2.0f);

    return Result;
}

API_INTERNAL b32
InRange(r3f32 Range, v3f32 Vec)
{
    return (
        Vec.X >= Range.Min.X && 
        Vec.X < Range.Max.X && 
        Vec.Y >= Range.Min.Y && 
        Vec.Y < Range.Max.Y &&
        Vec.Z >= Range.Min.Z &&
        Vec.Z < Range.Max.Z
    );
}

API_INTERNAL v3f32
Length(r3f32 Range)
{
    v3f32 Result = {};

    if (Range.Min.X < Range.Max.X)
        Result.X = Range.Max.X - Range.Min.X;

    if (Range.Min.Y < Range.Max.Y)
        Result.Y = Range.Max.Y - Range.Min.Y;

    if (Range.Min.Z < Range.Max.Z)
        Result.Z = Range.Max.Z - Range.Min.Z;

    return Result;
}

API_INTERNAL r3f32
Union(r3f32 RangeA, r3f32 RangeB)
{
    r3f32 Result = {};

    Result.Point0.X = MIN(RangeA.Min.X, RangeB.Min.X);
    Result.Point0.Y = MIN(RangeA.Min.Y, RangeB.Min.Y);
    Result.Point0.Z = MIN(RangeA.Min.Z, RangeB.Min.Z);
    Result.Point1.X = MAX(RangeA.Max.X, RangeB.Max.X);
    Result.Point1.Y = MAX(RangeA.Max.Y, RangeB.Max.Y);
    Result.Point1.Z = MAX(RangeA.Max.Z, RangeB.Max.Z);

    return Result;
}

API_INTERNAL r3f32
Intersect(r3f32 RangeA, r3f32 RangeB)
{
    r3f32 Result = {};

    Result.Point0.X = MAX(RangeA.Min.X, RangeB.Min.X);
    Result.Point0.Y = MAX(RangeA.Min.Y, RangeB.Min.Y);
    Result.Point0.Z = MAX(RangeA.Min.Z, RangeB.Min.Z);
    Result.Point1.X = MIN(RangeA.Max.X, RangeB.Max.X);
    Result.Point1.Y = MIN(RangeA.Max.Y, RangeB.Max.Y);
    Result.Point1.Z = MIN(RangeA.Max.Z, RangeB.Max.Z);

    return Result;
}

API_INTERNAL v3f32
Clamp(r3f32 Range, v3f32 Vec)
{
    CLAMP(Range.Min.X, Vec.X, Range.Max.X);
    CLAMP(Range.Min.Y, Vec.Y, Range.Max.Y);
    CLAMP(Range.Min.Z, Vec.Z, Range.Max.Z);

    return Vec;
}


API_INTERNAL r3f32 
Transform(r3f32 Range, m4f32 Mat)
{
    r3f32 Result = {};
    v3f32 T = Vec(Mat.V[3][0], Mat.V[3][1], Mat.V[3][2]);
    v3f32 Min = T;
    v3f32 Max = T;

    for (u32 I = 0; I < 3; ++I) {
        for (u32 J = 0; J < 3; ++J) {
            f32 A = Mat.V[J][I] * Range.Min.V[J];
            f32 B = Mat.V[J][I] * Range.Max.V[J];

            Min.V[I] += MIN(A, B);
            Max.V[I] += MAX(A, B);
        }
    }

    Result.Min = Min;
    Result.Max = Max;

    return Result;
}

//_____________________________________________________________________________

API_INTERNAL v3f32
HSVFromRGB(v3f32 Colour)
{
    f32 ColourMax = MAX(Colour.X, MAX(Colour.Y, Colour.Z));
    f32 ColourMin = MIN(Colour.X, MIN(Colour.Y, Colour.Z));
    f32 ColourDelta = ColourMax - ColourMin;
    f32 H = 0.0f;

    if (ColourDelta != 0.0f) {
        if (Colour.X == ColourMax) {
            H = FMod((Colour.Y - Colour.Z) / ColourDelta + 6.0f, 6.0f);
        } else if (Colour.Y == ColourMax) {
            H = (Colour.Z - Colour.X) / ColourDelta + 2.0f;
        } else if (Colour.Z == ColourMax) {
            H = (Colour.X - Colour.Y) / ColourDelta + 4.0f;
        }
    }

    f32 S = 0.0f;

    if (ColourMax != 0.0f)
        S = ColourDelta / ColourMax;

    f32 V = ColourMax;
    v3f32 Result = Vec(H / 6.0f, S, V);

    return Result;
}

API_INTERNAL v3f32
RGBFromHSV(v3f32 Colour)
{
    f32 H = FMod(Colour.X * 360.0f, 360.0f);
    f32 S = Colour.Y;
    f32 V = Colour.Z;
    f32 C = V * S;
    f32 X = C * (1.0f - AbsF32(FMod(H / 60.0f, 2.0f) - 1.0f));
    f32 M = V - C;
    f32 R = 0;
    f32 G = 0;
    f32 B = 0;

    if ((H >= 0.0f && H < 60.0f) || (H >= 360.0f && H < 420.0f)) {
        R = C;
        G = X;
        B = 0.0f;
    } else if (H >= 60.0f && H < 120.0f) {
        R = X;
        G = C;
        B = 0.0f;
    } else if (H >= 120.0f && H < 180.0f) {
        R = 0.0f;
        G = C;
        B = X;
    } else if (H >= 180.0f && H < 240.0f) {
        R = 0.0f;
        G = X;
        B = C;
    } else if (H >= 240.0f && H < 300.0f) {
        R = X;
        G = 0.0f;
        B = C;
    } else if ((H >= 300.0f && H <= 360.0f) || (H >= -60.0f && H <= 0.0f)) {
        R = C;
        G = 0.0f;
        B = X;
    }

    v3f32 Result = Vec(R + M, G + M, B + M);

    return Result;
}

API_INTERNAL v4f32
HSVAFromRGBA(v4f32 Colour)
{
    v3f32 RGBColour = Vec(Colour.X, Colour.Y, Colour.Z);
    v3f32 HSVColour = HSVFromRGB(RGBColour);
    v4f32 Result = Vec(HSVColour.X, HSVColour.Y, HSVColour.Z, Colour.W);

    return Result;
}

API_INTERNAL v4f32
RGBAFromHSVA(v4f32 Colour)
{
    v3f32 HSVColour = Vec(Colour.X, Colour.Y, Colour.Z);
    v3f32 RGBColour = RGBFromHSV(HSVColour);
    v4f32 Result = Vec(RGBColour.X, RGBColour.Y, RGBColour.Z, Colour.W);

    return Result;
}

API_INTERNAL u32
U32FromRGBA(v4f32 Colour)
{
    u32 Result = 0;

    Result |= ((u32) ((u8) (Colour.X * 255.0f))) << 24;
    Result |= ((u32) ((u8) (Colour.Y * 255.0f))) << 16;
    Result |= ((u32) ((u8) (Colour.Z * 255.0f))) <<  8;
    Result |= ((u32) ((u8) (Colour.W * 255.0f))) <<  0;

    return Result;
}

API_INTERNAL v4f32
RGBAFromU32(u32 Colour)
{
    v4f32 Result = Vec(
        ((Colour & 0xFF000000) >> 24) / 255.0f,
        ((Colour & 0x00FF0000) >> 16) / 255.0f,
        ((Colour & 0x0000FF00) >>  8) / 255.0f,
        ((Colour & 0x000000FF) >>  0) / 255.0f
    );

    return Result;
}

API_INTERNAL v3f32
LinearFromSRGB(v3f32 Colour)
{
    v3f32 Result = {};

    for (u64 I = 0; I < ARRAY_COUNT(Colour.V); ++I) {
        if (Colour.V[I] <=- 0.04045f)
            Result.V[I] = Colour.V[I] / 12.92f;
        else
            Result.V[I] = Pow((Colour.V[I] + 0.055f) / 1.055f, 2.4f);
    }

    return Result;
}

API_INTERNAL v4f32
LinearFromSRGBA(v4f32 Colour)
{
    v4f32 Result = {};

    Result.XYZ = LinearFromSRGB(Colour.XYZ);
    Result.W = Colour.W;

    return Result;
}

API_INTERNAL void
ListPush(Arena* MemPool, r1u64List* List, r1u64 Range)
    
{
    r1u64Node* Node = ArenaPushArray(MemPool, r1u64Node, 1);

    MemCpy(&Node->V, &Range, sizeof(r1u64));
    SLL_QUEUE_PUSH(List->Head, List->Tail, Node);
    ++List->Count;
}

API_INTERNAL void
ListPush(Arena* MemPool, r1i64List* List, r1i64 Range)
    
{
    r1i64Node* Node = ArenaPushArray(MemPool, r1i64Node, 1);

    MemCpy(&Node->V, &Range, sizeof(r1i64));
    SLL_QUEUE_PUSH(List->Head, List->Tail, Node);
    ++List->Count;
}
