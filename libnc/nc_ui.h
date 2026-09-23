#if !defined(__NC_UI_H__)
#define __NC_UI_H__

#include "nc_types.h"
#include "nc_string.h"
#include "nc_keyboard.h"
#include "nc_console.h"
#include "nc_draw.h"

struct UIBox;

// @defines____________________________________________________________________
enum UIIconKind {
    UI_ICON_KIND_NULL,
    UI_ICON_KIND_RIGHT_ARROW,
    UI_ICON_KIND_DOWN_ARROW,
    UI_ICON_KIND_LEFT_ARROW,
    UI_ICON_KIND_UP_ARROW,
    UI_ICON_KIND_RIGHT_CARET,
    UI_ICON_KIND_DOWN_CARET,
    UI_ICON_KIND_LEFT_CARET,
    UI_ICON_KIND_UP_CARET,
    UI_ICON_KIND_CHECK_HOLLOW,
    UI_ICON_KIND_CHECK_FILLED,
    UI_ICON_KIND_RADIO_HOLLOW,
    UI_ICON_KIND_RADIO_FILLED,
    UI_ICON_KIND_COUNT
};

enum UIMouseButtonKind {
    UI_MOUSE_BTN_KIND_LEFT,
    UI_MOUSE_BTN_KIND_MIDDLE,
    UI_MOUSE_BTN_KIND_RIGHT,
    UI_MOUSE_BTN_KIND_COUNT,
};

typedef u32 UIPermissionKind;
enum : u32 {
    UI_PERMISSION_KIND_CLICKS_LEFT      = (1 << 0),
    UI_PERMISSION_KIND_CLICKS_MIDDLE    = (1 << 1),
    UI_PERMISSION_KIND_CLICKS_RIGHT     = (1 << 2),
    UI_PERMISSION_KIND_SCROLL_X         = (1 << 3),
    UI_PERMISSION_KIND_SCROLL_Y         = (1 << 4),
    UI_PERMISSION_KIND_KEYBRD_PRIMARY   = (1 << 5),
    UI_PERMISSION_KIND_KEYBRD_SECONDARY = (1 << 6),
    UI_PERMISSION_KIND_TEXT             = (1 << 7),
    UI_PERMISSION_KIND_KEYBRD           = (UI_PERMISSION_KIND_KEYBRD_PRIMARY | UI_PERMISSION_KIND_KEYBRD_SECONDARY),
    UI_PERMISSION_KIND_CLICKS           = (UI_PERMISSION_KIND_CLICKS_LEFT | UI_PERMISSION_KIND_CLICKS_MIDDLE | UI_PERMISSION_KIND_CLICKS_RIGHT),
    UI_PERMISSION_KIND_ALL              = U32_MAX
};

enum UIFocusKind {
    UI_FOCUS_KIND_NULL,
    UI_FOCUS_KIND_OFF,
    UI_FOCUS_KIND_ON,
    UI_FOCUS_KIND_ROOT,
    UI_FOCUS_KIND_COUNT
};

enum UIEventKind {
    UI_EVENT_KIND_NULL,
    UI_EVENT_KIND_PRESS,
    UI_EVENT_KIND_RELEASE,
    UI_EVENT_KIND_TEXT,
    UI_EVENT_KIND_NAVIGATE,
    UI_EVENT_KIND_EDIT,
    UI_EVENT_KIND_MOUSE_MOVE,
    UI_EVENT_KIND_SCROLL,
    UI_EVENT_KIND_FILE_DROP,
    UI_EVENT_KIND_COUNT
};

enum UIEventActionSlot {
    UI_EVENT_ACTION_SLOT_NULL,
    UI_EVENT_ACTION_SLOT_ACCEPT,
    UI_EVENT_ACTION_SLOT_CANCEL,
    UI_EVENT_ACTION_SLOT_EDIT,
    UI_EVENT_ACTION_SLOT_COUNT
};

typedef u32 UIEventFlag;
enum : u32 {
    UI_EVENT_FLAG_KEEP_MARK             = (1 << 0),
    UI_EVENT_FLAG_DELETE                = (1 << 1),
    UI_EVENT_FLAG_COPY                  = (1 << 2),
    UI_EVENT_FLAG_PASTE                 = (1 << 3),
    UI_EVENT_FLAG_ZERO_DELTA_ON_SELECT  = (1 << 4),
    UI_EVENT_FLAG_PICK_SELECT_SIDE      = (1 << 5),
    UI_EVENT_FLAG_CAP_AT_LINE           = (1 << 6),
    UI_EVENT_FLAG_EXPLICIT_DIRECTIONAL  = (1 << 7),
    UI_EVENT_FLAG_REORDER               = (1 << 8),
    UI_EVENT_FLAG_SECONDARY             = (1 << 9)
};

enum UIEventDeltaStride {
    UI_EVENT_DELTA_STRIDE_NULL,
    UI_EVENT_DELTA_STRIDE_CHAR,
    UI_EVENT_DELTA_STRIDE_WORD,
    UI_EVENT_DELTA_STRIDE_LINE,
    UI_EVENT_DELTA_STRIDE_PAGE,
    UI_EVENT_DELTA_STRIDE_WHOLE,
    UI_EVENT_DELTA_STRIDE_COUNT
};

typedef u32 UITextOpKind;
enum : u32 {
    UI_TEXT_OP_KIND_INVALID = (1 << 0),
    UI_TEXT_OP_KIND_COPY    = (1 << 1)
};

enum UISizeKind {
    UI_SIZE_KIND_NULL,
    UI_SIZE_KIND_PIXELS,
    UI_SIZE_KIND_TEXT_CONTENT,
    UI_SIZE_KIND_PERCENT_OF_PARENT,
    UI_SIZE_KIND_SUM_OF_CHILDREN
};

enum UITextAlign {
    UI_TEXT_ALIGN_LEFT,
    UI_TEXT_ALIGN_CENTRE,
    UI_TEXT_ALIGN_RIGHT,
    UI_TEXT_ALIGN_COUNT
};

#define UI_BOX_CUSTOM_DRAW(X) void X(UIBox* Box, void* UserData)
typedef UI_BOX_CUSTOM_DRAW(UIBoxCustomDrawFuncType);

