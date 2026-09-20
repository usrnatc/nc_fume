#if !defined(__NC_CONSOLE_H__)
#define __NC_CONSOLE_H__

#include "nc_types.h"
#include "nc_system.h"
#include "nc_keyboard.h"

struct Arena;
struct TerminalState;

// @defines____________________________________________________________________

typedef u32 InputModifierKind;
enum : u32 {
    INPUT_MOD_KIND_CTRL  = (1 << 0),
    INPUT_MOD_KIND_SHIFT = (1 << 1),
    INPUT_MOD_KIND_ALT   = (1 << 2)
};

enum CursorKind {
    CURSOR_KIND_POINTER,
    CURSOR_KIND_I_BAR,
    CURSOR_KIND_LEFT_RIGHT,
    CURSOR_KIND_UP_DOWN,
    CURSOR_KIND_DOWN_RIGHT,
    CURSOR_KIND_UP_RIGHT,
    CURSOR_KIND_UP_DOWN_LEFT_RIGHT,
    CURSOR_KIND_HAND_POINT,
    CURSOR_KIND_DISABLED,
    CURSOR_KIND_COUNT
};

enum InputEventKind {
    EVENT_KIND_NULL,
    EVENT_KIND_PRESS,
    EVENT_KIND_RELEASE,
    EVENT_KIND_MOUSE_MOVE,
    EVENT_KIND_TEXT,
    EVENT_KIND_SCROLL,
    EVENT_KIND_CONSOLE_LOSE_FOCUS,
    EVENT_KIND_CONSOLE_CLOSE,
    EVENT_KIND_FILE_DROP,
    EVENT_KIND_WAKEUP,
    EVENT_KIND_COUNT
};

EXTERN_C_LINK_BEGIN
extern Str8 KEYBOARD_CONFIG_STR_TABLE[143];
EXTERN_C_LINK_END

// @types______________________________________________________________________
struct TerminalProperties {
    f32 DoubleClickTime;
    f32 CaretBlinkTime;
    f32 DefaultRefreshRate;
};

struct InputEvent {
    InputEvent*       Next;
    InputEvent*       Prev;
    u64               TimeStampUSecs;
    InputEventKind    Kind;
    InputModifierKind Modifier;
    InputKind         Input;
    b32               IsRepeat;
    b32               RightSided;
    u32               Character;
    u32               RepeatCount;
    v2f32             Position;
    v2f32             PositionDelta;
    Str8List          Strings;
};

struct InputEventList {
    u64         Count;
    InputEvent* Head;
    InputEvent* Tail;
};

// @runtime____________________________________________________________________
extern TerminalState* TERMINAL_STATE;
extern InputEventList EVENT_LIST;
extern Arena* TERMINAL_MEMPOOL;

// @functions__________________________________________________________________
EXTERN_C_LINK_BEGIN
u32 CodePointFromInput(InputModifierKind Mods, InputKind Input);
EXTERN_C_LINK_END

Str8 StrFromEventKind(InputEventKind Kind);
Str8 StrFromInputModifierInput(Arena* MemPool, InputModifierKind Modifiers, InputKind Input);
Str8List StrListFromInputModKind(Arena* MemPool, InputModifierKind Modifiers);
void ConsumeEvent(InputEventList* List, InputEvent* Event);
b32 InputPress(InputEventList* List, InputModifierKind Modifier, InputKind Input);
b32 InputRelease(InputEventList* List, InputModifierKind Modifier, InputKind Input);
b32 Text(InputEventList* List, u32 Character);
InputEventList EventListCopy(Arena* MemPool, InputEventList* List);
void EventListCat(InputEventList* Dst, InputEventList* Src);
InputEvent* ListPush(Arena* MemPool, InputEventList* List, InputEventKind Event);
void TerminalInit(void);
TerminalProperties* GetTerminalProperties(void);
r2f32 GetConsoleRect(void);
void SendWakeupEvent(void);
InputEventList GetEvents(Arena* MemPool, b32 Wait);
InputModifierKind GetModifiers(void);
b32 KeyIsDown(InputKind Input);
v2f32 GetMousePosition(void);
void SetCursorKind(CursorKind Cursor);
Str8 GetClipboardText(Arena* MemPool);
void SetClipboardText(Str8 String);

#endif // __NC_CONSOLE_H__
