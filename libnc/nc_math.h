#if !defined(__NC_MATH_H__)
#define __NC_MATH_H__

#include "nc_types.h"

struct Arena;

// @defines____________________________________________________________________
#define INF32 _INF32()

#define NC_EPSILON 1e-5f

#if !defined(NC_PI)
    #define NC_PI 3.1415926535897
#endif

#if !defined(NC_TAU) && defined(NC_PI)
    #define NC_TAU NC_PI * 2
#endif

#if !defined(NC_EULER)
    #define NC_EULER 2.718281828459045
#endif

#define RADFromTRN(X) ((X) * NC_TAU)
#define TRNFromRAD(X) ((X) / NC_TAU)
#define DEGFromTRN(X) ((X) * 360.0)
#define TRNFromDEG(X) ((X) / 360.0)
#define DEGFromRAD(X) DEGFromTRN(TRNFromRAD(X))
#define RADFromDEG(X) RADFromTRN(TRNFromDEG(X))

#define LOOKUP_TRIG_CYCLE(X, Y) do {               \
    i64 C = sizeof(X) / sizeof(f64);               \
    f64 NormIndex = (Y) * (f64) C;                 \
    i64 IndexLo = ((i64) FloorF64(NormIndex)) % C; \
    i64 IndexHi = (IndexLo + 1) % C;               \
    f64 XFrac = NormIndex - FloorF64(NormIndex);   \
    return LerpF64(XFrac, X[IndexLo], X[IndexHi]); \
} while (FALSE)

#define LOOKUP_TRIG_NONCYCLE(X, Y) do {                      \
    i64 C = sizeof(X) / sizeof(f64);                         \
    f64 NormIndex = (Y) * (f64) (C - 1);                     \
    i64 IndexLo = (i64) FloorF64(NormIndex);                 \
    i64 IndexHi = (IndexLo + 1 < C) ? IndexLo + 1 : IndexLo; \
    f64 XFrac = NormIndex - (f64) IndexLo;                   \
    return LerpF64(XFrac, X[IndexLo], X[IndexHi]);           \
} while (FALSE)

// @types______________________________________________________________________
union v2f64 {
    struct {
        f64 X;
        f64 Y;
    };

    f64 V[2];
};

union v2f32 {
    struct {
        f32 X;
        f32 Y;
    };

    f32 V[2];
};

union v2i64 {
    struct {
        i64 X;
        i64 Y;
    };

    i64 V[2];
};

union v2i32 {
    struct {
        i32 X;
        i32 Y;
    };

    i32 V[2];
};

union v2u32 {
    struct {
        u32 X;
        u32 Y;
    };

    u32 V[2];
};

union v2i16 {
    struct {
        i16 X;
        i16 Y;
    };

    i16 V[2];
};


union v3f64 {
    struct {
        f64 R;
        f64 G;
        f64 B;
    };

    struct {
        f64 X;
        f64 Y;
        f64 Z;
    };

    struct {
        v2f64 RG;
        f64 _B;
    };

    struct {
        v2f64 XY;
        f64 _Z;
    };

    struct {
        f64 _R;
        v2f64 GB;
    };

    struct {
        f64 _X;
        v2f64 YZ;
    };

    f64 V[3];
};

union v3f32 {
    struct {
        f32 R;
        f32 G;
        f32 B;
    };

    struct {
        f32 X;
        f32 Y;
        f32 Z;
    };

    struct {
        v2f32 RG;
        f32 _B;
    };

    struct {
        v2f32 XY;
        f32 _Z;
    };

    struct {
        f32 _R;
        v2f32 GB;
    };

    struct {
        f32 _X;
        v2f32 YZ;
    };

    f32 V[3];
};

union v3i32 {
    struct {
        i32 X;
        i32 Y;
        i32 Z;
    };

    struct {
        v2i32 XY;
        i32 _Z;
    };

    struct {
        i32 _X;
        v2i32 YZ;
    };

    i32 V[3];
};

union v3u32 {
    struct {
        u32 X;
        u32 Y;
        u32 Z;
    };

    struct {
        v2u32 XY;
        u32 _Z;
    };

    struct {
        u32 _X;
        v2u32 YZ;
    };

    u32 V[3];
};

union v4f64 {
    struct {
        f64 R;
        f64 G;
        f64 B;
        f64 A;
    };

    struct {
        f64 X;
        f64 Y;
        f64 Z;
        f64 W;
    };

    struct {
        v2f64 RG;
        v2f64 BA;
    };

    struct {
        v2f64 XY;
        v2f64 ZW;
    };

    struct {
        v3f64 RGB;
        f64 _A;
    };

    struct {
        v3f64 XYZ;
        f64 _W;
    };

    struct {
        f64 _R;
        v3f64 GBA;
    };

    struct {
        f64 _X;
        v3f64 YZW;
    };

