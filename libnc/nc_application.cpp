#include <stdarg.h>

#include "nc_application.h"
#include "nc_memory.h"
#include "nc_arena.h"
#include "nc_atomics.h"
#include "nc_config.h"
#include "nc_console.h"
#include "nc_content.h"
#include "nc_defines.h"
#include "nc_draw.h"
#include "nc_file.h"
#include "nc_file_stream.h"
#include "nc_log.h"
#include "nc_math.h"
#include "nc_metadesk.h"
#include "nc_object_cache.h"
#include "nc_render.h"
#include "nc_string.h"
#include "nc_system.h"
#include "nc_thread.h"
#include "nc_time.h"
#include "nc_tls.h"
#include "nc_ui.h"
#include "nc_ui_components.h"

AppViewState              __EMPTY_APP_VIEW_STATE_VALUE;
AppViewState* const       EMPTY_APP_VIEW_STATE_VALUE        = &__EMPTY_APP_VIEW_STATE_VALUE;
AppCommandKindInfo        __EMPTY_APP_COMMAND_KIND_INFO_VALUE;
AppCommandKindInfo* const EMPTY_APP_COMMAND_KIND_INFO_VALUE = &__EMPTY_APP_COMMAND_KIND_INFO_VALUE;
AppState*                 APP_STATE                         = NULL;

EXTERN_C_LINK_BEGIN
AppCommandKindInfo APP_COMMAND_KIND_INFO_TABLE[APP_COMMAND_KIND_COUNT] = {
    {},
    {
        Str8Lit("exit"),
        Str8Lit("Exit"),
        Str8Lit("Stops the program."),
        Str8Lit("quit,close,end"),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_X
    },
    {
        Str8Lit("open_palette"),
        Str8Lit("Open Palette"),
        Str8Lit("Shows the list of commands."),
        Str8Lit("help,cmd,lister,commands"),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_FIND
    },
    {
        Str8Lit("run_command"),
        Str8Lit("Run Command"),
        Str8Lit("Runs a command from the palette."),
        Str8Lit("help,cmd"),
        0,
        APP_ICON_KIND_NULL,
        {
            APP_QUERY_KIND_FLOATING | APP_QUERY_KIND_REQUIRED,
            APP_REGISTERS_SLOT_COMMAND_NAME,
            Str8Lit("commands")
        }
    },
    {
        Str8Lit("os_event"),
        Str8Lit(""),
        Str8Lit(""),
        Str8Lit(""),
        0,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("popup_accept"),
        Str8Lit("Accept"),
        Str8Lit("Accepts the question that is on the screen."),
        Str8Lit(""),
        0,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("popup_cancel"),
        Str8Lit("Cancel"),
        Str8Lit("Cancels the question that is on the screen."),
        Str8Lit(""),
        0,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("reset_to_default_bindings"),
        Str8Lit("Reset Keys"),
        Str8Lit("Sets all keys to their initial commands."),
        Str8Lit("keys,bindings,hotkeys"),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("new_panel_left"),
        Str8Lit("Split Panel Left"),
        Str8Lit("Makes a new panel on the left side of the focused panel."),
        Str8Lit("split,pane,side"),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_LEFT_ARROW
    },
    {
        Str8Lit("new_panel_up"),
        Str8Lit("Split Panel Up"),
        Str8Lit("Makes a new panel above the focused panel."),
        Str8Lit("split,pane,side"),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_UP_ARROW
    },
    {

        Str8Lit("new_panel_right"),
        Str8Lit("Split Panel Right"),
        Str8Lit("Makes a new panel on the right side of the focused panel."),
        Str8Lit("split,pane,side"),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_RIGHT_ARROW
    },
    {

        Str8Lit("new_panel_down"),
        Str8Lit("Split Panel Down"),
        Str8Lit("Makes a new panel below the focused panel."),
        Str8Lit("split,pane,side"),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_DOWN_ARROW
    },
    {
        Str8Lit("split_panel"),
        Str8Lit("Split Panel"),
        Str8Lit("Makes a new panel and moves a tab into it."),
        Str8Lit(""),
        0,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("next_panel"),
        Str8Lit("Next Panel"),
        Str8Lit("Moves the focus to the next panel."),
        Str8Lit(""),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("prev_panel"),
        Str8Lit("Previous Panel"),
        Str8Lit("Moves the focus to the previous panel."),
        Str8Lit(""),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("focus_panel"),
        Str8Lit("Focus Panel"),
        Str8Lit("Moves the focus to a panel."),
        Str8Lit(""),
        0,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("focus_panel_right"),
        Str8Lit("Focus Panel Right"),
        Str8Lit("Moves the focus to the panel on the right."),
        Str8Lit(""),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("focus_panel_left"),
        Str8Lit("Focus Panel Left"),
        Str8Lit("Moves the focus to the panel on the left."),
        Str8Lit(""),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("focus_panel_up"),
        Str8Lit("Focus Panel Up"),
        Str8Lit("Moves the focus to the panel above."),
        Str8Lit(""),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("focus_panel_down"),
        Str8Lit("Focus Panel Down"),
        Str8Lit("Moves the focus to the panel below."),
        Str8Lit(""),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("go_back"),
        Str8Lit("Go Back"),
        Str8Lit("Goes to the position that you had before."),
        Str8Lit("history"),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_LEFT_ARROW
    },
    {
        Str8Lit("go_forward"),
        Str8Lit("Go Forward"),
        Str8Lit("Goes to the position that you had before you went back."),
        Str8Lit("history"),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_RIGHT_ARROW
    },
    {
        Str8Lit("close_panel"),
        Str8Lit("Close Panel"),
        Str8Lit("Closes the panel and all of its tabs."),
        Str8Lit("split,pane"),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_X
    },
    {
        Str8Lit("focus_tab"),
        Str8Lit("Focus Tab"),
        Str8Lit("Shows a tab."),
        Str8Lit(""),
        0,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("next_tab"),
        Str8Lit("Next Tab"),
        Str8Lit("Shows the next tab."),
        Str8Lit(""),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("prev_tab"),
        Str8Lit("Previous Tab"),
        Str8Lit("Shows the previous tab."),
        Str8Lit(""),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("move_tab_right"),
        Str8Lit("Move Tab Right"),
        Str8Lit("Moves the tab one position to the right."),
        Str8Lit(""),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI | APP_COMMAND_KIND_FLAG_LIST_IN_TAB,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("move_tab_left"),
        Str8Lit("Move Tab Left"),
        Str8Lit("Moves the tab one position to the left."),
        Str8Lit(""),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI | APP_COMMAND_KIND_FLAG_LIST_IN_TAB,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("open_tab"),
        Str8Lit("Open Tab"),
        Str8Lit("Opens a new tab."),
        Str8Lit("view,new"),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_ADD,
        {
            APP_QUERY_KIND_FLOATING | APP_QUERY_KIND_REQUIRED,
            APP_REGISTERS_SLOT_COMMAND_NAME,
            Str8Lit("tab_commands")
        }
    },
    {
        Str8Lit("build_tab"),
        Str8Lit("Build Tab"),
        Str8Lit("Opens a new tab that shows a view of a file."),
        Str8Lit(""),
        0,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("duplicate_tab"),
        Str8Lit("Duplicate Tab"),
        Str8Lit("Makes a copy of the tab."),
        Str8Lit("copy,clone"),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI | APP_COMMAND_KIND_FLAG_LIST_IN_TAB,
        APP_ICON_KIND_ADD
    },
    {
        Str8Lit("close_tab"),
        Str8Lit("Close Tab"),
        Str8Lit("Closes the tab."),
        Str8Lit(""),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI | APP_COMMAND_KIND_FLAG_LIST_IN_TAB,
        APP_ICON_KIND_X
    },
    {
        Str8Lit("move_view"),
        Str8Lit("Move View"),
        Str8Lit("Moves a tab to a position in a panel."),
        Str8Lit(""),
        0,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("set_tab_view"),
        Str8Lit("Set Tab View"),
        Str8Lit("Changes the view that the tab is currently showing."),
        Str8Lit(""),
        0,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("change_file"),
        Str8Lit("Change File"),
        Str8Lit("Changes the tlog file that the tab is showing."),
        Str8Lit("file,tlog,load"),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI | APP_COMMAND_KIND_FLAG_LIST_IN_TAB,
        APP_ICON_KIND_FILE,
        {
            (
                APP_QUERY_KIND_ALLOW_FILES | 
                APP_QUERY_KIND_FLOATING | 
                APP_QUERY_KIND_REQUIRED
            ),
            APP_REGISTERS_SLOT_FILE_PATH,
            Str8Lit("")
        }
    },
    {
        Str8Lit("set_current_path"),
        Str8Lit("Set Current Path"),
        Str8Lit("Sets the folder that the file list starts in."),
        Str8Lit(""),
        0,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("open"),
        Str8Lit("Open"),
        Str8Lit("Opens a tlog file, or all tlog files in a folder."),
        Str8Lit("file,folder,tlog,load"),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_FOLDER,
        {
            APP_QUERY_KIND_ALLOW_FILES | APP_QUERY_KIND_ALLOW_FOLDERS | APP_QUERY_KIND_FLOATING | APP_QUERY_KIND_REQUIRED,
            APP_REGISTERS_SLOT_FILE_PATH,
            Str8Lit("")
        }
    },
    {
        Str8Lit("edit"),
        Str8Lit("Edit"),
        Str8Lit("Starts an edit of the selection."),
        Str8Lit(""),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("accept"),
        Str8Lit("Accept"),
        Str8Lit("Accepts the changes, or gives the answer yes."),
        Str8Lit(""),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("cancel"),
        Str8Lit("Cancel"),
        Str8Lit("Rejects the changes, closes a menu, or gives the answer no."),
        Str8Lit(""),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("move_left"),
        Str8Lit("Move Left"),
        Str8Lit("Moves the cursor left."),
        Str8Lit(""),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("move_right"),
        Str8Lit("Move Right"),
        Str8Lit("Moves the cursor right."),
        Str8Lit(""),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("move_up"),
        Str8Lit("Move Up"),
        Str8Lit("Moves the cursor up."),
        Str8Lit(""),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("move_down"),
        Str8Lit("Move Down"),
        Str8Lit("Moves the cursor down."),
        Str8Lit(""),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("move_left_select"),
        Str8Lit("Move Left Select"),
        Str8Lit("Moves the cursor left and selects."),
        Str8Lit(""),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("move_right_select"),
        Str8Lit("Move Right Select"),
        Str8Lit("Moves the cursor right and selects."),
        Str8Lit(""),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("move_up_select"),
        Str8Lit("Move Up Select"),
        Str8Lit("Moves the cursor up and selects."),
        Str8Lit(""),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("move_down_select"),
        Str8Lit("Move Down Select"),
        Str8Lit("Moves the cursor down and selects."),
        Str8Lit(""),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("move_left_chunk"),
        Str8Lit("Move Left Chunk"),
        Str8Lit("Moves the cursor left one word."),
        Str8Lit(""),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("move_right_chunk"),
        Str8Lit("Move Right Chunk"),
        Str8Lit("Moves the cursor right one word."),
        Str8Lit(""),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("move_up_chunk"),
        Str8Lit("Move Up Chunk"),
        Str8Lit("Moves the cursor up one group."),
        Str8Lit(""),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("move_down_chunk"),
        Str8Lit("Move Down Chunk"),
        Str8Lit("Moves the cursor down one group."),
        Str8Lit(""),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("move_left_chunk_select"),
        Str8Lit("Move Left Chunk Select"),
        Str8Lit("Moves the cursor left one word and selects."),
        Str8Lit(""),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("move_right_chunk_select"),
        Str8Lit("Move Right Chunk Select"),
        Str8Lit("Moves the cursor right one word and selects."),
        Str8Lit(""),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("move_up_chunk_select"),
        Str8Lit("Move Up Chunk Select"),
        Str8Lit("Moves the cursor up one group and selects."),
        Str8Lit(""),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("move_down_chunk_select"),
        Str8Lit("Move Down Chunk Select"),
        Str8Lit("Moves the cursor down one group and selects."),
        Str8Lit(""),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("move_up_page"),
        Str8Lit("Move Up Page"),
        Str8Lit("Moves the cursor up one page."),
        Str8Lit(""),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("move_down_page"),
        Str8Lit("Move Down Page"),
        Str8Lit("Moves the cursor down one page."),
        Str8Lit(""),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("move_up_whole"),
        Str8Lit("Move To Start"),
        Str8Lit("Moves the cursor to the first row."),
        Str8Lit(""),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("move_down_whole"),
        Str8Lit("Move To End"),
        Str8Lit("Moves the cursor to the last row."),
        Str8Lit(""),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("move_up_page_select"),
        Str8Lit("Move Up Page Select"),
        Str8Lit("Moves the cursor up one page and selects."),
        Str8Lit(""),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("move_down_page_select"),
        Str8Lit("Move Down Page Select"),
        Str8Lit("Moves the cursor down one page and selects."),
        Str8Lit(""),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("move_up_whole_select"),
        Str8Lit("Move To Start Select"),
        Str8Lit("Moves the cursor to the first row and selects."),
        Str8Lit(""),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("move_down_whole_select"),
        Str8Lit("Move To End Select"),
        Str8Lit("Moves the cursor to the last row and selects."),
        Str8Lit(""),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("move_home"),
        Str8Lit("Move Home"),
        Str8Lit("Moves the cursor to the start of the line."),
        Str8Lit(""),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("move_end"),
        Str8Lit("Move End"),
        Str8Lit("Moves the cursor to the end of the line."),
        Str8Lit(""),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("move_home_select"),
        Str8Lit("Move Home Select"),
        Str8Lit("Moves the cursor to the start of the line and selects."),
        Str8Lit(""),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("move_end_select"),
        Str8Lit("Move End Select"),
        Str8Lit("Moves the cursor to the end of the line and selects."),
        Str8Lit(""),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("select_all"),
        Str8Lit("Select All"),
        Str8Lit("Selects all items."),
        Str8Lit(""),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("delete_single"),
        Str8Lit("Delete"),
        Str8Lit("Deletes the character to the right of the cursor, or the selection."),
        Str8Lit(""),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("delete_chunk"),
        Str8Lit("Delete Word"),
        Str8Lit("Deletes the word to the right of the cursor, or the selection."),
        Str8Lit(""),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("backspace_single"),
        Str8Lit("Backspace"),
        Str8Lit("Deletes the character to the left of the cursor, or the selection."),
        Str8Lit(""),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("backspace_chunk"),
        Str8Lit("Backspace Word"),
        Str8Lit("Deletes the word to the left of the cursor, or the selection."),
        Str8Lit(""),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("copy"),
        Str8Lit("Copy"),
        Str8Lit("Copies the selection to the clipboard."),
        Str8Lit("csv,clipboard"),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI | APP_COMMAND_KIND_FLAG_LIST_IN_PACKET,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("cut"),
        Str8Lit("Cut"),
        Str8Lit("Copies the selection to the clipboard, then deletes the selection."),
        Str8Lit(""),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("paste"),
        Str8Lit("Paste"),
        Str8Lit("Puts the contents of the clipboard at the cursor."),
        Str8Lit(""),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("insert_text"),
        Str8Lit("Insert Text"),
        Str8Lit("Puts the text that caused this command at the cursor."),
        Str8Lit(""),
        0,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("move_next"),
        Str8Lit("Move Next"),
        Str8Lit("Moves the cursor to the next item."),
        Str8Lit(""),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("move_prev"),
        Str8Lit("Move Previous"),
        Str8Lit("Moves the cursor to the previous item."),
        Str8Lit(""),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("goto_offset"),
        Str8Lit("Go To Offset"),
        Str8Lit("Goes to the packet at a file offset. The offset is hexadecimal."),
        Str8Lit("jump,seek,address"),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI | APP_COMMAND_KIND_FLAG_LIST_IN_FILE,
        APP_ICON_KIND_RIGHT_ARROW,
        {
            APP_QUERY_KIND_REQUIRED,
            APP_REGISTERS_SLOT_OFFSET,
            Str8Lit("")
        }
    },
    {
        Str8Lit("goto_time"),
        Str8Lit("Go To Time"),
        Str8Lit("Goes to the first packet at or after a time. The time is in seconds from the start of the file."),
        Str8Lit("jump,seek,timestamp"),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI | APP_COMMAND_KIND_FLAG_LIST_IN_FILE,
        APP_ICON_KIND_RIGHT_ARROW,
        {
            APP_QUERY_KIND_REQUIRED,
            APP_REGISTERS_SLOT_STRING,
            Str8Lit("")
        }
    },
    {
        Str8Lit("search"),
        Str8Lit("Search"),
        Str8Lit("Finds the next row that contains the text."),
        Str8Lit("find"),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_FIND,
        {
            APP_QUERY_KIND_REQUIRED | APP_QUERY_KIND_KEEP_OLD_INPUT | APP_QUERY_KIND_SELECT_OLD_INPUT,
            APP_REGISTERS_SLOT_STRING,
            Str8Lit("")
        }
    },
    {
        Str8Lit("search_backwards"),
        Str8Lit("Search Backwards"),
        Str8Lit("Finds the previous row that contains the text."),
        Str8Lit("find"),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_FIND,
        {
            APP_QUERY_KIND_REQUIRED | APP_QUERY_KIND_KEEP_OLD_INPUT | APP_QUERY_KIND_SELECT_OLD_INPUT,
            APP_REGISTERS_SLOT_STRING,
            Str8Lit("")
        }
    },
    {
        Str8Lit("find_next"),
        Str8Lit("Find Next"),
        Str8Lit("Finds the next row that contains the text of the last search."),
        Str8Lit("search"),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_NULL,
        {
            APP_QUERY_KIND_KEEP_OLD_INPUT,
            APP_REGISTERS_SLOT_NULL,
            Str8Lit("")
        }
    },
    {
        Str8Lit("find_prev"),
        Str8Lit("Find Previous"),
        Str8Lit("Finds the previous row that contains the text of the last search."),
        Str8Lit("search"),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_NULL,
        {
            APP_QUERY_KIND_KEEP_OLD_INPUT,
            APP_REGISTERS_SLOT_NULL,
            Str8Lit("")
        }
    },
    {
        Str8Lit("filter"),
        Str8Lit("Filter"),
        Str8Lit("Shows only the rows that agree with the filter."),
        Str8Lit("sys,comp,msg,system,component,message"),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_FIND,
        {
            APP_QUERY_KIND_REQUIRED | APP_QUERY_KIND_KEEP_OLD_INPUT,
            APP_REGISTERS_SLOT_STRING,
            Str8Lit("")
        }
    },
    {
        Str8Lit("extract"),
        Str8Lit("Extract"),
        Str8Lit("Writes the selected bytes of the tlog file to a new file."),
        Str8Lit("write,save,export"),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI | APP_COMMAND_KIND_FLAG_LIST_IN_PACKET | APP_COMMAND_KIND_FLAG_LIST_IN_FILE,
        APP_ICON_KIND_FILE,
        {
            APP_QUERY_KIND_ALLOW_FILES | APP_QUERY_KIND_FLOATING | APP_QUERY_KIND_REQUIRED,
            APP_REGISTERS_SLOT_FILE_PATH,
            Str8Lit("")
        }
    },
    {
        Str8Lit("set_theme"),
        Str8Lit("Set Theme"),
        Str8Lit("Changes the colours of the program."),
        Str8Lit("colour,color,style"),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_NULL,
        {
            APP_QUERY_KIND_FLOATING | APP_QUERY_KIND_REQUIRED,
            APP_REGISTERS_SLOT_STRING,
            Str8Lit("themes")
        }
    },
    {
        Str8Lit("toggle_dev_menu"),
        Str8Lit("Developer Menu"),
        Str8Lit("Shows or hides the developer menu."),
        Str8Lit("debug"),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("push_query"),
        Str8Lit("Push Query"),
        Str8Lit("Opens an input line for a command."),
        Str8Lit(""),
        0,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("complete_query"),
        Str8Lit("Complete Query"),
        Str8Lit("Runs the command of the input line and closes the input line."),
        Str8Lit(""),
        0,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("cancel_query"),
        Str8Lit("Cancel Query"),
        Str8Lit("Closes the input line."),
        Str8Lit(""),
        0,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("update_query"),
        Str8Lit("Update Query"),
        Str8Lit("Changes the text of the input line."),
        Str8Lit(""),
        0,
        APP_ICON_KIND_NULL
    },
    {
        Str8Lit("files"),
        Str8Lit("Files"),
        Str8Lit("Opens a tab that shows the list of files."),
        Str8Lit("logs,tlog"),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI,
        APP_ICON_KIND_FOLDER
    },
    {
        Str8Lit("report"),
        Str8Lit("Report"),
        Str8Lit("Opens a tab that shows the report for the file."),
        Str8Lit("summary,totals"),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI | APP_COMMAND_KIND_FLAG_LIST_IN_FILE,
        APP_ICON_KIND_FILE
    },
    {
        Str8Lit("incorrect_packets"),
        Str8Lit("Incorrect Packets"),
        Str8Lit("Opens a tab that shows the incorrect packets of the file."),
        Str8Lit("bad,crc,corrupt"),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI | APP_COMMAND_KIND_FLAG_LIST_IN_FILE,
        APP_ICON_KIND_WARNING
    },
    {
        Str8Lit("lost_packets"),
        Str8Lit("Lost Packets"),
        Str8Lit("Opens a tab that shows the packets that an incorrect packet hid."),
        Str8Lit("hidden"),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI | APP_COMMAND_KIND_FLAG_LIST_IN_FILE,
        APP_ICON_KIND_WARNING
    },
    {
        Str8Lit("all_packets"),
        Str8Lit("All Packets"),
        Str8Lit("Opens a tab that shows all packets of the file."),
        Str8Lit("dump"),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI | APP_COMMAND_KIND_FLAG_LIST_IN_FILE,
        APP_ICON_KIND_FILE
    },
    {
        Str8Lit("messages"),
        Str8Lit("Messages"),
        Str8Lit("Opens a tab that shows the packet count for each message."),
        Str8Lit("msg"),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI | APP_COMMAND_KIND_FLAG_LIST_IN_FILE,
        APP_ICON_KIND_FILE
    },
    {
        Str8Lit("sources"),
        Str8Lit("Sources"),
        Str8Lit("Opens a tab that shows each system and component in the file."),
        Str8Lit("system,component,sysid"),
        APP_COMMAND_KIND_FLAG_LIST_IN_UI | APP_COMMAND_KIND_FLAG_LIST_IN_FILE,
        APP_ICON_KIND_FILE
    }
};

BindingTableEntry APP_DEFAULT_BINDING_TABLE[82] = {
    {
        Str8Lit("exit"),
        {
            INPUT_KIND_Q,
            INPUT_MOD_KIND_CTRL
        }
    },
    {
        Str8Lit("next_panel"),
        {
            INPUT_KIND_COMMA,
            INPUT_MOD_KIND_CTRL
        }
    },
    {
        Str8Lit("prev_panel"),
        {
            INPUT_KIND_COMMA,
            INPUT_MOD_KIND_CTRL | INPUT_MOD_KIND_SHIFT
        }
    },
    {
        Str8Lit("focus_panel_right"),
        {
            INPUT_KIND_RIGHT,
            INPUT_MOD_KIND_CTRL | INPUT_MOD_KIND_ALT
        }
    },
    {
        Str8Lit("focus_panel_left"),
        {
            INPUT_KIND_LEFT,
            INPUT_MOD_KIND_CTRL | INPUT_MOD_KIND_ALT
        }
    },
    {
        Str8Lit("focus_panel_up"),
        {
            INPUT_KIND_UP,
            INPUT_MOD_KIND_CTRL | INPUT_MOD_KIND_ALT
        }
    },
    {
        Str8Lit("focus_panel_down"),
        {
            INPUT_KIND_DOWN,
            INPUT_MOD_KIND_CTRL | INPUT_MOD_KIND_ALT
        }
    },
    {
        Str8Lit("go_back"),
        {
            INPUT_KIND_LEFT,
            INPUT_MOD_KIND_ALT
        }
    },
    {
        Str8Lit("go_forward"),
        {
            INPUT_KIND_RIGHT,
            INPUT_MOD_KIND_ALT
        }
    },
    {
        Str8Lit("next_tab"),
        {
            INPUT_KIND_PAGEDOWN,
            INPUT_MOD_KIND_CTRL
        }
    },
    {
        Str8Lit("prev_tab"),
        {
            INPUT_KIND_PAGEUP,
            INPUT_MOD_KIND_CTRL
        }
    },
    {
        Str8Lit("move_tab_right"),
        {
            INPUT_KIND_PAGEDOWN,
            INPUT_MOD_KIND_CTRL | INPUT_MOD_KIND_SHIFT
        }
    },
    {
        Str8Lit("move_tab_left"),
        {
            INPUT_KIND_PAGEUP,
            INPUT_MOD_KIND_CTRL | INPUT_MOD_KIND_SHIFT
        }
    },
    {
        Str8Lit("close_tab"),
        {
            INPUT_KIND_W,
            INPUT_MOD_KIND_CTRL
        }
    },
    {
        Str8Lit("new_panel_right"),
        {
            INPUT_KIND_P,
            INPUT_MOD_KIND_CTRL
        }
    },
    {
        Str8Lit("new_panel_down"),
        {
            INPUT_KIND_MINUS,
            INPUT_MOD_KIND_CTRL
        }
    },
    {
        Str8Lit("close_panel"),
        {
            INPUT_KIND_P,
            (
                INPUT_MOD_KIND_CTRL |
                INPUT_MOD_KIND_SHIFT |
                INPUT_MOD_KIND_ALT
            )
        }
    },
    {
        Str8Lit("open_tab"),
        {
            INPUT_KIND_T,
            INPUT_MOD_KIND_CTRL
        }
    },
    {
        Str8Lit("open"),
        {
            INPUT_KIND_O,
            INPUT_MOD_KIND_CTRL
        }
    },
    {
        Str8Lit("edit"),
        {
            INPUT_KIND_FUNC_2,
            0
        }
    },
    {
        Str8Lit("accept"),
        {
            INPUT_KIND_RETURN,
            0
        }
    },
    {
        Str8Lit("accept"),
        {
            INPUT_KIND_SPACE,
            0
        }
    },
    {
        Str8Lit("cancel"),
        {
            INPUT_KIND_ESC,
            0
        }
    },
    {
        Str8Lit("move_left"),
        {
            INPUT_KIND_LEFT,
            0
        }
    },
    {
        Str8Lit("move_right"),
        {
            INPUT_KIND_RIGHT,
            0
        }
    },
    {
        Str8Lit("move_up"),
        {
            INPUT_KIND_UP,
            0
        }
    },
    {
        Str8Lit("move_down"),
        {
            INPUT_KIND_DOWN,
            0
        }
    },
    {
        Str8Lit("move_left_select"),
        {
            INPUT_KIND_LEFT,
            INPUT_MOD_KIND_SHIFT
        }
    },
    {
        Str8Lit("move_right_select"),
        {
            INPUT_KIND_RIGHT,
            INPUT_MOD_KIND_SHIFT
        }
    },
    {
        Str8Lit("move_up_select"),
        {
            INPUT_KIND_UP,
            INPUT_MOD_KIND_SHIFT
        }
    },
    {
        Str8Lit("move_down_select"),
        {
            INPUT_KIND_DOWN,
            INPUT_MOD_KIND_SHIFT
        }
    },
    {
        Str8Lit("move_left_chunk"),
        {
            INPUT_KIND_LEFT,
            INPUT_MOD_KIND_CTRL
        }
    },
    {
        Str8Lit("move_right_chunk"),
        {
            INPUT_KIND_RIGHT,
            INPUT_MOD_KIND_CTRL
        }
    },
    {
        Str8Lit("move_up_chunk"),
        {
            INPUT_KIND_UP,
            INPUT_MOD_KIND_CTRL
        }
    },
    {
        Str8Lit("move_down_chunk"),
        {
            INPUT_KIND_DOWN,
            INPUT_MOD_KIND_CTRL
        }
    },
    {
        Str8Lit("move_left_chunk_select"),
        {
            INPUT_KIND_LEFT,
            INPUT_MOD_KIND_CTRL | INPUT_MOD_KIND_SHIFT
        }
    },
    {
        Str8Lit("move_right_chunk_select"),
        {
            INPUT_KIND_RIGHT,
            INPUT_MOD_KIND_CTRL | INPUT_MOD_KIND_SHIFT
        }
    },
    {
        Str8Lit("move_up_chunk_select"),
        {
            INPUT_KIND_UP,
            INPUT_MOD_KIND_CTRL | INPUT_MOD_KIND_SHIFT
        }
    },
    {
        Str8Lit("move_down_chunk_select"),
        {
            INPUT_KIND_DOWN,
            INPUT_MOD_KIND_CTRL | INPUT_MOD_KIND_SHIFT
        }
    },
    {
        Str8Lit("move_up_page"),
        {
            INPUT_KIND_PAGEUP,
            0
        }
    },
    {
        Str8Lit("move_down_page"),
        {
            INPUT_KIND_PAGEDOWN,
            0
        }
    },
    {
        Str8Lit("move_up_whole"),
        {
            INPUT_KIND_HOME,
            INPUT_MOD_KIND_CTRL
        }
    },
    {
        Str8Lit("move_down_whole"),
        {
            INPUT_KIND_END,
            INPUT_MOD_KIND_CTRL
        }
    },
    {
        Str8Lit("move_up_page_select"),
        {
            INPUT_KIND_PAGEUP,
            INPUT_MOD_KIND_SHIFT
        }
    },
    {
        Str8Lit("move_down_page_select"),
        {
            INPUT_KIND_PAGEDOWN,
            INPUT_MOD_KIND_SHIFT
        }
    },
    {
        Str8Lit("move_up_whole_select"),
        {
            INPUT_KIND_HOME,
            INPUT_MOD_KIND_CTRL | INPUT_MOD_KIND_SHIFT
        }
    },
    {
        Str8Lit("move_down_whole_select"),
        {
            INPUT_KIND_END,
            INPUT_MOD_KIND_CTRL | INPUT_MOD_KIND_SHIFT
        }
    },
    {
        Str8Lit("move_home"),
        {
            INPUT_KIND_HOME,
            0
        }
    },
    {
        Str8Lit("move_end"),
        {
            INPUT_KIND_END,
            0
        }
    },
    {
        Str8Lit("move_home_select"),
        {
            INPUT_KIND_HOME,
            INPUT_MOD_KIND_SHIFT
        }
    },
    {
        Str8Lit("move_end_select"),
        {
            INPUT_KIND_END,
            INPUT_MOD_KIND_SHIFT
        }
    },
    {
        Str8Lit("select_all"),
        {
            INPUT_KIND_A,
            INPUT_MOD_KIND_CTRL
        }
    },
    {
        Str8Lit("delete_single"),
        {
            INPUT_KIND_DELETE,
            0
        }
    },
    {
        Str8Lit("delete_chunk"),
        {
            INPUT_KIND_DELETE,
            INPUT_MOD_KIND_CTRL
        }
    },
    {
        Str8Lit("backspace_single"),
        {
            INPUT_KIND_BACKSPACE,
            0
        }
    },
    {
        Str8Lit("backspace_chunk"),
        {
            INPUT_KIND_BACKSPACE,
            INPUT_MOD_KIND_CTRL
        }
    },
    {
        Str8Lit("copy"),
        {
            INPUT_KIND_C,
            INPUT_MOD_KIND_CTRL
        }
    },
    {
        Str8Lit("copy"),
        {
            INPUT_KIND_INSERT,
            INPUT_MOD_KIND_CTRL
        }
    },
    {
        Str8Lit("cut"),
        {
            INPUT_KIND_X,
            INPUT_MOD_KIND_CTRL
        }
    },
    {
        Str8Lit("paste"),
        {
            INPUT_KIND_V,
            INPUT_MOD_KIND_CTRL
        }
    },
    {
        Str8Lit("paste"),
        {
            INPUT_KIND_INSERT,
            INPUT_MOD_KIND_SHIFT
        }
    },
    {
        Str8Lit("insert_text"),
        {
            INPUT_KIND_NULL,
            0
        }
    },
    {
        Str8Lit("move_next"),
        {
            INPUT_KIND_TAB,
            0
        }
    },
    {
        Str8Lit("move_prev"),
        {
            INPUT_KIND_TAB,
            INPUT_MOD_KIND_SHIFT
        }
    },
    {
        Str8Lit("goto_offset"),
        {
            INPUT_KIND_G,
            INPUT_MOD_KIND_CTRL
        }
    },
    {
        Str8Lit("goto_time"),
        {
            INPUT_KIND_G,
            INPUT_MOD_KIND_CTRL | INPUT_MOD_KIND_SHIFT
        }
    },
    {
        Str8Lit("search"),
        {
            INPUT_KIND_F,
            INPUT_MOD_KIND_CTRL
        }
    },
    {
        Str8Lit("search_backwards"),
        {
            INPUT_KIND_R,
            INPUT_MOD_KIND_CTRL
        }
    },
    {
        Str8Lit("find_next"),
        {
            INPUT_KIND_FUNC_3,
            0
        }
    },
    {
        Str8Lit("find_prev"),
        {
            INPUT_KIND_FUNC_3,
            INPUT_MOD_KIND_CTRL
        }
    },
    {
        Str8Lit("filter"),
        {
            INPUT_KIND_L,
            INPUT_MOD_KIND_CTRL
        }
    },
    {
        Str8Lit("extract"),
        {
            INPUT_KIND_E,
            INPUT_MOD_KIND_CTRL
        }
    },
    {
        Str8Lit("open_palette"),
        {
            INPUT_KIND_FUNC_1,
            0
        }
    },
    {
        Str8Lit("open_palette"),
        {
            INPUT_KIND_P,
            INPUT_MOD_KIND_CTRL | INPUT_MOD_KIND_SHIFT
        }
    },
    {
        Str8Lit("toggle_dev_menu"),
        {
            INPUT_KIND_D,
            INPUT_MOD_KIND_CTRL | INPUT_MOD_KIND_SHIFT | INPUT_MOD_KIND_ALT
        }
    },
    {
        Str8Lit("files"),
        {
            INPUT_KIND_1,
            INPUT_MOD_KIND_ALT
        }
    },
    {
        Str8Lit("report"),
        {
            INPUT_KIND_2,
            INPUT_MOD_KIND_ALT
        }
    },
    {
        Str8Lit("incorrect_packets"),
        {
            INPUT_KIND_3,
            INPUT_MOD_KIND_ALT
        }
    },
    {
        Str8Lit("lost_packets"),
        {
            INPUT_KIND_4,
            INPUT_MOD_KIND_ALT
        }
    },
    {
        Str8Lit("all_packets"),
        {
            INPUT_KIND_5,
            INPUT_MOD_KIND_ALT
        }
    },
    {
        Str8Lit("messages"),
        {
            INPUT_KIND_6,
            INPUT_MOD_KIND_ALT
        }
    },
    {
        Str8Lit("sources"),
        {
            INPUT_KIND_7,
            INPUT_MOD_KIND_ALT
        }
    }
};

