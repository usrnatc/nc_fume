#include "nc_base64.h"
#include "nc_profile.h"
#include "nc_simd.h"
#include "nc_arena.h"

force_inline constexpr u64 
B64GetPaddingSize(Str8 String) {
    ASSERT(String.Size >= 4 && !(String.Size & 3));

    const u64 Size = String.Size;
    u64 Result = (
        (String.Str[Size - 1] == '=') + 
        (String.Str[Size - 2] == '=')
    );    

    return Result;
}

force_inline constexpr u64
B64GetOutputSize(const u64 EncodedSize, const u64 PaddingSize)
{
    ASSERT(!(EncodedSize & 3));

    return (EncodedSize >> 2) * 3 - PaddingSize;
}

#if NC_SIMD_LEVEL == 8
INTERNAL v512
B64RawFromEscaped(const v512 V)
{
    const v512 Mask = _mm512_set1_epi8(0x0F);
    const v512 HiBits = _mm512_and_si512(
        _mm512_srli_epi32(V, 4),
        Mask
    );
    const v512 LoBitsLUT = _mm512_broadcast_i32x4(
        _mm_setr_epi8(
            0,   0,  19,   4,
          -65, -65, -71, -71,
            0,   0,   0,   0,
            0,   0,   0,   0
        )
    );
    const v512 Offsets = _mm512_shuffle_epi8(LoBitsLUT, HiBits);
    const __mmask64 SlashMask = _mm512_cmpeq_epi8_mask(V, _mm512_set1_epi8('/'));
    const v512 CorrectedOffsets = _mm512_mask_blend_epi8(
        SlashMask,
        Offsets,
        _mm512_set1_epi8(16)
    );

    return _mm512_add_epi8(V, CorrectedOffsets);
}

INTERNAL void
__B64Decode(u8 const* Src, u64 SrcLength, u8* Dst, OUT u64* Consumed)
{
    const v512 PackShuffle = _mm512_broadcast_i32x4(
        _mm_setr_epi8(
             2,  1,  0, 
             6,  5,  4, 
            10,  9,  8, 
            14, 13, 12,
            -1, -1, -1, -1
        )
    );
    const v512 MergeMul1 = _mm512_set1_epi32(0x01400140);
    const v512 MergeMul2 = _mm512_set1_epi32(0x00011000);
    u64 Index = 0;

    for ( ; Index + 64 <= SrcLength; Index += 64) {
        v512 String = _mm512_loadu_si512((v512 const*) (Src + Index));
        v512 Values = B64RawFromEscaped(String);
        v512 Merged = _mm512_maddubs_epi16(Values, MergeMul1);

        Merged = _mm512_madd_epi16(Merged, MergeMul2);

        v512 Packed = _mm512_shuffle_epi8(Merged, PackShuffle);

        _mm_storeu_si128((__m128i*) Dst, _mm512_castsi512_si128(Packed));
        _mm_storeu_si128((__m128i*) (Dst + 12), _mm512_extracti32x4_epi32(Packed, 1));
        _mm_storeu_si128((__m128i*) (Dst + 24), _mm512_extracti32x4_epi32(Packed, 2));
        _mm_storeu_si128((__m128i*) (Dst + 36), _mm512_extracti32x4_epi32(Packed, 3));
        Dst += 48;
    }

    *Consumed = Index;
}
#elif NC_SIMD_LEVEL >= 6
    INTERNAL v256
    B64RawFromEscaped(const v256 V)
    {
        const v256 Mask = _mm256_set1_epi8(0x0F);
        const v256 HiBits = _mm256_and_si256(
            _mm256_srli_epi32(V, 4),
            Mask
        );
        const v256 LoBitsLUT = _mm256_setr_epi8(
            0,   0,  19,   4, -65, -65, -71, -71, 
            0,   0,   0,   0,   0,   0,   0,   0,
            0,   0,  19,   4, -65, -65, -71, -71, 
            0,   0,   0,   0,   0,   0,   0,   0
        );
        const v256 Offsets = _mm256_shuffle_epi8(LoBitsLUT, HiBits);
        const v256 SlashMask = _mm256_cmpeq_epi8(V, _mm256_set1_epi8('/'));
        const v256 CorrectedOffsets = _mm256_blendv_epi8(
            Offsets,
            _mm256_set1_epi8(16),
            SlashMask
        );

        return _mm256_add_epi8(V, CorrectedOffsets);
    }

    INTERNAL void
    __B64Decode(u8 const* Src, u64 SrcLength, u8* Dst, OUT u64* Consumed)
    {
        const v256 PackShuffle = _mm256_setr_epi8(
             2,  1,  0, 
             6,  5,  4, 
            10,  9,  8, 
            14, 13, 12,
            -1, -1, -1, -1,
             2,  1,  0, 
             6,  5,  4, 
            10,  9,  8, 
            14, 13, 12,
            -1, -1, -1, -1
        );
        const v256 MergeMul1 = _mm256_set1_epi32(0x01400140);
        const v256 MergeMul2 = _mm256_set1_epi32(0x00011000);
        u64 Index = 0;

        for ( ; Index + 32 <= SrcLength; Index += 32) {
            v256 String = _mm256_loadu_si256((v256 const*) (Src + Index));
            v256 Values = B64RawFromEscaped(String);
            v256 Merged = _mm256_maddubs_epi16(Values, MergeMul1);

            Merged = _mm256_madd_epi16(Merged, MergeMul2);

            v256 Packed = _mm256_shuffle_epi8(Merged, PackShuffle);

            _mm_storeu_si128((v128*) Dst, _mm256_castsi256_si128(Packed));
            _mm_storeu_si128((v128*) (Dst + 12), _mm256_extracti128_si256(Packed, 1));
            Dst += 24;
        }

        *Consumed = Index;
    }
