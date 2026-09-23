#include <stdarg.h>

#include "nc_ui_components.h"
#include "nc_draw.h"
#include "nc_memory.h"
#include "nc_time.h"
#include "nc_tls.h"

INTERNAL
UI_BOX_CUSTOM_DRAW(UIDividerDraw)
{
    UIDividerDrawData* Data = (UIDividerDrawData*) UserData;
    Axis2D CrossAxis = FLIP_AXIS(Data->Axis);
    v2f32 Point0 = Box->Rect.Point0;
    v2f32 Point1 = Box->Rect.Point1;
    f32 DivCentre = FloorF32(
        (Point0.V[CrossAxis] + Point1.V[CrossAxis]) / 2.0f
    );

    Point0.V[CrossAxis] = DivCentre;
    Point1.V[CrossAxis] = DivCentre;
    DrawLine(
        Point0,
        Point1,
        UIColourFromTagsKeyName(Box->TagsKey, "border"_s8)
    );
}

void
UIDivider(UISize Size)
{
    UIBox* Parent = UIHeadParent();

    UISetNextPreferredSize(Parent->ChildLayoutAxis, Size);
    UISetNextPreferredSize(
        FLIP_AXIS(Parent->ChildLayoutAxis), 
        UI_PERCENT(1.0f, 0.0f)
    );

    UIBox* Box = UIBuildBoxFromKey(0, EMPTY_UI_KEY_VALUE);
    UIDividerDrawData* DrawData = ArenaPushArrayZero(
        UIBuildMemPool(),
        UIDividerDrawData,
        1
    );

    DrawData->Axis = FLIP_AXIS(Parent->ChildLayoutAxis);
    UIBoxEquipCustomDraw(Box, UIDividerDraw, DrawData);
}

UISignal
UILabel(Str8 String)
{
    UIBox* Box = UIBuildBoxFromStr(UI_BOX_KIND_DRAW_TEXT, String);

    UIBoxEquipDisplayStr(Box, String);

    UISignal Result = UISignalFromBox(Box);

    return Result;
}

UISignal
UILabel(char* Fmt, ...)
{
    TempArena Scratch = GetScratch(NULL, 0);
    va_list Args;

    va_start(Args, Fmt);

    Str8 String = ArenaPushStrFmtV(
        Scratch.MemPool,
        Fmt,
        Args
    );

    va_end(Args);

    UISignal Result = UILabel(String);

    ReleaseScratch(Scratch);

    return Result;
}

void 
UILabelMultiline(f32 Max, Str8 String)
{
    TempArena Scratch = GetScratch(NULL, 0);

    UISetNextChildLayoutAxis(AXIS_2D_Y);
    UISetNextPreferredHeight(UI_SUM_OF_CHILDREN(1.0f));

    UIBox* Box = UIBuildBoxFromKey(0, EMPTY_UI_KEY_VALUE);
    Str8List Lines = DWrappedLinesFromStr(
        Scratch.MemPool,
        UIHeadTabSize(),
        String,
        Max
    );

    for (
        Str8Node* Node = Lines.Head;
        Node;
        Node = Node->Next
    ) {
        UILabel(Node->String);
    }

    ReleaseScratch(Scratch);
}

void 
UILabelMultiline(f32 Max, char* Fmt, ...)
{
    TempArena Scratch = GetScratch(NULL, 0);
    va_list Args;

    va_start(Args, Fmt);

    Str8 String = ArenaPushStrFmtV(
        Scratch.MemPool,
        Fmt,
        Args
    );

    va_end(Args);
    UILabelMultiline(Max, String);
    ReleaseScratch(Scratch);
}

UISignal 
UIButton(Str8 String)
{
    UIBox* Box = UIBuildBoxFromStr(
        (
            UI_BOX_KIND_CLICKABLE |
            UI_BOX_KIND_DRAW_BACKGROUND |
            UI_BOX_KIND_DRAW_TEXT |
            UI_BOX_KIND_DRAW_HOT_EFFECTS |
            UI_BOX_KIND_DRAW_ACTIVE_EFFECTS
        ),
        String
    );
    UISignal Result = UISignalFromBox(Box);

    return Result;
}

UISignal 
UIButton(char* Fmt, ...)
{
    TempArena Scratch = GetScratch(NULL, 0);
    va_list Args;

    va_start(Args, Fmt);

    Str8 String = ArenaPushStrFmtV(
        Scratch.MemPool,
        Fmt,
        Args
    );

    va_end(Args);

    UISignal Result = UIButton(String);

    ReleaseScratch(Scratch);

    return Result;
}

UISignal 
UIHoverLabel(Str8 String)
{
    UIBox* Box = UIBuildBoxFromStr(
        (
            UI_BOX_KIND_CLICKABLE | 
            UI_BOX_KIND_DRAW_TEXT | 
            UI_BOX_KIND_DRAW_HOT_EFFECTS
        ),
        String
    );
    UISignal Result = UISignalFromBox(Box);

    return Result;
}

UISignal 
UIHoverLabel(char* Fmt, ...)
{
    TempArena Scratch = GetScratch(NULL, 0);
    va_list Args;

    va_start(Args, Fmt);

    Str8 String = ArenaPushStrFmtV(
        Scratch.MemPool,
        Fmt,
        Args
    );

    va_end(Args);

    UISignal Result = UIHoverLabel(String);

    ReleaseScratch(Scratch);

    return Result;
}

UI_BOX_CUSTOM_DRAW(UILineEditDraw)
{
    UILineEditDrawData* DrawData = (UILineEditDrawData*) UserData;
    f32 TabSize = Box->TabSize;
    f32 FocusActive = Box->Parent->Parent->FocusActive;
    v4f32 SelectionColour = UIColourFromTagsKeyName(
        Box->TagsKey,
        "selection"_s8
    );

    SelectionColour.W *= FocusActive * 0.2f + 0.8f;

    v4f32 TrailColour = UIColourFromTagsKeyName(
        Box->TagsKey,
        "cursor"_s8
    );

    TrailColour.W *= FocusActive * 0.25f;

    v2f32 TextPosition = UIBoxTextPosition(Box);
    Str8 EditedString = DrawData->EdittedString;
    f32 CursorOffset = DDimensionsFromStr(
        TabSize,
        StrPrefix(EditedString, DrawData->Cursor.Column - 1)
    ).X;
    f32 MarkOffset = DDimensionsFromStr(
        TabSize,
        StrPrefix(EditedString, DrawData->Mark.Column - 1)
    ).X;
    f32 CursorOffsetAnimated = UIAnimate(
        UIKeyFromStr(Box->Key, "cursor_off_px"_s8),
        CursorOffset
    );
    r2f32 SelectionRect = {
        TextPosition.X + MIN(CursorOffset, MarkOffset),
        TextPosition.Y,
        TextPosition.X + MAX(CursorOffset, MarkOffset),
        TextPosition.Y + 1.0f
    };
    r2f32 TrailRect = {
        TextPosition.X + MIN(CursorOffset, CursorOffsetAnimated),
        TextPosition.Y,
        TextPosition.X + MAX(CursorOffset, CursorOffsetAnimated),
        TextPosition.Y + 1.0f
    };

    DrawRect(SelectionRect, SelectionColour, 0.0f);

    if (DrawData->Trail)
        DrawRect(TrailRect, TrailColour, 0.0f);

    if (FocusActive > 0.5f)
        DrawCaret(Vec(TextPosition.X + CursorOffset, TextPosition.Y));

}