typedef u64 UIBoxKind;
enum : u64 {
    UI_BOX_KIND_MOUSE_CLICKABLE              = (1ULL <<  0),
    UI_BOX_KIND_KEYBOARD_CLICKABLE           = (1ULL <<  1),
    UI_BOX_KIND_DROP_SITE                    = (1ULL <<  2),
    UI_BOX_KIND_CLICK_TO_FOCUS               = (1ULL <<  3),
    UI_BOX_KIND_SCROLL                       = (1ULL <<  4),
    UI_BOX_KIND_VIEW_SCROLL_X                = (1ULL <<  5),
    UI_BOX_KIND_VIEW_SCROLL_Y                = (1ULL <<  6),
    UI_BOX_KIND_VIEW_CLAMP_X                 = (1ULL <<  7),
    UI_BOX_KIND_VIEW_CLAMP_Y                 = (1ULL <<  8),
    UI_BOX_KIND_FOCUS_HOT                    = (1ULL <<  9),
    UI_BOX_KIND_FOCUS_ACTIVE                 = (1ULL << 10),
    UI_BOX_KIND_FOCUS_HOT_DISABLED           = (1ULL << 11),
    UI_BOX_KIND_FOCUS_ACTIVE_DISABLED        = (1ULL << 12),
    UI_BOX_KIND_DEFAULT_FOCUS_NAV_X          = (1ULL << 13),
    UI_BOX_KIND_DEFAULT_FOCUS_NAV_Y          = (1ULL << 14),
    UI_BOX_KIND_DEFAULT_FOCUS_EDIT           = (1ULL << 15),
    UI_BOX_KIND_FOCUS_NAV_SKIP               = (1ULL << 16),
    UI_BOX_KIND_DISABLE_TRUNCATED_HOVER      = (1ULL << 17),
    UI_BOX_KIND_DISABLED                     = (1ULL << 18),
    UI_BOX_KIND_FLOATING_X                   = (1ULL << 19),
    UI_BOX_KIND_FLOATING_Y                   = (1ULL << 20),
    UI_BOX_KIND_FIXED_WIDTH                  = (1ULL << 21),
    UI_BOX_KIND_FIXED_HEIGHT                 = (1ULL << 22),
    UI_BOX_KIND_ALLOW_OVERFLOW_X             = (1ULL << 23),
    UI_BOX_KIND_ALLOW_OVERFLOW_Y             = (1ULL << 24),
    UI_BOX_KIND_SKIP_VIEW_OFF_X              = (1ULL << 25),
    UI_BOX_KIND_SKIP_VIEW_OFF_Y              = (1ULL << 26),
    UI_BOX_KIND_DRAW_DROP_SHADOW             = (1ULL << 27),
    UI_BOX_KIND_DRAW_BACKGROUND              = (1ULL << 29),
    UI_BOX_KIND_DRAW_BORDER                  = (1ULL << 30),
    UI_BOX_KIND_DRAW_SIDE_TOP                = (1ULL << 31),
    UI_BOX_KIND_DRAW_SIDE_BOTTOM             = (1ULL << 32),
    UI_BOX_KIND_DRAW_SIDE_LEFT               = (1ULL << 33),
    UI_BOX_KIND_DRAW_SIDE_RIGHT              = (1ULL << 34),
    UI_BOX_KIND_DRAW_TEXT                    = (1ULL << 35),
    UI_BOX_KIND_DRAW_TEXT_FASTPATH_CODEPOINT = (1ULL << 36),
    UI_BOX_KIND_DRAW_TEXT_WEAK               = (1ULL << 37),
    UI_BOX_KIND_DRAW_HOT_EFFECTS             = (1ULL << 38),
    UI_BOX_KIND_DRAW_ACTIVE_EFFECTS          = (1ULL << 39),
    UI_BOX_KIND_DRAW_OVERLAY                 = (1ULL << 40),
    UI_BOX_KIND_DRAW_FADE_TOP                = (1ULL << 41),
    UI_BOX_KIND_DRAW_FADE_BOTTOM             = (1ULL << 42),
    UI_BOX_KIND_DRAW_FADE_LEFT               = (1ULL << 43),
    UI_BOX_KIND_DRAW_FADE_RIGHT              = (1ULL << 44),
    UI_BOX_KIND_CLIP                         = (1ULL << 45),
    UI_BOX_KIND_ANIMATE_POS_X                = (1ULL << 46),
    UI_BOX_KIND_ANIMATE_POS_Y                = (1ULL << 47),
    UI_BOX_KIND_DISABLE_TEXT_TRUNC           = (1ULL << 48),
    UI_BOX_KIND_DISABLE_ID_STRING            = (1ULL << 49),
    UI_BOX_KIND_DISABLE_FOCUS_BORDER         = (1ULL << 50),
    UI_BOX_KIND_DISABLE_FOCUS_OVERLAY        = (1ULL << 51),
    UI_BOX_KIND_HAS_DISPLAY_STRING           = (1ULL << 52),
    UI_BOX_KIND_HAS_FUZZYMATCH_RANGES        = (1ULL << 53),
    UI_BOX_KIND_DEBUG                        = (1ULL << 56),
    UI_BOX_KIND_CLICKABLE                    = (UI_BOX_KIND_MOUSE_CLICKABLE | UI_BOX_KIND_KEYBOARD_CLICKABLE),
    UI_BOX_KIND_DEFAULT_FOCUS_NAV            = (UI_BOX_KIND_DEFAULT_FOCUS_NAV_X | UI_BOX_KIND_DEFAULT_FOCUS_NAV_Y | UI_BOX_KIND_DEFAULT_FOCUS_EDIT),
    UI_BOX_KIND_FLOATING                     = (UI_BOX_KIND_FLOATING_X | UI_BOX_KIND_FLOATING_Y),
    UI_BOX_KIND_FIXED_SIZE                   = (UI_BOX_KIND_FIXED_WIDTH | UI_BOX_KIND_FIXED_HEIGHT),
    UI_BOX_KIND_ALLOW_OVERFLOW               = (UI_BOX_KIND_ALLOW_OVERFLOW_X | UI_BOX_KIND_ALLOW_OVERFLOW_Y),
    UI_BOX_KIND_ANIMATE_POS                  = (UI_BOX_KIND_ANIMATE_POS_X | UI_BOX_KIND_ANIMATE_POS_Y),
    UI_BOX_KIND_VIEW_SCROLL                  = (UI_BOX_KIND_VIEW_SCROLL_X | UI_BOX_KIND_VIEW_SCROLL_Y),
    UI_BOX_KIND_VIEW_CLAMP                   = (UI_BOX_KIND_VIEW_CLAMP_X | UI_BOX_KIND_VIEW_CLAMP_Y),
    UI_BOX_KIND_DISABLE_FOCUS_EFFECTS        = (UI_BOX_KIND_DISABLE_FOCUS_BORDER | UI_BOX_KIND_DISABLE_FOCUS_OVERLAY)
};


