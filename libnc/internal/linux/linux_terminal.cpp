#include "linux_platform.h"
#include "nc_string.h"
#include "nc_file.h"
#include "nc_console.h"
#include "nc_memory.h"
#include "nc_time.h"

const int LINUX_TERMINAL_SIGNALS[4] = {
    SIGINT,
    SIGTERM,
    SIGHUP,
    SIGWINCH
};

struct TerminalState {
    Arena*             MemPool;
    Arena*             PasteMemPool;
    Arena*             ClipboardMemPool;
    int                In;
    int                Out;
    int                WakeupFD;
    struct termios     LastMode;
    struct sigaction   LastSignalActions[ARRAY_COUNT(LINUX_TERMINAL_SIGNALS)];
    TerminalProperties Properties;
    CursorKind         Cursor;
    r2f32              Rect;
    v2f32              Mouse;
    InputModifierKind  Modifiers;
    Str8List           PasteChunks;
    Str8               Clipboard;
    u8                 Pending[32];
    u64                PendingCount;
    b32                IsPasting;
    b32                CloseRequested;
    b32                InputIsDown[INPUT_KIND_COUNT];
    b32                ByteIsShifted[128];
    InputKind          InputKindFromByte[128];
};

EXTERN_C_LINK_BEGIN

Str8 KEYBOARD_CONFIG_STR_TABLE[143] = {
    Str8Lit("null"),
    Str8Lit("esc"),
    Str8Lit("f1"),
    Str8Lit("f2"),
    Str8Lit("f3"),
    Str8Lit("f4"),
    Str8Lit("f5"),
    Str8Lit("f6"),
    Str8Lit("f7"),
    Str8Lit("f8"),
    Str8Lit("f9"),
    Str8Lit("f10"),
    Str8Lit("f11"),
    Str8Lit("f12"),
    Str8Lit("f13"),
    Str8Lit("f14"),
    Str8Lit("f15"),
    Str8Lit("f16"),
    Str8Lit("f17"),
    Str8Lit("f18"),
    Str8Lit("f19"),
    Str8Lit("f20"),
    Str8Lit("f21"),
    Str8Lit("f22"),
    Str8Lit("f23"),
    Str8Lit("f24"),
    Str8Lit("tick"),
    Str8Lit("0"),
    Str8Lit("1"),
    Str8Lit("2"),
    Str8Lit("3"),
    Str8Lit("4"),
    Str8Lit("5"),
    Str8Lit("6"),
    Str8Lit("7"),
    Str8Lit("8"),
    Str8Lit("9"),
    Str8Lit("minus"),
    Str8Lit("equal"),
    Str8Lit("backspace"),
    Str8Lit("tab"),
    Str8Lit("q"),
    Str8Lit("w"),
    Str8Lit("e"),
    Str8Lit("r"),
    Str8Lit("t"),
    Str8Lit("y"),
    Str8Lit("u"),
    Str8Lit("i"),
    Str8Lit("o"),
    Str8Lit("p"),
    Str8Lit("left_bracket"),
    Str8Lit("right_bracket"),
    Str8Lit("backslash"),
    Str8Lit("caps_lock"),
    Str8Lit("a"),
    Str8Lit("s"),
    Str8Lit("d"),
    Str8Lit("f"),
    Str8Lit("g"),
    Str8Lit("h"),
    Str8Lit("j"),
    Str8Lit("k"),
    Str8Lit("l"),
    Str8Lit("semicolon"),
    Str8Lit("quote"),
    Str8Lit("return"),
    Str8Lit("shift"),
    Str8Lit("z"),
    Str8Lit("x"),
    Str8Lit("c"),
    Str8Lit("v"),
    Str8Lit("b"),
    Str8Lit("n"),
    Str8Lit("m"),
    Str8Lit("comma"),
    Str8Lit("period"),
    Str8Lit("slash"),
    Str8Lit("ctrl"),
    Str8Lit("alt"),
    Str8Lit("space"),
    Str8Lit("menu"),
    Str8Lit("scroll_lock"),
    Str8Lit("pause"),
    Str8Lit("insert"),
    Str8Lit("home"),
    Str8Lit("page_up"),
    Str8Lit("delete"),
    Str8Lit("end"),
    Str8Lit("page_down"),
    Str8Lit("up"),
    Str8Lit("left"),
    Str8Lit("down"),
    Str8Lit("right"),
    Str8Lit("ex0"),
    Str8Lit("ex1"),
    Str8Lit("ex2"),
    Str8Lit("ex3"),
    Str8Lit("ex4"),
    Str8Lit("ex5"),
    Str8Lit("ex6"),
    Str8Lit("ex7"),
    Str8Lit("ex8"),
    Str8Lit("ex9"),
    Str8Lit("ex10"),
    Str8Lit("ex11"),
    Str8Lit("ex12"),
    Str8Lit("ex13"),
    Str8Lit("ex14"),
    Str8Lit("ex15"),
    Str8Lit("ex16"),
    Str8Lit("ex17"),
    Str8Lit("ex18"),
    Str8Lit("ex19"),
    Str8Lit("ex20"),
    Str8Lit("ex21"),
    Str8Lit("ex22"),
    Str8Lit("ex23"),
    Str8Lit("ex24"),
    Str8Lit("ex25"),
    Str8Lit("ex26"),
    Str8Lit("ex27"),
    Str8Lit("ex28"),
    Str8Lit("ex29"),
    Str8Lit("num_lock"),
    Str8Lit("numpad_slash"),
    Str8Lit("numpad_star"),
    Str8Lit("numpad_minus"),
    Str8Lit("numpad_plus"),
    Str8Lit("numpad_period"),
    Str8Lit("numpad_0"),
    Str8Lit("numpad_1"),
    Str8Lit("numpad_2"),
    Str8Lit("numpad_3"),
    Str8Lit("numpad_4"),
    Str8Lit("numpad_5"),
    Str8Lit("numpad_6"),
    Str8Lit("numpad_7"),
    Str8Lit("numpad_8"),
    Str8Lit("numpad_9"),
    Str8Lit("left_mouse"),
    Str8Lit("middle_mouse"),
    Str8Lit("right_mouse")
};