UISignal 
UILineEdit(
    TextPoint* Cursor, 
    TextPoint* Mark, 
    u8* EditBuffer, 
    u64 EditBufferSize, 
    OUT u64* EditStrSize, 
    Str8 PreEditValue, 
    Str8 String
) {
    UIKey Key = UIKeyFromStr(UIActiveSeedKey(), String);
    b32 IsAutoFocusHot = UIIsKeyAutoFocusHot(Key);
    b32 IsAutoFocusActive = UIIsKeyAutoFocusActive(Key);

    UIPushFocusHot(IsAutoFocusHot ? UI_FOCUS_KIND_ON : UI_FOCUS_KIND_NULL);
    UIPushFocusActive(IsAutoFocusActive ? UI_FOCUS_KIND_ON : UI_FOCUS_KIND_NULL);

    b32 IsFocusHot = UIIsFocusHot();
    b32 IsFocusActive = UIIsFocusActive();
    b32 IsFocusHotDisabled = (!IsFocusHot && UIHeadFocusHot() == UI_FOCUS_KIND_ON);
    b32 IsFocusActiveDisabled = (!IsFocusActive && UIHeadFocusActive() == UI_FOCUS_KIND_ON);

    UISetNextHoverCursor(IsFocusActive ? CURSOR_KIND_I_BAR : CURSOR_KIND_POINTER);

    UIBox* Box = UIBuildBoxFromKey(
        (
            UI_BOX_KIND_DRAW_BACKGROUND |
            UI_BOX_KIND_MOUSE_CLICKABLE |
            UI_BOX_KIND_CLICK_TO_FOCUS |
            (
                (
                    IsAutoFocusHot || 
                    IsAutoFocusActive
                ) * UI_BOX_KIND_KEYBOARD_CLICKABLE
            ) |
            UI_BOX_KIND_DRAW_HOT_EFFECTS |
            (IsFocusActive || IsFocusActiveDisabled) * (
                UI_BOX_KIND_CLIP |
                UI_BOX_KIND_ALLOW_OVERFLOW_X |
                UI_BOX_KIND_VIEW_CLAMP
            )
        ),
        Key
    );

    b32 ChangesMade = FALSE;

    if (IsFocusActive) {
        TempArena Scratch = GetScratch(NULL, 0);

        for (UIEvent* Evt = NULL; UINextEvent(&Evt); ) {
            Str8 EditString = Str(EditBuffer, *EditStrSize);

            if (
                (
                    Evt->Kind != UI_EVENT_KIND_EDIT &&
                    Evt->Kind != UI_EVENT_KIND_NAVIGATE &&
                    Evt->Kind != UI_EVENT_KIND_TEXT
                ) ||
                Evt->DeltaI32.Y != 0
            ) {
                continue;
            }

            UITextOp Op = UISingleLineTextOpFromEvent(Scratch.MemPool, Evt, EditString, *Cursor, *Mark);

            if (Op.Range.Min != Op.Range.Max || Op.Replace.Size) {
                Str8 NewString = UIPushStrReplaceRange(
                    Scratch.MemPool,
                    EditString,
                    Rng(Op.Range.Min.Column, Op.Range.Max.Column),
                    Op.Replace
                );

                NewString.Size = MIN(EditBufferSize, NewString.Size);
                MemCpy(EditBuffer, NewString.Str, NewString.Size);
                *EditStrSize = NewString.Size;
            }

            if (Op.Kind & UI_TEXT_OP_KIND_COPY)
                SetClipboardText(Op.Copy);

            *Cursor = Op.Cursor;
            *Mark = Op.Mark;

            UIEatEvent(Evt);
            ChangesMade = TRUE;
        }

        ReleaseScratch(Scratch);
    }

    TextPoint MousePt = {};
    f32 CursorOffset = 0.0f;

    UIParent(Box) {
        Str8 EditString = Str(EditBuffer, *EditStrSize);

        if (!IsFocusActive && !IsFocusActiveDisabled) {
            Str8 DisplayString = UIDisplayPartFromKeyStr(String);

            if (PreEditValue.Size)
                DisplayString = PreEditValue;

            UILabel(DisplayString);
        } else {
            f32 TotalTextWidth = DDimensionsFromStr(UIHeadTabSize(), EditString).X;

            UISetNextPreferredWidth(UI_PX(TotalTextWidth + 2.0f, 1.0f));

            UIBox* EditStringBox = UIBuildBoxFromStr(
                UI_BOX_KIND_DRAW_TEXT | UI_BOX_KIND_DISABLE_TEXT_TRUNC,
                "###editstr"_s8
            );
            UILineEditDrawData* DrawData = ArenaPushArrayZero(
                UIBuildMemPool(),
                UILineEditDrawData,
                1
            );

            DrawData->EdittedString = ArenaPushStrCpy(UIBuildMemPool(), EditString);
            DrawData->Cursor = *Cursor;
            DrawData->Mark = *Mark;
            DrawData->Trail = TRUE;
            UIBoxEquipDisplayStr(EditStringBox, EditString);
            UIBoxEquipCustomDraw(EditStringBox, UILineEditDraw, DrawData);
            MousePt = TxtPt(1, 1 + UIBoxCharPositionFromXY(EditStringBox, UIMouse()));
            CursorOffset = DDimensionsFromStr(
                UIHeadTabSize(), 
                StrPrefix(EditString, Cursor->Column - 1)
            ).X;
        }
    }

    UISignal Sig = UISignalFromBox(Box);

    if (
        !IsFocusActive && 
        Sig.Kind & (
            UI_SIGNAL_KIND_CLICKED | 
            UI_SIGNAL_KIND_KEYBOARD_PRESSED
        )
    ) {
        UISetAutoFocusActiveKey({});
        Sig.Kind |= UI_SIGNAL_KIND_COMMIT;
    }

    if (IsFocusActive && UI_DRAGGING(Sig)) {
        if (UI_PRESSED(Sig))
            *Mark = MousePt;

        *Cursor = MousePt;
    }

    r1f32 CursorRangePX = Rng(
        CursorOffset - 2.0f,
        CursorOffset + 2.0f
    );
    r1f32 VisibleRangePX = Rng(
        Box->ViewOffsetTarget.X,
        Box->ViewOffsetTarget.X + Length(Box->Rect).X
    );

    CursorRangePX.Min = CLAMP_BOT(0.0f, CursorRangePX.Min);
    CursorRangePX.Max = CLAMP_BOT(0.0f, CursorRangePX.Max);

    f32 MinDelta = CursorRangePX.Min - VisibleRangePX.Min;
    f32 MaxDelta = CursorRangePX.Max - VisibleRangePX.Max;

    MinDelta = MIN(MinDelta, 0.0f);
    MaxDelta = MAX(MaxDelta, 0.0f);
    Box->ViewOffsetTarget.X += MinDelta;
    Box->ViewOffsetTarget.X += MaxDelta;
    UIPopFocusHot();
    UIPopFocusActive();

    return Sig;
}

UISignal 
UILineEdit(
    TextPoint* Cursor, 
    TextPoint* Mark, 
    u8* EditBuffer, 
    u64 EditBufferSize, 
    OUT u64* EditStrSize, 
    Str8 PreEditValue, 
    char* Fmt, 
    ...
) {
    TempArena Scratch = GetScratch(NULL, 0);
    va_list Args;

    va_start(Args, Fmt);

    Str8 String = ArenaPushStrFmtV(Scratch.MemPool, Fmt, Args);

    va_end(Args);

    UISignal Result = UILineEdit(
        Cursor, 
        Mark, 
        EditBuffer, 
        EditBufferSize, 
        EditStrSize, 
        PreEditValue, 
        String
    );

    ReleaseScratch(Scratch);

    return Result;
}

UISignal 
UIExpander(b32 IsExpanded, Str8 String)
{
    UISetNextHoverCursor(CURSOR_KIND_HAND_POINT);
    UISetNextTextAlignment(UI_TEXT_ALIGN_LEFT);

    UIBox* Box = UIBuildBoxFromStr(
        UI_BOX_KIND_CLICKABLE | UI_BOX_KIND_DRAW_TEXT, 
        String
    );

    UIBoxEquipDisplayStr(
        Box, 
        UIIconStringFromIconKind(
            IsExpanded 
                ? UI_ICON_KIND_DOWN_CARET 
                : UI_ICON_KIND_RIGHT_CARET
        )
    );

    UISignal Result = UISignalFromBox(Box);

    return Result;
}

UISignal 
UIExpander(b32 IsExpanded, char* Fmt, ...)
{
    TempArena Scratch = GetScratch(NULL, 0);
    va_list Args;

    va_start(Args, Fmt);

    Str8 String = ArenaPushStrFmtV(Scratch.MemPool, Fmt, Args);

    va_end(Args);

    UISignal Result = UIExpander(IsExpanded, String);

    ReleaseScratch(Scratch);

    return Result;
}

UISignal 
UISortHeader(b32 Sorting, b32 Ascending, Str8 String)
{
    UISetNextChildLayoutAxis(AXIS_2D_X);
    UISetNextHoverCursor(CURSOR_KIND_HAND_POINT);

    UIBox* Box = UIBuildBoxFromStr(
        (
            UI_BOX_KIND_CLICKABLE | 
            UI_BOX_KIND_DRAW_BACKGROUND | 
            UI_BOX_KIND_DRAW_ACTIVE_EFFECTS
        ),
        String
    );

    UIPushParent(Box);

    if (Sorting) {
        UISetNextPreferredWidth(UI_PX(2.0f, 1.0f));
        UISetNextTextAlignment(UI_TEXT_ALIGN_LEFT);

        UIBox* Icon = UIBuildBoxFromStr(UI_BOX_KIND_DRAW_TEXT, ""_s8);

        UIBoxEquipDisplayStr(
            Icon, 
            UIIconStringFromIconKind(
                Ascending 
                    ? UI_ICON_KIND_UP_CARET 
                    : UI_ICON_KIND_DOWN_CARET
            )
        );
    }

    UILabel(String);
    UIPopParent();

    UISignal Result = UISignalFromBox(Box);

    return Result;
}

UISignal 
UISortHeader(b32 Sorting, b32 Ascending, char* Fmt, ...)
{
    TempArena Scratch = GetScratch(NULL, 0);
    va_list Args;

    va_start(Args, Fmt);

    Str8 String = ArenaPushStrFmtV(Scratch.MemPool, Fmt, Args);

    va_end(Args);

    UISignal Result = UISortHeader(Sorting, Ascending, String);

    ReleaseScratch(Scratch);

    return Result;
}

UIBox* 
UIRowBegin(void)
{
    return UINamedRowBegin(""_s8);
}

UISignal 
UIRowEnd(void)
{
    return UINamedColumnEnd();
}

UIBox* 
UIColumnBegin(void)
{
    return UINamedColumnBegin(""_s8);
}

UISignal 
UIColumnEnd(void)
{
    return UINamedColumnEnd();
}

UIBox* 
UINamedRowBegin(Str8 String)
{
    UISetNextChildLayoutAxis(AXIS_2D_X);

    UIBox* Box = UIBuildBoxFromStr(0, String);

    UIPushParent(Box);

    return Box;
}

UISignal 
UINamedRowEnd(void)
{
    UIBox* Box = UIPopParent();
    UISignal Result = UISignalFromBox(Box);

    return Result;
}

UIBox* 
UINamedColumnBegin(Str8 String)
{
    UISetNextChildLayoutAxis(AXIS_2D_Y);

    UIBox* Box = UIBuildBoxFromStr(0, String);

    UIPushParent(Box);

    return Box;
}

UISignal 
UINamedColumnEnd(void)
{
    UIBox* Box = UIPopParent();
    UISignal Result = UISignalFromBox(Box);

    return Result;
}

UIBox* 
UIPaneBegin(r2f32 Rect, Str8 String)
{
    UIPushRect(Rect);
    UISetNextChildLayoutAxis(AXIS_2D_Y);

    UIBox* Box = UIBuildBoxFromStr(
        (
            UI_BOX_KIND_CLICKABLE | 
            UI_BOX_KIND_CLIP | 
            UI_BOX_KIND_DRAW_BORDER | 
            UI_BOX_KIND_DRAW_BACKGROUND
        ),
        String
    );

    UIPopRect();
    UIPushParent(Box);
    UIPushPreferredWidth(UI_PERCENT(1.0f, 0.0f));

    return Box;
}

UIBox* 
UIPaneBegin(r2f32 Rect, char* Fmt, ...)
{
    TempArena Scratch = GetScratch(NULL, 0);
    va_list Args;

    va_start(Args, Fmt);

    Str8 String = ArenaPushStrFmtV(Scratch.MemPool, Fmt, Args);

    va_end(Args);

    UIBox* Result = UIPaneBegin(Rect, String);

    ReleaseScratch(Scratch);

    return Result;
}

UISignal 
UIPaneEnd(void)
{
    UIPopPreferredWidth();

    UIBox* Box = UIPopParent();
    UISignal Result = UISignalFromBox(Box);

    return Result;
}

