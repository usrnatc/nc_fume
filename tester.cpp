#include "libnc.h"

#define TEST_LOG_ROWS 12

struct TestState {
    Arena* FrameMemPool;
    Arena* LogMemPool;
    Str8   Log[TEST_LOG_ROWS];
    u64    LogHead;
    u64    FrameIndex;
    u64    EventCount;
    f64    LastFrameUSecs;
    v2f32  Mouse;
    b32    ShouldAnimate;
    b32    ShouldQuit;
    Str8   PrevPaste;
    Str8   PrevDrop;
    u64    Ticks;
};

internal void
TestPushLog(TestState* State, Str8 Line)
{
    if (State->LogHead % 256 == 255) {
        ArenaClear(State->LogMemPool);
        MemZero(State->Log, sizeof(State->Log));
        State->PrevPaste = ""_s8;
        State->PrevDrop = ""_s8;
    }

    State->Log[State->LogHead % TEST_LOG_ROWS] = ArenaPushStrCpy(
        State->LogMemPool,
        Line
    );
    ++State->LogHead;
}

internal void
TestText(v2f32 Position, v4f32 Colour, FancyStrFlag Flags, Str8 String)
{
    FancyStrParams Params = {};
    FancyStrList List = {};

    Params.Colour = Colour;
    Params.Flags = Flags;
    ListPush(REND_STATE->FrameMemPool, &List, &Params, NULL, String);
    DrawFancyStrList(Position, 4.0f, &List, 100000.0f, ""_s8, NULL, {});
}

internal void
TestHandleEvents(TestState* State, InputEventList* Events)
{
    TempArena Scratch = GetScratch(NULL, 0);

    if (Events->Count >= 8)
        State->PrevPaste = ""_s8;

    for (InputEvent* Event = Events->Head; Event; Event = Event->Next) {
        ++State->EventCount;

        Str8 Line = {};

        switch (Event->Kind) {
            default: {
                Line = StrFromEventKind(Event->Kind);
            } break;

            case EVENT_KIND_PRESS:
            case EVENT_KIND_RELEASE: {
                Line = ArenaPushStrFmt(
                    Scratch.MemPool,
                    "%S %S%s%s at %d,%d",
                    PRINT_STR(StrFromEventKind(Event->Kind)),
                    PRINT_STR(StrFromInputModifierInput(Scratch.MemPool, Event->Modifier, Event->Input)),
                    Event->IsRepeat ? " repeat" : "",
                    Event->RightSided ? " right" : "",
                    (i32) Event->Position.X,
                    (i32) Event->Position.Y
                );

                if (Event->Kind == EVENT_KIND_PRESS && Event->Input == INPUT_KIND_ESC)
                    State->ShouldQuit = TRUE;

                if (Event->Kind == EVENT_KIND_PRESS && Event->Input == INPUT_KIND_C && (Event->Modifier & INPUT_MOD_KIND_CTRL))
                    State->ShouldQuit = TRUE;

                if (Event->Kind == EVENT_KIND_PRESS && Event->Input == INPUT_KIND_SPACE)
                    State->ShouldAnimate = !State->ShouldAnimate;

                if (Event->Kind == EVENT_KIND_PRESS && Event->Input == INPUT_KIND_FUNC_2) {
                    Str8List Parts = {};

                    for (u64 Index = 0; Index < REND_STATE->PrevFrame.Size; ++Index) {
                        u8 Byte = REND_STATE->PrevFrame.Str[Index];

                        if (Byte == 0x1B)
                            ListPush(Scratch.MemPool, &Parts, "\n\\e"_s8);
                        else
                            ListPush(Scratch.MemPool, &Parts, Str(&REND_STATE->PrevFrame.Str[Index], 1));
                    }

                    WriteFileContents("smoke_frame.txt"_s8, StrListJoin(Scratch.MemPool, &Parts, NULL));
                }

                if (Event->Kind == EVENT_KIND_PRESS && Event->Input == INPUT_KIND_FUNC_3)
                    REND_STATE->IsBufferValid = FALSE;
            } break;

            case EVENT_KIND_TEXT: {
                u8 Bytes[4];
                u32 Count = UTF8Encode(Bytes, Event->Character);

                Line = ArenaPushStrFmt(
                    Scratch.MemPool,
                    "Text U+%04X \"%S\"",
                    Event->Character,
                    PRINT_STR(Str(Bytes, Count))
                );

                if (Events->Count >= 8) {
                    State->PrevPaste = ArenaPushStrFmt(
                        State->LogMemPool,
                        "%S%S",
                        PRINT_STR(State->PrevPaste),
                        PRINT_STR(Str(Bytes, Count))
                    );
                }
            } break;

            case EVENT_KIND_MOUSE_MOVE: {
                State->Mouse = Event->Position;
                Line = ArenaPushStrFmt(
                    Scratch.MemPool,
                    "MouseMove %d,%d",
                    (i32) Event->Position.X,
                    (i32) Event->Position.Y
                );
            } break;

            case EVENT_KIND_SCROLL: {
                Line = ArenaPushStrFmt(
                    Scratch.MemPool,
                    "Scroll %.1f,%.1f at %d,%d",
                    Event->PositionDelta.X,
                    Event->PositionDelta.Y,
                    (i32) Event->Position.X,
                    (i32) Event->Position.Y
                );
            } break;

            case EVENT_KIND_FILE_DROP: {
                Str8JoinPart Join = { ""_s8, " | "_s8, ""_s8 };

                State->PrevDrop = StrListJoin(State->LogMemPool, &Event->Strings, &Join);
                Line = ArenaPushStrFmt(
                    Scratch.MemPool,
                    "FileDrop %llu path(s): %S",
                    Event->Strings.Count,
                    PRINT_STR(State->PrevDrop)
                );
            } break;

            case EVENT_KIND_CONSOLE_CLOSE: {
                State->ShouldQuit = TRUE;
                Line = "ConsoleClose"_s8;
            } break;
        }

        if (Event->Kind != EVENT_KIND_MOUSE_MOVE || !Event->Next || Event->Next->Kind != EVENT_KIND_MOUSE_MOVE)
            TestPushLog(State, Line);
    }

    ReleaseScratch(Scratch);
}