u32
CodePointFromInput(InputModifierKind Mods, InputKind Input)
{
    u32 Result = 0;

    local readonly struct {
        u32 Character;
        InputKind Input;
        InputModifierKind Mod;
    } Map[] = {
        {
            '!',
            INPUT_KIND_1,
            INPUT_MOD_KIND_SHIFT
        },
        {
            '@',
            INPUT_KIND_2,
            INPUT_MOD_KIND_SHIFT
        },
        {
            '#',
            INPUT_KIND_3,
            INPUT_MOD_KIND_SHIFT
        },
        {
            '$',
            INPUT_KIND_4,
            INPUT_MOD_KIND_SHIFT
        },
        {
            '%',
            INPUT_KIND_5,
            INPUT_MOD_KIND_SHIFT
        },
        {
            '^',
            INPUT_KIND_6,
            INPUT_MOD_KIND_SHIFT
        },
        {
            '&',
            INPUT_KIND_7,
            INPUT_MOD_KIND_SHIFT
        },
        {
            '*',
            INPUT_KIND_8,
            INPUT_MOD_KIND_SHIFT
        },
        {
            '(',
            INPUT_KIND_9,
            INPUT_MOD_KIND_SHIFT
        },
        {
            ')',
            INPUT_KIND_0,
            INPUT_MOD_KIND_SHIFT
        },
        {
            '_',
            INPUT_KIND_MINUS,
            INPUT_MOD_KIND_SHIFT
        },
        {
            '_',
            INPUT_KIND_MINUS,
            INPUT_MOD_KIND_SHIFT
        },
        {
            '-',
            INPUT_KIND_MINUS,
            0
        },
        {
            '=',
            INPUT_KIND_EQUAL,
            0
        },
        {
            '+',
            INPUT_KIND_EQUAL,
            INPUT_MOD_KIND_SHIFT
        },
        {
            '`',
            INPUT_KIND_TILDE,
            0
        },
        {
            '~',
            INPUT_KIND_TILDE,
            INPUT_MOD_KIND_SHIFT
        },
        {
            '[',
            INPUT_KIND_LEFTBRACKET,
            0
        },
        {
            ']',
            INPUT_KIND_RIGHTBRACKET,
            0
        },
        {
            '{',
            INPUT_KIND_LEFTBRACKET,
            INPUT_MOD_KIND_SHIFT
        },
        {
            '}',
            INPUT_KIND_RIGHTBRACKET,
            INPUT_MOD_KIND_SHIFT
        },
        {
            '\\',
            INPUT_KIND_BACKSLASH,
            0
        },
        {
            '|',
            INPUT_KIND_BACKSLASH,
            INPUT_MOD_KIND_SHIFT
        },
        {
            ';',
            INPUT_KIND_SEMICOLON,
            0
        },
        {
            ':',
            INPUT_KIND_SEMICOLON,
            INPUT_MOD_KIND_SHIFT
        },
        {
            '\'',
            INPUT_KIND_QUOTE,
            0
        },
        {
            '"',
            INPUT_KIND_QUOTE,
            INPUT_MOD_KIND_SHIFT
        },
        {
            '.',
            INPUT_KIND_PERIOD,
            0
        },
        {
            ',',
            INPUT_KIND_COMMA,
            0
        },
        {
            '<',
            INPUT_KIND_PERIOD,
            INPUT_MOD_KIND_SHIFT
        },
        {
            '>',
            INPUT_KIND_COMMA,
            INPUT_MOD_KIND_SHIFT
        },
        {
            '/',
            INPUT_KIND_SLASH,
            0
        },
        {
            '?',
            INPUT_KIND_SLASH,
            INPUT_MOD_KIND_SHIFT
        },
        {
            'a',
            INPUT_KIND_A,
            0
        },
        {
            'b',
            INPUT_KIND_B,
            0
        },
        {
            'c',
            INPUT_KIND_C,
            0
        },
        {
            'd',
            INPUT_KIND_D,
            0
        },
        {
            'e',
            INPUT_KIND_E,
            0
        },
        {
            'f',
            INPUT_KIND_F,
            0
        },
        {
            'g',
            INPUT_KIND_G,
            0
        },
        {
            'h',
            INPUT_KIND_H,
            0
        },
        {
            'i',
            INPUT_KIND_I,
            0
        },
        {
            'j',
            INPUT_KIND_J,
            0
        },
        {
            'k',
            INPUT_KIND_K,
            0
        },
        {
            'l',
            INPUT_KIND_L,
            0
        },
        {
            'm',
            INPUT_KIND_M,
            0
        },
        {
            'n',
            INPUT_KIND_N,
            0
        },
        {
            'o',
            INPUT_KIND_O,
            0
        },
        {
            'p',
            INPUT_KIND_P,
            0
        },
        {
            'q',
            INPUT_KIND_Q,
            0
        },
        {
            'r',
            INPUT_KIND_R,
            0
        },
        {
            's',
            INPUT_KIND_S,
            0
        },
        {
            't',
            INPUT_KIND_T,
            0
        },
        {
            'u',
            INPUT_KIND_U,
            0
        },
        {
            'v',
            INPUT_KIND_V,
            0
        },
        {
            'w',
            INPUT_KIND_W,
            0
        },
        {
            'x',
            INPUT_KIND_X,
            0
        },
        {
            'y',
            INPUT_KIND_Y,
            0
        },
        {
            'z',
            INPUT_KIND_Z,
            0
        },
        {
            'A',
            INPUT_KIND_A,
            INPUT_MOD_KIND_SHIFT
        },
        {
            'B',
            INPUT_KIND_B,
            INPUT_MOD_KIND_SHIFT
        },
        {
            'C',
            INPUT_KIND_C,
            INPUT_MOD_KIND_SHIFT
        },
        {
            'D',
            INPUT_KIND_D,
            INPUT_MOD_KIND_SHIFT
        },
        {
            'E',
            INPUT_KIND_E,
            INPUT_MOD_KIND_SHIFT
        },
        {
            'F',
            INPUT_KIND_F,
            INPUT_MOD_KIND_SHIFT
        },
        {
            'G',
            INPUT_KIND_G,
            INPUT_MOD_KIND_SHIFT
        },
        {
            'H',
            INPUT_KIND_H,
            INPUT_MOD_KIND_SHIFT
        },
        {
            'I',
            INPUT_KIND_I,
            INPUT_MOD_KIND_SHIFT
        },
        {
            'J',
            INPUT_KIND_J,
            INPUT_MOD_KIND_SHIFT
        },
        {
            'K',
            INPUT_KIND_K,
            INPUT_MOD_KIND_SHIFT
        },
        {
            'L',
            INPUT_KIND_L,
            INPUT_MOD_KIND_SHIFT
        },
        {
            'M',
            INPUT_KIND_M,
            INPUT_MOD_KIND_SHIFT
        },
        {
            'N',
            INPUT_KIND_N,
            INPUT_MOD_KIND_SHIFT
        },
        {
            'O',
            INPUT_KIND_O,
            INPUT_MOD_KIND_SHIFT
        },
        {
            'P',
            INPUT_KIND_P,
            INPUT_MOD_KIND_SHIFT
        },
        {
            'Q',
            INPUT_KIND_Q,
            INPUT_MOD_KIND_SHIFT
        },
        {
            'R',
            INPUT_KIND_R,
            INPUT_MOD_KIND_SHIFT
        },
        {
            'S',
            INPUT_KIND_S,
            INPUT_MOD_KIND_SHIFT
        },
        {
            'T',
            INPUT_KIND_T,
            INPUT_MOD_KIND_SHIFT
        },
        {
            'U',
            INPUT_KIND_U,
            INPUT_MOD_KIND_SHIFT
        },
        {
            'V',
            INPUT_KIND_V,
            INPUT_MOD_KIND_SHIFT
        },
        {
            'W',
            INPUT_KIND_W,
            INPUT_MOD_KIND_SHIFT
        },
        {
            'X',
            INPUT_KIND_X,
            INPUT_MOD_KIND_SHIFT
        },
        {
            'Y',
            INPUT_KIND_Y,
            INPUT_MOD_KIND_SHIFT
        },
        {
            'Z',
            INPUT_KIND_Z,
            INPUT_MOD_KIND_SHIFT
        },
        {
            ' ',
            INPUT_KIND_SPACE,
            0
        }
    };

    if (Input >= INPUT_KIND_0 && Input <= INPUT_KIND_9)
        Result = '0' + (Input - INPUT_KIND_0);

    for (u64 Index = 0; Index < ARRAY_COUNT(Map); ++Index) {
        if (Map[Index].Input == Input && Map[Index].Mod == Mods) {
            Result = Map[Index].Character;
            break;
        }
    }

    return Result;
}