Str8 APP_ICON_KIND_TEXT_TABLE[APP_ICON_KIND_COUNT] = {
    Str8Lit(""),
    Str8Lit("\xE2\x96\xB8"),
    Str8Lit("\xE2\x97\x8B"),
    Str8Lit("i"),
    Str8Lit("!"),
    Str8Lit("\xE2\x86\x90"),
    Str8Lit("\xE2\x86\x92"),
    Str8Lit("\xE2\x86\x91"),
    Str8Lit("\xE2\x86\x93"),
    Str8Lit("\xE2\x97\x82"),
    Str8Lit("\xE2\x96\xB8"),
    Str8Lit("\xE2\x96\xB2"),
    Str8Lit("\xE2\x96\xBC"),
    Str8Lit("[ ]"),
    Str8Lit("[x]"),
    Str8Lit("( )"),
    Str8Lit("(\xE2\x80\xA2)"),
    Str8Lit("+"),
    Str8Lit("x"),
    Str8Lit("/"),
    Str8Lit("\xE2\x80\xA2")
};

Str8 APP_THEME_PRESET_DISPLAY_STR_TABLE[APP_THEME_PRESET_COUNT] = {
    Str8Lit("Ayu Dark"),
    Str8Lit("Default Dark"),
    Str8Lit("Default Light")
};

Str8 APP_THEME_PRESET_CODE_STR_TABLE[APP_THEME_PRESET_COUNT] = {
    Str8Lit("ayu_dark"),
    Str8Lit("default_dark"),
    Str8Lit("default_light")
};

Str8 APP_THEME_PRESET_CONFIG_STR_TABLE[APP_THEME_PRESET_COUNT] = {
    Str8Lit(
        "theme:\n{\n"
        "  theme_colour:{tags: background, value: 0x0a0e14ff}\n"
        "  theme_colour:{tags: text, value: 0xb3b1adff}\n"
        "  theme_colour:{tags: \"weak text\", value: 0x5c6773ff}\n"
        "  theme_colour:{tags: \"good text\", value: 0xc2d94cff}\n"
        "  theme_colour:{tags: \"bad text\", value: 0xf07178ff}\n"
        "  theme_colour:{tags: border, value: 0x393f4cff}\n"
        "  theme_colour:{tags: hover, value: 0xe6e4e1ff}\n"
        "  theme_colour:{tags: focus, value: 0xffb454ff}\n"
        "  theme_colour:{tags: overlay, value: 0x00000080}\n"
        "  theme_colour:{tags: fuzzy_match, value: 0x59c2ffff}\n"
        "  theme_colour:{tags: accent, value: 0xffb454ff}\n"
        "  theme_colour:{tags: selection, value: 0x59c2ff40}\n"
        "  theme_colour:{tags: cursor, value: 0xffb454ff}\n"
        "  theme_colour:{tags: \"alt background\", value: 0x131721ff}\n"
        "  theme_colour:{tags: \"pop background\", value: 0x27374eff}\n"
        "  theme_colour:{tags: \"pop text\", value: 0xe6e4e1ff}\n"
        "  theme_colour:{tags: \"pop weak text\", value: 0x8a9199ff}\n"
        "  theme_colour:{tags: \"floating background\", value: 0x131721ff}\n"
        "  theme_colour:{tags: \"floating text\", value: 0xb3b1adff}\n"
        "  theme_colour:{tags: \"floating border\", value: 0x393f4cff}\n"
        "  theme_colour:{tags: \"floating hover\", value: 0xe6e4e1ff}\n"
        "  theme_colour:{tags: \"floating weak text\", value: 0x5c6773ff}\n"
        "  theme_colour:{tags: \"floating fuzzy_match\", value: 0x59c2ffff}\n"
        "  theme_colour:{tags: \"floating alt background\", value: 0x0a0e14ff}\n"
        "  theme_colour:{tags: \"floating pop background\", value: 0x27374eff}\n"
        "  theme_colour:{tags: \"floating pop text\", value: 0xe6e4e1ff}\n"
        "  theme_colour:{tags: \"menu_bar background\", value: 0x131721ff}\n"
        "  theme_colour:{tags: \"menu_bar text\", value: 0xb3b1adff}\n"
        "  theme_colour:{tags: \"menu_bar hover\", value: 0xe6e4e1ff}\n"
        "  theme_colour:{tags: \"menu_bar weak text\", value: 0x5c6773ff}\n"
        "  theme_colour:{tags: \"menu_bar bad text\", value: 0xf07178ff}\n"
        "  theme_colour:{tags: \"menu_bar alt background\", value: 0x1f2430ff}\n"
        "  theme_colour:{tags: \"tab background\", value: 0xffb454ff}\n"
        "  theme_colour:{tags: \"tab text\", value: 0x0a0e14ff}\n"
        "  theme_colour:{tags: \"tab inactive background\", value: 0x131721ff}\n"
        "  theme_colour:{tags: \"tab inactive text\", value: 0xb3b1adff}\n"
        "  theme_colour:{tags: \"tab unfocused background\", value: 0x27374eff}\n"
        "  theme_colour:{tags: \"tab unfocused text\", value: 0xe6e4e1ff}\n"
        "  theme_colour:{tags: \"scroll_bar background\", value: 0x1f2430ff}\n"
        "  theme_colour:{tags: \"scroll_bar accent\", value: 0xffb454ff}\n"
        "  theme_colour:{tags: \"drop_site background\", value: 0xffb45440}\n"
        "}\n"
    ),
    Str8Lit(
        "theme:\n{\n"
        "  theme_colour:{tags: background, value: 0x1b1b1bff}\n"
        "  theme_colour:{tags: text, value: 0xe5e5e5ff}\n"
        "  theme_colour:{tags: \"weak text\", value: 0xa4a4a4ff}\n"
        "  theme_colour:{tags: \"good text\", value: 0x32a852ff}\n"
        "  theme_colour:{tags: \"bad text\", value: 0xe5534bff}\n"
        "  theme_colour:{tags: border, value: 0x404040ff}\n"
        "  theme_colour:{tags: hover, value: 0xffffffff}\n"
        "  theme_colour:{tags: focus, value: 0x3a90bbff}\n"
        "  theme_colour:{tags: overlay, value: 0x00000080}\n"
        "  theme_colour:{tags: fuzzy_match, value: 0x3a90bbff}\n"
        "  theme_colour:{tags: accent, value: 0x355b6eff}\n"
        "  theme_colour:{tags: selection, value: 0xffffff30}\n"
        "  theme_colour:{tags: cursor, value: 0xffffffff}\n"
        "  theme_colour:{tags: \"alt background\", value: 0x2b2b2bff}\n"
        "  theme_colour:{tags: \"pop background\", value: 0x355b6eff}\n"
        "  theme_colour:{tags: \"pop text\", value: 0xffffffff}\n"
        "  theme_colour:{tags: \"floating background\", value: 0x222222ff}\n"
        "  theme_colour:{tags: \"floating text\", value: 0xe5e5e5ff}\n"
        "  theme_colour:{tags: \"floating border\", value: 0x404040ff}\n"
        "  theme_colour:{tags: \"floating hover\", value: 0xffffffff}\n"
        "  theme_colour:{tags: \"floating weak text\", value: 0xa4a4a4ff}\n"
        "  theme_colour:{tags: \"floating fuzzy_match\", value: 0x3a90bbff}\n"
        "  theme_colour:{tags: \"floating alt background\", value: 0x1b1b1bff}\n"
        "  theme_colour:{tags: \"floating pop background\", value: 0x355b6eff}\n"
        "  theme_colour:{tags: \"floating pop text\", value: 0xffffffff}\n"
        "  theme_colour:{tags: \"menu_bar background\", value: 0x222222ff}\n"
        "  theme_colour:{tags: \"menu_bar text\", value: 0xe5e5e5ff}\n"
        "  theme_colour:{tags: \"menu_bar hover\", value: 0xffffffff}\n"
        "  theme_colour:{tags: \"menu_bar weak text\", value: 0xa4a4a4ff}\n"
        "  theme_colour:{tags: \"menu_bar bad text\", value: 0xe5534bff}\n"
        "  theme_colour:{tags: \"tab background\", value: 0x355b6eff}\n"
        "  theme_colour:{tags: \"tab text\", value: 0xffffffff}\n"
        "  theme_colour:{tags: \"tab inactive background\", value: 0x222222ff}\n"
        "  theme_colour:{tags: \"tab inactive text\", value: 0xa4a4a4ff}\n"
        "  theme_colour:{tags: \"scroll_bar background\", value: 0x222222ff}\n"
        "  theme_colour:{tags: \"scroll_bar accent\", value: 0xa4a4a4ff}\n"
        "  theme_colour:{tags: \"menu_bar alt background\", value: 0x333333ff}\n"
        "  theme_colour:{tags: \"tab unfocused background\", value: 0x2c3e48ff}\n"
        "  theme_colour:{tags: \"tab unfocused text\", value: 0xe5e5e5ff}\n"
        "  theme_colour:{tags: \"drop_site background\", value: 0x3a90bb40}\n"
        "}\n"
    ),
    Str8Lit(
        "theme:\n{\n"
        "  theme_colour:{tags: background, value: 0xffffffff}\n"
        "  theme_colour:{tags: text, value: 0x000000ff}\n"
        "  theme_colour:{tags: \"weak text\", value: 0x727272ff}\n"
        "  theme_colour:{tags: \"good text\", value: 0x217538ff}\n"
        "  theme_colour:{tags: \"bad text\", value: 0xb3261eff}\n"
        "  theme_colour:{tags: border, value: 0xcbcbcbff}\n"
        "  theme_colour:{tags: hover, value: 0x000000ff}\n"
        "  theme_colour:{tags: focus, value: 0x1a5b7cff}\n"
        "  theme_colour:{tags: overlay, value: 0x00000040}\n"
        "  theme_colour:{tags: fuzzy_match, value: 0x1a5b7cff}\n"
        "  theme_colour:{tags: accent, value: 0x5aabd9ff}\n"
        "  theme_colour:{tags: selection, value: 0x00000030}\n"
        "  theme_colour:{tags: cursor, value: 0x000000ff}\n"
        "  theme_colour:{tags: \"alt background\", value: 0xefefefff}\n"
        "  theme_colour:{tags: \"pop background\", value: 0xcbe4f2ff}\n"
        "  theme_colour:{tags: \"pop text\", value: 0x000000ff}\n"
        "  theme_colour:{tags: \"floating background\", value: 0xf8f8f8ff}\n"
        "  theme_colour:{tags: \"floating text\", value: 0x000000ff}\n"
        "  theme_colour:{tags: \"floating border\", value: 0xcbcbcbff}\n"
        "  theme_colour:{tags: \"floating hover\", value: 0x000000ff}\n"
        "  theme_colour:{tags: \"floating weak text\", value: 0x727272ff}\n"
        "  theme_colour:{tags: \"floating fuzzy_match\", value: 0x1a5b7cff}\n"
        "  theme_colour:{tags: \"floating alt background\", value: 0xffffffff}\n"
        "  theme_colour:{tags: \"floating pop background\", value: 0xcbe4f2ff}\n"
        "  theme_colour:{tags: \"floating pop text\", value: 0x000000ff}\n"
        "  theme_colour:{tags: \"menu_bar background\", value: 0x5aabd9ff}\n"
        "  theme_colour:{tags: \"menu_bar text\", value: 0x000000ff}\n"
        "  theme_colour:{tags: \"menu_bar hover\", value: 0x000000ff}\n"
        "  theme_colour:{tags: \"menu_bar weak text\", value: 0x1a3a4cff}\n"
        "  theme_colour:{tags: \"menu_bar bad text\", value: 0x7a0c06ff}\n"
        "  theme_colour:{tags: \"tab background\", value: 0xcbe4f2ff}\n"
        "  theme_colour:{tags: \"tab text\", value: 0x000000ff}\n"
        "  theme_colour:{tags: \"tab inactive background\", value: 0xf8f8f8ff}\n"
        "  theme_colour:{tags: \"tab inactive text\", value: 0x727272ff}\n"
        "  theme_colour:{tags: \"scroll_bar background\", value: 0xf8f8f8ff}\n"
        "  theme_colour:{tags: \"scroll_bar accent\", value: 0x727272ff}\n"
        "  theme_colour:{tags: \"menu_bar alt background\", value: 0x8ec6e6ff}\n"
        "  theme_colour:{tags: \"tab unfocused background\", value: 0xe3e3e3ff}\n"
        "  theme_colour:{tags: \"tab unfocused text\", value: 0x000000ff}\n"
        "  theme_colour:{tags: \"drop_site background\", value: 0x1a5b7c40}\n"
        "}\n"
    )
};

Str8 APP_TAB_FAST_PATH_VIEW_NAME_TABLE[APP_COMMAND_KIND_COUNT - APP_COMMAND_KIND_FIRST_TAB_FAST_PATH_CMD] = {
    Str8Lit("files"),
    Str8Lit("report"),
    Str8Lit("incorrect_packets"),
    Str8Lit("lost_packets"),
    Str8Lit("all_packets"),
    Str8Lit("messages"),
    Str8Lit("sources")
};
EXTERN_C_LINK_END

void 
CopyRegisters(Arena* MemPool, AppRegisters* Dst, AppRegisters* Src)
{
    MemCpy(Dst, Src, sizeof(AppRegisters));
    Dst->FilePath = ArenaPushStrCpy(MemPool, Src->FilePath);
    Dst->Expression = ArenaPushStrCpy(MemPool, Src->Expression);
    Dst->String = ArenaPushStrCpy(MemPool, Src->String);
    Dst->CommandName = ArenaPushStrCpy(MemPool, Src->CommandName);
}

AppRegisters* 
CopyRegisters(Arena* MemPool, AppRegisters* Regs)
{
    AppRegisters* Result = ArenaPushArrayZero(MemPool, AppRegisters, 1);

    CopyRegisters(MemPool, Result, Regs);

    return Result;
}

b32
AppDragIsActive(void)
{
    return (
        APP_STATE->DragDropState == APP_DRAG_DROP_STATE_DRAGGING ||
        APP_STATE->DragDropState == APP_DRAG_DROP_STATE_DROPPING
    );
}

void
AppDragBegin(AppRegistersSlot Slot)
{
    if (!AppDragIsActive()) {
        ArenaClear(APP_STATE->DragDropMemPool);
        APP_STATE->DragDropRegisters = CopyRegisters(
            APP_STATE->DragDropMemPool,
            Registers()
        );
        APP_STATE->DragDropRegistersSlot = Slot;
        APP_STATE->DragDropState = APP_DRAG_DROP_STATE_DRAGGING;
    }
}

b32
AppDragDrop(void)
{
    b32 Result = FALSE;

    if (APP_STATE->DragDropState == APP_DRAG_DROP_STATE_DROPPING) {
        Result = TRUE;
        APP_STATE->DragDropState = APP_DRAG_DROP_STATE_NULL;
    }

    return Result;
}

void
AppDragKill(void)
{
    APP_STATE->DragDropState = APP_DRAG_DROP_STATE_NULL;
}

void 
ListPush(Arena* MemPool, AppCommandList* List, Str8 Name, AppRegisters* Regs)
{
    AppCommandNode* Node = ArenaPushArrayZero(MemPool, AppCommandNode, 1);

    Node->Command.Name = ArenaPushStrCpy(MemPool, Name);
    Node->Command.Registers = CopyRegisters(MemPool, Regs);
    DLL_PUSH_BACK(List->Head, List->Tail, Node);
    ++List->Count;
}

void 
ListPush(Arena* MemPool, AppQueryResultList* List, AppQueryResult Entry)
{
    AppQueryResultNode* Node = ArenaPushArrayZero(
        MemPool, 
        AppQueryResultNode, 
        1
    );

    Node->V = Entry;
    DLL_PUSH_BACK(List->Head, List->Tail, Node);
    ++List->Count;
}

Str8 
AppSettingFromNameStr8(Str8 Name)
{
    Str8 Result = {};

    if (Name.Size) {
        ConfigNode* Cfg = EMPTY_CFG_NODE_VALUE;

        Cfg = ConfigNodeFromID(Registers()->View);

        if (Cfg != EMPTY_CFG_NODE_VALUE) {
            for (ConfigNode* C = Cfg; C != EMPTY_CFG_NODE_VALUE; C = C->Parent) {
                ConfigNode* Setting = ConfigNodeChildFromStr(C, Name);

                if (Setting != EMPTY_CFG_NODE_VALUE) {
                    Result = Setting->Head->String;
                    goto END;
                }
            }
        }

        Cfg = ConfigNodeFromID(Registers()->Panel);

        if (Cfg != EMPTY_CFG_NODE_VALUE) {
            for (
                ConfigNode* C = Cfg; 
                C != EMPTY_CFG_NODE_VALUE; 
                C = C->Parent
            ) {
                ConfigNode* Setting = ConfigNodeChildFromStr(C, Name);

                if (Setting != EMPTY_CFG_NODE_VALUE) {
                    Result = Setting->Head->String;
                    goto END;
                }
            }
        }

        {
            ConfigNode* Session = ConfigNodeChildFromStr(
                ConfigNodeRoot(), 
                "session"_s8
            );
            ConfigNode* Settings = ConfigNodeChildFromStr(
                Session, 
                "settings"_s8
            );
            ConfigNode* Setting = ConfigNodeChildFromStr(Settings, Name);

            if (Setting != EMPTY_CFG_NODE_VALUE)
                Result = Setting->Head->String;
        }
    }

END:
    return Result;
}

b32 
AppSettingFromNameB32(Str8 Name)
{
    Str8 Value = AppSettingFromNameStr8(Name);
    b32 Result = (
        Value.Size &&
        (
            U64FromStr(Value) != 0 ||
            StrMatch(Value, "true"_s8, STR_MATCH_ALL_CASES)
        )
    );

    return Result;
}

u64 
AppSettingFromNameU64(Str8 Name)
{
    Str8 Value = AppSettingFromNameStr8(Name);
    u64 Result = Value.Size 
        ? U64FromStr(Value) 
        : 0;

    return Result;
}

f32 
AppSettingFromNameF32(Str8 Name)
{
    Str8 Value = AppSettingFromNameStr8(Name);
    f32 Result = Value.Size 
        ? (f32) F64FromStr(Value) 
        : 0.0f;

    return Result;
}

AppViewState* 
AppViewStateFromConfig(ConfigNode* Config)
{
    AppViewState* Result = EMPTY_APP_VIEW_STATE_VALUE;
    ConfigID ID = Config->ID;

    if (
        ID &&
        ID == APP_STATE->ViewStateLastAccessedID &&
        ID == APP_STATE->ViewStateLastAccessed->CfgID
    ) {
        Result = APP_STATE->ViewStateLastAccessed;
    } else {
        u64 IDHash = Hash(Str8Struct(&ID));
        u64 SlotIndex = IDHash % APP_STATE->ViewStateSlotsCount;
        AppViewStateSlot* Slot = &APP_STATE->ViewStateSlots[SlotIndex];

        for (AppViewState* V = Slot->Head; V; V = V->HashNext) {
            if (V->CfgID == ID) {
                Result = V;
                break;
            }
        }
    }

    if (Result == EMPTY_APP_VIEW_STATE_VALUE) {
        Result = APP_STATE->FreeViewState;

        if (Result)
            SLL_STACK_POP_EX(APP_STATE->FreeViewState, HashNext);
        else
            Result = ArenaPushArrayZero(APP_STATE->MemPool, AppViewState, 1);

        MemCpy(Result, EMPTY_APP_VIEW_STATE_VALUE, sizeof(*Result));

        u64 IDHash = Hash(Str8Struct(&ID));
        u64 SlotIndex = IDHash % APP_STATE->ViewStateSlotsCount;
        AppViewStateSlot* Slot = &APP_STATE->ViewStateSlots[SlotIndex];

        DLL_PUSH_BACK_EX(
            NULL, 
            Slot->Head, 
            Slot->Tail, 
            Result, 
            HashNext, 
            HashPrev
        );
        Result->CfgID = ID;
        Result->MemPool = ArenaAlloc();
        Result->MemPoolResetPosition = ArenaGetPosition(Result->MemPool);
    }

    if (Result != EMPTY_APP_VIEW_STATE_VALUE)
        Result->LastFrameIndexTouched = APP_STATE->FrameIndex;

    APP_STATE->ViewStateLastAccessed = Result;
    APP_STATE->ViewStateLastAccessedID = ID;

    return Result;
}

Arena* 
AppViewMemPool(void)
{
    ConfigNode* View = ConfigNodeFromID(Registers()->View);
    AppViewState* VS = AppViewStateFromConfig(View);

    return VS->MemPool;
}

UIScrollPoint2D 
AppViewScrollPosition(void)
{
    ConfigNode* View = ConfigNodeFromID(Registers()->View);
    AppViewState* VS = AppViewStateFromConfig(View);

    return VS->ScrollPosition;
}

Str8 
AppViewQueryCommand(void)
{
    ConfigNode* View = ConfigNodeFromID(Registers()->View);
    ConfigNode* Query = ConfigNodeChildFromStr(View, "query"_s8);
    ConfigNode* Cmd = ConfigNodeChildFromStr(Query, "cmd"_s8);
    Str8 String = Cmd->Head->String;

    return String;
}

Str8 
AppViewQueryInput(void)
{
    ConfigNode* View = ConfigNodeFromID(Registers()->View);
    ConfigNode* Query = ConfigNodeChildFromStr(View, "query"_s8);
    ConfigNode* Input = ConfigNodeChildFromStr(Query, "input"_s8);
    Str8 String = Input->Head->String;

    return String;
}

Str8 
AppViewFilePath(void)
{
    ConfigNode* View = ConfigNodeFromID(Registers()->View);
    ConfigNode* File = ConfigNodeChildFromStr(View, "file"_s8);
    Str8 String = File->Head->String;

    return String;
}

void* 
AppViewStateFromSize(u64 Size)
{
    ConfigNode* View = ConfigNodeFromID(Registers()->View);
    AppViewState* VS = AppViewStateFromConfig(View);

    if (!VS->UserData)
        VS->UserData = ArenaPushArrayZero(VS->MemPool, u8, Size);

    return VS->UserData;
}

Arena* 
AppPushViewMemPool(void)
{
    ConfigNode* View = ConfigNodeFromID(Registers()->View);
    AppViewState* VS = AppViewStateFromConfig(View);
    AppArenaExt* Ext = ArenaPushArrayZero(VS->MemPool, AppArenaExt, 1);

    Ext->MemPool = ArenaAlloc();
    SLL_QUEUE_PUSH(VS->HeadArenaExt, VS->TailArenaExt, Ext);

    return Ext->MemPool;
}

void 
AppStoreViewLoadingInfo(b32 IsLoading, u64 Progress, u64 ProgressTarget)
{
    ConfigNode* View = ConfigNodeFromID(Registers()->View);
    AppViewState* VS = AppViewStateFromConfig(View);
    b32 LoadingStateIsNew = (
        IsLoading &&
        VS->LoadingTTarget != (f32) !!IsLoading
    );

    VS->LoadingTTarget = (f32) !!IsLoading;
    VS->LoadingProgressV = Progress;
    VS->LoadingProgressVTarget = ProgressTarget;

    if (
        LoadingStateIsNew ||
        VS->LastFrameIndexBuilt + 1 < APP_STATE->FrameIndex
    ) {
        VS->LoadingT = VS->LoadingTTarget;
    }
}

void 
AppStoreViewScrollPosition(UIScrollPoint2D Position)
{
    ConfigNode* View = ConfigNodeFromID(Registers()->View);
    AppViewState* VS = AppViewStateFromConfig(View);

    VS->ScrollPosition = Position;
}

void 
AppStoreViewParameter(Str8 Key, Str8 Value)
{
    ConfigNode* View = ConfigNodeFromID(Registers()->View);
    ConfigNode* Child = ConfigNodeChildFromStrOrAlloc(
        APP_STATE->Config,
        View,
        Key
    );

    ConfigNodeNewReplace(APP_STATE->Config, Child, Value);
}

void 
AppStoreViewParameter(Str8 Key, char* Fmt, ...)
{
    TempArena Scratch = GetScratch(NULL, 0);
    va_list Args;

    va_start(Args, Fmt);

    Str8 String = ArenaPushStrFmtV(
        Scratch.MemPool,
        Fmt,
        Args
    );

    AppStoreViewParameter(Key, String);
    va_end(Args);
    ReleaseScratch(Scratch);
}

void 
UILoadingOverlay(r2f32 Rect, f32 LoadingT, u64 ProgressV, u64 ProgressVTarget)
{
    if (LoadingT >= 0.001f) {
        UIFocus(UI_FOCUS_KIND_OFF) {
            v2f32 RectLength = Length(Rect);
            f32 Width = MIN(30.0f, RectLength.X);
            r2f32 IndicatorRect = Rng(
                FloorF32((RectLength.X - Width) / 2.0f),
                FloorF32(RectLength.Y / 2.0f),
                FloorF32((RectLength.X - Width) / 2.0f) + Width,
                FloorF32(RectLength.Y / 2.0f) + 1.0f
            );
            UIBox* Box = EMPTY_UI_BOX_VALUE;

            UITag("pop"_s8) {
                UIRect(IndicatorRect) {
                    UISetNextChildLayoutAxis(AXIS_2D_X);
                    Box = UIBuildBoxFromStr(
                        (
                            UI_BOX_KIND_DRAW_BACKGROUND |
                            UI_BOX_KIND_FLOATING |
                            UI_BOX_KIND_CLICKABLE
                        ),
                        "###loading_overlay"_s8
                    );
                }

                UIParent(Box) {
                    UIPreferredWidth(UI_TEXT_DIM(2.0f, 1.0f)) {
                        UIProgressSpinner(1.0f, "###loading_spinner"_s8);

                        if (ProgressVTarget && ProgressV != ProgressVTarget) {
                            UISetNextPreferredWidth(UI_PERCENT(1.0f, 0.0f));
                            UIProgressBar(
                                (f32) ((f64) ProgressV / (f64) ProgressVTarget),
                                "###loading_progress"_s8
                            );
                        } else {
                            UILabel("Loading"_s8);
                        }
                    }
                }
            }

            UISignalFromBox(Box);
        }
    }
}

void 
AppRequestFrame(void)
{
    APP_STATE->NumFramesRequested = 4;
}

Arena* 
AppFrameMemPool(void)
{
    return APP_STATE->FrameMemPools[APP_STATE->FrameIndex % ARRAY_COUNT(APP_STATE->FrameMemPools)];
}

void 
AppRegistersFillSlotFromStr(
    AppRegistersSlot Slot, 
    Str8 QueryExpr, 
    Str8 String
) {
    UNUSED(QueryExpr);

    switch (Slot) {
        default:
        case APP_REGISTERS_SLOT_STRING: {
            Registers()->String = ArenaPushStrCpy(AppFrameMemPool(), String);
        } break;

        case APP_REGISTERS_SLOT_FILE_PATH: {
            Registers()->String = ArenaPushStrCpy(AppFrameMemPool(), String);
            Registers()->FilePath = Registers()->String;
        } break;

        case APP_REGISTERS_SLOT_COMMAND_NAME: {
            Registers()->CommandName = ArenaPushStrCpy(
                AppFrameMemPool(), 
                String
            );
        } break;

        case APP_REGISTERS_SLOT_OFFSET: {
            Str8 Digits = String;

            if (StrMatch(StrPrefix(Digits, 2), "0x"_s8, STR_MATCH_ALL_CASES))
                Digits = StrSkip(Digits, 2);

            Registers()->String = ArenaPushStrCpy(AppFrameMemPool(), String);
            Registers()->Offset = U64FromStr(Digits, 16);
        } break;
    }
}

AppCommandKind 
AppCommandKindFromStr(Str8 String)
{
    AppCommandKind Result = APP_COMMAND_KIND_NULL;

    for (u64 I = 0; I < ARRAY_COUNT(APP_COMMAND_KIND_INFO_TABLE); ++I) {
        if (StrMatch(String, APP_COMMAND_KIND_INFO_TABLE[I].String, 0)) {
            Result = (AppCommandKind) I;
            break;
        }
    }

    if (Result == APP_COMMAND_KIND_NULL) {
        LogError(
            "'%S' [%llu] is not a valid command", 
            PRINT_STR(String), 
            String.Size
        );
    }

    return Result;
}

AppCommandKindInfo* 
AppCommandKindInfoFromStr(Str8 String)
{
    AppCommandKindInfo* Result = EMPTY_APP_COMMAND_KIND_INFO_VALUE;

    if (String.Size && String.Str) {
        AppCommandKind Kind = AppCommandKindFromStr(String);

        if (Kind != APP_COMMAND_KIND_NULL)
            Result = &APP_COMMAND_KIND_INFO_TABLE[Kind];
    }

    return Result;
}

void 
AppPushCmd(Str8 Name, AppRegisters* Regs)
{
    ListPush(
        APP_STATE->CommandMemPools[0], 
        &APP_STATE->Commands[0], 
        Name, 
        Regs
    );
}

b32 
AppNextCommand(AppCommand** Command)
{
    u64 Slot = APP_STATE->CommandsGeneration % ARRAY_COUNT(APP_STATE->Commands);
    AppCommandNode* StartNode = APP_STATE->Commands[Slot].Head;

    if (Command[0]) {
        StartNode = CAST_FROM_MEMBER(AppCommandNode, Command, Command[0]);
        StartNode = StartNode->Next;
    }

    Command[0] = NULL;

    if (StartNode)
        Command[0] = &StartNode->Command;

    return !!Command[0];
}

b32 
AppNextViewCommand(AppCommand** Command)
{
    for (; AppNextCommand(Command); ) {
        if (Registers()->View == Command[0]->Registers->View)
            break;
    }

    b32 Result = !!Command[0];

    return Result;
}

AppRegisters* 
__AppPushRegisters(AppRegisters* Regs)
{
    AppRegistersNode* Node = ArenaPushArrayZero(
        AppFrameMemPool(), 
        AppRegistersNode, 
        1
    );

    CopyRegisters(AppFrameMemPool(), &Node->V, Regs);
    SLL_STACK_PUSH(APP_STATE->HeadRegisters, Node);

    return &Node->V;
}

AppRegisters* 
AppPopRegisters(void)
{
    AppRegisters* Regs = &APP_STATE->HeadRegisters->V;

    SLL_STACK_POP(APP_STATE->HeadRegisters);

    if (!APP_STATE->HeadRegisters)
        APP_STATE->HeadRegisters = &APP_STATE->BaseRegisters;

    return Regs;
}