typedef u32 UISignalKind;
enum : u32 {
    UI_SIGNAL_KIND_LEFT_PRESSED           = (1 <<  0),
    UI_SIGNAL_KIND_MIDDLE_PRESSED         = (1 <<  1),
    UI_SIGNAL_KIND_RIGHT_PRESSED          = (1 <<  2),
    UI_SIGNAL_KIND_LEFT_DRAGGING          = (1 <<  3),
    UI_SIGNAL_KIND_MIDDLE_DRAGGING        = (1 <<  4),
    UI_SIGNAL_KIND_RIGHT_DRAGGING         = (1 <<  5),
    UI_SIGNAL_KIND_LEFT_DOUBLE_DRAGGING   = (1 <<  6),
    UI_SIGNAL_KIND_MIDDLE_DOUBLE_DRAGGING = (1 <<  7),
    UI_SIGNAL_KIND_RIGHT_DOUBLE_DRAGGING  = (1 <<  8),
    UI_SIGNAL_KIND_LEFT_TRIPLE_DRAGGING   = (1 <<  9),
    UI_SIGNAL_KIND_MIDDLE_TRIPLE_DRAGGING = (1 << 10),
    UI_SIGNAL_KIND_RIGHT_TRIPLE_DRAGGING  = (1 << 11),
    UI_SIGNAL_KIND_LEFT_RELEASED          = (1 << 12),
    UI_SIGNAL_KIND_MIDDLE_RELEASED        = (1 << 13),
    UI_SIGNAL_KIND_RIGHT_RELEASED         = (1 << 14),
    UI_SIGNAL_KIND_LEFT_CLICKED           = (1 << 15),
    UI_SIGNAL_KIND_MIDDLE_CLICKED         = (1 << 16),
    UI_SIGNAL_KIND_RIGHT_CLICKED          = (1 << 17),
    UI_SIGNAL_KIND_LEFT_DOUBLE_CLICKED    = (1 << 18),
    UI_SIGNAL_KIND_MIDDLE_DOUBLE_CLICKED  = (1 << 19),
    UI_SIGNAL_KIND_RIGHT_DOUBLE_CLICKED   = (1 << 20),
    UI_SIGNAL_KIND_LEFT_TRIPLE_CLICKED    = (1 << 21),
    UI_SIGNAL_KIND_MIDDLE_TRIPLE_CLICKED  = (1 << 22),
    UI_SIGNAL_KIND_RIGHT_TRIPLE_CLICKED   = (1 << 23),
    UI_SIGNAL_KIND_KEYBOARD_PRESSED       = (1 << 24),
    UI_SIGNAL_KIND_HOVERING               = (1 << 25),
    UI_SIGNAL_KIND_MOUSE_OVER             = (1 << 26),
    UI_SIGNAL_KIND_COMMIT                 = (1 << 27),
    UI_SIGNAL_KIND_PRESSED                = (UI_SIGNAL_KIND_LEFT_PRESSED | UI_SIGNAL_KIND_KEYBOARD_PRESSED),
    UI_SIGNAL_KIND_RELEASED               = (UI_SIGNAL_KIND_LEFT_RELEASED),
    UI_SIGNAL_KIND_CLICKED                = (UI_SIGNAL_KIND_LEFT_CLICKED | UI_SIGNAL_KIND_KEYBOARD_PRESSED),
    UI_SIGNAL_KIND_DOUBLE_CLICKED         = (UI_SIGNAL_KIND_LEFT_DOUBLE_CLICKED),
    UI_SIGNAL_KIND_TRIPLE_CLICKED         = (UI_SIGNAL_KIND_LEFT_TRIPLE_CLICKED),
    UI_SIGNAL_KIND_DRAGGING               = (UI_SIGNAL_KIND_LEFT_DRAGGING)
};


#define UI_PRESSED(X)        !!((X).Kind & UI_SIGNAL_KIND_PRESSED)
#define UI_CLICKED(X)        !!((X).Kind & UI_SIGNAL_KIND_CLICKED)
#define UI_RELEASED(X)       !!((X).Kind & UI_SIGNAL_KIND_RELEASED)
#define UI_DOUBLE_CLICKED(X) !!((X).Kind & UI_SIGNAL_KIND_DOUBLE_CLICKED)
#define UI_TRIPLE_CLICKED(X) !!((X).Kind & UI_SIGNAL_KIND_TRIPLE_CLICKED)
#define UI_MIDDLE_CLICKED(X) !!((X).Kind & UI_SIGNAL_KIND_MIDDLE_CLICKED)
#define UI_RIGHT_CLICKED(X)  !!((X).Kind & UI_SIGNAL_KIND_RIGHT_CLICKED)
#define UI_DRAGGING(X)       !!((X).Kind & UI_SIGNAL_KIND_DRAGGING)
#define UI_HOVERING(X)       !!((X).Kind & UI_SIGNAL_KIND_HOVERING)
#define UI_MOUSE_OVER(X)     !!((X).Kind & UI_SIGNAL_KIND_MOUSE_OVER)
#define UI_COMMITTED(X)      !!((X).Kind & UI_SIGNAL_KIND_COMMIT)

#define UI_PX(X, Y)           UISizeAlloc(UI_SIZE_KIND_PIXELS, (X), (Y))
#define UI_TEXT_DIM(X, Y)     UISizeAlloc(UI_SIZE_KIND_TEXT_CONTENT, (X), (Y))
#define UI_PERCENT(X, Y)      UISizeAlloc(UI_SIZE_KIND_PERCENT_OF_PARENT, (X), (Y))
#define UI_SUM_OF_CHILDREN(X) UISizeAlloc(UI_SIZE_KIND_SUM_OF_CHILDREN, 0.0f, (X))

#define UIBoxRecordDFPre(X, Y)  UIBoxRecordDF(X, Y, OFFSETOF(UIBox, Next), OFFSETOF(UIBox, Head))
#define UIBoxRecordDFPost(X, Y) UIBoxRecordDF(X, Y, OFFSETOF(UIBox, Prev), OFFSETOF(UIBox, Tail))

#define UIStoreDragStruct(X)     UIStoreDragData(Str((u8*) (X), sizeof(*(X))))
#define UIGetDragStruct(X) ((X*) UIGetDragData(sizeof(X)).Str)

#define UIAnimate(X, Y, ...) [&]() -> f32 {                                                                \
    UIAnimationParameters __Params = {(GetTLS()->UI_STATE->DefaultAnimationRate), (Y), 0.0f, 0.0f, FALSE}; \
    __VA_ARGS__;                                                                                           \
    return __UIAnimate((X), __Params);                                                                    \
}()