void 
UITableBegin(u64 ColumnPercentsCount, f32** ColumnPercents, Str8 String)
{
    ThreadLocalStorage* const __TLS = GetTLS();

    __TLS->UITableSizeColumnPercentsCount = ColumnPercentsCount;
    UISetNextPreferredHeight(UI_SUM_OF_CHILDREN(1.0f));
    UISetNextChildLayoutAxis(AXIS_2D_Y);

    UIBox* Table = UIBuildBoxFromStr(0, String);

    UIPushParent(Table);

    f32 XOffset = (
        (__TLS->UITableSizeColumnPercentsCount > 0) ? *ColumnPercents[0] : 0.0f) * 
        Length(Table->Rect
    ).X;

    for (
        u64 ColumnIndex = 1; 
        ColumnIndex < __TLS->UITableSizeColumnPercentsCount; 
        ++ColumnIndex
    ) {
        r2f32 Rect = {};

        Rect.X0 = FloorF32(XOffset) - 1.0f;
        Rect.Y0 = 0.0f;
        Rect.X1 = FloorF32(XOffset);
        Rect.Y1 = Length(Table->Rect).Y;
        XOffset += *ColumnPercents[ColumnIndex] * Length(Table->Rect).X;

        UIRect(Rect) {
            UISetNextHoverCursor(CURSOR_KIND_LEFT_RIGHT);

            UIBox* Box = UIBuildBoxFromStrFmt(
                UI_BOX_KIND_CLICKABLE, 
                (char*) "###%S_boundary_%llu", 
                PRINT_STR(Table->String), 
                ColumnIndex
            );
            f32* LeftPercentPtr = (ColumnIndex < __TLS->UITableSizeColumnPercentsCount) 
                ? ColumnPercents[ColumnIndex - 1] 
                : NULL;
            f32* RightPercentPtr = (ColumnIndex < __TLS->UITableSizeColumnPercentsCount) 
                ? ColumnPercents[ColumnIndex] 
                : NULL;
            UISignal Signal = UISignalFromBox(Box);

            if (UI_DRAGGING(Signal)) {
                if (UI_PRESSED(Signal)) {
                    v2f32 V = Vec(*LeftPercentPtr, *RightPercentPtr);

                    UIStoreDragStruct(&V);
                }

                f32 AdjustableTableDimension = 0.0f;

                if (Table->ChildLayoutAxis == AXIS_2D_Y) {
                    AdjustableTableDimension = Length(Table->Rect).X;
                } else {
                    u64 ChildIndex = 0;

                    for (UIBox* V = Table->Head; !IsUIBoxEmpty(V); V = V->Next, ++ChildIndex) {
                        u64 ColumnIndex = (ChildIndex + 1);

                        if (ColumnIndex < __TLS->UITableSizeColumnPercentsCount)
                            AdjustableTableDimension += Length(V->Rect).X;
                        else
                            break;
                    }
                }

                f32 MinSize = 3.0f;
                f32 LeftPercentBefore = UIGetDragStruct(v2f32)->X;
                f32 LeftPixelsBefore = LeftPercentBefore * AdjustableTableDimension;
                f32 LeftPixelsAfter = LeftPixelsBefore + UIDragDelta().X;

                if (LeftPixelsAfter < MinSize)
                    LeftPixelsAfter = MinSize;

                f32 LeftPercentAfter = LeftPixelsAfter / AdjustableTableDimension;
                f32 PercentDelta = LeftPercentAfter - LeftPercentBefore;
                f32 RightPercentBefore = UIGetDragStruct(v2f32)->Y;
                f32 RightPercentAfter = RightPercentBefore - PercentDelta;
                f32 RightPixelsAfter = RightPercentAfter * AdjustableTableDimension;

                if (RightPixelsAfter < MinSize) {
                    RightPixelsAfter = MinSize;
                    RightPercentAfter = RightPixelsAfter / AdjustableTableDimension;
                    PercentDelta = -(RightPercentAfter - RightPercentBefore);
                    LeftPercentAfter = LeftPercentBefore + PercentDelta;
                }

                *LeftPercentPtr = LeftPercentAfter;
                *RightPercentPtr = RightPercentAfter;
            }
        }
    }

    __TLS->UITableSizeColumnPercentsStable = ArenaPushArrayZero(
        UIBuildMemPool(), 
        f32, 
        __TLS->UITableSizeColumnPercentsCount
    );

    for (u64 I = 0; I < ColumnPercentsCount; ++I)
        __TLS->UITableSizeColumnPercentsStable[I] = *ColumnPercents[I];

    __TLS->UITableSizeVectorIndex = 0;
}

void 
UITableBegin(u64 ColumnPercentsCount, f32** ColumnPercents, char* Fmt, ...)
{
    TempArena Scratch = GetScratch(NULL, 0);
    va_list Args;

    va_start(Args, Fmt);

    Str8 String = ArenaPushStrFmtV(Scratch.MemPool, Fmt, Args);

    va_end(Args);
    UITableBegin(ColumnPercentsCount, ColumnPercents, String);
    ReleaseScratch(Scratch);
}

void 
UITableEnd(void)
{
    UIPopParent();
}

UIBox* 
UINamedTableVectorBegin(Str8 String)
{
    UISetNextPreferredWidth(UI_PERCENT(1.0f, 0.0f));
    UISetNextChildLayoutAxis(AXIS_2D_X);

    UIBox* Result = UIBuildBoxFromStr(UI_BOX_KIND_DRAW_SIDE_BOTTOM, String);

    ++GetTLS()->UITableSizeVectorIndex;
    GetTLS()->UITableSizeCellIndex = 0;
    UIPushParent(Result);

    return Result;
}

UIBox* 
UINamedTableVectorBegin(char* Fmt, ...)
{
    TempArena Scratch = GetScratch(NULL, 0);
    va_list Args;

    va_start(Args, Fmt);

    Str8 String = ArenaPushStrFmtV(Scratch.MemPool, Fmt, Args);

    va_end(Args);

    UIBox* Result = UINamedTableVectorBegin(String);

    ReleaseScratch(Scratch);

    return Result;
}

UIBox* 
UITableVectorBegin(void)
{
    UIBox* Table = UIHeadParent();
    UIBox* Result = UINamedTableVectorBegin(
        (char*) "###tbl_vec_%p_%llu", 
        Table, 
        GetTLS()->UITableSizeVectorIndex
    );

    return Result;
}

UISignal 
UITableVectorEnd(void)
{
    UIBox* Box = UIPopParent();
    UISignal Result = UISignalFromBox(Box);

    return Result;
}

UIBox* 
UITableCellBegin(void)
{
    u64 ColumnIndex = GetTLS()->UITableSizeCellIndex;
    f32 WidthPercent = (ColumnIndex < GetTLS()->UITableSizeColumnPercentsCount)
            ? GetTLS()->UITableSizeColumnPercentsStable[ColumnIndex]
            : 1.0f;
    UIBox* Result = UITableCellSizedBegin(UI_PERCENT(WidthPercent, 0.0f));

    return Result;
}

UISignal 
UITableCellEnd(void)
{
    UIBox* Cell = UIPopParent();
    UISignal Result = UISignalFromBox(Cell);

    return Result;
}

UIBox* 
UITableCellSizedBegin(UISize Size)
{
    UIBox* Vector = UIHeadParent();
    u64 ColumnIndex = GetTLS()->UITableSizeCellIndex;

    ++GetTLS()->UITableSizeCellIndex;
    UISetNextPreferredWidth(Size);
    UISetNextChildLayoutAxis(AXIS_2D_X);

    UIBox* Result = UIBuildBoxFromStrFmt(
        ((ColumnIndex > 0) ? UI_BOX_KIND_DRAW_SIDE_LEFT : 0), 
        (char*) "###tbl_cell_%p_%llu", 
        Vector,
        GetTLS()->UITableSizeCellIndex
    );

    UIPushParent(Result);

    return Result;
}

void 
ListPush(
    Arena* MemPool, 
    UIScrollListRowBlockChunkList* List, 
    u64 Capacity, 
    UIScrollListRowBlock* Block
) {
    UIScrollListRowBlockChunkNode* N = List->Tail;

    if (!N || N->Count >= N->Capacity) {
        N = ArenaPushArrayZero(MemPool, UIScrollListRowBlockChunkNode, 1);
        N->Capacity = Capacity;
        N->V = ArenaPushArray(MemPool, UIScrollListRowBlock, N->Capacity);
        SLL_QUEUE_PUSH(List->Head, List->Tail, N);
        ++List->ChunkCount;
    }

    MemCpy(&N->V[N->Count], Block, sizeof(N->V[N->Count]));
    ++N->Count;
    ++List->TotalCount;
}

UIScrollListRowBlockArray 
UIScrollListRowBlockArrayFromChunkList(
    Arena* MemPool, 
    UIScrollListRowBlockChunkList* List
) {
    UIScrollListRowBlockArray Result = {};

    Result.Count = List->TotalCount;
    Result.V = ArenaPushArray(MemPool, UIScrollListRowBlock, Result.Count);

    u64 Index = 0;

    for (
        UIScrollListRowBlockChunkNode* N = List->Head;
        N;
        N = N->Next
    ) {
        MemCpy(Result.V + Index, N->V, sizeof(N->V[0]) * N->Count);
        Index += N->Count;
    }

    return Result;
}

u64 
UIScrollListRowFromItem(UIScrollListRowBlockArray* Blocks, u64 Item)
{
    u64 Result = 0;
    u64 RowIndex = 0;
    u64 ItemIndex = 0;

    for (u64 BlockIndex = 0; BlockIndex < Blocks->Count; ++BlockIndex) {
        UIScrollListRowBlock* Block = &Blocks->V[BlockIndex];
        u64 NextRowIndex = RowIndex + Block->RowCount;
        u64 NextItemIndex = ItemIndex + Block->ItemCount;

        if (ItemIndex <= Item && Item < NextItemIndex) {
            u64 ItemOffsetRows = (Item - ItemIndex) * (Block->RowCount / Block->ItemCount);

            Result = RowIndex + ItemOffsetRows;
            break;
        }

        RowIndex = NextRowIndex;
        ItemIndex = NextItemIndex;
    }

    return Result;
}

u64 
UIScrollListItemFromRow(UIScrollListRowBlockArray* Blocks, u64 Row)
{
    u64 Result = 0;
    u64 RowIndex = 0;
    u64 ItemIndex = 0;

    for (u64 BlockIndex = 0; BlockIndex < Blocks->Count; ++BlockIndex) {
        UIScrollListRowBlock* Block = &Blocks->V[BlockIndex];
        u64 NextRowIndex = RowIndex + Block->RowCount;
        u64 NextItemIndex = ItemIndex + Block->ItemCount;

        if (RowIndex <= Row && Row < NextRowIndex) {
            Result = ItemIndex;
            break;
        }

        RowIndex = NextRowIndex;
        ItemIndex = NextItemIndex;
    }

    return Result;
}