EXTERN_C_LINK_END

TerminalState* TERMINAL_STATE = NULL;
InputEventList EVENT_LIST = {};
Arena* TERMINAL_MEMPOOL = NULL;

Str8
StrFromEventKind(InputEventKind Kind)
{
    Str8 Result = {};

    switch (Kind) {
        default: {} break;

        case EVENT_KIND_PRESS: {
            Result = Str8Lit("Press");
        } break;

        case EVENT_KIND_RELEASE: {
            Result = Str8Lit("Release");
        } break;

        case EVENT_KIND_MOUSE_MOVE: {
            Result = Str8Lit("MouseMove");
        } break;

        case EVENT_KIND_TEXT: {
            Result = Str8Lit("Text");
        } break;

        case EVENT_KIND_SCROLL: {
            Result = Str8Lit("Scroll");
        } break;

        case EVENT_KIND_CONSOLE_LOSE_FOCUS: {
            Result = Str8Lit("ConsoleLoseFocus");
        } break;

        case EVENT_KIND_CONSOLE_CLOSE: {
            Result = Str8Lit("ConsoleClose");
        } break;

        case EVENT_KIND_FILE_DROP: {
            Result = Str8Lit("FileDrop");
        } break;

        case EVENT_KIND_WAKEUP: {
            Result = Str8Lit("Wakeup");
        } break;
    }

    return Result;
}

Str8
StrFromInputModifierInput(
    Arena* MemPool, 
    InputModifierKind Modifiers, 
    InputKind Input
) {
    Str8 Result = {};

    if (Input != INPUT_KIND_NULL) {
        TempArena Scratch = GetScratch(&MemPool, 1);
        Str8List Mods = StrListFromInputModKind(Scratch.MemPool, Modifiers);
        Str8 KeyString = INPUT_DISPLAY_STR_TABLE[Input];

        ListPush(Scratch.MemPool, &Mods, KeyString);

        Str8JoinPart Join = {};

        Join.Sep = Str8Lit(" + ");
        Result = StrListJoin(MemPool, &Mods, &Join);
        ReleaseScratch(Scratch);
    }

    return Result;
}

Str8List
StrListFromInputModKind(Arena* MemPool, InputModifierKind Modifiers)
{
    Str8List Result = {};
    Str8 ModifierStrings[3] = {
        Str8Lit("Ctrl"),
        Str8Lit("Shift"),
        Str8Lit("Alt")
    };

    StrListFromKinds(
        MemPool,
        &Result,
        Modifiers,
        ModifierStrings,
        3
    );

    return Result;
}

void
ConsumeEvent(InputEventList *List, InputEvent *Event)
{
    DLL_REMOVE(List->Head, List->Tail, Event);
    Event->Next = NULL;
    Event->Prev = NULL;
    --List->Count;
}