#define UIParent(X)            DEFER(UIPushParent(X), UIPopParent())
#define UIChildLayoutAxis(X)   DEFER(UIPushChildLayoutAxis(X), UIPopChildLayoutAxis())
#define UIFixedX(X)            DEFER(UIPushFixedX(X), UIPopFixedX())
#define UIFixedY(X)            DEFER(UIPushFixedY(X), UIPopFixedY())
#define UIFixedWidth(X)        DEFER(UIPushFixedWidth(X), UIPopFixedWidth())
#define UIFixedHeight(X)       DEFER(UIPushFixedHeight(X), UIPopFixedHeight())
#define UIPreferredWidth(X)    DEFER(UIPushPreferredWidth(X), UIPopPreferredWidth())
#define UIPreferredHeight(X)   DEFER(UIPushPreferredHeight(X), UIPopPreferredHeight())
#define UIMinWidth(X)          DEFER(UIPushMinWidth(X), UIPopMinWidth())
#define UIMinHeight(X)         DEFER(UIPushMinHeight(X), UIPopMinHeight())
#define UIPermissionFlags(X)   DEFER(UIPushPermissionKind(X), UIPopPermissionKind())
#define UIFlags(X)             DEFER(UIPushFlags(X), UIPopFlags())
#define UIOmitFlags(X)         DEFER(UIPushOmitFlags(X), UIPopOmitFlags())
#define UIFocusHot(X)          DEFER(UIPushFocusHot(X), UIPopFocusHot())
#define UIFocusActive(X)       DEFER(UIPushFocusActive(X), UIPopFocusActive())
#define UIFastpathCodepoint(X) DEFER(UIPushFastpathCodepoint(X), UIPopFastpathCodepoint())
#define UIGroupKey(X)          DEFER(UIPushGroupKey(X), UIPopGroupKey())
#define UITag(X)               DEFER(UIPushTag(X), UIPopTag())
#define UIBackgroundColour(X)  DEFER(UIPushBackgroundColour(X), UIPopBackgroundColour())
#define UITextColour(X)        DEFER(UIPushTextColour(X), UIPopTextColour())
#define UIHoverCursor(X)       DEFER(UIPushHoverCursor(X), UIPopHoverCursor())
#define UITabSize(X)           DEFER(UIPushTabSize(X), UIPopTabSize())
#define UITextPadding(X)       DEFER(UIPushTextPadding(X), UIPopTextPadding())
#define UITextAlignment(X)     DEFER(UIPushTextAlignment(X), UIPopTextAlignment())
#define UIFixedPosition(V)     DEFER((UIPushFixedX((V).X), UIPushFixedY((V).Y)), (UIPopFixedX(), UIPopFixedY()))
#define UIFixedSize(V)         DEFER((UIPushFixedWidth((V).X), UIPushFixedHeight((V).Y)), (UIPopFixedWidth(), UIPopFixedHeight()))
#define UIWidthFill()          UIPreferredWidth(UI_PERCENT(1.0f, 0.0f))
#define UIHeightFill()         UIPreferredHeight(UI_PERCENT(1.0f, 0.0f))
#define UIRect(X)              DEFER(UIPushRect(X), UIPopRect())
#define UIPreferredSize(X, Y)  DEFER(UIPushPreferredSize((X), (Y)), UIPopPreferredSize((X)))
#define UIFocus(X)             DEFER((UIPushFocusHot(X), UIPushFocusActive(X)), (UIPopFocusHot(), UIPopFocusActive()))
#define UIAddFlags(X)          DEFER(UIPushFlags(UIHeadFlags() | (X)), UIPopFlags())
#define UITagFmt(...)          DEFER(UIPushTagFmt(__VA_ARGS__), UIPopTag())
#define UITooltipBase()        DEFER(UITooltipBeginBase(), UITooltipEndBase())
#define UITooltip()            DEFER(UITooltipBegin(), UITooltipEnd())
#define UIContextMenu(X)       DEFER_SAFE(UIBeginContextMenu(X), UIEndContextMenu())
#define UIDebug()              UIAddFlags(UI_BOX_KIND_DEBUG)

// @types______________________________________________________________________
struct UIIconInfo {
    Str8 IconKindTextMap[UI_ICON_KIND_COUNT];
};

struct UIEvent {
    UIEventKind        Kind;
    UIEventActionSlot  Slot;
    UIEventFlag        Flags;
    UIEventDeltaStride DeltaStride;
    InputKind          Input;
    InputModifierKind  Modifiers;
    Str8               String;
    Str8List           Paths;
    v2f32              Position;
    v2f32              DeltaF32;
    v2i32              DeltaI32;
    u64                TimestampUSecs;
};

struct UIEventNode {
    UIEventNode* Next;
    UIEventNode* Prev;
    UIEvent      V;
};

struct UIEventList {
    UIEventNode* Head;
    UIEventNode* Tail;
    u64          Count;
};

struct UITextOp {
    UITextOpKind Kind;
    Str8         Replace;
    Str8         Copy;
    TextRange    Range;
    TextPoint    Cursor;
    TextPoint    Mark;
};

struct UIKey {
    u64 V[1];
};

struct UISize {
    UISizeKind Kind;
    f32        Value;
    f32        Strictness;
};

struct UIThemePattern {
    Str8Array Tags;
    v4f32     Linear;
};

struct UITheme {
    UIThemePattern* Patterns;
    u64             PatternsCount;
};

struct UIAnimationInfo {
    f32 HotAnimationRate;
    f32 ActiveAnimationRate;
    f32 FocusAnimationRate;
    f32 ScrollAnimationRate;
};

struct UIScrollPoint {
    i64 Index;
    f32 Offset;
};

union UIScrollPoint2D {
    struct {
        UIScrollPoint X;
        UIScrollPoint Y;
    };

    UIScrollPoint V[2];
};

struct UIBox {
    UIBox*                   HashNext;
    UIBox*                   HashPrev;
    UIBox*                   Head;
    UIBox*                   Tail;
    UIBox*                   Next;
    UIBox*                   Prev;
    UIBox*                   Parent;
    u64                      ChildCount;
    UIKey                    Key;
    UIBoxKind                Kind;
    UIKey                    TagsKey;
    Str8                     String;
    UITextAlign              TextAlign;
    v2f32                    FixedPosition;
    v2f32                    FixedSize;
    v2f32                    MinSize;
    UISize                   PreferredSize[AXIS_2D_COUNT];
    Axis2D                   ChildLayoutAxis;
    CursorKind               HoverCursor;
    u32                      FastpathCodepoint;
    UIKey                    GroupKey;
    UIBoxCustomDrawFuncType* CustomDrawFunc;
    void*                    CustomDrawUserData;
    v4f32                    BackgroundColour;
    v4f32                    TextColour;
    v4f32                    BorderColour;
    f32                      TabSize;
    f32                      TextPadding;
    FancyStrList             DisplayFancyStrings;
    v2f32                    DisplayDimensions;
    r2f32                    Rect;
    v2f32                    FixedPositionAnimated;
    v2f32                    PositionDelta;
    FMRangeList              FuzzyMatchRanges;
    u64                      FirstTouchedBuildIndex;
    u64                      LastTouchedBuildIndex;
    u64                      FirstDisabledBuildIndex;
    f32                      Hot;
    f32                      Active;
    f32                      Disabled;
    f32                      FocusHot;
    f32                      FocusActive;
    f32                      FocusDisabled;
    v2f32                    ViewOffset;
    v2f32                    ViewOffsetTarget;
    v2f32                    ViewBounds;
    UIKey                    DefaultNavFocusHotKey;
    UIKey                    DefaultNavFocusActiveKey;
    UIKey                    DefaultNavFocusNextHotKey;
    UIKey                    DefaultNavFocusNextActiveKey;
};

struct UIBoxRecord {
    UIBox* Next;
    i32    PushCount;
    i32    PopCount;
};

struct UIBoxNode {
    UIBoxNode* Next;
    UIBox*     Box;
};

struct UIBoxList {
    UIBoxNode* Head;
    UIBoxNode* Tail;
    u64        Count;
};

struct UISignal {
    UIBox*            Box;
    InputModifierKind EventKind;
    v2i16             Scroll;
    UISignalKind      Kind;
};