    f64 V[4];
};

union v4f32 {
    struct {
        f32 R;
        f32 G;
        f32 B;
        f32 A;
    };

    struct {
        f32 X;
        f32 Y;
        f32 Z;
        f32 W;
    };

    struct {
        v2f32 RG;
        v2f32 BA;
    };

    struct {
        v2f32 XY;
        v2f32 ZW;
    };

    struct {
        v3f32 RGB;
        f32 _A;
    };

    struct {
        v3f32 XYZ;
        f32 _W;
    };

    struct {
        f32 _R;
        v3f32 GBA;
    };

    struct {
        f32 _X;
        v3f32 YZW;
    };

    f32 V[4];
};

union v4i32 {
    struct {
        i32 X;
        i32 Y;
        i32 Z;
        i32 W;
    };

    struct {
        v2i32 XY;
        v2i32 ZW;
    };

    struct {
        v3i32 XYZ;
        i32 _W;
    };

    struct {
        i32 _X;
        v3i32 YZW;
    };

    i32 V[4];
};

union v4u32 {
    struct {
        u32 X;
        u32 Y;
        u32 Z;
        u32 W;
    };

    struct {
        v2u32 XY;
        v2u32 ZW;
    };

    struct {
        v3u32 XYZ;
        u32 _W;
    };

    struct {
        u32 _X;
        v3u32 YZW;
    };

    u32 V[4];
};

struct m3f64 {
    f64 V[3][3];
};

struct m3f32 {
    f32 V[3][3];
};

struct m4f64 {
    f64 V[4][4];
};

struct m4f32 {
    f32 V[4][4];
};

union r1u32 {
    struct {
        u32 Min;
        u32 Max;
    };

    u32 V[2];
};

union r1i32 {
    struct {
        i32 Min;
        i32 Max;
    };

    i32 V[2];
};

union r1u64 {
    struct {
        u64 Min;
        u64 Max;
    };

    u64 V[2];
};

union r1i64 {
    struct {
        i64 Min;
        i64 Max;
    };

    i64 V[2];
};

union r1f32 {
    struct {
        f32 Min;
        f32 Max;
    };

    f32 V[2];
};

union r1f64 {
    struct {
        f64 Min;
        f64 Max;
    };

    f64 V[2];
};

union r2i16 {
    struct {
        v2i16 Min;
        v2i16 Max;
    };

    struct {
        v2i16 Point0;
        v2i16 Point1;
    };

    struct {
        i16 X0;
        i16 Y0;
        i16 X1;
        i16 Y1;
    };

    v2i16 V[2];
};

union r2i32 {
    struct {
        v2i32 Min;
        v2i32 Max;
    };

    struct {
        v2i32 Point0;
        v2i32 Point1;
    };

    struct {
        i32 X0;
        i32 Y0;
        i32 X1;
        i32 Y1;
    };

    v2i32 V[2];
};

union r2f32 {
    struct {
        v2f32 Min;
        v2f32 Max;
    };

    struct {
        v2f32 Point0;
        v2f32 Point1;
    };

    struct {
        f32 X0;
        f32 Y0;
        f32 X1;
        f32 Y1;
    };

    v2f32 V[2];
};

union r2f64 {
    struct {
        v2f64 Min;
        v2f64 Max;
    };

    struct {
        v2f64 Point0;
        v2f64 Point1;
    };

    struct {
        f64 X0;
        f64 Y0;
        f64 X1;
        f64 Y1;
    };

    v2f64 V[2];
};

union r2i64 {
    struct {
        v2i64 Min;
        v2i64 Max;
    };

    struct {
        v2i64 Point0;
        v2i64 Point1;
    };

    struct {
        i64 X0;
        i64 Y0;
        i64 X1;
        i64 Y1;
    };

    v2i64 V[2];
};

union r3f32 {
    struct {
        v3f32 Min;
        v3f32 Max;
    };

    struct {
        v3f32 Point0;
        v3f32 Point1;
    };

    struct {
        f32 X0;
        f32 Y0;
        f32 Z0;
        f32 X1;
        f32 Y1;
        f32 Z1;
    };

    struct {
        f32 R0;
        f32 G0;
        f32 B0;
        f32 R1;
        f32 G1;
        f32 B1;
    };

    v3f32 V[2];
};

union r3f64 {
    struct {
        v3f64 Min;
        v3f64 Max;
    };

    struct {
        v3f64 Point0;
        v3f64 Point1;
    };

    struct {
        f64 X0;
        f64 Y0;
        f64 Z0;
        f64 X1;
        f64 Y1;
        f64 Z1;
    };

    struct {
        f64 R0;
        f64 G0;
        f64 B0;
        f64 R1;
        f64 G1;
        f64 B1;
    };

    v3f64 V[2];
};

union r4f32 {
    struct {
        v4f32 Min;
        v4f32 Max;
    };

