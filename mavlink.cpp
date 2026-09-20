#include "mavlink.h"

u16 MAVLINK_CRC_TABLE[MAVLINK_CRC_SLICE_COUNT][256] = {};
u8 MAVLINK_CRC_REVERSE[256] = {};
u16 MAVLINK_MSG_SLOTS[MAVLINK_MSG_SLOTS_COUNT] = {};

#define X(ID, EXTRA, NAME) ID,

const u32 MAVLINK_MSG_IDS[MAVLINK_MSG_COUNT] = {
    MAVLINK_MSG_XLIST
};

#undef X

#define X(ID, EXTRA, NAME) EXTRA,

const u8 MAVLINK_MSG_EXTRAS[MAVLINK_MSG_COUNT] = {
    MAVLINK_MSG_XLIST
};

#undef X

#define X(ID, EXTRA, NAME) Str8Lit(STRINGIFY(NAME)),

const Str8 MAVLINK_MSG_NAMES[MAVLINK_MSG_COUNT] = {
    MAVLINK_MSG_XLIST
};

#undef X

const Str8 MAVLINK_FIELD_NAMES[MAVLINK_FIELD_COUNT] = {
    Str8Lit("the incompatibility flags"),
    Str8Lit("the compatibility flags"),
    Str8Lit("the sequence"),
    Str8Lit("the system ID"),
    Str8Lit("the component ID"),
    Str8Lit("the message ID"),
    Str8Lit("the payload"),
    Str8Lit("none")
};

void 
MAVLinkInit(void)
{
    for (u32 Index = 0; Index < 256; ++Index) {
        u16 CRC = (u16) Index;

        for (u32 Bit = 0; Bit < 8; ++Bit) {
            CRC = (u16) ((CRC >> 1) ^ ((CRC & 1) ? MAVLINK_CRC_POLYNOMIAL : 0));
        }

        MAVLINK_CRC_TABLE[0][Index] = CRC;
        MAVLINK_CRC_REVERSE[CRC >> 8] = (u8) Index;
    }

    for (u32 Slice = 1; Slice < MAVLINK_CRC_SLICE_COUNT; ++Slice) {
        for (u32 Index = 0; Index < 256; ++Index) {
            u16 Prev = MAVLINK_CRC_TABLE[Slice - 1][Index];

            MAVLINK_CRC_TABLE[Slice][Index] = (u16) (
                (Prev >> 8) ^
                 MAVLINK_CRC_TABLE[0][Prev & U8_MAX]
            );
        }
    }

    for (u32 Index = 0; Index < MAVLINK_MSG_SLOTS_COUNT; ++Index)
        MAVLINK_MSG_SLOTS[Index] = MAVLINK_MSG_SLOT_NONE;

    for (u32 Slot = 0; Slot < MAVLINK_MSG_COUNT; ++Slot) {
        if (MAVLINK_MSG_IDS[Slot] < MAVLINK_MSG_SLOTS_COUNT)
            MAVLINK_MSG_SLOTS[MAVLINK_MSG_IDS[Slot]] = (u16) Slot;
    }
}

INTERNAL u32 
MAVLinkFrameSizeFromPtr(u8* Ptr)
{
    b32 IsVersion2 = *Ptr == MAVLINK_MAGIC_VERSION_2;
    b32 IsSigned = Ptr[2] & IsVersion2 & MAVLINK_IFLAG_SIGNED;
    u32 Result = (
        MAVLINK_HEADER_SIZE_VERSION_1 +
        (IsVersion2 << 2) +
        Ptr[1] +
        MAVLINK_CRC_SIZE +
        (IsSigned * MAVLINK_SIGNATURE_SIZE)
    );

    return Result;
}

INTERNAL MAVLinkFrame 
MAVLinkFrameFromPtr(u8* Ptr)
{
    MAVLinkFrame Result = {};
    b32 IsVersion2 = *Ptr == MAVLINK_MAGIC_VERSION_2;
    b32 IsSigned = Ptr[2] & IsVersion2 & MAVLINK_IFLAG_SIGNED;
    u32 Shift = (IsVersion2 << 1);

    Result.Ptr = Ptr;
    Result.Flag = (
        (IsVersion2 * MAVLINK_FRAME_FLAG_IS_VERSION_2) |
        (IsSigned * MAVLINK_FRAME_FLAG_IS_SIGNED)
    );
    Result.PayloadSize = Ptr[1];
    Result.Seq = Ptr[2 + Shift];
    Result.SysID = Ptr[3 + Shift];
    Result.CompID = Ptr[4 + Shift];
    Result.MsgID = IsVersion2
        ? ((*(u32*) (Ptr + 7)) & 0x00FFFFFF)
        : Ptr[5];
    Result.MsgSlot = (Result.MsgID < MAVLINK_MSG_SLOTS_COUNT)
        ? MAVLINK_MSG_SLOTS[Result.MsgID]
        : (u16) MAVLINK_MSG_SLOT_NONE;
    Result.CRCSize = (
        MAVLINK_HEADER_SIZE_VERSION_1 - 1 +
        (IsVersion2 << 2) +
        Ptr[1]
    );
    Result.Size = (
        1 +
        Result.CRCSize +
        MAVLINK_CRC_SIZE +
        (IsSigned * MAVLINK_SIGNATURE_SIZE)
    );

    return Result;
}