internal void
TestDrawFrame(TestState* State)
{
    v4f32 Field   = RGBAFromU32(0x0000AAFF);
    v4f32 Grey    = RGBAFromU32(0xAAAAAAFF);
    v4f32 Black   = RGBAFromU32(0x000000FF);
    v4f32 White   = RGBAFromU32(0xFFFFFFFF);
    v4f32 Cyan    = RGBAFromU32(0x00AAAAFF);
    v4f32 Yellow  = RGBAFromU32(0xFFFF55FF);
    v4f32 Red     = RGBAFromU32(0xFF5555FF);
    v4f32 Green   = RGBAFromU32(0x55FF55FF);
    v4f32 Overlay = Vec(1.0f, 1.0f, 1.0f, 0.25f);

    ArenaClear(State->FrameMemPool);
    DBeginFrame(State->FrameMemPool, Grey, Field);

    v2i32 Size = RendSize();
    f32 W = (f32) Size.X;
    f32 H = (f32) Size.Y;

    DrawRect(Rng(0.0f, 0.0f, W, 1.0f), Grey, 0.0f);
    TestText(Vec(2.0f, 0.0f), Black, 0, "File  Edit  View  Help"_s8);
    TestText(Vec(2.0f, 0.0f), Red, FANCY_STR_FLAG_UNDERLINE, "F"_s8);
    TestText(Vec(W - 16.0f, 0.0f), Black, 0, "FUME TUI SMOKE"_s8);

    r2f32 BoxA = Rng(2.0f, 2.0f, 42.0f, 16.0f);

    DrawRect(BoxA, Field, 0.0f);
    DrawBorder(BoxA, Grey, FALSE);
    TestText(Vec(4.0f, 2.0f), Yellow, FANCY_STR_FLAG_BOLD, " GLYPHS "_s8);
    TestText(Vec(4.0f, 4.0f), White, 0,  "single  \xE2\x94\x8C\xE2\x94\x80\xE2\x94\xAC\xE2\x94\x80\xE2\x94\x90 \xE2\x94\x9C\xE2\x94\xBC\xE2\x94\xA4 \xE2\x94\x94\xE2\x94\xB4\xE2\x94\x98"_s8);
    TestText(Vec(4.0f, 5.0f), White, 0,  "double  \xE2\x95\x94\xE2\x95\x90\xE2\x95\xA6\xE2\x95\x90\xE2\x95\x97 \xE2\x95\xA0\xE2\x95\xAC\xE2\x95\xA3 \xE2\x95\x9A\xE2\x95\xA9\xE2\x95\x9D"_s8);
    TestText(Vec(4.0f, 6.0f), White, 0,  "rounded \xE2\x95\xAD\xE2\x94\x80\xE2\x95\xAE \xE2\x95\xB0\xE2\x94\x80\xE2\x95\xAF"_s8);
    TestText(Vec(4.0f, 7.0f), White, 0,  "eighths \xE2\x96\x8F\xE2\x96\x8E\xE2\x96\x8D\xE2\x96\x8C\xE2\x96\x8B\xE2\x96\x8A\xE2\x96\x89\xE2\x96\x88 \xE2\x96\x81\xE2\x96\x82\xE2\x96\x83\xE2\x96\x84\xE2\x96\x85\xE2\x96\x86\xE2\x96\x87\xE2\x96\x88"_s8);
    TestText(Vec(4.0f, 8.0f), White, 0,  "shade   \xE2\x96\x91\xE2\x96\x92\xE2\x96\x93\xE2\x96\x88"_s8);
    TestText(Vec(4.0f, 9.0f), White, 0,  "braille \xE2\xA0\x8B\xE2\xA0\x99\xE2\xA0\xB9\xE2\xA0\xB8\xE2\xA0\xBC\xE2\xA0\xB4\xE2\xA0\xA6\xE2\xA0\xA7\xE2\xA0\x87\xE2\xA0\x8F"_s8);
    TestText(Vec(4.0f, 10.0f), White, 0, "arrows  \xE2\x96\xB2\xE2\x96\xBC\xE2\x97\x82\xE2\x96\xB8 \xE2\x86\x91\xE2\x86\x93\xE2\x86\x90\xE2\x86\x92 \xE2\x86\x94\xE2\x86\x95 \xE2\x80\xBA \xE2\x80\xA6"_s8);
    TestText(Vec(4.0f, 11.0f), White, 0, "marks   \xE2\x9C\x93 \xE2\x80\xA2 \xE2\x97\x8F \xE2\x97\x8B \xE2\x9C\xA5 \xE2\x8A\x98 [x] (\xE2\x80\xA2)"_s8);
    TestText(Vec(4.0f, 12.0f), White, FANCY_STR_FLAG_BOLD, "bold"_s8);
    TestText(Vec(9.0f, 12.0f), White, FANCY_STR_FLAG_DIMMED, "dim"_s8);
    TestText(Vec(13.0f, 12.0f), White, FANCY_STR_FLAG_ITALIC, "italic"_s8);
    TestText(Vec(20.0f, 12.0f), White, FANCY_STR_FLAG_UNDERLINE, "under"_s8);
    TestText(Vec(26.0f, 12.0f), White, FANCY_STR_FLAG_STRIKETHROUGH, "strike"_s8);
    TestText(Vec(33.0f, 12.0f), White, FANCY_STR_FLAG_INVERSE, "inverse"_s8);
    TestText(Vec(4.0f, 13.0f), White, 0, "wide    \xE6\xBC\xA2\xE5\xAD\x97 \xF0\x9F\x9A\x80 <- should be ???"_s8);
    TestText(Vec(4.0f, 14.0f), White, 0, "tab\tstop\tstop"_s8);

    r2f32 BoxB = Rng(46.0f, 2.0f, 78.0f, 12.0f);

    DrawShadow(BoxB);
    DrawRect(BoxB, Grey, 0.0f);
    DrawBorder(BoxB, Black, TRUE);
    TestText(Vec(48.0f, 2.0f), Black, FANCY_STR_FLAG_BOLD, " BARS "_s8);

    f32 T = State->ShouldAnimate
        ? (f32) ((State->Ticks % 240) / 240.0)
        : CLAMP(0.0f, (State->Mouse.X - BoxB.X0 - 2.0f) / (BoxB.X1 - BoxB.X0 - 4.0f), 1.0f);

    r2f32 BarH = Rng(48.0f, 4.0f, 76.0f, 5.0f);
    r2f32 BarV = Rng(74.0f, 6.0f, 75.0f, 11.0f);
    r2f32 Knob = Rng(48.0f, 6.0f, 52.0f, 7.0f);

    DrawBar(BarH, AXIS_2D_X, Rng(BarH.X0, BarH.X0 + T * Length(BarH).X), Cyan, Black);
    DrawBar(BarV, AXIS_2D_Y, Rng(BarV.Y1 - T * Length(BarV).Y, BarV.Y1), Cyan, Black);
    DrawBar(Knob, AXIS_2D_X, Rng(Knob.X0 + T * 2.0f, Knob.X0 + T * 2.0f + 2.0f), White, Black);
    TestText(Vec(48.0f, 8.0f), Black, 0, ArenaPushStrFmt(State->FrameMemPool, "t = %.3f", T));
    TestText(Vec(48.0f, 9.0f), Black, 0, State->ShouldAnimate ? "SPACE: stop"_s8 : "SPACE: animate, or move the mouse"_s8);
    DrawRect(Rng(60.0f, 6.0f, 80.0f, 14.0f), Overlay, 0.0f);

    for (i32 X = 0; X < Size.X; ++X) {
        f32 U = (f32) X / (f32) MAX(Size.X - 1, 1);
        v4f32 C = Vec(U, 1.0f - U, 0.5f + 0.5f * SinF32(U * 6.2831f), 1.0f);

        RendFillRect(Rng(X, 17, X + 1, 18), C);
        RendFillRect(Rng(X, 18, X + 1, 19), C);
    }

    RendDarkenRect(Rng(0, 18, Size.X, 19), 0.5f);

    DClipScope(Rng(2.0f, 20.0f, 30.0f, 21.0f)) {
        DrawRect(Rng(0.0f, 20.0f, W, 21.0f), Black, 0.0f);
        TestText(Vec(2.0f, 20.0f), Green, 0, "clipped at column 30 ->|<- this text must not be visible"_s8);
    }

    {
        FancyStrParams Params = {};
        FancyStrList List = {};
        FMRangeList Ranges = {};

        Params.Colour = White;
        ListPush(State->FrameMemPool, &List, &Params, NULL, "truncated with a trailer, the trailer is one cell"_s8);
        DrawTruncatedFancyStrList(Vec(2.0f, 21.0f), 4.0f, &List, 30.0f, "\xE2\x80\xA6"_s8);

        FMRangeNode Node = { NULL, Rng((u64) 10, (u64) 16) };

        Ranges.Head = &Node;
        Ranges.Tail = &Node;
        Ranges.Count = 1;
        DrawTruncatedFancyStrListFuzzyMatches(Vec(2.0f, 22.0f), 4.0f, &List, W, &Ranges, Yellow);
    }

    DrawLine(Vec(46.0f, 14.0f), Vec(78.0f, 14.0f), Grey);
    DrawLine(Vec(62.0f, 13.0f), Vec(62.0f, 16.0f), Grey);
    DrawLine(Vec(46.0f, 16.0f), Vec(78.0f, 16.0f), Grey);

    f32 LogY = 24.0f;

    TestText(Vec(2.0f, LogY), Yellow, FANCY_STR_FLAG_BOLD, "EVENTS"_s8);

    for (u64 Index = 0; Index < TEST_LOG_ROWS; ++Index) {
        u64 LineIndex = (State->LogHead + Index) % TEST_LOG_ROWS;

        if (State->Log[LineIndex].Size)
            TestText(Vec(2.0f, LogY + 1.0f + (f32) Index), White, 0, State->Log[LineIndex]);
    }

    TestText(Vec(2.0f, LogY + 14.0f), Cyan, 0, ArenaPushStrFmt(State->FrameMemPool, "last paste: %S", PRINT_STR(StrPrefix(State->PrevPaste, 200))));
    TestText(Vec(2.0f, LogY + 15.0f), Cyan, 0, ArenaPushStrFmt(State->FrameMemPool, "last drop:  %S", PRINT_STR(StrPrefix(State->PrevDrop, 200))));

    DrawRect(Rng(0.0f, H - 1.0f, W, H), Grey, 0.0f);
    TestText(
        Vec(1.0f, H - 1.0f),
        Black,
        0,
        ArenaPushStrFmt(
            State->FrameMemPool,
            "%dx%d  frame %llu  %llu B  %.0f us  events %llu  mouse %d,%d  ESC quits",
            Size.X,
            Size.Y,
            State->FrameIndex,
            REND_STATE->PrevFrameBytes,
            State->LastFrameUSecs,
            State->EventCount,
            (i32) State->Mouse.X,
            (i32) State->Mouse.Y
        )
    );
    DrawCaret(Vec(4.0f, 15.0f));
    TestText(Vec(5.0f, 15.0f), Grey, FANCY_STR_FLAG_DIMMED, "<- caret"_s8);
    DrawMousePointer(State->Mouse, GetCursorKind());

    DEndFrame();
}