#elif NC_SIMD_LEVEL >= 2
    INTERNAL v128
    B64RawFromEscaped(const v128 V)
    {
        const v128 Mask = _mm_set1_epi8(0x0F);
        const v128 HiBits = _mm_and_si128(
            _mm_srli_epi32(V, 4),
            Mask
        );
        const v128 LoBitsLUT = _mm_setr_epi8(
            0,   0,  19,   4, -65, -65, -71, -71, 
            0,   0,   0,   0,   0,   0,   0,   0
        );
        const v128 Offsets = _mm_shuffle_epi8(LoBitsLUT, HiBits);
        const v128 SlashMask = _mm_cmpeq_epi8(V, _mm_set1_epi8('/'));
        const v128 CorrectedOffsets = _mm_blendv_epi8(
            Offsets,
            _mm_set1_epi8(16),
            SlashMask
        );

        return _mm_add_epi8(V, CorrectedOffsets);
    }

    INTERNAL void
    __B64Decode(u8 const* Src, u64 SrcLength, u8* Dst, OUT u64* Consumed)
    {
        const v128 PackShuffle = _mm_setr_epi8(
             2,  1,  0, 
             6,  5,  4, 
            10,  9,  8, 
            14, 13, 12,
            -1, -1, -1, -1
        );
        const v128 MergeMul1 = _mm_set1_epi32(0x01400140);
        const v128 MergeMul2 = _mm_set1_epi32(0x00011000);
        u64 Index = 0;

        for ( ; Index + 16 <= SrcLength; Index += 16) {
            v128 String = _mm_loadu_si128((v128 const*) (Src + Index));
            v128 Values = B64RawFromEscaped(String);
            v128 Merged = _mm_maddubs_epi16(Values, MergeMul1);

            Merged = _mm_madd_epi16(Merged, MergeMul2);

            v128 Packed = _mm_shuffle_epi8(Merged, PackShuffle);

            _mm_storeu_si128((v128*) Dst, Packed);
            Dst += 12;
        }

        *Consumed = Index;
    }
#endif