    struct {
        v4f32 Point0;
        v4f32 Point1;
    };

    struct {
        f32 X0;
        f32 Y0;
        f32 Z0;
        f32 W0;
        f32 X1;
        f32 Y1;
        f32 Z1;
        f32 W1;
    };

    struct {
        f32 R0;
        f32 G0;
        f32 B0;
        f32 A0;
        f32 R1;
        f32 G1;
        f32 B1;
        f32 A1;
    };

    v4f32 V[2];
};

union r4f64 {
    struct {
        v4f64 Min;
        v4f64 Max;
    };

    struct {
        v4f64 Point0;
        v4f64 Point1;
    };

    struct {
        f64 X0;
        f64 Y0;
        f64 Z0;
        f64 W0;
        f64 X1;
        f64 Y1;
        f64 Z1;
        f64 W1;
    };

    struct {
        f64 R0;
        f64 G0;
        f64 B0;
        f64 A0;
        f64 R1;
        f64 G1;
        f64 B1;
        f64 A1;
    };

    v4f64 V[2];
};

struct r1u64Node {
    r1u64Node* Next;
    r1u64 V;
};

struct r1u64List {
    r1u64Node* Head;
    r1u64Node* Tail;
    u64 Count;
};

struct r1i64Node {
    r1i64Node* Next;
    r1i64 V;
};

struct r1i64List {
    r1i64Node* Head;
    r1i64Node* Tail;
    u64 Count;
};

// @functions__________________________________________________________________
f32 _INF32(void);

// NOTE: see win32/win32_math.asm
// NOTE: see linux/linux_math.asm
EXTERN_C_LINK_BEGIN
extern i16   Abs16(i16 Val);
extern i32   Abs32(i32 Val);
extern i32   FMAdd32(i32 ValA, i32 ValB, i32 ValC);
extern i64   Abs64(i64 Val);
extern u64   Pow64(u64 Val, u64 Exp);
extern u64   Max64(u64 ValA, u64 ValB);
extern i64   FMAdd64(i64 ValA, i64 ValB, i64 ValC);
extern u64   Round64NextPow2(u64 Val);

extern f32   AbsF32(f32 Val);
extern f32   FloorF32(f32 Val);
extern f32   CeilF32(f32 Val);
extern f32   RoundF32(f32 Val);
extern f32   FMAddF32(f32 ValA, f32 ValB, f32 ValC);
extern f32   PowF32(f32 Val, f32 Exp);
extern f32   ExpF32(f32 Val);
extern f32   LnF32(f32 Val);
extern f32   Log10F32(f32 Val);
extern f32   Log2F32(f32 Val);
extern f32   LerpF32(f32 T, f32 ValA, f32 ValB);
extern f32   SqrtF32(f32 Val);
extern f32   InvSqrtF32(f32 Val);
extern f32   SinF32(f32 Val);
extern f32   CosF32(f32 Val);
extern f32   TanF32(f32 Val);
extern void  SinCosF32(f32 Val, OUT f32* SinRes, OUT f32* CosRes);
extern f32   ArcSinF32(f32 Val);
extern f32   ArcCosF32(f32 Val);
extern f32   ArcTanF32(f32 Val);
extern f32   ArcTan2F32(f32 ValA, f32 ValB);

extern f32   SSEPowF32(f32 Val, f32 Exp);
extern f32   SSEExpF32(f32 Val);
extern f32   SSELnF32(f32 Val);
extern f32   SSELog10F32(f32 Val);
extern f32   SSELog2F32(f32 Val);
extern f32   SSEInvSqrtF32(f32 Val);
extern f32   SSESinF32(f32 Val);
extern f32   SSECosF32(f32 Val);
extern f32   SSETanF32(f32 Val);
extern f32   SSEArcSinF32(f32 Val);
extern f32   SSEArcCosF32(f32 Val);
extern f32   SSEArcTanF32(f32 Val);
extern f32   SSEArcTan2F32(f32 ValA, f32 ValB);

extern f32   AVXPowF32(f32 Val, f32 Exp);
extern f32   AVXExpF32(f32 Val);
extern f32   AVXLnF32(f32 Val);
extern f32   AVXLog10F32(f32 Val);
extern f32   AVXLog2F32(f32 Val);
extern f32   AVXInvSqrtF32(f32 Val);
extern f32   AVXSinF32(f32 Val);
extern f32   AVXCosF32(f32 Val);
extern f32   AVXTanF32(f32 Val);
extern f32   AVXArcSinF32(f32 Val);
extern f32   AVXArcCosF32(f32 Val);
extern f32   AVXArcTanF32(f32 Val);
extern f32   AVXArcTan2F32(f32 ValA, f32 ValB);