UIScrollPoint 
UIScrollBar(
    Axis2D Axis, 
    UISize OffsetAxisSize, 
    UIScrollPoint Point, 
    r1i64 IndexRange, 
    i64 ViewNumIndices
) {
    UIPushTag("scroll_bar"_s8);

    i64 IndexRangeDim = MAX(Length(IndexRange), 1);
    UIBoxKind DisabledKind = 0;

    if (IndexRange.Min == IndexRange.Max)
        DisabledKind |= UI_BOX_KIND_DISABLED;

    f32 ScrollerFrac = CLAMP(
        0.05f,
        (f32) ((f64) MAX(ViewNumIndices, 1) / (f64) IndexRangeDim),
        1.0f
    );
    f32 ScrollFrac = (f32) ((f64) (Point.Index - IndexRange.Min) / (f64) IndexRangeDim);
    f32 BeforeFrac = (1.0f - ScrollerFrac) * ScrollFrac;
    f32 AfterFrac  = (1.0f - ScrollerFrac) - BeforeFrac;

    UISetNextPreferredSize(FLIP_AXIS(Axis), OffsetAxisSize);
    UISetNextChildLayoutAxis(Axis);

    UIBox* ContainerBox = UIBuildBoxFromKey(0, EMPTY_UI_KEY_VALUE);
    UISignal MinScrollSig = {};

    UIParent(ContainerBox) {
        UIPreferredSize(Axis, OffsetAxisSize) {
            UIFlags(DisabledKind) {
                UITextAlignment(UI_TEXT_ALIGN_CENTRE) {
                    Str8 ArrowString = UIIconStringFromIconKind(
                        (Axis == AXIS_2D_X) 
                            ? UI_ICON_KIND_LEFT_ARROW 
                            : UI_ICON_KIND_UP_ARROW
                    );

                    MinScrollSig = UIButton(
                        "%S##_min_scroll_%i", 
                        PRINT_STR(ArrowString), 
                        Axis
                    );
                }
            }
        }
    }

    UISignal SpaceBeforeSig = {};
    UISignal SpaceAfterSig = {};
    UISignal ScrollerSig = {};
    UIBox* ScrollAreaBox = EMPTY_UI_BOX_VALUE;
    UIBox* ScrollerBox = EMPTY_UI_BOX_VALUE;

    UIParent(ContainerBox) {
        UISetNextPreferredSize(Axis, UI_PERCENT(1, 0));
        UISetNextChildLayoutAxis(Axis);
        ScrollAreaBox = UIBuildBoxFromStrFmt(
            0, 
            (char*) "##_scroll_area_%i", 
            Axis
        );

        UIParent(ScrollAreaBox) {
            if (IndexRange.Max != IndexRange.Min) {
                UISetNextPreferredSize(
                    Axis, 
                    UI_PERCENT(BeforeFrac, 0)
                );

                UIBox* SpaceBeforeBox = UIBuildBoxFromStrFmt(
                    UI_BOX_KIND_CLICKABLE, 
                    (char*) "##scroll_area_before"
                );

                SpaceBeforeSig = UISignalFromBox(SpaceBeforeBox);
            }

            UIFlags(DisabledKind) {
                UIPreferredSize(Axis, UI_PERCENT(ScrollerFrac, 0)) {
                    ScrollerBox = UIBuildBoxFromStrFmt(
                        UI_BOX_KIND_CLICKABLE, 
                        (char*) "##_scroller_%i", 
                        Axis
                    );
                    ScrollerSig = UISignalFromBox(ScrollerBox);
                }
            }

            if (IndexRange.Max != IndexRange.Min) {
                UISetNextPreferredSize(Axis, UI_PERCENT(AfterFrac, 0));

                UIBox* SpaceAfterBox = UIBuildBoxFromStrFmt(
                    UI_BOX_KIND_CLICKABLE, 
                    (char*) "##scroll_area_after"
                );

                SpaceAfterSig = UISignalFromBox(SpaceAfterBox);
            }
        }
    }

    UIScrollBarDrawData* DrawData = ArenaPushArrayZero(
        UIBuildMemPool(), 
        UIScrollBarDrawData, 
        1
    );

    DrawData->Axis = Axis;
    DrawData->BeforeFrac = BeforeFrac;
    DrawData->ScrollerFrac = ScrollerFrac;
    DrawData->ScrollerBox = ScrollerBox;
    UIBoxEquipCustomDraw(ScrollAreaBox, UIScrollBarDraw, DrawData);

    UISignal MaxScrollSig = {};

    UIParent(ContainerBox) {
        UIPreferredSize(Axis, OffsetAxisSize) {
            UIFlags(DisabledKind) {
                UITextAlignment(UI_TEXT_ALIGN_CENTRE) {
                    Str8 ArrowString = UIIconStringFromIconKind(
                        (Axis == AXIS_2D_X) 
                            ? UI_ICON_KIND_RIGHT_ARROW 
                            : UI_ICON_KIND_DOWN_ARROW
                    );

                    MaxScrollSig = UIButton(
                        "%S##_max_scroll_%i", 
                        PRINT_STR(ArrowString), 
                        Axis
                    );
                }
            }
        }
    }

    UIScrollPoint NewPoint = Point;

    if (UI_DRAGGING(ScrollerSig)) {
        if (UI_PRESSED(ScrollerSig)) {
            UIScrollBarDragData DragData = {
                Point,
                FloorF32(Length(ScrollAreaBox->Rect).V[Axis])
            };

            UIStoreDragStruct(&DragData);
        }

        UIScrollBarDragData* DragData = UIGetDragStruct(UIScrollBarDragData);
        UIScrollPoint OriginalPoint = DragData->StartPoint;
        f32 DragDelta = UIDragDelta().V[Axis];
        f32 DragPercent = DragDelta / DragData->ScrollSpacePX;
        i64 NewIndex = OriginalPoint.Index + DragPercent * IndexRangeDim;

        NewIndex = CLAMP(IndexRange.Min, NewIndex, IndexRange.Max);
        UIScrollPointTargetIndex(&NewPoint, NewIndex);
        NewPoint.Offset = 0.0f;
    }

    if (UI_DRAGGING(MinScrollSig) || UI_DRAGGING(SpaceBeforeSig)) {
        i64 NewIndex = NewPoint.Index - 1;

        NewIndex = CLAMP(IndexRange.Min, NewIndex, IndexRange.Max);
        UIScrollPointTargetIndex(&NewPoint, NewIndex);
    }

    if (UI_DRAGGING(MaxScrollSig) || UI_DRAGGING(SpaceAfterSig)) {
        i64 NewIndex = NewPoint.Index + 1;

        NewIndex = CLAMP(IndexRange.Min, NewIndex, IndexRange.Max);
        UIScrollPointTargetIndex(&NewPoint, NewIndex);
    }

    UIPopTag();

    return NewPoint;
}

