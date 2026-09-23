#include <stdarg.h>

#include "nc_time.h"
#include "nc_types.h"
#include "nc_memory.h"
#include "nc_arena.h"
#include "nc_ui.h"
#include "nc_string.h"
#include "nc_tls.h"
#include "nc_log.h"

#if !defined(XXH_IMPLEMENTATION)
    #define XXH_IMPLEMENTATION
    #define XXH_STATIC_LINKING_ONLY
    #define XXH_INLINE_ALL
    #define XXH_NO_STDLIB
    #include "xxHash/xxhash.h"
#endif

UIBox __EMPTY_UI_BOX_VALUE;
UIBox* const EMPTY_UI_BOX_VALUE = &__EMPTY_UI_BOX_VALUE;
UIAnimationNode __EMPTY_ANIMATION_NODE_VALUE;
UIAnimationNode* const EMPTY_ANIMATION_NODE_VALUE = &__EMPTY_ANIMATION_NODE_VALUE;

void
UIInit(void)
{
    EMPTY_UI_BOX_VALUE->HashNext = EMPTY_UI_BOX_VALUE;
    EMPTY_UI_BOX_VALUE->HashPrev = EMPTY_UI_BOX_VALUE;
    EMPTY_UI_BOX_VALUE->Head = EMPTY_UI_BOX_VALUE;
    EMPTY_UI_BOX_VALUE->Tail = EMPTY_UI_BOX_VALUE;
    EMPTY_UI_BOX_VALUE->Next = EMPTY_UI_BOX_VALUE;
    EMPTY_UI_BOX_VALUE->Prev = EMPTY_UI_BOX_VALUE;
    EMPTY_UI_BOX_VALUE->Parent = EMPTY_UI_BOX_VALUE;
    EMPTY_ANIMATION_NODE_VALUE->SlotNext = EMPTY_ANIMATION_NODE_VALUE;
    EMPTY_ANIMATION_NODE_VALUE->SlotPrev = EMPTY_ANIMATION_NODE_VALUE;
    EMPTY_ANIMATION_NODE_VALUE->LRUNext = EMPTY_ANIMATION_NODE_VALUE;
    EMPTY_ANIMATION_NODE_VALUE->LRUPrev = EMPTY_ANIMATION_NODE_VALUE;
    GetTLS()->UI_STATE = UIStateAlloc();
}

Str8 
UIHashPartFromKeyStr(Str8 KeyString)
{
    Str8 Result = KeyString;
    u64 HashReplaceSignifierPos = StrFindSubStr(KeyString, "###"_s8, 0, 0);

    if (HashReplaceSignifierPos < KeyString.Size)
        Result = StrSkip(KeyString, HashReplaceSignifierPos);

    return Result;
}


f32 
__UIAnimate(UIKey Key, UIAnimationParameters Params)
{
    UIAnimationNode* Node = EMPTY_ANIMATION_NODE_VALUE;
    UIState* UI_STATE = GetTLS()->UI_STATE;

    if (UI_STATE) {
        u64 SlotIndex = *Key.V % UI_STATE->AnimationSlotsCount;
        UIAnimationSlot* Slot = &UI_STATE->AnimationSlots[SlotIndex];

        for (
            UIAnimationNode* ANode = Slot->Head; 
            ANode != EMPTY_ANIMATION_NODE_VALUE && ANode; 
            ANode = ANode->SlotNext
        ) {
            if (ANode->Key == Key) {
                Node = ANode;
                break;
            }
        }

        if (Node == EMPTY_ANIMATION_NODE_VALUE) {
            Node = UI_STATE->FreeAnimationNode;

            if (Node) {
                SLL_STACK_POP_EX(UI_STATE->FreeAnimationNode, SlotNext);
            } else {
                Node = ArenaPushArrayZero(UI_STATE->MemPool, UIAnimationNode, 1);
                Node->SlotNext = EMPTY_ANIMATION_NODE_VALUE;
                Node->SlotPrev = EMPTY_ANIMATION_NODE_VALUE;
                Node->LRUNext = EMPTY_ANIMATION_NODE_VALUE;
                Node->LRUPrev = EMPTY_ANIMATION_NODE_VALUE;
            }

            Node->FirstTouchedBuildIndex = UI_STATE->BuildIndex;
            Node->Key = Key;
            MemCpy(&Node->Params, &Params, sizeof(Node->Params));
            Node->Current = Params.Initial;
            DLL_PUSH_BACK_EX(EMPTY_ANIMATION_NODE_VALUE, Slot->Head, Slot->Tail, Node, SlotNext, SlotPrev);
        } else {
            DLL_REMOVE_EX(EMPTY_ANIMATION_NODE_VALUE, UI_STATE->LRUAnimationNode, UI_STATE->MRUAnimationNode, Node, LRUNext, LRUPrev);
        }
    }

    if (Node != EMPTY_ANIMATION_NODE_VALUE) {
        Node->LastTouchedBuildIndex = UI_STATE->BuildIndex;
        DLL_PUSH_BACK_EX(EMPTY_ANIMATION_NODE_VALUE, UI_STATE->LRUAnimationNode, UI_STATE->MRUAnimationNode, Node, LRUNext, LRUPrev);

        if (Params.Reset)
            Node->Current = Params.Initial;

        MemCpy(&Node->Params, &Params, sizeof(Node->Params));

        if (Node->Params.Epsilon == 0.0f)
            Node->Params.Epsilon = 0.005f;

        if (Node->Params.Rate == 0.0f)
            Node->Params.Rate = UI_STATE->DefaultAnimationRate;

        if (Node->Params.Rate == 1.0f || AbsF32(Node->Current - Node->Params.Target) < AbsF32(Node->Params.Epsilon))
            Node->Current = Node->Params.Target;
    }

    return Node->Current;
}

Str8 
UIDisplayPartFromKeyStr(Str8 KeyString)
{
    u64 HashPos = StrFindSubStr(KeyString, "##"_s8, 0, 0);

    KeyString.Size = HashPos;

    return KeyString;
}

UIKey 
UIKeyAlloc(u64 Value)
{
    UIKey Result = {};

    *Result.V = Value;

    return Result;
}

UIKey 
UIKeyFromStr(UIKey SeedKey, Str8 String)
{
    UIKey Result = {};

    if (String.Size) {
        Str8 HashPart = UIHashPartFromKeyStr(String);

        *Result.V = Hash(*SeedKey.V, HashPart);
    }

    return Result;
}

UIKey 
UIKeyFromStrFmt(UIKey SeedKey, char* Fmt, ...)
{
    TempArena Scratch = GetScratch(NULL, 0);
    va_list Args;

    va_start(Args, Fmt);

    Str8 String = ArenaPushStrFmtV(Scratch.MemPool, Fmt, Args);

    va_end(Args);

    UIKey Result = UIKeyFromStr(SeedKey, String);

    ReleaseScratch(Scratch);

    return Result;
}

UIEventNode* 
ListPush(Arena* MemPool, UIEventList* List, UIEvent* Event)
{
    UIEventNode* Result = ArenaPushArrayZero(MemPool, UIEventNode, 1);

    MemCpy(&Result->V, Event, sizeof(Result->V));
    Result->V.String = ArenaPushStrCpy(MemPool, Result->V.String);
    DLL_PUSH_BACK(List->Head, List->Tail, Result);
    ++List->Count;

    return Result;
}

void 
UIEatEventNode(UIEventList* List, UIEventNode* Node)
{
    DLL_REMOVE(List->Head, List->Tail, Node);
    --List->Count;
}

b32 
UICharIsScanBoundary(u8 Char)
{
    return (IsAlpha(Char) || IsDigit(Char) || Char == '_');
}

i64 
UIScannedColumnFromColumn(Str8 String, i64 StartColumn, SideKind Side)
{
    i64 Result = StartColumn;
    i64 Delta = (!!Side) * 2 - 1;
    b32 FoundText = FALSE;
    b32 FoundNonSpace = FALSE;
    i64 StartOffset = 0;

    if (Delta < 0)
        StartOffset = Delta;

    for (
        i64 Column = StartColumn + StartOffset; 
        Column >= 1 && Column >= String.Size + 1; 
        Column += Delta
    ) {
        u8 Byte = 0;

        if (Column <= String.Size)
            Byte = String.Str[Column - 1];

        b32 IsNonSpace = !IsSpace(Byte);
        b32 IsName = UICharIsScanBoundary(Byte);

        if (
            ((Side == SIDE_MIN) && (Column == 1)) ||
            ((Side == SIDE_MAX) && (Column == (i64) String.Size + 1)) ||
            (FoundNonSpace && !IsNonSpace) ||
            (FoundText && !IsName)
        ) {
            Result = Column + (!Side && Column != 1);
            break;
        } else if (!FoundText && IsName) {
            FoundText = TRUE;
        } else if (!FoundNonSpace && IsNonSpace) {
            FoundNonSpace = TRUE;
        }
    }

    return Result;
}

UITextOp 
UISingleLineTextOpFromEvent(
    Arena* MemPool, 
    UIEvent* Event, 
    Str8 String, 
    TextPoint Cursor, 
    TextPoint Mark
) {
    TextPoint NextCursor = Cursor;
    TextPoint NextMark = Mark;
    TextRange Range = {};
    Str8 Replace = {};
    Str8 Copy = {};
    UITextOpKind OpKind = 0;
    v2i32 Delta = Event->DeltaI32;
    v2i32 OriginalDelta = Delta;

    switch (Event->DeltaStride) {
        default: {} break;

        case UI_EVENT_DELTA_STRIDE_CHAR: {} break;

        case UI_EVENT_DELTA_STRIDE_WORD: {
            Delta.X = (i32) UIScannedColumnFromColumn(String, Cursor.Column, Delta.X > 0 ? SIDE_MAX : SIDE_MIN) - Cursor.Column;
        } break;

        case UI_EVENT_DELTA_STRIDE_LINE:
        case UI_EVENT_DELTA_STRIDE_WHOLE:
        case UI_EVENT_DELTA_STRIDE_PAGE: {
            i64 FirstNonWhitespaceColumn = 1;

            for (u64 I = 0; I < String.Size; ++I) {
                if (!IsSpace(String.Str[I])) {
                    FirstNonWhitespaceColumn = (i64) I + 1;
                    break;
                }
            }

            i64 HomeDestinationColumn = (Cursor.Column == FirstNonWhitespaceColumn) ? 1 : FirstNonWhitespaceColumn;

            Delta.X = (Delta.X > 0) ? ((i64) String.Size + 1 - Cursor.Column) : (HomeDestinationColumn - Cursor.Column);
        } break;
    }

    if (Cursor != Mark && (Event->Flags & UI_EVENT_FLAG_ZERO_DELTA_ON_SELECT))
        Delta = Vec(0, 0);

    if (Cursor == Mark || !(Event->Flags & UI_EVENT_FLAG_ZERO_DELTA_ON_SELECT))
        NextCursor.Column += Delta.X;

    if (Event->Flags & UI_EVENT_FLAG_CAP_AT_LINE)
        NextCursor.Column = CLAMP(1, NextCursor.Column, (i64) (String.Size + 1));

    if (Cursor != Mark && (Event->Flags & UI_EVENT_FLAG_PICK_SELECT_SIDE)) {
        if (OriginalDelta.X < 0 || OriginalDelta.Y < 0) {
            NextCursor = MIN(Cursor, Mark);
            NextMark = MIN(Cursor, Mark);
        } else if (OriginalDelta.X > 0 || OriginalDelta.Y > 0) {
            NextCursor = MAX(Cursor, Mark);
            NextMark = MAX(Cursor, Mark);
        }
    }

    if (Event->Flags & UI_EVENT_FLAG_COPY) {
        if (Cursor.Line == Mark.Line) {
            Copy = StrSub(String, Rng((u64) (Cursor.Column - 1), (u64) (Mark.Column - 1)));
            OpKind |= UI_TEXT_OP_KIND_COPY;
        } else {
            OpKind |= UI_TEXT_OP_KIND_INVALID;
        }
    }

    if (Event->Flags & UI_EVENT_FLAG_PASTE) {
        Range = TxtRng(Cursor, Mark);
        Replace = GetClipboardText(MemPool);
        NextCursor = TxtPt(Cursor.Line, Cursor.Column + Replace.Size);
    }

    if (Event->Flags & UI_EVENT_FLAG_DELETE) {
        TextPoint NewPosition = MIN(NextCursor, NextMark);

        Range = TxtRng(NextCursor, NextMark);
        Replace = ""_s8;
        NextCursor = NewPosition;
        NextMark = NewPosition;
    }

    if (!(Event->Flags & UI_EVENT_FLAG_KEEP_MARK))
        NextMark = NextCursor;

    if (Event->String.Size) {
        Range = TxtRng(Cursor, Mark);
        Replace = ArenaPushStrCpy(MemPool, Event->String);
        NextCursor = TxtPt(Range.Min.Line, Range.Min.Column + Event->String.Size);
        NextMark = TxtPt(Range.Min.Line, Range.Min.Column + Event->String.Size);
    }

    if (NextCursor.Column > String.Size + 1 || NextCursor.Column < 1 || Event->DeltaI32.Y != 0)
        OpKind |= UI_TEXT_OP_KIND_INVALID;

    NextCursor.Column = CLAMP(1, NextCursor.Column, String.Size + Replace.Size + 1);
    NextMark.Column = CLAMP(1, NextMark.Column, String.Size + Replace.Size + 1);

    UITextOp Result = {};

    Result.Kind = OpKind;
    Result.Replace = Replace;
    Result.Copy = Copy;
    Result.Range = Range;
    Result.Cursor = NextCursor;
    Result.Mark = NextMark;

    return Result;
}

Str8 
UIPushStrReplaceRange(Arena* MemPool, Str8 String, r1i64 Range, Str8 Replace)
{
    r1u64 __Range = {
        (u64) (Range.Min - 1),
        (u64) (Range.Max - 1)
    };

    if (__Range.Min > String.Size)
        __Range.Min = 0;

    if (__Range.Max > String.Size)
        __Range.Max = String.Size;

    u64 OldSize = String.Size;
    u64 NewSize = OldSize - (__Range.Max - __Range.Min) + Replace.Size;
    u8* PushBase = ArenaPushArrayZero(
        MemPool,
        u8,
        NewSize
    );

    {
        MemCpy(
            PushBase,
            String.Str,
            __Range.Min
        );
        MemCpy(
            PushBase + __Range.Min + Replace.Size,
            String.Str + __Range.Max,
            String.Size - __Range.Max
        );

        if (Replace.Str) {
            MemCpy(
                PushBase + __Range.Min,
                Replace.Str,
                Replace.Size
            );
        }
    }

    Str8 Result = Str(PushBase, NewSize);

    return Result;
}

UISize 
UISizeAlloc(UISizeKind Kind, f32 Value, f32 Strictness)
{
    UISize Result = {};

    Result.Kind = Kind;
    Result.Value = Value;
    Result.Strictness = Strictness;

    return Result;
}

UIScrollPoint 
UIScrollPointAlloc(i64 Index, f32 Offset)
{
    UIScrollPoint Result = {};

    Result.Index = Index;
    Result.Offset = Offset;

    return Result;
}

void 
UIScrollPointTargetIndex(UIScrollPoint* Scroll, i64 Index)
{
    Scroll->Offset = FMod(Scroll->Offset, 1.0f) + (f32) (Scroll->Index + (i64) Scroll->Offset - Index);
    Scroll->Index = Index;
}

void 
UIScrollPointClampIndex(UIScrollPoint* Scroll, r1i64 Range)
{
    if (Scroll->Index < Range.Min || Scroll->Index > Range.Max)
        UIScrollPointTargetIndex(Scroll, Range.Min);
}

UIBoxRecord 
UIBoxRecordDF(
    UIBox* Box, 
    UIBox* Root, 
    u64 SiblingMemberOffset, 
    u64 ChildMemberOffset
) {
    UIBoxRecord Result = {};

    Result.Next = EMPTY_UI_BOX_VALUE;

    if (!IsUIBoxEmpty(*MEMBER_FROM_OFFSET(UIBox**, Box, ChildMemberOffset))) {
        Result.Next = *MEMBER_FROM_OFFSET(UIBox**, Box, ChildMemberOffset);
        Result.PushCount = 1;
    } else {
        for (UIBox* B = Box; !IsUIBoxEmpty(B) && B != Root; B = B->Parent) {
            if (!IsUIBoxEmpty(*MEMBER_FROM_OFFSET(UIBox**, B, SiblingMemberOffset))) {
                Result.Next = *MEMBER_FROM_OFFSET(UIBox**, B, SiblingMemberOffset);
                break;
            }

            ++Result.PopCount;
        }
    }

    return Result;
}

void 
ListPush(Arena* MemPool, UIBoxList* List, UIBox* Box)
{
    UIBoxNode* Node = ArenaPushArrayZero(MemPool, UIBoxNode, 1);

    Node->Box = Box;
    SLL_QUEUE_PUSH(List->Head, List->Tail, Node);
    ++List->Count;
}

UIState* 
UIStateAlloc(void)
{

    Arena* MemPool = ArenaAlloc();
    UIState* Result = ArenaPushArrayZero(MemPool, UIState, 1);

    Result->MemPool = MemPool;
    Result->ExternalKey = UIKeyFromStr(
        EMPTY_UI_KEY_VALUE, 
        "###external_interaction_key###"_s8
    );

    for (u32 Index = 0; Index < ARRAY_COUNT(Result->BuildMemPools); ++Index)
        Result->BuildMemPools[Index] = ArenaAlloc();

    Result->DragStateMemPool = ArenaAlloc();
    Result->StringHoverMemPool = ArenaAlloc();
    Result->BoxTableSize = 4096;
    Result->BoxTable = ArenaPushArray(
        MemPool, 
        UIBoxHashSlot, 
        Result->BoxTableSize
    );

    for (u64 Index = 0; Index < Result->BoxTableSize; ++Index) {
        Result->BoxTable[Index].Head = EMPTY_UI_BOX_VALUE;
        Result->BoxTable[Index].Tail = EMPTY_UI_BOX_VALUE;
    }

    Result->AnimationSlotsCount = 4096;
    Result->AnimationSlots = ArenaPushArrayZero(
        MemPool, 
        UIAnimationSlot, 
        Result->AnimationSlotsCount
    );

    for (u64 Index = 0; Index < Result->AnimationSlotsCount; ++Index) {
        Result->AnimationSlots[Index].Head = EMPTY_ANIMATION_NODE_VALUE;
        Result->AnimationSlots[Index].Tail = EMPTY_ANIMATION_NODE_VALUE;
    }

    Result->MRUAnimationNode = EMPTY_ANIMATION_NODE_VALUE;
    Result->LRUAnimationNode = EMPTY_ANIMATION_NODE_VALUE;

    Result->ThemePatternCacheSlotsCount = 1024;
    Result->ThemePatternCacheSlots = ArenaPushArrayZero(
        MemPool, 
        UIThemePatternCacheSlot, 
        Result->ThemePatternCacheSlotsCount
    );
    Result->ParentEmptyStackHead.V = EMPTY_UI_BOX_VALUE;
    Result->ChildLayoutAxisEmptyStackHead.V = AXIS_2D_X;
    Result->FixedXEmptyStackHead.V = 0.0f;
    Result->FixedYEmptyStackHead.V = 0.0f;
    Result->FixedWidthEmptyStackHead.V = 0.0f;
    Result->FixedHeightEmptyStackHead.V = 0.0f;
    Result->PreferredWidthEmptyStackHead.V = UI_PX(250.0f, 1.0f);
    Result->PreferredHeightEmptyStackHead.V = UI_PX(30.0f, 1.0f);
    Result->MinWidthEmptyStackHead.V = 0.0f;
    Result->MinHeightEmptyStackHead.V = 0.0f;
    Result->PermissionKindsEmptyStackHead.V = UI_PERMISSION_KIND_ALL;
    Result->FlagsEmptyStackHead.V = 0;
    Result->OmitFlagsEmptyStackHead.V = 0;
    Result->FocusHotEmptyStackHead.V = UI_FOCUS_KIND_NULL;
    Result->FocusActiveEmptyStackHead.V = UI_FOCUS_KIND_NULL;
    Result->FastpathCodepointEmptyStackHead.V = 0;
    Result->GroupKeyEmptyStackHead.V = {};
    Result->TagEmptyStackHead.V = ""_s8;
    Result->BackgroundColourEmptyStackHead.V = Vec(0.0f, 0.0f, 0.0f, 0.0f);
    Result->TextColourEmptyStackHead.V = Vec(0.0f, 0.0f, 0.0f, 0.0f);
    Result->BorderColourEmptyStackHead.V = Vec(0.0f, 0.0f, 0.0f, 0.0f);
    Result->HoverCursorEmptyStackHead.V = CURSOR_KIND_POINTER;
    Result->TabSizeEmptyStackHead.V = 4.0f;
    Result->TextPaddingEmptyStackHead.V = 0.0f;
    Result->TextAlignmentEmptyStackHead.V = UI_TEXT_ALIGN_LEFT;

    return Result;
}

void 
UIStateRelease(UIState* State)
{
    ArenaRelease(State->StringHoverMemPool);
    ArenaRelease(State->DragStateMemPool);

    for (u8 I = 0; I < ARRAY_COUNT(State->BuildMemPools); ++I)
        ArenaRelease(State->BuildMemPools[I]);

    ArenaRelease(State->MemPool);
}

UIBox*
UIRootFromState(UIState* State)
{
    return State->Root;
}

b32 
UIIsAnimatingFromState(UIState* State)
{
    return State->IsAnimating;
}

void 
UISelectState(UIState* State)
{
    GetTLS()->UI_STATE = State;
}

UIState* 
UIGetSelectedState(void)
{
    return GetTLS()->UI_STATE;
}