extern f64   AbsF64(f64 Val);
extern f64   FloorF64(f64 Val);
extern f64   CeilF64(f64 Val);
extern f64   RoundF64(f64 Val);
extern f64   FMAddF64(f64 ValA, f64 ValB, f64 ValC);
extern f64   PowF64(f64 Val, f64 Exp);
extern f64   ExpF64(f64 Val);
extern f64   LnF64(f64 Val);
extern f64   Log10F64(f64 Val);
extern f64   Log2F64(f64 Val);
extern f64   SqrtF64(f64 Val);
extern f64   InvSqrtF64(f64 Val);
extern f64   LerpF64(f64 T, f64 ValA, f64 ValB);
extern f64   SinF64(f64 Val);
extern f64   CosF64(f64 Val);
extern f64   TanF64(f64 Val);
extern void  SinCosF64(f64 Val, OUT f64* SinRes, OUT f64* CosRes);
extern f64   ArcSinF64(f64 Val);
extern f64   ArcCosF64(f64 Val);
extern f64   ArcTanF64(f64 Val);
extern f64   ArcTan2F64(f64 ValA, f64 ValB);

extern f64   SSEPowF64(f64 Val, f64 Exp);
extern f64   SSEExpF64(f64 Val);
extern f64   SSELnF64(f64 Val);
extern f64   SSELog10F64(f64 Val);
extern f64   SSELog2F64(f64 Val);
extern f64   SSEInvSqrtF64(f64 Val);
extern f64   SSESinF64(f64 Val);
extern f64   SSECosF64(f64 Val);
extern f64   SSETanF64(f64 Val);
extern f64   SSEArcSinF64(f64 Val);
extern f64   SSEArcCosF64(f64 Val);
extern f64   SSEArcTanF64(f64 Val);
extern f64   SSEArcTan2F64(f64 ValA, f64 ValB);

extern f64   AVXPowF64(f64 Val, f64 Exp);
extern f64   AVXExpF64(f64 Val);
extern f64   AVXLnF64(f64 Val);
extern f64   AVXLog10F64(f64 Val);
extern f64   AVXLog2F64(f64 Val);
extern f64   AVXInvSqrtF64(f64 Val);
extern f64   AVXSinF64(f64 Val);
extern f64   AVXCosF64(f64 Val);
extern f64   AVXTanF64(f64 Val);
extern f64   AVXArcSinF64(f64 Val);
extern f64   AVXArcCosF64(f64 Val);
extern f64   AVXArcTanF64(f64 Val);
extern f64   AVXArcTan2F64(f64 ValA, f64 ValB);

extern void  LerpV2I64(v2i64* Result, v2i64* VecA, v2i64* VecB, f32 T);
extern v2i32 LerpV2I32(v2i32 VecA, v2i32 VecB, f32 T);
extern v2i16 LerpV2I16(v2i16 VecA, v2i16 VecB, f32 T);
extern void  LerpV4I32(v4i32* Result, v4i32* VecA, v4i32* VecB, f32 T);
EXTERN_C_LINK_END

f32 Cbrt(f32 Val);
f32 FMod(f32 ValA, f32 ValB);
f32 Pow(f32 Val, f32 Exp);
f32 Exp(f32 Val);
f32 Ln(f32 Val);
f32 Log10(f32 Val);
f32 Log2(f32 Val);
f32 InvSqrt(f32 Val);
f32 Sin(f32 Val);
f32 Cos(f32 Val);
f32 Tan(f32 Val);
f32 ArcSin(f32 Val);
f32 ArcCos(f32 Val);
f32 ArcTan(f32 Val);
f32 ArcTan2(f32 ValA, f32 ValB);

f64 Cbrt(f64 Val);
f64 FMod(f64 ValA, f64 ValB);
f64 Pow(f64 Val, f64 Exp);
f64 Exp(f64 Val);
f64 Ln(f64 Val);
f64 Log10(f64 Val);
f64 Log2(f64 Val);
f64 InvSqrt(f64 Val);
f64 Sin(f64 Val);
f64 Cos(f64 Val);
f64 Tan(f64 Val);
f64 ArcSin(f64 Val);
f64 ArcCos(f64 Val);
f64 ArcTan(f64 Val);
f64 ArcTan2(f64 ValA, f64 ValB);

v2f32  Vec(f32 X, f32 Y);
f32    Dot(v2f32 VecA, v2f32 VecB);
f32    LengthSq(v2f32 Vec);
f32    Length(v2f32 Vec);
v2f32  Norm(v2f32 Vec);
v2f32  Lerp(v2f32 VecA, v2f32 VecB, f32 T);
v2f32  operator+(v2f32 VecA, v2f32 VecB);
v2f32& operator+=(v2f32& VecA, v2f32 VecB);
v2f32  operator-(v2f32 VecA, v2f32 VecB);
v2f32& operator-=(v2f32& VecA, v2f32 VecB);
v2f32  operator*(v2f32 VecA, v2f32 VecB);
v2f32  operator*(v2f32 Vec, f32 Scale);
v2f32& operator*=(v2f32& VecA, v2f32 VecB);
v2f32& operator*=(v2f32& VecA, f32 Scale);
v2f32  operator/(v2f32 VecA, v2f32 VecB);
v2f32& operator/=(v2f32& VecA, v2f32 VecB);