void 
UIScrollListBegin(
    UIScrollListParameters* Params, 
    OUT UIScrollPoint* ScrollPoint, 
    OUT v2i64* Cursor, 
    OUT v2i64* Mark, 
    OUT r1i64* VisibleRowRange, 
    OUT UIScrollListSignal* Signal
) {
    r1i64 ScrollRowIndexRange = Rng(
        (i64) Params->ItemRange.Min, 
        (i64) CLAMP_BOT(
            Params->ItemRange.Min, 
            Params->ItemRange.Max - 1
        )
    );
    i64 NumPossibleVisibleRows = (i64) (
        Params->DimensionsPX.Y / Params->RowHeightPX
    );
    b32 Moved = FALSE;

    if (Params->Kind & UI_SCROLL_LIST_KIND_NAV && Cursor && UIIsFocusActive()) {
        v2i64 CursorPosition = *Cursor;
        v2i64 MarkPosition = (Mark) ? *Mark : CursorPosition;

        for (UIEvent* Evt = NULL; UINextEvent(&Evt); ) {
            if (
                (Evt->DeltaI32.X == 0 && Evt->DeltaI32.Y == 0) || 
                Evt->Flags & UI_EVENT_FLAG_DELETE
            ) {
                continue;
            }

            UIEatEvent(Evt);
            Moved = TRUE;

            switch (Evt->DeltaStride) {
                default: {
                    Moved = FALSE;
                } break;

                case UI_EVENT_DELTA_STRIDE_CHAR: {
                    for (
                        Axis2D Axis = (Axis2D) 0; 
                        Axis < AXIS_2D_COUNT; 
                        Axis = (Axis2D) (Axis + 1)
                    ) {
                        CursorPosition.V[Axis] += Evt->DeltaI32.V[Axis];

                        if (CursorPosition.V[Axis] < Params->CursorRange.Min.V[Axis])
                            CursorPosition.V[Axis] = Params->CursorRange.Max.V[Axis];

                        if (CursorPosition.V[Axis] > Params->CursorRange.Max.V[Axis])
                            CursorPosition.V[Axis] = Params->CursorRange.Min.V[Axis];

                        CursorPosition.V[Axis] = Clamp(
                            Rng(
                                (i64) Params->CursorRange.Min.V[Axis], 
                                (i64) Params->CursorRange.Max.V[Axis]
                            ), 
                            CursorPosition.V[Axis]
                        );
                    }
                } break;

                case UI_EVENT_DELTA_STRIDE_WORD:
                case UI_EVENT_DELTA_STRIDE_LINE:
                case UI_EVENT_DELTA_STRIDE_PAGE: {
                    CursorPosition.X = (
                        (Evt->DeltaI32.X > 0) 
                            ? Params->CursorRange.Max.X 
                            : (Evt->DeltaI32.X < 0) 
                                ? Params->CursorRange.Min.X + !!Params->CursorMinIsEmptySelection[AXIS_2D_X] 
                                : CursorPosition.X
                    );
                    CursorPosition.Y = (
                        (Evt->DeltaI32.Y > 0) 
                        ? +(NumPossibleVisibleRows - 3) 
                        : (Evt->DeltaI32.Y < 0) 
                            ? -(NumPossibleVisibleRows - 3) 
                            : 0
                    );
                    CursorPosition.Y = Clamp(
                        Rng(
                            (i64) Params->CursorRange.Min.Y + !!Params->CursorMinIsEmptySelection[AXIS_2D_Y], 
                            (i64) Params->CursorRange.Max.Y
                        ), 
                        CursorPosition.Y
                    );
                } break;

                case UI_EVENT_DELTA_STRIDE_WHOLE: {
                    for (
                        Axis2D Axis = (Axis2D) 0; 
                        Axis < AXIS_2D_COUNT; 
                        Axis = (Axis2D) (Axis + 1)
                    ) {
                        CursorPosition.V[Axis] = (
                            (Evt->DeltaI32.V[Axis] > 0) 
                                ? Params->CursorRange.Max.V[Axis] 
                                : (Evt->DeltaI32.V[Axis] < 0) 
                                    ? Params->CursorRange.Min.V[Axis] + !!Params->CursorMinIsEmptySelection[Axis] 
                                    : CursorPosition.V[Axis]
                        );
                    }
                } break;
            }

            if (!(Evt->Flags & UI_EVENT_FLAG_KEEP_MARK))
                MarkPosition = CursorPosition;
        }

        if (Moved) {
            *Cursor = CursorPosition;

            if (Mark)
                *Mark = MarkPosition;
        }
    }

    if (Params->Kind & UI_SCROLL_LIST_KIND_SNAP &&  Moved) {
        i64 CursorItemIndex = Cursor->Y - 1;

        if (
            Params->ItemRange.Min <= CursorItemIndex && 
            CursorItemIndex <= Params->ItemRange.Max
        ) {
            r1i64 VisibleRowRangePosition = Rng(
                ScrollPoint->Index + 0 - !!(ScrollPoint->Offset < 0),
                ScrollPoint->Index + 0 + NumPossibleVisibleRows + 1
            );
            r1i64 CursorVisibilityRowRange = {};

            if (!Params->RowBlocks.Count) {
                CursorVisibilityRowRange = Rng(
                    CursorItemIndex - 1, 
                    CursorItemIndex + 3
                );
            } else {
                CursorVisibilityRowRange.Min = (i64) UIScrollListRowFromItem(
                    &Params->RowBlocks, 
                    (u64) CursorItemIndex
                );
                CursorVisibilityRowRange.Max = CursorVisibilityRowRange.Min + 4;
            }

            i64 MinDelta = MIN(
                0, 
                CursorVisibilityRowRange.Min - VisibleRowRangePosition.Min
            );
            i64 MaxDelta = MAX(
                0, 
                CursorVisibilityRowRange.Max - VisibleRowRangePosition.Max
            );
            i64 NewIndex = ScrollPoint->Index + MinDelta + MaxDelta;

            NewIndex = Clamp(ScrollRowIndexRange, NewIndex);
            UIScrollPointTargetIndex(ScrollPoint, NewIndex);
        }
    }

    if (Signal)
        Signal->CursorMoved = Moved;

    r1i64 VisibleRowRangePosition = Rng(
        ScrollPoint->Index + (i64) (ScrollPoint->Offset) + 0 - !!(ScrollPoint->Offset < 0.0f),
        ScrollPoint->Index + (i64) (ScrollPoint->Offset) + 0 + NumPossibleVisibleRows + 1
    );

    VisibleRowRangePosition.Min = Clamp(
        Params->ItemRange, 
        VisibleRowRangePosition.Min
    );
    VisibleRowRangePosition.Max = Clamp(
        Params->ItemRange, 
        VisibleRowRangePosition.Max
    );
    *VisibleRowRange = VisibleRowRangePosition;
    GetTLS()->UIScrollListScrollBarDimensionsPX = 1.0f;
    GetTLS()->UIScrollListScrollPointPtr = ScrollPoint;
    GetTLS()->UIScrollListDimensionsPX = Params->DimensionsPX;
    GetTLS()->UIScrollListScrollIndexRange = *(r1u64*) &ScrollRowIndexRange;

    UIBox* ContainerBox = EMPTY_UI_BOX_VALUE;

    UIFixedWidth(Params->DimensionsPX.X) {
        UIFixedHeight(Params->DimensionsPX.Y) {
            UIChildLayoutAxis(AXIS_2D_X) {
                ContainerBox = UIBuildBoxFromKey(0, EMPTY_UI_KEY_VALUE);
            }
        }
    }

    UIBox* ScrollableContainerBox = EMPTY_UI_BOX_VALUE;

    UIParent(ContainerBox) {
        UIChildLayoutAxis(AXIS_2D_Y) {
            UIFixedWidth(
                Params->DimensionsPX.X - GetTLS()->UIScrollListScrollBarDimensionsPX
            ) {
                UIFixedHeight(Params->DimensionsPX.Y) {
                    UIBoxKind FadeKind = 0;

                    if (ScrollPoint->Index > Params->ItemRange.Min)
                        FadeKind |= UI_BOX_KIND_DRAW_FADE_TOP;

                    if (
                        ScrollPoint->Index + NumPossibleVisibleRows < 
                        Params->ItemRange.Max
                    ) {
                        FadeKind |= UI_BOX_KIND_DRAW_FADE_BOTTOM;
                    }

                    ScrollableContainerBox = UIBuildBoxFromStrFmt(
                        (
                            FadeKind |
                            UI_BOX_KIND_CLIP |
                            UI_BOX_KIND_ALLOW_OVERFLOW_Y |
                            UI_BOX_KIND_SCROLL
                        ),
                        (char*) "###sp"
                    );
                    ScrollableContainerBox->ViewOffset.Y = 
                    ScrollableContainerBox->ViewOffsetTarget.Y = 
                        Params->RowHeightPX * FMod(ScrollPoint->Offset, 1.0f) + Params->RowHeightPX * (ScrollPoint->Offset < 0.0f) - Params->RowHeightPX * (ScrollPoint->Offset == -1.0f && ScrollPoint->Index == 1);
                }
            }
        }
    }

    UIParent(ContainerBox) {
        UIFocus(UI_FOCUS_KIND_NULL) {
            UISetNextFixedWidth(GetTLS()->UIScrollListScrollBarDimensionsPX);
            UISetNextFixedHeight(GetTLS()->UIScrollListDimensionsPX.Y);

            *GetTLS()->UIScrollListScrollPointPtr = UIScrollBar(
                AXIS_2D_Y,
                UI_PX(GetTLS()->UIScrollListScrollBarDimensionsPX, 1.0f),
                *GetTLS()->UIScrollListScrollPointPtr,
                ScrollRowIndexRange,
                NumPossibleVisibleRows
            );
        }
    }

    UIPushParent(ContainerBox);
    UIPushParent(ScrollableContainerBox);
    UIPushPreferredHeight(UI_PX(Params->RowHeightPX, 1.0f));
}

void 
UIScrollListEnd(void)
{
    UIPopPreferredHeight();

    UIBox* ScrollableContainerBox = UIPopParent();
    UIBox* ContainerBox = UIPopParent();
    UISignal Sig = UISignalFromBox(ScrollableContainerBox);

    if (Sig.Scroll.Y != 0) {
        i64 NewIndex = GetTLS()->UIScrollListScrollPointPtr->Index + Sig.Scroll.Y;

        NewIndex = Clamp(
            *(r1i64*) &GetTLS()->UIScrollListScrollIndexRange, 
            NewIndex
        );
        UIScrollPointTargetIndex(
            GetTLS()->UIScrollListScrollPointPtr, 
            NewIndex
        );
    }

    UIScrollPointClampIndex(
        GetTLS()->UIScrollListScrollPointPtr, 
        *(r1i64*) &GetTLS()->UIScrollListScrollIndexRange
    );
}

UISignal 
UICheckBox(b32* Checked, Str8 String)
{
    UIPushHoverCursor(CURSOR_KIND_HAND_POINT);
    UISetNextChildLayoutAxis(AXIS_2D_X);

    UIBox* Box = UIBuildBoxFromStr(
        UI_BOX_KIND_CLICKABLE | UI_BOX_KIND_DRAW_HOT_EFFECTS,
        String
    );

    UIParent(Box) {
        UIPreferredWidth(UI_TEXT_DIM(0.0f, 1.0f)) {
            UIPreferredHeight(UI_PX(1.0f, 1.0f)) {
                UIBox* CheckBox = UIBuildBoxFromStr(
                    UI_BOX_KIND_DRAW_TEXT, 
                    "###check_box"_s8
                );

                UIBoxEquipDisplayStr(
                    CheckBox, 
                    UIIconStringFromIconKind(
                        *Checked 
                            ? UI_ICON_KIND_CHECK_FILLED 
                            : UI_ICON_KIND_CHECK_HOLLOW
                    )
                );
            }
        }

        UISpacer(UI_PX(1.0f, 1.0f));

        Str8 DisplayString = UIDisplayPartFromKeyStr(String);

        if (DisplayString.Size)
            UILabel(DisplayString);
    }

    UISignal Sig = UISignalFromBox(Box);

    if (UI_CLICKED(Sig))
        *Checked = !(*Checked);

    UIPopHoverCursor();

    return Sig;
}

UISignal 
UICheckBox(b32* Checked, char* Fmt, ...)
{
    TempArena Scratch = GetScratch(NULL, 0);
    va_list Args;

    va_start(Args, Fmt);

    Str8 String = ArenaPushStrFmtV(Scratch.MemPool, Fmt, Args);

    va_end(Args);

    UISignal Sig = UICheckBox(Checked, String);

    ReleaseScratch(Scratch);

    return Sig;
}

UISignal 
UIToggle(b32* Enabled, UIToggleKind Kind, Str8 String)
{
    UIPushHoverCursor(CURSOR_KIND_HAND_POINT);
    UISetNextChildLayoutAxis(AXIS_2D_X);

    UIBox* Box = UIBuildBoxFromStr(
        UI_BOX_KIND_CLICKABLE | UI_BOX_KIND_DRAW_HOT_EFFECTS,
        String
    );

    UIParent(Box) {
        if (Kind == UI_TOGGLE_KIND_CHECKBOX) {
            UIPreferredWidth(UI_TEXT_DIM(0.0f, 1.0f)) {
                UIPreferredHeight(UI_PX(1.0f, 1.0f)) {
                    UIBox* CheckBox = UIBuildBoxFromStr(
                        UI_BOX_KIND_DRAW_TEXT, 
                        "###toggle_box"_s8
                    );

                    UIBoxEquipDisplayStr(
                        CheckBox, 
                        UIIconStringFromIconKind(
                            *Enabled 
                                ? UI_ICON_KIND_CHECK_FILLED 
                                : UI_ICON_KIND_CHECK_HOLLOW
                        )
                    );
                }
            }
        } else if (Kind == UI_TOGGLE_KIND_SWITCH) {
            UIPreferredWidth(UI_PX(4.0f, 1.0f)) {
                UIPreferredHeight(UI_PX(1.0f, 1.0f)) {
                    UIKey AnimKey = UIKeyFromStr(
                        UIActiveSeedKey(),
                        "###toggle_anim"_s8
                    );
                    f32 EnabledT = UIAnimate(AnimKey, *Enabled ? 1.0f : 0.0f);
                    UIBox* SwitchBox = UIBuildBoxFromStr(
                        0,
                        "###toggle_switch"_s8
                    );
                    UIToggleSwitchDrawData* DrawData = ArenaPushArrayZero(
                        UIBuildMemPool(),
                        UIToggleSwitchDrawData,
                        1
                    );

                    DrawData->EnabledT = EnabledT;
                    UIBoxEquipCustomDraw(
                        SwitchBox,
                        UIToggleSwitchDraw,
                        DrawData
                    );
                }
            }
        }

        UISpacer(UI_PX(1.0f, 1.0f));

        Str8 DisplayString = UIDisplayPartFromKeyStr(String);

        if (DisplayString.Size)
            UILabel(DisplayString);
    }

    UISignal Sig = UISignalFromBox(Box);

    if (UI_CLICKED(Sig))
        *Enabled = !(*Enabled);

    UIPopHoverCursor();

    return Sig;
}