struct UINav {
    b32   Moved;
    v2i64 NewPosition;
};

struct UIAnimationParameters {
    f32 Rate;
    f32 Target;
    f32 Initial;
    f32 Epsilon;
    b32 Reset;
};

struct UIAnimationNode {
    UIAnimationNode*      SlotNext;
    UIAnimationNode*      SlotPrev;
    UIAnimationNode*      LRUNext;
    UIAnimationNode*      LRUPrev;
    u64                   FirstTouchedBuildIndex;
    u64                   LastTouchedBuildIndex;
    UIKey                 Key;
    UIAnimationParameters Params;
    f32                   Current;
};

struct UIAnimationSlot {
    UIAnimationNode* Head;
    UIAnimationNode* Tail;
};

struct UIParentNode {
    UIParentNode* Next;
    UIBox*        V;
};

struct UIChildLayoutAxisNode {
    UIChildLayoutAxisNode* Next;
    Axis2D                 V;
};

struct UIFixedXNode {
    UIFixedXNode* Next;
    f32           V;
};

struct UIFixedYNode {
    UIFixedYNode* Next;
    f32           V;
};

struct UIFixedWidthNode {
    UIFixedWidthNode* Next;
    f32               V;
};

struct UIFixedHeightNode {
    UIFixedHeightNode* Next;
    f32                V;
};

struct UIPreferredWidthNode {
    UIPreferredWidthNode* Next;
    UISize                V;
};

struct UIPreferredHeightNode {
    UIPreferredHeightNode* Next;
    UISize                 V;
};

struct UIMinWidthNode {
    UIMinWidthNode* Next;
    f32             V;
};

struct UIMinHeightNode {
    UIMinHeightNode* Next;
    f32              V;
};

struct UIPermissionKindsNode {
    UIPermissionKindsNode* Next;
    UIPermissionKind       V;
};

struct UIFlagsNode {
    UIFlagsNode* Next;
    UIBoxKind    V;
};

struct UIOmitFlagsNode {
    UIOmitFlagsNode* Next;
    UIBoxKind        V;
};

struct UIFocusHotNode {
    UIFocusHotNode* Next;
    UIFocusKind     V;
};

struct UIFocusActiveNode {
    UIFocusActiveNode* Next;
    UIFocusKind        V;
};

struct UIFastpathCodepointNode {
    UIFastpathCodepointNode* Next;
    u32                      V;
};

struct UIGroupKeyNode {
    UIGroupKeyNode* Next;
    UIKey           V;
};

struct UITagNode {
    UITagNode* Next;
    Str8       V;
};

struct UIBackgroundColourNode {
    UIBackgroundColourNode* Next;
    v4f32                   V;
};

struct UITextColourNode {
    UITextColourNode* Next;
    v4f32             V;
};

struct UIBorderColourNode {
    UIBorderColourNode* Next;
    v4f32               V;
};

struct UIHoverCursorNode {
    UIHoverCursorNode* Next;
    CursorKind         V;
};

struct UITabSizeNode {
    UITabSizeNode* Next;
    f32            V;
};

struct UITextPaddingNode {
    UITextPaddingNode* Next;
    f32                V;
};

struct UITextAlignmentNode {
    UITextAlignmentNode* Next;
    UITextAlign          V;
};

struct UITagsCacheNode {
    UITagsCacheNode* Next;
    UIKey            Key;
    Str8Array        Tags;
};

struct UITagsCacheSlot {
    UITagsCacheNode* Head;
    UITagsCacheNode* Tail;
};

struct UITagsKeyStackNode {
    UITagsKeyStackNode* Next;
    UIKey               Key;
};

struct UIThemePatternCacheNode {
    UIThemePatternCacheNode* SlotNext;
    UIThemePatternCacheNode* SlotPrev;
    UIThemePatternCacheNode* LRUNext;
    UIThemePatternCacheNode* LRUPrev;
    u64                      LastBuildIndexAccessed;
    UIKey                    Key;
    v4f32                    TargetRGBA;
    v4f32                    CurrentRGBA;
};

struct UIThemePatternCacheSlot {
    UIThemePatternCacheNode* Head;
    UIThemePatternCacheNode* Tail;
};

struct UIBoxHashSlot {
    UIBox* Head;
    UIBox* Tail;
};

struct UIState {
    Arena*                   MemPool;
    UIKey                    ExternalKey;
    Arena*                   BuildMemPools[2];
    u64                      BuildIndex;
    UIBox*                   HeadFreeBox;
    u64                      BoxTableSize;
    UIBoxHashSlot*           BoxTable;
    UIAnimationNode*         FreeAnimationNode;
    UIAnimationNode*         LRUAnimationNode;
    UIAnimationNode*         MRUAnimationNode;
    u64                      AnimationSlotsCount;
    UIAnimationSlot*         AnimationSlots;
    b32                      IsInOpenContextMenu;
    Str8                     AutoCompleteString;
    UIKey                    TooltipAnchorKey;
    Str8Array                CurrentGenerationTags;
    u64                      CurrentGenerationTagsGeneration;
    UITagsKeyStackNode*      TagsKeyStackHead;
    UITagsKeyStackNode*      TagsKeyStackFree;
    u64                      TagsCacheSlotsCount;
    UITagsCacheSlot*         TagsCacheSlots;
    u64                      ThemePatternCacheSlotsCount;
    UIThemePatternCacheSlot* ThemePatternCacheSlots;
    UIThemePatternCacheNode* ThemePatternCacheNodeFree;
    UIThemePatternCacheNode* LRUThemePatternCacheNode;
    UIThemePatternCacheNode* MRUThemePatternCacheNode;
    UIBox*                   Root;
    UIBox*                   TooltipRoot;
    UIBox*                   ContextMenuRoot;
    UIKey                    DefaultNavRootKey;
    u64                      BuildBoxCount;
    u64                      LastBuildBoxCount;
    b32                      ContextMenuTouchedThisFrame;
    b32                      IsAnimating;
    UIIconInfo               IconInfo;
    UITheme*                 Theme;
    UIAnimationInfo          AnimationInfo;
    UIEventList*             Events;
    v2f32                    Mouse;
    f32                      AnimationDelta;
    f32                      DefaultAnimationRate;
    UIKey                    HotBoxKey;
    UIKey                    ActiveBoxKey[UI_MOUSE_BTN_KIND_COUNT];
    UIKey                    DropHotBoxKey;
    UIKey                    ClipboardCopyKey;
    u64                      PressTimestampHistoryUSecs[UI_MOUSE_BTN_KIND_COUNT][3];
    UIKey                    PressKeyHistory[UI_MOUSE_BTN_KIND_COUNT][3];
    v2f32                    PressPositionHistory[UI_MOUSE_BTN_KIND_COUNT][3];
    v2f32                    DragStartMouse;
    Arena*                   DragStateMemPool;
    Str8                     DragStateData;
    Arena*                   StringHoverMemPool;
    Str8                     StringHoverString;
    FancyStrList             StringHoverFancyStrings;
    u64                      StringHoverBeginUSecs;
    u64                      StringHoverBuildIndex;
    u64                      LastTimeMouseMovedUSecs;
    f32                      TooltipOpenT;
    b32                      TooltipOpen;
    UIKey                    ContextMenuAnchorKey;
    UIKey                    NextContextMenuAnchorKey;
    v2f32                    ContextMenuAnchorBoxLastPosition;
    v2f32                    ContextMenuAnchorOffset;
    b32                      ContextMenuOpen;
    b32                      NextContextMenuOpen;
    f32                      ContextMenuOpenT;
    UIKey                    ContextMenuKey;
    b32                      ContextMenuChanged;