b32
InputPress(InputEventList* List, InputModifierKind Modifier, InputKind Input)
{
    b32 Result = FALSE;

    for (InputEvent* Event = List->Head; Event; Event = Event->Next) {
        if (
            Event->Kind == EVENT_KIND_PRESS &&
            Event->Input == Input &&
            Event->Modifier == Modifier
        ) {
            Result = TRUE;
            ConsumeEvent(List, Event);
            break;
        }
    }

    return Result;
}

b32
InputRelease(
    InputEventList* List, 
    InputModifierKind Modifier, 
    InputKind Input
) {
    b32 Result = FALSE;

    for (InputEvent* Event = List->Head; Event; Event = Event->Next) {
        if (
            Event->Kind == EVENT_KIND_RELEASE &&
            Event->Input == Input &&
            Event->Modifier == Modifier
        ) {
            Result = TRUE;
            ConsumeEvent(List, Event);
            break;
        }
    }

    return Result;
}

b32
Text(InputEventList* List, u32 Character)
{
    b32 Result = FALSE;

    for (InputEvent* Event = List->Head; Event; Event = Event->Next) {
        if (
            Event->Kind == EVENT_KIND_TEXT &&
            Event->Character == Character
        ) {
            Result = TRUE;
            ConsumeEvent(List, Event);
            break;
        }
    }

    return Result;
}

InputEventList
EventListCopy(Arena* MemPool, InputEventList* List)
{
    InputEventList Result = {};

    for (InputEvent* Event = List->Head; Event; Event = Event->Next) {
        InputEvent* NewEvent = ArenaPushArrayZero(MemPool, InputEvent, 1);

        MemCpy(NewEvent, Event, sizeof(*Event));
        NewEvent->Strings = StrListCopy(MemPool, &Event->Strings);
        DLL_PUSH_BACK(Result.Head, Result.Tail, NewEvent);
        ++Result.Count;
    }

    return Result;
}

void
EventListCat(InputEventList* Dst, InputEventList* Src)
{
    if (Dst->Tail && Src->Head) {
        Dst->Tail->Next = Src->Head;
        Src->Head->Prev = Dst->Tail;
        Dst->Tail = Src->Tail;
        Dst->Count += Src->Count;
    } else if (!Dst->Tail && Src->Head) {
        MemCpy(Dst, Src, sizeof(*Src));
    }

    MemSet(Src, 0, sizeof(*Src));
}

InputEvent*
ListPush(Arena* MemPool, InputEventList* List, InputEventKind Event)
{
    InputEvent* NewEvent = ArenaPushArrayZero(MemPool, InputEvent, 1);

    DLL_PUSH_BACK(List->Head, List->Tail, NewEvent);
    NewEvent->TimeStampUSecs = (u64) TimeGetTimestamp();
    NewEvent->Kind = Event;
    ++List->Count;

    return NewEvent;
}

InputEvent*
PushEvent(InputEventKind Kind)
{
    InputEvent* Result = ListPush(TERMINAL_MEMPOOL, &EVENT_LIST, Kind);

    Result->Modifier = TERMINAL_STATE->Modifiers;

    return Result;
}

void
PushKey(InputKind Input, InputModifierKind Modifiers, u32 CodePoint)
{
    TERMINAL_STATE->Modifiers = Modifiers;

    if (Input != INPUT_KIND_NULL) {
        InputEvent* Event = PushEvent(EVENT_KIND_PRESS);

        Event->Input = Input;
        Event->RepeatCount = 1;
    }

    if (CodePoint) {
        InputEvent* Event = PushEvent(EVENT_KIND_TEXT);

        Event->Character = CodePoint;
    }

    if (Input != INPUT_KIND_NULL) {
        InputEvent* Event = PushEvent(EVENT_KIND_RELEASE);

        Event->Input = Input;
    }

    TERMINAL_STATE->Modifiers = (InputModifierKind) 0;
}

Str8List
PathListFromPaste(Arena* MemPool, Str8 Paste)
{
    Str8List Result = {};
    Str8 Trimmed = StrSkipChopWhitespace(Paste);

    if (SystemFileExists(Trimmed) || SystemDirExists(Trimmed)) {
        ListPush(MemPool, &Result, Trimmed);
    } else {
        b32 AllExist = (Trimmed.Size != 0);

        for (u64 Offset = 0; AllExist && Offset < Trimmed.Size;) {
            u8 Stop = ' ';

            if (Trimmed.Str[Offset] == '"' || Trimmed.Str[Offset] == '\'') {
                Stop = Trimmed.Str[Offset];
                ++Offset;
            }

            u64 End = Offset;

            while (End < Trimmed.Size && Trimmed.Str[End] != Stop)
                ++End;

            Str8 Path = Str(Trimmed.Str + Offset, End - Offset);

            AllExist = (SystemFileExists(Path) || SystemDirExists(Path));
            ListPush(MemPool, &Result, Path);
            Offset = End + 1;

            while (Offset < Trimmed.Size && Trimmed.Str[Offset] == ' ')
                ++Offset;
        }

        if (!AllExist)
            MemSet(&Result, 0, sizeof(Result));
    }

    return Result;
}

void
FlushPaste(void)
{
    Str8JoinPart Join = {};
    Str8 Text = StrListJoin(
        TERMINAL_MEMPOOL, 
        &TERMINAL_STATE->PasteChunks, 
        &Join
    );
    Str8List Paths = PathListFromPaste(TERMINAL_MEMPOOL, Text);

    TERMINAL_STATE->Modifiers = (InputModifierKind) 0;

    if (Paths.Count) {
        InputEvent* Event = PushEvent(EVENT_KIND_FILE_DROP);

        Event->Position = TERMINAL_STATE->Mouse;
        Event->Strings = Paths;
    } else {
        for (
            u64 Offset = 0; 
            Offset < Text.Size;
        ) {
            UnicodeDecode Decode = UTF8Decode(
                Text.Str + Offset, 
                Text.Size - Offset
            );
            b32 AfterReturn = (Offset && Text.Str[Offset - 1] == '\r');

            if (
                Decode.CodePoint == '\r' || 
                (
                    Decode.CodePoint == '\n' && 
                    !AfterReturn
                )
            ) {
                PushKey(INPUT_KIND_RETURN, 0, 0);
            } else if (Decode.CodePoint == '\t') {
                PushKey(INPUT_KIND_TAB, 0, 0);
            } else if (
                Decode.CodePoint >= 32 && 
                Decode.CodePoint != 127 && 
                Decode.CodePoint != U32_MAX
            ) {
                InputEvent* Event = PushEvent(EVENT_KIND_TEXT);

                Event->Character = Decode.CodePoint;
            }

            Offset += Decode.Inc ? Decode.Inc : 1;
        }
    }

    ArenaClear(TERMINAL_STATE->PasteMemPool);
    MemSet(
        &TERMINAL_STATE->PasteChunks, 
        0, 
        sizeof(TERMINAL_STATE->PasteChunks)
    );
    TERMINAL_STATE->IsPasting = FALSE;
}