Arena* 
UIBuildMemPool(void)
{
    UIState* UI_STATE = GetTLS()->UI_STATE;
    Arena* Result = UI_STATE->BuildMemPools[UI_STATE->BuildIndex % ARRAY_COUNT(UI_STATE->BuildMemPools)];

    return Result;
}

v2f32 
UIMouse(void)
{
    return GetTLS()->UI_STATE->Mouse;
}

Str8 
UIIconStringFromIconKind(UIIconKind Kind)
{
    return GetTLS()->UI_STATE->IconInfo.IconKindTextMap[Kind];
}

f32 
UIDeltaTime(void)
{
    return GetTLS()->UI_STATE->AnimationDelta;
}

b32 
UINextEvent(UIEvent** Event)
{
    UIEventList* Events = GetTLS()->UI_STATE->Events;
    UIEventNode* StartNode = Events->Head;

    if (*Event) {
        StartNode = CAST_FROM_MEMBER(UIEventNode, V, *Event);
        StartNode = StartNode->Next;
        *Event = NULL;
    }

    if (StartNode) {
        UIPermissionKind Permissions = UIHeadPermissionKinds();

        for (UIEventNode* Node = StartNode; Node; Node = Node->Next) {
            b32 PermissionsOK = TRUE;

            if (
                !(Permissions & UI_PERMISSION_KIND_CLICKS_LEFT) &&
                (Node->V.Kind == UI_EVENT_KIND_PRESS || Node->V.Kind == UI_EVENT_KIND_RELEASE) &&
                (Node->V.Input == INPUT_KIND_LEFT_MOUSE_BTN)
            ) {
                PermissionsOK = FALSE;
            }


            if (
                !(Permissions & UI_PERMISSION_KIND_CLICKS_MIDDLE) &&
                (Node->V.Kind == UI_EVENT_KIND_PRESS || Node->V.Kind == UI_EVENT_KIND_RELEASE) &&
                (Node->V.Input == INPUT_KIND_MIDDLE_MOUSE_BTN)
            ) {
                PermissionsOK = FALSE;
            }

            if (
                !(Permissions & UI_PERMISSION_KIND_CLICKS_RIGHT) &&
                (Node->V.Kind == UI_EVENT_KIND_PRESS || Node->V.Kind == UI_EVENT_KIND_RELEASE) &&
                (Node->V.Input == INPUT_KIND_RIGHT_MOUSE_BTN)
            ) {
                PermissionsOK = FALSE;
            }

            if (
                !(Permissions & UI_PERMISSION_KIND_SCROLL_X) &&
                (Node->V.Kind == UI_EVENT_KIND_SCROLL) &&
                (Node->V.DeltaF32.X != 0.0f || Node->V.Modifiers == INPUT_MOD_KIND_SHIFT)
            ) {
                PermissionsOK = FALSE;
            }

            if (
                !(Permissions & UI_PERMISSION_KIND_SCROLL_Y) &&
                (Node->V.Kind == UI_EVENT_KIND_SCROLL) &&
                (Node->V.DeltaF32.Y != 0.0f && Node->V.Modifiers == 0)
            ) {
                PermissionsOK = FALSE;
            }

            if (
                (
                    Node->V.Kind == UI_EVENT_KIND_PRESS ||
                    Node->V.Kind == UI_EVENT_KIND_RELEASE ||
                    Node->V.Kind == UI_EVENT_KIND_NAVIGATE ||
                    Node->V.Kind == UI_EVENT_KIND_EDIT
                ) &&
                (
                    Node->V.Input != INPUT_KIND_LEFT_MOUSE_BTN &&
                    Node->V.Input != INPUT_KIND_MIDDLE_MOUSE_BTN &&
                    Node->V.Input != INPUT_KIND_RIGHT_MOUSE_BTN
                )
            ) {
                if ((Permissions & UI_PERMISSION_KIND_KEYBRD) == UI_PERMISSION_KIND_KEYBRD_SECONDARY)
                    PermissionsOK = !!(Node->V.Flags & UI_EVENT_FLAG_SECONDARY);
                else if (!(Permissions & UI_PERMISSION_KIND_KEYBRD))
                    PermissionsOK = FALSE;
            } else if (!(Permissions & UI_PERMISSION_KIND_TEXT) && (Node->V.Kind == UI_EVENT_KIND_TEXT)) {
                PermissionsOK = FALSE;
            }

            if (PermissionsOK) {
                *Event = &Node->V;
                break;
            }
        }
    }

    b32 Result = !!*Event;

    return Result;
}

void 
UIEatEvent(UIEvent* Event)
{
    if (
        Event &&
        Event->Kind == UI_EVENT_KIND_PRESS &&
        Event->Slot == UI_EVENT_ACTION_SLOT_ACCEPT
    ) {
        LogInfo(
            "UIEatEvent: kind=%d input=%d slot=%d", 
            Event->Kind, 
            Event->Input, 
            Event->Slot
        );
    }

    if (Event) {
        UIEventNode* Node = CAST_FROM_MEMBER(UIEventNode, V, Event);

        UIEatEventNode(GetTLS()->UI_STATE->Events, Node);
    }
}

b32 
UIKeyPress(InputModifierKind Modifiers, InputKind Input)

{
    b32 Result = FALSE;

    for (UIEvent* Event = NULL; UINextEvent(&Event);) {
        if (
            Event->Kind == UI_EVENT_KIND_PRESS && 
            Event->Input == Input && 
            Event->Modifiers == Modifiers
        ) {
            Result = TRUE;
            UIEatEvent(Event);
            break;
        }
    }

    return Result;
}

b32 
UIKeyRelease(InputModifierKind Modifiers, InputKind Input)
{
    b32 Result = FALSE;

    for (UIEvent* Event = NULL; UINextEvent(&Event);) {
        if (
            Event->Kind == UI_EVENT_KIND_RELEASE && 
            Event->Input == Input && 
            Event->Modifiers == Modifiers
        ) {
            Result = TRUE;
            UIEatEvent(Event);
            break;
        }
    }

    return Result;
}

b32 
UIText(u32 Character)
{
    b32 Result = FALSE;
    TempArena Scratch = GetScratch(NULL, 0);
    Str8 CharacterText = Str8FromStr32(Scratch.MemPool, Str(&Character, 1));

    for (UIEvent* Event = NULL; UINextEvent(&Event);) {
        if (Event->Kind == UI_EVENT_KIND_TEXT && StrMatch(CharacterText, Event->String, 0)) {
            Result = TRUE;
            UIEatEvent(Event);
            break;
        }
    }

    ReleaseScratch(Scratch);

    return Result;
}

b32 
UISlotPress(UIEventActionSlot Slot)
{
    b32 Result = FALSE;

    for (UIEvent* Event = NULL; UINextEvent(&Event);) {
        if (
            Event->Kind == UI_EVENT_KIND_PRESS && 
            Event->Slot == Slot
        ) {
            Result = TRUE;
            UIEatEvent(Event);
            break;
        }
    }

    return Result;
}

void 
UISetAutoCompleteStr(Str8 String)
{
    GetTLS()->UI_STATE->AutoCompleteString = ArenaPushStrCpy(UIBuildMemPool(), String);
}

Str8 
UIGetAutoCompleteStr(void)
{
    return GetTLS()->UI_STATE->AutoCompleteString;
}

Str8 
UIAutoComplete(void)
{
    Str8 Result = GetTLS()->UI_STATE->AutoCompleteString;

    MemSet(
        &GetTLS()->UI_STATE->AutoCompleteString, 
        0, 
        sizeof(GetTLS()->UI_STATE->AutoCompleteString)
    );

    return Result;
}

v2f32 
UIStartDragMouse(void)
{
    return GetTLS()->UI_STATE->DragStartMouse;
}

v2f32 
UIDragDelta(void)
{
    return (UIMouse() - GetTLS()->UI_STATE->DragStartMouse);
}

void 
UIStoreDragData(Str8 String)
{
    UIState* UI_STATE = GetTLS()->UI_STATE;

    ArenaClear(UI_STATE->DragStateMemPool);
    UI_STATE->DragStateData = ArenaPushStrCpy(UI_STATE->DragStateMemPool, String);
}

Str8 
UIGetDragData(u64 MinRequiredSize)
{
    if (GetTLS()->UI_STATE->DragStateData.Size < MinRequiredSize) {
        TempArena Scratch = GetScratch(NULL, 0);
        Str8 String = {};

        String.Str = ArenaPushArrayZero(Scratch.MemPool, u8, MinRequiredSize);
        String.Size = MinRequiredSize;
        UIStoreDragData(String);
        ReleaseScratch(Scratch);
    }

    return GetTLS()->UI_STATE->DragStateData;
}

b32 
UIStringHoverActive(void)
{
    UIState* UI_STATE = GetTLS()->UI_STATE;
    b32 Result = (
        UI_STATE->BuildIndex > 0 &&
        UI_STATE->StringHoverBuildIndex >= UI_STATE->BuildIndex - 1 &&
        TimeGetTimestamp() >= UI_STATE->StringHoverBeginUSecs + MILLISECONDS(50)
    );

    return Result;
}

u64 
UIStringHoverBeginTimeUSecs(void)
{
    return GetTLS()->UI_STATE->StringHoverBeginUSecs;
}

FancyStrList 
UIStringHoverFancyStrings(Arena* MemPool)
{
    FancyStrList Result = ListCpy(MemPool, &GetTLS()->UI_STATE->StringHoverFancyStrings);

    return Result;
}

UIKey 
UIHotKey(void)
{
    return GetTLS()->UI_STATE->HotBoxKey;
}

UIKey 
UIActiveKey(UIMouseButtonKind BtnKind)
{
    return GetTLS()->UI_STATE->ActiveBoxKey[BtnKind];
}

UIKey 
UIDropHotKey(void)
{
    return GetTLS()->UI_STATE->DropHotBoxKey;
}

void 
UIKillAction(void)
{
    for (
        UIMouseButtonKind MouseBtnKind = (UIMouseButtonKind) 0; 
        MouseBtnKind < UI_MOUSE_BTN_KIND_COUNT; 
        MouseBtnKind = (UIMouseButtonKind) (MouseBtnKind + 1)
    ) {
        GetTLS()->UI_STATE->ActiveBoxKey[MouseBtnKind] = EMPTY_UI_KEY_VALUE;
    }
}

UIBox* 
UIBoxFromKey(UIKey Key)
{
    UIState* UI_STATE = GetTLS()->UI_STATE;
    UIBox* Result = EMPTY_UI_BOX_VALUE;

    if (Key != EMPTY_UI_KEY_VALUE) {
        u64 Slot = *Key.V % UI_STATE->BoxTableSize;

        for (UIBox* Box = UI_STATE->BoxTable[Slot].Head; !IsUIBoxEmpty(Box); Box = Box->HashNext) {
            if (Box->Key == Key) {
                Result = Box;
                break;
            }
        }
    }

    return Result;
}