    struct {
        UIParentNode            ParentEmptyStackHead;
        UIChildLayoutAxisNode   ChildLayoutAxisEmptyStackHead;
        UIFixedXNode            FixedXEmptyStackHead;
        UIFixedYNode            FixedYEmptyStackHead;
        UIFixedWidthNode        FixedWidthEmptyStackHead;
        UIFixedHeightNode       FixedHeightEmptyStackHead;
        UIPreferredWidthNode    PreferredWidthEmptyStackHead;
        UIPreferredHeightNode   PreferredHeightEmptyStackHead;
        UIMinWidthNode          MinWidthEmptyStackHead;
        UIMinHeightNode         MinHeightEmptyStackHead;
        UIPermissionKindsNode   PermissionKindsEmptyStackHead;
        UIFlagsNode             FlagsEmptyStackHead;
        UIOmitFlagsNode         OmitFlagsEmptyStackHead;
        UIFocusHotNode          FocusHotEmptyStackHead;
        UIFocusActiveNode       FocusActiveEmptyStackHead;
        UIFastpathCodepointNode FastpathCodepointEmptyStackHead;
        UIGroupKeyNode          GroupKeyEmptyStackHead;
        UITagNode               TagEmptyStackHead;
        UIBackgroundColourNode  BackgroundColourEmptyStackHead;
        UITextColourNode        TextColourEmptyStackHead;
        UIBorderColourNode      BorderColourEmptyStackHead;
        UIHoverCursorNode       HoverCursorEmptyStackHead;
        UITabSizeNode           TabSizeEmptyStackHead;
        UITextPaddingNode       TextPaddingEmptyStackHead;
        UITextAlignmentNode     TextAlignmentEmptyStackHead;
    };

    struct {
        UIParentNode* Head;
        UIBox*        TailValue;
        UIParentNode* Free;
        u64           Generation;
        b32           AutoPop;
    } ParentStack;

    struct {
        UIChildLayoutAxisNode* Head;
        Axis2D                 TailValue;
        UIChildLayoutAxisNode* Free;
        u64                    Generation;
        b32                    AutoPop;
    } ChildLayoutAxisStack;

    struct {
        UIFixedXNode* Head;
        f32           TailValue;
        UIFixedXNode* Free;
        u64           Generation;
        b32           AutoPop;
    } FixedXStack;

    struct {
        UIFixedYNode* Head;
        f32           TailValue;
        UIFixedYNode* Free;
        u64           Generation;
        b32           AutoPop;
    } FixedYStack;

    struct {
        UIFixedWidthNode* Head;
        f32               TailValue;
        UIFixedWidthNode* Free;
        u64               Generation;
        b32               AutoPop;
    } FixedWidthStack;

    struct {
        UIFixedHeightNode* Head;
        f32                TailValue;
        UIFixedHeightNode* Free;
        u64                Generation;
        b32                AutoPop;
    } FixedHeightStack;

    struct {
        UIPreferredWidthNode* Head;
        UISize                TailValue;
        UIPreferredWidthNode* Free;
        u64                   Generation;
        b32                   AutoPop;
    } PreferredWidthStack;

    struct {
        UIPreferredHeightNode* Head;
        UISize                 TailValue;
        UIPreferredHeightNode* Free;
        u64                    Generation;
        b32                    AutoPop;
    } PreferredHeightStack;

    struct {
        UIMinWidthNode* Head;
        f32             TailValue;
        UIMinWidthNode* Free;
        u64             Generation;
        b32             AutoPop;
    } MinWidthStack;

    struct {
        UIMinHeightNode* Head;
        f32              TailValue;
        UIMinHeightNode* Free;
        u64              Generation;
        b32              AutoPop;
    } MinHeightStack;

    struct {
        UIPermissionKindsNode* Head;
        UIPermissionKind       TailValue;
        UIPermissionKindsNode* Free;
        u64                    Generation;
        b32                    AutoPop;
    } PermissionKindsStack;

    struct {
        UIFlagsNode* Head;
        UIBoxKind    TailValue;
        UIFlagsNode* Free;
        u64          Generation;
        b32          AutoPop;
    } FlagsStack;

    struct {
        UIOmitFlagsNode* Head;
        UIBoxKind        TailValue;
        UIOmitFlagsNode* Free;
        u64              Generation;
        b32              AutoPop;
    } OmitFlagsStack;

    struct {
        UIFocusHotNode* Head;
        UIFocusKind     TailValue;
        UIFocusHotNode* Free;
        u64             Generation;
        b32             AutoPop;
    } FocusHotStack;

    struct {
        UIFocusActiveNode* Head;
        UIFocusKind        TailValue;
        UIFocusActiveNode* Free;
        u64                Generation;
        b32                AutoPop;
    } FocusActiveStack;

    struct {
        UIFastpathCodepointNode* Head;
        u32                      TailValue;
        UIFastpathCodepointNode* Free;
        u64                      Generation;
        b32                      AutoPop;
    } FastpathCodepointStack;

    struct {
        UIGroupKeyNode* Head;
        UIKey           TailValue;
        UIGroupKeyNode* Free;
        u64             Generation;
        b32             AutoPop;
    } GroupKeyStack;

    struct {
        UITagNode* Head;
        Str8       TailValue;
        UITagNode* Free;
        u64        Generation;
        b32        AutoPop;
    } TagStack;

    struct {
        UIBackgroundColourNode* Head;
        v4f32                   TailValue;
        UIBackgroundColourNode* Free;
        u64                     Generation;
        b32                     AutoPop;
    } BackgroundColourStack;

    struct {
        UITextColourNode* Head;
        v4f32             TailValue;
        UITextColourNode* Free;
        u64               Generation;
        b32               AutoPop;
    } TextColourStack;

    struct {
        UIBorderColourNode* Head;
        v4f32               TailValue;
        UIBorderColourNode* Free;
        u64                 Generation;
        b32                 AutoPop;
    } BorderColourStack;

    struct {
        UIHoverCursorNode* Head;
        CursorKind         TailValue;
        UIHoverCursorNode* Free;
        u64                Generation;
        b32                AutoPop;
    } HoverCursorStack;

    struct {
        UITabSizeNode* Head;
        f32            TailValue;
        UITabSizeNode* Free;
        u64            Generation;
        b32            AutoPop;
    } TabSizeStack;