void
PushMouseSequence(u32 Code, u32 Column, u32 Row, b32 IsDown)
{
    local readonly InputKind Map[] = {
        INPUT_KIND_LEFT_MOUSE_BTN,
        INPUT_KIND_MIDDLE_MOUSE_BTN,
        INPUT_KIND_RIGHT_MOUSE_BTN
    };

    v2f32 Position = Vec((f32) Column - 1.0f, (f32) Row - 1.0f);
    u32 Button = Code & 3;

    TERMINAL_STATE->Modifiers = (InputModifierKind) 0;

    if (Code & 4)
        TERMINAL_STATE->Modifiers |= INPUT_MOD_KIND_SHIFT;

    if (Code & 8)
        TERMINAL_STATE->Modifiers |= INPUT_MOD_KIND_ALT;

    if (Code & 16)
        TERMINAL_STATE->Modifiers |= INPUT_MOD_KIND_CTRL;

    if (
        Position.X != TERMINAL_STATE->Mouse.X || 
        Position.Y != TERMINAL_STATE->Mouse.Y
    ) {
        InputEvent* Event = EVENT_LIST.Tail;

        if (!Event || Event->Kind != EVENT_KIND_MOUSE_MOVE)
            Event = PushEvent(EVENT_KIND_MOUSE_MOVE);

        Event->Position = Position;
        TERMINAL_STATE->Mouse = Position;
    }

    if (Code & 64) {
        InputEvent* Event = PushEvent(EVENT_KIND_SCROLL);
        f32 Notches = (Button & 1) ? 1.0f : -1.0f;

        Event->Position = Position;
        Event->PositionDelta = (Button & 2) ? Vec(Notches, 0.0f) : Vec(0.0f, Notches);
    } else if (!(Code & 32) && Button < ARRAY_COUNT(Map)) {
        InputEvent* Event = PushEvent(IsDown ? EVENT_KIND_PRESS : EVENT_KIND_RELEASE);

        Event->Input = Map[Button];
        Event->Position = Position;
        TERMINAL_STATE->InputIsDown[Map[Button]] = IsDown;
    }
}