v2f64  Vec(f64 X, f64 Y);
f64    Dot(v2f64 VecA, v2f64 VecB);
f64    LengthSq(v2f64 Vec);
f64    Length(v2f64 Vec);
v2f64  Norm(v2f64 Vec);
v2f64  Lerp(v2f64 VecA, v2f64 VecB, f64 T);
v2f64  operator+(v2f64 VecA, v2f64 VecB);
v2f64& operator+=(v2f64& VecA, v2f64 VecB);
v2f64  operator-(v2f64 VecA, v2f64 VecB);
v2f64& operator-=(v2f64& VecA, v2f64 VecB);
v2f64  operator*(v2f64 VecA, v2f64 VecB);
v2f64  operator*(v2f64 Vec, f64 Scale);
v2f64& operator*=(v2f64& VecA, v2f64 VecB);
v2f64& operator*=(v2f64& VecA, f64 Scale);
v2f64  operator/(v2f64 VecA, v2f64 VecB);
v2f64& operator/=(v2f64& VecA, v2f64 VecB);

v2i64 Vec(i64 X, i64 Y);
i64   Dot(v2i64 VecA, v2i64 VecB);
i64   LengthSq(v2i64 Vec);
i64   Length(v2i64 Vec);
v2i64 Norm(v2i64 Vec);
v2i64 Lerp(v2i64 VecA, v2i64 VecB, f32 T);
v2i64 operator+(v2i64 VecA, v2i64 VecB);
v2i64 operator-(v2i64 VecA, v2i64 VecB);
v2i64 operator*(v2i64 VecA, v2i64 VecB);
v2i64 operator*(v2i64 Vec, i64 Scale);
v2i64 operator/(v2i64 VecA, v2i64 VecB);

v2i32 Vec(i32 X, i32 Y);
i32   Dot(v2i32 VecA, v2i32 VecB);
i32   LengthSq(v2i32 Vec);
i32   Length(v2i32 Vec);
v2i32 Norm(v2i32 Vec);
v2i32 Lerp(v2i32 VecA, v2i32 VecB, f32 T);
v2i32 operator+(v2i32 VecA, v2i32 VecB);
v2i32 operator-(v2i32 VecA, v2i32 VecB);
v2i32 operator*(v2i32 VecA, v2i32 VecB);
v2i32 operator*(v2i32 Vec, i32 Scale);
v2i32 operator/(v2i32 VecA, v2i32 VecB);

v2i16  Vec(i16 X, i16 Y);
i16    Dot(v2i16 VecA, v2i16 VecB);
i16    LengthSq(v2i16 Vec);
i16    Length(v2i16 Vec);
v2i16  Norm(v2i16 Vec);
v2i16  Lerp(v2i16 VecA, v2i16 VecB, f32 T);
v2i16  operator+(v2i16 VecA, v2i16 VecB);
v2i16& operator+=(v2i16& VecA, v2i16 VecB);
v2i16  operator-(v2i16 VecA, v2i16 VecB);
v2i16& operator-=(v2i16& VecA, v2i16 VecB);
v2i16  operator*(v2i16 VecA, v2i16 VecB);
v2i16  operator*(v2i16 Vec, i16 Scale);
v2i16& operator*=(v2i16& VecA, v2i16 VecB);
v2i16& operator*=(v2i16& VecA, i16 Scale);
v2i16  operator/(v2i16 VecA, v2i16 VecB);
v2i16& operator/=(v2i16& VecA, v2i16 VecB);

v3f32 Vec(f32 X, f32 Y, f32 Z);
f32   Dot(v3f32 VecA, v3f32 VecB);
f32   LengthSq(v3f32 Vec);
f32   Length(v3f32 Vec);
v3f32 Norm(v3f32 Vec);
v3f32 Lerp(v3f32 VecA, v3f32 VecB, f32 T);
v3f32 Cross(v3f32 VecA, v3f32 VecB);
v3f32 operator+(v3f32 VecA, v3f32 VecB);
v3f32 operator-(v3f32 VecA, v3f32 VecB);
v3f32 operator*(v3f32 VecA, v3f32 VecB);
v3f32 operator*(v3f32 Vec, f32 Scale);
v3f32 operator/(v3f32 VecA, v3f32 VecB);
b32 operator<(v3f32 VecA, v3f32 VecB);
b32 operator>(v3f32 VecA, v3f32 VecB);
v3f32& operator+=(v3f32& VecA, v3f32 VecB);

