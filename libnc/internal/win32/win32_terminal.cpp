#include "win32_platform.h"
#include "win32_keyboard.h"
#include "nc_string.h"
#include "nc_console.h"
#include "nc_memory.h"
#include "nc_time.h"

#define WIN_TERMINAL_PASTE_TEXT_MIN 8

struct TerminalState {
    Arena*             MemPool;
    HANDLE             In;
    HANDLE             Out;
    HANDLE             WakeupEvent;
    DWORD              LastInMode;
    DWORD              LastOutMode;
    TerminalProperties Properties;
    CursorKind         Cursor;
    r2f32              Rect;
    v2f32              Mouse;
    DWORD              MouseButtons;
    InputModifierKind  Modifiers;
    u32                HighSurrogate;
    b32                CloseRequested;
    b32                InputIsDown[INPUT_KIND_COUNT];
    InputKind          InputKindFromVKey[256];
};

struct WinPaste {
    u32* CodePoints;
    u64  Count;
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
        case EVENT_KIND_NULL:
        case EVENT_KIND_COUNT: {} break;

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
StrFromInputModifierInput(Arena* MemPool, InputModifierKind Modifiers, InputKind Input)
{
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
ConsumeEvent(InputEventList* List, InputEvent* Event)
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
InputRelease(InputEventList* List, InputModifierKind Modifier, InputKind Input)
{
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

InputModifierKind 
ModifiersFromControlKeyState(DWORD State)
{
    InputModifierKind Result = (InputModifierKind) 0;

    if (State & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED))
        Result |= INPUT_MOD_KIND_CTRL;

    if (State & SHIFT_PRESSED)
        Result |= INPUT_MOD_KIND_SHIFT;

    if (State & (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED))
        Result |= INPUT_MOD_KIND_ALT;

    return Result;
}

u32 
CodePointFromKeyRecord(KEY_EVENT_RECORD* Key)
{
    u32 Result = 0;
    u32 Unit = Key->uChar.UnicodeChar;
    b32 CarriesText = (Key->bKeyDown || Key->wVirtualKeyCode == VK_MENU);

    if (CarriesText && Unit >= 0xD800 && Unit <= 0xDBFF) {
        TERMINAL_STATE->HighSurrogate = Unit;
    } else if (CarriesText && Unit >= 0xDC00 && Unit <= 0xDFFF) {
        if (TERMINAL_STATE->HighSurrogate)
            Result = 0x10000 + ((TERMINAL_STATE->HighSurrogate - 0xD800) << 10) + (Unit - 0xDC00);

        TERMINAL_STATE->HighSurrogate = 0;
    } else if (CarriesText && Unit >= 32 && Unit != 127) {
        Result = Unit;
    }

    return Result;
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

            if (Trimmed.Str[Offset] == '"') {
                Stop = '"';
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
FlushPaste(WinPaste* Paste)
{
    if (Paste->Count) {
        u8* Text = ArenaPushArray(TERMINAL_MEMPOOL, u8, Paste->Count * 4);
        u64 TextSize = 0;

        for (u64 Index = 0; Index < Paste->Count; ++Index)
            TextSize += UTF8Encode(Text + TextSize, Paste->CodePoints[Index]);

        Str8List Paths = PathListFromPaste(TERMINAL_MEMPOOL, Str(Text, TextSize));

        if (Paths.Count) {
            InputEvent* Event = PushEvent(EVENT_KIND_FILE_DROP);

            Event->Position = TERMINAL_STATE->Mouse;
            Event->Strings = Paths;
        } else {
            for (u64 Index = 0; Index < Paste->Count; ++Index) {
                InputEvent* Event = PushEvent(EVENT_KIND_TEXT);

                Event->Character = Paste->CodePoints[Index];
            }
        }

        Paste->Count = 0;
    }
}

void 
PushKeyRecord(KEY_EVENT_RECORD* Key, WinPaste* Paste)
{
    InputKind Input = TERMINAL_STATE->InputKindFromVKey[Key->wVirtualKeyCode & 0xFF];
    u32 CodePoint = CodePointFromKeyRecord(Key);
    b32 IsModifier = (Input == INPUT_KIND_CTRL || Input == INPUT_KIND_SHIFT || Input == INPUT_KIND_ALT);
    b32 IsPastedText = (Paste->CodePoints && Key->uChar.UnicodeChar >= 32 && Key->uChar.UnicodeChar != 127);

    TERMINAL_STATE->Modifiers = ModifiersFromControlKeyState(Key->dwControlKeyState);

    if (!IsPastedText && !IsModifier && Key->bKeyDown)
        FlushPaste(Paste);

    if (!IsPastedText && Input != INPUT_KIND_NULL) {
        InputEvent* Event = PushEvent(Key->bKeyDown ? EVENT_KIND_PRESS : EVENT_KIND_RELEASE);

        Event->Input = Input;
        Event->RepeatCount = Key->wRepeatCount;
        Event->IsRepeat = (Key->bKeyDown && TERMINAL_STATE->InputIsDown[Input]);
        Event->RightSided = (
            ((Input == INPUT_KIND_CTRL || Input == INPUT_KIND_ALT) && (Key->dwControlKeyState & ENHANCED_KEY)) ||
            (Input == INPUT_KIND_SHIFT && Key->wVirtualScanCode == 0x36)
        );

        if (Input == INPUT_KIND_ALT)
            Event->Modifier &= ~INPUT_MOD_KIND_ALT;

        if (Input == INPUT_KIND_CTRL)
            Event->Modifier &= ~INPUT_MOD_KIND_CTRL;

        if (Input == INPUT_KIND_SHIFT)
            Event->Modifier &= ~INPUT_MOD_KIND_SHIFT;
    }

    TERMINAL_STATE->InputIsDown[Input] = Key->bKeyDown;

    if (CodePoint && IsPastedText) {
        Paste->CodePoints[Paste->Count++] = CodePoint;
    } else if (CodePoint) {
        InputEvent* Event = PushEvent(EVENT_KIND_TEXT);

        Event->Character = CodePoint;
    }
}

void 
PushMouseRecord(MOUSE_EVENT_RECORD* Mouse)
{
    local readonly struct {
        DWORD     Button;
        InputKind Input;
    } Map[] = {
        {FROM_LEFT_1ST_BUTTON_PRESSED, INPUT_KIND_LEFT_MOUSE_BTN},
        {FROM_LEFT_2ND_BUTTON_PRESSED, INPUT_KIND_MIDDLE_MOUSE_BTN},
        {RIGHTMOST_BUTTON_PRESSED,     INPUT_KIND_RIGHT_MOUSE_BTN}
    };

    v2f32 Position = Vec((f32) Mouse->dwMousePosition.X, (f32) Mouse->dwMousePosition.Y);
    DWORD Buttons = Mouse->dwButtonState & 0xFFFF;
    DWORD Changed = Buttons ^ TERMINAL_STATE->MouseButtons;
    i16 WheelDelta = (i16) (Mouse->dwButtonState >> 16);

    TERMINAL_STATE->Modifiers = ModifiersFromControlKeyState(Mouse->dwControlKeyState);

    if (Position.X != TERMINAL_STATE->Mouse.X || Position.Y != TERMINAL_STATE->Mouse.Y) {
        InputEvent* Event = EVENT_LIST.Tail;

        if (!Event || Event->Kind != EVENT_KIND_MOUSE_MOVE)
            Event = PushEvent(EVENT_KIND_MOUSE_MOVE);

        Event->Position = Position;
        TERMINAL_STATE->Mouse = Position;
    }

    for (u64 Index = 0; Index < ARRAY_COUNT(Map); ++Index) {
        if (Changed & Map[Index].Button) {
            b32 IsDown = !!(Buttons & Map[Index].Button);
            InputEvent* Event = PushEvent(IsDown ? EVENT_KIND_PRESS : EVENT_KIND_RELEASE);

            Event->Input = Map[Index].Input;
            Event->Position = Position;
            TERMINAL_STATE->InputIsDown[Map[Index].Input] = IsDown;
        }
    }

    TERMINAL_STATE->MouseButtons = Buttons;

    if (Mouse->dwEventFlags & (MOUSE_WHEELED | MOUSE_HWHEELED)) {
        InputEvent* Event = PushEvent(EVENT_KIND_SCROLL);
        f32 Notches = (f32) WheelDelta / (f32) WHEEL_DELTA;

        Event->Position = Position;
        Event->PositionDelta = (Mouse->dwEventFlags & MOUSE_WHEELED) ? Vec(0.0f, -Notches) : Vec(Notches, 0.0f);
    }
}

void 
PushFocusRecord(FOCUS_EVENT_RECORD* Focus)
{
    if (!Focus->bSetFocus) {
        PushEvent(EVENT_KIND_CONSOLE_LOSE_FOCUS);
        MemSet(TERMINAL_STATE->InputIsDown, 0, sizeof(TERMINAL_STATE->InputIsDown));
        TERMINAL_STATE->MouseButtons = 0;
        TERMINAL_STATE->Modifiers = (InputModifierKind) 0;
    }
}

BOOL WINAPI 
WinConsoleCtrlFunc(DWORD CtrlKind)
{
    TERMINAL_STATE->CloseRequested = TRUE;
    SetEvent(TERMINAL_STATE->WakeupEvent);

    if (CtrlKind != CTRL_C_EVENT && CtrlKind != CTRL_BREAK_EVENT)
        SleepMSecs(U32_MAX);

    return TRUE;
}

void 
RefreshConsoleRect(void)
{
    CONSOLE_SCREEN_BUFFER_INFO Info = {};

    if (GetConsoleScreenBufferInfo(TERMINAL_STATE->Out, &Info)) {
        TERMINAL_STATE->Rect.X0 = 0.0f;
        TERMINAL_STATE->Rect.Y0 = 0.0f;
        TERMINAL_STATE->Rect.X1 = (f32) (Info.srWindow.Right - Info.srWindow.Left + 1);
        TERMINAL_STATE->Rect.Y1 = (f32) (Info.srWindow.Bottom - Info.srWindow.Top + 1);
    }
}

void 
TerminalInit(void)
{
    Arena* MemPool = ArenaAlloc();

    TERMINAL_STATE = ArenaPushArrayZero(MemPool, TerminalState, 1);
    TERMINAL_STATE->MemPool = MemPool;
    TERMINAL_STATE->In = GetStdHandle(PRINT_STDIN_FILENO);
    TERMINAL_STATE->Out = GetStdHandle(PRINT_STDOUT_FILENO);
    TERMINAL_STATE->WakeupEvent = CreateEventW(NULL, FALSE, FALSE, NULL);
    TERMINAL_STATE->Properties.DoubleClickTime = GetDoubleClickTime() / 1000.0f;
    TERMINAL_STATE->Properties.CaretBlinkTime = GetCaretBlinkTime() / 1000.0f;
    TERMINAL_STATE->Properties.DefaultRefreshRate = 60.0f;
    TERMINAL_STATE->Cursor = CURSOR_KIND_POINTER;

    GetConsoleMode(TERMINAL_STATE->In, &TERMINAL_STATE->LastInMode);
    GetConsoleMode(TERMINAL_STATE->Out, &TERMINAL_STATE->LastOutMode);
    SetConsoleMode(TERMINAL_STATE->In, ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT | ENABLE_EXTENDED_FLAGS);
    SetConsoleMode(TERMINAL_STATE->Out, TERMINAL_STATE->LastOutMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING | DISABLE_NEWLINE_AUTO_RETURN);
    SetConsoleCtrlHandler(WinConsoleCtrlFunc, TRUE);

    Str8 Enter = "\x1b[?1049h\x1b[?25l"_s8;

    WriteFile(TERMINAL_STATE->Out, Enter.Str, (DWORD) Enter.Size, NULL, NULL);
    RefreshConsoleRect();

    InputKind* Table = TERMINAL_STATE->InputKindFromVKey;

    Table[(unsigned int) 'A'] = INPUT_KIND_A;
    Table[(unsigned int) 'B'] = INPUT_KIND_B;
    Table[(unsigned int) 'C'] = INPUT_KIND_C;
    Table[(unsigned int) 'D'] = INPUT_KIND_D;
    Table[(unsigned int) 'E'] = INPUT_KIND_E;
    Table[(unsigned int) 'F'] = INPUT_KIND_F;
    Table[(unsigned int) 'G'] = INPUT_KIND_G;
    Table[(unsigned int) 'H'] = INPUT_KIND_H;
    Table[(unsigned int) 'I'] = INPUT_KIND_I;
    Table[(unsigned int) 'J'] = INPUT_KIND_J;
    Table[(unsigned int) 'K'] = INPUT_KIND_K;
    Table[(unsigned int) 'L'] = INPUT_KIND_L;
    Table[(unsigned int) 'M'] = INPUT_KIND_M;
    Table[(unsigned int) 'N'] = INPUT_KIND_N;
    Table[(unsigned int) 'O'] = INPUT_KIND_O;
    Table[(unsigned int) 'P'] = INPUT_KIND_P;
    Table[(unsigned int) 'Q'] = INPUT_KIND_Q;
    Table[(unsigned int) 'R'] = INPUT_KIND_R;
    Table[(unsigned int) 'S'] = INPUT_KIND_S;
    Table[(unsigned int) 'T'] = INPUT_KIND_T;
    Table[(unsigned int) 'U'] = INPUT_KIND_U;
    Table[(unsigned int) 'V'] = INPUT_KIND_V;
    Table[(unsigned int) 'W'] = INPUT_KIND_W;
    Table[(unsigned int) 'X'] = INPUT_KIND_X;
    Table[(unsigned int) 'Y'] = INPUT_KIND_Y;
    Table[(unsigned int) 'Z'] = INPUT_KIND_Z;

    for (u64 I = '0', J = INPUT_KIND_0; I <= '9'; ++I, ++J)
        Table[I] = (InputKind) J;

    for (u64 I = VK_F1, J = INPUT_KIND_FUNC_1; I <= VK_F24; ++I, ++J)
        Table[I] = (InputKind) J;

    for (u64 I = VK_NUMPAD0, J = INPUT_KIND_NUM_0; I <= VK_NUMPAD9; ++I, ++J)
        Table[I] = (InputKind) J;

    for (u64 I = 0xDF, J = INPUT_KIND_EX0; I < 0xFF; ++I, ++J)
        Table[I] = (InputKind) J;

    Table[VK_SPACE]      = INPUT_KIND_SPACE;
    Table[VK_OEM_3]      = INPUT_KIND_TILDE;
    Table[VK_OEM_MINUS]  = INPUT_KIND_MINUS;
    Table[VK_OEM_PLUS]   = INPUT_KIND_EQUAL;
    Table[VK_OEM_4]      = INPUT_KIND_LEFTBRACKET;
    Table[VK_OEM_6]      = INPUT_KIND_RIGHTBRACKET;
    Table[VK_OEM_1]      = INPUT_KIND_SEMICOLON;
    Table[VK_OEM_7]      = INPUT_KIND_QUOTE;
    Table[VK_OEM_COMMA]  = INPUT_KIND_COMMA;
    Table[VK_OEM_PERIOD] = INPUT_KIND_PERIOD;
    Table[VK_OEM_2]      = INPUT_KIND_SLASH;
    Table[VK_OEM_5]      = INPUT_KIND_BACKSLASH;
    Table[VK_TAB]        = INPUT_KIND_TAB;
    Table[VK_PAUSE]      = INPUT_KIND_PAUSE;
    Table[VK_ESCAPE]     = INPUT_KIND_ESC;
    Table[VK_UP]         = INPUT_KIND_UP;
    Table[VK_LEFT]       = INPUT_KIND_LEFT;
    Table[VK_DOWN]       = INPUT_KIND_DOWN;
    Table[VK_RIGHT]      = INPUT_KIND_RIGHT;
    Table[VK_BACK]       = INPUT_KIND_BACKSPACE;
    Table[VK_RETURN]     = INPUT_KIND_RETURN;
    Table[VK_DELETE]     = INPUT_KIND_DELETE;
    Table[VK_INSERT]     = INPUT_KIND_INSERT;
    Table[VK_PRIOR]      = INPUT_KIND_PAGEUP;
    Table[VK_NEXT]       = INPUT_KIND_PAGEDOWN;
    Table[VK_HOME]       = INPUT_KIND_HOME;
    Table[VK_END]        = INPUT_KIND_END;
    Table[VK_CAPITAL]    = INPUT_KIND_CAPSLOCK;
    Table[VK_NUMLOCK]    = INPUT_KIND_NUMLOCK;
    Table[VK_SCROLL]     = INPUT_KIND_SCROLLLOCK;
    Table[VK_APPS]       = INPUT_KIND_SUPER;
    Table[VK_CONTROL]    = INPUT_KIND_CTRL;
    Table[VK_LCONTROL]   = INPUT_KIND_CTRL;
    Table[VK_RCONTROL]   = INPUT_KIND_CTRL;
    Table[VK_SHIFT]      = INPUT_KIND_SHIFT;
    Table[VK_LSHIFT]     = INPUT_KIND_SHIFT;
    Table[VK_RSHIFT]     = INPUT_KIND_SHIFT;
    Table[VK_MENU]       = INPUT_KIND_ALT;
    Table[VK_LMENU]      = INPUT_KIND_ALT;
    Table[VK_RMENU]      = INPUT_KIND_ALT;
    Table[VK_DIVIDE]     = INPUT_KIND_NUM_SLASH;
    Table[VK_MULTIPLY]   = INPUT_KIND_NUM_STAR;
    Table[VK_SUBTRACT]   = INPUT_KIND_NUM_MINUS;
    Table[VK_ADD]        = INPUT_KIND_NUM_PLUS;
    Table[VK_DECIMAL]    = INPUT_KIND_NUM_PERIOD;
}

void 
TerminalRelease(void)
{
    Str8 Leave = "\x1b[?25h\x1b[?1049l"_s8;

    WriteFile(TERMINAL_STATE->Out, Leave.Str, (DWORD) Leave.Size, NULL, NULL);
    SetConsoleMode(TERMINAL_STATE->Out, TERMINAL_STATE->LastOutMode);
    SetConsoleMode(TERMINAL_STATE->In, TERMINAL_STATE->LastInMode);
    SetConsoleCtrlHandler(WinConsoleCtrlFunc, FALSE);
    CloseHandle(TERMINAL_STATE->WakeupEvent);
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
    SetEvent(TERMINAL_STATE->WakeupEvent);
}

void
TerminalWrite(Str8 Bytes)
{
    for (u64 Offset = 0; Offset < Bytes.Size; ) {
        DWORD Written = 0;

        if (
            !WriteFile(
                TERMINAL_STATE->Out, 
                Bytes.Str + Offset, 
                (DWORD) (Bytes.Size - Offset), 
                &Written, 
                NULL
            )
        ) {
            break;
        }

        Offset += Written;
    }
}

InputEventList 
GetEvents(Arena* MemPool, b32 Wait)
{
    TERMINAL_MEMPOOL = MemPool;
    MemSet(&EVENT_LIST, 0, sizeof(EVENT_LIST));

    HANDLE WaitHandles[2] = {
        TERMINAL_STATE->In,
        TERMINAL_STATE->WakeupEvent
    };

    DWORD WaitResult = WaitForMultipleObjects(ARRAY_COUNT(WaitHandles), WaitHandles, FALSE, Wait ? INFINITE : 0);
    DWORD RecordCount = 0;

    if (WaitResult == WAIT_OBJECT_0 + 1)
        PushEvent(EVENT_KIND_WAKEUP);

    GetNumberOfConsoleInputEvents(TERMINAL_STATE->In, &RecordCount);

    if (RecordCount) {
        INPUT_RECORD* Records = ArenaPushArray(MemPool, INPUT_RECORD, RecordCount);

        ReadConsoleInputW(TERMINAL_STATE->In, Records, RecordCount, &RecordCount);

        u64 DistinctTextCount = 0;
        u32 LastTextUnit = 0;

        for (u64 Index = 0; Index < RecordCount; ++Index) {
            KEY_EVENT_RECORD* Key = &Records[Index].Event.KeyEvent;

            if (
                Records[Index].EventType == KEY_EVENT && 
                Key->bKeyDown && 
                Key->uChar.UnicodeChar >= 32 && 
                (u32) Key->uChar.UnicodeChar != LastTextUnit
            ) {
                LastTextUnit = Key->uChar.UnicodeChar;
                ++DistinctTextCount;
            }
        }

        WinPaste Paste = {};

        if (DistinctTextCount >= WIN_TERMINAL_PASTE_TEXT_MIN)
            Paste.CodePoints = ArenaPushArray(MemPool, u32, RecordCount);

        for (u64 Index = 0; Index < RecordCount; ++Index) {
            switch (Records[Index].EventType) {
                default: {} break;

                case KEY_EVENT: {
                    PushKeyRecord(&Records[Index].Event.KeyEvent, &Paste);
                } break;

                case MOUSE_EVENT: {
                    PushMouseRecord(&Records[Index].Event.MouseEvent);
                } break;

                case FOCUS_EVENT: {
                    PushFocusRecord(&Records[Index].Event.FocusEvent);
                } break;

                case WINDOW_BUFFER_SIZE_EVENT: {
                    PushEvent(EVENT_KIND_WAKEUP);
                } break;
            }
        }

        FlushPaste(&Paste);
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
    if (OpenClipboard(NULL)) {
        EmptyClipboard();

        HANDLE StringCopyHandle = GlobalAlloc(GMEM_MOVEABLE, String.Size + 1);

        if (StringCopyHandle) {
            u8* CopyBuffer = (u8*) GlobalLock(StringCopyHandle);

            MemCpy(CopyBuffer, String.Str, String.Size);
            CopyBuffer[String.Size] = 0;
            GlobalUnlock(StringCopyHandle);
            SetClipboardData(CF_TEXT, StringCopyHandle);
        }

        CloseClipboard();
    }
}

Str8 
GetClipboardText(Arena* MemPool)
{
    Str8 Result = {};

    if (IsClipboardFormatAvailable(CF_TEXT) && OpenClipboard(NULL)) {
        HANDLE DataHandle = GetClipboardData(CF_TEXT);

        if (DataHandle) {
            u8* Buffer = (u8*) GlobalLock(DataHandle);

            if (Buffer) {
                u64 Size = StrLen(Buffer);

                Result = ArenaPushStrCpy(MemPool, Str(Buffer, Size));
                GlobalUnlock(DataHandle);
            }
        }

        CloseClipboard();
    }

    return Result;
}