u64
PushInputBytes(u8* Bytes, u64 Count)
{
    local readonly struct {
        u8        Final;
        u32       Param;
        InputKind Input;
    } Map[] = {
        {'A', 0,  INPUT_KIND_UP},
        {'B', 0,  INPUT_KIND_DOWN},
        {'C', 0,  INPUT_KIND_RIGHT},
        {'D', 0,  INPUT_KIND_LEFT},
        {'H', 0,  INPUT_KIND_HOME},
        {'F', 0,  INPUT_KIND_END},
        {'P', 0,  INPUT_KIND_FUNC_1},
        {'Q', 0,  INPUT_KIND_FUNC_2},
        {'R', 0,  INPUT_KIND_FUNC_3},
        {'S', 0,  INPUT_KIND_FUNC_4},
        {'Z', 0,  INPUT_KIND_TAB},
        {'~', 1,  INPUT_KIND_HOME},
        {'~', 2,  INPUT_KIND_INSERT},
        {'~', 3,  INPUT_KIND_DELETE},
        {'~', 4,  INPUT_KIND_END},
        {'~', 5,  INPUT_KIND_PAGEUP},
        {'~', 6,  INPUT_KIND_PAGEDOWN},
        {'~', 7,  INPUT_KIND_HOME},
        {'~', 8,  INPUT_KIND_END},
        {'~', 11, INPUT_KIND_FUNC_1},
        {'~', 12, INPUT_KIND_FUNC_2},
        {'~', 13, INPUT_KIND_FUNC_3},
        {'~', 14, INPUT_KIND_FUNC_4},
        {'~', 15, INPUT_KIND_FUNC_5},
        {'~', 17, INPUT_KIND_FUNC_6},
        {'~', 18, INPUT_KIND_FUNC_7},
        {'~', 19, INPUT_KIND_FUNC_8},
        {'~', 20, INPUT_KIND_FUNC_9},
        {'~', 21, INPUT_KIND_FUNC_10},
        {'~', 23, INPUT_KIND_FUNC_11},
        {'~', 24, INPUT_KIND_FUNC_12}
    };

    Str8 PasteEnd = "\x1b[201~"_s8;
    InputKind* Table = TERMINAL_STATE->InputKindFromByte;
    InputModifierKind Alt = (InputModifierKind) 0;
    u64 Offset = 0;

    for (u64 Used = 1; Used && Offset < Count; Offset += Used) {
        u8* At = Bytes + Offset;
        u64 Left = Count - Offset;
        InputModifierKind Modifiers = Alt;

        Alt = (InputModifierKind) 0;
        Used = 0;

        if (TERMINAL_STATE->IsPasting) {
            u64 TextSize = 0;
            u64 MarkSize = MIN(Left, PasteEnd.Size);

            while (TextSize < Left && At[TextSize] != 0x1B)
                ++TextSize;

            if (TextSize) {
                Str8 Chunk = ArenaPushStrCpy(
                    TERMINAL_STATE->PasteMemPool, 
                    Str(At, TextSize)
                );

                ListPush(
                    TERMINAL_STATE->PasteMemPool, 
                    &TERMINAL_STATE->PasteChunks, 
                    Chunk
                );
                Used = TextSize;
            } else if (
                !StrMatch(
                    Str(At, MarkSize), 
                    Str(PasteEnd.Str, MarkSize), 
                    0
                )
            ) {
                Used = 1;
            } else if (MarkSize == PasteEnd.Size) {
                FlushPaste();
                Used = MarkSize;
            }
        } else if (
            At[0] == 0x1B && 
            Left >= 3 && 
            (
                At[1] == '[' || 
                At[1] == 'O'
            )
        ) {
            u32 Params[4] = {};
            u64 ParamIndex = 0;
            u64 End = 2;
            b32 IsMouse = (At[1] == '[' && At[2] == '<');

            while (End < Left && At[End] >= 0x20 && At[End] <= 0x3F) {
                if (At[End] == ';')
                    ++ParamIndex;
                else if (IsDigit(At[End]) && ParamIndex < ARRAY_COUNT(Params))
                    Params[ParamIndex] = Params[ParamIndex] * 10 + (At[End] - '0');

                ++End;
            }

            if (End < Left) {
                u8 Final = At[End];

                Used = End + 1;

                if (IsMouse && (Final == 'M' || Final == 'm')) {
                    PushMouseSequence(
                        Params[0], 
                        Params[1], 
                        Params[2], 
                        Final == 'M'
                    );
                } else if (Final == '~' && Params[0] == 200) {
                    TERMINAL_STATE->IsPasting = TRUE;
                } else if (Final == 'O' && At[1] == '[') {
                    PushEvent(EVENT_KIND_CONSOLE_LOSE_FOCUS);
                    MemSet(
                        TERMINAL_STATE->InputIsDown, 
                        0, 
                        sizeof(TERMINAL_STATE->InputIsDown)
                    );
                    TERMINAL_STATE->Modifiers = (InputModifierKind) 0;
                } else {
                    InputKind Input = INPUT_KIND_NULL;
                    u32 ModifierCode = Params[1] ? Params[1] - 1 : 0;

                    if ((ModifierCode & 1) || Final == 'Z')
                        Modifiers |= INPUT_MOD_KIND_SHIFT;

                    if (ModifierCode & 2)
                        Modifiers |= INPUT_MOD_KIND_ALT;

                    if (ModifierCode & 4)
                        Modifiers |= INPUT_MOD_KIND_CTRL;

                    for (u64 Index = 0; Index < ARRAY_COUNT(Map); ++Index) {
                        if (
                            Map[Index].Final == Final &&
                            (
                                Final != '~' || 
                                Map[Index].Param == Params[0]
                            )
                        ) {
                            Input = Map[Index].Input;
                            break;
                        }
                    }

                    PushKey(Input, Modifiers, 0);
                }
            }
        } else if (At[0] == 0x1B && Left >= 2) {
            Alt = INPUT_MOD_KIND_ALT;
            Used = 1;
        } else if (At[0] == 0x1B) {
            PushKey(INPUT_KIND_ESC, Modifiers, 0);
            Used = 1;
        } else if (At[0] < 32 || At[0] == 127) {
            if (At[0] != '\t' && At[0] != '\n' && At[0] != '\r' && At[0] != 127)
                Modifiers |= INPUT_MOD_KIND_CTRL;

            PushKey(Table[At[0]], Modifiers, 0);
            Used = 1;
        } else if (At[0] < 128) {
            if (TERMINAL_STATE->ByteIsShifted[At[0]])
                Modifiers |= INPUT_MOD_KIND_SHIFT;

            PushKey(Table[At[0]], Modifiers, At[0]);
            Used = 1;
        } else {
            u64 Size = UTF8_CLASS[At[0] >> 3];

            if (Size <= Left || Size > 4) {
                UnicodeDecode Decode = UTF8Decode(At, Left);

                if (Decode.CodePoint >= 128 && Decode.CodePoint != U32_MAX)
                    PushKey(INPUT_KIND_NULL, Modifiers, Decode.CodePoint);

                Used = Decode.Inc ? Decode.Inc : 1;
            }
        }
    }

    return Offset;
}

void
LinuxTerminalSignalFunc(int Signal)
{
    u64 WakeupCount = 1;

    if (Signal != SIGWINCH)
        TERMINAL_STATE->CloseRequested = TRUE;

    write(TERMINAL_STATE->WakeupFD, &WakeupCount, sizeof(WakeupCount));
}

void
RefreshConsoleRect(void)
{
    struct winsize Size = {};

    if (!ioctl(TERMINAL_STATE->Out, TIOCGWINSZ, &Size) && Size.ws_col && Size.ws_row) {
        TERMINAL_STATE->Rect.X0 = 0.0f;
        TERMINAL_STATE->Rect.Y0 = 0.0f;
        TERMINAL_STATE->Rect.X1 = (f32) Size.ws_col;
        TERMINAL_STATE->Rect.Y1 = (f32) Size.ws_row;
    }
}