v3f64 Vec(f64 X, f64 Y, f64 Z);
f64   Dot(v3f64 VecA, v3f64 VecB);
f64   LengthSq(v3f64 Vec);
f64   Length(v3f64 Vec);
v3f64 Norm(v3f64 Vec);
v3f64 Lerp(v3f64 VecA, v3f64 VecB, f64 T);
v3f64 Cross(v3f64 VecA, v3f64 VecB);
v3f64 operator+(v3f64 VecA, v3f64 VecB);
v3f64 operator-(v3f64 VecA, v3f64 VecB);
v3f64 operator*(v3f64 VecA, v3f64 VecB);
v3f64 operator*(v3f64 Vec, f64 Scale);
v3f64 operator/(v3f64 VecA, v3f64 VecB);

v3i32 Vec(i32 X, i32 Y, i32 Z);
i32   Dot(v3i32 VecA, v3i32 VecB);
i32   LengthSq(v3i32 Vec);
i32   Length(v3i32 Vec);
v3i32 Norm(v3i32 Vec);
v3i32 Lerp(v3i32 VecA, v3i32 VecB, f32 T);
v3i32 Cross(v3i32 VecA, v3i32 VecB);
v3i32 operator+(v3i32 VecA, v3i32 VecB);
v3i32 operator-(v3i32 VecA, v3i32 VecB);
v3i32 operator*(v3i32 VecA, v3i32 VecB);
v3i32 operator*(v3i32 Vec, i32 Scale);
v3i32 operator/(v3i32 VecA, v3i32 VecB);

v4f32 Vec(f32 X, f32 Y, f32 Z, f32 W);
f32   Dot(v4f32 VecA, v4f32 VecB);
f32   LengthSq(v4f32 Vec);
f32   Length(v4f32 Vec);
v4f32 Norm(v4f32 Vec);
v4f32 Lerp(v4f32 VecA, v4f32 VecB, f32 T);
v4f32 operator+(v4f32 VecA, v4f32 VecB);
v4f32 operator-(v4f32 VecA, v4f32 VecB);
v4f32 operator*(v4f32 VecA, v4f32 VecB);
v4f32 operator*(v4f32 Vec, f32 Scale);
v4f32 operator/(v4f32 VecA, v4f32 VecB);
v4f32 operator/(v4f32 Vec, f32 Value);

v4f64 Vec(f64 X, f64 Y, f64 Z, f64 W);
f64   Dot(v4f64 VecA, v4f64 VecB);
f64   LengthSq(v4f64 Vec);
f64   Length(v4f64 Vec);
v4f64 Norm(v4f64 Vec);
v4f64 Lerp(v4f64 VecA, v4f64 VecB, f64 T);
v4f64 operator+(v4f64 VecA, v4f64 VecB);
v4f64 operator-(v4f64 VecA, v4f64 VecB);
v4f64 operator*(v4f64 VecA, v4f64 VecB);
v4f64 operator*(v4f64 Vec, f64 Scale);
v4f64 operator/(v4f64 VecA, v4f64 VecB);

v4i32 Vec(i32 X, i32 Y, i32 Z, i32 W);
i32   Dot(v4i32 VecA, v4i32 VecB);
i32   LengthSq(v4i32 Vec);
i32   Length(v4i32 Vec);
v4i32 Norm(v4i32 Vec);
v4i32 Lerp(v4i32 VecA, v4i32 VecB, f32 T);
v4i32 operator+(v4i32 VecA, v4i32 VecB);
v4i32 operator-(v4i32 VecA, v4i32 VecB);
v4i32 operator*(v4i32 VecA, v4i32 VecB);
v4i32 operator*(v4i32 Vec, i32 Scale);
v4i32 operator/(v4i32 VecA, v4i32 VecB);

m3f32 Mat3(f32 Diag);
m3f32 Translation(v2f32 Vec);
m3f32 Scaler(v2f32 Vec);
m3f32 operator*(m3f32 MatA, m3f32 MatB);

m3f64 Mat3(f64 Diag);
m3f64 Translation(v2f64 Vec);
m3f64 Scaler(v2f64 Vec);
m3f64 operator*(m3f64 MatA, m3f64 MatB);

m4f32 Mat4(f32 Diag);
m4f32 Translation(v3f32 Vec);
m4f32 Scaler(v3f32 Vec);
m4f32 Perspective(f32 FOV, f32 AspectRatio, f32 NearZ, f32 FarZ);
m4f32 Orthographic(f32 Left, f32 Right, f32 Bottom, f32 Top, f32 NearZ, f32 FarZ);
m4f32 LookAt(v3f32 Eye, v3f32 Centre, v3f32 Up);
m4f32 Rotation(v3f32 Axis, f32 Turns);
m4f32 Inverse(m4f32 Mat);
m4f32 DeRotate(m4f32 Mat);
m4f32 Transpose(m4f32 Mat);
m4f32 operator*(m4f32 MatA, m4f32 MatB);
v4f32 operator*(m4f32 Mat, v4f32 Vector);
m4f32 operator*(m4f32 Mat, f32 Scale);

