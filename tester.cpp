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

    // menu bar
    DrawRect(Rng(0.0f, 0.0f, W, 1.0f), Grey, 0.0f);
    TestText(Vec(2.0f, 0.0f), Black, 0, "File  Edit  View  Help"_s8);
    TestText(Vec(2.0f, 0.0f), Red, FANCY_STR_FLAG_UNDERLINE, "F"_s8);
    TestText(Vec(W - 16.0f, 0.0f), Black, 0, "FUME TUI SMOKE"_s8);

    // single border, title on the top edge
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

    // double border with shadow, and the alpha overlay across its corner
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

    // truecolour ramp, then the same ramp darkened by half
    for (i32 X = 0; X < Size.X; ++X) {
        f32 U = (f32) X / (f32) MAX(Size.X - 1, 1);
        v4f32 C = Vec(U, 1.0f - U, 0.5f + 0.5f * SinF32(U * 6.2831f), 1.0f);

        RendFillRect(Rng(X, 17, X + 1, 18), C);
        RendFillRect(Rng(X, 18, X + 1, 19), C);
    }

    RendDarkenRect(Rng(0, 18, Size.X, 19), 0.5f);

    // clip test: text that starts inside and runs out of the clip
    DClipScope(Rng(2.0f, 20.0f, 30.0f, 21.0f)) {
        DrawRect(Rng(0.0f, 20.0f, W, 21.0f), Black, 0.0f);
        TestText(Vec(2.0f, 20.0f), Green, 0, "clipped at column 30 ->|<- this text must not be visible"_s8);
    }

    // truncation with a trailer, and a fuzzy range
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

    // line join test
    DrawLine(Vec(46.0f, 14.0f), Vec(78.0f, 14.0f), Grey);
    DrawLine(Vec(62.0f, 13.0f), Vec(62.0f, 16.0f), Grey);
    DrawLine(Vec(46.0f, 16.0f), Vec(78.0f, 16.0f), Grey);

    // event log
    f32 LogY = 24.0f;

    TestText(Vec(2.0f, LogY), Yellow, FANCY_STR_FLAG_BOLD, "EVENTS"_s8);

    for (u64 Index = 0; Index < TEST_LOG_ROWS; ++Index) {
        u64 LineIndex = (State->LogHead + Index) % TEST_LOG_ROWS;

        if (State->Log[LineIndex].Size)
            TestText(Vec(2.0f, LogY + 1.0f + (f32) Index), White, 0, State->Log[LineIndex]);
    }

    TestText(Vec(2.0f, LogY + 14.0f), Cyan, 0, ArenaPushStrFmt(State->FrameMemPool, "last paste: %S", PRINT_STR(StrPrefix(State->PrevPaste, 200))));
    TestText(Vec(2.0f, LogY + 15.0f), Cyan, 0, ArenaPushStrFmt(State->FrameMemPool, "last drop:  %S", PRINT_STR(StrPrefix(State->PrevDrop, 200))));

    // status row
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

    // caret in the box, pointer under the mouse, pointer last
    DrawCaret(Vec(4.0f, 15.0f));
    TestText(Vec(5.0f, 15.0f), Grey, FANCY_STR_FLAG_DIMMED, "<- caret"_s8);
    DrawMousePointer(State->Mouse, GetCursorKind());

    DEndFrame();
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