#define TEST_UI_ROW_COUNT 200000

struct TestUIThemeEntry {
    u32  Colour;
    Str8 Tags[2];
};

struct TestUIState {
    Arena*        FrameMemPool;
    UIScrollPoint Scroll;
    v2i64         Cursor;
    v2i64         Mark;
    f32           ColumnPercents[6];
    TextPoint     EditCursor;
    TextPoint     EditMark;
    u8            EditBuffer[32];
    u64           EditSize;
    i64           SortColumn;
    b32           SortAscending;
    b32           IsLoading;
    Str8          LastAction;
    u64           FrameIndex;
    f64           LastFrameUSecs;
    b32           ShouldQuit;
};

internal void
TestUIPushEvents(
    Arena* MemPool, 
    TestUIState* State, 
    InputEventList* Events, 
    UIEventList* UIEvents
) {
    for (InputEvent* Event = Events->Head; Event; Event = Event->Next) {
        b32 Shift = !!(Event->Modifier & INPUT_MOD_KIND_SHIFT);
        b32 Ctrl = !!(Event->Modifier & INPUT_MOD_KIND_CTRL);
        u8 TextBytes[4] = {};
        UIEvent UIEvt = {};

        UIEvt.Input = Event->Input;
        UIEvt.Modifiers = Event->Modifier;
        UIEvt.Position = Event->Position;
        UIEvt.TimestampUSecs = Event->TimeStampUSecs;

        switch (Event->Kind) {
            default: {} break;

            case EVENT_KIND_CONSOLE_CLOSE: {
                State->ShouldQuit = TRUE;
            } break;

            case EVENT_KIND_RELEASE: {
                UIEvt.Kind = UI_EVENT_KIND_RELEASE;
            } break;

            case EVENT_KIND_MOUSE_MOVE: {
                UIEvt.Kind = UI_EVENT_KIND_MOUSE_MOVE;
            } break;

            case EVENT_KIND_SCROLL: {
                UIEvt.Kind = UI_EVENT_KIND_SCROLL;
                UIEvt.DeltaF32 = Event->PositionDelta;
            } break;

            case EVENT_KIND_FILE_DROP: {
                UIEvt.Kind = UI_EVENT_KIND_FILE_DROP;
                UIEvt.Paths = Event->Strings;
            } break;

            case EVENT_KIND_TEXT: {
                UIEvt.Kind = UI_EVENT_KIND_TEXT;
                UIEvt.String = Str(
                    TextBytes, 
                    UTF8Encode(TextBytes, Event->Character)
                );
            } break;

            case EVENT_KIND_PRESS: {
                UIEvt.Kind = UI_EVENT_KIND_PRESS;

                switch (Event->Input) {
                    default: {} break;

                    case INPUT_KIND_Q: {
                        if (Ctrl)
                            State->ShouldQuit = TRUE;
                    } break;

                    case INPUT_KIND_RETURN: {
                        UIEvt.Slot = UI_EVENT_ACTION_SLOT_ACCEPT;
                    } break;

                    case INPUT_KIND_ESC: {
                        UIEvt.Slot = UI_EVENT_ACTION_SLOT_CANCEL;
                    } break;

                    case INPUT_KIND_LEFT:
                    case INPUT_KIND_RIGHT: {
                        UIEvt.Kind = UI_EVENT_KIND_NAVIGATE;
                        UIEvt.Flags = UI_EVENT_FLAG_EXPLICIT_DIRECTIONAL | (
                            Shift 
                                ? UI_EVENT_FLAG_KEEP_MARK 
                                : (UI_EVENT_FLAG_PICK_SELECT_SIDE | UI_EVENT_FLAG_ZERO_DELTA_ON_SELECT)
                        );
                        UIEvt.DeltaStride = Ctrl 
                            ? UI_EVENT_DELTA_STRIDE_WORD 
                            : UI_EVENT_DELTA_STRIDE_CHAR;
                        UIEvt.DeltaI32 = Vec((Event->Input == INPUT_KIND_LEFT) ? -1 : 1, 0);
                    } break;

                    case INPUT_KIND_UP:
                    case INPUT_KIND_DOWN: {
                        UIEvt.Kind = UI_EVENT_KIND_NAVIGATE;
                        UIEvt.Flags = UI_EVENT_FLAG_EXPLICIT_DIRECTIONAL | (
                            Shift ? UI_EVENT_FLAG_KEEP_MARK : 0
                        );
                        UIEvt.DeltaStride = UI_EVENT_DELTA_STRIDE_CHAR;
                        UIEvt.DeltaI32 = Vec(0, (Event->Input == INPUT_KIND_UP) ? -1 : 1);
                    } break;

                    case INPUT_KIND_HOME:
                    case INPUT_KIND_END: {
                        i32 Direction = (Event->Input == INPUT_KIND_HOME) ? -1 : 1;

                        UIEvt.Kind = UI_EVENT_KIND_NAVIGATE;
                        UIEvt.Flags = Shift ? UI_EVENT_FLAG_KEEP_MARK : 0;
                        UIEvt.DeltaStride = Ctrl 
                            ? UI_EVENT_DELTA_STRIDE_WHOLE 
                            : UI_EVENT_DELTA_STRIDE_LINE;
                        UIEvt.DeltaI32 = Ctrl ? Vec(0, Direction) : Vec(Direction, 0);
                    } break;

                    case INPUT_KIND_PAGEUP:
                    case INPUT_KIND_PAGEDOWN: {
                        UIEvt.Kind = UI_EVENT_KIND_NAVIGATE;
                        UIEvt.Flags = Shift ? UI_EVENT_FLAG_KEEP_MARK : 0;
                        UIEvt.DeltaStride = UI_EVENT_DELTA_STRIDE_PAGE;
                        UIEvt.DeltaI32 = Vec(0, (Event->Input == INPUT_KIND_PAGEUP) ? -1 : 1);
                    } break;

                    case INPUT_KIND_BACKSPACE:
                    case INPUT_KIND_DELETE: {
                        UIEvt.Kind = UI_EVENT_KIND_EDIT;
                        UIEvt.Flags = UI_EVENT_FLAG_DELETE | UI_EVENT_FLAG_ZERO_DELTA_ON_SELECT;
                        UIEvt.DeltaStride = Ctrl 
                            ? UI_EVENT_DELTA_STRIDE_WORD 
                            : UI_EVENT_DELTA_STRIDE_CHAR;
                        UIEvt.DeltaI32 = Vec((Event->Input == INPUT_KIND_BACKSPACE) ? -1 : 1, 0);
                    } break;
                }
            } break;
        }

        if (UIEvt.Kind != UI_EVENT_KIND_NULL)
            ListPush(MemPool, UIEvents, &UIEvt);
    }
}