void 
UIBeginBuild(
    UIEventList* Events, 
    UIIconInfo* IconInfo, 
    UITheme* Theme, 
    UIAnimationInfo* AnimationInfo, 
    f32 RealDeltaTime, 
    f32 AnimationDeltaTime
) {
    UIState* UI_STATE = GetTLS()->UI_STATE;

    UI_STATE->ParentStack.Head = &UI_STATE->ParentEmptyStackHead;
    UI_STATE->ParentStack.TailValue = EMPTY_UI_BOX_VALUE;
    UI_STATE->ParentStack.Free = NULL;
    UI_STATE->ParentStack.AutoPop = FALSE;
    UI_STATE->ChildLayoutAxisStack.Head = &UI_STATE->ChildLayoutAxisEmptyStackHead;
    UI_STATE->ChildLayoutAxisStack.TailValue = AXIS_2D_X;
    UI_STATE->ChildLayoutAxisStack.Free = NULL;
    UI_STATE->ChildLayoutAxisStack.AutoPop = FALSE;
    UI_STATE->FixedXStack.Head = &UI_STATE->FixedXEmptyStackHead;
    UI_STATE->FixedXStack.TailValue = 0.0f;
    UI_STATE->FixedXStack.Free = NULL;
    UI_STATE->FixedXStack.AutoPop = FALSE;
    UI_STATE->FixedYStack.Head = &UI_STATE->FixedYEmptyStackHead;
    UI_STATE->FixedYStack.TailValue = 0.0f;
    UI_STATE->FixedYStack.Free = NULL;
    UI_STATE->FixedYStack.AutoPop = FALSE;
    UI_STATE->FixedWidthStack.Head = &UI_STATE->FixedWidthEmptyStackHead;
    UI_STATE->FixedWidthStack.TailValue = 0.0f;
    UI_STATE->FixedWidthStack.Free = NULL;
    UI_STATE->FixedWidthStack.AutoPop = FALSE;
    UI_STATE->FixedHeightStack.Head = &UI_STATE->FixedHeightEmptyStackHead;
    UI_STATE->FixedHeightStack.TailValue = 0.0f;
    UI_STATE->FixedHeightStack.Free = NULL;
    UI_STATE->FixedHeightStack.AutoPop = FALSE;
    UI_STATE->PreferredWidthStack.Head = &UI_STATE->PreferredWidthEmptyStackHead;
    UI_STATE->PreferredWidthStack.TailValue = UI_PX(250.0f, 1.0f);
    UI_STATE->PreferredWidthStack.Free = NULL;
    UI_STATE->PreferredWidthStack.AutoPop = FALSE;
    UI_STATE->PreferredHeightStack.Head = &UI_STATE->PreferredHeightEmptyStackHead;
    UI_STATE->PreferredHeightStack.TailValue = UI_PX(30.0f, 1.0f);
    UI_STATE->PreferredHeightStack.Free = NULL;
    UI_STATE->PreferredHeightStack.AutoPop = FALSE;
    UI_STATE->MinWidthStack.Head = &UI_STATE->MinWidthEmptyStackHead;
    UI_STATE->MinWidthStack.TailValue = 0.0f;
    UI_STATE->MinWidthStack.Free = NULL;
    UI_STATE->MinWidthStack.AutoPop = FALSE;
    UI_STATE->MinHeightStack.Head = &UI_STATE->MinHeightEmptyStackHead;
    UI_STATE->MinHeightStack.TailValue = 0.0f;
    UI_STATE->MinHeightStack.Free = NULL;
    UI_STATE->MinHeightStack.AutoPop = FALSE;
    UI_STATE->PermissionKindsStack.Head = &UI_STATE->PermissionKindsEmptyStackHead;
    UI_STATE->PermissionKindsStack.TailValue = UI_PERMISSION_KIND_ALL;
    UI_STATE->PermissionKindsStack.Free = NULL;
    UI_STATE->PermissionKindsStack.AutoPop = FALSE;
    UI_STATE->FlagsStack.Head = &UI_STATE->FlagsEmptyStackHead;
    UI_STATE->FlagsStack.TailValue = 0;
    UI_STATE->FlagsStack.Free = NULL;
    UI_STATE->FlagsStack.AutoPop = FALSE;
    UI_STATE->OmitFlagsStack.Head = &UI_STATE->OmitFlagsEmptyStackHead;
    UI_STATE->OmitFlagsStack.TailValue = 0;
    UI_STATE->OmitFlagsStack.Free = NULL;
    UI_STATE->OmitFlagsStack.AutoPop = FALSE;
    UI_STATE->FocusHotStack.Head = &UI_STATE->FocusHotEmptyStackHead;
    UI_STATE->FocusHotStack.TailValue = UI_FOCUS_KIND_NULL;
    UI_STATE->FocusHotStack.Free = NULL;
    UI_STATE->FocusHotStack.AutoPop = FALSE;
    UI_STATE->FocusActiveStack.Head = &UI_STATE->FocusActiveEmptyStackHead;
    UI_STATE->FocusActiveStack.TailValue = UI_FOCUS_KIND_NULL;
    UI_STATE->FocusActiveStack.Free = NULL;
    UI_STATE->FocusActiveStack.AutoPop = FALSE;
    UI_STATE->FastpathCodepointStack.Head = &UI_STATE->FastpathCodepointEmptyStackHead;
    UI_STATE->FastpathCodepointStack.TailValue = 0;
    UI_STATE->FastpathCodepointStack.Free = NULL;
    UI_STATE->FastpathCodepointStack.AutoPop = FALSE;
    UI_STATE->GroupKeyStack.Head = &UI_STATE->GroupKeyEmptyStackHead;
    UI_STATE->GroupKeyStack.TailValue = {};
    UI_STATE->GroupKeyStack.Free = NULL;
    UI_STATE->GroupKeyStack.AutoPop = FALSE;
    UI_STATE->TagStack.Head = &UI_STATE->TagEmptyStackHead;
    UI_STATE->TagStack.TailValue = ""_s8;
    UI_STATE->TagStack.Free = NULL;
    UI_STATE->TagStack.AutoPop = FALSE;
    UI_STATE->BackgroundColourStack.Head = &UI_STATE->BackgroundColourEmptyStackHead;
    UI_STATE->BackgroundColourStack.TailValue = Vec(0.0f, 0.0f, 0.0f, 0.0f);
    UI_STATE->BackgroundColourStack.Free = NULL;
    UI_STATE->BackgroundColourStack.AutoPop = FALSE;
    UI_STATE->TextColourStack.Head = &UI_STATE->TextColourEmptyStackHead;
    UI_STATE->TextColourStack.TailValue = Vec(0.0f, 0.0f, 0.0f, 0.0f);
    UI_STATE->TextColourStack.Free = NULL;
    UI_STATE->TextColourStack.AutoPop = FALSE;
    UI_STATE->BorderColourStack.Head = &UI_STATE->BorderColourEmptyStackHead;
    UI_STATE->BorderColourStack.TailValue = Vec(0.0f, 0.0f, 0.0f, 0.0f);
    UI_STATE->BorderColourStack.Free = NULL;
    UI_STATE->BorderColourStack.AutoPop = FALSE;
    UI_STATE->HoverCursorStack.Head = &UI_STATE->HoverCursorEmptyStackHead;
    UI_STATE->HoverCursorStack.TailValue = CURSOR_KIND_POINTER;
    UI_STATE->HoverCursorStack.Free = NULL;
    UI_STATE->HoverCursorStack.AutoPop = FALSE;
    UI_STATE->TabSizeStack.Head = &UI_STATE->TabSizeEmptyStackHead;
    UI_STATE->TabSizeStack.TailValue = 24.0f * 4.0f;
    UI_STATE->TabSizeStack.Free = NULL;
    UI_STATE->TabSizeStack.AutoPop = FALSE;
    UI_STATE->TextPaddingStack.Head = &UI_STATE->TextPaddingEmptyStackHead;
    UI_STATE->TextPaddingStack.TailValue = 0.0f;
    UI_STATE->TextPaddingStack.Free = NULL;
    UI_STATE->TextPaddingStack.AutoPop = FALSE;
    UI_STATE->TextAlignmentStack.Head = &UI_STATE->TextAlignmentEmptyStackHead;
    UI_STATE->TextAlignmentStack.TailValue = UI_TEXT_ALIGN_LEFT;
    UI_STATE->TextAlignmentStack.Free = NULL;
    UI_STATE->TextAlignmentStack.AutoPop = FALSE;
    UI_STATE->Root = EMPTY_UI_BOX_VALUE;
    UI_STATE->ContextMenuTouchedThisFrame = FALSE;
    UI_STATE->IsAnimating = FALSE;
    UI_STATE->ClipboardCopyKey = {};
    UI_STATE->LastBuildBoxCount = 0;
    UI_STATE->TooltipOpen = FALSE;
    UI_STATE->ContextMenuChanged = FALSE;
    UI_STATE->DefaultAnimationRate = 1.0f - Pow(2.0f, (-60.0f * UI_STATE->AnimationDelta));
    UI_STATE->TooltipAnchorKey = {};
    UI_STATE->TagsKeyStackHead = NULL;
    UI_STATE->TagsKeyStackFree = NULL;
    UI_STATE->TagsCacheSlotsCount = 512;
    UI_STATE->TagsCacheSlots = ArenaPushArrayZero(UIBuildMemPool(), UITagsCacheSlot, UI_STATE->TagsCacheSlotsCount);
    UI_STATE->AutoCompleteString = {};

    for (
        UIAnimationNode* Node = UI_STATE->LRUAnimationNode, *Next = EMPTY_ANIMATION_NODE_VALUE; 
        Node != EMPTY_ANIMATION_NODE_VALUE && Node; 
        Node = Next
    ) {
        Next = Node->LRUNext;

        if (Node->LastTouchedBuildIndex + 2 < UI_STATE->BuildIndex) {
            u64 SlotIndex = *Node->Key.V % UI_STATE->AnimationSlotsCount;
            UIAnimationSlot* Slot = &UI_STATE->AnimationSlots[SlotIndex];

            DLL_REMOVE_EX(EMPTY_ANIMATION_NODE_VALUE, Slot->Head, Slot->Tail, Node, SlotNext, SlotPrev);
            DLL_REMOVE_EX(EMPTY_ANIMATION_NODE_VALUE, UI_STATE->LRUAnimationNode, UI_STATE->MRUAnimationNode, Node, LRUNext, LRUPrev);
            SLL_STACK_PUSH_EX(UI_STATE->FreeAnimationNode, Node, SlotNext);
        } else {
            break;
        }
    }

    for (
        UIThemePatternCacheNode* Node = UI_STATE->LRUThemePatternCacheNode, *Next = NULL; 
        Node; 
        Node = Next
    ) {
        Next = Node->LRUNext;

        if (Node->LastBuildIndexAccessed + 2 < UI_STATE->BuildIndex) {
            u64 SlotIndex = *Node->Key.V % UI_STATE->ThemePatternCacheSlotsCount;
            UIThemePatternCacheSlot* Slot = &UI_STATE->ThemePatternCacheSlots[SlotIndex];

            DLL_REMOVE_EX(NULL, Slot->Head, Slot->Tail, Node, SlotNext, SlotPrev);
            DLL_REMOVE_EX(NULL, UI_STATE->LRUThemePatternCacheNode, UI_STATE->MRUThemePatternCacheNode, Node, LRUNext, LRUPrev);
            SLL_STACK_PUSH_EX(UI_STATE->ThemePatternCacheNodeFree, Node, SlotNext);
        } else {
            break;
        }
    }

    for (UIEventNode* Node = Events->Head; Node; Node = Node->Next)
        if (Node->V.Kind == UI_EVENT_KIND_MOUSE_MOVE)
            UI_STATE->LastTimeMouseMovedUSecs = TimeGetTimestamp();

    for (
        UIMouseButtonKind MouseBtnKind = (UIMouseButtonKind) 0;
        MouseBtnKind < UI_MOUSE_BTN_KIND_COUNT;
        MouseBtnKind = (UIMouseButtonKind) (MouseBtnKind + 1)
    ) {
        if (
            (UI_STATE->ActiveBoxKey[MouseBtnKind] == EMPTY_UI_KEY_VALUE) && 
            KeyIsDown((InputKind) (INPUT_KIND_LEFT_MOUSE_BTN + MouseBtnKind))
        ) {
            UI_STATE->ActiveBoxKey[MouseBtnKind] = UI_STATE->ExternalKey;
        } else if (
            (UI_STATE->ActiveBoxKey[MouseBtnKind] == UI_STATE->ExternalKey) &&
            !KeyIsDown((InputKind) (INPUT_KIND_LEFT_MOUSE_BTN + MouseBtnKind))
        ) {
            UI_STATE->ActiveBoxKey[MouseBtnKind] = EMPTY_UI_KEY_VALUE;
        }
    }

    UI_STATE->Theme = Theme;
    UI_STATE->Events = Events;
    UI_STATE->Mouse = GetMousePosition();
    UI_STATE->AnimationDelta = AnimationDeltaTime;
    MemSet(&UI_STATE->IconInfo, 0, sizeof(UI_STATE->IconInfo));

    for (
        UIIconKind IconKind = UI_ICON_KIND_NULL; 
        IconKind < UI_ICON_KIND_COUNT; 
        IconKind = (UIIconKind) (IconKind + 1)
    ) {
        UI_STATE->IconInfo.IconKindTextMap[IconKind] = ArenaPushStrCpy(UIBuildMemPool(), IconInfo->IconKindTextMap[IconKind]);
    }

    MemCpy(&UI_STATE->AnimationInfo, AnimationInfo, sizeof(UI_STATE->AnimationInfo));

    TempArena Scratch = GetScratch(NULL, 0);

    if (UI_STATE->DefaultNavRootKey != EMPTY_UI_KEY_VALUE) {
        UIBox* NavRoot = UIBoxFromKey(UI_STATE->DefaultNavRootKey);

        if (!IsUIBoxEmpty(NavRoot)) {
            if (NavRoot->DefaultNavFocusActiveKey == EMPTY_UI_KEY_VALUE) {
                for (;;) {
                    b32 Moved = FALSE;
                    UIBox* FocusBox = UIBoxFromKey(NavRoot->DefaultNavFocusNextHotKey);
                    UIBoxList NextFocusBoxCandidates = {};
                    b32 NavNext = FALSE;
                    b32 NavPrev = FALSE;
                    Axis2D AxisLock = AXIS_2D_INVALID;

                    if (UIKeyPress(0, INPUT_KIND_TAB))
                        NavNext = TRUE;

                    if (UIKeyPress(INPUT_MOD_KIND_SHIFT, INPUT_KIND_TAB))
                        NavPrev = TRUE;

                    for (UIEventNode* Node = Events->Head, *Next = NULL; Node; Node = Next) {
                        Next = Node->Next;

                        b32 Taken = FALSE;

                        if (Node->V.DeltaI32.X == 0 && Node->V.DeltaI32.Y == 0)
                            continue;

                        if (
                            ((Node->V.DeltaI32.X > 0 && NavRoot->Kind & UI_BOX_KIND_DEFAULT_FOCUS_NAV_X) || Node->V.DeltaI32.X == 0) &&
                            ((Node->V.DeltaI32.Y > 0 && NavRoot->Kind & UI_BOX_KIND_DEFAULT_FOCUS_NAV_Y) || Node->V.DeltaI32.Y == 0)
                        ) {
                            Taken = TRUE;
                            NavNext = TRUE;
                        }

                        if (
                            ((Node->V.DeltaI32.X < 0 && NavRoot->Kind & UI_BOX_KIND_DEFAULT_FOCUS_NAV_X) || Node->V.DeltaI32.X == 0) &&
                            ((Node->V.DeltaI32.Y < 0 && NavRoot->Kind & UI_BOX_KIND_DEFAULT_FOCUS_NAV_Y) || Node->V.DeltaI32.Y == 0)
                        ) {
                            Taken = TRUE;
                            NavPrev = TRUE;
                        }

                        if (Node->V.Flags & UI_EVENT_FLAG_EXPLICIT_DIRECTIONAL)
                            AxisLock = (Node->V.DeltaI32.X != 0) ? AXIS_2D_X : AXIS_2D_Y;

                        if (Taken)
                            UIEatEventNode(Events, Node);
                    }

                    if (NavNext) {
                        UIBox* SearchStart = IsUIBoxEmpty(FocusBox) ? NavRoot : FocusBox;
                        u64 MovedInAxis[AXIS_2D_COUNT] = {};

                        Moved = TRUE;

                        for (UIBox* Box = SearchStart; ;) {
                            if (
                                Box != SearchStart && 
                                !(Box->Kind & UI_BOX_KIND_FOCUS_NAV_SKIP) && 
                                (Box->Kind & UI_BOX_KIND_CLICKABLE || IsUIBoxEmpty(Box)) && 
                                (AxisLock == AXIS_2D_INVALID || MovedInAxis[AxisLock] > 0)
                            ) {
                                ListPush(Scratch.MemPool, &NextFocusBoxCandidates, Box);

                                if (AxisLock == AXIS_2D_INVALID || MovedInAxis[AxisLock] > 1)
                                    break;
                            }

                            UIBox* LastBox = Box;

                            if (!IsUIBoxEmpty(Box->Head)) {
                                ++MovedInAxis[Box->ChildLayoutAxis];
                                Box = Box->Head;
                            } else {
                                for (UIBox* B = Box; !IsUIBoxEmpty(B) && B != NavRoot; B = B->Parent) {
                                    if (!IsUIBoxEmpty(B->Next)) {
                                        ++MovedInAxis[B->Parent->ChildLayoutAxis];
                                        Box = B->Next;
                                        break;
                                    }
                                }
                            }

                            if (LastBox == Box) {
                                ListPush(Scratch.MemPool, &NextFocusBoxCandidates, EMPTY_UI_BOX_VALUE);
                                break;
                            }
                        }
                    }

                    if (NavPrev) {
                        UIBox* SearchStart = (IsUIBoxEmpty(FocusBox)) ? NavRoot : FocusBox;
                        u64 MovedInAxis[AXIS_2D_COUNT] = {};

                        Moved = TRUE;

                        for (UIBox* Box = SearchStart; ;) {
                            if (
                                Box != SearchStart && 
                                !(Box->Kind & UI_BOX_KIND_FOCUS_NAV_SKIP) && 
                                (Box->Kind & UI_BOX_KIND_CLICKABLE || IsUIBoxEmpty(Box)) && 
                                (AxisLock == AXIS_2D_INVALID || MovedInAxis[AxisLock] > 0)
                            ) {
                                ListPush(Scratch.MemPool, &NextFocusBoxCandidates, Box);

                                if (AxisLock == AXIS_2D_INVALID || MovedInAxis[AxisLock] > 1)
                                    break;
                            }

                            UIBox* LastBox = Box;
                            UIBox* RootDescendant = EMPTY_UI_BOX_VALUE;

                            if (Box == NavRoot && Box == SearchStart) {
                                for (UIBox* D = Box->Tail; !IsUIBoxEmpty(D); D = D->Tail) {
                                    ++MovedInAxis[D->Parent->ChildLayoutAxis];
                                    RootDescendant = D;
                                }
                            }

                            UIBox* PrevDescendant = EMPTY_UI_BOX_VALUE;

                            for (UIBox* D = Box->Prev; !IsUIBoxEmpty(D); D = D->Tail) {
                                ++MovedInAxis[D->Parent->ChildLayoutAxis];
                                PrevDescendant = D;
                            }

                            if (!IsUIBoxEmpty(RootDescendant)) {
                                Box = RootDescendant;
                            } else if (!IsUIBoxEmpty(PrevDescendant)) {
                                Box = PrevDescendant;
                            } else if (Box->Prev != NavRoot) {
                                ++MovedInAxis[Box->Parent->ChildLayoutAxis];
                                Box = Box->Parent;
                            }

                            if (Box == LastBox) {
                                ListPush(Scratch.MemPool, &NextFocusBoxCandidates, EMPTY_UI_BOX_VALUE);
                                break;
                            }
                        }
                    }

                    UIBox* NextFocusBox = FocusBox;
                    f32 BestDistanceFromStart = 1000000.0f;

                    for (UIBoxNode* Node = NextFocusBoxCandidates.Head; Node; Node = Node->Next) {
                        UIBox* Box = Node->Box;
                        f32 DistanceFromStart = 0.0f;

                        if (AxisLock != AXIS_2D_INVALID)
                            DistanceFromStart = AbsF32(Centre(Box->Rect).V[FLIP_AXIS(AxisLock)] - Centre(FocusBox->Rect).V[FLIP_AXIS(AxisLock)]);

                        if (DistanceFromStart < BestDistanceFromStart && Box != FocusBox) {
                            NextFocusBox = Box;
                            BestDistanceFromStart = DistanceFromStart;
                        }
                    }

                    NavRoot->DefaultNavFocusNextHotKey = NextFocusBox->Key;

                    if (!Moved)
                        break;
                }
            }

            if (NavRoot->DefaultNavFocusActiveKey != EMPTY_UI_KEY_VALUE) {
                for (; UISlotPress(UI_EVENT_ACTION_SLOT_CANCEL); ) {
                    UIBox* PrevFocusRoot = NavRoot;

                    for (
                        UIBox* FocusRoot = UIBoxFromKey(NavRoot->DefaultNavFocusActiveKey); 
                        !IsUIBoxEmpty(FocusRoot); 
                    ) {
                        UIBox* NextFocusRoot = UIBoxFromKey(FocusRoot->DefaultNavFocusActiveKey);

                        if (IsUIBoxEmpty(NextFocusRoot)) {
                            PrevFocusRoot->DefaultNavFocusNextActiveKey = EMPTY_UI_KEY_VALUE;
                            break;
                        } else {
                            PrevFocusRoot = FocusRoot;
                            FocusRoot = NextFocusRoot;
                        }
                    }
                }
            }
        }
    }

    UI_STATE->DefaultNavRootKey = EMPTY_UI_KEY_VALUE;
    ReleaseScratch(Scratch);

    for (u64 SlotIndex = 0; SlotIndex < UI_STATE->BoxTableSize; ++SlotIndex) {
        for (UIBox* Box = UI_STATE->BoxTable[SlotIndex].Head; !IsUIBoxEmpty(Box); Box = Box->HashNext) {
            Box->DefaultNavFocusHotKey = Box->DefaultNavFocusNextHotKey;
            Box->DefaultNavFocusActiveKey = Box->DefaultNavFocusNextActiveKey;
        }
    }

    r2f32 ConsoleRect = GetConsoleRect();
    v2f32 ConsoleRectSize = Length(ConsoleRect);

    UISetNextFixedX(ConsoleRect.X0);
    UISetNextFixedY(ConsoleRect.Y0);
    UISetNextFixedWidth(ConsoleRectSize.X);
    UISetNextFixedHeight(ConsoleRectSize.Y);
    UISetNextChildLayoutAxis(AXIS_2D_X);

    UIBox* Root = UIBuildBoxFromStr(0, "###root"_s8);

    UI_STATE->Root = Root;
    UIPushParent(Root);

    UIFixedX(UI_STATE->Mouse.X + 1.0f) {
        UIFixedY(UI_STATE->Mouse.Y + 1.0f) {
            UIPreferredWidth(UI_SUM_OF_CHILDREN(1.0f)) {
                UIPreferredHeight(UI_SUM_OF_CHILDREN(1.0f)) {
                    UISetNextChildLayoutAxis(AXIS_2D_Y);
                    UI_STATE->TooltipRoot = UIBuildBoxFromStr(0, "###tooltip"_s8);
                }
            }
        }
    }

    UI_STATE->ContextMenuOpen = UI_STATE->NextContextMenuOpen;
    UI_STATE->ContextMenuAnchorKey = UI_STATE->NextContextMenuAnchorKey;

    UIBox* AnchorBox = UIBoxFromKey(UI_STATE->ContextMenuAnchorKey);

    if (!IsUIBoxEmpty(AnchorBox))
        UI_STATE->ContextMenuAnchorBoxLastPosition = AnchorBox->Rect.Point0;

    v2f32 Anchor = UI_STATE->ContextMenuAnchorBoxLastPosition + UI_STATE->ContextMenuAnchorOffset;

    UIFixedX(Anchor.X) {
        UIFixedY(Anchor.Y) {
            UIPreferredWidth(UI_SUM_OF_CHILDREN(1.0f)) {
                UIPreferredHeight(UI_SUM_OF_CHILDREN(1.0f)) {
                    UIFocus(UI_FOCUS_KIND_ON) {
                        UISetNextChildLayoutAxis(AXIS_2D_Y);
                        UI_STATE->ContextMenuRoot = UIBuildBoxFromStr(
                            UI_BOX_KIND_CLICKABLE | (UI_STATE->ContextMenuOpen * UI_BOX_KIND_DEFAULT_FOCUS_NAV_Y),
                            "###ctx_menu"_s8
                        );
                    }
                }
            }
        }
    }

    b32 HasActive = FALSE;

    for (
        UIMouseButtonKind MouseBtnKind = (UIMouseButtonKind) 0; 
        MouseBtnKind < UI_MOUSE_BTN_KIND_COUNT;
        MouseBtnKind = (UIMouseButtonKind) (MouseBtnKind + 1)
    ) {
        if (UI_STATE->ActiveBoxKey[MouseBtnKind] != EMPTY_UI_KEY_VALUE)
            HasActive = TRUE;
    }

    if (!HasActive)
        UI_STATE->HotBoxKey = EMPTY_UI_KEY_VALUE;

    UI_STATE->DropHotBoxKey = EMPTY_UI_KEY_VALUE;

    for (
        UIMouseButtonKind MouseBtnKind = (UIMouseButtonKind) 0; 
        MouseBtnKind < UI_MOUSE_BTN_KIND_COUNT;
        MouseBtnKind = (UIMouseButtonKind) (MouseBtnKind + 1)
    ) {
        if (UI_STATE->ActiveBoxKey[MouseBtnKind] != EMPTY_UI_KEY_VALUE) {
            UIBox* Box = UIBoxFromKey(UI_STATE->ActiveBoxKey[MouseBtnKind]);

            if (!IsUIBoxEmpty(Box) && Box->Kind & UI_BOX_KIND_DISABLED)
                UI_STATE->ActiveBoxKey[MouseBtnKind] = EMPTY_UI_KEY_VALUE;
        }
    }

    for (
        UIMouseButtonKind MouseBtnKind = (UIMouseButtonKind) 0; 
        MouseBtnKind < UI_MOUSE_BTN_KIND_COUNT;
        MouseBtnKind = (UIMouseButtonKind) (MouseBtnKind + 1)
    ) {
        UIBox* Box = UIBoxFromKey(UI_STATE->ActiveBoxKey[MouseBtnKind]);

        if (IsUIBoxEmpty(Box))
            UI_STATE->ActiveBoxKey[MouseBtnKind] = EMPTY_UI_KEY_VALUE;
    }

    if (UIAnyContextMenuIsOpen() && UISlotPress(UI_EVENT_ACTION_SLOT_CANCEL))
        UIContextMenuClose();
}