UISignal 
UIToggle(b32* Enabled, UIToggleKind Kind, char* Fmt, ...)
{
    TempArena Scratch = GetScratch(NULL, 0);
    va_list Args;

    va_start(Args, Fmt);

    Str8 String = ArenaPushStrFmtV(Scratch.MemPool, Fmt, Args);

    va_end(Args);

    UISignal Sig = UIToggle(Enabled, Kind, String);

    ReleaseScratch(Scratch);

    return Sig;
}

UISliderResult 
UISlider(f32* Value, r1f32 Range, UISliderKind Kind, Str8 String)
{
    UISliderResult Result = {};
    f32 OriginalValue = *Value;
    Axis2D Axis = (Kind == UI_SLIDER_KIND_HORZ) ? AXIS_2D_X : AXIS_2D_Y;

    UIPushHoverCursor(CURSOR_KIND_HAND_POINT);

    UIBox* Box = UIBuildBoxFromStr(UI_BOX_KIND_CLICKABLE, String);

    UISliderDrawData* DrawData = ArenaPushArrayZero(
        UIBuildMemPool(), 
        UISliderDrawData, 
        1
    );

    DrawData->Value = *Value;
    DrawData->Range = Range;
    DrawData->Kind = Kind;
    UIBoxEquipCustomDraw(Box, UISliderDraw, DrawData);

    UISignal Sig = UISignalFromBox(Box);

    if (UI_DRAGGING(Sig)) {
        f32 TrackStart = Box->Rect.Point0.V[Axis];
        f32 TrackEnd = Box->Rect.Point1.V[Axis];
        f32 TrackLen = TrackEnd - TrackStart;

        if (TrackLen > 0.0f) {
            f32 MousePos = UIMouse().V[Axis] + 0.5f;
            f32 NormPos = (MousePos - TrackStart) / TrackLen;

            NormPos = CLAMP(0.0f, NormPos, 1.0f);

            if (Axis == AXIS_2D_Y)
                NormPos = 1.0f - NormPos;

            *Value = LerpF32(NormPos, Range.Min, Range.Max);
        }

        if (UI_PRESSED(Sig))
            UIStoreDragStruct(&OriginalValue);

        if (UISlotPress(UI_EVENT_ACTION_SLOT_CANCEL)) {
            f32* StoredValue = UIGetDragStruct(f32);

            *Value = *StoredValue;
            UIKillAction();
        }
    }

    DrawData->Value = *Value;
    Result.Sig = Sig;
    Result.Value = *Value;
    Result.Changed = (*Value != OriginalValue);
    UIPopHoverCursor();

    return Result;
}

UISliderResult 
UISlider(f32* Value, r1f32 Range, UISliderKind Kind, char* Fmt, ...)
{
    TempArena Scratch = GetScratch(NULL, 0);
    va_list Args;

    va_start(Args, Fmt);

    Str8 String = ArenaPushStrFmtV(Scratch.MemPool, Fmt, Args);

    va_end(Args);

    UISliderResult Result = UISlider(Value, Range, Kind, String);

    ReleaseScratch(Scratch);

    return Result;
}

UISliderResult 
UISlider(i32* Value, r1i32 Range, UISliderKind Kind, Str8 String)
{
    f32 FloatValue = (f32) *Value;
    UISliderResult Result = UISlider(
        &FloatValue, 
        Rng((f32) Range.Min, (f32) Range.Max), 
        Kind, 
        String
    );

    *Value = (i32) RoundF32(FloatValue);
    Result.Value = (f32) *Value;

    return Result;
}

UISliderResult 
UISlider(i32* Value, r1i32 Range, UISliderKind Kind, char* Fmt, ...)
{
    TempArena Scratch = GetScratch(NULL, 0);
    va_list Args;

    va_start(Args, Fmt);

    Str8 String = ArenaPushStrFmtV(Scratch.MemPool, Fmt, Args);

    va_end(Args);

    UISliderResult Result = UISlider(Value, Range, Kind, String);

    ReleaseScratch(Scratch);

    return Result;
}

UIDropDownResult 
UIDropDown(i64* SelectedIndex, Str8Array Options, Str8 String)
{
    UIDropDownResult Result = {};
    UIKey Key = UIKeyFromStr(UIActiveSeedKey(), String);
    UIKey MenuKey = UIKeyFromStr(Key, "###dropdown_menu"_s8);
    b32 MenuOpen = UIContextMenuIsOpen(MenuKey);

    UIPushHoverCursor(CURSOR_KIND_HAND_POINT);
    UISetNextChildLayoutAxis(AXIS_2D_X);

    UIBox* Box = UIBuildBoxFromStr(
        (
            UI_BOX_KIND_CLICKABLE |
            UI_BOX_KIND_DRAW_BACKGROUND |
            UI_BOX_KIND_DRAW_HOT_EFFECTS |
            UI_BOX_KIND_DRAW_ACTIVE_EFFECTS
        ),
        String
    );

    UIParent(Box) {
        UIWidthFill() {
            if (*SelectedIndex >= 0 && (u64) *SelectedIndex < Options.Count)
                UILabel(Options.Data[*SelectedIndex]);
            else
                UILabel("Select..."_s8);
        }

        UISpacer(UI_PX(1.0f, 1.0f));

        UIPreferredWidth(UI_PX(2.0f, 1.0f)) {
            UIBox* Arrow = UIBuildBoxFromStr(
                UI_BOX_KIND_DRAW_TEXT, 
                "###arrow"_s8
            );

            UIBoxEquipDisplayStr(
                Arrow, 
                MenuOpen 
                    ? UIIconStringFromIconKind(UI_ICON_KIND_UP_CARET) 
                    : UIIconStringFromIconKind(UI_ICON_KIND_DOWN_CARET)
            );
        }
    }

    UISignal Sig = UISignalFromBox(Box);

    if (UI_CLICKED(Sig)) {
        if (MenuOpen) {
            UIContextMenuClose();
        } else {
            UIContextMenuOpen(
                MenuKey, 
                Box->Key, 
                Vec(0.0f, Length(Box->Rect).Y)
            );
        }
    }

    UIContextMenu(MenuKey) {
        UIPreferredWidth(UI_PX(Length(Box->Rect).X, 1.0f)) {
            for (u64 I = 0; I < Options.Count; ++I) {
                UISignal ItemSig = UIButton(
                    "%S###item_%llu", 
                    PRINT_STR(Options.Data[I]), 
                    I
                );

                if (UI_CLICKED(ItemSig)) {
                    if (*SelectedIndex != (i64) I) {
                        Result.Changed = TRUE;
                        *SelectedIndex = (i64) I;
                    }

                    UIContextMenuClose();
                }
            }
        }
    }

    Result.Sig = Sig;
    Result.SelectedIndex = *SelectedIndex;
    UIPopHoverCursor();

    return Result;
}

UIDropDownResult 
UIDropDown(i64* SelectedIndex, Str8Array Options, char* Fmt, ...)
{
    TempArena Scratch = GetScratch(NULL, 0);
    va_list Args;

    va_start(Args, Fmt);

    Str8 String = ArenaPushStrFmtV(Scratch.MemPool, Fmt, Args);

    va_end(Args);

    UIDropDownResult Result = UIDropDown(SelectedIndex, Options, String);

    ReleaseScratch(Scratch);

    return Result;
}

UIDropDownResult 
UIComboBox(i64* SelectedIndex, Str8Array Options, b32* IsOpen, Str8 String)
{
    UIDropDownResult Result = UIDropDown(SelectedIndex, Options, String);

    *IsOpen = UIContextMenuIsOpen(
        UIKeyFromStr(
            UIKeyFromStr(
                UIActiveSeedKey(), 
                String
            ), 
            "###dropdown_menu"_s8
        )
    );

    return Result;
}

UIDropDownResult 
UIComboBox(i64* SelectedIndex, Str8Array Options, b32* IsOpen, char* Fmt, ...)
{
    TempArena Scratch = GetScratch(NULL, 0);
    va_list Args;

    va_start(Args, Fmt);

    Str8 String = ArenaPushStrFmtV(Scratch.MemPool, Fmt, Args);

    va_end(Args);

    UIDropDownResult Result = UIComboBox(
        SelectedIndex, 
        Options, 
        IsOpen, 
        String
    );

    ReleaseScratch(Scratch);

    return Result;
}