m4f64 Mat4(f64 Diag);
m4f64 Translation(v3f64 Vec);
m4f64 Scaler(v3f64 Vec);
m4f64 Perspective(f64 FOV, f64 AspectRatio, f64 NearZ, f64 FarZ);
m4f64 Orthographic(f64 Left, f64 Right, f64 Bottom, f64 Top, f64 NearZ, f64 FarZ);
m4f64 LookAt(v3f64 Eye, v3f64 Centre, v3f64 Up);
m4f64 Rotation(v3f64 Axis, f64 Turns);
m4f64 Inverse(m4f64 Mat);
m4f64 DeRotate(m4f64 Mat);
m4f64 Transpose(m4f64 Mat);
m4f64 operator*(m4f64 MatA, m4f64 MatB);
m4f64 operator*(m4f64 Mat, f64 Scale);

r1u32 Rng(u32 Min, u32 Max);
r1u32 Shift(r1u32 Range, u32 Val);
r1u32 Widen(r1u32 Range, u32 Val);
u32   Centre(r1u32 Range);
b32   InRange(r1u32 Range, u32 Val);
u32   Length(r1u32 Range);
r1u32 Union(r1u32 RangeA, r1u32 RangeB);
r1u32 Intersect(r1u32 RangeA, r1u32 RangeB);
u32   Clamp(r1u32 Range, u32 Val);

r1i32 Rng(i32 Min, i32 Max);
r1i32 Shift(r1i32 Range, i32 Val);
r1i32 Widen(r1i32 Range, i32 Val);
i32   Centre(r1i32 Range);
b32   InRange(r1i32 Range, i32 Val);
i32   Length(r1i32 Range);
r1i32 Union(r1i32 RangeA, r1i32 RangeB);
r1i32 Intersect(r1i32 RangeA, r1i32 RangeB);
i32   Clamp(r1i32 Range, i32 Val);

r1u64 Rng(u64 Min, u64 Max);
r1u64 Shift(r1u64 Range, u64 Val);
r1u64 Widen(r1u64 Range, u64 Val);
u64   Centre(r1u64 Range);
b32   InRange(r1u64 Range, u64 Val);
u64   Length(r1u64 Range);
r1u64 Union(r1u64 RangeA, r1u64 RangeB);
r1u64 Intersect(r1u64 RangeA, r1u64 RangeB);
u64   Clamp(r1u64 Range, u64 Val);

r1i64 Rng(i64 Min, i64 Max);
r1i64 Shift(r1i64 Range, i64 Val);
r1i64 Widen(r1i64 Range, i64 Val);
i64   Centre(r1i64 Range);
b32   InRange(r1i64 Range, i64 Val);
i64   Length(r1i64 Range);
r1i64 Union(r1i64 RangeA, r1i64 RangeB);
r1i64 Intersect(r1i64 RangeA, r1i64 RangeB);
i64   Clamp(r1i64 Range, i64 Val);

r1f32 Rng(f32 Min, f32 Max);
r1f32 Shift(r1f32 Range, f32 Val);
r1f32 Widen(r1f32 Range, f32 Val);
f32   Centre(r1f32 Range);
b32   InRange(r1f32 Range, f32 Val);
f32   Length(r1f32 Range);
r1f32 Union(r1f32 RangeA, r1f32 RangeB);
r1f32 Intersect(r1f32 RangeA, r1f32 RangeB);
f32   Clamp(r1f32 Range, f32 Val);

r1f64 Rng(f64 Min, f64 Max);
r1f64 Shift(r1f64 Range, f64 Val);
r1f64 Widen(r1f64 Range, f64 Val);
f64   Centre(r1f64 Range);
b32   InRange(r1f64 Range, f64 Val);
f64   Length(r1f64 Range);
r1f64 Union(r1f64 RangeA, r1f64 RangeB);
r1f64 Intersect(r1f64 RangeA, r1f64 RangeB);
f64   Clamp(r1f64 Range, f64 Val);

r2i16 Rng(v2i16 Min, v2i16 Max);
r2i16 Rng(i16 MinX, i16 MinY, i16 MaxX, i16 MaxY);
r2i16 Shift(r2i16 Range, v2i16 Vec);
r2i16 Widen(r2i16 Range, v2i16 Vec);
v2i16 Centre(r2i16 Range);
b32   InRange(r2i16 Range, v2i16 Vec);
v2i16 Length(r2i16 Range);
r2i16 Union(r2i16 RangeA, r2i16 RangeB);
r2i16 Intersect(r2i16 RangeA, r2i16 RangeB);
v2i16 Clamp(r2i16 Range, v2i16 Vec);