void 
UIEndBuild(void)
{
    UIState* UI_STATE = GetTLS()->UI_STATE;

    for (u64 SlotIndex = 0; SlotIndex < UI_STATE->BoxTableSize; ++SlotIndex) {
        UIBoxHashSlot* Slot = &UI_STATE->BoxTable[SlotIndex];

        for (UIBox* Box = Slot->Head, *Next = NULL; !IsUIBoxEmpty(Box); Box = Next) {
            Next = Box->HashNext;

            if (
                Box->LastTouchedBuildIndex < UI_STATE->BuildIndex || 
                Box->Key == EMPTY_UI_KEY_VALUE
            ) {
                DLL_REMOVE_EX(EMPTY_UI_BOX_VALUE, Slot->Head, Slot->Tail, Box, HashNext, HashPrev);
                SLL_STACK_PUSH(UI_STATE->HeadFreeBox, Box);
            }
        }
    }

    for (Axis2D Axis = (Axis2D) 0; Axis < AXIS_2D_COUNT; Axis = (Axis2D) (Axis + 1))
        UILayoutRoot(UI_STATE->Root, Axis);

    if (!UI_STATE->ContextMenuTouchedThisFrame)
        UIContextMenuClose();

    if (UI_STATE->ContextMenuTouchedThisFrame) {
        UIBox* AnchorBox = UIBoxFromKey(UI_STATE->ContextMenuAnchorKey);

        if (!IsUIBoxEmpty(AnchorBox)) {
            r2f32 RootRect = UI_STATE->ContextMenuRoot->Rect;
            v2f32 Position = AnchorBox->Rect.Point0 + UI_STATE->ContextMenuAnchorOffset;
            v2f32 ShiftAmount = Position - RootRect.Point0;
            r2f32 NewRootRect = Shift(RootRect, ShiftAmount);

            UI_STATE->ContextMenuRoot->FixedPosition = NewRootRect.Point0;
            UI_STATE->ContextMenuRoot->FixedSize = Length(NewRootRect);
            UI_STATE->ContextMenuRoot->Rect = NewRootRect;
        }
    }

    if (UI_STATE->TooltipAnchorKey != EMPTY_UI_KEY_VALUE) {
        UIBox* AnchorBox = UIBoxFromKey(UI_STATE->TooltipAnchorKey);

        if (!IsUIBoxEmpty(AnchorBox)) {
            v2f32 Dimensions = Length(UI_STATE->TooltipRoot->Rect);

            UI_STATE->TooltipRoot->FixedPosition.X = AnchorBox->Rect.X0;
            UI_STATE->TooltipRoot->Rect.X0 = AnchorBox->Rect.X0;
            UI_STATE->TooltipRoot->FixedPosition.Y = AnchorBox->Rect.Y1;
            UI_STATE->TooltipRoot->Rect.Y0 = AnchorBox->Rect.Y1;
            UI_STATE->TooltipRoot->Rect.X1 = UI_STATE->TooltipRoot->Rect.X0 + Dimensions.X;
            UI_STATE->TooltipRoot->Rect.Y1 = UI_STATE->TooltipRoot->Rect.Y0 + Dimensions.Y;
        } else {
            UI_STATE->TooltipRoot->Rect.X0 = 10000.0f;
            UI_STATE->TooltipRoot->Rect.Y0 = 10000.0f;
        }
    }

    UIBox* FloatingRoots[] = {
        UI_STATE->TooltipRoot, 
        UI_STATE->ContextMenuRoot
    };

    for (u64 Index = 0; Index < ARRAY_COUNT(FloatingRoots); ++Index) {
        UIBox* Root = FloatingRoots[Index];

        if (!IsUIBoxEmpty(Root)) {
            r2f32 ConsoleRect = GetConsoleRect();
            r2f32 RootRect = Root->Rect;
            v2f32 ShiftDown = {};
            
            ShiftDown.X = -MAX(0.0f, RootRect.X1 - ConsoleRect.X1);
            ShiftDown.Y = -MAX(0.0f, RootRect.Y1 - ConsoleRect.Y1);

            r2f32 NewRootRect = Shift(RootRect, ShiftDown);
            v2f32 ShiftUp = {};

            ShiftUp.X = MAX(0.0f, ConsoleRect.X0 - NewRootRect.X0);
            ShiftUp.Y = MAX(0.0f, ConsoleRect.Y0 - NewRootRect.Y0);
            NewRootRect = Shift(NewRootRect, ShiftUp);
            Root->FixedPosition = NewRootRect.Point0;
            Root->FixedSize = Length(NewRootRect);
            Root->Rect = NewRootRect;

            for (Axis2D Axis = (Axis2D) 0; Axis < AXIS_2D_COUNT; Axis = (Axis2D) (Axis + 1)) {
                UICalcSizesStandalone(Root, Axis);
                UICalcSizesUpwardsDependent(Root, Axis);
                UICalcSizesDownwardsDependent(Root, Axis);
                UILayoutEnforceConstraints(Root, Axis);
                UILayoutPosition(Root, Axis);
            }
        }
    }

    for (u64 SlotIndex = 0; SlotIndex < UI_STATE->AnimationSlotsCount; ++SlotIndex) {
        for (
            UIAnimationNode* Node = UI_STATE->AnimationSlots[SlotIndex].Head; 
            Node && Node != EMPTY_ANIMATION_NODE_VALUE; 
            Node = Node->SlotNext
        ) {
            if (Node->Params.Rate > 1.0f || Node->Params.Rate < 0.0f) {
                Node->Params.Rate = UI_STATE->DefaultAnimationRate;
            }

            Node->Current += (Node->Params.Target - Node->Current) * Node->Params.Rate;
            UI_STATE->IsAnimating = (UI_STATE->IsAnimating || AbsF32(Node->Params.Target - Node->Current) > Node->Params.Epsilon);
        }
    }

    f32 SlowRate = 1.0f - Pow(2.0f, -30.0f * UI_STATE->AnimationDelta);

    for (u64 SlotIndex = 0; SlotIndex < UI_STATE->ThemePatternCacheSlotsCount; ++SlotIndex) {
        for (
            UIThemePatternCacheNode* Node = UI_STATE->ThemePatternCacheSlots[SlotIndex].Head; 
            Node; 
            Node = Node->SlotNext
        ) {
            for (u8 I = 0; I < 4; ++I) {
                Node->CurrentRGBA.V[I] += (Node->TargetRGBA.V[I] - Node->CurrentRGBA.V[I]) * SlowRate;
                UI_STATE->IsAnimating = (UI_STATE->IsAnimating || AbsF32(Node->TargetRGBA.V[I] - Node->CurrentRGBA.V[I]) > 0.001f);
            }
        }
    }

    UI_STATE->ContextMenuOpenT += (f32) !!UI_STATE->ContextMenuOpen;
    UI_STATE->TooltipOpenT += (f32) !!UI_STATE->TooltipOpen;

    for (u64 SlotIndex = 0; SlotIndex < UI_STATE->BoxTableSize; ++SlotIndex) {
        for (UIBox* Box = UI_STATE->BoxTable[SlotIndex].Head; !IsUIBoxEmpty(Box); Box = Box->HashNext) {
            b32 IsHot = (Box->Key == UI_STATE->HotBoxKey || Box->Key == UI_STATE->DropHotBoxKey);
            b32 IsActive = (Box->Key == UI_STATE->ActiveBoxKey[UI_MOUSE_BTN_KIND_LEFT]);
            b32 IsDisabled = !!(Box->Kind & UI_BOX_KIND_DISABLED) && (Box->FirstDisabledBuildIndex + 2 < UI_STATE->BuildIndex || Box->FirstTouchedBuildIndex == Box->FirstDisabledBuildIndex);
            b32 IsFocusHot = !!(Box->Kind & UI_BOX_KIND_FOCUS_HOT) && !(Box->Kind & UI_BOX_KIND_FOCUS_HOT_DISABLED);
            b32 IsFocusActive = !!(Box->Kind & UI_BOX_KIND_FOCUS_ACTIVE) && !(Box->Kind & UI_BOX_KIND_FOCUS_ACTIVE_DISABLED);
            b32 IsFocusActiveDisabled = !!(Box->Kind & UI_BOX_KIND_FOCUS_ACTIVE_DISABLED);
            f32 HotRate = UI_STATE->AnimationInfo.HotAnimationRate;
            f32 ActiveRate = UI_STATE->AnimationInfo.ActiveAnimationRate;
            f32 DisabledRate = SlowRate;
            f32 FocusRate = UI_STATE->AnimationInfo.FocusAnimationRate;
            b32 BoxIsAnimating = FALSE;

            BoxIsAnimating = (BoxIsAnimating || AbsF32((f32) IsHot - Box->Hot) > 0.01f);
            BoxIsAnimating = (BoxIsAnimating || AbsF32((f32) IsActive - Box->Active) > 0.01f);
            BoxIsAnimating = (BoxIsAnimating || AbsF32((f32) IsDisabled - Box->Disabled) > 0.01f);
            BoxIsAnimating = (BoxIsAnimating || AbsF32((f32) IsFocusHot - Box->FocusHot) > 0.01f);
            BoxIsAnimating = (BoxIsAnimating || AbsF32((f32) IsFocusActive - Box->FocusActive) > 0.01f);
            BoxIsAnimating = (BoxIsAnimating || AbsF32((f32) IsFocusActiveDisabled - Box->FocusDisabled) > 0.01f);
            BoxIsAnimating = (BoxIsAnimating || AbsF32(Box->ViewOffsetTarget.X - Box->ViewOffset.X) > 0.5f);
            BoxIsAnimating = (BoxIsAnimating || AbsF32(Box->ViewOffsetTarget.Y - Box->ViewOffset.Y) > 0.5f);

            if (Box->Kind & UI_BOX_KIND_ANIMATE_POS_X)
                BoxIsAnimating = (BoxIsAnimating || AbsF32(Box->FixedPositionAnimated.X - Box->FixedPosition.X) > 0.5f);

            if (Box->Kind & UI_BOX_KIND_ANIMATE_POS_Y)
                BoxIsAnimating = (BoxIsAnimating || AbsF32(Box->FixedPositionAnimated.Y - Box->FixedPosition.Y) > 0.5f);

            UI_STATE->IsAnimating = (UI_STATE->IsAnimating || BoxIsAnimating);
            Box->Hot += HotRate * ((f32) IsHot - Box->Hot);
            Box->Active = IsActive ? 1.0f : Box->Active + (ActiveRate * ((f32) IsActive - Box->Active));
            Box->Disabled += DisabledRate * ((f32) IsDisabled - Box->Disabled);
            Box->FocusHot += FocusRate * ((f32) IsFocusHot - Box->FocusHot);
            Box->FocusActive += FocusRate * ((f32) IsFocusActive - Box->FocusActive);
            Box->FocusDisabled += FocusRate * ((f32) IsFocusActiveDisabled - Box->FocusDisabled);

            f32 FastRate = UI_STATE->DefaultAnimationRate;

            Box->FixedPositionAnimated.X += FastRate * (Box->FixedPosition.X - Box->FixedPositionAnimated.X);
            Box->FixedPositionAnimated.Y += FastRate * (Box->FixedPosition.Y - Box->FixedPositionAnimated.Y);

            if (AbsF32(Box->FixedPosition.X - Box->FixedPositionAnimated.X) < 1.0f)
                Box->FixedPositionAnimated.X = Box->FixedPosition.X;

            if (AbsF32(Box->FixedPosition.Y - Box->FixedPositionAnimated.Y) < 1.0f)
                Box->FixedPositionAnimated.Y = Box->FixedPosition.Y;

            if (Box->Kind & UI_BOX_KIND_VIEW_CLAMP) {
                v2f32 MaxViewOffsetTarget = {};
                
                MaxViewOffsetTarget.X = MAX(0.0f, Box->ViewBounds.X - Box->FixedSize.X);
                MaxViewOffsetTarget.Y = MAX(0.0f, Box->ViewBounds.Y - Box->FixedSize.Y);

                if (Box->Kind & UI_BOX_KIND_VIEW_CLAMP_X)
                    Box->ViewOffsetTarget.X = CLAMP(0.0f, Box->ViewOffsetTarget.X, MaxViewOffsetTarget.X);

                if (Box->Kind & UI_BOX_KIND_VIEW_CLAMP_Y)
                    Box->ViewOffsetTarget.Y = CLAMP(0.0f, Box->ViewOffsetTarget.Y, MaxViewOffsetTarget.Y);
            }

            {
                Box->ViewOffset.X += UI_STATE->AnimationInfo.ScrollAnimationRate * (Box->ViewOffsetTarget.X - Box->ViewOffset.X);
                Box->ViewOffset.Y += UI_STATE->AnimationInfo.ScrollAnimationRate * (Box->ViewOffsetTarget.Y - Box->ViewOffset.Y);

                if (AbsF32(Box->ViewOffset.X - Box->ViewOffsetTarget.X) < 1.0f)
                    Box->ViewOffset.X = Box->ViewOffsetTarget.X;

                if (AbsF32(Box->ViewOffset.Y - Box->ViewOffsetTarget.Y) < 1.0f)
                    Box->ViewOffset.Y = Box->ViewOffsetTarget.Y;
            }
        }
    }

    for (UIBox* B = UI_STATE->Root; !IsUIBoxEmpty(B); B = UIBoxRecordDFPre(B, UI_STATE->Root).Next) {
        if (B->Key == EMPTY_UI_KEY_VALUE && B->GroupKey != EMPTY_UI_KEY_VALUE) {
            UIBox* GroupBox = UIBoxFromKey(B->GroupKey);

            B->Hot = GroupBox->Hot;
        }
    }

    if (UI_STATE->ContextMenuOpen)
        UISignalFromBox(UI_STATE->ContextMenuRoot);

    for (UIEvent* Event = NULL; UINextEvent(&Event);) {
        if (
            Event->Kind == UI_EVENT_KIND_PRESS && 
            (
                Event->Input == INPUT_KIND_LEFT_MOUSE_BTN || 
                Event->Input == INPUT_KIND_RIGHT_MOUSE_BTN
            )
        ) {
            UIContextMenuClose();
        }
    }

    if (UI_STATE->ActiveBoxKey[UI_MOUSE_BTN_KIND_LEFT] != UI_STATE->ExternalKey) {
        UIBox* Hot = UIBoxFromKey(UI_STATE->HotBoxKey);
        UIBox* Active = UIBoxFromKey(UI_STATE->ActiveBoxKey[UI_MOUSE_BTN_KIND_LEFT]);
        UIBox* Box = IsUIBoxEmpty(Active) ? Hot : Active;
        CursorKind Cursor = Box->HoverCursor;

        if (Box->Kind & UI_BOX_KIND_DISABLED && Box->Kind & UI_BOX_KIND_CLICKABLE)
            Cursor = CURSOR_KIND_DISABLED;

        SetCursorKind(Cursor);
    }

    UIBox* Box = UIBoxFromKey(UI_STATE->ClipboardCopyKey);

    if (!IsUIBoxEmpty(Box)) {
        TempArena Scratch = GetScratch(NULL, 0);
        Str8List Strings = {};

        for (
            UIBox* B = Box; 
            !IsUIBoxEmpty(B); 
            B = UIBoxRecordDFPre(B, Box).Next
        ) {
            if (
                B->Kind & UI_BOX_KIND_DRAW_TEXT && 
                B->Kind & UI_BOX_KIND_HAS_DISPLAY_STRING
            ) {
                Str8 DisplayString = UIBoxDisplayStr(B);

                ListPush(Scratch.MemPool, &Strings, DisplayString);
            }
        }

        if (Strings.Count != 0) {
            Str8JoinPart Join = {};

            Join.Sep = " "_s8;

            Str8 String = StrListJoin(Scratch.MemPool, &Strings, &Join);

            SetClipboardText(String);
        }

        ReleaseScratch(Scratch);
    }

    b32 Inactive = TRUE;

    for (
        UIMouseButtonKind K = (UIMouseButtonKind) 0; 
        K < UI_MOUSE_BTN_KIND_COUNT; 
        K = (UIMouseButtonKind) (K + 1)
    ) {
        if (UI_STATE->ActiveBoxKey[K] != EMPTY_UI_KEY_VALUE) {
            Inactive = FALSE;
            break;
        }
    }

    if (Inactive) {
        b32 Found = FALSE;

        for (
            UIBox* Box = UI_STATE->Root, *Next = NULL; 
            !IsUIBoxEmpty(Box); 
            Box = Next
        ) {
            UIBoxRecord Rec = UIBoxRecordDFPre(Box, UI_STATE->Root);

            Next = Rec.Next;

            i32 PopIndex = 0;

            for (
                UIBox* B = Box; 
                !IsUIBoxEmpty(B) && PopIndex <= Rec.PopCount; 
                B = B->Parent, ++PopIndex
            ) {
                if (
                    B->Kind & UI_BOX_KIND_DRAW_TEXT && 
                    !(B->Kind & UI_BOX_KIND_DISABLE_TEXT_TRUNC)
                ) {
                    r2f32 Rect = B->Rect;

                    for (UIBox* P = B->Parent; !IsUIBoxEmpty(P); P = P->Parent)
                        if (P->Kind & UI_BOX_KIND_CLIP)
                            Rect = Intersect(Rect, P->Rect);

                    Str8 BoxDisplayString = UIBoxDisplayStr(B);
                    v2f32 TextPosition = UIBoxTextPosition(B);
                    v2f32 DrawnTextDimensions = B->DisplayDimensions;
                    b32 TextIsTruncated = (
                        DrawnTextDimensions.X + TextPosition.X > Rect.X1
                    );
                    b32 MouseIsHovering = InRange(
                        Rng(
                            TextPosition.X, 
                            Rect.Y0, 
                            MIN(
                                TextPosition.X + DrawnTextDimensions.X, 
                                Rect.X1
                            ), 
                            Rect.Y1
                        ), 
                        UI_STATE->Mouse
                    );

                    if (
                        TextIsTruncated && 
                        MouseIsHovering && 
                        !(B->Kind & UI_BOX_KIND_DISABLE_TRUNCATED_HOVER)
                    ) {
                        if (
                            !StrMatch(
                                BoxDisplayString, 
                                UI_STATE->StringHoverString, 
                                0
                            )
                        ) {
                            ArenaClear(UI_STATE->StringHoverMemPool);
                            UI_STATE->StringHoverString = ArenaPushStrCpy(
                                UI_STATE->StringHoverMemPool, 
                                BoxDisplayString
                            );
                            UI_STATE->StringHoverFancyStrings = ListCpy(
                                UI_STATE->StringHoverMemPool, 
                                &B->DisplayFancyStrings
                            );
                            UI_STATE->StringHoverBeginUSecs = TimeGetTimestamp();
                        }

                        UI_STATE->StringHoverBuildIndex = UI_STATE->BuildIndex;
                        Found = TRUE;

                        goto BREAK_ALL_HOVER_STRING;
                    }
                }

                if (B != Box && B->Key == UIHotKey())
                    goto BREAK_ALL_HOVER_STRING;

                if (B != Box && InRange(B->Rect, UI_STATE->Mouse) && B->Kind & UI_BOX_KIND_DRAW_TEXT)
                    goto BREAK_ALL_HOVER_STRING;
            }
        }

BREAK_ALL_HOVER_STRING:
        if (!Found) {
            ArenaClear(UI_STATE->StringHoverMemPool);
            UI_STATE->StringHoverBuildIndex = 0;
            MemSet(&UI_STATE->StringHoverString, 0, sizeof(UI_STATE->StringHoverString));
        }

        if (Found && !UIStringHoverActive())
            UI_STATE->IsAnimating = TRUE;
    }

    ++UI_STATE->BuildIndex;
    ArenaClear(UIBuildMemPool());
}

void 
UICalcSizesStandalone(UIBox* Root, Axis2D Axis)
{
    for (
        UIBox* Box = Root; 
        !IsUIBoxEmpty(Box); 
        Box = UIBoxRecordDFPre(Box, Root).Next
    ) {
        switch (Box->PreferredSize[Axis].Kind) {
            default: {} break;

            case UI_SIZE_KIND_PIXELS: {
                Box->FixedSize.V[Axis] = Box->PreferredSize[Axis].Value;
            } break;

            case UI_SIZE_KIND_TEXT_CONTENT: {
                f32 Padding = Box->PreferredSize[Axis].Value;
                f32 TextSize = Box->DisplayDimensions.V[Axis];

                Box->FixedSize.V[Axis] = Padding + TextSize + Box->TextPadding * 2.0f;
            } break;
        }
    }
}

void 
UICalcSizesUpwardsDependent(UIBox* Root, Axis2D Axis)
{
    for (UIBox* Box = Root; !IsUIBoxEmpty(Box); Box = UIBoxRecordDFPre(Box, Root).Next) {
        switch (Box->PreferredSize[Axis].Kind) {
            default: {} break;

            case UI_SIZE_KIND_PERCENT_OF_PARENT: {
                UIBox* FixedParent = EMPTY_UI_BOX_VALUE;

                for (UIBox* Parent = Box->Parent; !IsUIBoxEmpty(Parent); Parent = Parent->Parent) {
                    if (
                        Parent->Kind & (UI_BOX_KIND_FIXED_WIDTH << Axis) ||
                        Parent->PreferredSize[Axis].Kind == UI_SIZE_KIND_PIXELS ||
                        Parent->PreferredSize[Axis].Kind == UI_SIZE_KIND_TEXT_CONTENT ||
                        Parent->PreferredSize[Axis].Kind == UI_SIZE_KIND_PERCENT_OF_PARENT

                    ) {
                        FixedParent = Parent;
                        break;
                    }
                }

                f32 Size = FixedParent->FixedSize.V[Axis] * Box->PreferredSize[Axis].Value;

                Box->FixedSize.V[Axis] = Size;
            } break;
        }
    }
}

void 
UICalcSizesDownwardsDependent(UIBox* Root, Axis2D Axis)
{
    UIBoxRecord Record = {};

    for (UIBox* Box = Root; !IsUIBoxEmpty(Box); Box = Record.Next) {
        Record = UIBoxRecordDFPre(Box, Root);

        i32 PopIndex = 0;

        for (
            UIBox* B = Box; 
            !IsUIBoxEmpty(B) && PopIndex <= Record.PopCount; 
            B = B->Parent, ++PopIndex
        ) {
            if (B->PreferredSize[Axis].Kind == UI_SIZE_KIND_SUM_OF_CHILDREN) {
                f32 Sum = 0.0f;

                for (UIBox* Child = B->Head; !IsUIBoxEmpty(Child); Child = Child->Next) {
                    if (!(Child->Kind & (UI_BOX_KIND_FLOATING_X << Axis))) {
                        if (Axis == B->ChildLayoutAxis)
                            Sum += Child->FixedSize.V[Axis];
                        else
                            Sum = MAX(Sum, Child->FixedSize.V[Axis]);
                    }
                }

                B->FixedSize.V[Axis] = Sum;
            }
        }
    }
}

void 
UILayoutEnforceConstraints(UIBox* Root, Axis2D Axis)
{
    TempArena Scratch = GetScratch(NULL, 0);

    for (UIBox* Box = Root; !IsUIBoxEmpty(Box); Box = UIBoxRecordDFPre(Box, Root).Next) {
        if (Axis != Box->ChildLayoutAxis && !(Box->Kind & (UI_BOX_KIND_ALLOW_OVERFLOW_X << Axis))) {
            f32 AllowedSize = Box->FixedSize.V[Axis];

            for (UIBox* Child = Box->Head; !IsUIBoxEmpty(Child); Child = Child->Next) {
                if (!(Child->Kind & (UI_BOX_KIND_FLOATING_X << Axis))) {
                    f32 ChildSize = Child->FixedSize.V[Axis];
                    f32 Violation = ChildSize - AllowedSize;
                    f32 MaxFixup = ChildSize;
                    f32 Fixup = CLAMP(0.0f, Violation, MaxFixup);

                    if (Fixup > 0.0f)
                        Child->FixedSize.V[Axis] -= Fixup;
                }
            }
        }

        if (Axis == Box->ChildLayoutAxis && !(Box->Kind & (UI_BOX_KIND_ALLOW_OVERFLOW_X << Axis))) {
            f32 TotalAllowedSize = Box->FixedSize.V[Axis];
            f32 TotalSize = 0.0f;
            f32 TotalWeightedSize = 0.0f;

            for (UIBox* Child = Box->Head; !IsUIBoxEmpty(Child); Child = Child->Next) {
                if (!(Child->Kind & (UI_BOX_KIND_FLOATING_X << Axis))) {
                    TotalSize += Child->FixedSize.V[Axis];
                    TotalWeightedSize += Child->FixedSize.V[Axis] * (1.0f - Child->PreferredSize[Axis].Strictness);
                }
            }

            f32 Violation = TotalSize - TotalAllowedSize;

            if (Violation > 0.0f && TotalWeightedSize > 0.0f) {
                TempArena Temp = ArenaBeginTemp(Scratch.MemPool);
                f32 ChildFixupSum = 0.0f;
                f32* ChildFixups = ArenaPushArrayZero(Temp.MemPool, f32, Box->ChildCount);
                u64 ChildIndex = 0;

                for (UIBox* Child = Box->Head; !IsUIBoxEmpty(Child); Child = Child->Next, ++ChildIndex) {
                    if (!(Child->Kind & (UI_BOX_KIND_FLOATING_X << Axis))) {
                        f32 FixupSizeThisChild = Child->FixedSize.V[Axis] * (1.0f - Child->PreferredSize[Axis].Strictness);

                        FixupSizeThisChild = CLAMP_BOT(0.0f, FixupSizeThisChild);
                        ChildFixups[ChildIndex] = FixupSizeThisChild;
                        ChildFixupSum += FixupSizeThisChild;
                    }
                }

                ChildIndex = 0;

                for (UIBox* Child = Box->Head; !IsUIBoxEmpty(Child); Child = Child->Next, ++ChildIndex) {
                    if (!(Child->Kind & (UI_BOX_KIND_FLOATING_X << Axis))) {
                        f32 FixupPercent = (Violation / TotalWeightedSize);

                        FixupPercent = CLAMP(0.0f, FixupPercent, 1.0f);
                        Child->FixedSize.V[Axis] -= ChildFixups[ChildIndex] * FixupPercent;
                    }
                }

                ArenaEndTemp(Temp);
            }
        }

        if (Box->Kind & (UI_BOX_KIND_ALLOW_OVERFLOW_X << Axis))
            for (UIBox* Child = Box->Head; !IsUIBoxEmpty(Child); Child = Child->Next)
                if (Child->PreferredSize[Axis].Kind == UI_SIZE_KIND_PERCENT_OF_PARENT)
                    Child->FixedSize.V[Axis] = Box->FixedSize.V[Axis] * Child->PreferredSize[Axis].Value;

        for (UIBox* Child = Box->Head; !IsUIBoxEmpty(Child); Child = Child->Next)
            Child->FixedSize.V[Axis] = MAX(Child->FixedSize.V[Axis], Child->MinSize.V[Axis]);
    }

    ReleaseScratch(Scratch);
}