MDNode* 
AppThemeTreeFromName(Arena* MemPool, Access* Acc, Str8 Name)
{
    MDNode* Result = EMPTY_MD_NODE_VALUE;

    for (u32 Index = 0; Index < APP_THEME_PRESET_COUNT; ++Index) {
        if (
            StrMatch(Name, APP_THEME_PRESET_DISPLAY_STR_TABLE[Index], 0) ||
            StrMatch(Name, APP_THEME_PRESET_CODE_STR_TABLE[Index], 0)
        ) {
            Result = APP_STATE->ThemePresetTrees[Index];
            break;
        }
    }

    if (Result == EMPTY_MD_NODE_VALUE) {
        Str8 ThemesFolder = ArenaPushStrFmt(
            MemPool,
            "%S/%s/themes",
            PRINT_STR(GetProcessProperties()->UserProgramDataPath),
            NC_DATA_DIR
        );
        Str8 Path = ArenaPushStrFmt(
            MemPool,
            "%S/%S",
            PRINT_STR(ThemesFolder),
            PRINT_STR(Name)
        );
        CKey Key = FSKeyFromPathRange(Path, Rng((u64) 0ULL, U64_MAX), 0);
        u128 FileHash = CHashFromKey(Key, 0);
        Str8 Text = CDataFromHash(Acc, FileHash);

        if (Text.Size)
            Result = MDTreeFromStr(MemPool, Text);
    }

    if (Result == EMPTY_MD_NODE_VALUE)
        Result = APP_STATE->ThemePresetTrees[APP_THEME_PRESET_AYU_DARK];

    return Result;
}

FancyStrList
AppTitleFStrFromConfig(Arena* MemPool, ConfigNode* Config)
{
    FancyStrList Result = {};
    FancyStrParams Params = {};
    Str8 DisplayName = Config->String;
    Str8 FileName = StrSkipLastSlash(
        ConfigNodeChildFromStr(
            Config, 
            "file"_s8
        )->Head->String
    );

    for (
        u64 Index = 0; 
        Index < ARRAY_COUNT(APP_TAB_FAST_PATH_VIEW_NAME_TABLE); 
        ++Index
    ) {
        if (
            StrMatch(
                Config->String, 
                APP_TAB_FAST_PATH_VIEW_NAME_TABLE[Index], 
                0
            )
        ) {
            DisplayName = APP_COMMAND_KIND_INFO_TABLE[APP_COMMAND_KIND_FIRST_TAB_FAST_PATH_CMD + Index].DisplayName;
            break;
        }
    }

    Params.Colour = UIColourFromName("text"_s8);
    ListPush(MemPool, &Result, &Params, NULL, DisplayName);

    if (FileName.Size) {
        Params.Flags = FANCY_STR_FLAG_DIMMED;
        ListPush(MemPool, &Result, &Params, NULL, " "_s8);
        ListPush(MemPool, &Result, &Params, NULL, FileName);
    }

    return Result;
}

AppQueryExpressionKind 
AppQueryExpressionKindFromStr(Str8 Expression)
{
    AppQueryExpressionKind Result = APP_QUERY_EXPR_KIND_NULL;

    if (StrMatch(Expression, "commands"_s8, 0))
        Result = APP_QUERY_EXPR_KIND_COMMANDS;
    else if (StrMatch(Expression, "tab_commands"_s8, 0))
        Result = APP_QUERY_EXPR_KIND_TAB_COMMANDS;
    else if (StrMatch(Expression, "themes"_s8, 0))
        Result = APP_QUERY_EXPR_KIND_THEMES;
    else if (StrMatch(Expression, "file_path"_s8, 0))
        Result = APP_QUERY_EXPR_KIND_FILE_PATH;

    return Result;
}

AppQueryResultList 
AppQueryResultsFromExpression(Arena* MemPool, Str8 Expression, Str8 Input)
{
    AppQueryResultList Result = {};
    AppQueryExpressionKind Kind = AppQueryExpressionKindFromStr(Expression);

    switch (Kind) {
        default: {} break;

        case APP_QUERY_EXPR_KIND_COMMANDS:
        case APP_QUERY_EXPR_KIND_TAB_COMMANDS: {
            u64 HeadIndex = (Kind == APP_QUERY_EXPR_KIND_TAB_COMMANDS)
                ? APP_COMMAND_KIND_FIRST_TAB_FAST_PATH_CMD
                : 1;

            for (u64 Index = HeadIndex; Index < APP_COMMAND_KIND_COUNT; ++Index) {
                AppCommandKindInfo* Info = &APP_COMMAND_KIND_INFO_TABLE[Index];

                if (!(Info->Flags & APP_COMMAND_KIND_FLAG_LIST_IN_UI))
                    continue;

                AppQueryResult Entry = {};

                Entry.String = Info->String;
                Entry.DisplayString = Info->DisplayName;
                Entry.Description = Info->Description;
                Entry.SearchTags = Info->SearchTags;
                Entry.IconKind = Info->IconKind;
                Entry.CommandInfo = Info;
                ListPush(MemPool, &Result, Entry);
            }
        } break;

        case APP_QUERY_EXPR_KIND_THEMES: {
            for (u32 Index = 0; Index < APP_THEME_PRESET_COUNT; ++Index) {
                AppQueryResult Entry = {};

                Entry.String = APP_THEME_PRESET_DISPLAY_STR_TABLE[Index];
                Entry.DisplayString = APP_THEME_PRESET_DISPLAY_STR_TABLE[Index];
                ListPush(MemPool, &Result, Entry);
            }
        } break;

        case APP_QUERY_EXPR_KIND_FILE_PATH: {
            Str8 Folder = StrChopLastSlash(Input);
            FileIter* Iter = SystemFileIterBegin(MemPool, Folder, 0);
            FileInfo Info = {};

            while (SystemFileIterNext(MemPool, Iter, &Info)) {
                b32 IsFolder = !!(Info.Properties.Kind & SYS_FILE_IS_DIR);

                // NOTE(nc): we only care about folders and *.tlog or *.TLOG files
                if (
                    !IsFolder &&
                    !StrMatch(
                        StrSkipLastDot(Info.Name),
                        "tlog"_s8,
                        STR_MATCH_ALL_CASES
                    )
                ) {
                    continue;
                }

                AppQueryResult Entry = {};

                Entry.String = ArenaPushStrFmt(
                    MemPool,
                    "%S/%S",
                    PRINT_STR(Folder),
                    PRINT_STR(Info.Name)
                );
                Entry.DisplayString = StrSkipLastSlash(Entry.String);
                Entry.IconKind = IsFolder 
                    ? APP_ICON_KIND_FOLDER 
                    : APP_ICON_KIND_FILE;
                Entry.Description = IsFolder
                    ? "Folder"_s8
                    : ArenaPushStrFmt(
                        MemPool, 
                        "%llu bytes", 
                        Info.Properties.Size
                    );
                ListPush(MemPool, &Result, Entry);
            }

            SystemFileIterEnd(Iter);
        } break;
    }

    return Result;
}

void 
AppQueryViewUI(r2f32 Rect)
{
    TempArena Scratch = GetScratch(NULL, 0);
    AppRegisters* QueryRegs = APP_STATE->QueryRegs;
    Str8 CommandName = QueryRegs->CommandName;
    AppCommandKindInfo* Info = AppCommandKindInfoFromStr(CommandName);
    b32 IsFilePath = (Info->Query.Slot == APP_REGISTERS_SLOT_FILE_PATH);
    Str8 Expression = IsFilePath ? "file_path"_s8 : QueryRegs->Expression;
    Str8 Input = Str(APP_STATE->QueryBuffer, APP_STATE->QueryStringSize);
    Str8 Needle = IsFilePath ? StrSkipLastSlash(Input) : Input;
    AppQueryResultList Results = AppQueryResultsFromExpression(
        Scratch.MemPool,
        Expression,
        Input
    );
    AppQueryResult** Rows = ArenaPushArrayZero(
        Scratch.MemPool,
        AppQueryResult*,
        Results.Count
    );
    u64 RowsCount = 0;

    for (AppQueryResultNode* N = Results.Head; N; N = N->Next) {
        FMRangeList NameMatches = FuzzyFind(
            Scratch.MemPool,
            Needle,
            N->V.DisplayString
        );
        FMRangeList TagsMatches = FuzzyFind(
            Scratch.MemPool,
            Needle,
            N->V.SearchTags
        );

        if (
            NameMatches.Count == NameMatches.StrPartCount ||
            TagsMatches.Count == TagsMatches.StrPartCount
        ) {
            Rows[RowsCount++] = &N->V;
        }
    }

    v2f32 RectLength = Length(Rect);
    f32 Width = CLAMP(20.0f, RectLength.X - 8.0f, 76.0f);
    f32 ListHeight = CLAMP(1.0f, RectLength.Y - 6.0f, 16.0f);
    r2f32 ListerRect = Rng(
        Rect.X0 + FloorF32((RectLength.X - Width) / 2.0f),
        Rect.Y0 + 1.0f,
        Rect.X0 + FloorF32((RectLength.X - Width) / 2.0f) + Width,
        Rect.Y0 + 1.0f + ListHeight + 4.0f
    );
    AppQueryResult* Accepted = NULL;
    b32 IsAccepted = FALSE;
    UISignal EditSig = {};

    APP_STATE->TextEditMode = TRUE;
    APP_STATE->QueryListCursor.Y = MIN(
        APP_STATE->QueryListCursor.Y, 
        (i64) RowsCount
    );
    APP_STATE->QueryListMark = APP_STATE->QueryListCursor;

    UIFocus(UI_FOCUS_KIND_ON) {
        UITag("floating"_s8) {
            UISetNextFlags(UI_BOX_KIND_DRAW_DROP_SHADOW);

            UIPane(ListerRect, "###query_lister"_s8) {
                Str8 InputPreEdit = ArenaPushStrCpy(Scratch.MemPool, Input);

                UISpacer(UI_PX(1.0f, 1.0f));
                UISetNextPreferredHeight(UI_PX(ListHeight + 2.0f, 1.0f));
                UIRowBegin();
                UISpacer(UI_PX(1.0f, 1.0f));
                UISetNextPreferredWidth(UI_PX(Width - 2.0f, 1.0f));
                UISetNextPreferredHeight(UI_PERCENT(1.0f, 0.0f));
                UIColumnBegin();

                UIWidthFill() {
                    UINamedRow("###query_input_row"_s8) {
                        UIBox* InputRow = UIHeadParent();
                        UIKey InputKey = UIKeyFromStr(
                            InputRow->Key,
                            "###query_input"_s8
                        );

                        InputRow->DefaultNavFocusActiveKey = InputKey;
                        InputRow->DefaultNavFocusNextActiveKey = InputKey;

                        UIPreferredWidth(UI_TEXT_DIM(2.0f, 1.0f)) {
                            UITag("weak"_s8) {
                                UILabel(APP_ICON_KIND_TEXT_TABLE[APP_ICON_KIND_FIND]);

                                if (CommandName.Size)
                                    UILabel(Info->DisplayName);
                            }
                        }

                        UITag("alt"_s8) {
                            EditSig = UILineEdit(
                                &APP_STATE->QueryCursor,
                                &APP_STATE->QueryMark,
                                APP_STATE->QueryBuffer,
                                sizeof(APP_STATE->QueryBuffer),
                                &APP_STATE->QueryStringSize,
                                Input,
                                "###query_input"_s8
                            );
                        }
                    }

                    if (
                        !StrMatch(
                            InputPreEdit,
                            Str(APP_STATE->QueryBuffer, APP_STATE->QueryStringSize),
                            0
                        )
                    ) {
                        APP_STATE->QueryListCursor = {};
                        APP_STATE->QueryListCursor.Y = IsFilePath ? 0 : 1;
                        APP_STATE->QueryListMark = APP_STATE->QueryListCursor;
                        APP_STATE->QueryScroll = {};
                        AppRequestFrame();
                    }

                    UIDivider(UI_PX(1.0f, 1.0f));

                    UIScrollListParameters Params = {};
                    UIScrollListSignal ListSig = {};
                    r1i64 Visible = {};

                    Params.Kind = UI_SCROLL_LIST_KIND_ALL;
                    Params.DimensionsPX = Vec(Width - 2.0f, ListHeight);
                    Params.RowHeightPX = 1.0f;
                    Params.CursorRange.Max.Y = (i64) RowsCount;
                    Params.ItemRange = Rng((i64) 0, (i64) RowsCount);
                    Params.CursorMinIsEmptySelection[AXIS_2D_Y] = TRUE;

                    UIScrollList(
                        &Params,
                        &APP_STATE->QueryScroll,
                        &APP_STATE->QueryListCursor,
                        &APP_STATE->QueryListMark,
                        &Visible,
                        &ListSig
                    ) {
                        for (
                            i64 Row = Visible.Min;
                            Row <= Visible.Max && Row < (i64) RowsCount;
                            ++Row
                        ) {
                            AppQueryResult* Entry = Rows[Row];
                            b32 IsCursor = (
                                APP_STATE->QueryListCursor.Y == Row + 1
                            );
                            FMRangeList Matches = FuzzyFind(
                                Scratch.MemPool,
                                Needle,
                                Entry->DisplayString
                            );
                            Str8 Note = Entry->Description;

                            UITag(
                                IsCursor 
                                    ? "pop"_s8 
                                    : ""_s8
                            ) {
                                UISetNextChildLayoutAxis(AXIS_2D_X);

                                UIBox* RowBox = UIBuildBoxFromStrFmt(
                                    (
                                        UI_BOX_KIND_MOUSE_CLICKABLE |
                                        UI_BOX_KIND_DRAW_BACKGROUND |
                                        UI_BOX_KIND_DRAW_HOT_EFFECTS
                                    ),
                                    "###query_row_%lld",
                                    Row
                                );

                                UIParent(RowBox) {
                                    UIPreferredWidth(UI_TEXT_DIM(2.0f, 1.0f)) {
                                        if (Entry->IconKind != APP_ICON_KIND_NULL) {
                                            UITag("weak"_s8) {
                                                UILabel(
                                                    APP_ICON_KIND_TEXT_TABLE[Entry->IconKind]
                                                );
                                            }
                                        }

                                        UIBox* LabelBox = UILabel(
                                            Entry->DisplayString
                                        ).Box;

                                        UIBoxEquipFuzzyMatchRanges(
                                            LabelBox, 
                                            &Matches
                                        );
                                        UISpacer(UI_PERCENT(1.0f, 0.0f));

                                        if (Entry->CommandInfo) {
                                            AppCommandBindingButtons(
                                                Entry->String,
                                                ""_s8,
                                                U64_MAX,
                                                APP_COMMAND_BINDING_BTN_FLAG_ADD_NEW
                                            );
                                        } else if (Note.Size) {
                                            UITag("weak"_s8) {
                                                UILabel(Note);
                                            }
                                        }
                                    }
                                }

                                UISignal RowSig = UISignalFromBox(RowBox);

                                if (UI_PRESSED(RowSig)) {
                                    APP_STATE->QueryListCursor.Y = Row + 1;
                                    APP_STATE->QueryListMark = APP_STATE->QueryListCursor;
                                }

                                if (UI_CLICKED(RowSig)) {
                                    Accepted = Entry;
                                    IsAccepted = TRUE;
                                }
                            }
                        }
                    }

                    APP_STATE->QueryScroll.Offset = 0.0f;
                }

                    UIColumnEnd();
                    UIRowEnd();
            }
        }

        for (UIEvent* Evt = NULL; UINextEvent(&Evt); ) {
            if (
                Evt->Kind == UI_EVENT_KIND_PRESS &&
                Evt->Input == INPUT_KIND_LEFT_MOUSE_BTN &&
                !InRange(ListerRect, Evt->Position)
            ) {
                AppCmd(APP_COMMAND_KIND_CANCEL_QUERY);
                break;
            }
        }

        if (UISlotPress(UI_EVENT_ACTION_SLOT_CANCEL))
            AppCmd(APP_COMMAND_KIND_CANCEL_QUERY);

        if (
            UI_COMMITTED(EditSig) ||
            UISlotPress(UI_EVENT_ACTION_SLOT_ACCEPT)
        ) {
            IsAccepted = TRUE;

            if (APP_STATE->QueryListCursor.Y > 0)
                Accepted = Rows[APP_STATE->QueryListCursor.Y - 1];
            else if (!IsFilePath && RowsCount)
                Accepted = Rows[0];
        }
    }

    if (
        IsAccepted && 
        IsFilePath && 
        Accepted && 
        Accepted->IconKind == APP_ICON_KIND_FOLDER
    ) {
        Str8 NewInput = ArenaPushStrFmt(
            Scratch.MemPool,
            "%S/",
            PRINT_STR(Accepted->String)
        );

        APP_STATE->QueryStringSize = MIN(
            sizeof(APP_STATE->QueryBuffer), 
            NewInput.Size
        );
        MemCpy(
            APP_STATE->QueryBuffer, 
            NewInput.Str, 
            APP_STATE->QueryStringSize
        );
        APP_STATE->QueryCursor = TxtPt(1, 1 + APP_STATE->QueryStringSize);
        APP_STATE->QueryMark = APP_STATE->QueryCursor;
        APP_STATE->QueryListCursor = {};
        APP_STATE->QueryListMark = {};
        APP_STATE->QueryScroll = {};
        AppRequestFrame();
    } else if (IsAccepted && (Accepted || IsFilePath)) {
        Str8 Value = Accepted 
            ? Accepted->String 
            : StrTrimLastSlash(Input);

        AppCmd(APP_COMMAND_KIND_CANCEL_QUERY);

        AppRegistersScope() {
            CopyRegisters(AppFrameMemPool(), Registers(), QueryRegs);

            if (!CommandName.Size) {
                AppCmd(
                    APP_COMMAND_KIND_RUN_COMMAND,
                    __Registers.CommandName = Value,
                    __Registers.Expression = ""_s8
                );
            } else {
                AppRegistersFillSlotFromStr(
                    Info->Query.Slot, 
                    Expression, 
                    Value
                );
                Registers()->Expression = ""_s8;
                AppPushCmd(CommandName, Registers());
            }
        }
    }

    ReleaseScratch(Scratch);
}

UISignal 
AppIconButton(AppIconKind Kind, FMRangeList* Matches, Str8 String)
{
    Str8 DisplayString = UIDisplayPartFromKeyStr(String);

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
        if (Kind != APP_ICON_KIND_NULL) {
            UIPreferredWidth(UI_TEXT_DIM(2.0f, 1.0f)) {
                UITag("weak"_s8) {
                    UILabel(APP_ICON_KIND_TEXT_TABLE[Kind]);
                }
            }
        }

        if (DisplayString.Size) {
            UIPreferredWidth(UI_PERCENT(1.0f, 0.0f)) {
                UIBox* LabelBox = UILabel(DisplayString).Box;

                if (Matches)
                    UIBoxEquipFuzzyMatchRanges(LabelBox, Matches);
            }
        }
    }

    UISignal Result = UISignalFromBox(Box);

    return Result;
}

UISignal 
AppIconButton(AppIconKind Kind, FMRangeList* Matches, char* Fmt, ...)
{
    TempArena Scratch = GetScratch(NULL, 0);
    va_list Args;

    va_start(Args, Fmt);

    Str8 String = ArenaPushStrFmtV(
        Scratch.MemPool,
        Fmt,
        Args
    );
    UISignal Result = AppIconButton(Kind, Matches, String);

    va_end(Args);
    ReleaseScratch(Scratch);

    return Result;
}

UISignal 
AppMenuBarButton(Str8 String)
{
    UIBox* Box = UIBuildBoxFromStr(
        (
            UI_BOX_KIND_DRAW_TEXT |
            UI_BOX_KIND_DRAW_BACKGROUND |
            UI_BOX_KIND_CLICKABLE |
            UI_BOX_KIND_DRAW_HOT_EFFECTS
        ),
        String
    );

    UISignal Result = UISignalFromBox(Box);

    return Result;
}

void
AppCommandBindingButtons(
    Str8 Name,
    Str8 Filter,
    u64 Limit,
    AppCommandBindingButtonFlag Flags
) {
    TempArena Scratch = GetScratch(NULL, 0);
    ConfigInputMapNodePtrList InputMapNodes = ConfigInputMapNodePtrListFromName(
        Scratch.MemPool,
        APP_STATE->KeyMap,
        Name
    );
    u64 InputMapIndex = 0;

    for (
        ConfigInputMapNodePtr* N = InputMapNodes.Head;
        N && InputMapIndex < Limit;
        N = N->Next
    ) {
        ConfigBinding Binding = N->V->Binding;

        if (Binding.Input == INPUT_KIND_NULL)
            continue;

        ++InputMapIndex;
        UISpacer(UI_PX(1.0f, 1.0f));

        b32 RebindingActiveForThisBinding = (
            APP_STATE->BindChangeActive &&
            StrMatch(APP_STATE->BindChangeCommandName, Name, 0) &&
            N->V->ID == APP_STATE->BindChangeBindingID
        );
        b32 IsReserved = (
            !Binding.ModKind &&
            (
                Binding.Input == INPUT_KIND_ESC ||
                Binding.Input == INPUT_KIND_DELETE
            )
        );
        b32 CanEdit = (
            !(Flags & APP_COMMAND_BINDING_BTN_FLAG_NO_EDIT) &&
            !IsReserved
        );
        b32 HasConflicts = FALSE;
        ConfigInputMapNodePtrList NodesWithThisBinding = ConfigInputMapNodePtrListFromBinding(
            Scratch.MemPool,
            APP_STATE->KeyMap,
            Binding
        );

        for (
            ConfigInputMapNodePtr* N2 = NodesWithThisBinding.Head;
            N2;
            N2 = N2->Next
        ) {
            if (!StrMatch(N->V->Name, N2->V->Name, 0)) {
                HasConflicts = TRUE;
                break;
            }
        }

        Str8 KeyBindingString = RebindingActiveForThisBinding
            ? "\xE2\x80\xA6"_s8
            : StrFromInputModifierInput(
                Scratch.MemPool,
                Binding.ModKind,
                Binding.Input
            );
        FMRangeList Matches = {};

        if (Filter.Size)
            Matches = FuzzyFind(Scratch.MemPool, Filter, KeyBindingString);

        UISetNextTag(
            RebindingActiveForThisBinding
                ? "pop"_s8
                : HasConflicts
                    ? "bad"_s8
                    : "alt"_s8
        );
        UISetNextTextAlignment(UI_TEXT_ALIGN_CENTRE);
        UISetNextGroupKey(EMPTY_UI_KEY_VALUE);
        UISetNextFastpathCodepoint(0);
        UISetNextPreferredWidth(UI_TEXT_DIM(0.0f, 1.0f));

        UIBox* Box = UIBuildBoxFromStrFmt(
            (
                UI_BOX_KIND_DRAW_TEXT |
                UI_BOX_KIND_DRAW_BACKGROUND |
                UI_BOX_KIND_FOCUS_NAV_SKIP |
                (
                    CanEdit * (
                        UI_BOX_KIND_MOUSE_CLICKABLE |
                        UI_BOX_KIND_DRAW_HOT_EFFECTS |
                        UI_BOX_KIND_DRAW_ACTIVE_EFFECTS
                    )
                )
            ),
            "%S###bind_btn_%S_%x_%x",
            PRINT_STR(KeyBindingString),
            PRINT_STR(Name),
            Binding.Input,
            Binding.ModKind
        );

        if (Filter.Size)
            UIBoxEquipFuzzyMatchRanges(Box, &Matches);

        UISignal Sig = UISignalFromBox(Box);

        if (!APP_STATE->BindChangeActive && UI_CLICKED(Sig)) {
            ArenaClear(APP_STATE->BindChangeMemPool);
            APP_STATE->BindChangeActive = TRUE;
            APP_STATE->BindChangeCommandName = ArenaPushStrCpy(
                APP_STATE->BindChangeMemPool,
                Name
            );
            APP_STATE->BindChangeBindingID = N->V->ID;
            APP_STATE->BindChangeUIKey = Box->Key;
        } else if (APP_STATE->BindChangeActive && UI_CLICKED(Sig)) {
            APP_STATE->BindChangeActive = FALSE;
        }

        if (UI_HOVERING(Sig) && HasConflicts) {
            UITooltip() {
                UILabel("Other commands that have this key:"_s8);

                for (
                    ConfigInputMapNodePtr* N2 = NodesWithThisBinding.Head;
                    N2;
                    N2 = N2->Next
                ) {
                    if (!StrMatch(N2->V->Name, N->V->Name, 0))
                        UILabel(AppCommandKindInfoFromStr(N2->V->Name)->DisplayName);
                }
            }
        }

        if (RebindingActiveForThisBinding) {
            UISetNextTag("bad"_s8);
            UISetNextTextAlignment(UI_TEXT_ALIGN_CENTRE);
            UISetNextGroupKey(EMPTY_UI_KEY_VALUE);
            UISetNextFastpathCodepoint(0);
            UISetNextPreferredWidth(UI_PX(3.0f, 1.0f));

            UIBox* RemoveBox = UIBuildBoxFromStrFmt(
                (
                    UI_BOX_KIND_DRAW_TEXT |
                    UI_BOX_KIND_DRAW_BACKGROUND |
                    UI_BOX_KIND_FOCUS_NAV_SKIP |
                    UI_BOX_KIND_MOUSE_CLICKABLE |
                    UI_BOX_KIND_DRAW_HOT_EFFECTS |
                    UI_BOX_KIND_DRAW_ACTIVE_EFFECTS
                ),
                "%S###remove_binding_%S",
                PRINT_STR(APP_ICON_KIND_TEXT_TABLE[APP_ICON_KIND_X]),
                PRINT_STR(Name)
            );

            if (UI_PRESSED(UISignalFromBox(RemoveBox))) {
                ConfigNodeRelease(
                    APP_STATE->Config,
                    ConfigNodeFromID(APP_STATE->BindChangeBindingID)
                );
                APP_STATE->BindChangeActive = FALSE;
            }
        }
    }

    if (Flags & APP_COMMAND_BINDING_BTN_FLAG_ADD_NEW) {
        b32 AddingNewBinding = (
            APP_STATE->BindChangeActive &&
            StrMatch(APP_STATE->BindChangeCommandName, Name, 0) &&
            !APP_STATE->BindChangeBindingID
        );

        UISpacer(UI_PX(1.0f, 1.0f));
        UISetNextTag(AddingNewBinding ? "pop"_s8 : "alt"_s8);
        UISetNextTextAlignment(UI_TEXT_ALIGN_CENTRE);
        UISetNextGroupKey(EMPTY_UI_KEY_VALUE);
        UISetNextFastpathCodepoint(0);
        UISetNextPreferredWidth(UI_PX(3.0f, 1.0f));

        Str8 BindingString = (AddingNewBinding) 
            ? "\xE2\x80\xA6"_s8 
            : APP_ICON_KIND_TEXT_TABLE[APP_ICON_KIND_ADD];
        UIBox* Box = UIBuildBoxFromStrFmt(
            (
                UI_BOX_KIND_DRAW_TEXT |
                UI_BOX_KIND_DRAW_BACKGROUND |
                UI_BOX_KIND_FOCUS_NAV_SKIP |
                UI_BOX_KIND_MOUSE_CLICKABLE |
                UI_BOX_KIND_DRAW_HOT_EFFECTS |
                UI_BOX_KIND_DRAW_ACTIVE_EFFECTS
            ),
            "%S###add_binding_%S",
            PRINT_STR(BindingString),
            PRINT_STR(Name)
        );

        // UIBox* Box = UIBuildBoxFromStrFmt(
        //     (
        //         UI_BOX_KIND_DRAW_TEXT |
        //         UI_BOX_KIND_DRAW_BACKGROUND |
        //         UI_BOX_KIND_FOCUS_NAV_SKIP |
        //         UI_BOX_KIND_MOUSE_CLICKABLE |
        //         UI_BOX_KIND_DRAW_HOT_EFFECTS |
        //         UI_BOX_KIND_DRAW_ACTIVE_EFFECTS
        //     ),
        //     "%S###add_binding_%S",
        //     AddingNewBinding
        //         ? PRINT_STR("\xE2\x80\xA6"_s8)
        //         : PRINT_STR(APP_ICON_KIND_TEXT_TABLE[APP_ICON_KIND_ADD]),
        //     PRINT_STR(Name)
        // );
        UISignal Sig = UISignalFromBox(Box);

        if (!AddingNewBinding && UI_CLICKED(Sig)) {
            ArenaClear(APP_STATE->BindChangeMemPool);
            APP_STATE->BindChangeActive = TRUE;
            APP_STATE->BindChangeCommandName = ArenaPushStrCpy(
                APP_STATE->BindChangeMemPool,
                Name
            );
            APP_STATE->BindChangeBindingID = 0;
            APP_STATE->BindChangeUIKey = Box->Key;
        } else if (AddingNewBinding && UI_CLICKED(Sig)) {
            APP_STATE->BindChangeActive = FALSE;
        }
    }

    ReleaseScratch(Scratch);
}

UISignal 
AppCommandSpecButton(Str8 Name)
{
    TempArena Scratch = GetScratch(NULL, 0);
    AppCommandKindInfo* Info = AppCommandKindInfoFromStr(Name);
    ConfigInputMapNodePtrList InputMapNodes = ConfigInputMapNodePtrListFromName(
        Scratch.MemPool,
        APP_STATE->KeyMap,
        Name
    );

    UISetNextChildLayoutAxis(AXIS_2D_X);

    UIBox* Box = UIBuildBoxFromStrFmt(
        (
            UI_BOX_KIND_DRAW_BACKGROUND |
            UI_BOX_KIND_DRAW_HOT_EFFECTS |
            UI_BOX_KIND_DRAW_ACTIVE_EFFECTS |
            UI_BOX_KIND_CLICKABLE
        ),
        "###cmd_%p",
        Info
    );

    UIParent(Box) {
        UIPreferredWidth(UI_TEXT_DIM(2.0f, 1.0f)) {
            UIFlags(UI_BOX_KIND_DRAW_TEXT_FASTPATH_CODEPOINT) {
                UIFastpathCodepoint(Box->FastpathCodepoint) {
                    UILabel(Info->DisplayName);
                }
            }

            UISpacer(UI_PERCENT(1.0f, 0.0f));
            UISetNextFlags(
                UI_BOX_KIND_MOUSE_CLICKABLE | UI_BOX_KIND_FOCUS_NAV_SKIP
            );
            UISetNextGroupKey(EMPTY_UI_KEY_VALUE);

            UIPreferredWidth(UI_SUM_OF_CHILDREN(1.0f)) {
                UINamedRow("###bindings"_s8) {
                    AppCommandBindingButtons(
                        Name,
                        ""_s8,
                        1,
                        InputMapNodes.Head
                            ? 0
                            : APP_COMMAND_BINDING_BTN_FLAG_ADD_NEW
                    );
                }
            }
        }
    }

    UISignal Sig = UISignalFromBox(Box);

    ReleaseScratch(Scratch);

    return Sig;
}

void 
AppCommandListMenuButtons(
    Str8* CommandNames, 
    u64 CommandNamesCount, 
    u32* FastPointCodePoints
) {
    UISetNextPreferredHeight(UI_SUM_OF_CHILDREN(1.0f));

    UIColumn() {
        for (u64 Index = 0; Index < CommandNamesCount; ++Index) {
            if (!CommandNames[Index].Size) {
                UIDivider(UI_PX(1.0f, 1.0f));
            } else {
                AppCommandKind Kind = AppCommandKindFromStr(CommandNames[Index]);
                b32 IsDisabled = (
                    Kind >= APP_COMMAND_KIND_FIRST_TAB_FAST_PATH_CMD &&
                    APP_COMMAND_KIND_INFO_TABLE[Kind].Flags & APP_COMMAND_KIND_FLAG_LIST_IN_FILE &&
                    !Registers()->FilePath.Size
                );

                if (IsDisabled)
                    UISetNextFlags(UI_BOX_KIND_DISABLED);

                UISetNextFastpathCodepoint(FastPointCodePoints[Index]);

                UISignal Sig = AppCommandSpecButton(CommandNames[Index]);

                if (UI_CLICKED(Sig) && !IsDisabled) {
                    AppCmd(
                        APP_COMMAND_KIND_RUN_COMMAND,
                        __Registers.CommandName = CommandNames[Index]
                    );
                    UIContextMenuClose();
                    APP_STATE->MenuBarFocused = FALSE;
                }
            }
        }
    }
}