UINumericEditResult 
UINumericEdit(f64* Value, r1f64 Range, f64 Step, Str8 String)
{
    UINumericEditResult Result = {};
    f64 OriginalValue = *Value;

    UISetNextChildLayoutAxis(AXIS_2D_X);

    UIBox* ContainerBox = UIBuildBoxFromStr(
        UI_BOX_KIND_DRAW_BACKGROUND | UI_BOX_KIND_SCROLL,
        String
    );

    TextPoint Cursor = TxtPt(1, 1);
    TextPoint Mark = Cursor;
    u8 EditBuffer[64] = {};
    u64 EditStrSize = 0;

    TempArena Scratch = GetScratch(NULL, 0);
    Str8 ValueStr = ArenaPushStrFmt(Scratch.MemPool, "%.4lf", *Value);

    MemCpy(
        EditBuffer, 
        ValueStr.Str, 
        MIN(ValueStr.Size, sizeof(EditBuffer) - 1)
    );
    EditStrSize = MIN(ValueStr.Size, sizeof(EditBuffer) - 1);
    Cursor.Column = (i64) EditStrSize + 1;

    UIParent(ContainerBox) {
        UIPreferredWidth(UI_PX(3.0f, 1.0f)) {
            UIPushHoverCursor(CURSOR_KIND_HAND_POINT);
            UISetNextTextAlignment(UI_TEXT_ALIGN_CENTRE);

            UISignal DecrSig = UIButton(
                "%S###decr", 
                PRINT_STR(UIIconStringFromIconKind(UI_ICON_KIND_LEFT_CARET))
            );

            if (UI_CLICKED(DecrSig)) {
                *Value = CLAMP(Range.Min, *Value - Step, Range.Max);
                Result.Changed = TRUE;
            }

            UIPopHoverCursor();
        }

        UIWidthFill() {
            UISignal EditSig = UILineEdit(
                &Cursor, 
                &Mark, 
                EditBuffer, 
                sizeof(EditBuffer), 
                &EditStrSize, 
                ValueStr, 
                "###edit"_s8
            );

            if (UI_COMMITTED(EditSig)) {
                Str8 EditStr = Str(EditBuffer, EditStrSize);
                f64 ParsedValue = 0.0;

                if (EditStr.Size > 0) {
                    ParsedValue = F64FromStr(StrPrefix(EditStr, 64));
                }

                *Value = CLAMP(Range.Min, ParsedValue, Range.Max);
                Result.Comitted = TRUE;
                Result.Changed = (*Value != OriginalValue);
            }

            Result.Sig = EditSig;
        }

        UIPreferredWidth(UI_PX(3.0f, 1.0f)) {
            UIPushHoverCursor(CURSOR_KIND_HAND_POINT);
            UISetNextTextAlignment(UI_TEXT_ALIGN_CENTRE);

            UISignal IncrSig = UIButton(
                "%S###incr", 
                PRINT_STR(UIIconStringFromIconKind(UI_ICON_KIND_RIGHT_CARET))
            );

            if (UI_CLICKED(IncrSig)) {
                *Value = CLAMP(Range.Min, *Value + Step, Range.Max);
                Result.Changed = TRUE;
            }

            UIPopHoverCursor();
        }
    }

    UISignal ContainerSig = UISignalFromBox(ContainerBox);

    if (ContainerSig.Scroll.Y != 0) {
        *Value = CLAMP(
            Range.Min, 
            *Value - Step * (f64) ContainerSig.Scroll.Y, 
            Range.Max
        );
        Result.Changed = (*Value != OriginalValue);
    }

    ReleaseScratch(Scratch);

    Result.Value = *Value;

    return Result;
}

UINumericEditResult 
UINumericEdit(f64* Value, r1f64 Range, f64 Step, char* Fmt, ...)
{
    TempArena Scratch = GetScratch(NULL, 0);
    va_list Args;

    va_start(Args, Fmt);

    Str8 String = ArenaPushStrFmtV(Scratch.MemPool, Fmt, Args);

    va_end(Args);

    UINumericEditResult Result = UINumericEdit(Value, Range, Step, String);

    ReleaseScratch(Scratch);

    return Result;
}

UINumericEditResult 
UINumericEdit(i64* Value, r1i64 Range, i64 Step, Str8 String)
{
    f64 FloatValue = (f64) *Value;
    UINumericEditResult Result = UINumericEdit(
        &FloatValue, 
        Rng(
            (f64) Range.Min, 
            (f64) Range.Max
        ),
        (f64) Step, 
        String
    );

    *Value = (i64) RoundF64(FloatValue);
    Result.Value = (f64) *Value;

    return Result;
}

UINumericEditResult 
UINumericEdit(i64* Value, r1i64 Range, i64 Step, char* Fmt, ...)
{
    TempArena Scratch = GetScratch(NULL, 0);
    va_list Args;

    va_start(Args, Fmt);

    Str8 String = ArenaPushStrFmtV(Scratch.MemPool, Fmt, Args);

    va_end(Args);

    UINumericEditResult Result = UINumericEdit(Value, Range, Step, String);

    ReleaseScratch(Scratch);

    return Result;
}

UISignal 
UIProgressBar(f32 Progress, Str8 String)
{
    UIBox* Box = UIBuildBoxFromStr(0, String);

    UIProgressBarDrawData* DrawData = ArenaPushArrayZero(
        UIBuildMemPool(), 
        UIProgressBarDrawData, 
        1
    );

    DrawData->Progress = Progress;
    UIBoxEquipCustomDraw(Box, UIProgressBarDraw, DrawData);

    UISignal Result = UISignalFromBox(Box);

    return Result;
}

UISignal 
UIProgressBar(f32 Progress, char* Fmt, ...)
{
    TempArena Scratch = GetScratch(NULL, 0);
    va_list Args;

    va_start(Args, Fmt);

    Str8 String = ArenaPushStrFmtV(Scratch.MemPool, Fmt, Args);

    va_end(Args);

    UISignal Result = UIProgressBar(Progress, String);

    ReleaseScratch(Scratch);

    return Result;
}

UISignal 
UIProgressSpinner(f32 SpinRate, Str8 String)
{
    local readonly Str8 Frames[] = {
        Str8Lit("\xE2\xA0\x8B"),
        Str8Lit("\xE2\xA0\x99"),
        Str8Lit("\xE2\xA0\xB9"),
        Str8Lit("\xE2\xA0\xB8"),
        Str8Lit("\xE2\xA0\xBC"),
        Str8Lit("\xE2\xA0\xB4"),
        Str8Lit("\xE2\xA0\xA6"),
        Str8Lit("\xE2\xA0\xA7"),
        Str8Lit("\xE2\xA0\x87"),
        Str8Lit("\xE2\xA0\x8F")
    };
    f64 Seconds = (f64) TimeGetTimestamp() / 1000000.0;
    u64 FrameIndex = (u64) (Seconds * (f64) SpinRate * (f64) ARRAY_COUNT(Frames)) % ARRAY_COUNT(Frames);

    UISetNextPreferredWidth(UI_PX(1.0f, 1.0f));
    UISetNextPreferredHeight(UI_PX(1.0f, 1.0f));
    UISetNextTextColour(UIColourFromName("accent"_s8));

    UIBox* Box = UIBuildBoxFromStr(UI_BOX_KIND_DRAW_TEXT, String);

    UIBoxEquipDisplayStr(Box, Frames[FrameIndex]);
    GetTLS()->UI_STATE->IsAnimating = TRUE;

    UISignal Result = UISignalFromBox(Box);

    return Result;
}

UISignal 
UIProgressSpinner(f32 SpinRate, char* Fmt, ...)
{
    TempArena Scratch = GetScratch(NULL, 0);
    va_list Args;

    va_start(Args, Fmt);

    Str8 String = ArenaPushStrFmtV(Scratch.MemPool, Fmt, Args);

    va_end(Args);

    UISignal Result = UIProgressSpinner(SpinRate, String);

    ReleaseScratch(Scratch);

    return Result;
}

b32 
UITreeNodeBegin(b32* IsExpanded, b32 HasChildren, Str8 String)
{
    ThreadLocalStorage* TLS = GetTLS();

    ++TLS->UITreeNodeDepth;

    f32 IndentSize = 2.0f * (f32) (TLS->UITreeNodeDepth - 1);

    UISetNextChildLayoutAxis(AXIS_2D_Y);

    UIBox* ContainerBox = UIBuildBoxFromStr(0, String);

    UIPushParent(ContainerBox);

    UISetNextChildLayoutAxis(AXIS_2D_X);

    UIBox* HeaderBox = UIBuildBoxFromStr(
        UI_BOX_KIND_CLICKABLE | UI_BOX_KIND_DRAW_HOT_EFFECTS,
        "###header"_s8
    );

    UIParent(HeaderBox) {
        UISpacer(UI_PX(IndentSize, 1.0f));

        if (HasChildren) {
            UIPreferredWidth(UI_PX(2.0f, 1.0f)) {
                UIPreferredHeight(UI_PX(1.0f, 1.0f)) {
                    UIPushHoverCursor(CURSOR_KIND_HAND_POINT);

                    UISignal ExpanderSig = UIExpander(
                        *IsExpanded, 
                        "###expander"_s8
                    );

                    if (UI_CLICKED(ExpanderSig))
                        *IsExpanded = !(*IsExpanded);

                    UIPopHoverCursor();
                }
            }
        } else {
            UISpacer(UI_PX(2.0f, 1.0f));
        }

        UILabel(UIDisplayPartFromKeyStr(String));
    }

    UISignal HeaderSig = UISignalFromBox(HeaderBox);

    if (HasChildren && UI_CLICKED(HeaderSig))
        *IsExpanded = !(*IsExpanded);

    if (HasChildren && *IsExpanded) {
        UISetNextChildLayoutAxis(AXIS_2D_Y);

        UIBox* ChildContainer = UIBuildBoxFromStr(0, "###children"_s8);

        UIPushParent(ChildContainer);

        return TRUE;
    }

    return FALSE;
}

b32 
UITreeNodeBegin(b32* IsExpanded, b32 HasChildren, char* Fmt, ...)
{
    TempArena Scratch = GetScratch(NULL, 0);
    va_list Args;

    va_start(Args, Fmt);

    Str8 String = ArenaPushStrFmtV(Scratch.MemPool, Fmt, Args);

    va_end(Args);

    b32 Result = UITreeNodeBegin(IsExpanded, HasChildren, String);

    ReleaseScratch(Scratch);

    return Result;
}

void 
UITreeNodeEnd(void)
{
    ThreadLocalStorage* TLS = GetTLS();

    UIPopParent();
    --TLS->UITreeNodeDepth;
}

void 
UITreeNodeLeaf(Str8 String)
{
    ThreadLocalStorage* TLS = GetTLS();
    f32 IndentSize = 2.0f * (f32) TLS->UITreeNodeDepth;

    UISetNextChildLayoutAxis(AXIS_2D_X);

    UIBox* Box = UIBuildBoxFromStr(
        UI_BOX_KIND_CLICKABLE | UI_BOX_KIND_DRAW_HOT_EFFECTS, 
        String
    );

    UIParent(Box) {
        UISpacer(UI_PX(IndentSize, 1.0f));
        UISpacer(UI_PX(2.0f, 1.0f));
        UILabel(UIDisplayPartFromKeyStr(String));
    }
}

void 
UITreeNodeLeaf(char* Fmt, ...)
{
    TempArena Scratch = GetScratch(NULL, 0);
    va_list Args;

    va_start(Args, Fmt);

    Str8 String = ArenaPushStrFmtV(Scratch.MemPool, Fmt, Args);

    va_end(Args);
    UITreeNodeLeaf(String);
    ReleaseScratch(Scratch);
}

void 
UIRadioGroupBegin(i64* SelectedIndex, i64 OptionCount, Str8 String)
{
    ThreadLocalStorage* TLS = GetTLS();

    TLS->UIRadioGroupSelectedIndex = SelectedIndex;
    TLS->UIRadioGroupOptionCount = OptionCount;
    UISetNextChildLayoutAxis(AXIS_2D_Y);

    UIBox* Box = UIBuildBoxFromStr(0, String);

    UIPushParent(Box);
}