internal void
TestUIBuild(TestUIState* State)
{
    Str8 Menus[] = { 
        "File"_s8, 
        "Edit"_s8, 
        "View"_s8, 
        "Help"_s8 
    };
    Str8 MenuItems[ARRAY_COUNT(Menus)][3] = {
        {
            "Open"_s8, 
            "Extract"_s8, 
            "Exit"_s8 
        },
        { 
            "Copy"_s8, 
            "Find"_s8, 
            "Go to"_s8 
        },
        { 
            "Files"_s8, 
            "Packets"_s8, 
            "Sources"_s8 
        },
        { 
            "Keys"_s8, 
            "About"_s8, 
            "Version"_s8 
        }
    };
    Str8 ColumnNames[] = { 
        "Row"_s8, 
        "Offset"_s8, 
        "Time"_s8, 
        "Sys"_s8, 
        "Comp"_s8, 
        "Message"_s8 
    };
    Str8 MessageNames[] = {
        "HEARTBEAT"_s8, 
        "ATTITUDE"_s8, 
        "GLOBAL_POSITION_INT"_s8, 
        "SYS_STATUS"_s8, 
        "RADIO_STATUS"_s8, 
        "SYSTEM_TIME"_s8
    };
    v4f32 Blue = RGBAFromU32(0x0000AAFF);
    v4f32 DarkBlue = RGBAFromU32(0x000080FF);
    v4f32 Grey = RGBAFromU32(0xAAAAAAFF);
    v4f32 Black = RGBAFromU32(0x000000FF);
    v4f32 Cyan = RGBAFromU32(0x00AAAAFF);
    v4f32 Red = RGBAFromU32(0xFF5555FF);
    v2f32 Console = Length(GetConsoleRect());
    f32 ListHeight = Console.Y - 4.0f;
    UIKey RowMenuKey = UIKeyFromStr(EMPTY_UI_KEY_VALUE, "###row_menu"_s8);
    i64 HoveredColumn = -1;
    f32* ColumnPercents[ARRAY_COUNT(State->ColumnPercents)] = {};

    for (u64 Index = 0; Index < ARRAY_COUNT(ColumnPercents); ++Index)
        ColumnPercents[Index] = &State->ColumnPercents[Index];

    UIPushPreferredWidth(UI_TEXT_DIM(2.0f, 1.0f));
    UIPushPreferredWidth(UI_PERCENT(1.0f, 0.0f));
    UIPushPreferredHeight(UI_PX(1.0f, 1.0f));
    UISetNextPreferredHeight(UI_PERCENT(1.0f, 1.0f));

    UIColumn() {
        UISetNextFlags(UI_BOX_KIND_DRAW_BACKGROUND);
        UISetNextBackgroundColour(Grey);

        UIRow() {
            UIPushBackgroundColour(Grey);
            UIPushTextColour(Black);
            UIPushPreferredWidth(UI_TEXT_DIM(2.0f, 1.0f));
            UIPushTextAlignment(UI_TEXT_ALIGN_CENTRE);

            for (u64 MenuIndex = 0; MenuIndex < ARRAY_COUNT(Menus); ++MenuIndex) {
                UISetNextFastpathCodepoint(Menus[MenuIndex].Str[0]);
                UISetNextFlags(UI_BOX_KIND_DRAW_TEXT_FASTPATH_CODEPOINT);

                UISignal MenuSig = UIButton(Menus[MenuIndex]);
                UIKey MenuKey = UIKeyFromStr(MenuSig.Box->Key, "###menu"_s8);

                if (UI_PRESSED(MenuSig)) {
                    if (UIContextMenuIsOpen(MenuKey))
                        UIContextMenuClose();
                    else
                        UIContextMenuOpen(MenuKey, MenuSig.Box->Key, Vec(0.0f, 1.0f));
                }

                UIContextMenu(MenuKey) {
                    UIPushTag("floating"_s8);
                    UIPushPreferredWidth(UI_PX(16.0f, 1.0f));
                    UIPushTextAlignment(UI_TEXT_ALIGN_LEFT);
                    UIPushTextPadding(1.0f);

                    for (u64 ItemIndex = 0; ItemIndex < 3; ++ItemIndex) {
                        if (UI_CLICKED(UIButton(MenuItems[MenuIndex][ItemIndex]))) {
                            State->LastAction = MenuItems[MenuIndex][ItemIndex];
                            State->ShouldQuit = (MenuIndex == 0 && ItemIndex == 2);
                            UIContextMenuClose();
                        }
                    }

                    UIPopTextPadding();
                    UIPopTextAlignment();
                    UIPopPreferredWidth();
                    UIPopTag();
                }
            }

            UISpacer(UI_PERCENT(1.0f, 0.0f));
            UILabel("FUME UI SMOKE "_s8);
            UIPopTextAlignment();
            UIPopPreferredWidth();
            UIPopTextColour();
            UIPopBackgroundColour();
        }

        UIPushBackgroundColour(Cyan);
        UIPushTextColour(Black);
        UIPushTextPadding(1.0f);
        UISetNextPreferredWidth(UI_PX(Console.X - 1.0f, 1.0f));

        UITable(ARRAY_COUNT(ColumnPercents), ColumnPercents, "###header"_s8) {
            UITableVector() {
                for (i64 Column = 0; Column < (i64) ARRAY_COUNT(ColumnNames); ++Column) {
                    UITableCell() {
                        UISignal HeaderSig = UISortHeader(
                            State->SortColumn == Column, 
                            State->SortAscending, 
                            ColumnNames[Column]
                        );

                        if (UI_CLICKED(HeaderSig)) {
                            State->SortAscending = (State->SortColumn == Column) 
                                ? !State->SortAscending 
                                : TRUE;
                            State->SortColumn = Column;
                        }

                        if (UI_HOVERING(HeaderSig))
                            HoveredColumn = Column;
                    }
                }
            }
        }

        UIPopTextPadding();
        UIPopTextColour();
        UIPopBackgroundColour();

        if (HoveredColumn >= 0) {
            UITooltip() {
                UILabel("Click to sort by %S", PRINT_STR(ColumnNames[HoveredColumn]));
            }
        }

        UIPushFocusHot(UI_FOCUS_KIND_ON);
        UIPushFocusActive(UI_FOCUS_KIND_ON);
        UISetNextChildLayoutAxis(AXIS_2D_X);

        UIBox* EditRow = UIBuildBoxFromStr(
            UI_BOX_KIND_DEFAULT_FOCUS_EDIT, 
            "###edit_row"_s8
        );

        UIParent(EditRow) {
            UIPushPreferredWidth(UI_TEXT_DIM(2.0f, 1.0f));
            UILabel("Go to row:"_s8);
            UIPopPreferredWidth();
            UISetNextBackgroundColour(DarkBlue);

            UISignal EditSig = UILineEdit(
                &State->EditCursor,
                &State->EditMark,
                State->EditBuffer,
                sizeof(State->EditBuffer),
                &State->EditSize,
                Str(State->EditBuffer, State->EditSize),
                "click here, type a row number, press Enter###goto_edit"_s8
            );

            if (UI_COMMITTED(EditSig)) {
                i64 Row = (i64) U64FromStr(Str(State->EditBuffer, State->EditSize));

                Row = CLAMP(0, Row, TEST_UI_ROW_COUNT - 1);
                State->Cursor.Y = Row + 1;
                State->Mark = State->Cursor;
                State->LastAction = "go to row"_s8;
                UIScrollPointTargetIndex(&State->Scroll, MAX(Row - 3, 0));
            }
        }

        UIPopFocusActive();
        UIPopFocusHot();

        if (ListHeight >= 1.0f) {
            UIScrollListParameters Params = {};
            UIScrollListSignal ListSig = {};
            r1i64 Visible = {};

            Params.Kind = UI_SCROLL_LIST_KIND_ALL;
            Params.DimensionsPX = Vec(Console.X, ListHeight);
            Params.RowHeightPX = 1.0f;
            Params.CursorRange.Max.Y = TEST_UI_ROW_COUNT;
            Params.ItemRange = Rng((i64) 0, (i64) TEST_UI_ROW_COUNT);
            Params.CursorMinIsEmptySelection[AXIS_2D_Y] = TRUE;
            UIPushFocusActive(UI_FOCUS_KIND_ON);

            UIScrollList(&Params, &State->Scroll, &State->Cursor, &State->Mark, &Visible, &ListSig) {
                UITable(ARRAY_COUNT(ColumnPercents), ColumnPercents, "###rows"_s8) {
                    for (
                        i64 Row = Visible.Min; 
                        Row <= Visible.Max && Row < TEST_UI_ROW_COUNT; 
                        ++Row
                    ) {
                        i64 Item = State->SortAscending 
                            ? Row 
                            : (TEST_UI_ROW_COUNT - 1 - Row);
                        b32 IsBad = (Item % 997 == 500);
                        b32 IsCursor = (State->Cursor.Y == Row + 1);
                        Str8 Cells[] = {
                            ArenaPushStrFmt(State->FrameMemPool, "%llu", (u64) Item),
                            ArenaPushStrFmt(State->FrameMemPool, "%llu", (u64) Item * 41),
                            ArenaPushStrFmt(State->FrameMemPool, "%.2f", (f64) Item * 0.01),
                            IsBad ? "255"_s8 : "1"_s8,
                            (Item & 1) ? "1"_s8 : "0"_s8,
                            IsBad 
                                ? "incorrect CRC"_s8 
                                : MessageNames[Item % ARRAY_COUNT(MessageNames)]
                        };

                        UISetNextFlags(
                            UI_BOX_KIND_MOUSE_CLICKABLE | 
                            UI_BOX_KIND_DRAW_BACKGROUND | 
                            UI_BOX_KIND_DRAW_HOT_EFFECTS
                        );
                        UISetNextBackgroundColour(
                            IsCursor ? Cyan : (Row & 1) ? DarkBlue : Blue
                        );
                        UITableVectorBegin();
                        UIPushTextColour(IsCursor ? Black : IsBad ? Red : Grey);
                        UIPushTextPadding(1.0f);

                        for (u64 Column = 0; Column < ARRAY_COUNT(Cells); ++Column) {
                            UIPushTextAlignment(
                                (Column < 5) ? UI_TEXT_ALIGN_RIGHT : UI_TEXT_ALIGN_LEFT
                            );

                            UITableCell() {
                                UILabel(Cells[Column]);
                            }

                            UIPopTextAlignment();
                        }

                        UIPopTextPadding();
                        UIPopTextColour();

                        UISignal RowSig = UITableVectorEnd();

                        if (UI_PRESSED(RowSig) || UI_RIGHT_CLICKED(RowSig)) {
                            State->Cursor.Y = Row + 1;
                            State->Mark = State->Cursor;
                        }

                        if (UI_RIGHT_CLICKED(RowSig)) {
                            UIContextMenuOpen(
                                RowMenuKey, 
                                RowSig.Box->Key, 
                                UIMouse() - RowSig.Box->Rect.Point0
                            );
                        }
                    }
                }
            }

            UIPopFocusActive();
            State->Scroll.Offset = 0.0f;
        }

        UIContextMenu(RowMenuKey) {
            UIPushTag("floating"_s8);
            UIPushPreferredWidth(UI_PX(22.0f, 1.0f));
            UIPushTextPadding(1.0f);

            if (UI_CLICKED(UIButton("Copy row number"_s8))) {
                SetClipboardText(
                    ArenaPushStrFmt(
                        State->FrameMemPool, 
                        "%llu", 
                        (u64) (State->Cursor.Y - 1)
                    )
                );
                State->LastAction = "copy row number"_s8;
                UIContextMenuClose();
            }

            if (UI_CLICKED(UIButton("Go to first row"_s8))) {
                State->Cursor.Y = 1;
                State->Mark = State->Cursor;
                State->LastAction = "go to first row"_s8;
                UIScrollPointTargetIndex(&State->Scroll, 0);
                UIContextMenuClose();
            }

            if (UI_CLICKED(UIButton("Go to last row"_s8))) {
                State->Cursor.Y = TEST_UI_ROW_COUNT;
                State->Mark = State->Cursor;
                State->LastAction = "go to last row"_s8;
                UIScrollPointTargetIndex(&State->Scroll, TEST_UI_ROW_COUNT - 1);
                UIContextMenuClose();
            }

            UIPopTextPadding();
            UIPopPreferredWidth();
            UIPopTag();
        }

        UISetNextFlags(UI_BOX_KIND_DRAW_BACKGROUND);
        UISetNextBackgroundColour(Grey);

        UIRow() {
            UIPushBackgroundColour(Grey);
            UIPushTextColour(Black);
            UIPushPreferredWidth(UI_TEXT_DIM(2.0f, 1.0f));
            UISetNextPreferredWidth(UI_SUM_OF_CHILDREN(1.0f));
            UICheckBox(&State->IsLoading, "Loading"_s8);

            if (State->IsLoading) {
                UIProgressSpinner(1.0f, "###spinner"_s8);
                UISetNextPreferredWidth(UI_PX(20.0f, 1.0f));
                UIProgressBar(
                    (f32) (State->FrameIndex % 200) / 200.0f, 
                    "###progress"_s8
                );
            }

            UILabel(
                "row %llu of %llu   action: %S   frame %llu   %llu B   %.0f us   Ctrl+Q quits###status",
                (u64) State->Cursor.Y,
                (u64) TEST_UI_ROW_COUNT,
                PRINT_STR(State->LastAction),
                State->FrameIndex,
                REND_STATE->PrevFrameBytes,
                State->LastFrameUSecs
            );
            UIPopPreferredWidth();
            UIPopTextColour();
            UIPopBackgroundColour();
        }
    }

    UIPopPreferredHeight();
    UIPopPreferredWidth();
    UIPopPreferredWidth();
}