void 
AppInit(CommandLine* CLI)
{
    EMPTY_APP_VIEW_STATE_VALUE->HashNext = EMPTY_APP_VIEW_STATE_VALUE;
    EMPTY_APP_VIEW_STATE_VALUE->HashPrev = EMPTY_APP_VIEW_STATE_VALUE;

    Arena* MemPool = ArenaAlloc();

    APP_STATE = ArenaPushArrayZero(MemPool, AppState, 1);
    APP_STATE->MemPool = MemPool;

    for (
        u64 Index = 0; 
        Index < ARRAY_COUNT(APP_STATE->FrameMemPools); 
        ++Index
    ) {
        APP_STATE->FrameMemPools[Index] = ArenaAlloc();
    }

    APP_STATE->NumFramesRequested = 2;

    for (
        u64 Index = 0; 
        Index < ARRAY_COUNT(APP_STATE->CommandMemPools); 
        ++Index
    ) {
        APP_STATE->CommandMemPools[Index] = ArenaAlloc();
    }

    APP_STATE->PopUpMemPool = ArenaAlloc();
    APP_STATE->QueryMemPool = ArenaAlloc();
    APP_STATE->DragDropMemPool = ArenaAlloc();
    APP_STATE->DragDropRegisters = ArenaPushArrayZero(
        APP_STATE->DragDropMemPool,
        AppRegisters,
        1
    );
    APP_STATE->BindChangeMemPool = ArenaAlloc();
    APP_STATE->HeadRegisters = &APP_STATE->BaseRegisters;
    APP_STATE->UI = UIStateAlloc();

    {
        APP_STATE->CfgSchemaTable = ArenaPushArrayZero(
            APP_STATE->MemPool,
            ConfigSchemaTable,
            1
        );
        APP_STATE->CfgSchemaTable->SlotsCount = 64;
        APP_STATE->CfgSchemaTable->Slots = ArenaPushArrayZero(
            APP_STATE->MemPool,
            ConfigSchemaNode*,
            APP_STATE->CfgSchemaTable->SlotsCount
        );
    }

    {
        for (
            AppThemePreset ThemePreset = 0;
            ThemePreset < APP_THEME_PRESET_COUNT;
            ++ThemePreset
        ) {
            APP_STATE->ThemePresetTrees[ThemePreset] = MDTreeFromStr(
                APP_STATE->MemPool,
                APP_THEME_PRESET_CONFIG_STR_TABLE[ThemePreset]
            )->Head;
        }
    }

    APP_STATE->Config = ConfigStateAlloc();
    ConfigContextSelect(ConfigContextFromState(APP_STATE->Config));

    ConfigNode* Session = ConfigNodeNew(
        APP_STATE->Config, 
        ConfigNodeRoot(), 
        "session"_s8
    );

    ConfigNodeNew(APP_STATE->Config, ConfigNodeRoot(), "command_line"_s8);
    ConfigNodeNew(APP_STATE->Config, ConfigNodeRoot(), "transient"_s8);

    APP_STATE->ViewStateSlotsCount = 1024;
    APP_STATE->ViewStateSlots = ArenaPushArrayZero(
        MemPool,
        AppViewStateSlot,
        APP_STATE->ViewStateSlotsCount
    );

    {
        struct {
            Str8 Name;
            Str8 Value;
        } Settings[] = {
            {
                Str8Lit("focus_menu_bar_with_alt"),
                Str8Lit("1")
            },
            {
                Str8Lit("animations"),
                Str8Lit("1")
            },
            {
                Str8Lit("scrolling_animations"),
                Str8Lit("1")
            }
        };
        ConfigNode* SettingsRoot = ConfigNodeNew(
            APP_STATE->Config, 
            Session, 
            "settings"_s8
        );

        for (u64 Index = 0; Index < ARRAY_COUNT(Settings); ++Index) {
            ConfigNode* Setting = ConfigNodeNew(
                APP_STATE->Config,
                SettingsRoot,
                Settings[Index].Name
            );

            ConfigNodeNew(APP_STATE->Config, Setting, Settings[Index].Value);
        }
    }

    {
        Str8 ThemeName = CommandLineString(CLI, "theme"_s8);
        ConfigNode* Theme = ConfigNodeNew(
            APP_STATE->Config, 
            Session, 
            "theme"_s8
        );

        if (!ThemeName.Size)
            ThemeName = APP_THEME_PRESET_DISPLAY_STR_TABLE[APP_THEME_PRESET_AYU_DARK];

        ConfigNodeNew(APP_STATE->Config, Theme, ThemeName);
    }

    {
        ConfigNode* Window = ConfigNodeNew(
            APP_STATE->Config, 
            Session, 
            "window"_s8
        );
        ConfigNode* Panels = ConfigNodeNew(
            APP_STATE->Config, 
            Window, 
            "panels"_s8
        );
        ConfigNode* Tab = ConfigNodeNew(
            APP_STATE->Config,
            Panels,
            APP_TAB_FAST_PATH_VIEW_NAME_TABLE[0]
        );

        ConfigNodeNew(APP_STATE->Config, Panels, "selected"_s8);
        ConfigNodeNew(APP_STATE->Config, Tab, "selected"_s8);
        BaseRegisters()->Window = Window->ID;
        BaseRegisters()->Panel = Panels->ID;
        BaseRegisters()->Tab = Tab->ID;
        BaseRegisters()->View = Tab->ID;
    }

    AppCmd(APP_COMMAND_KIND_RESET_TO_DEFAULT_BINDINGS);

    for (Str8Node* Node = CLI->Inputs.Head; Node; Node = Node->Next)
        AppCmd(APP_COMMAND_KIND_OPEN, __Registers.FilePath = Node->String);
}