    struct {
        UITextPaddingNode* Head;
        f32                TailValue;
        UITextPaddingNode* Free;
        u64                Generation;
        b32                AutoPop;
    } TextPaddingStack;

    struct {
        UITextAlignmentNode* Head;
        UITextAlign          TailValue;
        UITextAlignmentNode* Free;
        u64                  Generation;
        b32                  AutoPop;
    } TextAlignmentStack;
};

// @runtime____________________________________________________________________
extern UIBox* const EMPTY_UI_BOX_VALUE;
extern UIAnimationNode* const EMPTY_ANIMATION_NODE_VALUE;

constexpr UIKey EMPTY_UI_KEY_VALUE = {};

extern UIBox* UIColumnBegin(void);
extern UISignal UIColumnEnd(void);
extern UIBox* UIRowBegin(void);
extern UISignal UIRowEnd(void);

// @functions__________________________________________________________________
UIBox* UIHeadParent(void);
Axis2D UIHeadChildLayoutAxis(void);
f32 UIHeadFixedX(void);
f32 UIHeadFixedY(void);
f32 UIHeadFixedWidth(void);
f32 UIHeadFixedHeight(void);
UISize UIHeadPreferredWidth(void);
UISize UIHeadPreferredHeight(void);
f32 UIHeadMinWidth(void);
f32 UIHeadMinHeight(void);
UIPermissionKind UIHeadPermissionKinds(void);
UIBoxKind UIHeadFlags(void);
UIBoxKind UIHeadOmitFlags(void);
UIFocusKind UIHeadFocusHot(void);
UIFocusKind UIHeadFocusActive(void);
u32 UIHeadFastpathCodepoint(void);
UIKey UIHeadGroupKey(void);
Str8 UIHeadTag(void);
v4f32 UIHeadBackgroundColour(void);
v4f32 UIHeadTextColour(void);
v4f32 UIHeadBorderColour(void);
CursorKind UIHeadHoverCursor(void);
f32 UIHeadTabSize(void);
f32 UIHeadTextPadding(void);
UITextAlign UIHeadTextAlignment(void);
UIBox* UITailParent(void);
Axis2D UITailChildLayoutAxis(void);
f32 UITailFixedX(void);
f32 UITailFixedY(void);
f32 UITailFixedWidth(void);
f32 UITailFixedHeight(void);
UISize UITailPreferredWidth(void);
UISize UITailPreferredHeight(void);
f32 UITailMinWidth(void);
f32 UITailMinHeight(void);
UIPermissionKind UITailPermissionKinds(void);
UIBoxKind UITailFlags(void);
UIBoxKind UITailOmitFlags(void);
UIFocusKind UITailFocusHot(void);
UIFocusKind UITailFocusActive(void);
u32 UITailFastpathCodepoint(void);
UIKey UITailGroupKey(void);
Str8 UITailTag(void);
v4f32 UITailBackgroundColour(void);
v4f32 UITailTextColour(void);
v4f32 UITailBorderColour(void);
CursorKind UITailHoverCursor(void);
f32 UITailTabSize(void);
f32 UITailTextPadding(void);
UITextAlign UITailTextAlignment(void);
UIBox* UIPushParent(UIBox* Value);
Axis2D UIPushChildLayoutAxis(Axis2D Value);
f32 UIPushFixedX(f32 Value);
f32 UIPushFixedY(f32 Value);
f32 UIPushFixedWidth(f32 Value);
f32 UIPushFixedHeight(f32 Value);
UISize UIPushPreferredWidth(UISize Value);
UISize UIPushPreferredHeight(UISize Value);
f32 UIPushMinWidth(f32 Value);
f32 UIPushMinHeight(f32 Value);
UIPermissionKind UIPushPermissionKind(UIPermissionKind Value);
UIBoxKind UIPushFlags(UIBoxKind Value);
UIBoxKind UIPushOmitFlags(UIBoxKind Value);
UIFocusKind UIPushFocusHot(UIFocusKind Value);
UIFocusKind UIPushFocusActive(UIFocusKind Value);
u32 UIPushFastpathCodepoint(u32 Value);
UIKey UIPushGroupKey(UIKey Value);
Str8 UIPushTag(Str8 Value);
v4f32 UIPushBackgroundColour(v4f32 Value);
v4f32 UIPushTextColour(v4f32 Value);
v4f32 UIPushBorderColour(v4f32 Value);
CursorKind UIPushHoverCursor(CursorKind Value);
f32 UIPushTabSize(f32 Value);
f32 UIPushTextPadding(f32 Value);
UITextAlign UIPushTextAlignment(UITextAlign Value);
UIBox* UIPopParent(void);
Axis2D UIPopChildLayoutAxis(void);
f32 UIPopFixedX(void);
f32 UIPopFixedY(void);
f32 UIPopFixedWidth(void);
f32 UIPopFixedHeight(void);
UISize UIPopPreferredWidth(void);
UISize UIPopPreferredHeight(void);
f32 UIPopMinWidth(void);
f32 UIPopMinHeight(void);
UIPermissionKind UIPopPermissionKind(void);
UIBoxKind UIPopFlags(void);
UIBoxKind UIPopOmitFlags(void);
UIFocusKind UIPopFocusHot(void);
UIFocusKind UIPopFocusActive(void);
u32 UIPopFastpathCodepoint(void);
UIKey UIPopGroupKey(void);
Str8 UIPopTag(void);
v4f32 UIPopBackgroundColour(void);
v4f32 UIPopTextColour(void);
v4f32 UIPopBorderColour(void);
CursorKind UIPopHoverCursor(void);
f32 UIPopTabSize(void);
f32 UIPopTextPadding(void);
UITextAlign UIPopTextAlignment(void);
UIBox* UISetNextParent(UIBox* Value);
Axis2D UISetNextChildLayoutAxis(Axis2D Value);
f32 UISetNextFixedX(f32 Value);
f32 UISetNextFixedY(f32 Value);
f32 UISetNextFixedWidth(f32 Value);
f32 UISetNextFixedHeight(f32 Value);
UISize UISetNextPreferredWidth(UISize Value);
UISize UISetNextPreferredHeight(UISize Value);
f32 UISetNextMinWidth(f32 Value);
f32 UISetNextMinHeight(f32 Value);
UIPermissionKind UISetNextPermissionKinds(UIPermissionKind Value);
UIBoxKind UISetNextFlags(UIBoxKind Value);
UIBoxKind UISetNextOmitFlags(UIBoxKind Value);
UIFocusKind UISetNextFocusHot(UIFocusKind Value);
UIFocusKind UISetNextFocusActive(UIFocusKind Value);
u32 UISetNextFastpathCodepoint(u32 Value);
UIKey UISetNextGroupKey(UIKey Value);
Str8 UISetNextTag(Str8 Value);
v4f32 UISetNextBackgroundColour(v4f32 Value);
v4f32 UISetNextTextColour(v4f32 Value);
v4f32 UISetNextBorderColour(v4f32 Value);
CursorKind UISetNextHoverCursor(CursorKind Value);
f32 UISetNextTabSize(f32 Value);
f32 UISetNextTextPadding(f32 Value);
UITextAlign UISetNextTextAlignment(UITextAlign Value);
void UIInit(void);
Str8 UIHashPartFromKeyStr(Str8 KeyString);
Str8 UIDisplayPartFromKeyStr(Str8 KeyString);
UIKey UIKeyAlloc(u64 Value);
UIKey UIKeyFromStr(UIKey SeedKey, Str8 String);
UIKey UIKeyFromStrFmt(UIKey SeedKey, char* Fmt, ...);
UIEventNode* ListPush(Arena* MemPool, UIEventList* List, UIEvent* Event);
void UIEatEventNode(UIEventList* List, UIEventNode* Node);
b32 UICharIsScanBoundary(u8 Char);
i64 UIScannedColumnFromColumn(Str8 String, i64 StartColumn, SideKind Side);
UITextOp UISingleLineTextOpFromEvent(Arena* MemPool, UIEvent* Event, Str8 String, TextPoint Cursor, TextPoint Mark);
Str8 UIPushStrReplaceRange(Arena* MemPool, Str8 String, r1i64 Range, Str8 Replace);
UISize UISizeAlloc(UISizeKind Kind, f32 Value, f32 Strictness);
UIScrollPoint UIScrollPointAlloc(i64 Index, f32 Offset);
void UIScrollPointTargetIndex(UIScrollPoint* Scroll, i64 Index);
void UIScrollPointClampIndex(UIScrollPoint* Scroll, r1i64 Range);
void ListPush(Arena* MemPool, UIBoxList* List, UIBox* Box);
UIState* UIStateAlloc(void);
void UIStateRelease(UIState* State);
UIBox* UIRootFromState(UIState* State);
b32 UIIsAnimatingFromState(UIState* State);
void UISelectState(UIState* State);
UIState* UIGetSelectedState(void);
Arena* UIBuildMemPool(void);
v2f32 UIMouse(void);
Str8 UIIconStringFromIconKind(UIIconKind Kind);
f32 UIDeltaTime(void);
b32 UINextEvent(UIEvent** Event);
void UIEatEvent(UIEvent* Event);
b32 UIKeyPress(InputModifierKind Modifiers, InputKind Input);
b32 UIKeyRelease(InputModifierKind Modifiers, InputKind Input);
b32 UIText(u32 Character);
b32 UISlotPress(UIEventActionSlot Slot);
void UISetAutoCompleteStr(Str8 String);
Str8 UIGetAutoCompleteStr(void);
Str8 UIAutoComplete(void);
v2f32 UIStartDragMouse(void);
v2f32 UIDragDelta(void);
void UIStoreDragData(Str8 String);
Str8 UIGetDragData(u64 MinRequiredSize);
b32 UIStringHoverActive(void);
u64 UIStringHoverBeginTimeUSecs(void);
FancyStrList UIStringHoverFancyStrings(Arena* MemPool);
UIKey UIHotKey(void);
UIKey UIActiveKey(UIMouseButtonKind BtnKind);
UIKey UIDropHotKey(void);
void UIKillAction(void);
UIBox* UIBoxFromKey(UIKey Key);
void UIBeginBuild(UIEventList* Events, UIIconInfo* IconInfo, UITheme* Theme, UIAnimationInfo* AnimationInfo, f32 RealDeltaTime, f32 AnimationDeltaTime);
void UIEndBuild(void);
void UICalcSizesStandalone(UIBox* Root, Axis2D Axis);
void UICalcSizesUpwardsDependent(UIBox* Root, Axis2D Axis);
void UICalcSizesDownwardsDependent(UIBox* Root, Axis2D Axis);
void UILayoutEnforceConstraints(UIBox* Root, Axis2D Axis);
void UILayoutPosition(UIBox* Root, Axis2D Axis);
void UILayoutRoot(UIBox* Root, Axis2D Axis);
UISignal UISpacer(UISize Size);
void UITooltipBeginBase(void);
void UITooltipEndBase(void);
void UITooltipBegin(void);
void UITooltipEnd(void);
void UIContextMenuOpen(UIKey Key, UIKey AnchorBoxKey, v2f32 AnchorOffset);
void UIContextMenuClose(void);
b32 UIBeginContextMenu(UIKey Key);
void UIEndContextMenu(void);
b32 UIContextMenuIsOpen(UIKey Key);
b32 UIAnyContextMenuIsOpen(void);
b32 UIIsFocusHot(void);
b32 UIIsFocusActive(void);
b32 UIIsKeyAutoFocusActive(UIKey Key);
b32 UIIsKeyAutoFocusHot(UIKey Key);
void UISetAutoFocusActiveKey(UIKey Key);
void UISetAutoFocusHotKey(UIKey Key);
UIKey UIHeadTagsKey(void);
v4f32 UIColourFromName(Str8 Name);
v4f32 UIColourFromTagsKeyExtras(UIKey Key, Str8Array Extras);
v4f32 UIColourFromTagsKeyName(UIKey Key, Str8 Name);
UIBox* UIBuildBoxFromKey(UIBoxKind Kind, UIKey Key);
UIKey UIActiveSeedKey(void);
UIBox* UIBuildBoxFromStr(UIBoxKind Kind, Str8 String);
UIBox* UIBuildBoxFromStrFmt(UIBoxKind Kind, char* Fmt, ...);
Str8 UIBoxDisplayStr(UIBox* Box);
v2f32 UIBoxTextPosition(UIBox* Box);
u64 UIBoxCharPositionFromXY(UIBox* Box, v2f32 XY);
UISignal UISignalFromBox(UIBox* Box);
void UIBoxEquipDisplayStr(UIBox* Box, Str8 String);
void UIBoxEquipDisplayFancyStrs(UIBox* Box, FancyStrList* Strings);
void UIBoxEquipCustomDraw(UIBox* Box, UIBoxCustomDrawFuncType* CustomDrawFunc, void* UserData);
f32 __UIAnimate(UIKey Key, UIAnimationParameters Params);
void __UIPushTagsKeyFromAppendedStr(Str8 String);
void __UIPopTagsKey(void);
r2f32 UIPushRect(r2f32 Rect);
r2f32 UIPopRect(void);
void UISetNextRect(r2f32 Rect);
UISize UIPushPreferredSize(Axis2D Axis, UISize Size);
UISize UIPopPreferredSize(Axis2D Axis);
UISize UISetNextPreferredSize(Axis2D Axis, UISize Size);
void UIPushTagFmt(char* Fmt, ...);
b32 IsUIBoxEmpty(UIBox* Box);
UIBoxRecord UIBoxRecordDF(UIBox* Box, UIBox* Root, u64 SiblingMemberOffset, u64 ChildMemberOffset);
void UIBoxEquipFuzzyMatchRanges(UIBox* Box, FMRangeList* Matches);


#if defined(NC_LANG_CPP)
    b32 operator==(UIKey KeyA, UIKey KeyB);
    b32 operator!=(UIKey KeyA, UIKey KeyB);
#endif

#endif // __NC_UI_H__