internal void
TestUILoop(void)
{
    TestUIThemeEntry ThemeTable[] = {
        { 0x0000AAFF, { "background"_s8 } },
        { 0xAAAAAAFF, { "text"_s8 } },
        { 0xAAAAAAFF, { "border"_s8 } },
        { 0xFFFFFFFF, { "hover"_s8 } },
        { 0xFFFF55FF, { "focus"_s8 } },
        { 0x00000080, { "overlay"_s8 } },
        { 0xFFFF55FF, { "fuzzy_match"_s8 } },
        { 0x00AAAAFF, { "accent"_s8 } },
        { 0xFFFFFF66, { "selection"_s8 } },
        { 0xFFFFFFFF, { "cursor"_s8 } },
        { 0xAAAAAAFF, { "floating"_s8, "background"_s8 } },
        { 0x000000FF, { "floating"_s8, "text"_s8 } },
        { 0x000000FF, { "floating"_s8, "hover"_s8 } },
        { 0x000055FF, { "scroll_bar"_s8, "background"_s8 } },
        { 0xFFFFFFFF, { "scroll_bar"_s8, "accent"_s8 } }
    };
    UIThemePattern Patterns[ARRAY_COUNT(ThemeTable)] = {};

    for (u64 Index = 0; Index < ARRAY_COUNT(ThemeTable); ++Index) {
        Patterns[Index].Tags.Data = ThemeTable[Index].Tags;
        Patterns[Index].Tags.Count = ThemeTable[Index].Tags[1].Size ? 2 : 1;
        Patterns[Index].Linear = RGBAFromU32(ThemeTable[Index].Colour);
    }

    UITheme Theme = { Patterns, ARRAY_COUNT(Patterns) };
    UIAnimationInfo AnimationInfo = { 1.0f, 1.0f, 1.0f, 1.0f };
    UIIconInfo Icons = {};

    Icons.IconKindTextMap[UI_ICON_KIND_RIGHT_ARROW] = "\xE2\x96\xB8"_s8;
    Icons.IconKindTextMap[UI_ICON_KIND_DOWN_ARROW] = "\xE2\x96\xBC"_s8;
    Icons.IconKindTextMap[UI_ICON_KIND_LEFT_ARROW] = "\xE2\x97\x82"_s8;
    Icons.IconKindTextMap[UI_ICON_KIND_UP_ARROW] = "\xE2\x96\xB2"_s8;
    Icons.IconKindTextMap[UI_ICON_KIND_RIGHT_CARET] = "\xE2\x96\xB8"_s8;
    Icons.IconKindTextMap[UI_ICON_KIND_DOWN_CARET] = "\xE2\x96\xBC"_s8;
    Icons.IconKindTextMap[UI_ICON_KIND_LEFT_CARET] = "\xE2\x97\x82"_s8;
    Icons.IconKindTextMap[UI_ICON_KIND_UP_CARET] = "\xE2\x96\xB2"_s8;
    Icons.IconKindTextMap[UI_ICON_KIND_CHECK_HOLLOW] = "[ ]"_s8;
    Icons.IconKindTextMap[UI_ICON_KIND_CHECK_FILLED] = "[x]"_s8;
    Icons.IconKindTextMap[UI_ICON_KIND_RADIO_HOLLOW] = "( )"_s8;
    Icons.IconKindTextMap[UI_ICON_KIND_RADIO_FILLED] = "(\xE2\x80\xA2)"_s8;

    TestUIState State = {};

    State.FrameMemPool = ArenaAlloc();
    State.ColumnPercents[0] = 0.12f;
    State.ColumnPercents[1] = 0.16f;
    State.ColumnPercents[2] = 0.16f;
    State.ColumnPercents[3] = 0.08f;
    State.ColumnPercents[4] = 0.08f;
    State.ColumnPercents[5] = 0.40f;
    State.EditCursor = TxtPt(1, 1);
    State.EditMark = TxtPt(1, 1);
    State.SortAscending = TRUE;
    State.LastAction = "none"_s8;

    UIInit();

    Arena* EventsMemPool = ArenaAlloc();
    PerfCounter PrevTime = TimeGetTimestamp();
    b32 Animating = FALSE;

    while (!State.ShouldQuit) {
        ArenaClear(EventsMemPool);
        ArenaClear(State.FrameMemPool);

        InputEventList Events = GetEvents(EventsMemPool, !Animating);
        UIEventList UIEvents = {};

        TestUIPushEvents(EventsMemPool, &State, &Events, &UIEvents);

        if (Animating)
            SleepMSecs(16);

        PerfCounter Now = TimeGetTimestamp();
        f32 DeltaTime = (f32) (TimeElapsedUSec(PrevTime, Now) / 1000000.0);

        PrevTime = Now;
        DeltaTime = MIN(DeltaTime, 0.1f);

        UIBeginBuild(&UIEvents, &Icons, &Theme, &AnimationInfo, DeltaTime, DeltaTime);
        TestUIBuild(&State);
        UIEndBuild();

        DBeginFrame(
            State.FrameMemPool, 
            RGBAFromU32(0xAAAAAAFF), 
            RGBAFromU32(0x0000AAFF)
        );
        UIDrawRoot(UIRootFromState(UIGetSelectedState()));
        DEndFrame();

        Animating = UIIsAnimatingFromState(UIGetSelectedState());
        State.LastFrameUSecs = TimeElapsedUSec(Now, TimeGetTimestamp());
        ++State.FrameIndex;
    }
}