void 
AppFrame(void)
{
    TempArena Scratch = GetScratch(NULL, 0);

    ++APP_STATE->FrameDepth;
    ArenaClear(AppFrameMemPool());
    APP_STATE->HeadRegisters = &APP_STATE->BaseRegisters;
    CopyRegisters(
        AppFrameMemPool(),
        &APP_STATE->HeadRegisters->V,
        &APP_STATE->HeadRegisters->V
    );

    b32 AllowTextHotkeys = !APP_STATE->TextEditMode;

    APP_STATE->TextEditMode = FALSE;

    if (APP_STATE->FrameDepth == 1) {
        TempArena Scratch = GetScratch(NULL, 0);
        ConfigNode* Window = ConfigNodeFromID(Registers()->Window);
        ConfigPanelTree PanelTree = ConfigPanelTreeFromConfig(
            Scratch.MemPool,
            Window
        );

        for (
            ConfigPanelNode* P = PanelTree.Root;
            P != EMPTY_CFG_PANEL_NODE_VALUE;
            P = ConfigPanelNodeRecDepthFirstPre(PanelTree.Root, P).Next
        ) {
            for (
                ConfigNodePtrNode* Node = P->Tabs.Head;
                Node;
                Node = Node->Next
            ) {
                AppViewStateFromConfig(Node->V);
            }

            if (
                P->SelectedTab == EMPTY_CFG_NODE_VALUE &&
                P->Tabs.Head
            ) {
                AppCmd(
                    APP_COMMAND_KIND_FOCUS_TAB,
                    __Registers.Panel = P->Config->ID,
                    __Registers.Tab = P->Tabs.Head->V->ID
                );
            }
        }

        ReleaseScratch(Scratch);
    }

    if (APP_STATE->FrameDepth == 1) {
        for (
            u64 SlotIndex = 0; 
            SlotIndex < APP_STATE->ViewStateSlotsCount; 
            ++SlotIndex
        ) {
            for (
                AppViewState* VS = APP_STATE->ViewStateSlots[SlotIndex].Head, *Next;
                VS;
                VS = Next
            ) {
                Next = VS->HashNext;

                if (VS->LastFrameIndexTouched + 2 < APP_STATE->FrameIndex) {
                    for (
                        AppArenaExt* Ext = VS->HeadArenaExt;
                        Ext;
                        Ext = Ext->Next
                    ) {
                        ArenaRelease(Ext->MemPool);
                    }

                    ArenaRelease(VS->MemPool);
                    DLL_REMOVE_EX(
                        NULL,
                        APP_STATE->ViewStateSlots[SlotIndex].Head,
                        APP_STATE->ViewStateSlots[SlotIndex].Tail,
                        VS,
                        HashNext,
                        HashPrev
                    );
                    SLL_STACK_PUSH_EX(
                        APP_STATE->FreeViewState,
                        VS,
                        HashNext
                    );

                    if (APP_STATE->ViewStateLastAccessed == VS) {
                        APP_STATE->ViewStateLastAccessed = EMPTY_APP_VIEW_STATE_VALUE;
                        APP_STATE->ViewStateLastAccessedID = 0;
                    }
                }
            }
        }
    }

    if (APP_STATE->FrameDepth == 1) {
        f32 SlowRate = 1.0f - PowF32(2.0f, (-10.0f * APP_STATE->FrameDeltaTime));

        for (
            u64 SlotIndex = 0;
            SlotIndex < APP_STATE->ViewStateSlotsCount;
            ++SlotIndex
        ) {
            for (
                AppViewState* VS = APP_STATE->ViewStateSlots[SlotIndex].Head;
                VS;
                VS = VS->HashNext
            ) {
                f32 ScrollXDiff = (-VS->ScrollPosition.X.Offset);
                f32 ScrollYDiff = (-VS->ScrollPosition.Y.Offset);
                f32 LoadingTDiff = (VS->LoadingTTarget - VS->LoadingT);

                VS->ScrollPosition.X.Offset += ScrollXDiff * APP_STATE->ScrollingAnimationRate;
                VS->ScrollPosition.Y.Offset += ScrollYDiff * APP_STATE->ScrollingAnimationRate;
                VS->LoadingT += LoadingTDiff * SlowRate;

                if (
                    AbsF32(LoadingTDiff) > 0.01f ||
                    AbsF32(ScrollXDiff) > 0.01f ||
                    AbsF32(ScrollYDiff) > 0.01f
                ) {
                    AppRequestFrame();
                }

                if (AbsF32(ScrollXDiff) <= 0.01f)
                    VS->ScrollPosition.X.Offset = 0.0f;

                if (AbsF32(ScrollYDiff) <= 0.01f)
                    VS->ScrollPosition.Y.Offset = 0.0f;

                ConfigNode* VConfig = ConfigNodeFromID(VS->CfgID);

                if (
                    ConfigNodeChildFromStr(
                        VConfig,
                        "selected"_s8
                    ) != EMPTY_CFG_NODE_VALUE
                ) {
                    if (VS->LoadingTTarget > 0.5f)
                        AppRequestFrame();

                    VS->LoadingTTarget = 0.0f;
                }
            }
        }
    }

    InputEventList Events = {};

    if (APP_STATE->FrameDepth == 1)
        Events = GetEvents(Scratch.MemPool, !APP_STATE->NumFramesRequested);

    Access* FrameAccessRestore = APP_STATE->FrameAccess;

    APP_STATE->FrameAccess = AccessOpen();

    f32 TargetHz = GetTerminalProperties()->DefaultRefreshRate;

    if (TargetHz < 1.0f)
        TargetHz = 60.0f;

    u64 FrameTimeTargetCapacityUSecs = (u64) (MILLION(1) / TargetHz);

    APP_STATE->FrameDeltaTime = 1.0f / TargetHz;

    u64 StartTimeUSecs = TimeNow();

    if (!APP_STATE->PopUpActive && APP_STATE->BindChangeActive) {
        if (InputPress(&Events, 0, INPUT_KIND_ESC)) {
            AppRequestFrame();
            APP_STATE->BindChangeActive = FALSE;
        }

        if (
            APP_STATE->BindChangeActive &&
            InputPress(&Events, 0, INPUT_KIND_DELETE)
        ) {
            AppRequestFrame();
            ConfigNodeRelease(
                APP_STATE->Config,
                ConfigNodeFromID(APP_STATE->BindChangeBindingID)
            );
            APP_STATE->BindChangeActive = FALSE;
        }

        for (
            InputEvent* Event = Events.Head, *Next = NULL;
            Event && APP_STATE->BindChangeActive;
            Event = Next
        ) {
            Next = Event->Next;

            if (
                Event->Kind == EVENT_KIND_PRESS &&
                Event->Input != INPUT_KIND_ESC &&
                Event->Input != INPUT_KIND_RETURN &&
                Event->Input != INPUT_KIND_BACKSPACE &&
                Event->Input != INPUT_KIND_DELETE &&
                Event->Input != INPUT_KIND_LEFT_MOUSE_BTN &&
                Event->Input != INPUT_KIND_RIGHT_MOUSE_BTN &&
                Event->Input != INPUT_KIND_MIDDLE_MOUSE_BTN &&
                Event->Input != INPUT_KIND_CTRL &&
                Event->Input != INPUT_KIND_ALT &&
                Event->Input != INPUT_KIND_SHIFT
            ) {
                APP_STATE->BindChangeActive = FALSE;

                ConfigNode* Binding = ConfigNodeFromID(
                    APP_STATE->BindChangeBindingID
                );

                if (Binding == EMPTY_CFG_NODE_VALUE) {
                    ConfigNode* Session = ConfigNodeChildFromStr(
                        ConfigNodeRoot(),
                        "session"_s8
                    );
                    ConfigNode* InputBindings = ConfigNodeChildFromStrOrAlloc(
                        APP_STATE->Config,
                        Session,
                        "keybindings"_s8
                    );

                    Binding = ConfigNodeNew(
                        APP_STATE->Config,
                        InputBindings,
                        ""_s8
                    );
                }

                ConfigNodeReleaseAllChildren(APP_STATE->Config, Binding);
                ConfigNodeNew(
                    APP_STATE->Config,
                    Binding,
                    APP_STATE->BindChangeCommandName
                );
                ConfigNodeNew(
                    APP_STATE->Config,
                    Binding,
                    INPUT_DISPLAY_STR_TABLE[Event->Input]
                );

                if (Event->Modifier & INPUT_MOD_KIND_CTRL)
                    ConfigNodeNew(APP_STATE->Config, Binding, "ctrl"_s8);

                if (Event->Modifier & INPUT_MOD_KIND_SHIFT)
                    ConfigNodeNew(APP_STATE->Config, Binding, "shift"_s8);

                if (Event->Modifier & INPUT_MOD_KIND_ALT)
                    ConfigNodeNew(APP_STATE->Config, Binding, "alt"_s8);

                Text(
                    &Events,
                    CodePointFromInput(Event->Modifier, Event->Input)
                );
                ConsumeEvent(&Events, Event);
                AppRequestFrame();
            }
        }
    }

    APP_STATE->KeyMap = ConfigInputMapFromConfig(AppFrameMemPool());

    for (
        InputEvent* Event = Events.Head, *Next = NULL;
        Event;
        Event = Next
    ) {
        AppRegistersScope() {
            Next = Event->Next;

            b32 Take = FALSE;

            if (
                AppDragIsActive() &&
                Event->Kind == EVENT_KIND_RELEASE &&
                Event->Input == INPUT_KIND_LEFT_MOUSE_BTN
            ) {
                APP_STATE->DragDropState = APP_DRAG_DROP_STATE_DROPPING;
            }

            if (!Take && Event->Kind == EVENT_KIND_CONSOLE_CLOSE) {
                Take = TRUE;
                AppCmd(APP_COMMAND_KIND_EXIT);
            }

            if (Event->Kind == EVENT_KIND_CONSOLE_LOSE_FOCUS) {
                APP_STATE->MenuBarKeyHeld = FALSE;
                APP_STATE->MenuBarFocusPressStarted = FALSE;
            }

            if (
                Event->Kind == EVENT_KIND_PRESS &&
                APP_STATE->ErrorStrSize &&
                APP_STATE->ErrorFrameIndex < APP_STATE->FrameIndex
            ) {
                APP_STATE->ErrorStrSize = 0;
                AppRequestFrame();
            }

            if (APP_STATE->AltMenuBarEnabled) {
                if (
                    !Take &&
                    Event->Kind == EVENT_KIND_PRESS &&
                    Event->Input == INPUT_KIND_ALT &&
                    !Event->Modifier &&
                    !Event->IsRepeat
                ) {
                    Take = TRUE;
                    AppRequestFrame();
                    APP_STATE->MenuBarFocusedOnPress = APP_STATE->MenuBarFocused;
                    APP_STATE->MenuBarKeyHeld = TRUE;
                    APP_STATE->MenuBarFocusPressStarted = TRUE;
                }

                if (
                    APP_STATE->MenuBarFocused &&
                    Event->Kind == EVENT_KIND_PRESS &&
                    Event->Input == INPUT_KIND_ALT &&
                    !Event->Modifier &&
                    !Event->IsRepeat
                ) {
                    Take = TRUE;
                    AppRequestFrame();
                    APP_STATE->MenuBarFocused = FALSE;
                } else if (
                    APP_STATE->MenuBarFocusPressStarted &&
                    !APP_STATE->MenuBarFocused &&
                    Event->Kind == EVENT_KIND_RELEASE &&
                    !Event->Modifier &&
                    Event->Input == INPUT_KIND_ALT &&
                    !Event->IsRepeat
                ) {
                    Take = TRUE;
                    AppRequestFrame();
                    APP_STATE->MenuBarFocused = !APP_STATE->MenuBarFocusedOnPress;
                    APP_STATE->MenuBarFocusPressStarted = FALSE;
                } else if (
                    Event->Kind == EVENT_KIND_PRESS &&
                    Event->Input == INPUT_KIND_ESC &&
                    APP_STATE->MenuBarFocused &&
                    !UIAnyContextMenuIsOpen()
                ) {
                    Take = FALSE;
                    AppRequestFrame();
                    APP_STATE->MenuBarFocused = FALSE;
                }

                if (
                    Event->Kind == EVENT_KIND_RELEASE &&
                    Event->Input == INPUT_KIND_ALT
                ) {
                    APP_STATE->MenuBarKeyHeld = FALSE;
                }
            }

            if (!Take && Event->Kind == EVENT_KIND_PRESS) {
                ConfigBinding Binding = {};

                Binding.Input = Event->Input;
                Binding.ModKind = Event->Modifier;

                ConfigInputMapNodePtrList KeyMapNodes = ConfigInputMapNodePtrListFromBinding(
                    Scratch.MemPool,
                    APP_STATE->KeyMap,
                    Binding
                );

                if (KeyMapNodes.Head) {
                    u32 HitCharacter = CodePointFromInput(
                        Event->Modifier, 
                        Event->Input
                    );
                    b32 IsPrintable = (
                        HitCharacter >= 32 && 
                        HitCharacter < 127
                    );

                    if (!HitCharacter || AllowTextHotkeys) {
                        AppCmd(
                            APP_COMMAND_KIND_RUN_COMMAND,
                            __Registers.CommandName = KeyMapNodes.Head->V->Name
                        );

                        if (AllowTextHotkeys && IsPrintable) {
                            Text(&Events, HitCharacter);
                            Next = Event->Next;
                        }

                        Take = TRUE;

                        if (Event->Modifier & INPUT_MOD_KIND_ALT)
                            APP_STATE->MenuBarFocusPressStarted = FALSE;
                    }
                } else if (
                    Event->Input >= INPUT_KIND_FUNC_1 && 
                    Event->Input <= INPUT_KIND_FUNC_9
                ) {
                    APP_STATE->MenuBarFocusPressStarted = FALSE;
                }

                AppRequestFrame();
            }

            if (!Take && Event->Kind == EVENT_KIND_TEXT) {
                u8 TextBytes[4] = {};
                Str8 Insertion = Str(
                    TextBytes,
                    UTF8Encode(TextBytes, Event->Character)
                );

                AppCmd(
                    APP_COMMAND_KIND_INSERT_TEXT, 
                    __Registers.String = Insertion
                );
                AppRequestFrame();
                Take = TRUE;

                if (Event->Modifier & INPUT_MOD_KIND_ALT)
                    APP_STATE->MenuBarFocusPressStarted = FALSE;
            }

            if (!Take) {
                Take = TRUE;
                AppCmd(APP_COMMAND_KIND_OS_EVENT, __Registers.OSEvent = Event);
            }

            if (Take)
                ConsumeEvent(&Events, Event);
        }
    }

    APP_STATE->AltMenuBarEnabled = AppSettingFromNameB32(
        "focus_menu_bar_with_alt"_s8
    );

    AppCommand* Command = NULL;

    if (APP_STATE->FrameDepth == 1) {
        for (; AppNextCommand(&Command);) {
            AppRegistersScope() {
                AppCommandKind Kind = AppCommandKindFromStr(Command->Name);

                CopyRegisters(
                    AppFrameMemPool(), 
                    Registers(), 
                    Command->Registers
                );
                AppRequestFrame();

                u64 PanelSiblingOffset = 0;
                u64 PanelChildOffset = 0;
                v2i32 PanelChangeDirection = {};
                UIEvent Event = {};

                switch (Kind) {
                    default: {
                        if (
                            Kind >= APP_COMMAND_KIND_FIRST_TAB_FAST_PATH_CMD
                        ) {
                            u64 FastPathIndex = (
                                Kind - APP_COMMAND_KIND_FIRST_TAB_FAST_PATH_CMD
                            );
                            Str8 ViewName = APP_TAB_FAST_PATH_VIEW_NAME_TABLE[FastPathIndex];
                            b32 NeedsFile = !!(
                                APP_COMMAND_KIND_INFO_TABLE[Kind].Flags &
                                APP_COMMAND_KIND_FLAG_LIST_IN_FILE
                            );

                            if (NeedsFile && !Registers()->FilePath.Size) {
                                AppCmd(APP_COMMAND_KIND_OPEN_FILES);
                            } else {
                                AppCmd(
                                    APP_COMMAND_KIND_BUILD_TAB,
                                    __Registers.String = ViewName,
                                    __Registers.FilePath = NeedsFile 
                                        ? Registers()->FilePath 
                                        : ""_s8
                                );
                            }
                        }
                    } break;

                    case APP_COMMAND_KIND_EXIT: {
                        APP_STATE->ShouldQuit = TRUE;
                    } break;

                    case APP_COMMAND_KIND_OPEN_PALETTE: {
                        AppCmd(
                            APP_COMMAND_KIND_PUSH_QUERY,
                            __Registers.CommandName = ""_s8,
                            __Registers.Expression = "commands"_s8
                        );
                    } break;

                    case APP_COMMAND_KIND_RUN_COMMAND:
                    case APP_COMMAND_KIND_OPEN_TAB: {
                        AppCommandKindInfo* Info = AppCommandKindInfoFromStr(
                            Command->Registers->CommandName
                        );

                        if (!(Info->Query.Kind & APP_QUERY_KIND_REQUIRED)) {
                            AppRegistersScope(__Registers.CommandName = ""_s8) {
                                AppPushCmd(
                                    Command->Registers->CommandName, 
                                    Registers()
                                );
                            }
                        } else {
                            AppCmd(
                                APP_COMMAND_KIND_PUSH_QUERY,
                                __Registers.Expression = Info->Query.Expression
                            );
                        }
                    } break;

                    case APP_COMMAND_KIND_OS_EVENT: {
                        InputEvent* OSEvent = Registers()->OSEvent;

                        if (!OSEvent)
                            break;

                        UIEventKind EventKind = UI_EVENT_KIND_NULL;

                        switch (OSEvent->Kind) {
                            default: {} break;

                            case EVENT_KIND_PRESS: {
                                EventKind = UI_EVENT_KIND_PRESS;
                            } break;

                            case EVENT_KIND_RELEASE: {
                                EventKind = UI_EVENT_KIND_RELEASE;
                            } break;

                            case EVENT_KIND_MOUSE_MOVE: {
                                EventKind = UI_EVENT_KIND_MOUSE_MOVE;
                            } break;

                            case EVENT_KIND_TEXT: {
                                EventKind = UI_EVENT_KIND_TEXT;
                            } break;

                            case EVENT_KIND_SCROLL: {
                                EventKind = UI_EVENT_KIND_SCROLL;
                            } break;

                            case EVENT_KIND_FILE_DROP: {
                                EventKind = UI_EVENT_KIND_FILE_DROP;
                            } break;
                        }

                        if (EventKind != UI_EVENT_KIND_NULL) {
                            Event.Kind = EventKind;
                            Event.Input = OSEvent->Input;
                            Event.Modifiers = OSEvent->Modifier;

                            if (OSEvent->Kind == EVENT_KIND_TEXT) {
                                u8* TextBytes = ArenaPushArrayZero(
                                    AppFrameMemPool(), 
                                    u8, 
                                    4
                                );

                                Event.String = Str(
                                    TextBytes,
                                    UTF8Encode(TextBytes, OSEvent->Character)
                                );
                            }

                            Event.Paths = OSEvent->Strings;
                            Event.Position = OSEvent->Position;
                            Event.DeltaF32 = OSEvent->PositionDelta;
                            Event.TimestampUSecs = OSEvent->TimeStampUSecs;
                        }
                    } break;

                    case APP_COMMAND_KIND_POP_UP_ACCEPT: {
                        APP_STATE->PopUpActive = FALSE;
                        APP_STATE->PopUpKey = {};

                        for (
                            AppCommandNode* N = APP_STATE->PopUpCommands.Head;
                            N;
                            N = N->Next
                        ) {
                            AppPushCmd(N->Command.Name, N->Command.Registers);
                        }
                    } break;

                    case APP_COMMAND_KIND_POP_UP_CANCEL: {
                        APP_STATE->PopUpActive = FALSE;
                        APP_STATE->PopUpKey = {};
                    } break;

                    case APP_COMMAND_KIND_RESET_TO_DEFAULT_BINDINGS: {
                        ConfigNode* Session = ConfigNodeChildFromStr(
                            ConfigNodeRoot(),
                            "session"_s8
                        );
                        ConfigNodePtrList AllInputBindings = ConfigNodeChildListFromStr(
                            Scratch.MemPool,
                            Session,
                            "keybindings"_s8
                        );

                        for (
                            ConfigNodePtrNode* N = AllInputBindings.Head;
                            N;
                            N = N->Next
                        ) {
                            ConfigNodeRelease(APP_STATE->Config, N->V);
                        }

                        ConfigNode* InputBindings = ConfigNodeNew(
                            APP_STATE->Config,
                            Session,
                            "keybindings"_s8
                        );

                        for (
                            u32 Index = 0;
                            Index < ARRAY_COUNT(APP_DEFAULT_BINDING_TABLE);
                            ++Index
                        ) {
                            Str8 Name = APP_DEFAULT_BINDING_TABLE[Index].String;
                            ConfigBinding Binding = APP_DEFAULT_BINDING_TABLE[Index].Binding;
                            ConfigNode* BindingRoot = ConfigNodeNew(
                                APP_STATE->Config,
                                InputBindings,
                                ""_s8
                            );

                            ConfigNodeNew(
                                APP_STATE->Config,
                                BindingRoot,
                                Name
                            );
                            ConfigNodeNew(
                                APP_STATE->Config,
                                BindingRoot,
                                INPUT_DISPLAY_STR_TABLE[Binding.Input]
                            );

                            if (Binding.ModKind & INPUT_MOD_KIND_CTRL) {
                                ConfigNodeNew(
                                    APP_STATE->Config,
                                    BindingRoot,
                                    "ctrl"_s8
                                );
                            }

                            if (Binding.ModKind & INPUT_MOD_KIND_SHIFT) {
                                ConfigNodeNew(
                                    APP_STATE->Config,
                                    BindingRoot,
                                    "shift"_s8
                                );
                            }

                            if (Binding.ModKind & INPUT_MOD_KIND_ALT) {
                                ConfigNodeNew(
                                    APP_STATE->Config,
                                    BindingRoot,
                                    "alt"_s8
                                );
                            }
                        }
                    } break;

                    case APP_COMMAND_KIND_NEW_PANEL_LEFT:
                    case APP_COMMAND_KIND_NEW_PANEL_UP:
                    case APP_COMMAND_KIND_NEW_PANEL_RIGHT:
                    case APP_COMMAND_KIND_NEW_PANEL_DOWN:
                    case APP_COMMAND_KIND_SPLIT_PANEL: {
                        Direction2D SplitDir = DIR_2D_INVALID;
                        ConfigNode* SplitPanel = ConfigNodeFromID(
                            Registers()->Panel
                        );

                        if (Kind == APP_COMMAND_KIND_NEW_PANEL_LEFT) {
                            SplitDir = DIR_2D_LEFT;
                        } else if (Kind == APP_COMMAND_KIND_NEW_PANEL_UP) {
                            SplitDir = DIR_2D_UP;
                        } else if (Kind == APP_COMMAND_KIND_NEW_PANEL_RIGHT) {
                            SplitDir = DIR_2D_RIGHT;
                        } else if (Kind == APP_COMMAND_KIND_NEW_PANEL_DOWN) {
                            SplitDir = DIR_2D_DOWN;
                        } else {
                            SplitDir = Registers()->Direction;
                            SplitPanel = ConfigNodeFromID(Registers()->DstPanel);
                        }

                        if (
                            SplitDir != DIR_2D_INVALID &&
                            SplitPanel != EMPTY_CFG_NODE_VALUE
                        ) {
                            Axis2D SplitAxis = Axis2DFromDir2D(SplitDir);
                            SideKind SplitSide = SideFromDir2D(SplitDir);
                            ConfigNode* NewPanelConfig = EMPTY_CFG_NODE_VALUE;
                            ConfigPanelTree PanelTree = ConfigPanelTreeFromConfig(
                                Scratch.MemPool,
                                SplitPanel
                            );
                            ConfigPanelNode* Panel = ConfigPanelNodeFromConfigTree(
                                PanelTree.Root,
                                SplitPanel
                            );
                            ConfigPanelNode* Parent = Panel->Parent;

                            if (
                                Parent != EMPTY_CFG_PANEL_NODE_VALUE &&
                                Parent->SplitAxis == SplitAxis
                            ) {
                                NewPanelConfig = ConfigNodeAlloc(APP_STATE->Config);
                                ConfigNodeInsertChild(
                                    APP_STATE->Config,
                                    Parent->Config,
                                    (SplitSide == SIDE_MAX)
                                        ? SplitPanel
                                        : SplitPanel->Prev,
                                    NewPanelConfig
                                );
                                ConfigNodeEquipStr(
                                    APP_STATE->Config,
                                    NewPanelConfig,
                                    "%f",
                                    1.0f / (Parent->ChildCount + 1)
                                );

                                for (
                                    ConfigPanelNode* Child = Parent->Head;
                                    Child != EMPTY_CFG_PANEL_NODE_VALUE;
                                    Child = Child->Next
                                ) {
                                    f32 OldPercent = Child->PercentOfParent;
                                    f32 NewPercent = OldPercent * ((f32) Parent->ChildCount / (Parent->ChildCount + 1));

                                    ConfigNodeEquipStr(
                                        APP_STATE->Config,
                                        Child->Config,
                                        "%f",
                                        NewPercent
                                    );
                                }
                            } else {
                                ConfigNode* SplitPanelPrev = Panel->Prev->Config;
                                ConfigNode* NewParent = ConfigNodeAlloc(APP_STATE->Config);
                                ConfigNode* NewSibling = ConfigNodeAlloc(APP_STATE->Config);

                                ConfigNodeEquipStr(
                                    APP_STATE->Config,
                                    NewParent,
                                    SplitPanel->String
                                );
                                ConfigNodeEquipStr(
                                    APP_STATE->Config,
                                    SplitPanel,
                                    "0.5"_s8
                                );
                                ConfigNodeEquipStr(
                                    APP_STATE->Config,
                                    NewSibling,
                                    "0.5"_s8
                                );

                                if (Parent->Config != EMPTY_CFG_NODE_VALUE) {
                                    ConfigNodeUnhook(
                                        APP_STATE->Config,
                                        Parent->Config,
                                        SplitPanel
                                    );
                                    ConfigNodeInsertChild(
                                        APP_STATE->Config,
                                        Parent->Config,
                                        SplitPanelPrev,
                                        NewParent
                                    );
                                } else {
                                    ConfigNode* Window = ConfigWindowFromConfig(
                                        SplitPanel
                                    );

                                    ConfigNodeEquipStr(
                                        APP_STATE->Config,
                                        NewParent,
                                        "panels"_s8
                                    );
                                    ConfigNodeInsertChild(
                                        APP_STATE->Config,
                                        Window,
                                        Window->Tail,
                                        NewParent
                                    );

                                    if (SplitAxis == AXIS_2D_X) {
                                        ConfigNodeChildFromStrOrAlloc(
                                            APP_STATE->Config,
                                            Window,
                                            "split_x"_s8
                                        );
                                    } else {
                                        ConfigNodeRelease(
                                            APP_STATE->Config,
                                            ConfigNodeChildFromStr(
                                                Window,
                                                "split_x"_s8
                                            )
                                        );
                                    }
                                }

                                ConfigNode* Min = SplitPanel;
                                ConfigNode* Max = NewSibling;

                                if (SplitSide == SIDE_MIN)
                                    SWAP(ConfigNode*, Min, Max);

                                ConfigNodeInsertChild(
                                    APP_STATE->Config,
                                    NewParent,
                                    NewParent->Tail,
                                    Min
                                );
                                ConfigNodeInsertChild(
                                    APP_STATE->Config,
                                    NewParent,
                                    NewParent->Tail,
                                    Max
                                );
                                NewPanelConfig = NewSibling;
                            }

                            ConfigNode* DragTab = ConfigNodeFromID(Registers()->View);
                            ConfigNode* OriginPanel = DragTab->Parent;

                            if (
                                Kind == APP_COMMAND_KIND_SPLIT_PANEL &&
                                DragTab != EMPTY_CFG_NODE_VALUE &&
                                OriginPanel != EMPTY_CFG_NODE_VALUE
                            ) {
                                ConfigNodeUnhook(
                                    APP_STATE->Config,
                                    OriginPanel,
                                    DragTab
                                );
                                ConfigNodeInsertChild(
                                    APP_STATE->Config,
                                    NewPanelConfig,
                                    NewPanelConfig->Tail,
                                    DragTab
                                );
                                AppCmd(
                                    APP_COMMAND_KIND_FOCUS_TAB,
                                    __Registers.Tab = DragTab->ID
                                );
                            }

                            AppCmd(
                                APP_COMMAND_KIND_FOCUS_PANEL,
                                __Registers.Panel = NewPanelConfig->ID
                            );

                            if (
                                Kind == APP_COMMAND_KIND_SPLIT_PANEL &&
                                DragTab != EMPTY_CFG_NODE_VALUE &&
                                OriginPanel != EMPTY_CFG_NODE_VALUE
                            ) {
                                ConfigPanelTree OriginPanelTree = ConfigPanelTreeFromConfig(
                                    Scratch.MemPool,
                                    OriginPanel
                                );
                                ConfigPanelNode* OriginPanelNode = ConfigPanelNodeFromConfigTree(
                                    OriginPanelTree.Root,
                                    OriginPanel
                                );

                                if (
                                    OriginPanelNode->SelectedTab == EMPTY_CFG_NODE_VALUE &&
                                    OriginPanelNode->Tabs.Head
                                ) {
                                    AppCmd(
                                        APP_COMMAND_KIND_FOCUS_TAB,
                                        __Registers.Tab = OriginPanelNode->Tabs.Head->V->ID
                                    );
                                }

                                if (
                                    OriginPanel != SplitPanel &&
                                    !OriginPanelNode->Tabs.Count
                                ) {
                                    AppCmd(
                                        APP_COMMAND_KIND_CLOSE_PANEL,
                                        __Registers.Panel = OriginPanel->ID
                                    );
                                }
                            }
                        }
                    } break;

                    case APP_COMMAND_KIND_NEXT_PANEL:
                    case APP_COMMAND_KIND_PREV_PANEL: {
                        if (Kind == APP_COMMAND_KIND_NEXT_PANEL) {
                            PanelSiblingOffset = OFFSETOF(ConfigPanelNode, Next);
                            PanelChildOffset = OFFSETOF(ConfigPanelNode, Head);
                        } else {
                            PanelSiblingOffset = OFFSETOF(ConfigPanelNode, Prev);
                            PanelChildOffset = OFFSETOF(ConfigPanelNode, Tail);
                        }

                        ConfigPanelTree PanelTree = ConfigPanelTreeFromConfig(
                            Scratch.MemPool,
                            ConfigNodeFromID(Registers()->Window)
                        );
                        ConfigPanelNode* NextFocused = EMPTY_CFG_PANEL_NODE_VALUE;

                        for (
                            ConfigPanelNode* P = PanelTree.Focused;
                            P != EMPTY_CFG_PANEL_NODE_VALUE;
                            P = ConfigPanelNodeRecDepthFirst(
                                PanelTree.Root,
                                P,
                                PanelSiblingOffset,
                                PanelChildOffset
                            ).Next
                        ) {
                            if (
                                P != PanelTree.Focused &&
                                P->Head == EMPTY_CFG_PANEL_NODE_VALUE
                            ) {
                                NextFocused = P;
                                break;
                            }
                        }

                        if (NextFocused == EMPTY_CFG_PANEL_NODE_VALUE) {
                            for (
                                ConfigPanelNode* P = PanelTree.Root;
                                P != EMPTY_CFG_PANEL_NODE_VALUE;
                                P = ConfigPanelNodeRecDepthFirst(
                                    PanelTree.Root,
                                    P,
                                    PanelSiblingOffset,
                                    PanelChildOffset
                                ).Next
                            ) {
                                if (
                                    P != PanelTree.Focused &&
                                    P->Head == EMPTY_CFG_PANEL_NODE_VALUE
                                ) {
                                    NextFocused = P;
                                    break;
                                }
                            }
                        }

                        if (NextFocused != EMPTY_CFG_PANEL_NODE_VALUE) {
                            AppCmd(
                                APP_COMMAND_KIND_FOCUS_PANEL,
                                __Registers.Panel = NextFocused->Config->ID
                            );
                        }
                    } break;

                    case APP_COMMAND_KIND_FOCUS_PANEL: {
                        ConfigNode* Panel = ConfigNodeFromID(Registers()->Panel);
                        ConfigPanelTree PanelTree = ConfigPanelTreeFromConfig(
                            Scratch.MemPool,
                            Panel
                        );
                        ConfigNode* SelectionConfig = EMPTY_CFG_NODE_VALUE;

                        for (
                            ConfigPanelNode* P = PanelTree.Root;
                            P != EMPTY_CFG_PANEL_NODE_VALUE;
                            P = ConfigPanelNodeRecDepthFirstPre(PanelTree.Root, P).Next
                        ) {
                            ConfigNode* PConfig = P->Config;
                            ConfigNode* PSelection = ConfigNodeChildFromStr(
                                PConfig,
                                "selected"_s8
                            );

                            if (SelectionConfig == EMPTY_CFG_NODE_VALUE) {
                                SelectionConfig = PSelection;
                            } else {
                                for (
                                    ConfigNode* S = PSelection;
                                    S != EMPTY_CFG_NODE_VALUE;
                                    S = ConfigNodeChildFromStr(
                                        PConfig,
                                        "selected"_s8
                                    )
                                ) {
                                    ConfigNodeRelease(APP_STATE->Config, S);
                                }
                            }
                        }

                        if (Panel != EMPTY_CFG_NODE_VALUE) {
                            if (SelectionConfig == EMPTY_CFG_NODE_VALUE) {
                                SelectionConfig = ConfigNodeAlloc(APP_STATE->Config);
                                ConfigNodeEquipStr(
                                    APP_STATE->Config,
                                    SelectionConfig,
                                    "selected"_s8
                                );
                            }

                            ConfigNodeInsertChild(
                                APP_STATE->Config,
                                Panel,
                                EMPTY_CFG_NODE_VALUE,
                                SelectionConfig
                            );
                            APP_STATE->MenuBarFocused = FALSE;
                        }
                    } break;

                    case APP_COMMAND_KIND_FOCUS_PANEL_RIGHT:
                    case APP_COMMAND_KIND_FOCUS_PANEL_LEFT:
                    case APP_COMMAND_KIND_FOCUS_PANEL_UP:
                    case APP_COMMAND_KIND_FOCUS_PANEL_DOWN: {
                        if (Kind == APP_COMMAND_KIND_FOCUS_PANEL_RIGHT)
                            PanelChangeDirection = Vec(1, 0);
                        else if (Kind == APP_COMMAND_KIND_FOCUS_PANEL_LEFT)
                            PanelChangeDirection = Vec(-1, 0);
                        else if (Kind == APP_COMMAND_KIND_FOCUS_PANEL_UP)
                            PanelChangeDirection = Vec(0, -1);
                        else
                            PanelChangeDirection = Vec(0, 1);

                        ConfigNode* Window = ConfigNodeFromID(
                            Registers()->Window
                        );
                        ConfigPanelTree PanelTree = ConfigPanelTreeFromConfig(Scratch.MemPool, Window);
                        ConfigPanelNode* SourcePanel = PanelTree.Focused;
                        r2f32 SourcePanelRect = ConfigTargetRectFromPanelNode(
                            Rng(
                                Vec(0.0f, 0.0f),
                                Vec(1000.0f, 1000.0f)
                            ),
                            PanelTree.Root,
                            SourcePanel
                        );
                        v2f32 SourcePanelCentre = Centre(SourcePanelRect);
                        v2f32 SourcePanelHalfDim = Length(SourcePanelRect) * 0.5f;
                        v2f32 TravelDim = SourcePanelHalfDim + Vec(10.0f, 10.0f);
                        v2f32 TravelDistance = (
                            SourcePanelCentre +
                            (
                                TravelDim * Vec(
                                    (f32) PanelChangeDirection.X,
                                    (f32) PanelChangeDirection.Y
                                )
                            )
                        );
                        ConfigPanelNode* DestinationRoot = EMPTY_CFG_PANEL_NODE_VALUE;

                        for (
                            ConfigPanelNode* P = PanelTree.Root;
                            P != EMPTY_CFG_PANEL_NODE_VALUE;
                            P = ConfigPanelNodeRecDepthFirstPre(PanelTree.Root, P).Next
                        ) {
                            if (P == SourcePanel || P->Head != EMPTY_CFG_PANEL_NODE_VALUE)
                                continue;

                            r2f32 PRect = ConfigTargetRectFromPanelNode(
                                Rng(
                                    Vec(0.0f, 0.0f),
                                    Vec(1000.0f, 1000.0f)
                                ),
                                PanelTree.Root,
                                P
                            );

                            if (InRange(PRect, TravelDistance)) {
                                DestinationRoot = P;
                                break;
                            }
                        }

                        if (DestinationRoot != EMPTY_CFG_PANEL_NODE_VALUE) {
                            ConfigPanelNode* DestinationPanel = EMPTY_CFG_PANEL_NODE_VALUE;

                            for (
                                ConfigPanelNode* P = DestinationRoot;
                                P != EMPTY_CFG_PANEL_NODE_VALUE;
                                P = ConfigPanelNodeRecDepthFirstPre(DestinationRoot, P).Next
                            ) {
                                if (P->Head == EMPTY_CFG_PANEL_NODE_VALUE && P != SourcePanel) {
                                    DestinationPanel = P;
                                    break;
                                }
                            }

                            AppCmd(
                                APP_COMMAND_KIND_FOCUS_PANEL,
                                __Registers.Panel = DestinationPanel->Config->ID
                            );
                        }
                    } break;

                    case APP_COMMAND_KIND_CLOSE_PANEL: {
                        ConfigNode* Window = ConfigNodeFromID(Registers()->Window);
                        ConfigPanelTree PanelTree = ConfigPanelTreeFromConfig(
                            Scratch.MemPool,
                            Window
                        );
                        ConfigPanelNode* Panel = ConfigPanelNodeFromConfigTree(
                            PanelTree.Root,
                            ConfigNodeFromID(Registers()->Panel)
                        );
                        ConfigPanelNode* Parent = Panel->Parent;

                        if (Parent != EMPTY_CFG_PANEL_NODE_VALUE) {
                            ConfigPanelNode* NewFocused = EMPTY_CFG_PANEL_NODE_VALUE;

                            if (Parent->ChildCount == 2) {
                                ConfigPanelNode* KeepChild = (Panel == Parent->Head)
                                    ? Parent->Tail
                                    : Parent->Head;
                                ConfigPanelNode* Grandparent = Parent->Parent;
                                ConfigPanelNode* ParentPrev = Parent->Prev;
                                f32 PercentOfParent = Parent->PercentOfParent;

                                ConfigNodeUnhook(
                                    APP_STATE->Config,
                                    Parent->Config,
                                    KeepChild->Config
                                );
                                ConfigNodeUnhook(
                                    APP_STATE->Config,
                                    (Grandparent != EMPTY_CFG_PANEL_NODE_VALUE)
                                        ? Grandparent->Config
                                        : Window,
                                    Parent->Config
                                );
                                ConfigNodeRelease(
                                    APP_STATE->Config,
                                    Parent->Config
                                );

                                if (Grandparent == EMPTY_CFG_PANEL_NODE_VALUE) {
                                    if (KeepChild->SplitAxis == AXIS_2D_X) {
                                        ConfigNodeChildFromStrOrAlloc(
                                            APP_STATE->Config,
                                            Window,
                                            "split_x"_s8
                                        );
                                    } else {
                                        ConfigNodeRelease(
                                            APP_STATE->Config,
                                            ConfigNodeChildFromStr(
                                                Window,
                                                "split_x"_s8
                                            )
                                        );
                                    }

                                    ConfigNodeEquipStr(
                                        APP_STATE->Config,
                                        KeepChild->Config,
                                        "panels"_s8
                                    );
                                    ConfigNodeInsertChild(
                                        APP_STATE->Config,
                                        Window,
                                        Window->Tail,
                                        KeepChild->Config
                                    );
                                } else {
                                    ConfigNodeInsertChild(
                                        APP_STATE->Config,
                                        Grandparent->Config,
                                        ParentPrev->Config,
                                        KeepChild->Config
                                    );
                                    ConfigNodeEquipStr(
                                        APP_STATE->Config,
                                        KeepChild->Config,
                                        "%f",
                                        PercentOfParent
                                    );
                                }

                                if (
                                    Grandparent != EMPTY_CFG_PANEL_NODE_VALUE &&
                                    Grandparent->SplitAxis == KeepChild->SplitAxis &&
                                    KeepChild->Head != EMPTY_CFG_PANEL_NODE_VALUE
                                ) {
                                    ConfigNodeUnhook(
                                        APP_STATE->Config,
                                        Grandparent->Config,
                                        KeepChild->Config
                                    );

                                    ConfigNode* Prev = ParentPrev->Config;

                                    for (
                                        ConfigPanelNode* Child = KeepChild->Head;
                                        Child != EMPTY_CFG_PANEL_NODE_VALUE;
                                        Child = Child->Next
                                    ) {
                                        ConfigNodeUnhook(
                                            APP_STATE->Config,
                                            KeepChild->Config,
                                            Child->Config
                                        );
                                        ConfigNodeInsertChild(
                                            APP_STATE->Config,
                                            Grandparent->Config,
                                            Prev,
                                            Child->Config
                                        );
                                        ConfigNodeEquipStr(
                                            APP_STATE->Config,
                                            Child->Config,
                                            "%f",
                                            Child->PercentOfParent * PercentOfParent
                                        );
                                        Prev = Child->Config;
                                    }

                                    ConfigNodeRelease(
                                        APP_STATE->Config,
                                        KeepChild->Config
                                    );
                                }

                                NewFocused = KeepChild;
                            } else {
                                f32 RemovedSizePercent = Panel->PercentOfParent;

                                NewFocused = (Panel->Prev != EMPTY_CFG_PANEL_NODE_VALUE)
                                    ? Panel->Prev
                                    : Panel->Next;
                                ConfigNodeUnhook(
                                    APP_STATE->Config,
                                    Parent->Config,
                                    Panel->Config
                                );
                                ConfigNodeRelease(
                                    APP_STATE->Config,
                                    Panel->Config
                                );

                                for (
                                    ConfigPanelNode* Child = Parent->Head;
                                    Child != EMPTY_CFG_PANEL_NODE_VALUE;
                                    Child = Child->Next
                                ) {
                                    if (Child == Panel)
                                        continue;

                                    ConfigNodeEquipStr(
                                        APP_STATE->Config,
                                        Child->Config,
                                        "%f",
                                        Child->PercentOfParent / (1.0f - RemovedSizePercent)
                                    );
                                }
                            }

                            if (PanelTree.Focused == Panel) {
                                for (
                                    ConfigPanelNode* Child = NewFocused->Head;
                                    Child != EMPTY_CFG_PANEL_NODE_VALUE;
                                    Child = Child->Head
                                ) {
                                    NewFocused = Child;
                                }

                                AppCmd(
                                    APP_COMMAND_KIND_FOCUS_PANEL,
                                    __Registers.Panel = NewFocused->Config->ID
                                );
                            }
                        }
                    } break;

                    case APP_COMMAND_KIND_FOCUS_TAB: {
                        ConfigNode* Tab = ConfigNodeFromID(Registers()->Tab);
                        ConfigNode* Panel = Tab->Parent;
                        ConfigPanelTree PanelTree = ConfigPanelTreeFromConfig(
                            Scratch.MemPool,
                            Panel
                        );
                        ConfigPanelNode* PanelNode = ConfigPanelNodeFromConfigTree(
                            PanelTree.Root,
                            Panel
                        );
                        ConfigNode* SelectionConfig = EMPTY_CFG_NODE_VALUE;

                        for (
                            ConfigNodePtrNode* N = PanelNode->Tabs.Head;
                            N;
                            N = N->Next
                        ) {
                            ConfigNode* TabSelectionConfig = ConfigNodeChildFromStr(
                                N->V,
                                "selected"_s8
                            );

                            if (SelectionConfig == EMPTY_CFG_NODE_VALUE) {
                                SelectionConfig = TabSelectionConfig;
                            } else {
                                for (
                                    ConfigNode* S = TabSelectionConfig;
                                    S != EMPTY_CFG_NODE_VALUE;
                                    S = ConfigNodeChildFromStr(N->V, "selected"_s8)
                                ) {
                                    ConfigNodeRelease(APP_STATE->Config, S);
                                }
                            }
                        }

                        if (Tab != EMPTY_CFG_NODE_VALUE) {
                            if (SelectionConfig == EMPTY_CFG_NODE_VALUE) {
                                SelectionConfig = ConfigNodeAlloc(APP_STATE->Config);
                                ConfigNodeEquipStr(
                                    APP_STATE->Config,
                                    SelectionConfig,
                                    "selected"_s8
                                );
                            }

                            ConfigNodeInsertChild(
                                APP_STATE->Config,
                                Tab,
                                EMPTY_CFG_NODE_VALUE,
                                SelectionConfig
                            );
                        }
                    } break;

                    case APP_COMMAND_KIND_NEXT_TAB: {
                        ConfigNode* Window = ConfigNodeFromID(Registers()->Window);
                        ConfigPanelTree PanelTree = ConfigPanelTreeFromConfig(
                            Scratch.MemPool,
                            Window
                        );
                        ConfigPanelNode* Focused = PanelTree.Focused;
                        ConfigNodePtrNode* SelectedTabNode = NULL;

                        for (
                            ConfigNodePtrNode* N = Focused->Tabs.Head;
                            N;
                            N = N->Next
                        ) {
                            if (N->V == Focused->SelectedTab) {
                                SelectedTabNode = N;
                                break;
                            }
                        }

                        ConfigNode* NextSelectedTab = EMPTY_CFG_NODE_VALUE;
                        u64 Index = 0;

                        for (
                            ConfigNodePtrNode* TabNode = SelectedTabNode;
                            TabNode && (
                                TabNode != SelectedTabNode || 
                                !Index
                            );
                            (
                                (!TabNode->Next)
                                ? TabNode = Focused->Tabs.Head
                                : TabNode = TabNode->Next
                            ), ++Index
                        ) {
                            if (TabNode != SelectedTabNode) {
                                NextSelectedTab = TabNode->V;
                                break;
                            }
                        }

                        if (NextSelectedTab != EMPTY_CFG_NODE_VALUE) {
                            AppCmd(
                                APP_COMMAND_KIND_FOCUS_TAB, 
                                __Registers.Tab = NextSelectedTab->ID
                            );
                        }
                    } break;

                    case APP_COMMAND_KIND_PREV_TAB: {
                        ConfigNode* Window = ConfigNodeFromID(
                            Registers()->Window
                        );
                        ConfigPanelTree PanelTree = ConfigPanelTreeFromConfig(
                            Scratch.MemPool, 
                            Window
                        );
                        ConfigPanelNode* Focused = PanelTree.Focused;
                        ConfigNodePtrNode* SelectedTabNode = NULL;

                        for (
                            ConfigNodePtrNode* N = Focused->Tabs.Tail; 
                            N; 
                            N = N->Prev
                        ) {
                            if (N->V == Focused->SelectedTab) {
                                SelectedTabNode = N;
                                break;
                            }
                        }

                        ConfigNode* NextSelectedTab = EMPTY_CFG_NODE_VALUE;
                        u64 Index = 0;

                        for (
                            ConfigNodePtrNode* TabNode = SelectedTabNode;
                            TabNode && (TabNode != SelectedTabNode || !Index);
                            (
                                (!TabNode->Prev)
                                ? TabNode = Focused->Tabs.Tail
                                : TabNode = TabNode->Prev
                            ), ++Index
                        ) {
                            if (TabNode != SelectedTabNode) {
                                NextSelectedTab = TabNode->V;
                                break;
                            }
                        }

                        if (NextSelectedTab != EMPTY_CFG_NODE_VALUE) {
                            AppCmd(
                                APP_COMMAND_KIND_FOCUS_TAB, 
                                __Registers.Tab = NextSelectedTab->ID
                            );
                        }
                    } break;

                    case APP_COMMAND_KIND_MOVE_TAB_RIGHT:
                    case APP_COMMAND_KIND_MOVE_TAB_LEFT: {
                        ConfigNode* Tab = ConfigNodeFromID(Registers()->Tab);
                        ConfigNode* PanelConfig = Tab->Parent;
                        ConfigPanelTree PanelTree = ConfigPanelTreeFromConfig(
                            Scratch.MemPool, 
                            PanelConfig
                        );
                        ConfigPanelNode* Panel = ConfigPanelNodeFromConfigTree(
                            PanelTree.Root, 
                            PanelConfig
                        );
                        ConfigNode* TabPrev2 = EMPTY_CFG_NODE_VALUE;
                        ConfigNode* TabPrev = EMPTY_CFG_NODE_VALUE;
                        ConfigNode* TabNext = EMPTY_CFG_NODE_VALUE;
                        ConfigNode* Prev2 = EMPTY_CFG_NODE_VALUE;
                        ConfigNode* Prev = EMPTY_CFG_NODE_VALUE;
                        ConfigNode* Next = EMPTY_CFG_NODE_VALUE;

                        for (
                            ConfigNodePtrNode* N = Panel->Tabs.Head;
                            N;
                            (
                                Prev2 = Prev,
                                Prev = N->V,
                                N = N->Next
                            )
                        ) {
                            Next = (N->Next) ? N->Next->V : EMPTY_CFG_NODE_VALUE;

                            if (N->V == Tab) {
                                TabPrev2 = Prev2;
                                TabPrev = Prev;
                                TabNext = Next;
                                break;
                            }
                        }

                        ConfigNode* NewPrev = (Kind == APP_COMMAND_KIND_MOVE_TAB_RIGHT)
                            ? TabNext
                            : TabPrev2;

                        if (NewPrev == TabPrev && Panel->Tabs.Tail)
                            NewPrev = Panel->Tabs.Tail->V;

                        if (Tab != EMPTY_CFG_NODE_VALUE && NewPrev != Tab) {
                            ConfigNodeUnhook(APP_STATE->Config, PanelConfig, Tab);
                            ConfigNodeInsertChild(
                                APP_STATE->Config,
                                PanelConfig,
                                NewPrev,
                                Tab
                            );
                        }
                    } break;

                    case APP_COMMAND_KIND_BUILD_TAB: {
                        ConfigNode* Panel = ConfigNodeFromID(Registers()->Panel);
                        ConfigNode* Tab = EMPTY_CFG_NODE_VALUE;

                        for (
                            ConfigNode* Child = Panel->Head;
                            Child != EMPTY_CFG_NODE_VALUE;
                            Child = Child->Next
                        ) {
                            if (
                                StrMatch(Child->String, Registers()->String, 0) &&
                                StrMatch(
                                    ConfigNodeChildFromStr(
                                        Child, 
                                        "file"_s8
                                    )->Head->String,
                                    Registers()->FilePath,
                                    0
                                )
                            ) {
                                Tab = Child;
                                break;
                            }
                        }

                        if (
                            Tab == EMPTY_CFG_NODE_VALUE && 
                            Panel != EMPTY_CFG_NODE_VALUE
                        ) {
                            Tab = ConfigNodeNew(
                                APP_STATE->Config,
                                Panel,
                                Registers()->String
                            );

                            if (Registers()->FilePath.Size) {
                                ConfigNode* File = ConfigNodeNew(
                                    APP_STATE->Config,
                                    Tab,
                                    "file"_s8
                                );

                                ConfigNodeNew(
                                    APP_STATE->Config,
                                    File,
                                    Registers()->FilePath
                                );
                            }
                        }

                        if (Tab != EMPTY_CFG_NODE_VALUE) {
                            AppCmd(
                                APP_COMMAND_KIND_FOCUS_TAB, 
                                __Registers.Tab = Tab->ID
                            );
                        }
                    } break;

                    case APP_COMMAND_KIND_DUPLICATE_TAB: {
                        ConfigNode* Source = ConfigNodeFromID(Registers()->Tab);

                        if (Source != EMPTY_CFG_NODE_VALUE) {
                            ConfigNode* Destination = ConfigNodeCpy(
                                APP_STATE->Config,
                                Source
                            );

                            ConfigNodeInsertChild(
                                APP_STATE->Config,
                                Source->Parent,
                                Source,
                                Destination
                            );
                            AppCmd(
                                APP_COMMAND_KIND_FOCUS_TAB,
                                __Registers.Tab = Destination->ID
                            );
                        }
                    } break;

                    case APP_COMMAND_KIND_MOVE_VIEW: {
                        ConfigNode* View = ConfigNodeFromID(Registers()->View);
                        ConfigNode* PrevTab = ConfigNodeFromID(Registers()->PrevTab);
                        ConfigNode* SrcPanel = View->Parent;
                        ConfigNode* DstPanel = ConfigNodeFromID(Registers()->DstPanel);

                        if (
                            View != EMPTY_CFG_NODE_VALUE &&
                            DstPanel != EMPTY_CFG_NODE_VALUE &&
                            PrevTab != View
                        ) {
                            ConfigNodeUnhook(APP_STATE->Config, SrcPanel, View);
                            ConfigNodeInsertChild(
                                APP_STATE->Config,
                                DstPanel,
                                PrevTab,
                                View
                            );
                            AppCmd(
                                APP_COMMAND_KIND_FOCUS_TAB,
                                __Registers.Tab = View->ID
                            );
                            AppCmd(
                                APP_COMMAND_KIND_FOCUS_PANEL,
                                __Registers.Panel = DstPanel->ID
                            );

                            if (SrcPanel != DstPanel) {
                                ConfigPanelTree SrcPanelTree = ConfigPanelTreeFromConfig(
                                    Scratch.MemPool,
                                    SrcPanel
                                );
                                ConfigPanelNode* SrcPanelNode = ConfigPanelNodeFromConfigTree(
                                    SrcPanelTree.Root,
                                    SrcPanel
                                );

                                if (!SrcPanelNode->Tabs.Count) {
                                    AppCmd(
                                        APP_COMMAND_KIND_CLOSE_PANEL,
                                        __Registers.Panel = SrcPanel->ID
                                    );
                                } else if (SrcPanelNode->SelectedTab == EMPTY_CFG_NODE_VALUE) {
                                    AppCmd(
                                        APP_COMMAND_KIND_FOCUS_TAB,
                                        __Registers.Tab = SrcPanelNode->Tabs.Head->V->ID
                                    );
                                }
                            }
                        }
                    } break;

                    case APP_COMMAND_KIND_CLOSE_TAB: {
                        ConfigNode* Tab = ConfigNodeFromID(Registers()->Tab);
                        ConfigPanelTree PanelTree = ConfigPanelTreeFromConfig(
                            Scratch.MemPool, 
                            Tab
                        );
                        ConfigPanelNode* Panel = ConfigPanelNodeFromConfigTree(
                            PanelTree.Root, 
                            Tab->Parent
                        );

                        if (Panel->SelectedTab == Tab) {
                            b32 FoundSelected = FALSE;
                            ConfigNode* NextSelectedTab = EMPTY_CFG_NODE_VALUE;

                            for (
                                ConfigNodePtrNode* N = Panel->Tabs.Head;
                                N;
                                N = N->Next
                            ) {
                                if (N->V == Panel->SelectedTab) {
                                    FoundSelected = TRUE;
                                } else {
                                    NextSelectedTab = N->V;

                                    if (FoundSelected)
                                        break;
                                }
                            }

                            if (NextSelectedTab != EMPTY_CFG_NODE_VALUE) {
                                AppCmd(
                                    APP_COMMAND_KIND_FOCUS_TAB, 
                                    __Registers.Tab = NextSelectedTab->ID
                                );
                            }
                        }

                        if (Tab != EMPTY_CFG_NODE_VALUE)
                            ConfigNodeRelease(APP_STATE->Config, Tab);
                    } break;

                    case APP_COMMAND_KIND_SET_TAB_VIEW: {
                        ConfigNode* Tab = ConfigNodeFromID(Registers()->Tab);

                        if (
                            Tab != EMPTY_CFG_NODE_VALUE &&
                            Registers()->String.Size &&
                            !StrMatch(Tab->String, Registers()->String, 0)
                        ) {
                            AppViewState* VS = AppViewStateFromConfig(Tab);

                            ConfigNodeEquipStr(
                                APP_STATE->Config,
                                Tab,
                                Registers()->String
                            );

                            for (
                                AppArenaExt* Ext = VS->HeadArenaExt;
                                Ext;
                                Ext = Ext->Next
                            ) {
                                ArenaRelease(Ext->MemPool);
                            }

                            ArenaPopTo(VS->MemPool, VS->MemPoolResetPosition);
                            VS->UserData = NULL;
                            VS->HeadArenaExt = NULL;
                            VS->TailArenaExt = NULL;
                            VS->ScrollPosition = {};
                            VS->QueryIsOpen = FALSE;
                        }
                    } break;

                    case APP_COMMAND_KIND_SET_TAB_FILE: {
                        ConfigNode* Tab = ConfigNodeFromID(Registers()->Tab);
                        ConfigNode* TabFile = ConfigNodeChildFromStr(Tab, "file"_s8);
                        Str8 Path = StrTrimLastSlash(
                            PathAbsoluteDstFromRelativeDstSrc(
                                Scratch.MemPool,
                                Registers()->FilePath,
                                GetCurrentPath(Scratch.MemPool)
                            )
                        );
                        FileProperties Props = SystemGetFileProperties(Path);
                        b32 IsFolder = !!(Props.Kind & SYS_FILE_IS_DIR);

                        if (!Props.Created || IsFolder) {
                            Str8 Error = IsFolder
                                ? ArenaPushStrFmt(
                                    Scratch.MemPool,
                                    "FUME cannot show \"%S\" in a tab. It is a folder.",
                                    PRINT_STR(Path)
                                )
                                : ArenaPushStrFmt(
                                    Scratch.MemPool,
                                    "FUME cannot open \"%S\". The file does not exist.",
                                    PRINT_STR(Path)
                                );

                            APP_STATE->ErrorStrSize = MIN(
                                sizeof(APP_STATE->ErrorBuffer),
                                Error.Size
                            );
                            MemCpy(
                                APP_STATE->ErrorBuffer,
                                Error.Str,
                                APP_STATE->ErrorStrSize
                            );
                            APP_STATE->ErrorFrameIndex = APP_STATE->FrameIndex;
                        } else if (TabFile == EMPTY_CFG_NODE_VALUE) {
                            AppCmd(
                                APP_COMMAND_KIND_OPEN,
                                __Registers.FilePath = Path
                            );
                        } else if (!PathMatchNormalised(TabFile->Head->String, Path)) {
                            ConfigNode* Session = ConfigNodeChildFromStr(
                                ConfigNodeRoot(),
                                "session"_s8
                            );
                            AppViewState* VS = AppViewStateFromConfig(Tab);
                            b32 IsRecorded = FALSE;

                            ConfigNodeNewReplace(APP_STATE->Config, TabFile, Path);

                            for (
                                ConfigNode* Child = Session->Head;
                                Child != EMPTY_CFG_NODE_VALUE;
                                Child = Child->Next
                            ) {
                                if (
                                    StrMatch(Child->String, "file"_s8, 0) &&
                                    PathMatchNormalised(Child->Head->String, Path)
                                ) {
                                    IsRecorded = TRUE;
                                    break;
                                }
                            }

                            if (!IsRecorded) {
                                ConfigNode* File = ConfigNodeNew(
                                    APP_STATE->Config,
                                    Session,
                                    "file"_s8
                                );

                                ConfigNodeNew(APP_STATE->Config, File, Path);
                            }

                            for (
                                AppArenaExt* Ext = VS->HeadArenaExt;
                                Ext;
                                Ext = Ext->Next
                            ) {
                                ArenaRelease(Ext->MemPool);
                            }

                            ArenaPopTo(VS->MemPool, VS->MemPoolResetPosition);
                            VS->UserData = NULL;
                            VS->HeadArenaExt = NULL;
                            VS->TailArenaExt = NULL;
                            VS->ScrollPosition = {};
                            AppCmd(
                                APP_COMMAND_KIND_SET_CURRENT_PATH,
                                __Registers.FilePath = StrChopLastSlash(Path)
                            );
                        }
                    } break;

                    case APP_COMMAND_KIND_SET_CURRENT_PATH: {
                        ConfigNode* Session = ConfigNodeChildFromStr(
                            ConfigNodeRoot(), 
                            "session"_s8
                        );
                        ConfigNode* CurrentPath = ConfigNodeChildFromStrOrAlloc(
                            APP_STATE->Config,
                            Session,
                            "current_path"_s8
                        );

                        ConfigNodeNewReplace(
                            APP_STATE->Config,
                            CurrentPath,
                            Registers()->FilePath
                        );
                    } break;

                    case APP_COMMAND_KIND_OPEN: {
                        Str8 Path = StrTrimLastSlash(
                            PathAbsoluteDstFromRelativeDstSrc(
                                Scratch.MemPool,
                                Registers()->FilePath,
                                GetCurrentPath(Scratch.MemPool)
                            )
                        );
                        FileProperties Props = SystemGetFileProperties(Path);
                        b32 IsFolder = !!(Props.Kind & SYS_FILE_IS_DIR);
                        Str8List Paths = {};

                        if (IsFolder) {
                            FileIter* Iter = SystemFileIterBegin(
                                Scratch.MemPool,
                                Path,
                                SYS_FILE_ITER_SKIP_DIRS
                            );
                            FileInfo Info = {};

                            while (SystemFileIterNext(Scratch.MemPool, Iter, &Info)) {
                                if (Info.Properties.Kind & SYS_FILE_IS_DIR)
                                    continue;

                                // NOTE(nc): we only care about *.tlog or *.TLOG files
                                if (
                                    !StrMatch(
                                        StrSkipLastDot(Info.Name),
                                        "tlog"_s8,
                                        STR_MATCH_ALL_CASES
                                    )
                                ) {
                                    continue;
                                }

                                ListPushFmt(
                                    Scratch.MemPool,
                                    &Paths,
                                    "%S/%S",
                                    PRINT_STR(Path),
                                    PRINT_STR(Info.Name)
                                );
                            }

                            SystemFileIterEnd(Iter);
                        } else if (Props.Created) {
                            ListPush(Scratch.MemPool, &Paths, Path);
                        }

                        ConfigNode* Session = ConfigNodeChildFromStr(
                            ConfigNodeRoot(), 
                            "session"_s8
                        );

                        for (
                            Str8Node* Node = Paths.Head; 
                            Node; 
                            Node = Node->Next
                        ) {
                            b32 IsRecorded = FALSE;

                            for (
                                ConfigNode* Child = Session->Head;
                                Child != EMPTY_CFG_NODE_VALUE;
                                Child = Child->Next
                            ) {
                                if (
                                    StrMatch(Child->String, "file"_s8, 0) &&
                                    PathMatchNormalised(
                                        Child->Head->String, 
                                        Node->String
                                    )
                                ) {
                                    IsRecorded = TRUE;
                                    break;
                                }
                            }

                            if (!IsRecorded) {
                                ConfigNode* File = ConfigNodeNew(
                                    APP_STATE->Config,
                                    Session,
                                    "file"_s8
                                );

                                ConfigNodeNew(
                                    APP_STATE->Config, 
                                    File, 
                                    Node->String
                                );
                            }
                        }

                        if (!Paths.Count) {
                            Str8 Error = {};

                            if (IsFolder) {
                                Error = ArenaPushStrFmt(
                                    Scratch.MemPool,
                                    "FUME found no tlog files in \"%S\".",
                                    PRINT_STR(Path)
                                );
                            } else {
                                Error = ArenaPushStrFmt(
                                    Scratch.MemPool,
                                    "FUME cannot open \"%S\". The file does not exist.",
                                    PRINT_STR(Path)
                                );
                            }

                            APP_STATE->ErrorStrSize = MIN(
                                sizeof(APP_STATE->ErrorBuffer), 
                                Error.Size
                            );
                            MemCpy(
                                APP_STATE->ErrorBuffer, 
                                Error.Str, 
                                APP_STATE->ErrorStrSize
                            );
                            APP_STATE->ErrorFrameIndex = APP_STATE->FrameIndex;
                        } else if (IsFolder) {
                            AppCmd(APP_COMMAND_KIND_OPEN_FILES);
                            AppCmd(
                                APP_COMMAND_KIND_SET_CURRENT_PATH, 
                                __Registers.FilePath = Path
                            );
                        } else {
                            AppCmd(
                                APP_COMMAND_KIND_BUILD_TAB,
                                __Registers.String = "pending"_s8,
                                __Registers.FilePath = Path
                            );
                            AppCmd(
                                APP_COMMAND_KIND_SET_CURRENT_PATH,
                                __Registers.FilePath = StrChopLastSlash(Path)
                            );
                        }
                    } break;

                    case APP_COMMAND_KIND_EDIT: {
                        Event.Kind = UI_EVENT_KIND_PRESS;
                        Event.Slot = UI_EVENT_ACTION_SLOT_EDIT;
                    } break;

                    case APP_COMMAND_KIND_ACCEPT: {
                        Event.Kind = UI_EVENT_KIND_PRESS;
                        Event.Slot = UI_EVENT_ACTION_SLOT_ACCEPT;
                    } break;

                    case APP_COMMAND_KIND_CANCEL: {
                        Event.Kind = UI_EVENT_KIND_PRESS;
                        Event.Slot = UI_EVENT_ACTION_SLOT_CANCEL;
                    } break;

                    case APP_COMMAND_KIND_MOVE_LEFT: {
                        Event.Kind = UI_EVENT_KIND_NAVIGATE;
                        Event.Flags = UI_EVENT_FLAG_PICK_SELECT_SIDE | UI_EVENT_FLAG_ZERO_DELTA_ON_SELECT | UI_EVENT_FLAG_EXPLICIT_DIRECTIONAL;
                        Event.DeltaStride = UI_EVENT_DELTA_STRIDE_CHAR;
                        Event.DeltaI32 = Vec(-1, 0);
                    } break;

                    case APP_COMMAND_KIND_MOVE_RIGHT: {
                        Event.Kind = UI_EVENT_KIND_NAVIGATE;
                        Event.Flags = UI_EVENT_FLAG_PICK_SELECT_SIDE | UI_EVENT_FLAG_ZERO_DELTA_ON_SELECT | UI_EVENT_FLAG_EXPLICIT_DIRECTIONAL;
                        Event.DeltaStride = UI_EVENT_DELTA_STRIDE_CHAR;
                        Event.DeltaI32 = Vec(1, 0);
                    } break;

                    case APP_COMMAND_KIND_MOVE_UP: {
                        Event.Kind = UI_EVENT_KIND_NAVIGATE;
                        Event.Flags = UI_EVENT_FLAG_EXPLICIT_DIRECTIONAL | UI_EVENT_FLAG_SECONDARY;
                        Event.DeltaStride = UI_EVENT_DELTA_STRIDE_CHAR;
                        Event.DeltaI32 = Vec(0, -1);
                    } break;

                    case APP_COMMAND_KIND_MOVE_DOWN: {
                        Event.Kind = UI_EVENT_KIND_NAVIGATE;
                        Event.Flags = UI_EVENT_FLAG_EXPLICIT_DIRECTIONAL | UI_EVENT_FLAG_SECONDARY;
                        Event.DeltaStride = UI_EVENT_DELTA_STRIDE_CHAR;
                        Event.DeltaI32 = Vec(0, 1);
                    } break;

                    case APP_COMMAND_KIND_MOVE_LEFT_SELECT: {
                        Event.Kind = UI_EVENT_KIND_NAVIGATE;
                        Event.Flags = UI_EVENT_FLAG_SECONDARY | UI_EVENT_FLAG_KEEP_MARK;
                        Event.DeltaStride = UI_EVENT_DELTA_STRIDE_CHAR;
                        Event.DeltaI32 = Vec(-1, 0);
                    } break;

                    case APP_COMMAND_KIND_MOVE_RIGHT_SELECT: {
                        Event.Kind = UI_EVENT_KIND_NAVIGATE;
                        Event.Flags = UI_EVENT_FLAG_SECONDARY | UI_EVENT_FLAG_KEEP_MARK;
                        Event.DeltaStride = UI_EVENT_DELTA_STRIDE_CHAR;
                        Event.DeltaI32 = Vec(1, 0);
                    } break;

                    case APP_COMMAND_KIND_MOVE_UP_SELECT: {
                        Event.Kind = UI_EVENT_KIND_NAVIGATE;
                        Event.Flags = UI_EVENT_FLAG_EXPLICIT_DIRECTIONAL | UI_EVENT_FLAG_SECONDARY | UI_EVENT_FLAG_KEEP_MARK;
                        Event.DeltaStride = UI_EVENT_DELTA_STRIDE_CHAR;
                        Event.DeltaI32 = Vec(0, -1);
                    } break;

                    case APP_COMMAND_KIND_MOVE_DOWN_SELECT: {
                        Event.Kind = UI_EVENT_KIND_NAVIGATE;
                        Event.Flags = UI_EVENT_FLAG_EXPLICIT_DIRECTIONAL | UI_EVENT_FLAG_SECONDARY | UI_EVENT_FLAG_KEEP_MARK;
                        Event.DeltaStride = UI_EVENT_DELTA_STRIDE_CHAR;
                        Event.DeltaI32 = Vec(0, 1);
                    } break;

                    case APP_COMMAND_KIND_MOVE_LEFT_CHUNK: {
                        Event.Kind = UI_EVENT_KIND_NAVIGATE;
                        Event.Flags = UI_EVENT_FLAG_EXPLICIT_DIRECTIONAL;
                        Event.DeltaStride = UI_EVENT_DELTA_STRIDE_WORD;
                        Event.DeltaI32 = Vec(-1, 0);
                    } break;

                    case APP_COMMAND_KIND_MOVE_RIGHT_CHUNK: {
                        Event.Kind = UI_EVENT_KIND_NAVIGATE;
                        Event.Flags = UI_EVENT_FLAG_EXPLICIT_DIRECTIONAL;
                        Event.DeltaStride = UI_EVENT_DELTA_STRIDE_WORD;
                        Event.DeltaI32 = Vec(1, 0);
                    } break;

                    case APP_COMMAND_KIND_MOVE_UP_CHUNK: {
                        Event.Kind = UI_EVENT_KIND_NAVIGATE;
                        Event.Flags = UI_EVENT_FLAG_EXPLICIT_DIRECTIONAL | UI_EVENT_FLAG_SECONDARY;
                        Event.DeltaStride = UI_EVENT_DELTA_STRIDE_WORD;
                        Event.DeltaI32 = Vec(0, -1);
                    } break;

                    case APP_COMMAND_KIND_MOVE_DOWN_CHUNK: {
                        Event.Kind = UI_EVENT_KIND_NAVIGATE;
                        Event.Flags = UI_EVENT_FLAG_EXPLICIT_DIRECTIONAL | UI_EVENT_FLAG_SECONDARY;
                        Event.DeltaStride = UI_EVENT_DELTA_STRIDE_WORD;
                        Event.DeltaI32 = Vec(0, 1);
                    } break;

                    case APP_COMMAND_KIND_MOVE_LEFT_CHUNK_SELECT: {
                        Event.Kind = UI_EVENT_KIND_NAVIGATE;
                        Event.Flags = UI_EVENT_FLAG_KEEP_MARK | UI_EVENT_FLAG_EXPLICIT_DIRECTIONAL;
                        Event.DeltaStride = UI_EVENT_DELTA_STRIDE_WORD;
                        Event.DeltaI32 = Vec(-1, 0);
                    } break;

                    case APP_COMMAND_KIND_MOVE_RIGHT_CHUNK_SELECT: {
                        Event.Kind = UI_EVENT_KIND_NAVIGATE;
                        Event.Flags = UI_EVENT_FLAG_KEEP_MARK | UI_EVENT_FLAG_EXPLICIT_DIRECTIONAL;
                        Event.DeltaStride = UI_EVENT_DELTA_STRIDE_WORD;
                        Event.DeltaI32 = Vec(1, 0);
                    } break;

                    case APP_COMMAND_KIND_MOVE_UP_CHUNK_SELECT: {
                        Event.Kind = UI_EVENT_KIND_NAVIGATE;
                        Event.Flags = UI_EVENT_FLAG_KEEP_MARK | UI_EVENT_FLAG_EXPLICIT_DIRECTIONAL;
                        Event.DeltaStride = UI_EVENT_DELTA_STRIDE_WORD;
                        Event.DeltaI32 = Vec(0, -1);
                    } break;

                    case APP_COMMAND_KIND_MOVE_DOWN_CHUNK_SELECT: {
                        Event.Kind = UI_EVENT_KIND_NAVIGATE;
                        Event.Flags = UI_EVENT_FLAG_KEEP_MARK | UI_EVENT_FLAG_EXPLICIT_DIRECTIONAL;
                        Event.DeltaStride = UI_EVENT_DELTA_STRIDE_WORD;
                        Event.DeltaI32 = Vec(0, 1);
                    } break;

                    case APP_COMMAND_KIND_MOVE_UP_PAGE: {
                        Event.Kind = UI_EVENT_KIND_NAVIGATE;
                        Event.DeltaStride = UI_EVENT_DELTA_STRIDE_PAGE;
                        Event.DeltaI32 = Vec(0, -1);
                    } break;

                    case APP_COMMAND_KIND_MOVE_DOWN_PAGE: {
                        Event.Kind = UI_EVENT_KIND_NAVIGATE;
                        Event.DeltaStride = UI_EVENT_DELTA_STRIDE_PAGE;
                        Event.DeltaI32 = Vec(0, 1);
                    } break;

                    case APP_COMMAND_KIND_MOVE_UP_WHOLE: {
                        Event.Kind = UI_EVENT_KIND_NAVIGATE;
                        Event.DeltaStride = UI_EVENT_DELTA_STRIDE_WHOLE;
                        Event.DeltaI32 = Vec(0, -1);
                    } break;

                    case APP_COMMAND_KIND_MOVE_DOWN_WHOLE: {
                        Event.Kind = UI_EVENT_KIND_NAVIGATE;
                        Event.DeltaStride = UI_EVENT_DELTA_STRIDE_WHOLE;
                        Event.DeltaI32 = Vec(0, 1);
                    } break;

                    case APP_COMMAND_KIND_MOVE_UP_PAGE_SELECT: {
                        Event.Kind = UI_EVENT_KIND_NAVIGATE;
                        Event.Flags = UI_EVENT_FLAG_KEEP_MARK;
                        Event.DeltaStride = UI_EVENT_DELTA_STRIDE_PAGE;
                        Event.DeltaI32 = Vec(0, -1);
                    } break;

                    case APP_COMMAND_KIND_MOVE_DOWN_PAGE_SELECT: {
                        Event.Kind = UI_EVENT_KIND_NAVIGATE;
                        Event.Flags = UI_EVENT_FLAG_KEEP_MARK;
                        Event.DeltaStride = UI_EVENT_DELTA_STRIDE_PAGE;
                        Event.DeltaI32 = Vec(0, 1);
                    } break;

                    case APP_COMMAND_KIND_MOVE_UP_WHOLE_SELECT: {
                        Event.Kind = UI_EVENT_KIND_NAVIGATE;
                        Event.Flags = UI_EVENT_FLAG_KEEP_MARK;
                        Event.DeltaStride = UI_EVENT_DELTA_STRIDE_WHOLE;
                        Event.DeltaI32 = Vec(0, -1);
                    } break;

                    case APP_COMMAND_KIND_MOVE_DOWN_WHOLE_SELECT: {
                        Event.Kind = UI_EVENT_KIND_NAVIGATE;
                        Event.Flags = UI_EVENT_FLAG_KEEP_MARK;
                        Event.DeltaStride = UI_EVENT_DELTA_STRIDE_WHOLE;
                        Event.DeltaI32 = Vec(0, 1);
                    } break;

                    case APP_COMMAND_KIND_MOVE_HOME: {
                        Event.Kind = UI_EVENT_KIND_NAVIGATE;
                        Event.DeltaStride = UI_EVENT_DELTA_STRIDE_LINE;
                        Event.DeltaI32 = Vec(-1, 0);
                    } break;

                    case APP_COMMAND_KIND_MOVE_END: {
                        Event.Kind = UI_EVENT_KIND_NAVIGATE;
                        Event.DeltaStride = UI_EVENT_DELTA_STRIDE_LINE;
                        Event.DeltaI32 = Vec(1, 0);
                    } break;

                    case APP_COMMAND_KIND_MOVE_HOME_SELECT: {
                        Event.Kind = UI_EVENT_KIND_NAVIGATE;
                        Event.Flags = UI_EVENT_FLAG_KEEP_MARK;
                        Event.DeltaStride = UI_EVENT_DELTA_STRIDE_LINE;
                        Event.DeltaI32 = Vec(-1, 0);
                    } break;

                    case APP_COMMAND_KIND_MOVE_END_SELECT: {
                        Event.Kind = UI_EVENT_KIND_NAVIGATE;
                        Event.Flags = UI_EVENT_FLAG_KEEP_MARK;
                        Event.DeltaStride = UI_EVENT_DELTA_STRIDE_LINE;
                        Event.DeltaI32 = Vec(1, 0);
                    } break;

                    case APP_COMMAND_KIND_SELECT_ALL: {
                        UIEvent MoveEvent = {};

                        MoveEvent.Kind = UI_EVENT_KIND_NAVIGATE;
                        MoveEvent.DeltaStride = UI_EVENT_DELTA_STRIDE_WHOLE;
                        MoveEvent.DeltaI32 = Vec(-1, 0);
                        ListPush(Scratch.MemPool, &APP_STATE->UIEvents, &MoveEvent);
                        Event.Kind = UI_EVENT_KIND_NAVIGATE;
                        Event.Flags = UI_EVENT_FLAG_KEEP_MARK;
                        Event.DeltaStride = UI_EVENT_DELTA_STRIDE_WHOLE;
                        Event.DeltaI32 = Vec(1, 0);
                    } break;

                    case APP_COMMAND_KIND_DELETE_SINGLE: {
                        Event.Kind = UI_EVENT_KIND_EDIT;
                        Event.Flags = UI_EVENT_FLAG_DELETE | UI_EVENT_FLAG_ZERO_DELTA_ON_SELECT;
                        Event.DeltaStride = UI_EVENT_DELTA_STRIDE_CHAR;
                        Event.DeltaI32 = Vec(1, 0);
                    } break;

                    case APP_COMMAND_KIND_DELETE_CHUNK: {
                        Event.Kind = UI_EVENT_KIND_EDIT;
                        Event.Flags = UI_EVENT_FLAG_DELETE | UI_EVENT_FLAG_ZERO_DELTA_ON_SELECT;
                        Event.DeltaStride = UI_EVENT_DELTA_STRIDE_WORD;
                        Event.DeltaI32 = Vec(1, 0);
                    } break;

                    case APP_COMMAND_KIND_BACKSPACE_SINGLE: {
                        Event.Kind = UI_EVENT_KIND_EDIT;
                        Event.Flags = UI_EVENT_FLAG_DELETE | UI_EVENT_FLAG_ZERO_DELTA_ON_SELECT;
                        Event.DeltaStride = UI_EVENT_DELTA_STRIDE_CHAR;
                        Event.DeltaI32 = Vec(-1, 0);
                    } break;

                    case APP_COMMAND_KIND_BACKSPACE_CHUNK: {
                        Event.Kind = UI_EVENT_KIND_EDIT;
                        Event.Flags = UI_EVENT_FLAG_DELETE | UI_EVENT_FLAG_ZERO_DELTA_ON_SELECT;
                        Event.DeltaStride = UI_EVENT_DELTA_STRIDE_WORD;
                        Event.DeltaI32 = Vec(-1, 0);
                    } break;

                    case APP_COMMAND_KIND_COPY: {
                        Event.Kind = UI_EVENT_KIND_EDIT;
                        Event.Flags = UI_EVENT_FLAG_COPY | UI_EVENT_FLAG_KEEP_MARK;
                    } break;

                    case APP_COMMAND_KIND_CUT: {
                        Event.Kind = UI_EVENT_KIND_EDIT;
                        Event.Flags = UI_EVENT_FLAG_COPY | UI_EVENT_FLAG_DELETE;
                    } break;

                    case APP_COMMAND_KIND_PASTE: {
                        Event.Kind = UI_EVENT_KIND_TEXT;
                        Event.String = GetClipboardText(Scratch.MemPool);
                    } break;

                    case APP_COMMAND_KIND_INSERT_TEXT: {
                        Event.Kind = UI_EVENT_KIND_TEXT;
                        Event.String = Registers()->String;
                    } break;

                    case APP_COMMAND_KIND_MOVE_NEXT: {
                        Event.Kind = UI_EVENT_KIND_NAVIGATE;
                        Event.Flags = UI_EVENT_FLAG_SECONDARY;
                        Event.DeltaStride = UI_EVENT_DELTA_STRIDE_CHAR;
                        Event.DeltaI32 = Vec(0, 1);
                    } break;

                    case APP_COMMAND_KIND_MOVE_PREV: {
                        Event.Kind = UI_EVENT_KIND_NAVIGATE;
                        Event.Flags = UI_EVENT_FLAG_SECONDARY;
                        Event.DeltaStride = UI_EVENT_DELTA_STRIDE_CHAR;
                        Event.DeltaI32 = Vec(0, -1);
                    } break;

                    case APP_COMMAND_KIND_GO_BACK:
                    case APP_COMMAND_KIND_GO_FORWARD:
                    case APP_COMMAND_KIND_GOTO_OFFSET:
                    case APP_COMMAND_KIND_GOTO_TIME:
                    case APP_COMMAND_KIND_SEARCH:
                    case APP_COMMAND_KIND_SEARCH_BACKWARDS:
                    case APP_COMMAND_KIND_FIND_NEXT:
                    case APP_COMMAND_KIND_FIND_PREV:
                    case APP_COMMAND_KIND_FILTER:
                    case APP_COMMAND_KIND_EXTRACT: {} break;

                    case APP_COMMAND_KIND_SET_THEME: {
                        ConfigNode* Session = ConfigNodeChildFromStr(
                            ConfigNodeRoot(), 
                            "session"_s8
                        );
                        ConfigNode* Theme = ConfigNodeChildFromStrOrAlloc(
                            APP_STATE->Config,
                            Session,
                            "theme"_s8
                        );

                        ConfigNodeNewReplace(
                            APP_STATE->Config,
                            Theme,
                            Registers()->String
                        );
                    } break;

                    case APP_COMMAND_KIND_TOGGLE_DEV_MENU: {
                        APP_STATE->DevMenuIsOpen ^= TRUE;
                    } break;

                    case APP_COMMAND_KIND_PUSH_QUERY: {
                        Str8 CommandName = Registers()->CommandName;
                        AppCommandKindInfo* Info = AppCommandKindInfoFromStr(
                            CommandName
                        );
                        b32 IsFloating = (
                            !CommandName.Size || 
                            Info->Query.Kind & APP_QUERY_KIND_FLOATING
                        );

                        UIContextMenuClose();
                        APP_STATE->MenuBarFocused = FALSE;

                        if (IsFloating) {
                            Str8 InitialInput = ""_s8;

                            if (Info->Query.Slot == APP_REGISTERS_SLOT_FILE_PATH) {
                                ConfigNode* Session = ConfigNodeChildFromStr(
                                    ConfigNodeRoot(), 
                                    "session"_s8
                                );
                                ConfigNode* CurrentPath = ConfigNodeChildFromStr(
                                    Session, 
                                    "current_path"_s8
                                );
                                Str8 CurrentPathString = CurrentPath->Head->String;

                                if (!CurrentPathString.Size) {
                                    CurrentPathString = PathNormalisedFromStr(
                                        Scratch.MemPool, 
                                        GetCurrentPath(Scratch.MemPool)
                                    );
                                }

                                InitialInput = ArenaPushStrFmt(
                                    Scratch.MemPool, 
                                    "%S/", 
                                    PRINT_STR(CurrentPathString)
                                );
                            }

                            APP_STATE->QueryIsActive = TRUE;
                            ArenaClear(APP_STATE->QueryMemPool);
                            APP_STATE->QueryRegs = CopyRegisters(
                                APP_STATE->QueryMemPool, 
                                Registers()
                            );
                            APP_STATE->QueryStringSize = MIN(
                                sizeof(APP_STATE->QueryBuffer), 
                                InitialInput.Size
                            );
                            MemCpy(
                                APP_STATE->QueryBuffer, 
                                InitialInput.Str, 
                                APP_STATE->QueryStringSize
                            );
                            APP_STATE->QueryCursor = TxtPt(
                                1, 
                                1 + APP_STATE->QueryStringSize
                            );
                            APP_STATE->QueryMark = APP_STATE->QueryCursor;
                            APP_STATE->QueryScroll = {};
                            APP_STATE->QueryListCursor = {};
                            APP_STATE->QueryListCursor.Y = (Info->Query.Slot != APP_REGISTERS_SLOT_FILE_PATH);
                            APP_STATE->QueryListMark = APP_STATE->QueryListCursor;
                        } else if (
                            ConfigNodeFromID(
                                Registers()->View
                            ) != EMPTY_CFG_NODE_VALUE
                        ) {
                            ConfigNode* View = ConfigNodeFromID(Registers()->View);
                            ConfigNode* Query = ConfigNodeChildFromStrOrAlloc(
                                APP_STATE->Config,
                                View,
                                "query"_s8
                            );
                            ConfigNode* Cmd = ConfigNodeChildFromStrOrAlloc(
                                APP_STATE->Config,
                                Query,
                                "cmd"_s8
                            );
                            ConfigNode* Input = ConfigNodeChildFromStrOrAlloc(
                                APP_STATE->Config,
                                Query,
                                "input"_s8
                            );
                            Str8 InitialInput = ""_s8;

                            if (Info->Query.Kind & APP_QUERY_KIND_KEEP_OLD_INPUT)
                                InitialInput = Input->Head->String;

                            Str8 CurrentQueryCommandName = ArenaPushStrCpy(
                                Scratch.MemPool, 
                                Cmd->Head->String
                            );

                            ConfigNodeNewReplace(
                                APP_STATE->Config,
                                Input,
                                InitialInput
                            );
                            ConfigNodeNewReplace(
                                APP_STATE->Config,
                                Cmd,
                                CommandName
                            );

                            AppViewState* VS = AppViewStateFromConfig(View);

                            if (
                                !VS->QueryIsOpen && 
                                Info->Query.Kind & APP_QUERY_KIND_SELECT_OLD_INPUT
                            ) {
                                VS->QueryCursor = TxtPt(1, 1 + Input->Head->String.Size);
                                VS->QueryMark = TxtPt(1, 1);
                            } else {
                                VS->QueryCursor = TxtPt(1, 1 + Input->Head->String.Size);
                                VS->QueryMark = VS->QueryCursor;
                            }

                            if (
                                !StrMatch(CurrentQueryCommandName, CommandName, 0) ||
                                VS->ContentsAreFocused
                            ) {
                                VS->QueryIsOpen = TRUE;
                            } else {
                                VS->QueryIsOpen ^= TRUE;
                            }

                            VS->ContentsAreFocused = FALSE;
                        }
                    } break;

                    case APP_COMMAND_KIND_COMPLETE_QUERY: {
                        ConfigNode* View = ConfigNodeFromID(Registers()->View);
                        Str8 CommandName = AppViewQueryCommand();

                        if (CommandName.Size)
                            AppPushCmd(CommandName, Registers());

                        AppCommandKindInfo* Info = AppCommandKindInfoFromStr(
                            CommandName
                        );

                        if (!(Info->Query.Kind & APP_QUERY_KIND_KEEP_OLD_INPUT)) {
                            AppViewState* VS = AppViewStateFromConfig(View);

                            VS->QueryIsOpen = FALSE;
                            VS->QueryStringSize = 0;
                        }
                    } break;

                    case APP_COMMAND_KIND_CANCEL_QUERY: {
                        APP_STATE->QueryIsActive = FALSE;
                        ArenaClear(APP_STATE->QueryMemPool);
                        APP_STATE->QueryRegs = NULL;
                    } break;

                    case APP_COMMAND_KIND_UPDATE_QUERY: {
                        ConfigNode* View = ConfigNodeFromID(Registers()->View);
                        ConfigNode* Query = ConfigNodeChildFromStrOrAlloc(
                            APP_STATE->Config,
                            View,
                            "query"_s8
                        );
                        ConfigNode* Input = ConfigNodeChildFromStrOrAlloc(
                            APP_STATE->Config,
                            Query,
                            "input"_s8
                        );

                        ConfigNodeNewReplace(
                            APP_STATE->Config,
                            Input,
                            Registers()->String
                        );

                        AppViewState* VS = AppViewStateFromConfig(View);

                        VS->QueryStringSize = MIN(
                            sizeof(VS->QueryBuffer), 
                            Registers()->String.Size
                        );
                        VS->QueryCursor = TxtPt(1, 1 + VS->QueryStringSize);
                        VS->QueryMark = VS->QueryCursor;
                        MemCpy(
                            VS->QueryBuffer, 
                            Registers()->String.Str, 
                            VS->QueryStringSize
                        );
                    } break;
                }

                if (Event.Kind != UI_EVENT_KIND_NULL)
                    ListPush(Scratch.MemPool, &APP_STATE->UIEvents, &Event);
            }
        }
    }

    if (APP_STATE->FrameDepth == 1) {
        Arena* HeadMemPool = APP_STATE->CommandMemPools[0];
        AppCommandList HeadCommands = APP_STATE->Commands[0];

        MemCpy(
            APP_STATE->CommandMemPools,
            APP_STATE->CommandMemPools + 1,
            sizeof(APP_STATE->CommandMemPools[0]) * (ARRAY_COUNT(APP_STATE->CommandMemPools) - 1)
        );
        MemCpy(
            APP_STATE->Commands,
            APP_STATE->Commands + 1,
            sizeof(APP_STATE->Commands[0]) * (ARRAY_COUNT(APP_STATE->Commands) - 1)
        );
        APP_STATE->CommandMemPools[ARRAY_COUNT(APP_STATE->CommandMemPools) - 1] = HeadMemPool;
        APP_STATE->Commands[ARRAY_COUNT(APP_STATE->Commands) - 1] = HeadCommands;
        ArenaClear(APP_STATE->CommandMemPools[0]);
        MemZero(&APP_STATE->Commands[0], sizeof(APP_STATE->Commands[0]));
        ++APP_STATE->CommandsGeneration;
    }

    f32 MasterAnimationsF = (f32) !!AppSettingFromNameB32("animations"_s8);
    f32 ScrollingAnimationsF = (f32) !!AppSettingFromNameB32("scrolling_animations"_s8);

    APP_STATE->CatchAllAnimationRate = 1.0f - MasterAnimationsF * PowF32(2.0f, (-60.0f * APP_STATE->FrameDeltaTime));
    APP_STATE->ScrollingAnimationRate = 1.0f - MasterAnimationsF * ScrollingAnimationsF * PowF32(2.0f, (-60.0f * APP_STATE->FrameDeltaTime));

    AppPushRegisters();
    AppWindowFrame();
    MemSet(&APP_STATE->UIEvents, 0, sizeof(APP_STATE->UIEvents));

    if (APP_STATE->DragDropState == APP_DRAG_DROP_STATE_DROPPING)
        APP_STATE->DragDropState = APP_DRAG_DROP_STATE_NULL;

    AppRegisters* WindowRegs = AppPopRegisters();

    MemCpy(Registers(), WindowRegs, sizeof(*WindowRegs));

    if (APP_STATE->NumFramesRequested)
        --APP_STATE->NumFramesRequested;

    AccessClose(APP_STATE->FrameAccess);
    APP_STATE->FrameAccess = FrameAccessRestore;

    u64 EndTimeUSecs = TimeNow();
    u64 FrameTimeUSecs = EndTimeUSecs - StartTimeUSecs;

    APP_STATE->FrameTimeUSecsHistory[
        APP_STATE->FrameIndex % ARRAY_COUNT(APP_STATE->FrameTimeUSecsHistory)
    ] = FrameTimeUSecs;

    if (
        APP_STATE->NumFramesRequested &&
        FrameTimeUSecs < FrameTimeTargetCapacityUSecs
    ) {
        SleepMSecs((u32) ((FrameTimeTargetCapacityUSecs - FrameTimeUSecs) / THOUSAND(1)));
    }

    ++APP_STATE->FrameIndex;
    APP_STATE->TimeSecs += APP_STATE->FrameDeltaTime;
    APP_STATE->TimeUSecs += FrameTimeUSecs;

    if (APP_STATE->FrameDepth == 1)
        ++APP_STATE->CommandsGeneration;

    --APP_STATE->FrameDepth;
    ReleaseScratch(Scratch);
}