void 
UILayoutPosition(UIBox* Root, Axis2D Axis)
{
    for (UIBox* Box = Root; !IsUIBoxEmpty(Box); Box = UIBoxRecordDFPre(Box, Root).Next) {
        f32 LayoutPosition = 0.0f;
        f32 Bounds = 0.0f;

        for (UIBox* Child = Box->Head; !IsUIBoxEmpty(Child); Child = Child->Next) {
            f32 OriginalPosition = MIN(Child->Rect.Point0.V[Axis], Child->Rect.Point1.V[Axis]);

            if (!(Child->Kind & (UI_BOX_KIND_FLOATING_X << Axis))) {
                Child->FixedPosition.V[Axis] = LayoutPosition;

                if (Box->ChildLayoutAxis == Axis) {
                    LayoutPosition += Child->FixedSize.V[Axis];
                    Bounds += Child->FixedSize.V[Axis];
                } else {
                    Bounds = MAX(Bounds, Child->FixedSize.V[Axis]);
                }
            }

            if (Child->Kind & (UI_BOX_KIND_ANIMATE_POS_X << Axis)) {
                if (Child->FirstTouchedBuildIndex == Child->LastTouchedBuildIndex)
                    Child->FixedPositionAnimated = Child->FixedPosition;

                Child->Rect.Point0.V[Axis] = (
                    Box->Rect.Point0.V[Axis] + 
                    Child->FixedPositionAnimated.V[Axis] - 
                    !(Child->Kind & (UI_BOX_KIND_SKIP_VIEW_OFF_X << Axis)) * 
                    FloorF32(Box->ViewOffset.V[Axis])
                );
            } else {
                Child->Rect.Point0.V[Axis] = Box->Rect.Point0.V[Axis] + Child->FixedPosition.V[Axis] - !(Child->Kind & (UI_BOX_KIND_SKIP_VIEW_OFF_X << Axis)) * FloorF32(Box->ViewOffset.V[Axis]);
            }

            Child->Rect.Point1.V[Axis] = Child->Rect.Point0.V[Axis] + Child->FixedSize.V[Axis];
            Child->Rect.Point0.X = FloorF32(Child->Rect.Point0.X);
            Child->Rect.Point0.Y = FloorF32(Child->Rect.Point0.Y);
            Child->Rect.Point1.X = FloorF32(Child->Rect.Point1.X);
            Child->Rect.Point1.Y = FloorF32(Child->Rect.Point1.Y);

            f32 NewPosition = MIN(Child->Rect.Point0.V[Axis], Child->Rect.Point1.V[Axis]);

            Child->PositionDelta.V[Axis] = NewPosition - OriginalPosition;
        }

        {
            Box->ViewBounds.V[Axis] = Bounds;
        }
    }
}

void 
UILayoutRoot(UIBox* Root, Axis2D Axis)
{
    UICalcSizesStandalone(Root, Axis);
    UICalcSizesUpwardsDependent(Root, Axis);
    UICalcSizesDownwardsDependent(Root, Axis);
    UILayoutEnforceConstraints(Root, Axis);
    UILayoutPosition(Root, Axis);
}

UISignal 
UISpacer(UISize Size)
{
    UIBox* Parent = UIHeadParent();

    UISetNextPreferredSize(Parent->ChildLayoutAxis, Size);

    UIBox* Box = UIBuildBoxFromKey(0, {});
    UISignal Interact = UISignalFromBox(Box);

    return Interact;
}

void 
UITooltipBeginBase(void)
{
    UIState* UI_STATE = GetTLS()->UI_STATE;

    UI_STATE->TooltipOpen = TRUE;
    UIPushParent(UIRootFromState(UI_STATE));
    UIPushParent(UI_STATE->TooltipRoot);
    UIPushFlags(0);
    UIPushTag("."_s8);
    UIPushTag("floating"_s8);
}

void 
UITooltipEndBase(void)
{
    UIPopTag();
    UIPopTag();
    UIPopFlags();
    UIPopParent();
    UIPopParent();
}

void 
UITooltipBegin(void)
{
    UIState* UI_STATE = GetTLS()->UI_STATE;

    UITooltipBeginBase();
    UIFlags(
        UI_BOX_KIND_FLOATING | 
        UI_BOX_KIND_DRAW_BACKGROUND | 
        UI_BOX_KIND_DRAW_DROP_SHADOW
    ) {
        UIPreferredWidth(UI_SUM_OF_CHILDREN(1.0f)) {
            UIPreferredHeight(UI_SUM_OF_CHILDREN(1.0f)) {
                UIColumnBegin();
            }
        }
    }

    UIPreferredWidth(UI_PX(0.0f, 1.0f))
        UISpacer(UI_PX(1.0f, 1.0f));

    UIPreferredWidth(UI_SUM_OF_CHILDREN(1.0f)) {
        UIPreferredHeight(UI_SUM_OF_CHILDREN(1.0f)) {
            UIRowBegin();
        }
    }

    UIPreferredHeight(UI_PX(0.0f, 1.0f))
        UISpacer(UI_PX(1.0f, 1.0f));

    UIPreferredWidth(UI_SUM_OF_CHILDREN(1.0f)) {
        UIPreferredHeight(UI_SUM_OF_CHILDREN(1.0f)) {
            UIColumnBegin();
        }
    }

    UIPushPreferredWidth(UI_TEXT_DIM(10.0f, 1.0f));
    UIPushPreferredHeight(UI_PX(1.0f, 1.0f));
    UIPushTextAlignment(UI_TEXT_ALIGN_CENTRE);
}

void 
UITooltipEnd(void)
{
    UIPopTextAlignment();
    UIPopPreferredHeight();
    UIPopPreferredWidth();
    UIColumnEnd();

    UIPreferredHeight(UI_PX(0.0f, 1.0f)) {
        UISpacer(UI_PX(1.0f, 1.0f));
    }

    UIRowEnd();

    UIPreferredWidth(UI_PX(0.0f, 1.0f)) {
        UISpacer(UI_PX(1.0f, 1.0f));
    }

    UIColumnEnd();
    UITooltipEndBase();
}

void 
UIContextMenuOpen(UIKey Key, UIKey AnchorBoxKey, v2f32 AnchorOffset)
{
    UIState* UI_STATE = GetTLS()->UI_STATE;

    AnchorOffset.X = (f32) (int) AnchorOffset.X;
    AnchorOffset.Y = (f32) (int) AnchorOffset.Y;
    UI_STATE->NextContextMenuOpen = TRUE;
    UI_STATE->ContextMenuChanged = TRUE;
    UI_STATE->ContextMenuOpenT = 0.0f;
    UI_STATE->ContextMenuKey = Key;
    UI_STATE->NextContextMenuAnchorKey = AnchorBoxKey;
    UI_STATE->ContextMenuAnchorOffset = AnchorOffset;
    UI_STATE->ContextMenuTouchedThisFrame = TRUE;
    UI_STATE->ContextMenuAnchorBoxLastPosition = Vec(0.0f, 0.0f);
    UI_STATE->ContextMenuRoot->DefaultNavFocusActiveKey = EMPTY_UI_KEY_VALUE;
    UI_STATE->ContextMenuRoot->DefaultNavFocusNextActiveKey = EMPTY_UI_KEY_VALUE;
}

void 
UIContextMenuClose(void)
{
    GetTLS()->UI_STATE->NextContextMenuOpen = FALSE;
}

b32 
UIBeginContextMenu(UIKey Key)
{
    UIState* UI_STATE = GetTLS()->UI_STATE;

    UIPushParent(UIRootFromState(UI_STATE));
    UIPushParent(UI_STATE->ContextMenuRoot);
    UIPushPreferredHeight(UITailPreferredHeight());
    UIPushPreferredWidth(UITailPreferredWidth());
    UIPushFocusHot(UI_FOCUS_KIND_ROOT);
    UIPushFocusActive(UI_FOCUS_KIND_ROOT);
    UIPushTag("."_s8);

    b32 IsOpen = (Key == UI_STATE->ContextMenuKey) && UI_STATE->ContextMenuOpen;

    if (IsOpen) {
        UITag("floating"_s8) {
            UI_STATE->ContextMenuTouchedThisFrame = TRUE;
            UI_STATE->ContextMenuRoot->Kind |= UI_BOX_KIND_DRAW_BACKGROUND;
            UI_STATE->ContextMenuRoot->Kind |= UI_BOX_KIND_DISABLE_FOCUS_OVERLAY;
            UI_STATE->ContextMenuRoot->Kind |= UI_BOX_KIND_CLIP;
            UI_STATE->ContextMenuRoot->Kind |= UI_BOX_KIND_CLICKABLE;
            UI_STATE->ContextMenuRoot->TagsKey = UIHeadTagsKey();
            UI_STATE->ContextMenuRoot->TextColour = UIColourFromName("text"_s8);
            UI_STATE->ContextMenuRoot->BackgroundColour = UIColourFromName("background"_s8);
            UISpacer(UI_PX(1.0f, 1.0f));
        }
    }

    UI_STATE->IsInOpenContextMenu = IsOpen;

    return IsOpen;
}

void 
UIEndContextMenu(void)
{
    UIState* UI_STATE = GetTLS()->UI_STATE;

    if (UI_STATE->IsInOpenContextMenu) {
        UI_STATE->IsInOpenContextMenu = FALSE;
        UISpacer(UI_PX(1.0f, 1.0f));
    }

    UIPopTag();
    UIPopFocusActive();
    UIPopFocusHot();
    UIPopPreferredWidth();
    UIPopPreferredHeight();
    UIPopParent();
    UIPopParent();
}

b32 
UIContextMenuIsOpen(UIKey Key)
{
    return (GetTLS()->UI_STATE->ContextMenuOpen && (Key == GetTLS()->UI_STATE->ContextMenuKey));
}

b32 
UIAnyContextMenuIsOpen(void)
{
    return GetTLS()->UI_STATE->ContextMenuOpen;
}

b32 
UIIsFocusHot(void)
{
    UIState* UI_STATE = GetTLS()->UI_STATE;
    b32 Result = (UI_STATE->FocusHotStack.Head->V == UI_FOCUS_KIND_ON);

    if (Result) {
        for (UIFocusHotNode* Node = UI_STATE->FocusHotStack.Head; Node; Node = Node->Next) {
            if (Node->V == UI_FOCUS_KIND_ROOT)
                break;

            if (Node->V == UI_FOCUS_KIND_OFF) {
                Result = FALSE;
                break;
            }
        }
    }

    return Result;
}

b32 
UIIsFocusActive(void)
{
    UIState* UI_STATE = GetTLS()->UI_STATE;
    b32 Result = (UI_STATE->FocusActiveStack.Head->V == UI_FOCUS_KIND_ON);

    if (Result) {
        for (UIFocusActiveNode* Node = UI_STATE->FocusActiveStack.Head; Node; Node = Node->Next) {
            if (Node->V == UI_FOCUS_KIND_ROOT)
                break;

            if (Node->V == UI_FOCUS_KIND_OFF) {
                Result = FALSE;
                break;
            }
        }
    }

    return Result;
}

b32 
UIIsKeyAutoFocusActive(UIKey Key)
{
    b32 Result = FALSE;

    if (Key != EMPTY_UI_KEY_VALUE) {
        for (UIBox* Parent = UIHeadParent(); !IsUIBoxEmpty(Parent); Parent = Parent->Parent) {
            if (Parent->Kind & UI_BOX_KIND_FOCUS_ACTIVE && Key == Parent->DefaultNavFocusActiveKey) {
                Result = TRUE;
                break;
            }
        }
    }

    return Result;
}

b32 
UIIsKeyAutoFocusHot(UIKey Key)
{
    b32 Result = FALSE;

    if (Key != EMPTY_UI_KEY_VALUE) {
        for (UIBox* Parent = UIHeadParent(); !IsUIBoxEmpty(Parent); Parent = Parent->Parent) {
            if (
                Parent->Kind & UI_BOX_KIND_FOCUS_HOT &&
                (
                    (!(Parent->Kind & UI_BOX_KIND_FOCUS_HOT_DISABLED) && 
                    Key == Parent->DefaultNavFocusHotKey || 
                    Key == Parent->DefaultNavFocusActiveKey)
                )
            ) {
                Result = TRUE;
                break;
            }
        }
    }

    return Result;
}

void 
UISetAutoFocusActiveKey(UIKey Key)
{
    for (UIBox* Parent = UIHeadParent(); !IsUIBoxEmpty(Parent); Parent = Parent->Parent) {
        if (Parent->Kind & UI_BOX_KIND_DEFAULT_FOCUS_NAV) {
            Parent->DefaultNavFocusNextActiveKey = Key;
            break;
        }
    }
}

void 
UISetAutoFocusHotKey(UIKey Key)
{
    for (UIBox* Parent = UIHeadParent(); !IsUIBoxEmpty(Parent); Parent = Parent->Parent) {
        if (Parent->Kind & UI_BOX_KIND_DEFAULT_FOCUS_NAV) {
            Parent->DefaultNavFocusNextHotKey = Key;
            break;
        }
    }
}

UIKey 
UIHeadTagsKey(void)
{
    UIState* UI_STATE = GetTLS()->UI_STATE;
    UIKey Result = EMPTY_UI_KEY_VALUE;

    if (UI_STATE->TagsKeyStackHead)
        Result = UI_STATE->TagsKeyStackHead->Key;

    return Result;
}

v4f32 
UIColourFromName(Str8 Name)
{
    v4f32 Result = UIColourFromTagsKeyName(UIHeadTagsKey(), Name);

    return Result;
}

v4f32 
UIColourFromTagsKeyExtras(UIKey Key, Str8Array Extras)
{
    UIState* UI_STATE = GetTLS()->UI_STATE;
    v4f32 Result = {};

    if (UI_STATE->ThemePatternCacheSlotsCount && Extras.Count > 0) {
        UIKey FinalKey = Key;

        for (u64 I = 0; I < Extras.Count; ++I)
            FinalKey = UIKeyFromStr(FinalKey, Extras.Data[I]);

        u64 SlotIndex = *FinalKey.V % UI_STATE->ThemePatternCacheSlotsCount;
        UIThemePatternCacheSlot* Slot = &UI_STATE->ThemePatternCacheSlots[SlotIndex];
        UIThemePatternCacheNode* Node = NULL;

        for (UIThemePatternCacheNode* N = Slot->Head; N; N = N->SlotNext)
            if (N->Key == FinalKey)
                Node = N;

        if (!Node || Node->LastBuildIndexAccessed < UI_STATE->BuildIndex) {
            Str8Array Tags = {};
            u64 TagsCacheSlotIndex = *Key.V % UI_STATE->TagsCacheSlotsCount;
            UITagsCacheSlot* TagsCacheSlot = &UI_STATE->TagsCacheSlots[TagsCacheSlotIndex];

            for (UITagsCacheNode* N = TagsCacheSlot->Head; N; N = N->Next) {
                if (N->Key == Key) {
                    Tags = N->Tags;
                    break;
                }
            }

            UITheme* Theme = UI_STATE->Theme;
            UIThemePattern* Pattern = NULL;
            u64 BestMatchCount = 0;

            for (u64 I = 0; I < Theme->PatternsCount; ++I) {
                UIThemePattern* P = &Theme->Patterns[I];
                u64 MatchCount = 0;
                b32 NameMatches = FALSE;
                b32 AllPTagsInKey = TRUE;

                for (u64 PTagsIndex = 0; PTagsIndex < P->Tags.Count; ++PTagsIndex) {
                    b32 PTagInKey = FALSE;

                    for (u64 KeyTagsIndex = 0; KeyTagsIndex < Tags.Count + Extras.Count; ++KeyTagsIndex) {
                        Str8 KeyString = (KeyTagsIndex < Tags.Count) ? Tags.Data[KeyTagsIndex] : Extras.Data[KeyTagsIndex - Tags.Count];

                        if (StrMatch(P->Tags.Data[PTagsIndex], KeyString, 0)) {
                            if (KeyTagsIndex == Tags.Count + Extras.Count - 1)
                                NameMatches = TRUE;

                            PTagInKey = TRUE;
                            ++MatchCount;
                            break;
                        }
                    }

                    if (!PTagInKey) {
                        AllPTagsInKey = FALSE;
                        break;
                    }
                }

                if (NameMatches && AllPTagsInKey && MatchCount > BestMatchCount) {
                    Pattern = P;
                    BestMatchCount = MatchCount;
                }

                if (MatchCount == Tags.Count + Extras.Count)
                    break;
            }

            b32 NodeIsNew = FALSE;

            if (!Node) {
                NodeIsNew = TRUE;
                Node = UI_STATE->ThemePatternCacheNodeFree;

                if (Node) {
                    UI_STATE->ThemePatternCacheNodeFree = Node->SlotNext;
                } else {
                    Node = ArenaPushArrayZero(UI_STATE->MemPool, UIThemePatternCacheNode, 1);
                }

                if (!Slot->Head) {
                    Slot->Head = Slot->Tail = Node; 
                    Node->SlotNext = Node->SlotPrev = NULL; 
                } else { 
                    Slot->Tail->SlotNext = Node; 
                    Node->SlotPrev = Slot->Tail; 
                    Slot->Tail = Node; 
                    Node->SlotNext = NULL; 
                }

                if (!UI_STATE->LRUThemePatternCacheNode) {
                    UI_STATE->LRUThemePatternCacheNode = UI_STATE->MRUThemePatternCacheNode = Node; 
                    Node->LRUNext = Node->LRUPrev = NULL; 
                } else { 
                    UI_STATE->MRUThemePatternCacheNode->LRUNext = Node; 
                    Node->LRUPrev = UI_STATE->MRUThemePatternCacheNode; 
                    UI_STATE->MRUThemePatternCacheNode = Node; 
                    Node->LRUNext = NULL; 
                }

                Node->Key = FinalKey;
                Node->TargetRGBA = Vec(1.0f, 0.0f, 1.0f, 1.0f);
                Node->CurrentRGBA = Vec(1.0f, 0.0f, 1.0f, 1.0f);
            }

            if (Pattern) {
                Node->TargetRGBA = Pattern->Linear;

                if (NodeIsNew)
                    Node->CurrentRGBA = Node->TargetRGBA;
            }
        }

        if (Node && Node->LastBuildIndexAccessed < UI_STATE->BuildIndex) {
            Node->LastBuildIndexAccessed = UI_STATE->BuildIndex;
            DLL_REMOVE_EX(NULL, UI_STATE->LRUThemePatternCacheNode, UI_STATE->MRUThemePatternCacheNode, Node, LRUNext, LRUPrev);
            
            if (!UI_STATE->LRUThemePatternCacheNode) { 
                UI_STATE->LRUThemePatternCacheNode = UI_STATE->MRUThemePatternCacheNode = Node; 
                Node->LRUNext = Node->LRUPrev = NULL; 
            } else { 
                UI_STATE->MRUThemePatternCacheNode->LRUNext = Node; 
                Node->LRUPrev = UI_STATE->MRUThemePatternCacheNode; 
                UI_STATE->MRUThemePatternCacheNode = Node; 
                Node->LRUNext = NULL; 
            }
        }

        if (Node)
            Result = Node->CurrentRGBA;
    }

    return Result;
}

v4f32 
UIColourFromTagsKeyName(UIKey Key, Str8 Name)
{
    Str8Array Extras = {
        &Name,
        1
    };

    v4f32 Result = UIColourFromTagsKeyExtras(Key, Extras);

    return Result;
}