void
TerminalInit(void)
{
    Arena* MemPool = ArenaAlloc();

    TERMINAL_STATE = ArenaPushArrayZero(MemPool, TerminalState, 1);
    TERMINAL_STATE->MemPool = MemPool;
    TERMINAL_STATE->PasteMemPool = ArenaAlloc();
    TERMINAL_STATE->ClipboardMemPool = ArenaAlloc();
    TERMINAL_STATE->In = (int) PRINT_STDIN_FILENO;
    TERMINAL_STATE->Out = (int) PRINT_STDOUT_FILENO;
    TERMINAL_STATE->WakeupFD = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    TERMINAL_STATE->Properties.DoubleClickTime = 0.5f;
    TERMINAL_STATE->Properties.CaretBlinkTime = 0.5f;
    TERMINAL_STATE->Properties.DefaultRefreshRate = 60.0f;
    TERMINAL_STATE->Cursor = CURSOR_KIND_POINTER;

    tcgetattr(TERMINAL_STATE->In, &TERMINAL_STATE->LastMode);

    struct termios Mode = TERMINAL_STATE->LastMode;

    cfmakeraw(&Mode);
    Mode.c_cc[VMIN] = 0;
    Mode.c_cc[VTIME] = 0;
    tcsetattr(TERMINAL_STATE->In, TCSAFLUSH, &Mode);

    struct sigaction Handler = {};

    Handler.sa_handler = LinuxTerminalSignalFunc;
    Handler.sa_flags = SA_RESTART;
    sigfillset(&Handler.sa_mask);

    for (u64 Index = 0; Index < ARRAY_COUNT(LINUX_TERMINAL_SIGNALS); ++Index)
        sigaction(LINUX_TERMINAL_SIGNALS[Index], &Handler, &TERMINAL_STATE->LastSignalActions[Index]);

    Str8 Enter = "\x1b[?1049h\x1b[?25l\x1b[?1000h\x1b[?1002h\x1b[?1003h\x1b[?1006h\x1b[?1004h\x1b[?2004h"_s8;

    write(TERMINAL_STATE->Out, Enter.Str, Enter.Size);
    RefreshConsoleRect();

    InputKind* Table = TERMINAL_STATE->InputKindFromByte;

    Table[(unsigned int) 'a'] = INPUT_KIND_A;
    Table[(unsigned int) 'b'] = INPUT_KIND_B;
    Table[(unsigned int) 'c'] = INPUT_KIND_C;
    Table[(unsigned int) 'd'] = INPUT_KIND_D;
    Table[(unsigned int) 'e'] = INPUT_KIND_E;
    Table[(unsigned int) 'f'] = INPUT_KIND_F;
    Table[(unsigned int) 'g'] = INPUT_KIND_G;
    Table[(unsigned int) 'h'] = INPUT_KIND_H;
    Table[(unsigned int) 'i'] = INPUT_KIND_I;
    Table[(unsigned int) 'j'] = INPUT_KIND_J;
    Table[(unsigned int) 'k'] = INPUT_KIND_K;
    Table[(unsigned int) 'l'] = INPUT_KIND_L;
    Table[(unsigned int) 'm'] = INPUT_KIND_M;
    Table[(unsigned int) 'n'] = INPUT_KIND_N;
    Table[(unsigned int) 'o'] = INPUT_KIND_O;
    Table[(unsigned int) 'p'] = INPUT_KIND_P;
    Table[(unsigned int) 'q'] = INPUT_KIND_Q;
    Table[(unsigned int) 'r'] = INPUT_KIND_R;
    Table[(unsigned int) 's'] = INPUT_KIND_S;
    Table[(unsigned int) 't'] = INPUT_KIND_T;
    Table[(unsigned int) 'u'] = INPUT_KIND_U;
    Table[(unsigned int) 'v'] = INPUT_KIND_V;
    Table[(unsigned int) 'w'] = INPUT_KIND_W;
    Table[(unsigned int) 'x'] = INPUT_KIND_X;
    Table[(unsigned int) 'y'] = INPUT_KIND_Y;
    Table[(unsigned int) 'z'] = INPUT_KIND_Z;

    for (u64 I = 'a'; I <= 'z'; ++I) {
        Table[I - 'a' + 'A'] = Table[I];
        Table[I - 'a' + 1] = Table[I];
        TERMINAL_STATE->ByteIsShifted[I - 'a' + 'A'] = TRUE;
    }

    local readonly struct {
        u8        Plain;
        u8        Shifted;
        InputKind Input;
    } Symbols[] = {
        {'`',  '~', INPUT_KIND_TILDE},
        {'1',  '!', INPUT_KIND_1},
        {'2',  '@', INPUT_KIND_2},
        {'3',  '#', INPUT_KIND_3},
        {'4',  '$', INPUT_KIND_4},
        {'5',  '%', INPUT_KIND_5},
        {'6',  '^', INPUT_KIND_6},
        {'7',  '&', INPUT_KIND_7},
        {'8',  '*', INPUT_KIND_8},
        {'9',  '(', INPUT_KIND_9},
        {'0',  ')', INPUT_KIND_0},
        {'-',  '_', INPUT_KIND_MINUS},
        {'=',  '+', INPUT_KIND_EQUAL},
        {'[',  '{', INPUT_KIND_LEFTBRACKET},
        {']',  '}', INPUT_KIND_RIGHTBRACKET},
        {'\\', '|', INPUT_KIND_BACKSLASH},
        {';',  ':', INPUT_KIND_SEMICOLON},
        {'\'', '"', INPUT_KIND_QUOTE},
        {',',  '<', INPUT_KIND_COMMA},
        {'.',  '>', INPUT_KIND_PERIOD},
        {'/',  '?', INPUT_KIND_SLASH}
    };

    for (u64 Index = 0; Index < ARRAY_COUNT(Symbols); ++Index) {
        Table[Symbols[Index].Plain] = Symbols[Index].Input;
        Table[Symbols[Index].Shifted] = Symbols[Index].Input;
        TERMINAL_STATE->ByteIsShifted[Symbols[Index].Shifted] = TRUE;
    }

    Table[(unsigned int) ' '] = INPUT_KIND_SPACE;
    Table[0x00]               = INPUT_KIND_SPACE;
    Table[0x08]               = INPUT_KIND_BACKSPACE;
    Table[0x09]               = INPUT_KIND_TAB;
    Table[0x0A]               = INPUT_KIND_RETURN;
    Table[0x0D]               = INPUT_KIND_RETURN;
    Table[0x7F]               = INPUT_KIND_BACKSPACE;
}