void 
AppWindowFrame(void)
{
    TempArena Scratch = GetScratch(NULL, 0);
    ConfigNode* Window = ConfigNodeFromID(Registers()->Window);
    ConfigPanelTree PanelTree = ConfigPanelTreeFromConfig(
        Scratch.MemPool, 
        Window
    );
    b32 PopUpIsOpen = APP_STATE->PopUpActive;
    b32 QueryIsOpen = APP_STATE->QueryIsActive;

    if (PopUpIsOpen)
        APP_STATE->MenuBarKeyHeld = FALSE;

    UISelectState(APP_STATE->UI);
    Registers()->Panel = PanelTree.Focused->Config->ID;
    Registers()->Tab = PanelTree.Focused->SelectedTab->ID;
    Registers()->View = PanelTree.Focused->SelectedTab->ID;

    {
        Access* Acc = AccessOpen();
        ConfigNode* Session = ConfigNodeChildFromStr(
            ConfigNodeRoot(), 
            "session"_s8
        );
        ConfigNode* ThemeConfig = ConfigNodeChildFromStr(
            Session, 
            "theme"_s8
        );
        Str8 ThemeName = (
            ThemeConfig != EMPTY_CFG_NODE_VALUE && 
            ThemeConfig->Head != EMPTY_CFG_NODE_VALUE
        ) ? ThemeConfig->Head->String
          : APP_THEME_PRESET_DISPLAY_STR_TABLE[APP_THEME_PRESET_AYU_DARK];
        MDNode* ThemeTree = AppThemeTreeFromName(
            Scratch.MemPool, 
            Acc, 
            ThemeName
        );

        struct ThemePatternNode {
            ThemePatternNode* Next;
            UIThemePattern    Pattern;
        };

        ThemePatternNode* HeadPattern = NULL;
        ThemePatternNode* TailPattern = NULL;
        u64 PatternCount = 0;

        for (
            MDNode* N = ThemeTree;
            !MDNodeIsEmpty(N);
            N = MDNodeRecordDepthFirstPreOrder(N, ThemeTree).Next
        ) {
            if (StrMatch(N->String, "theme_colour"_s8, 0)) {
                MDNode* TagsChild = MDChildFromStr(N, "tags"_s8, 0);
                MDNode* ValueChild = MDChildFromStr(N, "value"_s8, 0);
                u8 SplitChar = ' ';
                Str8List Tags = StrSplit(
                    Scratch.MemPool,
                    TagsChild->Head->String,
                    &SplitChar,
                    1,
                    FALSE
                );
                u32 ColourU32 = U32FromStr(ValueChild->Head->String, 16);
                ThemePatternNode* Node = ArenaPushArrayZero(
                    Scratch.MemPool,
                    ThemePatternNode,
                    1
                );

                Node->Pattern.Tags = StrArrayFromList(
                    AppFrameMemPool(), 
                    &Tags
                );
                Node->Pattern.Linear = RGBAFromU32(ColourU32);
                SLL_QUEUE_PUSH(HeadPattern, TailPattern, Node);
                ++PatternCount;
            }
        }

        APP_STATE->Theme = ArenaPushArrayZero(AppFrameMemPool(), UITheme, 1);
        APP_STATE->Theme->PatternsCount = PatternCount;
        APP_STATE->Theme->Patterns = ArenaPushArrayZero(
            AppFrameMemPool(),
            UIThemePattern,
            APP_STATE->Theme->PatternsCount
        );

        u64 Index = 0;

        for (ThemePatternNode* N = HeadPattern; N; N = N->Next, ++Index)
            APP_STATE->Theme->Patterns[Index] = N->Pattern;

        AccessClose(Acc);
    }

    UIIconInfo IconInfo = {};

    IconInfo.IconKindTextMap[UI_ICON_KIND_RIGHT_ARROW] = APP_ICON_KIND_TEXT_TABLE[APP_ICON_KIND_RIGHT_CARET];
    IconInfo.IconKindTextMap[UI_ICON_KIND_DOWN_ARROW] = APP_ICON_KIND_TEXT_TABLE[APP_ICON_KIND_DOWN_CARET];
    IconInfo.IconKindTextMap[UI_ICON_KIND_LEFT_ARROW] = APP_ICON_KIND_TEXT_TABLE[APP_ICON_KIND_LEFT_CARET];
    IconInfo.IconKindTextMap[UI_ICON_KIND_UP_ARROW] = APP_ICON_KIND_TEXT_TABLE[APP_ICON_KIND_UP_CARET];
    IconInfo.IconKindTextMap[UI_ICON_KIND_RIGHT_CARET] = APP_ICON_KIND_TEXT_TABLE[APP_ICON_KIND_RIGHT_CARET];
    IconInfo.IconKindTextMap[UI_ICON_KIND_DOWN_CARET] = APP_ICON_KIND_TEXT_TABLE[APP_ICON_KIND_DOWN_CARET];
    IconInfo.IconKindTextMap[UI_ICON_KIND_LEFT_CARET] = APP_ICON_KIND_TEXT_TABLE[APP_ICON_KIND_LEFT_CARET];
    IconInfo.IconKindTextMap[UI_ICON_KIND_UP_CARET] = APP_ICON_KIND_TEXT_TABLE[APP_ICON_KIND_UP_CARET];
    IconInfo.IconKindTextMap[UI_ICON_KIND_CHECK_HOLLOW] = APP_ICON_KIND_TEXT_TABLE[APP_ICON_KIND_CHECK_HOLLOW];
    IconInfo.IconKindTextMap[UI_ICON_KIND_CHECK_FILLED] = APP_ICON_KIND_TEXT_TABLE[APP_ICON_KIND_CHECK_FILLED];
    IconInfo.IconKindTextMap[UI_ICON_KIND_RADIO_HOLLOW] = APP_ICON_KIND_TEXT_TABLE[APP_ICON_KIND_RADIO_HOLLOW];
    IconInfo.IconKindTextMap[UI_ICON_KIND_RADIO_FILLED] = APP_ICON_KIND_TEXT_TABLE[APP_ICON_KIND_RADIO_FILLED];

    UIAnimationInfo AnimationInfo = {};

    AnimationInfo.HotAnimationRate = APP_STATE->CatchAllAnimationRate;
    AnimationInfo.ActiveAnimationRate = APP_STATE->CatchAllAnimationRate;
    AnimationInfo.FocusAnimationRate = 1.0f;
    AnimationInfo.ScrollAnimationRate = APP_STATE->ScrollingAnimationRate;
    UIBeginBuild(
        &APP_STATE->UIEvents,
        &IconInfo,
        APP_STATE->Theme,
        &AnimationInfo,
        APP_STATE->FrameDeltaTime,
        APP_STATE->FrameDeltaTime
    );
    UIPushTextPadding(1.0f);
    UIPushPreferredWidth(UI_PX(20.0f, 1.0f));
    UIPushPreferredHeight(UI_PX(1.0f, 1.0f));

    r2f32 WindowRect = GetConsoleRect();
    r2f32 TopBarRect = Rng(
        WindowRect.X0,
        WindowRect.Y0,
        WindowRect.X1,
        WindowRect.Y0 + 1.0f
    );
    r2f32 BottomBarRect = Rng(
        WindowRect.X0,
        WindowRect.Y1 - 1.0f,
        WindowRect.X1,
        WindowRect.Y1
    );
    r2f32 ContentRect = Rng(
        WindowRect.X0,
        TopBarRect.Y1,
        WindowRect.X1,
        BottomBarRect.Y0
    );

    if (
        AppDragIsActive() &&
        APP_STATE->DragDropRegistersSlot == APP_REGISTERS_SLOT_VIEW
    ) {
        ConfigNode* DragTab = ConfigNodeFromID(
            APP_STATE->DragDropRegisters->View
        );

        if (
            DragTab == EMPTY_CFG_NODE_VALUE ||
            UISlotPress(UI_EVENT_ACTION_SLOT_CANCEL)
        ) {
            AppDragKill();
            UIKillAction();
        } else {
            UITooltip() {
                FancyStrList Title = AppTitleFStrFromConfig(
                    Scratch.MemPool,
                    DragTab
                );
                UIBox* Box = UIBuildBoxFromKey(UI_BOX_KIND_DRAW_TEXT, {});

                UIBoxEquipDisplayFancyStrs(Box, &Title);
            }
        }
    }

    if (UIStringHoverActive()) {
        UITooltip() {
            TempArena Scratch = GetScratch(NULL, 0);
            FancyStrList FancyStrings = UIStringHoverFancyStrings(Scratch.MemPool);
            UIBox* Box = UIBuildBoxFromKey(UI_BOX_KIND_DRAW_TEXT, {});

            UIBoxEquipDisplayFancyStrs(Box, &FancyStrings);
            ReleaseScratch(Scratch);
        }
    }

    if (APP_STATE->DevMenuIsOpen) {
        u64 FrameTimeSumUSecs = 0;
        u64 FrameTimeCount = MIN(
            ARRAY_COUNT(APP_STATE->FrameTimeUSecsHistory), 
            APP_STATE->FrameIndex
        );

        for (u64 Index = 0; Index < FrameTimeCount; ++Index)
            FrameTimeSumUSecs += APP_STATE->FrameTimeUSecsHistory[Index];

        UITag("floating"_s8) {
            UIPane(
                Rng(
                    WindowRect.X1 - 46.0f,
                    WindowRect.Y0 + 2.0f,
                    WindowRect.X1 - 2.0f,
                    WindowRect.Y0 + 13.0f
                ),
                "###dev_ctx_menu"_s8
            ) {
                UIWidthFill() {
                    UILabel("Frame: %llu", APP_STATE->FrameIndex);
                    UILabel("Frames requested: %llu", APP_STATE->NumFramesRequested);
                    UILabel("Frame time: %llu us", FrameTimeCount ? FrameTimeSumUSecs / FrameTimeCount : 0);
                    UILabel("Frame bytes: %llu", REND_STATE->PrevFrameBytes);
                    UILabel("UI is animating: %d", (i32) UIIsAnimatingFromState(APP_STATE->UI));
                    UILabel("UI box count: %llu", APP_STATE->UI->LastBuildBoxCount);
                    UILabel("UI memory: %llu", ArenaGetPosition(APP_STATE->UI->MemPool));
                    UILabel(
                        "UI build memory: %llu, %llu",
                        ArenaGetPosition(APP_STATE->UI->BuildMemPools[0]),
                        ArenaGetPosition(APP_STATE->UI->BuildMemPools[1])
                    );
                    UILabel(
                        "Frame memory: %llu, %llu",
                        ArenaGetPosition(APP_STATE->FrameMemPools[0]),
                        ArenaGetPosition(APP_STATE->FrameMemPools[1])
                    );
                }
            }
        }
    }

    if (APP_STATE->PopUpActive) {
        UIFocus(UI_FOCUS_KIND_ROOT) {
            v2f32 WindowLength = Length(WindowRect);
            f32 PopUpWidth = MIN(60.0f, WindowLength.X);
            r2f32 PopUpRect = Rng(
                WindowRect.X0 + FloorF32((WindowLength.X - PopUpWidth) / 2.0f),
                WindowRect.Y0 + FloorF32(WindowLength.Y / 2.0f) - 4.0f,
                WindowRect.X0 + FloorF32((WindowLength.X - PopUpWidth) / 2.0f) + PopUpWidth,
                WindowRect.Y0 + FloorF32(WindowLength.Y / 2.0f) + 3.0f
            );
            UIBox* BackgroundBox = EMPTY_UI_BOX_VALUE;

            UIContextMenuClose();

            UITag("floating"_s8) {
                UISetNextFlags(
                    UI_BOX_KIND_DEFAULT_FOCUS_NAV |
                    UI_BOX_KIND_DISABLE_FOCUS_OVERLAY |
                    UI_BOX_KIND_DRAW_DROP_SHADOW
                );

                UIPane(PopUpRect, "###popup"_s8) {
                    UIBox* PopUpBox = UIHeadParent();

                    UIWidthFill() {
                        UITextAlignment(UI_TEXT_ALIGN_CENTRE) {
                            UILabel(APP_STATE->PopUpTitle);

                            UITag("weak"_s8) {
                                UILabel(APP_STATE->PopUpDescription);
                            }

                            UISpacer(UI_PX(1.0f, 1.0f));

                            UIRow() {
                                UIPadding(UI_PERCENT(1.0f, 0.0f)) {
                                    UIPreferredWidth(UI_PX(12.0f, 1.0f)) {
                                        UITag("pop"_s8) {
                                            if (
                                                UI_CLICKED(UIButton("OK"_s8)) ||
                                                (
                                                    (PopUpBox->DefaultNavFocusHotKey == EMPTY_UI_KEY_VALUE) &&
                                                    UISlotPress(UI_EVENT_ACTION_SLOT_ACCEPT)
                                                )
                                            ) {
                                                AppCmd(APP_COMMAND_KIND_POP_UP_ACCEPT);
                                            }

                                            UISpacer(UI_PX(2.0f, 1.0f));

                                            if (UI_CLICKED(UIButton("Cancel"_s8)) || UISlotPress(UI_EVENT_ACTION_SLOT_CANCEL)) {
                                                AppCmd(APP_COMMAND_KIND_POP_UP_CANCEL);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            UIRect(WindowRect) {
                BackgroundBox = UIBuildBoxFromStr(
                    (
                        UI_BOX_KIND_FLOATING |
                        UI_BOX_KIND_CLICKABLE |
                        UI_BOX_KIND_SCROLL |
                        UI_BOX_KIND_DRAW_OVERLAY
                    ),
                    "###popup_background"_s8
                );
            }

            UISignalFromBox(BackgroundBox);
        }
    }

    if (QueryIsOpen && !PopUpIsOpen)
        AppQueryViewUI(ContentRect);

    UIKey FileMenuKey = UIKeyFromStr({}, "_file_menu_key_"_s8);
    UIKey ViewMenuKey = UIKeyFromStr({}, "_view_menu_key_"_s8);
    UIKey GoMenuKey = UIKeyFromStr({}, "_go_menu_key_"_s8);
    UIKey TabMenuKey = UIKeyFromStr({}, "_tab_menu_key_"_s8);
    UIKey PanelMenuKey = UIKeyFromStr({}, "_panel_menu_key_"_s8);
    UIKey HelpMenuKey = UIKeyFromStr({}, "_help_menu_key_"_s8);
    UIKey TabCtxMenuKey = UIKeyFromStr({}, "_tab_ctx_menu_key_"_s8);

    UIContextMenu(FileMenuKey) {
        UIPreferredWidth(UI_PX(48.0f, 1.0f)) {
            UITag("floating"_s8) {
                Str8 Commands[] = {
                    APP_COMMAND_KIND_INFO_TABLE[APP_COMMAND_KIND_OPEN].String,
                    APP_COMMAND_KIND_INFO_TABLE[APP_COMMAND_KIND_OPEN_PALETTE].String,
                    {},
                    APP_COMMAND_KIND_INFO_TABLE[APP_COMMAND_KIND_EXTRACT].String,
                    {},
                    APP_COMMAND_KIND_INFO_TABLE[APP_COMMAND_KIND_EXIT].String
                };
                u32 CodePoints[] = {
                    'o',
                    'p',
                    0,
                    'e',
                    0,
                    'x'
                };

                STATIC_ASSERT(
                    ARRAY_COUNT(CodePoints) == ARRAY_COUNT(Commands), 
                    FileMenuCommandsSizeCheck
                );
                AppCommandListMenuButtons(
                    Commands, 
                    ARRAY_COUNT(Commands), 
                    CodePoints
                );
            }
        }
    }

    UIContextMenu(ViewMenuKey) {
        UIPreferredWidth(UI_PX(48.0f, 1.0f)) {
            UITag("floating"_s8) {
                Str8 Commands[] = {
                    APP_COMMAND_KIND_INFO_TABLE[APP_COMMAND_KIND_OPEN_FILES].String,
                    APP_COMMAND_KIND_INFO_TABLE[APP_COMMAND_KIND_OPEN_REPORT].String,
                    APP_COMMAND_KIND_INFO_TABLE[APP_COMMAND_KIND_OPEN_INCORRECT_PACKETS].String,
                    APP_COMMAND_KIND_INFO_TABLE[APP_COMMAND_KIND_OPEN_LOST_PACKETS].String,
                    APP_COMMAND_KIND_INFO_TABLE[APP_COMMAND_KIND_OPEN_ALL_PACKETS].String,
                    APP_COMMAND_KIND_INFO_TABLE[APP_COMMAND_KIND_OPEN_MESSAGES].String,
                    APP_COMMAND_KIND_INFO_TABLE[APP_COMMAND_KIND_OPEN_SOURCES].String,
                    {},
                    APP_COMMAND_KIND_INFO_TABLE[APP_COMMAND_KIND_SET_THEME].String
                };
                u32 CodePoints[] = {
                    'f',
                    'r',
                    'i',
                    'l',
                    'a',
                    'm',
                    's',
                    0,
                    't'
                };

                STATIC_ASSERT(
                    ARRAY_COUNT(CodePoints) == ARRAY_COUNT(Commands), 
                    ViewMenuCommandsSizeCheck
                );
                AppCommandListMenuButtons(
                    Commands, 
                    ARRAY_COUNT(Commands), 
                    CodePoints
                );
            }
        }
    }

    UIContextMenu(GoMenuKey) {
        UIPreferredWidth(UI_PX(48.0f, 1.0f)) {
            UITag("floating"_s8) {
                Str8 Commands[] = {
                    APP_COMMAND_KIND_INFO_TABLE[APP_COMMAND_KIND_GOTO_OFFSET].String,
                    APP_COMMAND_KIND_INFO_TABLE[APP_COMMAND_KIND_GOTO_TIME].String,
                    {},
                    APP_COMMAND_KIND_INFO_TABLE[APP_COMMAND_KIND_SEARCH].String,
                    APP_COMMAND_KIND_INFO_TABLE[APP_COMMAND_KIND_SEARCH_BACKWARDS].String,
                    APP_COMMAND_KIND_INFO_TABLE[APP_COMMAND_KIND_FIND_NEXT].String,
                    APP_COMMAND_KIND_INFO_TABLE[APP_COMMAND_KIND_FIND_PREV].String,
                    APP_COMMAND_KIND_INFO_TABLE[APP_COMMAND_KIND_FILTER].String,
                    {},
                    APP_COMMAND_KIND_INFO_TABLE[APP_COMMAND_KIND_GO_BACK].String,
                    APP_COMMAND_KIND_INFO_TABLE[APP_COMMAND_KIND_GO_FORWARD].String
                };
                u32 CodePoints[] = {
                    'o',
                    't',
                    0,
                    's',
                    'w',
                    'n',
                    'p',
                    'f',
                    0,
                    'b',
                    'r'
                };

                STATIC_ASSERT(
                    ARRAY_COUNT(CodePoints) == ARRAY_COUNT(Commands), 
                    GoMenuCommandsSizeCheck
                );
                AppCommandListMenuButtons(
                    Commands, 
                    ARRAY_COUNT(Commands), 
                    CodePoints
                );
            }
        }
    }

    UIContextMenu(TabMenuKey) {
        UIPreferredWidth(UI_PX(48.0f, 1.0f)) {
            UITag("floating"_s8) {
                Str8 Commands[] = {
                    APP_COMMAND_KIND_INFO_TABLE[APP_COMMAND_KIND_OPEN_TAB].String,
                    APP_COMMAND_KIND_INFO_TABLE[APP_COMMAND_KIND_DUPLICATE_TAB].String,
                    APP_COMMAND_KIND_INFO_TABLE[APP_COMMAND_KIND_SET_TAB_FILE].String,
                    APP_COMMAND_KIND_INFO_TABLE[APP_COMMAND_KIND_CLOSE_TAB].String,
                    {},
                    APP_COMMAND_KIND_INFO_TABLE[APP_COMMAND_KIND_MOVE_TAB_LEFT].String,
                    APP_COMMAND_KIND_INFO_TABLE[APP_COMMAND_KIND_MOVE_TAB_RIGHT].String,
                    {},
                    APP_COMMAND_KIND_INFO_TABLE[APP_COMMAND_KIND_NEXT_TAB].String,
                    APP_COMMAND_KIND_INFO_TABLE[APP_COMMAND_KIND_PREV_TAB].String
                };
                u32 CodePoints[] = {
                    'o',
                    'd',
                    'f',
                    'c',
                    0,
                    'l',
                    'r',
                    0,
                    'n',
                    'p'
                };

                STATIC_ASSERT(
                    ARRAY_COUNT(CodePoints) == ARRAY_COUNT(Commands), 
                    TabMenuCommandsSizeCheck
                );
                AppCommandListMenuButtons(
                    Commands, 
                    ARRAY_COUNT(Commands), 
                    CodePoints
                );
            }
        }
    }

    UIContextMenu(PanelMenuKey) {
        UIPreferredWidth(UI_PX(48.0f, 1.0f)) {
            UITag("floating"_s8) {
                Str8 Commands[] = {
                    APP_COMMAND_KIND_INFO_TABLE[APP_COMMAND_KIND_NEW_PANEL_RIGHT].String,
                    APP_COMMAND_KIND_INFO_TABLE[APP_COMMAND_KIND_NEW_PANEL_DOWN].String,
                    APP_COMMAND_KIND_INFO_TABLE[APP_COMMAND_KIND_NEW_PANEL_LEFT].String,
                    APP_COMMAND_KIND_INFO_TABLE[APP_COMMAND_KIND_NEW_PANEL_UP].String,
                    {},
                    APP_COMMAND_KIND_INFO_TABLE[APP_COMMAND_KIND_CLOSE_PANEL].String,
                    {},
                    APP_COMMAND_KIND_INFO_TABLE[APP_COMMAND_KIND_NEXT_PANEL].String,
                    APP_COMMAND_KIND_INFO_TABLE[APP_COMMAND_KIND_PREV_PANEL].String
                };
                u32 CodePoints[] = {
                    'r',
                    'd',
                    'l',
                    'u',
                    0,
                    'c',
                    0,
                    'n',
                    'p'
                };

                STATIC_ASSERT(
                    ARRAY_COUNT(CodePoints) == ARRAY_COUNT(Commands),
                    PanelMenuCommandsSizeCheck
                );
                AppCommandListMenuButtons(
                    Commands,
                    ARRAY_COUNT(Commands),
                    CodePoints
                );
            }
        }
    }

    UIContextMenu(HelpMenuKey) {
        UIPreferredWidth(UI_PX(48.0f, 1.0f)) {
            UITag("floating"_s8) {
                Str8 Commands[] = {
                    APP_COMMAND_KIND_INFO_TABLE[APP_COMMAND_KIND_OPEN_PALETTE].String,
                    APP_COMMAND_KIND_INFO_TABLE[APP_COMMAND_KIND_RESET_TO_DEFAULT_BINDINGS].String,
                    APP_COMMAND_KIND_INFO_TABLE[APP_COMMAND_KIND_TOGGLE_DEV_MENU].String
                };
                u32 CodePoints[] = {
                    'p',
                    'k',
                    'd'
                };

                STATIC_ASSERT(
                    ARRAY_COUNT(CodePoints) == ARRAY_COUNT(Commands), 
                    HelpMenuCommandsSizeCheck
                );
                AppCommandListMenuButtons(
                    Commands, 
                    ARRAY_COUNT(Commands), 
                    CodePoints
                );
            }
        }
    }

    UIContextMenu(TabCtxMenuKey) {
        ConfigNode* MenuTab = ConfigNodeFromID(APP_STATE->TabCtxMenuTab);

        if (MenuTab == EMPTY_CFG_NODE_VALUE) {
            UIContextMenuClose();
        } else {
            UIPreferredWidth(UI_PX(48.0f, 1.0f)) {
                UITag("floating"_s8) {
                    AppRegistersScope(
                        __Registers.Panel = MenuTab->Parent->ID,
                        __Registers.Tab = MenuTab->ID,
                        __Registers.View = MenuTab->ID
                    ) {
                        Str8 TabFilePath = AppViewFilePath();

                        if (TabFilePath.Size) {
                            UISetNextPreferredHeight(UI_SUM_OF_CHILDREN(1.0f));

                            UIColumn() {
                                for (
                                    u64 Index = 0;
                                    Index < ARRAY_COUNT(APP_TAB_FAST_PATH_VIEW_NAME_TABLE);
                                    ++Index
                                ) {
                                    AppCommandKindInfo* ViewInfo = &APP_COMMAND_KIND_INFO_TABLE[APP_COMMAND_KIND_FIRST_TAB_FAST_PATH_CMD + Index];
                                    Str8 ViewName = APP_TAB_FAST_PATH_VIEW_NAME_TABLE[Index];

                                    if (!(ViewInfo->Flags & APP_COMMAND_KIND_FLAG_LIST_IN_FILE))
                                        continue;

                                    UISignal Sig = AppIconButton(
                                        StrMatch(MenuTab->String, ViewName, 0)
                                            ? APP_ICON_KIND_RADIO_FILLED
                                            : APP_ICON_KIND_RADIO_HOLLOW,
                                        NULL,
                                        "%S###tab_view_%S",
                                        PRINT_STR(ViewInfo->DisplayName),
                                        PRINT_STR(ViewName)
                                    );

                                    if (UI_CLICKED(Sig)) {
                                        AppCmd(
                                            APP_COMMAND_KIND_SET_TAB_VIEW,
                                            __Registers.String = ViewName
                                        );
                                        UIContextMenuClose();
                                    }
                                }

                                UIDivider(UI_PX(1.0f, 1.0f));
                                UISetNextFocusHot(UI_FOCUS_KIND_ON);
                                UISetNextFocusActive(UI_FOCUS_KIND_ON);

                                UINamedRow("###tab_file_row"_s8) {
                                    UIKey FileEditKey = UIKeyFromStr(
                                        UIHeadParent()->Key,
                                        "###tab_file"_s8
                                    );

                                    if (UIIsKeyAutoFocusActive(FileEditKey))
                                        APP_STATE->TextEditMode = TRUE;

                                    UIPreferredWidth(UI_TEXT_DIM(2.0f, 1.0f)) {
                                        UITag("weak"_s8) {
                                            UILabel(APP_ICON_KIND_TEXT_TABLE[APP_ICON_KIND_FILE]);
                                        }
                                    }

                                    UIPreferredWidth(UI_PERCENT(1.0f, 0.0f)) {
                                        UITag("alt"_s8) {
                                            UISignal EditSig = UILineEdit(
                                                &APP_STATE->TabCtxMenuCursor,
                                                &APP_STATE->TabCtxMenuMark,
                                                APP_STATE->TabCtxMenuFileBuffer,
                                                sizeof(APP_STATE->TabCtxMenuFileBuffer),
                                                &APP_STATE->TabCtxMenuFileSize,
                                                TabFilePath,
                                                "###tab_file"_s8
                                            );

                                            if (UI_COMMITTED(EditSig)) {
                                                AppCmd(
                                                    APP_COMMAND_KIND_SET_TAB_FILE,
                                                    __Registers.FilePath = Str(
                                                        APP_STATE->TabCtxMenuFileBuffer,
                                                        APP_STATE->TabCtxMenuFileSize
                                                    )
                                                );
                                                UIContextMenuClose();
                                            }
                                        }
                                    }
                                }

                                UIDivider(UI_PX(1.0f, 1.0f));
                            }
                        }

                        Str8 Commands[] = {
                            APP_COMMAND_KIND_INFO_TABLE[APP_COMMAND_KIND_DUPLICATE_TAB].String,
                            APP_COMMAND_KIND_INFO_TABLE[APP_COMMAND_KIND_CLOSE_TAB].String,
                            {},
                            APP_COMMAND_KIND_INFO_TABLE[APP_COMMAND_KIND_MOVE_TAB_LEFT].String,
                            APP_COMMAND_KIND_INFO_TABLE[APP_COMMAND_KIND_MOVE_TAB_RIGHT].String
                        };
                        u32 CodePoints[] = {
                            0,
                            0,
                            0,
                            0,
                            0
                        };

                        STATIC_ASSERT(
                            ARRAY_COUNT(CodePoints) == ARRAY_COUNT(Commands),
                            TabCtxMenuCommandsSizeCheck
                        );
                        AppCommandListMenuButtons(
                            Commands,
                            ARRAY_COUNT(Commands),
                            CodePoints
                        );
                    }
                }
            }
        }
    }

    UIFocus(
        (APP_STATE->MenuBarFocused && !UIAnyContextMenuIsOpen())
        ? UI_FOCUS_KIND_ON
        : UI_FOCUS_KIND_NULL
    ) {
        UITag("menu_bar"_s8) {
            UIBox* TopBarBox = EMPTY_UI_BOX_VALUE;

            UIRect(TopBarRect) {
                UISetNextChildLayoutAxis(AXIS_2D_X);
                UISetNextFlags(
                    UI_BOX_KIND_DEFAULT_FOCUS_NAV | UI_BOX_KIND_DISABLE_FOCUS_OVERLAY
                );
                TopBarBox = UIBuildBoxFromStr(
                    UI_BOX_KIND_CLICKABLE | UI_BOX_KIND_DRAW_BACKGROUND,
                    "###top_bar"_s8
                );
            }

            UIParent(TopBarBox) {
                UIFocus(UI_FOCUS_KIND_NULL) {
                    UIKey MenuBarGroupKey = UIKeyFromStr(
                        EMPTY_UI_KEY_VALUE, 
                        "###top_bar_group"_s8
                    );

                    UIPreferredWidth(UI_TEXT_DIM(2.0f, 1.0f)) {
                        UIGroupKey(MenuBarGroupKey) {
                            UITextAlignment(UI_TEXT_ALIGN_CENTRE) {
                                struct {
                                    Str8      Name;
                                    u32       CodePoint;
                                    InputKind Input;
                                    UIKey     MenuKey;
                                } Items[] = {
                                    {
                                        Str8Lit("File"),
                                        'f',
                                        INPUT_KIND_F,
                                        FileMenuKey
                                    },
                                    {
                                        Str8Lit("View"),
                                        'v',
                                        INPUT_KIND_V,
                                        ViewMenuKey
                                    },
                                    {
                                        Str8Lit("Go"),
                                        'g',
                                        INPUT_KIND_G,
                                        GoMenuKey
                                    },
                                    {
                                        Str8Lit("Tab"),
                                        't',
                                        INPUT_KIND_T,
                                        TabMenuKey
                                    },
                                    {
                                        Str8Lit("Panel"),
                                        'p',
                                        INPUT_KIND_P,
                                        PanelMenuKey
                                    },
                                    {
                                        Str8Lit("Help"),
                                        'h',
                                        INPUT_KIND_H,
                                        HelpMenuKey
                                    }
                                };

                                b32 MenuOpen = FALSE;
                                u64 OpenMenuIndex = 0;

                                for (u64 I = 0; I < ARRAY_COUNT(Items); ++I) {
                                    if (UIContextMenuIsOpen(Items[I].MenuKey)) {
                                        MenuOpen = TRUE;
                                        OpenMenuIndex = I;
                                        break;
                                    }
                                }

                                u64 OpenMenuIndexPrime = OpenMenuIndex;

                                if (MenuOpen && APP_STATE->MenuBarFocused) {
                                    for (UIEvent* Evt = NULL; UINextEvent(&Evt); ) {
                                        b32 Taken = FALSE;

                                        if (Evt->DeltaI32.X > 0) {
                                            Taken = TRUE;
                                            ++OpenMenuIndexPrime;
                                            OpenMenuIndexPrime = OpenMenuIndexPrime % ARRAY_COUNT(Items);
                                        }

                                        if (Evt->DeltaI32.X < 0) {
                                            Taken = TRUE;
                                            OpenMenuIndexPrime = OpenMenuIndexPrime > 0 
                                                ? OpenMenuIndexPrime - 1 
                                                : (ARRAY_COUNT(Items) - 1);
                                        }

                                        if (Taken)
                                            UIEatEvent(Evt);
                                    }
                                }

                                for (u64 I = 0; I < ARRAY_COUNT(Items); ++I) {
                                    UISetNextFastpathCodepoint(Items[I].CodePoint);

                                    b32 AltFastpathKey = FALSE;

                                    if (
                                        APP_STATE->AltMenuBarEnabled && 
                                        UIKeyPress(INPUT_MOD_KIND_ALT, Items[I].Input)
                                    ) {
                                        AltFastpathKey = TRUE;
                                    }

                                    if (
                                        (
                                            APP_STATE->MenuBarKeyHeld || 
                                            APP_STATE->MenuBarFocused
                                        ) && 
                                        !UIAnyContextMenuIsOpen()
                                    ) {
                                        UISetNextFlags(UI_BOX_KIND_DRAW_TEXT_FASTPATH_CODEPOINT);
                                    }

                                    UISignal Sig = AppMenuBarButton(Items[I].Name);

                                    if (MenuOpen) {
                                        if (
                                            (UI_HOVERING(Sig) && !UIContextMenuIsOpen(Items[I].MenuKey)) ||
                                            (OpenMenuIndexPrime == I && OpenMenuIndexPrime != OpenMenuIndex)
                                        ) {
                                            UIContextMenuOpen(
                                                Items[I].MenuKey, 
                                                Sig.Box->Key, 
                                                Vec(0.0f, Sig.Box->Rect.Y1 - Sig.Box->Rect.Y0)
                                            );
                                        }
                                    } else if (UI_PRESSED(Sig) || AltFastpathKey) {
                                        if (UIContextMenuIsOpen(Items[I].MenuKey)) {
                                            UIContextMenuClose();
                                        } else {
                                            UIContextMenuOpen(
                                                Items[I].MenuKey, 
                                                Sig.Box->Key, 
                                                Vec(0.0f, Sig.Box->Rect.Y1 - Sig.Box->Rect.Y0)
                                            );
                                        }
                                    }
                                }
                            }
                        }

                        UISpacer(UI_PERCENT(1.0f, 0.0f));

                        UITag("weak"_s8) {
                            UILabel("FUME"_s8);
                        }
                    }
                }
            }

            UISignalFromBox(TopBarBox);
        }
    }

    UITag("menu_bar"_s8) {
        UIBox* BottomBarBox = EMPTY_UI_BOX_VALUE;

        UIRect(BottomBarRect) {
            UISetNextChildLayoutAxis(AXIS_2D_X);
            BottomBarBox = UIBuildBoxFromStr(
                UI_BOX_KIND_DRAW_BACKGROUND,
                "###bottom_bar"_s8
            );
        }

        UIParent(BottomBarBox) {
            UIPreferredWidth(UI_TEXT_DIM(2.0f, 1.0f)) {
                if (APP_STATE->ErrorStrSize) {
                    UITag("bad"_s8) {
                        UILabel(Str(APP_STATE->ErrorBuffer, APP_STATE->ErrorStrSize));
                    }
                } else if (Registers()->FilePath.Size) {
                    UILabel(Registers()->FilePath);
                } else {
                    UILabel("No file"_s8);
                }

                UISpacer(UI_PERCENT(1.0f, 0.0f));

                AppCommandKind BarCommands[] = {
                    APP_COMMAND_KIND_OPEN_PALETTE,
                    APP_COMMAND_KIND_OPEN,
                    APP_COMMAND_KIND_EXIT
                };

                for (u64 Index = 0; Index < ARRAY_COUNT(BarCommands); ++Index) {
                    AppCommandKindInfo* BarInfo = &APP_COMMAND_KIND_INFO_TABLE[BarCommands[Index]];
                    ConfigInputMapNodePtrList BarKeys = ConfigInputMapNodePtrListFromName(
                        Scratch.MemPool,
                        APP_STATE->KeyMap,
                        BarInfo->String
                    );
                    UISignal Sig = UIButton(
                        "%S###bar_cmd_%S",
                        PRINT_STR(BarInfo->DisplayName),
                        PRINT_STR(BarInfo->String)
                    );

                    if (UI_CLICKED(Sig)) {
                        AppCmd(
                            APP_COMMAND_KIND_RUN_COMMAND,
                            __Registers.CommandName = BarInfo->String
                        );
                    }

                    AppCommandBindingButtons(
                        BarInfo->String,
                        ""_s8,
                        1,
                        BarKeys.Head
                            ? 0
                            : APP_COMMAND_BINDING_BTN_FLAG_ADD_NEW
                    );
                    UISpacer(UI_PX(1.0f, 1.0f));
                }
            }
        }
    }

    if (ContentRect.X1 > ContentRect.X0 && ContentRect.Y1 > ContentRect.Y0) {
        for (
            ConfigPanelNode* Panel = PanelTree.Root;
            Panel != EMPTY_CFG_PANEL_NODE_VALUE;
            Panel = ConfigPanelNodeRecDepthFirstPre(PanelTree.Root, Panel).Next
        ) {
            if (
                Panel->Head == EMPTY_CFG_PANEL_NODE_VALUE ||
                Panel->SplitAxis != AXIS_2D_X
            ) {
                continue;
            }

            r2f32 ParentRect = ConfigTargetRectFromPanelNode(
                ContentRect,
                PanelTree.Root,
                Panel
            );
            f32 TotalSize = Length(ParentRect).X;

            for (
                ConfigPanelNode* MinChild = Panel->Head;
                MinChild->Next != EMPTY_CFG_PANEL_NODE_VALUE;
                MinChild = MinChild->Next
            ) {
                ConfigPanelNode* MaxChild = MinChild->Next;
                r2f32 MinChildRect = ConfigTargetRectFromPanelNodeChild(
                    ParentRect,
                    Panel,
                    MinChild
                );
                r2f32 BoundaryRect = Rng(
                    MinChildRect.X1 - 1.0f,
                    ParentRect.Y0,
                    MinChildRect.X1,
                    ParentRect.Y1
                );
                UIBox* BoundaryBox = EMPTY_UI_BOX_VALUE;

                UIRect(BoundaryRect) {
                    UITag("menu_bar"_s8) {
                        UISetNextHoverCursor(CURSOR_KIND_LEFT_RIGHT);
                        BoundaryBox = UIBuildBoxFromStrFmt(
                            (
                                UI_BOX_KIND_MOUSE_CLICKABLE |
                                UI_BOX_KIND_DRAW_BACKGROUND |
                                UI_BOX_KIND_DRAW_HOT_EFFECTS |
                                UI_BOX_KIND_DRAW_ACTIVE_EFFECTS
                            ),
                            "###panel_boundary_%p_%p",
                            MinChild->Config,
                            MaxChild->Config
                        );
                    }
                }

                UISignal Sig = UISignalFromBox(BoundaryBox);

                if (UI_DOUBLE_CLICKED(Sig)) {
                    f32 HalfPercent = 0.5f * (MinChild->PercentOfParent + MaxChild->PercentOfParent);

                    UIKillAction();
                    MinChild->PercentOfParent = HalfPercent;
                    MaxChild->PercentOfParent = HalfPercent;
                    ConfigNodeEquipStr(
                        APP_STATE->Config,
                        MinChild->Config,
                        "%f",
                        HalfPercent
                    );
                    ConfigNodeEquipStr(
                        APP_STATE->Config,
                        MaxChild->Config,
                        "%f",
                        HalfPercent
                    );
                } else if (UI_PRESSED(Sig)) {
                    v2f32 Percents = Vec(
                        MinChild->PercentOfParent,
                        MaxChild->PercentOfParent
                    );

                    UIStoreDragStruct(&Percents);
                } else if (UI_DRAGGING(Sig)) {
                    v2f32 Percents = *UIGetDragStruct(v2f32);
                    f32 BothSize = (Percents.X + Percents.Y) * TotalSize;

                    if (BothSize >= 20.0f) {
                        f32 MinSize = CLAMP(
                            10.0f,
                            Percents.X * TotalSize + UIDragDelta().X,
                            BothSize - 10.0f
                        );

                        MinChild->PercentOfParent = MinSize / TotalSize;
                        MaxChild->PercentOfParent = (BothSize - MinSize) / TotalSize;
                        ConfigNodeEquipStr(
                            APP_STATE->Config,
                            MinChild->Config,
                            "%f",
                            MinChild->PercentOfParent
                        );
                        ConfigNodeEquipStr(
                            APP_STATE->Config,
                            MaxChild->Config,
                            "%f",
                            MaxChild->PercentOfParent
                        );
                    }
                }
            }
        }

        for (
            ConfigPanelNode* Panel = PanelTree.Root;
            Panel != EMPTY_CFG_PANEL_NODE_VALUE;
            Panel = ConfigPanelNodeRecDepthFirstPre(PanelTree.Root, Panel).Next
        ) {
            if (Panel->Head != EMPTY_CFG_PANEL_NODE_VALUE)
                continue;

            b32 PanelIsFocused = (
                !APP_STATE->MenuBarFocused &&
                !QueryIsOpen &&
                !PopUpIsOpen &&
                !UIAnyContextMenuIsOpen() &&
                PanelTree.Focused == Panel
            );
            ConfigNode* SelectedTab = Panel->SelectedTab;
            AppViewState* SelectedTabViewState = AppViewStateFromConfig(SelectedTab);

            UIFocus((PanelIsFocused) ? UI_FOCUS_KIND_NULL : UI_FOCUS_KIND_OFF) {
                r2f32 PanelRect = ConfigTargetRectFromPanelNode(ContentRect, PanelTree.Root, Panel);

                if (PanelRect.X1 < ContentRect.X1)
                    PanelRect.X1 -= 1.0f;

                r2f32 TabBarRect = Rng(
                    PanelRect.X0,
                    PanelRect.Y0,
                    PanelRect.X1,
                    PanelRect.Y0 + 1.0f
                );
                r2f32 PanelContentRect = Rng(
                    PanelRect.X0,
                    PanelRect.Y0 + 1.0f,
                    PanelRect.X1,
                    PanelRect.Y1
                );

                if (Panel->TabSide == SIDE_MAX) {
                    TabBarRect.Y0 = PanelRect.Y1 - 1.0f;
                    TabBarRect.Y1 = PanelRect.Y1;
                    PanelContentRect.Y0 = PanelRect.Y0;
                    PanelContentRect.Y1 = PanelRect.Y1 - 1.0f;
                }

                b32 BuildPanel = (
                    PanelContentRect.X1 > PanelContentRect.X0 &&
                    PanelContentRect.Y1 > PanelContentRect.Y0
                );

                if (
                    BuildPanel &&
                    AppDragIsActive() &&
                    APP_STATE->DragDropRegistersSlot == APP_REGISTERS_SLOT_VIEW
                ) {
                    ConfigNode* DragTab = ConfigNodeFromID(
                        APP_STATE->DragDropRegisters->View
                    );
                    b32 DragIsFromThisPanel = (DragTab->Parent == Panel->Config);
                    f32 SideX = FloorF32(Length(PanelContentRect).X / 4.0f);
                    f32 SideY = FloorF32(Length(PanelContentRect).Y / 4.0f);

                    struct {
                        Direction2D Direction;
                        r2f32       Rect;
                    } Sites[] = {
                        {
                            DIR_2D_LEFT,
                            Rng(
                                PanelContentRect.X0,
                                PanelContentRect.Y0,
                                PanelContentRect.X0 + SideX,
                                PanelContentRect.Y1
                            )
                        },
                        {
                            DIR_2D_RIGHT,
                            Rng(
                                PanelContentRect.X1 - SideX,
                                PanelContentRect.Y0,
                                PanelContentRect.X1,
                                PanelContentRect.Y1
                            )
                        },
                        {
                            DIR_2D_UP,
                            Rng(
                                PanelContentRect.X0 + SideX,
                                PanelContentRect.Y0,
                                PanelContentRect.X1 - SideX,
                                PanelContentRect.Y0 + SideY
                            )
                        },
                        {
                            DIR_2D_DOWN,
                            Rng(
                                PanelContentRect.X0 + SideX,
                                PanelContentRect.Y1 - SideY,
                                PanelContentRect.X1 - SideX,
                                PanelContentRect.Y1
                            )
                        },
                        {
                            DIR_2D_INVALID,
                            Rng(
                                PanelContentRect.X0 + SideX,
                                PanelContentRect.Y0 + SideY,
                                PanelContentRect.X1 - SideX,
                                PanelContentRect.Y1 - SideY
                            )
                        }
                    };

                    for (u64 Index = 0; Index < ARRAY_COUNT(Sites); ++Index) {
                        Direction2D Direction = Sites[Index].Direction;

                        if (
                            DragIsFromThisPanel &&
                            (
                                Direction == DIR_2D_INVALID ||
                                Panel->Tabs.Count < 2
                            )
                        ) {
                            continue;
                        }

                        UIBox* SiteBox = EMPTY_UI_BOX_VALUE;

                        UIRect(Sites[Index].Rect) {
                            SiteBox = UIBuildBoxFromStrFmt(
                                UI_BOX_KIND_DROP_SITE,
                                "###drop_site_%d_%p",
                                (i32) Direction,
                                Panel->Config
                            );
                        }

                        UISignalFromBox(SiteBox);

                        if (SiteBox->Key != UIDropHotKey())
                            continue;

                        r2f32 FutureRect = PanelRect;

                        if (Direction != DIR_2D_INVALID) {
                            Axis2D SplitAxis = Axis2DFromDir2D(Direction);
                            SideKind SplitSide = SideFromDir2D(Direction);

                            FutureRect.V[FLIP_SIDE(SplitSide)].V[SplitAxis] = FloorF32(
                                Centre(PanelRect).V[SplitAxis]
                            );
                        }

                        UIRect(FutureRect) {
                            UITag("drop_site"_s8) {
                                UIBuildBoxFromKey(
                                    UI_BOX_KIND_DRAW_BACKGROUND,
                                    EMPTY_UI_KEY_VALUE
                                );
                            }
                        }

                        if (AppDragDrop()) {
                            if (Direction != DIR_2D_INVALID) {
                                AppCmd(
                                    APP_COMMAND_KIND_SPLIT_PANEL,
                                    __Registers.DstPanel = Panel->Config->ID,
                                    __Registers.Panel = APP_STATE->DragDropRegisters->Panel,
                                    __Registers.Tab = APP_STATE->DragDropRegisters->View,
                                    __Registers.View = APP_STATE->DragDropRegisters->View,
                                    __Registers.Direction = Direction
                                );
                            } else {
                                AppCmd(
                                    APP_COMMAND_KIND_MOVE_VIEW,
                                    __Registers.DstPanel = Panel->Config->ID,
                                    __Registers.Panel = APP_STATE->DragDropRegisters->Panel,
                                    __Registers.Tab = APP_STATE->DragDropRegisters->View,
                                    __Registers.View = APP_STATE->DragDropRegisters->View,
                                    __Registers.PrevTab = ConfigTailNode(&Panel->Tabs)->ID
                                );
                            }
                        }
                    }
                }

                UIBox* PanelBox = EMPTY_UI_BOX_VALUE;

                if (BuildPanel) {
                    UIRect(PanelContentRect) {
                        UIChildLayoutAxis(AXIS_2D_Y) {
                            UIFocus(UI_FOCUS_KIND_ON) {
                                UIKey PanelKey = UIKeyFromStrFmt({}, "panel_box_%p", Panel->Config);

                                PanelBox = UIBuildBoxFromKey(
                                    (
                                        UI_BOX_KIND_MOUSE_CLICKABLE |
                                        UI_BOX_KIND_CLIP |
                                        UI_BOX_KIND_DISABLE_FOCUS_EFFECTS
                                    ),
                                    PanelKey
                                );
                            }
                        }
                    }
                }

                UIBox* LoadingOverlayContainer = EMPTY_UI_BOX_VALUE;

                if (BuildPanel) {
                    UIParent(PanelBox) {
                        UIWidthFill() {
                            UIHeightFill() {
                                LoadingOverlayContainer = UIBuildBoxFromKey(
                                    UI_BOX_KIND_FLOATING, 
                                    EMPTY_UI_KEY_VALUE
                                );
                            }
                        }
                    }
                }

                if (BuildPanel) {
                    UIParent(PanelBox) {
                        UIFocus(
                            (PanelIsFocused) 
                                ? UI_FOCUS_KIND_NULL 
                                : UI_FOCUS_KIND_OFF
                        ) {
                            UIWidthFill() {
                                AppPushRegisters(
                                    __Registers.Panel = Panel->Config->ID,
                                    __Registers.Tab = SelectedTab->ID,
                                    __Registers.View = SelectedTab->ID
                                );

                                {
                                    Str8 ViewFilePath = AppViewFilePath();

                                    if (ViewFilePath.Size)
                                        Registers()->FilePath = ViewFilePath;
                                }

                                UIBox* ViewContainerBox = EMPTY_UI_BOX_VALUE;

                                UIFixedWidth(Length(PanelContentRect).X) {
                                    UIFixedHeight(Length(PanelContentRect).Y) {
                                        UIChildLayoutAxis(AXIS_2D_Y) {
                                            ViewContainerBox = UIBuildBoxFromKey(
                                                0, 
                                                EMPTY_UI_KEY_VALUE
                                            );
                                        }
                                    }
                                }

                                UIParent(ViewContainerBox) {
                                    if (SelectedTab != EMPTY_CFG_NODE_VALUE) {
                                        AppViewUI(PanelContentRect);
                                    } else {
                                        Str8 Commands[] = {
                                            APP_COMMAND_KIND_INFO_TABLE[APP_COMMAND_KIND_OPEN].String,
                                            APP_COMMAND_KIND_INFO_TABLE[APP_COMMAND_KIND_OPEN_FILES].String,
                                            APP_COMMAND_KIND_INFO_TABLE[APP_COMMAND_KIND_OPEN_TAB].String,
                                            APP_COMMAND_KIND_INFO_TABLE[APP_COMMAND_KIND_CLOSE_PANEL].String
                                        };
                                        u64 CommandsCount = ARRAY_COUNT(Commands);

                                        if (Panel->Parent == EMPTY_CFG_PANEL_NODE_VALUE)
                                            --CommandsCount;

                                        UISpacer(UI_PERCENT(1.0f, 0.0f));

                                        for (u64 Index = 0; Index < CommandsCount; ++Index) {
                                            UIRow() {
                                                UISpacer(UI_PERCENT(1.0f, 0.0f));

                                                UIPreferredWidth(UI_PX(40.0f, 1.0f)) {
                                                    UISignal Sig = AppCommandSpecButton(
                                                        Commands[Index]
                                                    );

                                                    if (UI_CLICKED(Sig)) {
                                                        AppCmd(
                                                            APP_COMMAND_KIND_RUN_COMMAND,
                                                            __Registers.CommandName = Commands[Index]
                                                        );
                                                    }
                                                }

                                                UISpacer(UI_PERCENT(1.0f, 0.0f));
                                            }
                                        }

                                        UISpacer(UI_PERCENT(1.0f, 0.0f));
                                    }
                                }

                                AppRegisters* ViewRegs = AppPopRegisters();

                                if (PanelIsFocused)
                                    MemCpy(Registers(), ViewRegs, sizeof(AppRegisters));
                            }
                        }
                    }
                }

                if (BuildPanel) {
                    f32 SelectedTabLoadingT = SelectedTabViewState->LoadingT;

                    if (SelectedTabLoadingT > 0.01f) {
                        UIParent(LoadingOverlayContainer) {
                            UILoadingOverlay(
                                PanelContentRect,
                                SelectedTabLoadingT,
                                SelectedTabViewState->LoadingProgressV,
                                SelectedTabViewState->LoadingProgressVTarget
                            );
                        }
                    }
                }

                if (BuildPanel) {
                    UISignal PanelSig = UISignalFromBox(PanelBox);

                    if (UI_PRESSED(PanelSig)) {
                        AppCmd(
                            APP_COMMAND_KIND_FOCUS_PANEL, 
                            __Registers.Panel = Panel->Config->ID
                        );
                    }
                }

                ConfigPanelNode* ResizeMaxChild = EMPTY_CFG_PANEL_NODE_VALUE;

                for (
                    ConfigPanelNode* P = Panel;
                    Panel->TabSide == SIDE_MIN && P->Parent != EMPTY_CFG_PANEL_NODE_VALUE;
                    P = P->Parent
                ) {
                    if (
                        P->Parent->SplitAxis == AXIS_2D_Y &&
                        P->Prev != EMPTY_CFG_PANEL_NODE_VALUE
                    ) {
                        ResizeMaxChild = P;
                        break;
                    }
                }

                UIBox* TabBarBox = EMPTY_UI_BOX_VALUE;

                if (BuildPanel) {
                    UITag("tab"_s8) {
                        UITag("inactive"_s8) {
                            UIRect(TabBarRect) {

                                if (ResizeMaxChild != EMPTY_CFG_PANEL_NODE_VALUE)
                                    UISetNextHoverCursor(CURSOR_KIND_UP_DOWN);

                                UISetNextChildLayoutAxis(AXIS_2D_X);
                                TabBarBox = UIBuildBoxFromStrFmt(
                                    (
                                        UI_BOX_KIND_CLIP |
                                        UI_BOX_KIND_ALLOW_OVERFLOW_X |
                                        UI_BOX_KIND_VIEW_CLAMP_X |
                                        UI_BOX_KIND_VIEW_SCROLL_X |
                                        UI_BOX_KIND_DRAW_BACKGROUND |
                                        UI_BOX_KIND_CLICKABLE
                                    ),
                                    "tab_bar_%p",
                                    Panel->Config
                                );
                            }
                        }
                    }
                }

                ConfigNode* TabDropPrev = EMPTY_CFG_NODE_VALUE;
                b32 TabDropIsActive = FALSE;

                if (
                    BuildPanel &&
                    AppDragIsActive() &&
                    APP_STATE->DragDropRegistersSlot == APP_REGISTERS_SLOT_VIEW
                ) {
                    ConfigNode* DragTab = ConfigNodeFromID(
                        APP_STATE->DragDropRegisters->View
                    );
                    UIBox* TabDropSiteBox = EMPTY_UI_BOX_VALUE;

                    UIRect(TabBarRect) {
                        TabDropSiteBox = UIBuildBoxFromStrFmt(
                            UI_BOX_KIND_DROP_SITE,
                            "###tab_drop_site_%p",
                            Panel->Config
                        );
                    }

                    UISignalFromBox(TabDropSiteBox);
                    TabDropIsActive = (TabDropSiteBox->Key == UIDropHotKey());

                    for (
                        ConfigNodePtrNode* N = Panel->Tabs.Head;
                        N;
                        N = N->Next
                    ) {
                        UIBox* LastTabBox = UIBoxFromKey(
                            UIKeyFromStrFmt(TabBarBox->Key, "tab_%p", N->V)
                        );

                        if (
                            !IsUIBoxEmpty(LastTabBox) &&
                            Centre(LastTabBox->Rect).X < UIMouse().X
                        ) {
                            TabDropPrev = N->V;
                        }
                    }

                    for (
                        ConfigNodePtrNode* N = Panel->Tabs.Head;
                        N;
                        N = N->Next
                    ) {
                        if (
                            N->V == DragTab &&
                            (
                                TabDropPrev == DragTab ||
                                TabDropPrev == (
                                    N->Prev 
                                        ? N->Prev->V 
                                        : EMPTY_CFG_NODE_VALUE
                                )
                            )
                        ) {
                            TabDropIsActive = FALSE;
                        }
                    }
                }

                if (BuildPanel) {
                    UIFocus(UI_FOCUS_KIND_OFF) {
                        UIParent(TabBarBox) {
                            UITag("tab"_s8) {
                                if (
                                    TabDropIsActive &&
                                    TabDropPrev == EMPTY_CFG_NODE_VALUE
                                ) {
                                    UISetNextPreferredWidth(UI_PX(1.0f, 1.0f));
                                    UISetNextBackgroundColour(UIColourFromName("hover"_s8));
                                    UIBuildBoxFromKey(
                                        UI_BOX_KIND_DRAW_BACKGROUND,
                                        EMPTY_UI_KEY_VALUE
                                    );
                                }

                                for (
                                    ConfigNodePtrNode* N = Panel->Tabs.Head;
                                    N;
                                    N = N->Next
                                ) {
                                    ConfigNode* Tab = N->V;

                                    AppRegistersScope(
                                        __Registers.Panel = Panel->Config->ID,
                                        __Registers.View = Tab->ID,
                                        __Registers.Tab = Tab->ID
                                    ) {
                                        b32 TabIsSelected = (Tab == Panel->SelectedTab);

                                        UITag(
                                            (!TabIsSelected) 
                                                ? "inactive"_s8 
                                                : (PanelTree.Focused != Panel)
                                                    ? "unfocused"_s8
                                                    : ""_s8
                                        ) {
                                            FancyStrList String = AppTitleFStrFromConfig(
                                                Scratch.MemPool, 
                                                Tab
                                            );
                                            f32 TabWidth = DDimensionsFromFancyStrList(
                                                UIHeadTabSize(), 
                                                &String
                                            ).X + 2.0f;

                                            UISetNextChildLayoutAxis(AXIS_2D_X);
                                            UISetNextPreferredWidth(UI_SUM_OF_CHILDREN(1.0f));

                                            UIBox* TabBox = UIBuildBoxFromStrFmt(
                                                (
                                                    UI_BOX_KIND_DRAW_HOT_EFFECTS |
                                                    UI_BOX_KIND_DRAW_BACKGROUND |
                                                    UI_BOX_KIND_CLICKABLE
                                                ),
                                                "tab_%p",
                                                Tab
                                            );

                                            UIParent(TabBox) {
                                                UIPreferredWidth(UI_PX(TabWidth, 1.0f)) {
                                                    UIBox* NameBox = UIBuildBoxFromKey(
                                                        UI_BOX_KIND_DRAW_TEXT, 
                                                        EMPTY_UI_KEY_VALUE
                                                    );

                                                    UIBoxEquipDisplayFancyStrs(NameBox, &String);
                                                }

                                                if (TabIsSelected) {
                                                    UIPreferredWidth(UI_PX(3.0f, 1.0f)) {
                                                        UITextAlignment(UI_TEXT_ALIGN_CENTRE) {
                                                            UIBox* CloseBox = UIBuildBoxFromStrFmt(
                                                                (
                                                                    UI_BOX_KIND_CLICKABLE |
                                                                    UI_BOX_KIND_DRAW_TEXT |
                                                                    UI_BOX_KIND_DRAW_HOT_EFFECTS |
                                                                    UI_BOX_KIND_DRAW_ACTIVE_EFFECTS
                                                                ),
                                                                "%S###close_view_%p",
                                                                PRINT_STR(APP_ICON_KIND_TEXT_TABLE[APP_ICON_KIND_X]),
                                                                Tab
                                                            );
                                                            UISignal Sig = UISignalFromBox(CloseBox);

                                                            if (UI_CLICKED(Sig) || UI_MIDDLE_CLICKED(Sig))
                                                                AppCmd(APP_COMMAND_KIND_CLOSE_TAB);
                                                        }
                                                    }
                                                }
                                            }

                                            {
                                                UISignal Sig = UISignalFromBox(TabBox);

                                                if (UI_PRESSED(Sig)) {
                                                    AppCmd(APP_COMMAND_KIND_FOCUS_TAB);
                                                    AppCmd(APP_COMMAND_KIND_FOCUS_PANEL);
                                                } else if (
                                                    UI_DRAGGING(Sig) &&
                                                    !AppDragIsActive() &&
                                                    Length(UIDragDelta()) > 1.0f
                                                ) {
                                                    AppDragBegin(APP_REGISTERS_SLOT_VIEW);
                                                } else if (UI_RIGHT_CLICKED(Sig)) {
                                                    Str8 TabFilePath = AppViewFilePath();

                                                    APP_STATE->TabCtxMenuTab = Tab->ID;
                                                    APP_STATE->TabCtxMenuFileSize = MIN(
                                                        sizeof(APP_STATE->TabCtxMenuFileBuffer),
                                                        TabFilePath.Size
                                                    );
                                                    MemCpy(
                                                        APP_STATE->TabCtxMenuFileBuffer,
                                                        TabFilePath.Str,
                                                        APP_STATE->TabCtxMenuFileSize
                                                    );
                                                    APP_STATE->TabCtxMenuCursor = TxtPt(
                                                        1,
                                                        1 + APP_STATE->TabCtxMenuFileSize
                                                    );
                                                    APP_STATE->TabCtxMenuMark = APP_STATE->TabCtxMenuCursor;
                                                    UIContextMenuOpen(
                                                        TabCtxMenuKey,
                                                        TabBox->Key,
                                                        Vec(0.0f, 1.0f)
                                                    );
                                                } else if (UI_MIDDLE_CLICKED(Sig)) {
                                                    AppCmd(APP_COMMAND_KIND_CLOSE_TAB);
                                                }
                                            }

                                            if (TabDropIsActive && Tab == TabDropPrev) {
                                                UISetNextPreferredWidth(UI_PX(1.0f, 1.0f));
                                                UISetNextBackgroundColour(UIColourFromName("hover"_s8));
                                                UIBuildBoxFromKey(
                                                    UI_BOX_KIND_DRAW_BACKGROUND,
                                                    EMPTY_UI_KEY_VALUE
                                                );
                                            }
                                        }
                                    }
                                }

                                UITag("inactive"_s8) {
                                    UIPreferredWidth(UI_PX(3.0f, 1.0f)) {
                                        UITextAlignment(UI_TEXT_ALIGN_CENTRE) {
                                            UIBox* AddNewBox = UIBuildBoxFromStrFmt(
                                                (
                                                    UI_BOX_KIND_DRAW_TEXT |
                                                    UI_BOX_KIND_DRAW_HOT_EFFECTS |
                                                    UI_BOX_KIND_DRAW_ACTIVE_EFFECTS |
                                                    UI_BOX_KIND_CLICKABLE |
                                                    UI_BOX_KIND_DISABLE_TEXT_TRUNC
                                                ),
                                                "%S###add_new_tab_button_%p",
                                                PRINT_STR(APP_ICON_KIND_TEXT_TABLE[APP_ICON_KIND_ADD]),
                                                Panel->Config
                                            );
                                            UISignal Sig = UISignalFromBox(AddNewBox);

                                            if (UI_PRESSED(Sig)) {
                                                AppCmd(APP_COMMAND_KIND_FOCUS_PANEL, __Registers.Panel = Panel->Config->ID);

                                                if (APP_STATE->QueryIsActive) {
                                                    AppCmd(APP_COMMAND_KIND_CANCEL_QUERY);
                                                } else {
                                                    AppCmd(
                                                        APP_COMMAND_KIND_RUN_COMMAND,
                                                        __Registers.Panel = Panel->Config->ID,
                                                        __Registers.CommandName = APP_COMMAND_KIND_INFO_TABLE[APP_COMMAND_KIND_OPEN_TAB].String
                                                    );
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    UISignal TabBarSig = UISignalFromBox(TabBarBox);

                    if (ResizeMaxChild != EMPTY_CFG_PANEL_NODE_VALUE) {
                        ConfigPanelNode* ResizeMinChild = ResizeMaxChild->Prev;
                        f32 TotalSize = Length(
                            ConfigTargetRectFromPanelNode(
                                ContentRect,
                                PanelTree.Root,
                                ResizeMaxChild->Parent
                            )
                        ).Y;

                        if (UI_DOUBLE_CLICKED(TabBarSig)) {
                            f32 HalfPercent = 0.5f * (ResizeMinChild->PercentOfParent + ResizeMaxChild->PercentOfParent);

                            UIKillAction();
                            ConfigNodeEquipStr(
                                APP_STATE->Config,
                                ResizeMinChild->Config,
                                "%f",
                                HalfPercent
                            );
                            ConfigNodeEquipStr(
                                APP_STATE->Config,
                                ResizeMaxChild->Config,
                                "%f",
                                HalfPercent
                            );
                        } else if (UI_PRESSED(TabBarSig)) {
                            v2f32 Percents = Vec(
                                ResizeMinChild->PercentOfParent,
                                ResizeMaxChild->PercentOfParent
                            );

                            UIStoreDragStruct(&Percents);
                        } else if (UI_DRAGGING(TabBarSig)) {
                            v2f32 Percents = *UIGetDragStruct(v2f32);
                            f32 BothSize = (Percents.X + Percents.Y) * TotalSize;

                            if (BothSize >= 6.0f) {
                                f32 MinSize = CLAMP(
                                    3.0f,
                                    Percents.X * TotalSize + UIDragDelta().Y,
                                    BothSize - 3.0f
                                );

                                ConfigNodeEquipStr(
                                    APP_STATE->Config,
                                    ResizeMinChild->Config,
                                    "%f",
                                    MinSize / TotalSize
                                );
                                ConfigNodeEquipStr(
                                    APP_STATE->Config,
                                    ResizeMaxChild->Config,
                                    "%f",
                                    (BothSize - MinSize) / TotalSize
                                );
                            }
                        }
                    }

                    if (TabDropIsActive && AppDragDrop()) {
                        AppCmd(
                            APP_COMMAND_KIND_MOVE_VIEW,
                            __Registers.DstPanel = Panel->Config->ID,
                            __Registers.Panel = APP_STATE->DragDropRegisters->Panel,
                            __Registers.Tab = APP_STATE->DragDropRegisters->View,
                            __Registers.View = APP_STATE->DragDropRegisters->View,
                            __Registers.PrevTab = TabDropPrev->ID
                        );
                    }
                }

                {
                    for (UIEvent* Evt = NULL; UINextEvent(&Evt); ) {
                        if (
                            Evt->Kind == UI_EVENT_KIND_FILE_DROP && 
                            InRange(PanelRect, Evt->Position)
                        ) {
                            for (
                                Str8Node* N = Evt->Paths.Head; 
                                N; 
                                N = N->Next
                            ) {
                                AppCmd(
                                    APP_COMMAND_KIND_OPEN,
                                    __Registers.FilePath = N->String,
                                    __Registers.Panel = Panel->Config->ID
                                );
                            }

                            UIEatEvent(Evt);
                        }
                    }
                }
            }
        }
    }

    UIPopPreferredHeight();
    UIPopPreferredWidth();
    UIPopTextPadding();
    UIEndBuild();

    if (
        APP_STATE->BindChangeActive &&
        (
            IsUIBoxEmpty(UIBoxFromKey(APP_STATE->BindChangeUIKey)) ||
            (
                UIActiveKey(UI_MOUSE_BTN_KIND_LEFT) != EMPTY_UI_KEY_VALUE &&
                UIActiveKey(UI_MOUSE_BTN_KIND_LEFT) != APP_STATE->BindChangeUIKey
            )
        )
    ) {
        APP_STATE->BindChangeActive = FALSE;
    }

    if (UIIsAnimatingFromState(APP_STATE->UI))
        AppRequestFrame();

    DBeginFrame(
        AppFrameMemPool(),
        UIColourFromName("text"_s8),
        UIColourFromName("background"_s8)
    );
    UIDrawRoot(UIRootFromState(APP_STATE->UI));
    DEndFrame();
    ++APP_STATE->FramesAlive;
    ReleaseScratch(Scratch);
}