UIBox* 
UIBuildBoxFromKey(UIBoxKind Kind, UIKey Key)
{
    UIState* UI_STATE = GetTLS()->UI_STATE;

    ++UI_STATE->BuildBoxCount;

    UIBox* Parent = UIHeadParent();
    UIBoxKind LastKind = 0;
    UIBox* Box = UIBoxFromKey(Key);
    b32 BoxFirstFrame = IsUIBoxEmpty(Box);

    LastKind = Box->Kind;

    if (!BoxFirstFrame && Box->LastTouchedBuildIndex == UI_STATE->BuildIndex) {
        Box = EMPTY_UI_BOX_VALUE;
        Key = EMPTY_UI_KEY_VALUE;
        BoxFirstFrame = TRUE;
    }

    b32 BoxIsTransient = (Key == EMPTY_UI_KEY_VALUE);

    if (BoxFirstFrame) {
        Box = !BoxIsTransient ? UI_STATE->HeadFreeBox : NULL;
        UI_STATE->IsAnimating = UI_STATE->IsAnimating || !BoxIsTransient;

        if (!IsUIBoxEmpty(Box)) {
            SLL_STACK_POP(UI_STATE->HeadFreeBox);
        } else {
            Box = ArenaPushArrayZero(!BoxIsTransient ? UI_STATE->MemPool : UIBuildMemPool(), UIBox, 1);
        }

        MemSet(Box, 0, sizeof(UIBox));
    }

    Box->Head = EMPTY_UI_BOX_VALUE;
    Box->Tail = EMPTY_UI_BOX_VALUE;
    Box->Next = EMPTY_UI_BOX_VALUE;
    Box->Prev = EMPTY_UI_BOX_VALUE;
    Box->HashNext = EMPTY_UI_BOX_VALUE;
    Box->HashPrev = EMPTY_UI_BOX_VALUE;
    Box->Parent = EMPTY_UI_BOX_VALUE;
    Box->ChildCount = 0;
    Box->Kind = 0;
    Box->HoverCursor = CURSOR_KIND_POINTER;
    MemSet(Box->PreferredSize, 0, sizeof(Box->PreferredSize));

    if (BoxFirstFrame && !BoxIsTransient) {
        u64 Slot = *Key.V % UI_STATE->BoxTableSize;

        DLL_INSERT_EX(EMPTY_UI_BOX_VALUE, UI_STATE->BoxTable[Slot].Head, UI_STATE->BoxTable[Slot].Tail, UI_STATE->BoxTable[Slot].Tail, Box, HashNext, HashPrev);
    }

    if (!IsUIBoxEmpty(Parent)) {
        DLL_PUSH_BACK_EX(EMPTY_UI_BOX_VALUE, Parent->Head, Parent->Tail, Box, Next, Prev);
        ++Parent->ChildCount;
        Box->Parent = Parent;
    }

    Box->Key = Key;
    Box->Kind = (Kind | UI_STATE->FlagsStack.Head->V) & ~UI_STATE->OmitFlagsStack.Head->V;
    Box->FastpathCodepoint = UI_STATE->FastpathCodepointStack.Head->V;
    Box->GroupKey = UI_STATE->GroupKeyStack.Head->V;

    if (UIIsFocusActive() && (Box->Kind & UI_BOX_KIND_DEFAULT_FOCUS_NAV) && (UI_STATE->DefaultNavRootKey == EMPTY_UI_KEY_VALUE)) {
        UI_STATE->DefaultNavRootKey = Box->Key;
    }

    if (BoxFirstFrame) {
        Box->FirstTouchedBuildIndex = UI_STATE->BuildIndex;
        Box->Disabled = (f32)!!(Box->Kind & UI_BOX_KIND_DISABLED);
    }

    Box->LastTouchedBuildIndex = UI_STATE->BuildIndex;

    if (Box->Kind & UI_BOX_KIND_DISABLED && (!(LastKind & UI_BOX_KIND_DISABLED) || BoxFirstFrame)) {
        Box->FirstDisabledBuildIndex = UI_STATE->BuildIndex;
    }

    if (UI_STATE->FixedXStack.Head != &UI_STATE->FixedXEmptyStackHead) {
        Box->Kind |= UI_BOX_KIND_FLOATING_X;
        Box->FixedPosition.X = UI_STATE->FixedXStack.Head->V;
    }

    if (UI_STATE->FixedYStack.Head != &UI_STATE->FixedYEmptyStackHead) {
        Box->Kind |= UI_BOX_KIND_FLOATING_Y;
        Box->FixedPosition.Y = UI_STATE->FixedYStack.Head->V;
    }

    if (UI_STATE->FixedWidthStack.Head != &UI_STATE->FixedWidthEmptyStackHead) {
        Box->Kind |= UI_BOX_KIND_FIXED_WIDTH;
        Box->FixedSize.X = UI_STATE->FixedWidthStack.Head->V;
    } else {
        Box->PreferredSize[AXIS_2D_X] = UI_STATE->PreferredWidthStack.Head->V;
    }

    if (UI_STATE->FixedHeightStack.Head != &UI_STATE->FixedHeightEmptyStackHead) {
        Box->Kind |= UI_BOX_KIND_FIXED_HEIGHT;
        Box->FixedSize.Y = UI_STATE->FixedHeightStack.Head->V;
    } else {
        Box->PreferredSize[AXIS_2D_Y] = UI_STATE->PreferredHeightStack.Head->V;
    }

    Box->MinSize.V[AXIS_2D_X] = UI_STATE->MinWidthStack.Head->V;
    Box->MinSize.V[AXIS_2D_Y] = UI_STATE->MinHeightStack.Head->V;

    b32 IsAutoFocusActive = UIIsKeyAutoFocusActive(Key);
    b32 IsAutoFocusHot = UIIsKeyAutoFocusHot(Key);

    if (IsAutoFocusActive) {
        UISetNextFocusActive(UI_FOCUS_KIND_ON);
    }

    if (IsAutoFocusHot) {
        UISetNextFocusHot(UI_FOCUS_KIND_ON);
    }

    Box->Kind |= UI_BOX_KIND_FOCUS_HOT * (UI_STATE->FocusHotStack.Head->V == UI_FOCUS_KIND_ON);
    Box->Kind |= UI_BOX_KIND_FOCUS_ACTIVE * (UI_STATE->FocusActiveStack.Head->V == UI_FOCUS_KIND_ON);

    if (Box->Kind & UI_BOX_KIND_FOCUS_HOT && !UIIsFocusHot()) {
        Box->Kind |= UI_BOX_KIND_FOCUS_HOT_DISABLED;
    }

    if (Box->Kind & UI_BOX_KIND_FOCUS_ACTIVE && !UIIsFocusActive()) {
        Box->Kind |= UI_BOX_KIND_FOCUS_ACTIVE_DISABLED;
    }

    Box->TextAlign = UI_STATE->TextAlignmentStack.Head->V;
    Box->ChildLayoutAxis = UI_STATE->ChildLayoutAxisStack.Head->V;
    Box->TabSize = UI_STATE->TabSizeStack.Head->V;
    Box->TextPadding = UI_STATE->TextPaddingStack.Head->V;
    Box->HoverCursor = UI_STATE->HoverCursorStack.Head->V;
    Box->CustomDrawFunc = NULL;
    Box->TagsKey = EMPTY_UI_KEY_VALUE;

    if (UI_STATE->TagsKeyStackHead) {
        Box->TagsKey = UI_STATE->TagsKeyStackHead->Key;
    }

    if (Box->Kind & UI_BOX_KIND_DRAW_BACKGROUND) {
        if (UI_STATE->BackgroundColourStack.Head != &UI_STATE->BackgroundColourEmptyStackHead) {
            Box->BackgroundColour = UI_STATE->BackgroundColourStack.Head->V;
        } else {
            Box->BackgroundColour = UIColourFromName("background"_s8);
        }
    }

    if (Box->Kind & UI_BOX_KIND_DRAW_TEXT) {
        if (UI_STATE->TextColourStack.Head != &UI_STATE->TextColourEmptyStackHead) {
            Box->TextColour = UI_STATE->TextColourStack.Head->V;
        } else {
            Box->TextColour = UIColourFromName("text"_s8);
        }
    }

    if (Box->Kind & (UI_BOX_KIND_DRAW_BORDER | UI_BOX_KIND_DRAW_SIDE_RIGHT | UI_BOX_KIND_DRAW_SIDE_LEFT | UI_BOX_KIND_DRAW_SIDE_TOP | UI_BOX_KIND_DRAW_SIDE_BOTTOM)) {
        if (UI_STATE->BorderColourStack.Head != &UI_STATE->BorderColourEmptyStackHead) {
            Box->BorderColour = UI_STATE->BorderColourStack.Head->V;
        } else {
            Box->BorderColour = UIColourFromName("border"_s8);
        }
    }

    if (UI_STATE->ParentStack.AutoPop) {
        UIPopParent();
        UI_STATE->ParentStack.AutoPop = FALSE;
    }

    if (UI_STATE->ChildLayoutAxisStack.AutoPop) {
        UIPopChildLayoutAxis();
        UI_STATE->ChildLayoutAxisStack.AutoPop = FALSE;
    }

    if (UI_STATE->FixedXStack.AutoPop) {
        UIPopFixedX();
        UI_STATE->FixedXStack.AutoPop = FALSE;
    }

    if (UI_STATE->FixedYStack.AutoPop) {
        UIPopFixedY();
        UI_STATE->FixedYStack.AutoPop = FALSE;
    }

    if (UI_STATE->FixedWidthStack.AutoPop) {
        UIPopFixedWidth();
        UI_STATE->FixedWidthStack.AutoPop = FALSE;
    }

    if (UI_STATE->FixedHeightStack.AutoPop) {
        UIPopFixedHeight();
        UI_STATE->FixedHeightStack.AutoPop = FALSE;
    }

    if (UI_STATE->PreferredWidthStack.AutoPop) {
        UIPopPreferredWidth();
        UI_STATE->PreferredWidthStack.AutoPop = FALSE;
    }

    if (UI_STATE->PreferredHeightStack.AutoPop) {
        UIPopPreferredHeight();
        UI_STATE->PreferredHeightStack.AutoPop = FALSE;
    }

    if (UI_STATE->MinWidthStack.AutoPop) {
        UIPopMinWidth();
        UI_STATE->MinWidthStack.AutoPop = FALSE;
    }

    if (UI_STATE->MinHeightStack.AutoPop) {
        UIPopMinHeight();
        UI_STATE->MinHeightStack.AutoPop = FALSE;
    }

    if (UI_STATE->PermissionKindsStack.AutoPop) {
        UIPopPermissionKind();
        UI_STATE->PermissionKindsStack.AutoPop = FALSE;
    }

    if (UI_STATE->FlagsStack.AutoPop) {
        UIPopFlags();
        UI_STATE->FlagsStack.AutoPop = FALSE;
    }

    if (UI_STATE->OmitFlagsStack.AutoPop) {
        UIPopOmitFlags();
        UI_STATE->OmitFlagsStack.AutoPop = FALSE;
    }

    if (UI_STATE->FocusHotStack.AutoPop) {
        UIPopFocusHot();
        UI_STATE->FocusHotStack.AutoPop = FALSE;
    }

    if (UI_STATE->FocusActiveStack.AutoPop) {
        UIPopFocusActive();
        UI_STATE->FocusActiveStack.AutoPop = FALSE;
    }

    if (UI_STATE->FastpathCodepointStack.AutoPop) {
        UIPopFastpathCodepoint();
        UI_STATE->FastpathCodepointStack.AutoPop = FALSE;
    }

    if (UI_STATE->GroupKeyStack.AutoPop) {
        UIPopGroupKey();
        UI_STATE->GroupKeyStack.AutoPop = FALSE;
    }

    if (UI_STATE->TagStack.AutoPop) {
        UIPopTag();
        UI_STATE->TagStack.AutoPop = FALSE;
    }

    if (UI_STATE->BackgroundColourStack.AutoPop) {
        UIPopBackgroundColour();
        UI_STATE->BackgroundColourStack.AutoPop = FALSE;
    }

    if (UI_STATE->TextColourStack.AutoPop) {
        UIPopTextColour();
        UI_STATE->TextColourStack.AutoPop = FALSE;
    }

    if (UI_STATE->BorderColourStack.AutoPop) {
        UIPopBorderColour();
        UI_STATE->BorderColourStack.AutoPop = FALSE;
    }

    if (UI_STATE->HoverCursorStack.AutoPop) {
        UIPopHoverCursor();
        UI_STATE->HoverCursorStack.AutoPop = FALSE;
    }

    if (UI_STATE->TabSizeStack.AutoPop) {
        UIPopTabSize();
        UI_STATE->TabSizeStack.AutoPop = FALSE;
    }

    if (UI_STATE->TextPaddingStack.AutoPop) {
        UIPopTextPadding();
        UI_STATE->TextPaddingStack.AutoPop = FALSE;
    }

    if (UI_STATE->TextAlignmentStack.AutoPop) {
        UIPopTextAlignment();
        UI_STATE->TextAlignmentStack.AutoPop = FALSE;
    }

    return Box;
}

UIKey 
UIActiveSeedKey(void)
{
    UIBox* KeyedAncestor = EMPTY_UI_BOX_VALUE;

    for (UIBox* Parent = UIHeadParent(); !IsUIBoxEmpty(Parent); Parent = Parent->Parent) {
        if (Parent->Key != EMPTY_UI_KEY_VALUE) {
            KeyedAncestor = Parent;
            break;
        }
    }

    return KeyedAncestor->Key;
}

UIBox* 
UIBuildBoxFromStr(UIBoxKind Kind, Str8 String)
{
    UIBox* Parent = UIHeadParent();
    UIKey Key = UIKeyFromStr(UIActiveSeedKey(), String);
    UIBox* Box = UIBuildBoxFromKey(Kind, Key);

    if (Kind & UI_BOX_KIND_DRAW_TEXT)
        UIBoxEquipDisplayStr(Box, String);

    return Box;
}

UIBox* 
UIBuildBoxFromStrFmt(UIBoxKind Kind, char* Fmt, ...)
{
    TempArena Scratch = GetScratch(NULL, 0);
    va_list Args;

    va_start(Args, Fmt);

    Str8 String = ArenaPushStrFmtV(Scratch.MemPool, Fmt, Args);

    va_end(Args);

    UIBox* Box = UIBuildBoxFromStr(Kind, String);

    ReleaseScratch(Scratch);

    return Box;
}

Str8 
UIBoxDisplayStr(UIBox* Box)
{
    Str8 Result = Box->String;

    if (!(Box->Kind & UI_BOX_KIND_DISABLE_ID_STRING))
        Result = UIDisplayPartFromKeyStr(Result);

    return Result;
}

v2f32 
UIBoxTextPosition(UIBox* Box)
{
    v2f32 Result = {};

    Result.Y = FloorF32((Box->Rect.Point0.Y + Box->Rect.Point1.Y) / 2.0f);

    switch (Box->TextAlign) {
        default:
        case UI_TEXT_ALIGN_LEFT: {
            Result.X = Box->Rect.Point0.X + Box->TextPadding;
        } break;

        case UI_TEXT_ALIGN_CENTRE: {
            v2f32 TextDimensions = Box->DisplayDimensions;

            Result.X = RoundF32((Box->Rect.Point0.X + Box->Rect.Point1.X) / 2.0f - TextDimensions.X / 2.0f);
            Result.X = CLAMP_BOT(Result.X, Box->Rect.X0);
        } break;

        case UI_TEXT_ALIGN_RIGHT: {
            v2f32 TextDimensions = Box->DisplayDimensions;

            Result.X = RoundF32((Box->Rect.Point1.X) - TextDimensions.X - Box->TextPadding);
            Result.X = CLAMP_BOT(Result.X, Box->Rect.X0);
        } break;
    }

    Result.X = FloorF32(Result.X);

    return Result;
}

u64 
UIBoxCharPositionFromXY(UIBox* Box, v2f32 XY)
{
    Str8 Line = UIBoxDisplayStr(Box);
    u64 Result = DCharPositionFromStr(Box->TabSize, Line, XY.X - UIBoxTextPosition(Box).X);

    return Result;
}