r2i32 Rng(v2i32 Min, v2i32 Max);
r2i32 Rng(i32 MinX, i32 MinY, i32 MaxX, i32 MaxY);
r2i32 Shift(r2i32 Range, v2i32 Vec);
r2i32 Widen(r2i32 Range, v2i32 Vec);
v2i32 Centre(r2i32 Range);
b32   InRange(r2i32 Range, v2i32 Vec);
v2i32 Length(r2i32 Range);
r2i32 Union(r2i32 RangeA, r2i32 RangeB);
r2i32 Intersect(r2i32 RangeA, r2i32 RangeB);
v2i32 Clamp(r2i32 Range, v2i32 Vec);

r2i64 Rng(v2i64 Min, v2i64 Max);
r2i64 Rng(i64 MinX, i64 MinY, i64 MaxX, i64 MaxY);
r2i64 Shift(r2i64 Range, v2i64 Vec);
r2i64 Widen(r2i64 Range, v2i64 Vec);
v2i64 Centre(r2i64 Range);
b32   InRange(r2i64 Range, v2i64 Vec);
v2i64 Length(r2i64 Range);
r2i64 Union(r2i64 RangeA, r2i64 RangeB);
r2i64 Intersect(r2i64 RangeA, r2i64 RangeB);
v2i64 Clamp(r2i64 Range, v2i64 Vec);

r2f32 Rng(v2f32 Min, v2f32 Max);
r2f32 Rng(f32 MinX, f32 MinY, f32 MaxX, f32 MaxY);
r2f32 Shift(r2f32 Range, v2f32 Vec);
r2f32 Widen(r2f32 Range, v2f32 Vec);
r2f32 Widen(r2f32 Range, f32 Value);
v2f32 Centre(r2f32 Range);
b32   InRange(r2f32 Range, v2f32 Vec);
v2f32 Length(r2f32 Range);
r2f32 Union(r2f32 RangeA, r2f32 RangeB);
r2f32 Intersect(r2f32 RangeA, r2f32 RangeB);
v2f32 Clamp(r2f32 Range, v2f32 Vec);

r2f64 Rng(v2f64 Min, v2f64 Max);
r2f64 Rng(f64 MinX, f64 MinY, f64 MaxX, f64 MaxY);
r2f64 Shift(r2f64 Range, v2f64 Vec);
r2f64 Widen(r2f64 Range, v2f64 Vec);
r2f64 Widen(r2f64 Range, f64 Value);
v2f64 Centre(r2f64 Range);
b32   InRange(r2f64 Range, v2f64 Vec);
v2f64 Length(r2f64 Range);
r2f64 Union(r2f64 RangeA, r2f64 RangeB);
r2f64 Intersect(r2f64 RangeA, r2f64 RangeB);
v2f64 Clamp(r2f64 Range, v2f64 Vec);

r3f32 Rng(v3f32 Min, v3f32 Max);
r3f32 Rng(f32 MinX, f32 MinY, f32 MinZ, f32 MaxX, f32 MaxY, f32 MaxZ);
r3f32 Shift(r3f32 Range, v3f32 Vec);
r3f32 Widen(r3f32 Range, v3f32 Vec);
r3f32 Widen(r3f32 Range, f32 Value);
v3f32 Centre(r3f32 Range);
b32   InRange(r3f32 Range, v3f32 Vec);
v3f32 Length(r3f32 Range);
r3f32 Union(r3f32 RangeA, r3f32 RangeB);
r3f32 Intersect(r3f32 RangeA, r3f32 RangeB);
v3f32 Clamp(r3f32 Range, v3f32 Vec);
r3f32 Transform(r3f32 Range, m4f32 Mat);

v3f32 HSVFromRGB(v3f32 Colour);
v3f32 RGBFromHSV(v3f32 Colour);
v4f32 HSVAFromRGBA(v4f32 Colour);
v4f32 RGBAFromHSVA(v4f32 Colour);
u32   U32FromRGBA(v4f32 Colour);
v4f32 RGBAFromU32(u32 Colour);
v3f32 LinearFromSRGB(v3f32 Colour);
v4f32 LinearFromSRGBA(v4f32 Colour);

void ListPush(Arena* MemPool, r1u64List* List, r1u64 Range);
void ListPush(Arena* MemPool, r1i64List* List, r1i64 Range);

INTERNAL u32 SafeTruncateU64(u64 Value);
INTERNAL u16 SafeTruncateU32(u32 Value);
INTERNAL u8 SafeTruncateU16(u16 Value);
INTERNAL i32 SafeTruncateI64(i64 Value);
INTERNAL i16 SafeTruncateI32(i32 Value);
INTERNAL i8 SafeTruncateI16(i16 Value);

#endif // __NC_MATH_H__