void 
UIRadioGroupBegin(i64* SelectedIndex, i64 OptionCount, char* Fmt, ...)
{
    TempArena Scratch = GetScratch(NULL, 0);
    va_list Args;

    va_start(Args, Fmt);

    Str8 String = ArenaPushStrFmtV(Scratch.MemPool, Fmt, Args);

    va_end(Args);

    UIRadioGroupBegin(SelectedIndex, OptionCount, String);
    ReleaseScratch(Scratch);
}

UISignal 
UIRadioOption(i64 Index, Str8 String)
{
    ThreadLocalStorage* TLS = GetTLS();

    UIPushHoverCursor(CURSOR_KIND_HAND_POINT);
    UISetNextChildLayoutAxis(AXIS_2D_X);

    UIBox* Box = UIBuildBoxFromStr(
        UI_BOX_KIND_CLICKABLE | UI_BOX_KIND_DRAW_HOT_EFFECTS,
        String
    );

    b32 IsSelected = (
        TLS->UIRadioGroupSelectedIndex && 
        *TLS->UIRadioGroupSelectedIndex == Index
    );

    UIParent(Box) {
        UIPreferredWidth(UI_TEXT_DIM(0.0f, 1.0f)) {
            UIPreferredHeight(UI_PX(1.0f, 1.0f)) {
                if (IsSelected)
                    UISetNextTextColour(UIColourFromName("accent"_s8));

                UIBox* RadioButton = UIBuildBoxFromStr(
                    UI_BOX_KIND_DRAW_TEXT, 
                    "###radio"_s8
                );

                UIBoxEquipDisplayStr(
                    RadioButton, 
                    UIIconStringFromIconKind(
                        IsSelected
                            ? UI_ICON_KIND_RADIO_FILLED 
                            : UI_ICON_KIND_RADIO_HOLLOW
                    )
                );
            }
        }

        UISpacer(UI_PX(1.0f, 1.0f));
        UILabel(UIDisplayPartFromKeyStr(String));
    }

    UISignal Sig = UISignalFromBox(Box);

    if (UI_CLICKED(Sig) && TLS->UIRadioGroupSelectedIndex)
        *TLS->UIRadioGroupSelectedIndex = Index;

    UIPopHoverCursor();

    return Sig;
}

UISignal 
UIRadioOption(i64 Index, char* Fmt, ...)
{
    TempArena Scratch = GetScratch(NULL, 0);
    va_list Args;

    va_start(Args, Fmt);

    Str8 String = ArenaPushStrFmtV(Scratch.MemPool, Fmt, Args);

    va_end(Args);

    UISignal Result = UIRadioOption(Index, String);

    ReleaseScratch(Scratch);

    return Result;
}

void 
UIRadioGroupEnd(void)
{
    ThreadLocalStorage* TLS = GetTLS();

    UIPopParent();
    TLS->UIRadioGroupSelectedIndex = NULL;
    TLS->UIRadioGroupOptionCount = 0;
}

UIBox* 
UIBreadCrumbBegin(i64 Count, Str8 String)
{
    ThreadLocalStorage* TLS = GetTLS();

    TLS->UIBreadCrumbCount = Count;
    TLS->UIBreadCrumbCurrentIndex = 0;

    UISetNextChildLayoutAxis(AXIS_2D_X);

    TLS->UIBreadCrumbContainer = UIBuildBoxFromStr(0, String);

    UIPushParent(TLS->UIBreadCrumbContainer);

    return TLS->UIBreadCrumbContainer;
}

UIBox* 
UIBreadCrumbBegin(i64 Count, char* Fmt, ...)
{
    TempArena Scratch = GetScratch(NULL, 0);
    va_list Args;

    va_start(Args, Fmt);

    Str8 String = ArenaPushStrFmtV(Scratch.MemPool, Fmt, Args);

    va_end(Args);

    UIBox* Result = UIBreadCrumbBegin(Count, String);

    ReleaseScratch(Scratch);

    return Result;
}

UISignal 
UIBreadCrumbItem(i64 Index, Str8 String)
{
    ThreadLocalStorage* TLS = GetTLS();

    UIPushHoverCursor(CURSOR_KIND_HAND_POINT);

    b32 IsLast = (Index == TLS->UIBreadCrumbCount - 1);

    UIBox* Box = UIBuildBoxFromStr(
        (
            UI_BOX_KIND_CLICKABLE |
            UI_BOX_KIND_DRAW_TEXT |
            UI_BOX_KIND_DRAW_HOT_EFFECTS |
            (IsLast ? 0 : UI_BOX_KIND_DRAW_TEXT_WEAK)
        ),
        String
    );

    UIBoxEquipDisplayStr(Box, UIDisplayPartFromKeyStr(String));

    UISignal Sig = UISignalFromBox(Box);

    if (!IsLast) {
        UISpacer(UI_PX(1.0f, 1.0f));

        UIPreferredWidth(UI_TEXT_DIM(0.0f, 1.0f)) {
            UIBox* Separator = UIBuildBoxFromStr(
                UI_BOX_KIND_DRAW_TEXT, 
                "###sep"_s8
            );

            UIBoxEquipDisplayStr(
                Separator, 
                UIIconStringFromIconKind(
                    UI_ICON_KIND_RIGHT_CARET
                )
            );
        }

        UISpacer(UI_PX(1.0f, 1.0f));
    }

    ++TLS->UIBreadCrumbCurrentIndex;

    UIPopHoverCursor();

    return Sig;
}

UISignal 
UIBreadCrumbItem(i64 Index, char* Fmt, ...)
{
    TempArena Scratch = GetScratch(NULL, 0);
    va_list Args;

    va_start(Args, Fmt);

    Str8 String = ArenaPushStrFmtV(Scratch.MemPool, Fmt, Args);

    va_end(Args);

    UISignal Result = UIBreadCrumbItem(Index, String);

    ReleaseScratch(Scratch);

    return Result;
}

void 
UIBreadCrumbEnd(void)
{
    ThreadLocalStorage* TLS = GetTLS();

    UIPopParent();
    TLS->UIBreadCrumbContainer = NULL;
    TLS->UIBreadCrumbCount = 0;
    TLS->UIBreadCrumbCurrentIndex = 0;
}

INTERNAL
UI_BOX_CUSTOM_DRAW(UISliderDraw)
{
    UISliderDrawData* Data = (UISliderDrawData*) UserData;
    Axis2D Axis = (Data->Kind == UI_SLIDER_KIND_HORZ) 
        ? AXIS_2D_X 
        : AXIS_2D_Y;
    f32 NormValue = (Data->Range.Max > Data->Range.Min)
        ? (Data->Value - Data->Range.Min) / (Data->Range.Max - Data->Range.Min)
        : 0.0f;

    NormValue = CLAMP(0.0f, NormValue, 1.0f);

    f32 TrackLen = Length(Box->Rect).V[Axis];
    r1f32 Fill = (Axis == AXIS_2D_X)
        ? Rng(Box->Rect.X0, Box->Rect.X0 + NormValue * TrackLen)
        : Rng(Box->Rect.Y1 - NormValue * TrackLen, Box->Rect.Y1);
    v4f32 TrackColour = UIColourFromTagsKeyName(Box->TagsKey, "background"_s8);
    v4f32 FillColour = Lerp(
        UIColourFromTagsKeyName(Box->TagsKey, "accent"_s8),
        UIColourFromTagsKeyName(Box->TagsKey, "text"_s8),
        Box->Active
    );

    DrawBar(Box->Rect, Axis, Fill, FillColour, TrackColour);
}

INTERNAL
UI_BOX_CUSTOM_DRAW(UIToggleSwitchDraw)
{
    UIToggleSwitchDrawData* Data = (UIToggleSwitchDrawData*) UserData;
    f32 KnobSize = 2.0f;
    f32 KnobX = LerpF32(Data->EnabledT, Box->Rect.X0, Box->Rect.X1 - KnobSize);
    v4f32 TrackColour = Lerp(
        UIColourFromTagsKeyName(Box->TagsKey, "background"_s8),
        UIColourFromTagsKeyName(Box->TagsKey, "accent"_s8),
        Data->EnabledT
    );
    v4f32 KnobColour = UIColourFromTagsKeyName(Box->TagsKey, "text"_s8);

    DrawBar(
        Box->Rect, 
        AXIS_2D_X, 
        Rng(KnobX, KnobX + KnobSize), 
        KnobColour, 
        TrackColour
    );
}

INTERNAL
UI_BOX_CUSTOM_DRAW(UIProgressBarDraw)
{
    UIProgressBarDrawData* Data = (UIProgressBarDrawData*) UserData;
    f32 Progress = CLAMP(0.0f, Data->Progress, 1.0f);
    r1f32 Fill = Rng(
        Box->Rect.X0, 
        Box->Rect.X0 + Progress * Length(Box->Rect).X
    );
    v4f32 TrackColour = UIColourFromTagsKeyName(Box->TagsKey, "background"_s8);
    v4f32 FillColour = UIColourFromTagsKeyName(Box->TagsKey, "accent"_s8);

    DrawBar(Box->Rect, AXIS_2D_X, Fill, FillColour, TrackColour);
}

INTERNAL
UI_BOX_CUSTOM_DRAW(UIScrollBarDraw)
{
    UIScrollBarDrawData* Data = (UIScrollBarDrawData*) UserData;
    Axis2D Axis = Data->Axis;
    f32 TrackStart = Box->Rect.Point0.V[Axis];
    f32 TrackLen = Length(Box->Rect).V[Axis];
    r1f32 Fill = Rng(
        TrackStart + Data->BeforeFrac * TrackLen,
        TrackStart + (Data->BeforeFrac + Data->ScrollerFrac) * TrackLen
    );
    f32 Emphasis = IsUIBoxEmpty(Data->ScrollerBox) 
        ? 0.0f : MAX(Data->ScrollerBox->Hot, Data->ScrollerBox->Active);
    v4f32 TrackColour = UIColourFromTagsKeyName(Box->TagsKey, "background"_s8);
    v4f32 ThumbColour = Lerp(
        UIColourFromTagsKeyName(Box->TagsKey, "border"_s8),
        UIColourFromTagsKeyName(Box->TagsKey, "accent"_s8),
        Emphasis
    );

    DrawBar(Box->Rect, Axis, Fill, ThumbColour, TrackColour);
}