global constexpr u8 B64_LUT[256] = {
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0, 62,  0,  0,  0, 63, 
    52, 53, 54, 55, 56, 57, 58, 59, 
    60, 61,  0,  0,  0,  0,  0,  0, 
     0,  0,  1,  2,  3,  4,  5,  6, 
     7,  8,  9, 10, 11, 12, 13, 14, 
    15, 16, 17, 18, 19, 20, 21, 22, 
    23, 24, 25,  0,  0,  0,  0,  0, 
     0, 26, 27, 28, 29, 30, 31, 32, 
    33, 34, 35, 36, 37, 38, 39, 40, 
    41, 42, 43, 44, 45, 46, 47, 48, 
    49, 50, 51,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0
};

INTERNAL void
B64Decode(u8 const* Src, u64 SrcLength, u8* Dst, u64 Padding)
{
    u8 const* End = Src + SrcLength;
    u64 FullChunks = (SrcLength >> 2);

    if (Padding)
        --FullChunks;

    for (u64 Index = 0; Index < FullChunks; ++Index) {
        u32 N0 = B64_LUT[Src[0]];
        u32 N1 = B64_LUT[Src[1]];
        u32 N2 = B64_LUT[Src[2]];
        u32 N3 = B64_LUT[Src[3]];
        u32 Bits = (N0 << 18) | (N1 << 12) | (N2 << 6) | N3;

        Dst[0] = (u8) (Bits >> 16);
        Dst[1] = (u8) (Bits >> 8);
        Dst[2] = (u8) (Bits);
        Src += 4;
        Dst += 3;
    }

    if (Padding) {
        u32 N0 = B64_LUT[Src[0]];
        u32 N1 = B64_LUT[Src[1]];
        u32 Bits = (N0 << 18) | (N1 << 12);

        if (Padding == 1) {
            u32 N2 = B64_LUT[Src[2]];

            Bits |= (N2 << 6);
            Dst[0] = (u8) (Bits >> 16);
            Dst[1] = (u8) (Bits >> 8);
        } else {
            Dst[0] = (u8) (Bits >> 16);
        }
    }
}

Str8 
B64Decode(Arena* MemPool, Str8 String)
{
    Str8 Result = {};
    const u64 EncodedSize = String.Size;

    if (!EncodedSize)
        return Result;

    const u64 PaddingSize = B64GetPaddingSize(String);
    const u64 ResultSize = B64GetOutputSize(EncodedSize, PaddingSize);
    u8* DecodedPtr = ArenaPushVArray(MemPool, u8, ResultSize + 32);
    u8* DstCursor = DecodedPtr;
    u8 const* SrcCursor = String.Str;
    u64 Processed = 0;

#if NC_SIMD_LEVEL == 8
    if (EncodedSize >= NC_SIMD_ALIGN512) {
        u64 Consumed = 0;

        __B64Decode(SrcCursor, EncodedSize, DstCursor, &Consumed);
        Processed += Consumed;
        SrcCursor += Consumed;
        DstCursor += (Consumed >> 2) * 3;
    }
#elif NC_SIMD_LEVEL >= 6
    if (EncodedSize >= NC_SIMD_ALIGN256) {
        u64 Consumed = 0;

        __B64Decode(SrcCursor, EncodedSize, DstCursor, &Consumed);
        Processed += Consumed;
        SrcCursor += Consumed;
        DstCursor += (Consumed >> 2) * 3;
    }
#elif NC_SIMD_LEVEL >= 2
    if (EncodedSize >= NC_SIMD_ALIGN128) {
        u64 Consumed = 0;

        __B64Decode(SrcCursor, EncodedSize, DstCursor, &Consumed);
        Processed += Consumed;
        SrcCursor += Consumed;
        DstCursor += (Consumed >> 2) * 3;
    }
#endif

    if (Processed < EncodedSize)
        B64Decode(SrcCursor, EncodedSize - Processed, DstCursor, PaddingSize);

    Result = Str(DecodedPtr, ResultSize);

    return Result;
}