INTERNAL u16 
MAVLinkCRC(u8* Ptr, u64 Size)
{
    u32 Result = MAVLINK_CRC_SEED;
    u8* PtrEnd = Ptr + Size;

    for ( ; Ptr + 8 <= PtrEnd; Ptr += 8) {
        u64 Word = (*(u64*) Ptr) ^ Result;

        Result = (u32) (
            MAVLINK_CRC_TABLE[7][(Word >>  0) & U8_MAX] ^
            MAVLINK_CRC_TABLE[6][(Word >>  8) & U8_MAX] ^
            MAVLINK_CRC_TABLE[5][(Word >> 16) & U8_MAX] ^
            MAVLINK_CRC_TABLE[4][(Word >> 24) & U8_MAX] ^
            MAVLINK_CRC_TABLE[3][(Word >> 32) & U8_MAX] ^
            MAVLINK_CRC_TABLE[2][(Word >> 40) & U8_MAX] ^
            MAVLINK_CRC_TABLE[1][(Word >> 48) & U8_MAX] ^
            MAVLINK_CRC_TABLE[0][(Word >> 56) & U8_MAX]
        );
    }

    for ( ; Ptr < PtrEnd; ++Ptr) {
        Result = (Result >> 8) ^ MAVLINK_CRC_TABLE[0][(Result ^ *Ptr) & U8_MAX];
    }

    return (u16) Result;
}

INTERNAL u16 
MAVLinkExtraFromCRC(u16 CRC, u16 Target)
{
    u32 Mixed = (u32) (Target ^ (CRC >> 8));
    u32 High = Mixed >> 8;
    u32 __X = (High ^ (High >> 5)) & U8_MAX;
    u32 Low = ((__X << 3) ^ (__X >> 4)) & U8_MAX;
    u32 Extra = (__X ^ (__X << 4) ^ CRC) & U8_MAX;
    u16 Result = (Low == (Mixed & U8_MAX))
        ? (u16) Extra
        : (u16) MAVLINK_EXTRA_NONE;

    return Result;
}

INTERNAL MAVLinkFrameKind
MAVLinkFrameKindFromFrame(MAVLinkFrame* Frame, OUT u16* Extra)
{
    MAVLinkFrameKind Result = MAVLINK_FRAME_KIND_BAD_CRC;
    u8* CRCPtr = Frame->Ptr + 1 + Frame->CRCSize;
    u32 Target = (u32) (*CRCPtr | CRCPtr[1] << 8);
    u32 CRC = MAVLinkCRC(Frame->Ptr + 1, Frame->CRCSize);

    if (LIKELY(Frame->MsgSlot != MAVLINK_MSG_SLOT_NONE)) {
        *Extra = MAVLINK_MSG_EXTRAS[Frame->MsgSlot];
        CRC = (CRC >> 8) ^ MAVLINK_CRC_TABLE[0][(CRC ^ *Extra) & U8_MAX];

        if (CRC == Target)
            Result = MAVLINK_FRAME_KIND_OKAY;
    } else {
        *Extra = MAVLinkExtraFromCRC((u16) CRC, (u16) Target);

        if (*Extra != MAVLINK_EXTRA_NONE)
            Result = MAVLINK_FRAME_KIND_UNKNOWN_MSG;
    }

    return Result;
}

MAVLinkByteFix
MAVLinkByteFixFromFrame(MAVLinkFrame* Frame)
{
    MAVLinkByteFix Result = {};

    Result.Field = MAVLINK_FIELD_NONE;

    if (Frame->MsgSlot == MAVLINK_MSG_SLOT_NONE)
        return Result;

    b32 IsVersion2 = !!(Frame->Flag & MAVLINK_FRAME_FLAG_IS_VERSION_2);
    u32 SeqIndex = IsVersion2 ? 4 : 2;
    u32 PayloadIndex = IsVersion2 
        ? MAVLINK_HEADER_SIZE_VERSION_2 
        : MAVLINK_HEADER_SIZE_VERSION_1;
    u8* CRCPtr = Frame->Ptr + 1 + Frame->CRCSize;
    u32 Target = (u32) (*CRCPtr | CRCPtr[1] << 8);
    u32 CRC = MAVLinkCRC(Frame->Ptr + 1, Frame->CRCSize);

    CRC = (CRC >> 8) ^ MAVLINK_CRC_TABLE[0][(CRC ^ MAVLINK_MSG_EXTRAS[Frame->MsgSlot]) & U8_MAX];

    u32 Diff = CRC ^ Target;

    for (u32 Index = Frame->CRCSize; Index >= 2; --Index) {
        u32 Slot = MAVLINK_CRC_REVERSE[Diff >> 8];

        Diff = (((Diff ^ MAVLINK_CRC_TABLE[0][Slot]) & U8_MAX) << 8) | Slot;

        u32 Delta = MAVLINK_CRC_REVERSE[Diff >> 8];

        if (MAVLINK_CRC_TABLE[0][Delta] != Diff)
            continue;

        if (Index >= PayloadIndex) {
            Result.Field = MAVLINK_FIELD_PAYLOAD;
        } else if (Index >= SeqIndex + 3) {
            Result.Field = MAVLINK_FIELD_MSG_ID;
        } else if (Index >= SeqIndex) {
            Result.Field = (MAVLinkField) (
                MAVLINK_FIELD_SEQ + (Index - SeqIndex)
            );
        } else {
            Result.Field = (Index == 2) 
                ? MAVLINK_FIELD_INCOMPAT_FLAGS 
                : MAVLINK_FIELD_COMPAT_FLAGS;
        }

        Result.Original = (u8) (Frame->Ptr[Index] ^ Delta);
    }

    return Result;
}