void
TerminalRelease(void)
{
    Str8 Leave = "\x1b[?2004l\x1b[?1004l\x1b[?1006l\x1b[?1003l\x1b[?1002l\x1b[?1000l\x1b[?25h\x1b[?1049l"_s8;

    write(TERMINAL_STATE->Out, Leave.Str, Leave.Size);
    tcsetattr(TERMINAL_STATE->In, TCSAFLUSH, &TERMINAL_STATE->LastMode);

    for (u64 Index = 0; Index < ARRAY_COUNT(LINUX_TERMINAL_SIGNALS); ++Index)
        sigaction(LINUX_TERMINAL_SIGNALS[Index], &TERMINAL_STATE->LastSignalActions[Index], NULL);

    close(TERMINAL_STATE->WakeupFD);
    ArenaRelease(TERMINAL_STATE->ClipboardMemPool);
    ArenaRelease(TERMINAL_STATE->PasteMemPool);
    ArenaRelease(TERMINAL_STATE->MemPool);
    TERMINAL_STATE = NULL;
}

TerminalProperties*
GetTerminalProperties(void)
{
    return &TERMINAL_STATE->Properties;
}

r2f32
GetConsoleRect(void)
{
    return TERMINAL_STATE->Rect;
}

void
SendWakeupEvent(void)
{
    u64 WakeupCount = 1;

    write(TERMINAL_STATE->WakeupFD, &WakeupCount, sizeof(WakeupCount));
}

void
TerminalWrite(Str8 Bytes)
{
    for (u64 Offset = 0; Offset < Bytes.Size; ) {
        i64 Written = write(
            TERMINAL_STATE->Out,
            Bytes.Str + Offset,
            Bytes.Size - Offset
        );

        if (Written < 0 && errno == EINTR)
            continue;

        if (Written <= 0)
            break;

        Offset += Written;
    }
}

InputEventList
GetEvents(Arena* MemPool, b32 Wait)
{
    TERMINAL_MEMPOOL = MemPool;
    MemSet(&EVENT_LIST, 0, sizeof(EVENT_LIST));

    struct pollfd WaitFDs[2] = {
        {TERMINAL_STATE->In, POLLIN, 0},
        {TERMINAL_STATE->WakeupFD, POLLIN, 0}
    };

    int WaitResult = poll(WaitFDs, ARRAY_COUNT(WaitFDs), Wait ? -1 : 0);
    int ByteCount = 0;

    if (WaitResult > 0 && (WaitFDs[1].revents & POLLIN)) {
        u64 WakeupCount = 0;

        read(TERMINAL_STATE->WakeupFD, &WakeupCount, sizeof(WakeupCount));
        PushEvent(EVENT_KIND_WAKEUP);
    }

    if (WaitResult > 0 && (WaitFDs[0].revents & (POLLHUP | POLLERR)))
        TERMINAL_STATE->CloseRequested = TRUE;

    ioctl(TERMINAL_STATE->In, FIONREAD, &ByteCount);

    if (ByteCount > 0) {
        u64 PendingCount = TERMINAL_STATE->PendingCount;
        u8* Bytes = ArenaPushArray(MemPool, u8, PendingCount + ByteCount);

        MemCpy(Bytes, TERMINAL_STATE->Pending, PendingCount);

        i64 ReadCount = read(TERMINAL_STATE->In, Bytes + PendingCount, ByteCount);
        u64 Count = PendingCount + ((ReadCount > 0) ? ReadCount : 0);
        u64 Used = PushInputBytes(Bytes, Count);
        u64 LeftCount = Count - Used;

        TERMINAL_STATE->PendingCount = (LeftCount <= sizeof(TERMINAL_STATE->Pending)) ? LeftCount : 0;
        MemCpy(TERMINAL_STATE->Pending, Bytes + Used, TERMINAL_STATE->PendingCount);
    }

    if (TERMINAL_STATE->CloseRequested) {
        TERMINAL_STATE->CloseRequested = FALSE;
        PushEvent(EVENT_KIND_CONSOLE_CLOSE);
    }

    RefreshConsoleRect();

    return EVENT_LIST;
}

InputModifierKind
GetModifiers(void)
{
    return TERMINAL_STATE->Modifiers;
}

b32
KeyIsDown(InputKind Input)
{
    return TERMINAL_STATE->InputIsDown[Input];
}

v2f32
GetMousePosition(void)
{
    return TERMINAL_STATE->Mouse;
}

void
SetCursorKind(CursorKind Cursor)
{
    TERMINAL_STATE->Cursor = Cursor;
}

CursorKind
GetCursorKind(void)
{
    return TERMINAL_STATE->Cursor;
}

void
SetClipboardText(Str8 String)
{
    local readonly u8 Alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    ArenaClear(TERMINAL_STATE->ClipboardMemPool);
    TERMINAL_STATE->Clipboard = ArenaPushStrCpy(TERMINAL_STATE->ClipboardMemPool, String);

    TempArena Scratch = GetScratch(NULL, 0);
    Str8 Pre = "\x1b]52;c;"_s8;
    u8* Sequence = ArenaPushArray(Scratch.MemPool, u8, Pre.Size + ((String.Size + 2) / 3) * 4 + 1);
    u64 SequenceSize = Pre.Size;

    MemCpy(Sequence, Pre.Str, Pre.Size);

    for (u64 Offset = 0; Offset < String.Size; Offset += 3) {
        u64 Left = String.Size - Offset;
        u32 Bits = ((u32) String.Str[Offset] << 16);

        if (Left > 1)
            Bits |= ((u32) String.Str[Offset + 1] << 8);

        if (Left > 2)
            Bits |= (u32) String.Str[Offset + 2];

        Sequence[SequenceSize++] = Alphabet[(Bits >> 18) & 63];
        Sequence[SequenceSize++] = Alphabet[(Bits >> 12) & 63];
        Sequence[SequenceSize++] = (Left > 1) ? Alphabet[(Bits >> 6) & 63] : '=';
        Sequence[SequenceSize++] = (Left > 2) ? Alphabet[Bits & 63] : '=';
    }

    Sequence[SequenceSize++] = 0x07;
    TerminalWrite(Str(Sequence, SequenceSize));
    ReleaseScratch(Scratch);
}

Str8
GetClipboardText(Arena* MemPool)
{
    return ArenaPushStrCpy(MemPool, TERMINAL_STATE->Clipboard);
}