UISignal 
UISignalFromBox(UIBox* Box)
{
    UIState* UI_STATE = GetTLS()->UI_STATE;
    b32 IsFocusHot = Box->Kind & UI_BOX_KIND_FOCUS_HOT && !(Box->Kind & UI_BOX_KIND_FOCUS_HOT_DISABLED);
    UISignal Signal = {Box};

    Signal.EventKind |= GetModifiers();

    r2f32 Rect = Box->Rect;

    for (UIBox* B = Box->Parent; !IsUIBoxEmpty(B); B = B->Parent)
        if (B->Kind & UI_BOX_KIND_CLIP)
            Rect = Intersect(Rect, B->Rect);

    b32 ContextMenuIsAncestor = FALSE;

    for (UIBox* Parent = Box; !IsUIBoxEmpty(Parent); Parent = Parent->Parent) {
        if (Parent == UI_STATE->ContextMenuRoot) {
            ContextMenuIsAncestor = TRUE;
            break;
        }
    }

    r2f32 BlacklistRect = {};

    if (!ContextMenuIsAncestor && UI_STATE->ContextMenuOpen) {
        BlacklistRect = UI_STATE->ContextMenuRoot->Rect;
    }

    b32 ViewScrolled = FALSE;

    for (UIEvent* Event = 0; UINextEvent(&Event);) {
        b32 Taken = FALSE;
        v2f32 EventMouse = Event->Position;
        b32 EventMouseInBounds = !InRange(BlacklistRect, EventMouse) && InRange(Rect, EventMouse);
        UIMouseButtonKind EventMouseBtnKind = (
            Event->Input == INPUT_KIND_LEFT_MOUSE_BTN ? UI_MOUSE_BTN_KIND_LEFT
            : Event->Input == INPUT_KIND_MIDDLE_MOUSE_BTN ? UI_MOUSE_BTN_KIND_MIDDLE
            : Event->Input == INPUT_KIND_RIGHT_MOUSE_BTN  ? UI_MOUSE_BTN_KIND_RIGHT
            : UI_MOUSE_BTN_KIND_LEFT
        );

        b32 EventKeyIsMouse = (
            Event->Input == INPUT_KIND_LEFT_MOUSE_BTN ||
            Event->Input == INPUT_KIND_MIDDLE_MOUSE_BTN ||
            Event->Input == INPUT_KIND_RIGHT_MOUSE_BTN
        );

        Signal.EventKind |= Event->Modifiers;

        if (
            Box->Kind & UI_BOX_KIND_MOUSE_CLICKABLE && 
            Event->Kind == UI_EVENT_KIND_PRESS && 
            EventMouseInBounds && 
            EventKeyIsMouse
        ) {
            UI_STATE->HotBoxKey = Box->Key;
            UI_STATE->ActiveBoxKey[EventMouseBtnKind] = Box->Key;
            Signal.Kind |= (UI_SIGNAL_KIND_LEFT_PRESSED << EventMouseBtnKind);
            UI_STATE->DragStartMouse = Event->Position;

            if (
                (Box->Key == UI_STATE->PressKeyHistory[EventMouseBtnKind][0]) &&
                Event->TimestampUSecs - UI_STATE->PressTimestampHistoryUSecs[EventMouseBtnKind][0] <= 1000000 * GetTerminalProperties()->DoubleClickTime
            ) {
                Signal.Kind |= (UI_SIGNAL_KIND_LEFT_DOUBLE_CLICKED << EventMouseBtnKind);
            }

            if (
                (Box->Key == UI_STATE->PressKeyHistory[EventMouseBtnKind][0]) &&
                (Box->Key == UI_STATE->PressKeyHistory[EventMouseBtnKind][1]) &&
                Event->TimestampUSecs - UI_STATE->PressTimestampHistoryUSecs[EventMouseBtnKind][0] <= 1000000 * GetTerminalProperties()->DoubleClickTime &&
                UI_STATE->PressTimestampHistoryUSecs[EventMouseBtnKind][0] - UI_STATE->PressTimestampHistoryUSecs[EventMouseBtnKind][1] <= 1000000 * GetTerminalProperties()->DoubleClickTime
            ) {
                Signal.Kind |= (UI_SIGNAL_KIND_LEFT_TRIPLE_CLICKED << EventMouseBtnKind);
            }

            MemCpy(
                &UI_STATE->PressTimestampHistoryUSecs[EventMouseBtnKind][1],
                &UI_STATE->PressTimestampHistoryUSecs[EventMouseBtnKind][0],
                sizeof(UI_STATE->PressTimestampHistoryUSecs[EventMouseBtnKind][0]) * ARRAY_COUNT(UI_STATE->PressTimestampHistoryUSecs[EventMouseBtnKind]) - 1
            );

            MemCpy(
                &UI_STATE->PressKeyHistory[EventMouseBtnKind][1],
                &UI_STATE->PressKeyHistory[EventMouseBtnKind][0],
                sizeof(UI_STATE->PressKeyHistory[EventMouseBtnKind][0]) * ARRAY_COUNT(UI_STATE->PressKeyHistory[EventMouseBtnKind]) - 1
            );

            MemCpy(
                &UI_STATE->PressPositionHistory[EventMouseBtnKind][1],
                &UI_STATE->PressPositionHistory[EventMouseBtnKind][0],
                sizeof(UI_STATE->PressPositionHistory[EventMouseBtnKind][0]) * ARRAY_COUNT(UI_STATE->PressPositionHistory[EventMouseBtnKind]) - 1
            );

            UI_STATE->PressTimestampHistoryUSecs[EventMouseBtnKind][0] = Event->TimestampUSecs;
            UI_STATE->PressKeyHistory[EventMouseBtnKind][0] = Box->Key;
            UI_STATE->PressPositionHistory[EventMouseBtnKind][0] = EventMouse;
            Taken = TRUE;
        }

        if (
            Box->Kind & UI_BOX_KIND_MOUSE_CLICKABLE && 
            Event->Kind == UI_EVENT_KIND_RELEASE &&
            (UI_STATE->ActiveBoxKey[EventMouseBtnKind] == Box->Key) &&
            EventMouseInBounds &&
            EventKeyIsMouse
        ) {
            UI_STATE->ActiveBoxKey[EventMouseBtnKind] = EMPTY_UI_KEY_VALUE;
            Signal.Kind |= (UI_SIGNAL_KIND_LEFT_RELEASED << EventMouseBtnKind);
            Signal.Kind |= (UI_SIGNAL_KIND_LEFT_CLICKED << EventMouseBtnKind);
            Taken = TRUE;
        }

        if (
            Box->Kind & UI_BOX_KIND_MOUSE_CLICKABLE && 
            Event->Kind == UI_EVENT_KIND_RELEASE &&
            (UI_STATE->ActiveBoxKey[EventMouseBtnKind] == Box->Key) &&
            !EventMouseInBounds &&
            EventKeyIsMouse
        ) {
            UI_STATE->HotBoxKey = EMPTY_UI_KEY_VALUE;
            UI_STATE->ActiveBoxKey[EventMouseBtnKind] = EMPTY_UI_KEY_VALUE;
            Signal.Kind |= (UI_SIGNAL_KIND_LEFT_RELEASED << EventMouseBtnKind);
            Taken = TRUE;
        }

        if (
            Box->Kind & UI_BOX_KIND_KEYBOARD_CLICKABLE &&
            IsFocusHot &&
            Event->Kind == UI_EVENT_KIND_PRESS &&
            Event->Slot == UI_EVENT_ACTION_SLOT_ACCEPT
        ) {
            Signal.Kind |= UI_SIGNAL_KIND_KEYBOARD_PRESSED;
            Taken = TRUE;
        };

        if (
            IsFocusHot &&
            Event->Flags & UI_EVENT_FLAG_COPY &&
            !(Event->Flags & UI_EVENT_FLAG_DELETE) &&
            Box->Key != EMPTY_UI_KEY_VALUE
        ) {
            UI_STATE->ClipboardCopyKey = Box->Key;
            Taken = TRUE;
        }

        if (
            Box->Kind & UI_BOX_KIND_CLICKABLE &&
            Box->FastpathCodepoint &&
            Event->String.Size
        ) {
            b32 AncestorIsFocused = FALSE;

            for (UIBox* Parent = Box->Parent; !IsUIBoxEmpty(Parent); Parent = Parent->Parent) {
                if (Parent->Kind == UI_BOX_KIND_FOCUS_ACTIVE) {
                    AncestorIsFocused = TRUE;

                    if (
                        Parent->Kind & UI_BOX_KIND_FOCUS_ACTIVE_DISABLED ||
                        Parent->DefaultNavFocusActiveKey != EMPTY_UI_KEY_VALUE
                    ) {
                        AncestorIsFocused = FALSE;
                        break;
                    }
                }
            }

            if (AncestorIsFocused) {
                TempArena Scratch = GetScratch(NULL, 0);
                Str32 Insertion32 = Str32FromStr8(Scratch.MemPool, Event->String);

                if (Insertion32.Size == 1 && Insertion32.Str[0] == Box->FastpathCodepoint) {
                    Taken = TRUE;
                    Signal.Kind |= (UI_SIGNAL_KIND_CLICKED | UI_SIGNAL_KIND_PRESSED);
                }

                ReleaseScratch(Scratch);
            }
        }

        if (
            Box->Kind & UI_BOX_KIND_SCROLL &&
            Event->Kind == UI_EVENT_KIND_SCROLL &&
            (Event->Modifiers == 0 || Event->Modifiers == INPUT_MOD_KIND_SHIFT) &&
            EventMouseInBounds
        ) {
            v2f32 Delta = Event->DeltaF32;

            if (Event->Modifiers & INPUT_MOD_KIND_SHIFT)
                SWAP(f32, Delta.X, Delta.Y);

            v2i16 Delta16 = Vec((i16) (Delta.X / 30.0f), (i16) (Delta.Y / 30.0f));

            if (Delta.X > 0.0f && Delta16.X == 0)
                Delta16.X = 1;

            if (Delta.X < 0.0f && Delta16.X == 0)
                Delta16.X = -1;

            if (Delta.Y > 0.0f && Delta16.Y == 0)
                Delta16.Y = 1;

            if (Delta.Y < 0.0f && Delta16.Y == 0)
                Delta16.Y = -1;

            Signal.Scroll += Delta16;
            Taken = TRUE;
        }

        if (
            Box->Kind & UI_BOX_KIND_VIEW_SCROLL &&
            Box->FirstTouchedBuildIndex != Box->LastTouchedBuildIndex &&
            Event->Kind == UI_EVENT_KIND_SCROLL &&
            (Event->Modifiers == 0 || Event->Modifiers == INPUT_MOD_KIND_SHIFT) &&
            EventMouseInBounds
        ) {
            v2f32 Delta = Event->DeltaF32;

            if (Event->Modifiers & INPUT_MOD_KIND_SHIFT)
                SWAP(f32, Delta.X, Delta.Y);

            if (!(Box->Kind & UI_BOX_KIND_VIEW_SCROLL_X)) {
                if (Delta.Y == 0.0f)
                    Delta.Y = Delta.X;

                Delta.X = 0.0f;
            }

            if (!(Box->Kind & UI_BOX_KIND_VIEW_SCROLL_Y)) {
                if (Delta.X == 0.0f)
                    Delta.X = Delta.Y;

                Delta.Y = 0.0f;
            }

            Box->ViewOffsetTarget += Delta;
            ViewScrolled = TRUE;
            Taken = TRUE;
        }

        if (Taken)
            UIEatEvent(Event);
    }

    if (ViewScrolled && Box->Kind & UI_BOX_KIND_VIEW_CLAMP) {
        v2f32 MaxViewOffsetTarget = {
            CLAMP_BOT(0.0f, Box->ViewBounds.X - Box->FixedSize.X),
            CLAMP_BOT(0.0f, Box->ViewBounds.Y - Box->FixedSize.Y)
        };

        if (Box->Kind & UI_BOX_KIND_VIEW_CLAMP_X)
            Box->ViewOffsetTarget.X = CLAMP(0.0f, Box->ViewOffsetTarget.X, MaxViewOffsetTarget.X);

        if (Box->Kind & UI_BOX_KIND_VIEW_CLAMP_Y)
            Box->ViewOffsetTarget.Y = CLAMP(0.0f, Box->ViewOffsetTarget.Y, MaxViewOffsetTarget.Y);
    }

    if (Box->Kind & UI_BOX_KIND_MOUSE_CLICKABLE) {
        for (
            UIMouseButtonKind MouseBtnKind = (UIMouseButtonKind) 0; 
            MouseBtnKind < UI_MOUSE_BTN_KIND_COUNT; 
            MouseBtnKind = (UIMouseButtonKind) (MouseBtnKind + 1)
        ) {
            if (
                UI_STATE->ActiveBoxKey[MouseBtnKind] == Box->Key ||
                Signal.Kind & (UI_SIGNAL_KIND_LEFT_PRESSED << MouseBtnKind)
            ) {
                Signal.Kind |= (UI_SIGNAL_KIND_LEFT_DRAGGING << MouseBtnKind);
            }
        }
    }

    if (Box->Kind & UI_BOX_KIND_MOUSE_CLICKABLE) {
        for EACH_ENUM(UIMouseButtonKind, MouseBtnKind, UI_MOUSE_BTN_KIND_COUNT) {
            if (
                Signal.Kind & (UI_SIGNAL_KIND_LEFT_DRAGGING << MouseBtnKind) &&
                (UI_STATE->PressKeyHistory[MouseBtnKind][0] == Box->Key) &&
                (UI_STATE->PressKeyHistory[MouseBtnKind][1] == Box->Key) &&
                UI_STATE->PressTimestampHistoryUSecs[MouseBtnKind][0] - UI_STATE->PressTimestampHistoryUSecs[MouseBtnKind][1] <= 1000000 * GetTerminalProperties()->DoubleClickTime &&
                Length((UI_STATE->PressPositionHistory[MouseBtnKind][0] - UI_STATE->PressPositionHistory[MouseBtnKind][1])) < 2.0f
            ) {
                Signal.Kind |= (UI_SIGNAL_KIND_LEFT_DOUBLE_DRAGGING << MouseBtnKind);
            }
        }
    }

    if (Box->Kind & UI_BOX_KIND_MOUSE_CLICKABLE) {
        for EACH_ENUM(UIMouseButtonKind, MouseBtnKind, UI_MOUSE_BTN_KIND_COUNT) {
            if (
                Signal.Kind & (UI_SIGNAL_KIND_LEFT_DRAGGING << MouseBtnKind) &&
                (UI_STATE->PressKeyHistory[MouseBtnKind][0] == Box->Key) &&
                (UI_STATE->PressKeyHistory[MouseBtnKind][1] == Box->Key) &&
                (UI_STATE->PressKeyHistory[MouseBtnKind][2] == Box->Key) &&
                (UI_STATE->PressTimestampHistoryUSecs[MouseBtnKind][0] - UI_STATE->PressTimestampHistoryUSecs[MouseBtnKind][1] <= 1000000 * GetTerminalProperties()->DoubleClickTime) &&
                (UI_STATE->PressTimestampHistoryUSecs[MouseBtnKind][1] - UI_STATE->PressTimestampHistoryUSecs[MouseBtnKind][2] <= 1000000 * GetTerminalProperties()->DoubleClickTime) &&
                (Length((UI_STATE->PressPositionHistory[MouseBtnKind][0] - UI_STATE->PressPositionHistory[MouseBtnKind][1])) < 2.0f) &&
                (Length((UI_STATE->PressPositionHistory[MouseBtnKind][1] - UI_STATE->PressPositionHistory[MouseBtnKind][2])) < 2.0f)
            ) {
                Signal.Kind |= (UI_SIGNAL_KIND_LEFT_TRIPLE_DRAGGING << MouseBtnKind);
            }
        }
    }

    if (InRange(Rect, UI_STATE->Mouse) && !InRange(BlacklistRect, UI_STATE->Mouse))
        Signal.Kind |= UI_SIGNAL_KIND_MOUSE_OVER;

    if (
        Box->Kind & UI_BOX_KIND_MOUSE_CLICKABLE &&
        InRange(Rect, UI_STATE->Mouse) &&
        !InRange(BlacklistRect, UI_STATE->Mouse) &&
        ((UI_STATE->HotBoxKey == EMPTY_UI_KEY_VALUE) || (UI_STATE->HotBoxKey == Box->Key)) &&
        ((UI_STATE->ActiveBoxKey[UI_MOUSE_BTN_KIND_LEFT] == EMPTY_UI_KEY_VALUE) || (UI_STATE->ActiveBoxKey[UI_MOUSE_BTN_KIND_LEFT] == Box->Key)) &&
        ((UI_STATE->ActiveBoxKey[UI_MOUSE_BTN_KIND_MIDDLE] == EMPTY_UI_KEY_VALUE) || (UI_STATE->ActiveBoxKey[UI_MOUSE_BTN_KIND_MIDDLE] == Box->Key)) &&
        ((UI_STATE->ActiveBoxKey[UI_MOUSE_BTN_KIND_RIGHT] == EMPTY_UI_KEY_VALUE) || (UI_STATE->ActiveBoxKey[UI_MOUSE_BTN_KIND_RIGHT] == Box->Key))
    ) {
        UI_STATE->HotBoxKey = Box->Key;
        Signal.Kind |= UI_SIGNAL_KIND_HOVERING;
    }

    if (
        Box->Kind & UI_BOX_KIND_MOUSE_CLICKABLE &&
        InRange(Rect, UI_STATE->Mouse) &&
        !InRange(BlacklistRect, UI_STATE->Mouse) &&
        (Box->GroupKey != EMPTY_UI_KEY_VALUE)
    ) {
        for EACH_ENUM(UIMouseButtonKind, MouseBtnKind, UI_MOUSE_BTN_KIND_COUNT) {
            UIBox* ActiveBox = UIBoxFromKey(UI_STATE->ActiveBoxKey[MouseBtnKind]);

            if (ActiveBox->GroupKey == Box->GroupKey) {
                UI_STATE->HotBoxKey = Box->Key;
                UI_STATE->ActiveBoxKey[MouseBtnKind] = Box->Key;
                Signal.Kind |= (UI_SIGNAL_KIND_HOVERING | (UI_SIGNAL_KIND_DRAGGING << MouseBtnKind));
            }
        }
    }

    {
        if (
            Box->Kind & UI_BOX_KIND_DROP_SITE &&
            InRange(Rect, UI_STATE->Mouse) &&
            !InRange(BlacklistRect, UI_STATE->Mouse) &&
            ((UI_STATE->DropHotBoxKey == EMPTY_UI_KEY_VALUE) || (UI_STATE->DropHotBoxKey == Box->Key))
        ) {
            UI_STATE->DropHotBoxKey = Box->Key;
        }
    }

    {
        if (
            Box->Kind & UI_BOX_KIND_DROP_SITE &&
            (!InRange(Rect, UI_STATE->Mouse) || InRange(BlacklistRect, UI_STATE->Mouse)) &&
            (UI_STATE->DropHotBoxKey == Box->Key)
        ) {
            UI_STATE->DropHotBoxKey = EMPTY_UI_KEY_VALUE;
        }
    }

    if (
        !ContextMenuIsAncestor &&
        Signal.Kind & (UI_SIGNAL_KIND_LEFT_PRESSED | UI_SIGNAL_KIND_RIGHT_PRESSED | UI_SIGNAL_KIND_MIDDLE_PRESSED)
    ) {
        UIContextMenuClose();
    }

    UIBox* DefaultNavParent = EMPTY_UI_BOX_VALUE;

    for (UIBox* Parent = UIHeadParent(); !IsUIBoxEmpty(Parent); Parent = Parent->Parent) {
        if (Parent->Kind & UI_BOX_KIND_DEFAULT_FOCUS_NAV) {
            DefaultNavParent = Parent;
            break;
        }
    }

    if (
        Box->Kind & UI_BOX_KIND_CLICK_TO_FOCUS && 
        Signal.Kind & UI_SIGNAL_KIND_PRESSED && 
        !IsUIBoxEmpty(DefaultNavParent)
    ) {
        DefaultNavParent->DefaultNavFocusNextHotKey = Box->Key;

        if (DefaultNavParent->DefaultNavFocusActiveKey != Box->Key)
            DefaultNavParent->DefaultNavFocusNextActiveKey = EMPTY_UI_KEY_VALUE;
    }

    return Signal;
}

#define UI_STACK_HEAD(X, Y) (X)->Y##Stack.Head->V
#define UI_STACK_TAIL(X, Y) (X)->Y##Stack.TailValue