void
EntryPoint(CommandLine* CLI)
{
    TestState State = {};

    State.FrameMemPool = ArenaAlloc();
    State.LogMemPool = ArenaAlloc();

    TerminalInit();
    RendInit(CLI);
    DrawInit();

    if (CommandLineHasFlag(CLI, "ui"_s8)) {
        TestUILoop();
        RendRelease();
        TerminalRelease();

        return;
    }

    Arena* EventsMemPool = ArenaAlloc();

    while (!State.ShouldQuit) {
        ArenaClear(EventsMemPool);

        InputEventList Events = GetEvents(EventsMemPool, !State.ShouldAnimate);

        TestHandleEvents(&State, &Events);

        if (State.ShouldAnimate) {
            ++State.Ticks;
            SleepMSecs(16);
        }

        PerfCounter Start = TimeGetTimestamp();

        TestDrawFrame(&State);
        State.LastFrameUSecs = TimeElapsedUSec(Start, TimeGetTimestamp());
        ++State.FrameIndex;
    }

    u64 LastFrameBytes = REND_STATE->PrevFrameBytes;

    RendRelease();
    TerminalRelease();

    PrintOut(
        "frames %llu   events %llu   last frame %llu bytes\n",
        State.FrameIndex,
        State.EventCount,
        LastFrameBytes
    );
}