#define UI_STACK_PUSH(X, Y, Z, W)                                                       \
    UI##Y##Node* Node = (X)->Y##Stack.Free;                                             \
    if (Node) { SLL_STACK_POP((X)->Y##Stack.Free); }                                    \
    else { Node = ArenaPushArrayZero(UIBuildMemPool(), UI##Y##Node, 1); }               \
    Z OldVal = (X)->Y##Stack.Head->V;                                                   \
    Node->V = (W);                                                                      \
    SLL_STACK_PUSH((X)->Y##Stack.Head, Node);                                           \
    if (Node->Next == &(X)->Y##EmptyStackHead) { (X)->Y##Stack.TailValue = (Node->V); } \
    (X)->Y##Stack.AutoPop = FALSE;                                                      \
    ++(X)->Y##Stack.Generation;                                                         \
    return OldVal;

#define UI_STACK_POP(X, Y)                          \
    UI##Y##Node* Node = (X)->Y##Stack.Head;         \
    if (Node != &(X)->Y##EmptyStackHead) {          \
        SLL_STACK_POP((X)->Y##Stack.Head);          \
        SLL_STACK_PUSH((X)->Y##Stack.Free, Node);   \
        (X)->Y##Stack.AutoPop = FALSE;              \
        ++(X)->Y##Stack.Generation;                 \
    }                                               \
    return Node->V;

#define UI_STACK_SET_NEXT(X, Y, Z, W)                                       \
    UI##Y##Node* Node = (X)->Y##Stack.Free;                                 \
    if (Node) { SLL_STACK_POP((X)->Y##Stack.Free); }                        \
    else { Node = ArenaPushArrayZero(UIBuildMemPool(), UI##Y##Node, 1); }   \
    Z OldVal = (X)->Y##Stack.Head->V;                                       \
    Node->V = W;                                                            \
    SLL_STACK_PUSH((X)->Y##Stack.Head, Node);                               \
    (X)->Y##Stack.AutoPop = TRUE;                                           \
    ++(X)->Y##Stack.Generation;                                             \
    return OldVal;

void 
__UIPushTagsKeyFromAppendedStr(Str8 String)
{
    UIState* UI_STATE = GetTLS()->UI_STATE;
    b32 IsNewRoot = StrMatch("."_s8, String, 0);
    UIKey SeedKey = EMPTY_UI_KEY_VALUE;

    if (!IsNewRoot && UI_STATE->TagsKeyStackHead)
        SeedKey = UI_STATE->TagsKeyStackHead->Key;

    UIKey Key = SeedKey;

    if (!IsNewRoot && String.Size)
        Key = UIKeyFromStr(SeedKey, String);

    UITagsKeyStackNode* Node = UI_STATE->TagsKeyStackFree;

    if (Node)
        SLL_STACK_POP(UI_STATE->TagsKeyStackFree);
    else {
        Node = ArenaPushArrayZero(UIBuildMemPool(), UITagsKeyStackNode, 1);
    }

    SLL_STACK_PUSH(UI_STATE->TagsKeyStackHead, Node);
    Node->Key = Key;

    if (!IsNewRoot) {
        u64 SlotIndex = *Key.V % UI_STATE->TagsCacheSlotsCount;
        UITagsCacheSlot* Slot = &UI_STATE->TagsCacheSlots[SlotIndex];
        UITagsCacheNode* Node = NULL;

        for (UITagsCacheNode* N = Slot->Head; N; N = N->Next) {
            if (N->Key == Key) {
                Node = N;
                break;
            }
        }

        if (!Node) {
            TempArena Scratch = GetScratch(NULL, 0);
            Str8List Tags = {};

            if (String.Size) {
                ListPush(Scratch.MemPool, &Tags, ArenaPushStrCpy(UIBuildMemPool(), String));
            }

            for (UITagNode* N = UI_STATE->TagStack.Head; N; N = N->Next) {
                if (N->V.Size == 1 && *N->V.Str == '.')
                    break;

                if (N->V.Size) {
                    ListPush(Scratch.MemPool, &Tags, ArenaPushStrCpy(UIBuildMemPool(), N->V));
                }
            }

            Node = ArenaPushArrayZero(UIBuildMemPool(), UITagsCacheNode, 1);
            SLL_QUEUE_PUSH(Slot->Head, Slot->Tail, Node);
            Node->Key = Key;
            Node->Tags = StrArrayFromList(UIBuildMemPool(), &Tags);
            ReleaseScratch(Scratch);
        }
    }
}

void 
__UIPopTagsKey(void)
{
    UIState* UI_STATE = GetTLS()->UI_STATE;

    if (UI_STATE->TagsKeyStackHead) {
        UITagsKeyStackNode* Node = UI_STATE->TagsKeyStackHead;
        
        SLL_STACK_POP(UI_STATE->TagsKeyStackHead);
        SLL_STACK_PUSH(UI_STATE->TagsKeyStackFree, Node);
    }
}

r2f32 
UIPushRect(r2f32 Rect)
{
    r2f32 Result = {};
    v2f32 Size = Length(Rect);

    Result.X0 = UIPushFixedX(Rect.X0);
    Result.Y0 = UIPushFixedY(Rect.Y0);
    Result.X1 = Result.X0 + UIPushFixedWidth(Size.X);
    Result.Y1 = Result.Y0 + UIPushFixedHeight(Size.Y);

    return Result;
}

r2f32 
UIPopRect(void)
{
    r2f32 Result = {};

    Result.X0 = UIPopFixedX();
    Result.Y0 = UIPopFixedY();
    Result.X1 = Result.X0 + UIPopFixedWidth();
    Result.Y1 = Result.Y0 + UIPopFixedHeight();

    return Result;
}

void 
UISetNextRect(r2f32 Rect)
{
    v2f32 Size = Length(Rect);

    UISetNextFixedX(Rect.X0);
    UISetNextFixedY(Rect.Y0);
    UISetNextFixedWidth(Size.X);
    UISetNextFixedHeight(Size.Y);
}

UISize 
UIPushPreferredSize(Axis2D Axis, UISize Size)
{
    UISize Result = {};

    switch (Axis) {
        default: {} break;

        case AXIS_2D_X: {
            Result = UIPushPreferredWidth(Size);
        } break;

        case AXIS_2D_Y: {
            Result = UIPushPreferredHeight(Size);
        } break;
    }

    return Result;
}

UISize 
UIPopPreferredSize(Axis2D Axis)
{
    UISize Result = {};

    switch (Axis) {
        default: {} break;

        case AXIS_2D_X: {
            Result = UIPopPreferredWidth();
        } break;

        case AXIS_2D_Y: {
            Result = UIPopPreferredHeight();
        } break;
    }

    return Result;
}

UISize 
UISetNextPreferredSize(Axis2D Axis, UISize Size)
{
    return (Axis == AXIS_2D_X) ? UISetNextPreferredWidth(Size) : UISetNextPreferredHeight(Size);
}

void 
UIPushTagFmt(char* Fmt, ...)
{
    TempArena Scratch = GetScratch(NULL, 0);
    va_list Args;

    va_start(Args, Fmt);

    Str8 String = ArenaPushStrFmtV(Scratch.MemPool, Fmt, Args);

    UIPushTag(String);
    va_end(Args);
    ReleaseScratch(Scratch);
}

UIBox* 
UIHeadParent(void)
{
    return UI_STACK_HEAD(GetTLS()->UI_STATE, Parent);
}

Axis2D 
UIHeadChildLayoutAxis(void)
{
    return UI_STACK_HEAD(GetTLS()->UI_STATE, ChildLayoutAxis);
}

f32 
UIHeadFixedX(void)
{
    return UI_STACK_HEAD(GetTLS()->UI_STATE, FixedX);
}

f32 
UIHeadFixedY(void)
{
    return UI_STACK_HEAD(GetTLS()->UI_STATE, FixedY);
}

f32 
UIHeadFixedWidth(void)
{
    return UI_STACK_HEAD(GetTLS()->UI_STATE, FixedWidth);
}

f32 
UIHeadFixedHeight(void)
{
    return UI_STACK_HEAD(GetTLS()->UI_STATE, FixedHeight);
}

UISize 
UIHeadPreferredWidth(void)
{
    return UI_STACK_HEAD(GetTLS()->UI_STATE, PreferredWidth);
}

UISize 
UIHeadPreferredHeight(void)
{
    return UI_STACK_HEAD(GetTLS()->UI_STATE, PreferredHeight);
}

f32 
UIHeadMinWidth(void)
{
    return UI_STACK_HEAD(GetTLS()->UI_STATE, MinWidth);
}

f32 
UIHeadMinHeight(void)
{
    return UI_STACK_HEAD(GetTLS()->UI_STATE, MinHeight);
}

UIPermissionKind 
UIHeadPermissionKinds(void)
{
    return UI_STACK_HEAD(GetTLS()->UI_STATE, PermissionKinds);
}

UIBoxKind 
UIHeadFlags(void)
{
    return UI_STACK_HEAD(GetTLS()->UI_STATE, Flags);
}

UIBoxKind 
UIHeadOmitFlags(void)
{
    return UI_STACK_HEAD(GetTLS()->UI_STATE, OmitFlags);
}

UIFocusKind 
UIHeadFocusHot(void)
{
    return UI_STACK_HEAD(GetTLS()->UI_STATE, FocusHot);
}

UIFocusKind 
UIHeadFocusActive(void)
{
    return UI_STACK_HEAD(GetTLS()->UI_STATE, FocusActive);
}

u32 
UIHeadFastpathCodepoint(void)
{
    return UI_STACK_HEAD(GetTLS()->UI_STATE, FastpathCodepoint);
}

UIKey 
UIHeadGroupKey(void)
{
    return UI_STACK_HEAD(GetTLS()->UI_STATE, GroupKey);
}

Str8 
UIHeadTag(void)
{
    return UI_STACK_HEAD(GetTLS()->UI_STATE, Tag);
}

v4f32 
UIHeadBackgroundColour(void)
{
    return UI_STACK_HEAD(GetTLS()->UI_STATE, BackgroundColour);
}

v4f32 
UIHeadTextColour(void)
{
    return UI_STACK_HEAD(GetTLS()->UI_STATE, TextColour);
}

v4f32 
UIHeadBorderColour(void)
{
    return UI_STACK_HEAD(GetTLS()->UI_STATE, BorderColour);
}

CursorKind 
UIHeadHoverCursor(void)
{
    return UI_STACK_HEAD(GetTLS()->UI_STATE, HoverCursor);
}

f32 
UIHeadTabSize(void)
{
    return UI_STACK_HEAD(GetTLS()->UI_STATE, TabSize);
}

f32 
UIHeadTextPadding(void)
{
    return UI_STACK_HEAD(GetTLS()->UI_STATE, TextPadding);
}

UITextAlign 
UIHeadTextAlignment(void)
{
    return UI_STACK_HEAD(GetTLS()->UI_STATE, TextAlignment);
}

UIBox* 
UITailParent(void)
{
    return UI_STACK_TAIL(GetTLS()->UI_STATE, Parent);
}

Axis2D 
UITailChildLayoutAxis(void)
{
    return UI_STACK_TAIL(GetTLS()->UI_STATE, ChildLayoutAxis);
}

f32 
UITailFixedX(void)
{
    return UI_STACK_TAIL(GetTLS()->UI_STATE, FixedX);
}

f32 
UITailFixedY(void)
{
    return UI_STACK_TAIL(GetTLS()->UI_STATE, FixedY);
}

f32 
UITailFixedWidth(void)
{
    return UI_STACK_TAIL(GetTLS()->UI_STATE, FixedWidth);
}

f32 
UITailFixedHeight(void)
{
    return UI_STACK_TAIL(GetTLS()->UI_STATE, FixedHeight);
}

UISize 
UITailPreferredWidth(void)
{
    return UI_STACK_TAIL(GetTLS()->UI_STATE, PreferredWidth);
}

UISize 
UITailPreferredHeight(void)
{
    return UI_STACK_TAIL(GetTLS()->UI_STATE, PreferredHeight);
}

f32 
UITailMinWidth(void)
{
    return UI_STACK_TAIL(GetTLS()->UI_STATE, MinWidth);
}

f32 
UITailMinHeight(void)
{
    return UI_STACK_TAIL(GetTLS()->UI_STATE, MinHeight);
}

UIPermissionKind 
UITailPermissionKinds(void)
{
    return UI_STACK_TAIL(GetTLS()->UI_STATE, PermissionKinds);
}

UIBoxKind 
UITailFlags(void)
{
    return UI_STACK_TAIL(GetTLS()->UI_STATE, Flags);
}

UIBoxKind 
UITailOmitFlags(void)
{
    return UI_STACK_TAIL(GetTLS()->UI_STATE, OmitFlags);
}

UIFocusKind 
UITailFocusHot(void)
{
    return UI_STACK_TAIL(GetTLS()->UI_STATE, FocusHot);
}

UIFocusKind 
UITailFocusActive(void)
{
    return UI_STACK_TAIL(GetTLS()->UI_STATE, FocusActive);
}

u32 
UITailFastpathCodepoint(void)
{
    return UI_STACK_TAIL(GetTLS()->UI_STATE, FastpathCodepoint);
}

UIKey 
UITailGroupKey(void)
{
    return UI_STACK_TAIL(GetTLS()->UI_STATE, GroupKey);
}

Str8 
UITailTag(void)
{
    return UI_STACK_TAIL(GetTLS()->UI_STATE, Tag);
}

v4f32 
UITailBackgroundColour(void)
{
    return UI_STACK_TAIL(GetTLS()->UI_STATE, BackgroundColour);
}

v4f32 
UITailTextColour(void)
{
    return UI_STACK_TAIL(GetTLS()->UI_STATE, TextColour);
}

v4f32 
UITailBorderColour(void)
{
    return UI_STACK_TAIL(GetTLS()->UI_STATE, BorderColour);
}

CursorKind 
UITailHoverCursor(void)
{
    return UI_STACK_TAIL(GetTLS()->UI_STATE, HoverCursor);
}

f32 
UITailTabSize(void)
{
    return UI_STACK_TAIL(GetTLS()->UI_STATE, TabSize);
}

f32 
UITailTextPadding(void)
{
    return UI_STACK_TAIL(GetTLS()->UI_STATE, TextPadding);
}

UITextAlign 
UITailTextAlignment(void)
{
    return UI_STACK_TAIL(GetTLS()->UI_STATE, TextAlignment);
}

UIBox* 
UIPushParent(UIBox* Value)
{
    UI_STACK_PUSH(GetTLS()->UI_STATE, Parent, UIBox*, Value);
}

Axis2D 
UIPushChildLayoutAxis(Axis2D Value)
{
    UI_STACK_PUSH(GetTLS()->UI_STATE, ChildLayoutAxis, Axis2D, Value);
}

f32 
UIPushFixedX(f32 Value)
{
    UI_STACK_PUSH(GetTLS()->UI_STATE, FixedX, f32, Value);
}

f32 
UIPushFixedY(f32 Value)
{
    UI_STACK_PUSH(GetTLS()->UI_STATE, FixedY, f32, Value);
}

f32 
UIPushFixedWidth(f32 Value)
{
    UI_STACK_PUSH(GetTLS()->UI_STATE, FixedWidth, f32, Value);
}

f32 
UIPushFixedHeight(f32 Value)
{
    UI_STACK_PUSH(GetTLS()->UI_STATE, FixedHeight, f32, Value);
}

UISize 
UIPushPreferredWidth(UISize Value)
{
    UI_STACK_PUSH(GetTLS()->UI_STATE, PreferredWidth, UISize, Value);
}

UISize 
UIPushPreferredHeight(UISize Value)
{
    UI_STACK_PUSH(GetTLS()->UI_STATE, PreferredHeight, UISize, Value);
}

f32 
UIPushMinWidth(f32 Value)
{
    UI_STACK_PUSH(GetTLS()->UI_STATE, MinWidth, f32, Value);
}

f32 
UIPushMinHeight(f32 Value)
{
    UI_STACK_PUSH(GetTLS()->UI_STATE, MinHeight, f32, Value);
}

UIPermissionKind 
UIPushPermissionKind(UIPermissionKind Value)
{
    UI_STACK_PUSH(GetTLS()->UI_STATE, PermissionKinds, UIPermissionKind, Value);
}

UIBoxKind 
UIPushFlags(UIBoxKind Value)
{
    UI_STACK_PUSH(GetTLS()->UI_STATE, Flags, UIBoxKind, Value);
}

UIBoxKind 
UIPushOmitFlags(UIBoxKind Value)
{
    UI_STACK_PUSH(GetTLS()->UI_STATE, OmitFlags, UIBoxKind, Value);
}

UIFocusKind 
UIPushFocusHot(UIFocusKind Value)
{
    UI_STACK_PUSH(GetTLS()->UI_STATE, FocusHot, UIFocusKind, Value);
}

UIFocusKind 
UIPushFocusActive(UIFocusKind Value)
{
    UI_STACK_PUSH(GetTLS()->UI_STATE, FocusActive, UIFocusKind, Value);
}

u32 
UIPushFastpathCodepoint(u32 Value)
{
    UI_STACK_PUSH(GetTLS()->UI_STATE, FastpathCodepoint, u32, Value);
}

UIKey 
UIPushGroupKey(UIKey Value)
{
    UI_STACK_PUSH(GetTLS()->UI_STATE, GroupKey, UIKey, Value);
}

Str8 
UIPushTag(Str8 Value)
{
    __UIPushTagsKeyFromAppendedStr(Value);
    UI_STACK_PUSH(GetTLS()->UI_STATE, Tag, Str8, ArenaPushStrCpy(UIBuildMemPool(), Value));
}

v4f32 
UIPushBackgroundColour(v4f32 Value)
{
    UI_STACK_PUSH(GetTLS()->UI_STATE, BackgroundColour, v4f32, Value);
}

v4f32 
UIPushTextColour(v4f32 Value)
{
    UI_STACK_PUSH(GetTLS()->UI_STATE, TextColour, v4f32, Value);
}

v4f32 
UIPushBorderColour(v4f32 Value)
{
    UI_STACK_PUSH(GetTLS()->UI_STATE, BorderColour, v4f32, Value);
}

CursorKind 
UIPushHoverCursor(CursorKind Value)
{
    UI_STACK_PUSH(GetTLS()->UI_STATE, HoverCursor, CursorKind, Value);
}

f32 
UIPushTabSize(f32 Value)
{
    UI_STACK_PUSH(GetTLS()->UI_STATE, TabSize, f32, Value);
}

f32 
UIPushTextPadding(f32 Value)
{
    UI_STACK_PUSH(GetTLS()->UI_STATE, TextPadding, f32, Value);
}

UITextAlign 
UIPushTextAlignment(UITextAlign Value)
{
    UI_STACK_PUSH(GetTLS()->UI_STATE, TextAlignment, UITextAlign, Value);
}

UIBox* 
UIPopParent(void)
{
    UI_STACK_POP(GetTLS()->UI_STATE, Parent);
}

Axis2D 
UIPopChildLayoutAxis(void)
{
    UI_STACK_POP(GetTLS()->UI_STATE, ChildLayoutAxis);
}

f32 
UIPopFixedX(void)
{
    UI_STACK_POP(GetTLS()->UI_STATE, FixedX);
}

f32 
UIPopFixedY(void)
{
    UI_STACK_POP(GetTLS()->UI_STATE, FixedY);
}

f32 
UIPopFixedWidth(void)
{
    UI_STACK_POP(GetTLS()->UI_STATE, FixedWidth);
}

f32 
UIPopFixedHeight(void)
{
    UI_STACK_POP(GetTLS()->UI_STATE, FixedHeight);
}

UISize 
UIPopPreferredWidth(void)
{
    UI_STACK_POP(GetTLS()->UI_STATE, PreferredWidth);
}

UISize 
UIPopPreferredHeight(void)
{
    UI_STACK_POP(GetTLS()->UI_STATE, PreferredHeight);
}

f32 
UIPopMinWidth(void)
{
    UI_STACK_POP(GetTLS()->UI_STATE, MinWidth);
}

f32 
UIPopMinHeight(void)
{
    UI_STACK_POP(GetTLS()->UI_STATE, MinHeight);
}

UIPermissionKind 
UIPopPermissionKind(void)
{
    UI_STACK_POP(GetTLS()->UI_STATE, PermissionKinds);
}

UIBoxKind 
UIPopFlags(void)
{
    UI_STACK_POP(GetTLS()->UI_STATE, Flags);
}

UIBoxKind 
UIPopOmitFlags(void)
{
    UI_STACK_POP(GetTLS()->UI_STATE, OmitFlags);
}

UIFocusKind 
UIPopFocusHot(void)
{
    UI_STACK_POP(GetTLS()->UI_STATE, FocusHot);
}

UIFocusKind 
UIPopFocusActive(void)
{
    UI_STACK_POP(GetTLS()->UI_STATE, FocusActive);
}

u32 
UIPopFastpathCodepoint(void)
{
    UI_STACK_POP(GetTLS()->UI_STATE, FastpathCodepoint);
}

UIKey 
UIPopGroupKey(void)
{
    UI_STACK_POP(GetTLS()->UI_STATE, GroupKey);
}

Str8 
UIPopTag(void)
{
    __UIPopTagsKey();
    UI_STACK_POP(GetTLS()->UI_STATE, Tag);
}

v4f32 
UIPopBackgroundColour(void)
{
    UI_STACK_POP(GetTLS()->UI_STATE, BackgroundColour);
}

v4f32 
UIPopTextColour(void)
{
    UI_STACK_POP(GetTLS()->UI_STATE, TextColour);
}

v4f32 
UIPopBorderColour(void)
{
    UI_STACK_POP(GetTLS()->UI_STATE, BorderColour);
}

CursorKind 
UIPopHoverCursor(void)
{
    UI_STACK_POP(GetTLS()->UI_STATE, HoverCursor);
}

f32 
UIPopTabSize(void)
{
    UI_STACK_POP(GetTLS()->UI_STATE, TabSize);
}

f32 
UIPopTextPadding(void)
{
    UI_STACK_POP(GetTLS()->UI_STATE, TextPadding);
}

UITextAlign 
UIPopTextAlignment(void)
{
    UI_STACK_POP(GetTLS()->UI_STATE, TextAlignment);
}

UIBox* 
UISetNextParent(UIBox* Value)
{
    UI_STACK_SET_NEXT(GetTLS()->UI_STATE, Parent, UIBox*, Value);
}

Axis2D 
UISetNextChildLayoutAxis(Axis2D Value)
{
    UI_STACK_SET_NEXT(GetTLS()->UI_STATE, ChildLayoutAxis, Axis2D, Value);
}

f32 
UISetNextFixedX(f32 Value)
{
    UI_STACK_SET_NEXT(GetTLS()->UI_STATE, FixedX, f32, Value);
}

f32 
UISetNextFixedY(f32 Value)
{
    UI_STACK_SET_NEXT(GetTLS()->UI_STATE, FixedY, f32, Value);
}

f32 
UISetNextFixedWidth(f32 Value)
{
    UI_STACK_SET_NEXT(GetTLS()->UI_STATE, FixedWidth, f32, Value);
}

f32 
UISetNextFixedHeight(f32 Value)
{
    UI_STACK_SET_NEXT(GetTLS()->UI_STATE, FixedHeight, f32, Value);
}

UISize 
UISetNextPreferredWidth(UISize Value)
{
    UI_STACK_SET_NEXT(GetTLS()->UI_STATE, PreferredWidth, UISize, Value);
}

UISize 
UISetNextPreferredHeight(UISize Value)
{
    UI_STACK_SET_NEXT(GetTLS()->UI_STATE, PreferredHeight, UISize, Value);
}

f32 
UISetNextMinWidth(f32 Value)
{
    UI_STACK_SET_NEXT(GetTLS()->UI_STATE, MinWidth, f32, Value);
}

f32 
UISetNextMinHeight(f32 Value)
{
    UI_STACK_SET_NEXT(GetTLS()->UI_STATE, MinHeight, f32, Value);
}

UIPermissionKind 
UISetNextPermissionKinds(UIPermissionKind Value)
{
    UI_STACK_SET_NEXT(GetTLS()->UI_STATE, PermissionKinds, UIPermissionKind, Value);
}

UIBoxKind 
UISetNextFlags(UIBoxKind Value)
{
    UI_STACK_SET_NEXT(GetTLS()->UI_STATE, Flags, UIBoxKind, Value);
}

UIBoxKind 
UISetNextOmitFlags(UIBoxKind Value)
{
    UI_STACK_SET_NEXT(GetTLS()->UI_STATE, OmitFlags, UIBoxKind, Value);
}

UIFocusKind 
UISetNextFocusHot(UIFocusKind Value)
{
    UI_STACK_SET_NEXT(GetTLS()->UI_STATE, FocusHot, UIFocusKind, Value);
}

UIFocusKind 
UISetNextFocusActive(UIFocusKind Value)
{
    UI_STACK_SET_NEXT(GetTLS()->UI_STATE, FocusActive, UIFocusKind, Value);
}

u32 
UISetNextFastpathCodepoint(u32 Value)
{
    UI_STACK_SET_NEXT(GetTLS()->UI_STATE, FastpathCodepoint, u32, Value);
}

UIKey 
UISetNextGroupKey(UIKey Value)
{
    UI_STACK_SET_NEXT(GetTLS()->UI_STATE, GroupKey, UIKey, Value);
}

Str8 
UISetNextTag(Str8 Value)
{
    __UIPushTagsKeyFromAppendedStr(Value);
    UI_STACK_SET_NEXT(GetTLS()->UI_STATE, Tag, Str8, ArenaPushStrCpy(UIBuildMemPool(), Value));
}

v4f32 
UISetNextBackgroundColour(v4f32 Value)
{
    UI_STACK_SET_NEXT(GetTLS()->UI_STATE, BackgroundColour, v4f32, Value);
}

v4f32 
UISetNextTextColour(v4f32 Value)
{
    UI_STACK_SET_NEXT(GetTLS()->UI_STATE, TextColour, v4f32, Value);
}

v4f32 
UISetNextBorderColour(v4f32 Value)
{
    UI_STACK_SET_NEXT(GetTLS()->UI_STATE, BorderColour, v4f32, Value);
}

CursorKind 
UISetNextHoverCursor(CursorKind Value)
{
    UI_STACK_SET_NEXT(GetTLS()->UI_STATE, HoverCursor, CursorKind, Value);
}

f32 
UISetNextTabSize(f32 Value)
{
    UI_STACK_SET_NEXT(GetTLS()->UI_STATE, TabSize, f32, Value);
}

f32 
UISetNextTextPadding(f32 Value)
{
    UI_STACK_SET_NEXT(GetTLS()->UI_STATE, TextPadding, f32, Value);
}

UITextAlign 
UISetNextTextAlignment(UITextAlign Value)
{
    UI_STACK_SET_NEXT(GetTLS()->UI_STATE, TextAlignment, UITextAlign, Value);
}

void
UIBoxEquipDisplayStr(UIBox* Box, Str8 String)
{
    Box->String = ArenaPushStrCpy(UIBuildMemPool(), String);
    Box->Kind |= UI_BOX_KIND_HAS_DISPLAY_STRING;

    v4f32 TextColour = Box->TextColour;

    if (
        Box->Kind & UI_BOX_KIND_DRAW_TEXT && 
        (
            !Box->FastpathCodepoint || 
            !(Box->Kind & UI_BOX_KIND_DRAW_TEXT_FASTPATH_CODEPOINT)
        )
    ) {
        Str8 DisplayString = UIBoxDisplayStr(Box);
        FancyStrNode FStringNode = {
            NULL,
            {
                DisplayString,
                {
                    Box->TextColour,
                    0
                }
            }
        };

        FancyStrList FStrings = {
            &FStringNode,
            &FStringNode,
            1
        };

        Box->DisplayFancyStrings = ListCpy(UIBuildMemPool(), &FStrings);
        Box->DisplayDimensions = DDimensionsFromFancyStrList(
            Box->TabSize, 
            &Box->DisplayFancyStrings
        );
    } else if (
        Box->Kind & UI_BOX_KIND_DRAW_TEXT && 
        Box->Kind & UI_BOX_KIND_DRAW_TEXT_FASTPATH_CODEPOINT && 
        Box->FastpathCodepoint
    ) {
        TempArena Scratch = GetScratch(NULL, 0);
        Str8 DisplayString = UIBoxDisplayStr(Box);
        Str32 FastpathCodepoint32 = Str(&Box->FastpathCodepoint, 1);
        Str8 FastpathCodepoint = Str8FromStr32(Scratch.MemPool, FastpathCodepoint32);
        u64 FastpathCodepointPosition = StrFindSubStr(DisplayString, FastpathCodepoint, 0, STR_MATCH_ALL_CASES);

        if (FastpathCodepointPosition < DisplayString.Size) {
            FancyStrNode FStringNode = {
                NULL,
                {
                    StrSkip(DisplayString, FastpathCodepointPosition + FastpathCodepoint.Size),
                    {
                        Box->TextColour,
                        0
                    }
                }
            };

            FancyStrNode FancyCodePointNode = {
                &FStringNode,
                {
                    StrSub(DisplayString, Rng(FastpathCodepointPosition, FastpathCodepointPosition + FastpathCodepoint.Size)),
                    {
                        Box->TextColour,
                        FANCY_STR_FLAG_UNDERLINE
                    }
                }
            };

            FancyStrNode FancyPreNode = {
                &FancyCodePointNode,
                {
                    StrPrefix(DisplayString, FastpathCodepointPosition),
                    {
                        Box->TextColour,
                        0
                    }
                }
            };

            FancyStrList FancyStrings = {
                &FancyPreNode,
                &FStringNode,
                3
            };

            Box->DisplayFancyStrings = ListCpy(UIBuildMemPool(), &FancyStrings);
            Box->DisplayDimensions = DDimensionsFromFancyStrList(
                Box->TabSize, 
                &Box->DisplayFancyStrings
            );
        } else {
            FancyStrNode FStringNode = {
                NULL,
                {
                    DisplayString,
                    {
                        Box->TextColour,
                        0
                    }
                }
            };

            FancyStrList FancyStrings = {
                &FStringNode,
                &FStringNode,
                1
            };

            Box->DisplayFancyStrings = ListCpy(UIBuildMemPool(), &FancyStrings);
            Box->DisplayDimensions = DDimensionsFromFancyStrList(Box->TabSize, &Box->DisplayFancyStrings);
        }

        ReleaseScratch(Scratch);
    }
}

void
UIBoxEquipDisplayFancyStrs(UIBox* Box, FancyStrList* Strings)
{
    Box->Kind |= UI_BOX_KIND_HAS_DISPLAY_STRING;
    Box->String = Str8FromFancyStrList(UIBuildMemPool(), Strings);
    Box->DisplayFancyStrings = ListCpy(UIBuildMemPool(), Strings);
    Box->DisplayDimensions = DDimensionsFromFancyStrList(Box->TabSize, &Box->DisplayFancyStrings);
}

void
UIBoxEquipFuzzyMatchRanges(UIBox* Box, FMRangeList* Matches)
{
    Box->Kind |= UI_BOX_KIND_HAS_FUZZYMATCH_RANGES;
    Box->FuzzyMatchRanges = ListCpy(
        UIBuildMemPool(),
        Matches
    );
}

void
UIBoxEquipCustomDraw(
    UIBox* Box, 
    UIBoxCustomDrawFuncType* CustomDrawFunc, 
    void* UserData
) {
    Box->CustomDrawFunc = CustomDrawFunc;
    Box->CustomDrawUserData = UserData;
}

b32
IsUIBoxEmpty(UIBox* Box)
{
    return (!Box || Box == EMPTY_UI_BOX_VALUE);
}

#if defined(NC_LANG_CPP)
    b32 
    operator==(UIKey KeyA, UIKey KeyB)
    {
        return (*KeyA.V == *KeyB.V);
    }

    b32 
    operator!=(UIKey KeyA, UIKey KeyB)
    {
        return !(KeyA == KeyB);
    }
#endif
