#if !defined(__WIN32_PLATFORM_H__)
#define __WIN32_PLATFORM_H__

#include "../../nc_types.h"

// @defines____________________________________________________________________
#define _WINVER                              WIN32_WIN10
#define _WIN32_WINNT                         WIN32_WIN10
#define WINVER                               _WINVER
#define WIN32_WINNT                          _WIN32_WINNT
#define ISOLATION_AWARE_ENABLED              TRUE
#define _WIN32_FUSION                        0x0100

#define MAKEWORD(X, Y)                       (WORD) (((Y) & 0xFF) << 8) | ((X) & 0xFF)

#define PASCAL                               __stdcall
#define CALLBACK                             __stdcall
#define WINAPI                               __stdcall
#define WINGDIAPI                            DLL_IMPORT
#define APIENTRY                             WINAPI
#define WINAPIV                              __cdecl
#define NTAPI                                __stdcall
#define WSAAPI                               FAR PASCAL
#define DECLSPEC_ALLOCATOR                   __declspec(allocator)

#define FAR
#define NEAR

#define S_OK                                 ((HRESULT) 0x00000000L)
#define E_NOINTERFACE                        ((HRESULT) 0x80004002L)

#define ATTACH_PARENT_PROCESS                ((DWORD) -1)

#define SE_LOCK_MEMORY_NAME                  TEXT("SeLockMemoryPrivilege")

#define SE_PRIVILEGE_ENABLED                 2

#define CP_UTF8                              65001

#define DLL_PROCESS_DETACH                   0
#define DLL_PROCESS_ATTACH                   1
#define DLL_THREAD_ATTACH                    2
#define DLL_THREAD_DETACH                    3

#define HEAP_GENERATE_EXCEPTIONS             0x00000004
#define HEAP_NO_SERIALIZE                    0x00000001
#define HEAP_ZERO_MEMORY                     0x00000008

#define ENABLE_INSERT_MODE                   0x0020
#define ENABLE_LINE_INPUT                    0x0002
#define ENABLE_MOUSE_INPUT                   0x0010
#define ENABLE_PROCESSED_INPUT               0x0001
#define ENABLE_QUICK_EDIT_MODE               0x0040
#define ENABLE_EXTENDED_FLAGS                0x0080

#define ENABLE_VIRTUAL_TERMINAL_PROCESSING   0x0004

#define INVALID_SOCKET                       (SOCKET)(~0)
#define SOCKET_ERROR                         (-1)

#define SOCK_STREAM                          1
#define SOCK_DGRAM                           2
#define SOCK_RAW                             3
#define SOCK_RDM                             4
#define SOCK_SEQPACKET                       5

#define AF_UNSPEC                             0
#define AF_INET                               2
#define AF_IPX                                6
#define AF_APPLETALK                         16
#define AF_NETBIOS                           17
#define AF_INET6                             23
#define AF_IRDA                              26
#define AF_BTH                               32

#define IPPROTO_IP                             0
#define IPPROTO_ICMP                           1
#define IPPROTO_IGMP                           2
#define IPPROTO_GGP                            3
#define IPPROTO_TCP                            6
#define IPPROTO_PUP                           12
#define IPPROTO_UDP                           17
#define IPPROTO_IDP                           22
#define IPPROTO_ND                            77
#define IPPROTO_RAW                          255
#define IPPROTO_MAX                          256

#define SO_DEBUG                             0x0001
#define SO_ACCEPTCONN                        0x0002
#define SO_REUSEADDR                         0x0004
#define SO_KEEPALIVE                         0x0008
#define SO_DONTROUTE                         0x0010
#define SO_BROADCAST                         0x0020
#define SO_USELOOPBACK                       0x0040
#define SO_LINGER                            0x0080
#define SO_OOBINLINE                         0x0100
#define SO_DONTLINGER                        (INT)(~SO_LINGER)
#define SO_EXCLUSIVEADDRUSE                  ((INT)(~SO_REUSEADDR))
#define SO_SNDBUF                            0x1001
#define SO_RCVBUF                            0x1002
#define SO_SNDLOWAT                          0x1003
#define SO_RCVLOWAT                          0x1004
#define SO_SNDTIMEO                          0x1005
#define SO_RCVTIMEO                          0x1006
#define SO_ERROR                             0x1007
#define SO_TYPE                              0x1008
#define SO_GROUP_ID                          0x2001
#define SO_GROUP_PRIORITY                    0x2002
#define SO_MAX_MSG_SIZE                      0x2003
#define SO_PROTOCOL_INFOA                    0x2004
#define SO_PROTOCOL_INFOW                    0x2005

#if defined(UNICODE) || defined(_UNICODE)
    #define SO_PROTOCOL_INFO                 SO_PROTOCOL_INFOW
#else
    #define SO_PROTOCOL_INFO                 SO_PROTOCOL_INFOA
#endif

#define TCP_NODELAY                          0x0001

#define SOL_SOCKET                           0xFFFF

#define IOCPARM_MASK                         0x7F
#define IOC_VOID                             0x20000000
#define IOC_OUT                              0x40000000
#define IOC_IN                               0x80000000
#define IOC_INOUT                            (IOC_IN | IOC_OUT)
#define _IO(X,Y)                             (IOC_VOID | ((X) << 8) | (Y))
#define _IOR(X,Y,T)                          (IOC_OUT | (((LONG) sizeof(T) & IOCPARM_MASK) << 16) | ((X) << 8) | (Y))
#define _IOW(X,Y,T)                          (IOC_IN | (((LONG) sizeof(T) & IOCPARM_MASK) << 16) | ((X) << 8) | (Y))
#define FIONREAD                             _IOR('f', 127, ULONG)
#define FIONBIO                              _IOW('f', 126, ULONG)
#define FIOASYNC                             _IOW('f', 125, ULONG)

#define ICC_ANIMATE_CLASS                    0x00000080
#define ICC_BAR_CLASSES                      0x00000004
#define ICC_COOL_CLASSES                     0x00000400
#define ICC_DATE_CLASSES                     0x00000100
#define ICC_HOTKEY_CLASS                     0x00000040
#define ICC_INTERNET_CLASSES                 0x00000800
#define ICC_LINK_CLASS                       0x00008000
#define ICC_LISTVIEW_CLASSES                 0x00000001
#define ICC_NATIVEFNTCTL_CLASS               0x00002000
#define ICC_PAGESCROLLER_CLASS               0x00001000
#define ICC_PROGRESS_CLASS                   0x00000020
#define ICC_STANDARD_CLASSES                 0x00004000
#define ICC_TAB_CLASSES                      0x00000008
#define ICC_TREEVIEW_CLASSES                 0x00000002
#define ICC_UPDOWN_CLASS                     0x00000010
#define ICC_USEREX_CLASSES                   0x00000200
#define ICC_WIN95_CLASSES                    0x000000FF

#define TLS_OUT_OF_INDEXES                   ((DWORD) -1)

#define SUCCEEDED(X)                         (((HRESULT) (X)) >= 0)
#define FAILED(X)                            (((HRESULT) (X)) < 0)

#define SD_RECEIVE                           0
#define SD_SEND                              1
#define SD_BOTH                              2

#define SHSTDAPI                             EXTERN_C_LINK DLL_IMPORT HRESULT WINAPI
#define SHFOLDERAPI                          SHSTDAPI

#define OFN_FILEMUSTEXIST                    0x00001000
#define OFN_NOCHANGEDIR                      0x00000008
#define OFN_PATHMUSTEXIST                    0x00000800
#define OFN_EXPLORER                         0x00080000

#define HIWORD(X)                            ((WORD) ((((DWORD_PTR) (X)) >> 16) & 0xFFFF))
#define LOWORD(X)                            ((WORD) (((DWORD_PTR) (X)) & 0xFFFF))

#define GET_Y_LPARAM(X)                      ((i16) ((((DWORD_PTR) (X)) >> 16) & 0xFFFF))
#define GET_X_LPARAM(X)                      ((i16) (((DWORD_PTR) (X)) & 0xFFFF))

#define MAKEINTRESOURCEA(X)                  ((LPSTR)((ULONG_PTR)((WORD)(X))))
#define MAKEINTRESOURCEW(X)                  ((LPWSTR)((ULONG_PTR)((WORD)(X))))

#define ISOLATIONAWARE_MANIFEST_RESOURCE_ID  MAKEINTRESOURCEW(2)

#define INFINITE                             ((DWORD) -1)

#define MAX_PATH                             0x104
#define MAX_COMPUTERNAME_LENGTH              15

#define PFD_DRAW_TO_WINDOW                   0X00000004
#define PFD_DRAW_TO_BITMAP                   0x00000008
#define PFD_SUPPORT_GDI                      0x00000010
#define PFD_SUPPORT_OPENGL                   0x00000020
#define PFD_GENERIC_ACCELERATED              0x00001000
#define PFD_GENERIC_FORMAT                   0x00000040
#define PFD_NEED_PALETTE                     0x00000080
#define PFD_NEED_SYSTEM_PALETTE              0x00000100
#define PFD_DOUBLEBUFFER                     0x00000001
#define PFD_STEREO                           0x00000002
#define PFD_SWAP_LAYER_BUFFERS               0x00000800
#define PFD_DEPTH_DONTCARE                   0x20000000
#define PFD_DOUBLEBUFFER_DONTCARE            0x40000000
#define PFD_STEREO_DONTCARE                  0x80000000
#define PFD_SWAP_COPY                        0x00000400
#define PFD_SWAP_EXCHANGE                    0x00000200
#define PFD_TYPE_RGBA                        0
#define PFD_TYPE_COLORINDEX                  1
#define PFD_MAIN_PLANE                       0

#define PAGE_NOACCESS                        0x01
#define PAGE_READONLY                        0x02
#define PAGE_READWRITE                       0x04
#define PAGE_WRITECOPY                       0x08
#define PAGE_EXECUTE                         0x10
#define PAGE_EXECUTE_READ                    0x20
#define PAGE_EXECUTE_READWRITE               0x40
#define PAGE_EXECUTE_WRITECOPY               0x80
#define PAGE_GUARD                           0x100
#define PAGE_NOCACHE                         0x200
#define PAGE_WRITECOMBINE                    0x400
#define PAGE_ENCLAVE_THREAD_CONTROL          0x80000000
#define PAGE_REVERT_TO_FILE_MAP              0x80000000
#define PAGE_TARGETS_NO_UPDATE               0x40000000
#define PAGE_TARGETS_INVALID                 0x40000000
#define PAGE_ENCLAVE_UNVALIDATED             0x20000000
#define PAGE_ENCLAVE_DECOMMIT                0x10000000

#define MEM_COMMIT                           0x00001000
#define MEM_RESERVE                          0x00002000
#define MEM_REPLACE_PLACEHOLDER              0x00004000
#define MEM_RESERVE_PLACEHOLDER              0x00040000
#define MEM_RESET                            0x00080000
#define MEM_TOP_DOWN                         0x00100000
#define MEM_WRITE_WATCH                      0x00200000
#define MEM_PHYSICAL                         0x00400000
#define MEM_ROTATE                           0x00800000
#define MEM_DIFFERENT_IMAGE_BASE_OK          0x00800000
#define MEM_RESET_UNDO                       0x01000000
#define MEM_LARGE_PAGES                      0x20000000
#define MEM_4MB_PAGES                        0x80000000
#define MEM_64K_PAGES                        (MEM_LARGE_PAGES | MEM_PHYSICAL)
#define MEM_UNMAP_WITH_TRANSIENT_BOOST       0x00000001
#define MEM_COALESCE_PLACEHOLDERS            0x00000001
#define MEM_PRESERVE_PLACEHOLDER             0x00000002
#define MEM_DECOMMIT                         0x00004000
#define MEM_RELEASE                          0x00008000
#define MEM_FREE                             0x00010000

#define LHND                                 0x0042
#define LMEM_FIXED                           0x0000
#define LMEM_MOVEABLE                        0x0002
#define LMEM_ZEROINIT                        0x0040
#define LPTR                                 (LMEM_FIXED | LMEM_ZEROINIT)
#define NONZEROLHND                          LMEM_MOVEABLE
#define NONZEROLPTR                          LMEM_FIXED

#define ACTCTX_FLAG_PROCESSOR_ARCHITECTURE_VALID 0x001
#define ACTCTX_FLAG_LANGID_VALID                 0x002
#define ACTCTX_FLAG_ASSEMBLY_DIRECTORY_VALID     0x004
#define ACTCTX_FLAG_RESOURCE_NAME_VALID          0x008
#define ACTCTX_FLAG_SET_PROCESS_DEFAULT          0x010
#define ACTCTX_FLAG_APPLICATION_NAME_VALID       0x020
#define ACTCTX_FLAG_HMODULE_VALID                0x080

#define INVALID_HANDLE_VALUE                 ((HANDLE) -1)

#define SECTION_QUERY                        0x0001
#define SECTION_MAP_WRITE                    0x0002
#define SECTION_MAP_READ                     0x0004
#define SECTION_MAP_EXECUTE                  0x0008
#define SECTION_EXTEND_SIZE                  0x0010
#define SECTION_MAP_EXECUTE_EXPLICIT         0x0020
#define SECTION_ALL_ACCESS                   (STANDARD_RIGHTS_REQUIRED | SECTION_QUERY | SECTION_MAP_WRITE | SECTION_MAP_READ | SECTION_MAP_EXECUTE | SECTION_EXTEND_SIZE)

#define FILE_MAP_WRITE                       SECTION_MAP_WRITE
#define FILE_MAP_READ                        SECTION_MAP_READ
#define FILE_MAP_ALL_ACCESS                  SECTION_ALL_ACCESS
#define FILE_MAP_EXECUTE                     SECTION_MAP_EXECUTE_EXPLICIT
#define FILE_MAP_COPY                        0x00000001
#define FILE_MAP_RESERVE                     0x80000000
#define FILE_MAP_TARGETS_INVALID             0x40000000
#define FILE_MAP_LARGE_PAGES                 0x20000000

#define FILE_SHARE_READ                      0x00000001  
#define FILE_SHARE_WRITE                     0x00000002  
#define FILE_SHARE_DELETE                    0x00000004

#define FILE_ATTRIBUTE_READONLY              0x00000001  
#define FILE_ATTRIBUTE_HIDDEN                0x00000002  
#define FILE_ATTRIBUTE_SYSTEM                0x00000004  
#define FILE_ATTRIBUTE_DIRECTORY             0x00000010  
#define FILE_ATTRIBUTE_ARCHIVE               0x00000020  
#define FILE_ATTRIBUTE_DEVICE                0x00000040  
#define FILE_ATTRIBUTE_NORMAL                0x00000080  
#define FILE_ATTRIBUTE_TEMPORARY             0x00000100  
#define FILE_ATTRIBUTE_SPARSE_FILE           0x00000200  
#define FILE_ATTRIBUTE_REPARSE_POINT         0x00000400  
#define FILE_ATTRIBUTE_COMPRESSED            0x00000800  
#define FILE_ATTRIBUTE_OFFLINE               0x00001000  
#define FILE_ATTRIBUTE_NOT_CONTENT_INDEXED   0x00002000  
#define FILE_ATTRIBUTE_ENCRYPTED             0x00004000  
#define FILE_ATTRIBUTE_INTEGRITY_STREAM      0x00008000  
#define FILE_ATTRIBUTE_VIRTUAL               0x00010000  
#define FILE_ATTRIBUTE_NO_SCRUB_DATA         0x00020000  
#define FILE_ATTRIBUTE_EA                    0x00040000  
#define FILE_ATTRIBUTE_PINNED                0x00080000  
#define FILE_ATTRIBUTE_UNPINNED              0x00100000  
#define FILE_ATTRIBUTE_RECALL_ON_OPEN        0x00040000  
#define FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS 0x00400000
#define INVALID_FILE_ATTRIBUTES              ((DWORD) -1)

#define CREATE_NEW                           1
#define CREATE_ALWAYS                        2
#define OPEN_EXISTING                        3
#define OPEN_ALWAYS                          4
#define TRUNCATE_EXISTING                    5

#define STANDARD_RIGHTS_REQUIRED             0x000F0000L
#define STANDARD_RIGHTS_READ                 READ_CONTROL
#define STANDARD_RIGHTS_WRITE                READ_CONTROL
#define STANDARD_RIGHTS_EXECUTE              READ_CONTROL
#define STANDARD_RIGHTS_ALL                  0x001F0000L
#define SPECIFIC_RIGHTS_ALL                  0x0000FFFFL

#define FILE_READ_DATA                       0x0001
#define FILE_LIST_DIRECTORY                  0x0001
#define FILE_WRITE_DATA                      0x0002
#define FILE_ADD_FILE                        0x0002
#define FILE_APPEND_DATA                     0x0004
#define FILE_ADD_SUBDIRECTORY                0x0004
#define FILE_CREATE_PIPE_INSTANCE            0x0004
#define FILE_READ_EA                         0x0008
#define FILE_WRITE_EA                        0x0010
#define FILE_EXECUTE                         0x0020
#define FILE_TRAVERSE                        0x0020
#define FILE_DELETE_CHILD                    0x0040
#define FILE_READ_ATTRIBUTES                 0x0080
#define FILE_WRITE_ATTRIBUTES                0x0100
#define FILE_ALL_ACCESS                      (STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | 0x1FF)

#define FILE_GENERIC_READ                    (STANDARD_RIGHTS_READ | FILE_READ_DATA | FILE_READ_ATTRIBUTES | FILE_READ_EA | SYNCHRONIZE)
#define FILE_GENERIC_WRITE                   (STANDARD_RIGHTS_WRITE | FILE_WRITE_DATA | FILE_WRITE_ATTRIBUTES | FILE_WRITE_EA | FILE_APPEND_DATA | SYNCHRONIZE)
#define FILE_GENERIC_EXECUTE                 (STANDARD_RIGHTS_EXECUTE | FILE_READ_ATTRIBUTES | FILE_EXECUTE | SYNCHRONIZE)

#define GENERIC_READ                         0x80000000L
#define GENERIC_WRITE                        0x40000000L
#define GENERIC_EXECUTE                      0x20000000L
#define GENERIC_ALL                          0x10000000L

#define FIND_FIRST_EX_CASE_SENSITIVE         1
#define FIND_FIRST_EX_LARGE_FETCH            2
#define FIND_FIRST_EX_ON_DISK_ENTRIES_ONLY   4

#define EXCEPTION_EXECUTE_HANDLER            1
#define EXCEPTION_CONTINUE_SEARCH            0
#define EXCEPTION_CONTINUE_EXECUTION         (-1)
#define EXCEPTION_MAXIMUM_PARAMETERS         ((DWORD) 15)

#define SHGFP_TYPE_CURRENT                   0

#define WAIT_ABANDONED                       0x00000080L
#define WAIT_OBJECT_0                        0x00000000L
#define WAIT_TIMEOUT                         0x00000102L
#define WAIT_FAILED                          ((DWORD) -1)

#define CONDITION_VARIABLE_LOCKMODE_SHARED   1

#define SEMAPHORE_ALL_ACCESS                 0x1F0003

#define CREATE_NO_WINDOW                     0x08000000
#define CREATE_UNICODE_ENVIRONMENT           0x00000400

#define STARTF_USESTDHANDLES                 0x00000100

#define WA_INACTIVE                          0
#define WA_ACTIVE                            1
#define WA_CLICKACTIVE                       2

#define WM_NULL                              0x0000
#define WM_CREATE                            0x0001
#define WM_DESTROY                           0x0002
#define WM_MOVE                              0x0003
#define WM_SIZE                              0x0005
#define WM_ACTIVATE                          0x0006
#define WM_SETFOCUS                          0x0007
#define WM_KILLFOCUS                         0x0008
#define WM_ENABLE                            0x000A
#define WM_SETREDRAW                         0x000B
#define WM_SETTEXT                           0x000C
#define WM_GETTEXT                           0x000D
#define WM_GETTEXTLENGTH                     0x000E
#define WM_PAINT                             0x000F
#define WM_CLOSE                             0x0010

#if !defined(_WIN32_WCE)
    #define WM_QUERYENDSESSION               0x0011
    #define WM_QUERYOPEN                     0x0013
    #define WM_ENDSESSION                    0x0016
#endif

#define WM_QUIT                              0x0012
#define WM_ERASEBKGND                        0x0014
#define WM_SYSCOLORCHANGE                    0x0015
#define WM_SHOWWINDOW                        0x0018
#define WM_WININICHANGE                      0x001A

#if(WINVER >= WIN32_NT4)
    #define WM_SETTINGCHANGE                 WM_WININICHANGE
#endif

#define WM_DEVMODECHANGE                     0x001B
#define WM_ACTIVATEAPP                       0x001C
#define WM_FONTCHANGE                        0x001D
#define WM_TIMECHANGE                        0x001E
#define WM_CANCELMODE                        0x001F
#define WM_SETCURSOR                         0x0020
#define WM_MOUSEACTIVATE                     0x0021
#define WM_CHILDACTIVATE                     0x0022
#define WM_QUEUESYNC                         0x0023
#define WM_GETMINMAXINFO                     0x0024
#define WM_PAINTICON                         0x0026
#define WM_ICONERASEBKGND                    0x0027
#define WM_NEXTDLGCTL                        0x0028
#define WM_SPOOLERSTATUS                     0x002A
#define WM_DRAWITEM                          0x002B
#define WM_MEASUREITEM                       0x002C
#define WM_DELETEITEM                        0x002D
#define WM_VKEYTOITEM                        0x002E
#define WM_CHARTOITEM                        0x002F
#define WM_SETFONT                           0x0030
#define WM_GETFONT                           0x0031
#define WM_SETHOTKEY                         0x0032
#define WM_GETHOTKEY                         0x0033
#define WM_QUERYDRAGICON                     0x0037
#define WM_COMPAREITEM                       0x0039
#define WM_POWER                             0x0048

#define PWR_OK                               1
#define PWR_FAIL                             (-1)
#define PWR_SUSPENDREQUEST                   1
#define PWR_SUSPENDRESUME                    2
#define PWR_CRITICALRESUME                   3

#define WM_COPYDATA                          0x004A
#define WM_CANCELJOURNAL                     0x004B

#if(WINVER >= 0x0400)
    #define WM_NOTIFY                        0x004E
    #define WM_INPUTLANGCHANGEREQUEST        0x0050
    #define WM_INPUTLANGCHANGE               0x0051
    #define WM_TCARD                         0x0052
    #define WM_HELP                          0x0053
    #define WM_USERCHANGED                   0x0054
    #define WM_NOTIFYFORMAT                  0x0055

    #define NFR_ANSI                         1
    #define NFR_UNICODE                      2
    #define NF_QUERY                         3
    #define NF_REQUERY                       4

    #define WM_CONTEXTMENU                   0x007B
    #define WM_STYLECHANGING                 0x007C
    #define WM_STYLECHANGED                  0x007D
    #define WM_DISPLAYCHANGE                 0x007E
    #define WM_GETICON                       0x007F
    #define WM_SETICON                       0x0080
#endif

#define WM_NCCREATE                          0x0081
#define WM_NCDESTROY                         0x0082
#define WM_NCCALCSIZE                        0x0083
#define WM_NCHITTEST                         0x0084
#define WM_NCPAINT                           0x0085
#define WM_NCACTIVATE                        0x0086
#define WM_GETDLGCODE                        0x0087

#if !defined(_WIN32_WCE)
    #define WM_SYNCPAINT                     0x0088
#endif

#define WM_NCMOUSEMOVE                       0x00A0
#define WM_NCLBUTTONDOWN                     0x00A1
#define WM_NCLBUTTONUP                       0x00A2
#define WM_NCLBUTTONDBLCLK                   0x00A3
#define WM_NCRBUTTONDOWN                     0x00A4
#define WM_NCRBUTTONUP                       0x00A5
#define WM_NCRBUTTONDBLCLK                   0x00A6
#define WM_NCMBUTTONDOWN                     0x00A7
#define WM_NCMBUTTONUP                       0x00A8
#define WM_NCMBUTTONDBLCLK                   0x00A9

#if(_WIN32_WINNT >= WIN32_2000)
    #define WM_NCXBUTTONDOWN                 0x00AB
    #define WM_NCXBUTTONUP                   0x00AC
    #define WM_NCXBUTTONDBLCLK               0x00AD
#endif


#if(_WIN32_WINNT >= WIN32_WINXP)
    #define WM_INPUT_DEVICE_CHANGE           0x00FE
#endif

#if(_WIN32_WINNT >= WIN32_WINXP)
    #define WM_INPUT                         0x00FF
#endif

#define WM_KEYFIRST                          0x0100
#define WM_KEYDOWN                           0x0100
#define WM_KEYUP                             0x0101
#define WM_CHAR                              0x0102
#define WM_DEADCHAR                          0x0103
#define WM_SYSKEYDOWN                        0x0104
#define WM_SYSKEYUP                          0x0105
#define WM_SYSCHAR                           0x0106
#define WM_SYSDEADCHAR                       0x0107

#if(_WIN32_WINNT >= WIN32_WINXP)
    #define WM_UNICHAR                       0x0109
    #define WM_KEYLAST                       0x0109
    #define UNICODE_NOCHAR                   0xFFFF
#else
    #define WM_KEYLAST                       0x0108
#endif

#if(WINVER >= WIN32_NT4)
    #define WM_IME_STARTCOMPOSITION          0x010D
    #define WM_IME_ENDCOMPOSITION            0x010E
    #define WM_IME_COMPOSITION               0x010F
    #define WM_IME_KEYLAST                   0x010F
#endif

#define WM_INITDIALOG                        0x0110
#define WM_COMMAND                           0x0111
#define WM_SYSCOMMAND                        0x0112
#define WM_TIMER                             0x0113
#define WM_HSCROLL                           0x0114
#define WM_VSCROLL                           0x0115
#define WM_INITMENU                          0x0116
#define WM_INITMENUPOPUP                     0x0117

#if(WINVER >= WIN32_WIN7)
    #define WM_GESTURE                       0x0119
    #define WM_GESTURENOTIFY                 0x011A
#endif

#define WM_MENUSELECT                        0x011F
#define WM_MENUCHAR                          0x0120
#define WM_ENTERIDLE                         0x0121

#if(WINVER >= WIN32_2000)
    #if !defined(_WIN32_WCE)
        #define WM_MENURBUTTONUP             0x0122
        #define WM_MENUDRAG                  0x0123
        #define WM_MENUGETOBJECT             0x0124
        #define WM_UNINITMENUPOPUP           0x0125
        #define WM_MENUCOMMAND               0x0126

        #if !defined(_WIN32_WCE)
            #if(_WIN32_WINNT >= WIN32_2000)
                #define WM_CHANGEUISTATE     0x0127
                #define WM_UPDATEUISTATE     0x0128
                #define WM_QUERYUISTATE      0x0129
                #define UIS_SET              1
                #define UIS_CLEAR            2
                #define UIS_INITIALIZE       3
                #define UISF_HIDEFOCUS       0x1
                #define UISF_HIDEACCEL       0x2

                #if(_WIN32_WINNT >= WIN32_WINXP)
                    #define UISF_ACTIVE      0x4
                #endif
            #endif
        #endif
    #endif
#endif

#define WM_CTLCOLORMSGBOX                    0x0132
#define WM_CTLCOLOREDIT                      0x0133
#define WM_CTLCOLORLISTBOX                   0x0134
#define WM_CTLCOLORBTN                       0x0135
#define WM_CTLCOLORDLG                       0x0136
#define WM_CTLCOLORSCROLLBAR                 0x0137
#define WM_CTLCOLORSTATIC                    0x0138
#define MN_GETHMENU                          0x01E1
#define WM_MOUSEFIRST                        0x0200
#define WM_MOUSEMOVE                         0x0200
#define WM_LBUTTONDOWN                       0x0201
#define WM_LBUTTONUP                         0x0202
#define WM_LBUTTONDBLCLK                     0x0203
#define WM_RBUTTONDOWN                       0x0204
#define WM_RBUTTONUP                         0x0205
#define WM_RBUTTONDBLCLK                     0x0206
#define WM_MBUTTONDOWN                       0x0207
#define WM_MBUTTONUP                         0x0208
#define WM_MBUTTONDBLCLK                     0x0209

#if (_WIN32_WINNT >= WIN32_NT4) || (_WIN32_WINDOWS > WIN32_NT4)
    #define WM_MOUSEWHEEL                    0x020A
#endif

#if (_WIN32_WINNT >= WIN32_2000)
    #define WM_XBUTTONDOWN                   0x020B
    #define WM_XBUTTONUP                     0x020C
    #define WM_XBUTTONDBLCLK                 0x020D
#endif

#if (_WIN32_WINNT >= WIN32_VISTA)
    #define WM_MOUSEHWHEEL                   0x020E
#endif

#if (_WIN32_WINNT >= WIN32_VISTA)
    #define WM_MOUSELAST                     0x020E
#elif (_WIN32_WINNT >= WIN32_2000)
    #define WM_MOUSELAST                     0x020D
#elif (_WIN32_WINNT >= WIN32_NT4) || (_WIN32_WINDOWS > WIN32_NT4)
    #define WM_MOUSELAST                     0x020A
#else
    #define WM_MOUSELAST                     0x0209
#endif

#if (_WIN32_WINNT >= WIN32_NT4)
    #define WHEEL_DELTA                      120
    #define GET_WHEEL_DELTA_WPARAM(wParam)   ((short) HIWORD(wParam))
    #define WHEEL_PAGESCROLL                 (UINT_MAX)
#endif

#if (_WIN32_WINNT >= WIN32_2000)
    #define GET_KEYSTATE_WPARAM(X)           (LOWORD(X))
    #define GET_NCHITTEST_WPARAM(X)          ((short) LOWORD(X))
    #define GET_XBUTTON_WPARAM(X)            (HIWORD(X))
    #define XBUTTON1                         0x0001
    #define XBUTTON2                         0x0002
#endif

#define WM_PARENTNOTIFY                      0x0210
#define WM_ENTERMENULOOP                     0x0211
#define WM_EXITMENULOOP                      0x0212

#if (WINVER >= WIN32_NT4)
    #define WM_NEXTMENU                      0x0213
    #define WM_SIZING                        0x0214
    #define WM_CAPTURECHANGED                0x0215
    #define WM_MOVING                        0x0216
#endif

#if (WINVER >= WIN32_NT4)
    #define WM_POWERBROADCAST                0x0218

    #if !defined(_WIN32_WCE)
        #define PBT_APMQUERYSUSPEND          0x0000
        #define PBT_APMQUERYSTANDBY          0x0001
        #define PBT_APMQUERYSUSPENDFAILED    0x0002
        #define PBT_APMQUERYSTANDBYFAILED    0x0003
        #define PBT_APMSUSPEND               0x0004
        #define PBT_APMSTANDBY               0x0005
        #define PBT_APMRESUMECRITICAL        0x0006
        #define PBT_APMRESUMESUSPEND         0x0007
        #define PBT_APMRESUMESTANDBY         0x0008
        #define PBTF_APMRESUMEFROMFAILURE    0x00000001
        #define PBT_APMBATTERYLOW            0x0009
        #define PBT_APMPOWERSTATUSCHANGE     0x000A
        #define PBT_APMOEMEVENT              0x000B
        #define PBT_APMRESUMEAUTOMATIC       0x0012

        #if (_WIN32_WINNT >= WIN32_SERVER_2003)
            #if !defined(PBT_POWERSETTINGCHANGE)
                #define PBT_POWERSETTINGCHANGE 0x8013
            #endif
        #endif
    #endif
#endif

#if (WINVER >= WIN32_NT4)
    #define WM_DEVICECHANGE                  0x0219
#endif

#define WM_MDICREATE                         0x0220
#define WM_MDIDESTROY                        0x0221
#define WM_MDIACTIVATE                       0x0222
#define WM_MDIRESTORE                        0x0223
#define WM_MDINEXT                           0x0224
#define WM_MDIMAXIMIZE                       0x0225
#define WM_MDITILE                           0x0226
#define WM_MDICASCADE                        0x0227
#define WM_MDIICONARRANGE                    0x0228
#define WM_MDIGETACTIVE                      0x0229
#define WM_MDISETMENU                        0x0230
#define WM_ENTERSIZEMOVE                     0x0231
#define WM_EXITSIZEMOVE                      0x0232
#define WM_DROPFILES                         0x0233
#define WM_MDIREFRESHMENU                    0x0234

#if (WINVER >= WIN32_WIN8)
    #define WM_POINTERDEVICECHANGE           0x238
    #define WM_POINTERDEVICEINRANGE          0x239
    #define WM_POINTERDEVICEOUTOFRANGE       0x23A
#endif


#if (WINVER >= WIN32_WIN7)
    #define WM_TOUCH                         0x0240
#endif

#if (WINVER >= WIN32_WIN8)
    #define WM_NCPOINTERUPDATE               0x0241
    #define WM_NCPOINTERDOWN                 0x0242
    #define WM_NCPOINTERUP                   0x0243
    #define WM_POINTERUPDATE                 0x0245
    #define WM_POINTERDOWN                   0x0246
    #define WM_POINTERUP                     0x0247
    #define WM_POINTERENTER                  0x0249
    #define WM_POINTERLEAVE                  0x024A
    #define WM_POINTERACTIVATE               0x024B
    #define WM_POINTERCAPTURECHANGED         0x024C
    #define WM_TOUCHHITTESTING               0x024D
    #define WM_POINTERWHEEL                  0x024E
    #define WM_POINTERHWHEEL                 0x024F
    #define DM_POINTERHITTEST                0x0250
    #define WM_POINTERROUTEDTO               0x0251
    #define WM_POINTERROUTEDAWAY             0x0252
    #define WM_POINTERROUTEDRELEASED         0x0253
#endif

#if (WINVER >= WIN32_NT4)
    #define WM_IME_SETCONTEXT                0x0281
    #define WM_IME_NOTIFY                    0x0282
    #define WM_IME_CONTROL                   0x0283
    #define WM_IME_COMPOSITIONFULL           0x0284
    #define WM_IME_SELECT                    0x0285
    #define WM_IME_CHAR                      0x0286
#endif

#if (WINVER >= WIN32_2000)
    #define WM_IME_REQUEST                   0x0288
#endif

#if (WINVER >= WIN32_NT4)
    #define WM_IME_KEYDOWN                   0x0290
    #define WM_IME_KEYUP                     0x0291
#endif

#if (_WIN32_WINNT >= WIN32_NT4) || (WINVER >= WIN32_2000)
    #define WM_MOUSEHOVER                    0x02A1
    #define WM_MOUSELEAVE                    0x02A3
#endif

#if (WINVER >= WIN32_2000)
    #define WM_NCMOUSEHOVER                  0x02A0
    #define WM_NCMOUSELEAVE                  0x02A2
#endif

#if (_WIN32_WINNT >= WIN32_WINXP)
    #define WM_WTSSESSION_CHANGE             0x02B1
    #define WM_TABLET_FIRST                  0x02C0
    #define WM_TABLET_LAST                   0x02DF
#endif

#if (WINVER >= WIN32_WIN7)
    #define WM_DPICHANGED                    0x02E0
#endif

#if (WINVER >= WIN32_SERVER_2016)
    #define WM_DPICHANGED_BEFOREPARENT       0x02E2
    #define WM_DPICHANGED_AFTERPARENT        0x02E3
    #define WM_GETDPISCALEDSIZE              0x02E4
#endif

#define WM_CUT                               0x0300
#define WM_COPY                              0x0301
#define WM_PASTE                             0x0302
#define WM_CLEAR                             0x0303
#define WM_UNDO                              0x0304
#define WM_RENDERFORMAT                      0x0305
#define WM_RENDERALLFORMATS                  0x0306
#define WM_DESTROYCLIPBOARD                  0x0307
#define WM_DRAWCLIPBOARD                     0x0308
#define WM_PAINTCLIPBOARD                    0x0309
#define WM_VSCROLLCLIPBOARD                  0x030A
#define WM_SIZECLIPBOARD                     0x030B
#define WM_ASKCBFORMATNAME                   0x030C
#define WM_CHANGECBCHAIN                     0x030D
#define WM_HSCROLLCLIPBOARD                  0x030E
#define WM_QUERYNEWPALETTE                   0x030F
#define WM_PALETTEISCHANGING                 0x0310
#define WM_PALETTECHANGED                    0x0311
#define WM_HOTKEY                            0x0312

#if(WINVER >= WIN32_NT4)
    #define WM_PRINT                         0x0317
    #define WM_PRINTCLIENT                   0x0318
#endif

#if(_WIN32_WINNT >= WIN32_2000)
    #define WM_APPCOMMAND                    0x0319
#endif

#if(_WIN32_WINNT >= WIN32_WINXP)
    #define WM_THEMECHANGED                  0x031A
#endif

#if(_WIN32_WINNT >= WIN32_WINXP)
    #define WM_CLIPBOARDUPDATE               0x031D
#endif

#if(_WIN32_WINNT >= 0x0600)
    #define WM_DWMCOMPOSITIONCHANGED         0x031E
    #define WM_DWMNCRENDERINGCHANGED         0x031F
    #define WM_DWMCOLORIZATIONCOLORCHANGED   0x0320
    #define WM_DWMWINDOWMAXIMIZEDCHANGE      0x0321
#endif

#if(_WIN32_WINNT >= WIN32_WIN7)
    #define WM_DWMSENDICONICTHUMBNAIL         0x0323
    #define WM_DWMSENDICONICLIVEPREVIEWBITMAP 0x0326
#endif

#if(WINVER >= WIN32_VISTA)
    #define WM_GETTITLEBARINFOEX             0x033F
#endif

#if(WINVER >= WIN32_NT4)
    #define WM_HANDHELDFIRST                 0x0358
    #define WM_HANDHELDLAST                  0x035F
    #define WM_AFXFIRST                      0x0360
    #define WM_AFXLAST                       0x037F
#endif

#define WM_PENWINFIRST                       0x0380
#define WM_PENWINLAST                        0x038F

#if(WINVER >= WIN32_NT4)
    #define WM_APP                           0x8000
#endif

#if(WINVER >= WIN32_2000)
    #if !defined(_WIN32_WCE)
        #define WM_GETOBJECT                 0x003D
    #endif
#endif

#define WM_COMPACTING                        0x0041
#define WM_COMMNOTIFY                        0x0044                             // WARN: Unsupported
#define WM_WINDOWPOSCHANGING                 0x0046
#define WM_WINDOWPOSCHANGED                  0x0047
#define WM_NCUAHDRAWCAPTION                  0xAE
#define WM_NCUAHDRAWFRAME                    0xAF

#define DM_ORIENTATION                       0x00000001L
#define DM_PAPERSIZE                         0x00000002L
#define DM_PAPERLENGTH                       0x00000004L
#define DM_PAPERWIDTH                        0x00000008L
#define DM_SCALE                             0x00000010L

#if(WINVER >= WIN32_2000)
    #define DM_POSITION                      0x00000020L
    #define DM_NUP                           0x00000040L
#endif

#if(WINVER >= WIN32_WINXP)
    #define DM_DISPLAYORIENTATION            0x00000080L
#endif

#define DM_COPIES                            0x00000100L
#define DM_DEFAULTSOURCE                     0x00000200L
#define DM_PRINTQUALITY                      0x00000400L
#define DM_COLOR                             0x00000800L
#define DM_DUPLEX                            0x00001000L
#define DM_YRESOLUTION                       0x00002000L
#define DM_TTOPTION                          0x00004000L
#define DM_COLLATE                           0x00008000L
#define DM_FORMNAME                          0x00010000L
#define DM_LOGPIXELS                         0x00020000L
#define DM_BITSPERPEL                        0x00040000L
#define DM_PELSWIDTH                         0x00080000L
#define DM_PELSHEIGHT                        0x00100000L
#define DM_DISPLAYFLAGS                      0x00200000L
#define DM_DISPLAYFREQUENCY                  0x00400000L

#if(WINVER >= WIN32_NT4)
    #define DM_ICMMETHOD                     0x00800000L
    #define DM_ICMINTENT                     0x01000000L
    #define DM_MEDIATYPE                     0x02000000L
    #define DM_DITHERTYPE                    0x04000000L
    #define DM_PANNINGWIDTH                  0x08000000L
    #define DM_PANNINGHEIGHT                 0x10000000L
#endif

#if(WINVER >= WIN32_WINXP)
    #define DM_DISPLAYFIXEDOUTPUT            0x20000000L
#endif

#define WVR_ALIGNTOP                         0x0010
#define WVR_ALIGNRIGHT                       0x0080
#define WVR_ALIGNLEFT                        0x0020
#define WVR_ALIGNBOTTOM                      0x0040
#define WVR_HREDRAW                          0x0100
#define WVR_VREDRAW                          0x0200
#define WVR_REDRAW                           0x0300
#define WVR_VALIDRECTS                       0x0400

#define WS_OVERLAPPED                        0x00000000L
#define WS_POPUP                             0x80000000L
#define WS_CHILD                             0x40000000L
#define WS_MINIMIZE                          0x20000000L
#define WS_VISIBLE                           0x10000000L
#define WS_DISABLED                          0x08000000L
#define WS_CLIPSIBLINGS                      0x04000000L
#define WS_CLIPCHILDREN                      0x02000000L
#define WS_MAXIMIZE                          0x01000000L
#define WS_CAPTION                           0x00C00000L
#define WS_BORDER                            0x00800000L
#define WS_DLGFRAME                          0x00400000L
#define WS_VSCROLL                           0x00200000L
#define WS_HSCROLL                           0x00100000L
#define WS_SYSMENU                           0x00080000L
#define WS_THICKFRAME                        0x00040000L
#define WS_GROUP                             0x00020000L
#define WS_TABSTOP                           0x00010000L
#define WS_MINIMIZEBOX                       0x00020000L
#define WS_MAXIMIZEBOX                       0x00010000L
#define WS_TILED                             WS_OVERLAPPED
#define WS_ICONIC                            WS_MINIMIZE
#define WS_SIZEBOX                           WS_THICKFRAME
#define WS_TILEDWINDOW                       WS_OVERLAPPEDWINDOW
#define WS_OVERLAPPEDWINDOW                  (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX)

#define WS_EX_APPWINDOW                      0x00040000L
#define WS_EX_NOREDIRECTIONBITMAP            0x00200000L

#define GWL_WNDPROC                          (-4)
#define GWL_HINSTANCE                        (-6)
#define GWL_HWNDPARENT                       (-8)
#define GWL_STYLE                            (-16)
#define GWL_EXSTYLE                          (-20)
#define GWL_USERDATA                         (-21)
#define GWL_ID                               (-12)

#define SM_CXFRAME                           32
#define SM_CYFRAME                           33
#define SM_CXPADDEDBORDER                    92

#define SW_HIDE                               0
#define SW_SHOWNORMAL                         1
#define SW_NORMAL                             1
#define SW_SHOWMINIMIZED                      2
#define SW_SHOWMAXIMIZED                      3
#define SW_MAXIMIZE                           3
#define SW_SHOWNOACTIVATE                     4
#define SW_SHOW                               5
#define SW_MINIMIZE                           6
#define SW_SHOWMINNOACTIVE                    7
#define SW_SHOWNA                             8
#define SW_RESTORE                            9
#define SW_SHOWDEFAULT                       10
#define SW_FORCEMINIMIZE                     11
#define SW_MAX                               11

#define HTERROR                              (-2)
#define HTTRANSPARENT                        (-1)
#define HTNOWHERE                             0
#define HTCLIENT                              1
#define HTCAPTION                             2
#define HTSYSMENU                             3
#define HTGROWBOX                             4
#define HTSIZE                               HTGROWBOX
#define HTMENU                                5
#define HTHSCROLL                             6
#define HTVSCROLL                             7
#define HTMINBUTTON                           8
#define HTMAXBUTTON                           9
#define HTLEFT                               10
#define HTRIGHT                              11
#define HTTOP                                12
#define HTTOPLEFT                            13
#define HTTOPRIGHT                           14
#define HTBOTTOM                             15
#define HTBOTTOMLEFT                         16
#define HTBOTTOMRIGHT                        17
#define HTBORDER                             18
#define HTREDUCE                             HTMINBUTTON
#define HTZOOM                               HTMAXBUTTON
#define HTSIZEFIRST                          HTLEFT
#define HTSIZELAST                           HTBOTTOMRIGHT

#if(WINVER >= WIN32_NT4)
    #define HTOBJECT                         19
    #define HTCLOSE                          20
    #define HTHELP                           21
#endif

#if defined(UNICODE) || defined(_UNICODE)
    #define IDC_ARROW                        ((LPCWSTR) ((ULONG_PTR) ((WORD) 32512)))
    #define IDC_IBEAM                        ((LPCWSTR) ((ULONG_PTR) ((WORD) 32513)))
    #define IDC_SIZEWE                       ((LPCWSTR) ((ULONG_PTR) ((WORD) 32644)))
    #define IDC_SIZENS                       ((LPCWSTR) ((ULONG_PTR) ((WORD) 32645)))
    #define IDC_SIZENWSE                     ((LPCWSTR) ((ULONG_PTR) ((WORD) 32642)))
    #define IDC_SIZENESW                     ((LPCWSTR) ((ULONG_PTR) ((WORD) 32643)))
    #define IDC_SIZEALL                      ((LPCWSTR) ((ULONG_PTR) ((WORD) 32646)))
    #define IDC_HAND                         ((LPCWSTR) ((ULONG_PTR) ((WORD) 32649)))
    #define IDC_NO                           ((LPCWSTR) ((ULONG_PTR) ((WORD) 32648)))
#else
    #define IDC_ARROW                        ((LPCSTR) 32512)
    #define IDC_IBEAM                        ((LPCSTR) 32513)
    #define IDC_SIZEWE                       ((LPCSTR) 32644)
    #define IDC_SIZENS                       ((LPCSTR) 32645)
    #define IDC_SIZENWSE                     ((LPCSTR) 32642)
    #define IDC_SIZENESW                     ((LPCSTR) 32643)
    #define IDC_SIZEALL                      ((LPCSTR) 32646)
    #define IDC_HAND                         ((LPCSTR) 32649)
    #define IDC_NO                           ((LPCSTR) 32648)
#endif


#define CS_VREDRAW                           0x0001
#define CS_HREDRAW                           0x0002
#define CS_DBLCLKS                           0x0008
#define CS_OWNDC                             0x0020
#define CS_CLASSDC                           0x0040
#define CS_PARENTDC                          0x0080
#define CS_NOCLOSE                           0x0200
#define CS_SAVEBITS                          0x0800
#define CS_BYTEALIGNCLIENT                   0x1000
#define CS_BYTEALIGNWINDOW                   0x2000
#define CS_GLOBALCLASS                       0x4000
#define CS_IME                               0x00010000

#if(_WIN32_WINNT >= WIN32_WINXP)
    #define CS_DROPSHADOW                    0x00020000
#endif

#define CCHDEVICENAME                        32
#define CCHFORMNAME                          32

#define CW_USEDEFAULT                        ((int) 0x80000000)

#define ENUM_CURRENT_SETTINGS                ((DWORD) -1)
#define ENUM_REGISTRY_SETTINGS               ((DWORD) -2)

#define GHND                                 0x0042
#define GMEM_FIXED                           0x0000
#define GMEM_MOVEABLE                        0x0002
#define GMEM_ZEROINIT                        0x0040
#define GPTR                                 0x0040

#define CF_TEXT                              1
#define CF_BITMAP                            2
#define CF_METAFILEPICT                      3
#define CF_SYLK                              4
#define CF_DIF                               5
#define CF_TIFF                              6
#define CF_OEMTEXT                           7
#define CF_DIB                               8
#define CF_PALETTE                           9
#define CF_PENDATA                           10
#define CF_RIFF                              11
#define CF_WAVE                              12
#define CF_UNICODETEXT                       13
#define CF_ENHMETAFILE                       14
#define CF_HDROP                             15
#define CF_LOCALE                            16
#define CF_DIBV5                             17
#define CF_OWNERDISPLAY                      0x0080
#define CF_DSPTEXT                           0x0081
#define CF_DSPBITMAP                         0x0082
#define CF_DSPMETAFILEPICT                   0x0083
#define CF_DSPENHMETAFILE                    0x008E
#define CF_PRIVATEFIRST                      0x0200
#define CF_PRIVATELAST                       0x02FF
#define CF_GDIOBJFIRST                       0x0300
#define CF_GDIOBJLAST                        0x03FF

#define MONITOR_DEFAULTTONULL                0x00000000
#define MONITOR_DEFAULTTOPRIMARY             0x00000001
#define MONITOR_DEFAULTTONEAREST             0x00000002

#define HWND_BOTTOM                          ((HWND)  1)
#define HWND_NOTOPMOST                       ((HWND) -2)
#define HWND_TOP                             ((HWND)  0)
#define HWND_TOPMOST                         ((HWND) -1)

#define SWP_NOSIZE                           0x0001
#define SWP_NOMOVE                           0x0002
#define SWP_NOZORDER                         0x0004
#define SWP_NOREDRAW                         0x0008
#define SWP_NOACTIVATE                       0x0010
#define SWP_FRAMECHANGED                     0x0020
#define SWP_SHOWWINDOW                       0x0040
#define SWP_HIDEWINDOW                       0x0080
#define SWP_NOCOPYBITS                       0x0100
#define SWP_NOOWNERZORDER                    0x0200
#define SWP_NOSENDCHANGING                   0x0400
#define SWP_DRAWFRAME                        SWP_FRAMECHANGED
#define SWP_NOREPOSITION                     SWP_NOOWNERZORDER

#define PM_NOREMOVE                          0x0000
#define PM_REMOVE                            0x0001
#define PM_NOYIELD                           0x0002

#define MB_OK                                0x00000000L
#define MB_ICONERROR                         0x00000010L
#define MB_SYSTEMMODAL                       0x000010000L

#define CSIDL_DESKTOP                        0x0000
#define CSIDL_INTERNET                       0x0001
#define CSIDL_PROGRAMS                       0x0002
#define CSIDL_CONTROLS                       0x0003
#define CSIDL_PRINTERS                       0x0004
#define CSIDL_PERSONAL                       0x0005
#define CSIDL_FAVORITES                      0x0006
#define CSIDL_STARTUP                        0x0007
#define CSIDL_RECENT                         0x0008
#define CSIDL_SENDTO                         0x0009
#define CSIDL_BITBUCKET                      0x000A
#define CSIDL_STARTMENU                      0x000B
#define CSIDL_MYDOCUMENTS                    CSIDL_PERSONAL
#define CSIDL_MYMUSIC                        0x000D
#define CSIDL_MYVIDEO                        0x000E
#define CSIDL_DESKTOPDIRECTORY               0x0010
#define CSIDL_DRIVES                         0x0011
#define CSIDL_NETWORK                        0x0012
#define CSIDL_NETHOOD                        0x0013
#define CSIDL_FONTS                          0x0014
#define CSIDL_TEMPLATES                      0x0015
#define CSIDL_COMMON_STARTMENU               0x0016
#define CSIDL_COMMON_PROGRAMS                0X0017
#define CSIDL_COMMON_STARTUP                 0x0018
#define CSIDL_COMMON_DESKTOPDIRECTORY        0x0019
#define CSIDL_APPDATA                        0x001a
#define CSIDL_PRINTHOOD                      0x001b

#ifndef CSIDL_LOCAL_APPDATA
    #define CSIDL_LOCAL_APPDATA              0x001C
#endif

#define CSIDL_ALTSTARTUP                     0x001D
#define CSIDL_COMMON_ALTSTARTUP              0x001E
#define CSIDL_COMMON_FAVORITES               0x001F

#ifndef _SHFOLDER_H_
    #define CSIDL_INTERNET_CACHE             0x0020
    #define CSIDL_COOKIES                    0x0021
    #define CSIDL_HISTORY                    0x0022
    #define CSIDL_COMMON_APPDATA             0x0023
    #define CSIDL_WINDOWS                    0x0024
    #define CSIDL_SYSTEM                     0x0025
    #define CSIDL_PROGRAM_FILES              0x0026
    #define CSIDL_MYPICTURES                 0x0027
#endif

#define CSIDL_PROFILE                        0x0028
#define CSIDL_SYSTEMX86                      0x0029
#define CSIDL_PROGRAM_FILESX86               0x002A

#ifndef _SHFOLDER_H_
    #define CSIDL_PROGRAM_FILES_COMMON       0x002B
#endif

#define CSIDL_PROGRAM_FILES_COMMONX86        0x002C
#define CSIDL_COMMON_TEMPLATES               0x002D

#ifndef _SHFOLDER_H_
    #define CSIDL_COMMON_DOCUMENTS           0x002E
    #define CSIDL_COMMON_ADMINTOOLS          0x002F
    #define CSIDL_ADMINTOOLS                 0x0030
#endif

#define CSIDL_CONNECTIONS                    0x0031
#define CSIDL_COMMON_MUSIC                   0x0035
#define CSIDL_COMMON_PICTURES                0x0036
#define CSIDL_COMMON_VIDEO                   0x0037
#define CSIDL_RESOURCES                      0x0038

#ifndef _SHFOLDER_H_
    #define CSIDL_RESOURCES_LOCALIZED        0x0039
#endif

#define CSIDL_COMMON_OEM_LINKS               0x003A
#define CSIDL_CDBURN_AREA                    0x003B
// ??????                                    0x003C
#define CSIDL_COMPUTERSNEARME                0x003D

#ifndef _SHFOLDER_H_
    #define CSIDL_FLAG_CREATE                0x8000
#endif

#define CSIDL_FLAG_DONT_VERIFY               0x4000
#define CSIDL_FLAG_DONT_UNEXPAND             0x2000

#if (NTDDI_VERSION >= NTDDI_WINXP)
    #define CSIDL_FLAG_NO_ALIAS              0x1000
    #define CSIDL_FLAG_PER_USER_INIT         0x0800
#endif

#define CSIDL_FLAG_MASK                      0xFF00

#define TOKEN_ADJUST_PRIVILEGES              0x0020
#define TOKEN_QUERY                          0x0008

#define ANYSIZE_ARRAY                        1

#define WSADESCRIPTION_LEN                   256
#define WSASYS_STATUS_LEN                    128

#define IOC_VENDOR                           0x18000000
#define IOC_IN                               0x80000000
// #define IOC_INOUT                            0xC0000000
#define _WSAIOW(X, Y)                         (IOC_IN | IOC_INOUT | (X) | (Y))

#define SIO_GET_MULTIPLE_EXTENSION_FUNCTION_POINTER _WSAIOW(IOC_VENDOR, 6)

#define SYMOPT_CASE_INSENSITIVE              0x1
#define SYMOPT_UNDNAME                       0x2
#define SYMOPT_DEFERRED_LOADS                0x4
#define SYMOPT_NO_CPP                        0x8
#define SYMOPT_LOAD_LINES                    0x10
#define SYMOPT_OMAP_FIND_NEAREST             0x20
#define SYMOPT_LOAD_ANYTHING                 0x40
#define SYMOPT_IGNORE_CVREC                  0x80
#define SYMOPT_NO_UNQUALIFIED_LOADS          0x100
#define SYMOPT_FAIL_CRITICAL_ERRORS          0x200
#define SYMOPT_EXACT_SYMBOLS                 0x400
#define SYMOPT_ALLOW_ABSOLUTE_SYMBOLS        0x800
#define SYMOPT_IGNORE_NT_SYMPATH             0x1000
#define SYMOPT_INCLUDE_32BIT_MODULES         0x2000
#define SYMOPT_PUBLICS_ONLY                  0x4000
#define SYMOPT_NO_PUBLICS                    0x8000
#define SYMOPT_AUTO_PUBLICS                  0x10000
#define SYMOPT_NO_IMAGE_SEARCH               0x20000
#define SYMOPT_SECURE                        0x40000
#define SYMOPT_NO_PROMPTS                    0x80000
#define SYMOPT_DEBUG                         0x80000000

#define MAX_SYM_NAME                         2000

#define IMAGE_FILE_MACHINE_AMD64             0x8664
#define IMAGE_FILE_MACHINE_ARM64             0xAA64

#define TD_ERROR_ICON                        MAKEINTRESOURCEW(-2)

#define TDN_HYPERLINK_CLICKED                3

#define OBJ_BITMAP                           7
#define DC_PEN                               19
#define DC_BRUSH                             18

#define RGB(R, G, B)                         ((COLORREF) (((BYTE) (R) | ((WORD)((BYTE) (G)) << 8)) | (((DWORD)(BYTE) (B)) << 16)))

#define HKEY_CURRENT_USER                    ((HKEY) (ULONG_PTR) ((LONG) 0x80000001))

#define SYNCHRONIZE                          (0x00100000L)
#define DELETE                               (0x00010000L)
#define READ_CONTROL                         (0x00020000L)
#define WRITE_DAC                            (0x00040000L)
#define WRITE_OWNER                          (0x00080000L)
#define KEY_QUERY_VALUE                      (0x0001)
#define KEY_SET_VALUE                        (0x0002)
#define KEY_CREATE_SUB_KEY                   (0x0004)
#define KEY_ENUMERATE_SUB_KEYS               (0x0008)
#define KEY_NOTIFY                           (0x0010)
#define KEY_CREATE_LINK                      (0x0020)
#define KEY_WOW64_64KEY                      (0x0100)
#define KEY_WOW64_32KEY                      (0x0200)
#define KEY_READ                             ((STANDARD_RIGHTS_READ | KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS | KEY_NOTIFY) & (~SYNCHRONIZE))
#define KEY_WRITE                            ((STANDARD_RIGHTS_WRITE | KEY_SET_VALUE | KEY_CREATE_SUB_KEY) & (~SYNCHRONIZE))
#define KEY_ALL_ACCESS                       ((STANDARD_RIGHTS_ALL | KEY_QUERY_VALUE | KEY_SET_VALUE | KEY_CREATE_SUB_KEY | KEY_ENUMERATE_SUB_KEYS | KEY_NOTIFY | KEY_CREATE_LINK) & (~SYNCHRONIZE))

#define X509_ASN_ENCODING                    0x00000001
#define PKCS_7_ASN_ENCODING                  0x00010000
#define CERT_STORE_PROV_MEMORY               ((LPCSTR) 2)
#define CERT_STORE_PROV_PKCS7                ((LPCSTR) 5)
#define CERT_STORE_CREATE_NEW_FLAG           0x00002000
#define CERT_STORE_ADD_ALWAYS                4
#define CERT_FIND_ANY                        0
#define CRYPT_EXPORTABLE                     0x00000001
#define CERT_NAME_ATTR_TYPE                  3
#define CERT_NAME_ISSUER_FLAG                0x1
#define szOID_COMMON_NAME                    "2.5.4.3"
#define SEC_E_OK                             ((SECURITY_STATUS) 0x00000000)
#define SEC_I_CONTINUE_NEEDED                ((SECURITY_STATUS) 0x00090312)
#define SEC_E_INCOMPLETE_MESSAGE             ((SECURITY_STATUS) 0x80090318)
#define SEC_I_CONTEXT_EXPIRED                ((SECURITY_STATUS) 0x00090317)
#define SECPKG_CRED_INBOUND                  0x00000001
#define SECPKG_CRED_OUTBOUND                 0x00000002
#define ISC_REQ_SEQUENCE_DETECT              0x00000008
#define ISC_REQ_REPLAY_DETECT                0x00000004
#define ISC_REQ_CONFIDENTIALITY              0x00000010
#define ISC_REQ_STREAM                       0x00008000
#define ISC_REQ_MUTUAL_AUTH                  0x00000002
#define ASC_REQ_SEQUENCE_DETECT              0x00000008
#define ASC_REQ_REPLAY_DETECT                0x00000004
#define ASC_REQ_CONFIDENTIALITY              0x00000010
#define ASC_REQ_STREAM                       0x00010000
#define ASC_REQ_MUTUAL_AUTH                  0x00000002
#define SECPKG_ATTR_STREAM_SIZES             4
#define SECPKG_ATTR_REMOTE_CERT_CONTEXT      83
#define SECBUFFER_EMPTY                      0
#define SECBUFFER_DATA                       1
#define SECBUFFER_TOKEN                      2
#define SECBUFFER_STREAM_HEADER              7
#define SECBUFFER_STREAM_TRAILER             6
#define SECBUFFER_VERSION                    0
#define UNISP_NAME_W                         L"Microsoft Unified Security Protocol Provider"
#define SCHANNEL_CRED_VERSION                4
#define SP_PROT_TLS1_3                       0x00002000
#define SP_PROT_TLS1_3_CLIENT                0x00002000
#define SP_PROT_TLS1_3_SERVER                0x00001000
#define SCH_CRED_NO_DEFAULT_CREDS            0x00000010
#define SCH_CRED_MANUAL_CRED_VALIDATION      0x00000008
#define SCH_CRED_NO_SERVERNAME_CHECK         0x00000004
#define SCHANNEL_SHUTDOWN                    1
#define BCRYPT_SHA256_ALGORITHM              L"SHA256"

#define WINHTTP_CALLBACK_STATUS_REQUEST_ERROR       2097152U
#define WINHTTP_CALLBACK_STATUS_REQUEST_SENT        32U
#define WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE   131072U
#define WINHTTP_CALLBACK_STATUS_DATA_AVAILABLE      262144U
#define WINHTTP_CALLBACK_STATUS_READ_COMPLETE       524288U
#define WINHTTP_ACCESS_TYPE_DEFAULT_PROXY           0
#define WINHTTP_ACCESS_TYPE_NO_PROXY                1
#define WINHTTP_ACCESS_TYPE_NAMED_PROXY             3
#define WINHTTP_NO_PROXY_NAME                       NULL
#define WINHTTP_NO_PROXY_BYPASS                     NULL
#define WINHTTP_NO_REFERER                          NULL
#define WINHTTP_DEFAULT_ACCEPT_TYPES                NULL
#define WINHTTP_NO_ADDITIONAL_HEADERS               NULL
#define WINHTTP_NO_REQUEST_DATA                     NULL
#define WINHTTP_HEADER_NAME_BY_INDEX                NULL
#define WINHTTP_NO_OUTPUT_BUFFER                    NULL
#define WINHTTP_NO_HEADER_INDEX                     NULL
#define WINHTTP_FLAG_ASYNC                          0x10000000
#define WINHTTP_FLAG_ESCAPE_PERCENT                 0x00000004
#define WINHTTP_FLAG_NULL_CODEPAGE                  0x00000008
#define WINHTTP_FLAG_ESCAPE_DISABLE                 0x00000040
#define WINHTTP_FLAG_ESCAPE_DISABLE_QUERY           0x00000080
#define WINHTTP_FLAG_BYPASS_PROXY_CACHE             0x00000100
#define WINHTTP_FLAG_REFRESH                        WINHTTP_FLAG_BYPASS_PROXY_CACHE
#define WINHTTP_FLAG_SECURE                         0x00800000
#define WINHTTP_CALLBACK_FLAG_ALL_NOTIFICATIONS     U32_MAX
#define WINHTTP_QUERY_STATUS_CODE                   19
#define WINHTTP_QUERY_FLAG_NUMBER                   0x20000000
#define WINHTTP_QUERY_MIME_VERSION                      0
#define WINHTTP_QUERY_CONTENT_TYPE                      1
#define WINHTTP_QUERY_CONTENT_TRANSFER_ENCODING         2
#define WINHTTP_QUERY_CONTENT_ID                        3
#define WINHTTP_QUERY_CONTENT_DESCRIPTION               4
#define WINHTTP_QUERY_CONTENT_LENGTH                    5
#define WINHTTP_QUERY_CONTENT_LANGUAGE                  6
#define WINHTTP_QUERY_ALLOW                             7
#define WINHTTP_QUERY_PUBLIC                            8
#define WINHTTP_QUERY_DATE                              9
#define WINHTTP_QUERY_EXPIRES                          10
#define WINHTTP_QUERY_LAST_MODIFIED                    11
#define WINHTTP_QUERY_MESSAGE_ID                       12
#define WINHTTP_QUERY_URI                              13
#define WINHTTP_QUERY_DERIVED_FROM                     14
#define WINHTTP_QUERY_COST                             15
#define WINHTTP_QUERY_LINK                             16
#define WINHTTP_QUERY_PRAGMA                           17
#define WINHTTP_QUERY_VERSION                          18
#define WINHTTP_QUERY_STATUS_CODE                      19
#define WINHTTP_QUERY_STATUS_TEXT                      20
#define WINHTTP_QUERY_RAW_HEADERS                      21
#define WINHTTP_QUERY_RAW_HEADERS_CRLF                 22
#define WINHTTP_QUERY_CONNECTION                       23
#define WINHTTP_QUERY_ACCEPT                           24
#define WINHTTP_QUERY_ACCEPT_CHARSET                   25
#define WINHTTP_QUERY_ACCEPT_ENCODING                  26
#define WINHTTP_QUERY_ACCEPT_LANGUAGE                  27
#define WINHTTP_QUERY_AUTHORIZATION                    28
#define WINHTTP_QUERY_CONTENT_ENCODING                 29
#define WINHTTP_QUERY_FORWARDED                        30
#define WINHTTP_QUERY_FROM                             31
#define WINHTTP_QUERY_IF_MODIFIED_SINCE                32
#define WINHTTP_QUERY_LOCATION                         33
#define WINHTTP_QUERY_ORIG_URI                         34
#define WINHTTP_QUERY_REFERER                          35
#define WINHTTP_QUERY_RETRY_AFTER                      36
#define WINHTTP_QUERY_SERVER                           37
#define WINHTTP_QUERY_TITLE                            38
#define WINHTTP_QUERY_USER_AGENT                       39
#define WINHTTP_QUERY_WWW_AUTHENTICATE                 40
#define WINHTTP_QUERY_PROXY_AUTHENTICATE               41
#define WINHTTP_QUERY_ACCEPT_RANGES                    42
#define WINHTTP_QUERY_SET_COOKIE                       43
#define WINHTTP_QUERY_COOKIE                           44
#define WINHTTP_QUERY_REQUEST_METHOD                   45
#define WINHTTP_QUERY_REFRESH                          46
#define WINHTTP_QUERY_CONTENT_DISPOSITION              47
#define WINHTTP_QUERY_AGE                              48
#define WINHTTP_QUERY_CACHE_CONTROL                    49
#define WINHTTP_QUERY_CONTENT_BASE                     50
#define WINHTTP_QUERY_CONTENT_LOCATION                 51
#define WINHTTP_QUERY_CONTENT_MD5                      52
#define WINHTTP_QUERY_CONTENT_RANGE                    53
#define WINHTTP_QUERY_ETAG                             54
#define WINHTTP_QUERY_HOST                             55
#define WINHTTP_QUERY_IF_MATCH                         56
#define WINHTTP_QUERY_IF_NONE_MATCH                    57
#define WINHTTP_QUERY_IF_RANGE                         58
#define WINHTTP_QUERY_IF_UNMODIFIED_SINCE              59
#define WINHTTP_QUERY_MAX_FORWARDS                     60
#define WINHTTP_QUERY_PROXY_AUTHORIZATION              61
#define WINHTTP_QUERY_RANGE                            62
#define WINHTTP_QUERY_TRANSFER_ENCODING                63
#define WINHTTP_QUERY_UPGRADE                          64
#define WINHTTP_QUERY_VARY                             65
#define WINHTTP_QUERY_VIA                              66
#define WINHTTP_QUERY_WARNING                          67
#define WINHTTP_QUERY_EXPECT                           68
#define WINHTTP_QUERY_PROXY_CONNECTION                 69
#define WINHTTP_QUERY_UNLESS_MODIFIED_SINCE            70
#define WINHTTP_QUERY_PROXY_SUPPORT                    75
#define WINHTTP_QUERY_AUTHENTICATION_INFO              76
#define WINHTTP_QUERY_PASSPORT_URLS                    77
#define WINHTTP_QUERY_PASSPORT_CONFIG                  78
#define WINHTTP_QUERY_MAX                              78
#define WINHTTP_QUERY_CUSTOM                        65535
#define WINHTTP_QUERY_FLAG_REQUEST_HEADERS          0x80000000
#define WINHTTP_QUERY_FLAG_SYSTEMTIME               0x40000000
#define WINHTTP_QUERY_FLAG_NUMBER64                 0x08000000

#define INTERNET_INVALID_PORT_NUMBER                   0
#define INTERNET_DEFAULT_FTP_PORT                     21
#define INTERNET_DEFAULT_SSH_PORT                     22
#define INTERNET_DEFAULT_GOPHER_PORT                  70
#define INTERNET_DEFAULT_HTTP_PORT                    80
#define INTERNET_DEFAULT_HTTPS_PORT                  443
#define INTERNET_DEFAULT_SOCKS_PORT                 1080

// @types______________________________________________________________________
typedef void           VOID;
typedef void*          HANDLE, *PHANDLE;
typedef void*          HWND;
typedef unsigned long* PDWORD;
typedef void*          HMONITOR;
typedef void*          HLOCAL;
typedef u32            HRESULT;
typedef void*          HDC;
typedef HANDLE         HINSTANCE;
typedef HANDLE         HICON;
typedef HANDLE         HBRUSH;
typedef HANDLE         HCURSOR;
typedef HANDLE         HMODULE;
typedef HANDLE         HMENU;
typedef HANDLE         HBITMAP;
typedef HANDLE         HGDIOBJ;
typedef HANDLE         HFONT;
typedef HANDLE         HPEN;
typedef HANDLE         HGLOBAL;
typedef HANDLE         HRGN;
typedef HANDLE         HDROP;
typedef unsigned long  ULONG;
typedef ULONG*         PULONG;
typedef unsigned short USHORT;
typedef short          SHORT;
typedef USHORT*        PUSHORT;
typedef unsigned char  UCHAR;
typedef UCHAR*         PUCHAR;
typedef unsigned long  DWORD;
typedef int            BOOL;
typedef unsigned char  BYTE;
typedef unsigned short WORD;
typedef float          FLOAT;
typedef FLOAT*         PFLOAT;
typedef BOOL*          PBOOL;
typedef BOOL*          LPBOOL;
typedef BYTE*          PBYTE;
typedef BYTE*          LPBYTE;
typedef int*           PINT;
typedef int*           LPINT;
typedef WORD*          PWORD;
typedef WORD*          LPWORD;
typedef long*          LPLONG;
typedef DWORD*         PDWORD;
typedef DWORD*         LPDWORD;
typedef void*          LPVOID;
typedef const void*    LPCVOID;
typedef void*          PVOID;
typedef u64            DWORDLONG;
typedef ULONG          SFGAOF;
typedef HANDLE         WSAEVENT;
typedef HWND           HGLRC;
typedef LPVOID         HINTERNET;
typedef SHORT          INTERNET_PORT;

#if !defined(_M_IX86)
 typedef i64 LONGLONG; 
#else
 typedef double LONGLONG;
#endif

#if !defined(_M_IX86)
 typedef u64 ULONGLONG;
#else
 typedef double ULONGLONG;
#endif

// TODO: Need to clean these up, merge similar types and remove unneeded
typedef LONGLONG                USN;
typedef WORD                    ATOM;
typedef i64                     INT_PTR, *PINT_PTR;
typedef u64                     UINT_PTR, *PUINT_PTR;
typedef i64                     LONG_PTR, *PLONG_PTR;
typedef u64                     ULONG_PTR;
typedef UINT_PTR                WPARAM;
typedef LONG_PTR                LPARAM;
typedef LONG_PTR                LRESULT;
typedef int                     INT;
typedef unsigned int            UINT;
typedef unsigned int            *PUINT;
typedef signed char             INT8, *PINT8;
typedef signed short            INT16, *PINT16;
typedef signed int              INT32, *PINT32;
typedef i64                     INT64, *PINT64;
typedef unsigned char           UINT8, *PUINT8;
typedef unsigned short          UINT16, *PUINT16;
typedef unsigned int            UINT32, *PUINT32;
typedef u64                     UINT64, *PUINT64;
typedef signed int              LONG32, *PLONG32;
typedef unsigned int            ULONG32, *PULONG32;
typedef unsigned int            DWORD32, *PDWORD32;
typedef long                    LONG;
typedef u64                     ULONG64, *PULONG64;
typedef u64                     DWORD64, *PDWORD64;
typedef i64                     LONG64;
typedef char                    CHAR;
typedef CHAR                    *PCHAR, *LPCH, *PCH;
typedef const CHAR              *LPCCH, *PCCH;
typedef ULONG_PTR               DWORD_PTR;
typedef CHAR                    *NPSTR, *LPSTR, *PSTR;
typedef PSTR                    *PZPSTR;
typedef const PSTR              *PCZPSTR;
typedef const CHAR              *LPCSTR, *PCSTR;
typedef PCSTR                   *PZPCSTR;
typedef const PCSTR             *PCZPCSTR;
typedef CHAR                    *PZZSTR;
typedef const CHAR              *PCZZSTR;
typedef CHAR                    *PNZCH;
typedef const CHAR              *PCNZCH;
typedef wchar_t                 WCHAR;
typedef WCHAR                   *NWPSTR, *LPWSTR, *PWSTR;
typedef PWSTR                   *PZPWSTR;
typedef const PWSTR             *PCZPWSTR;
typedef __unaligned WCHAR       *LPUWSTR, *PUWSTR;
typedef const WCHAR             *LPCWSTR, *PCWSTR;
typedef PCWSTR                  *PZPCWSTR;
typedef const PCWSTR            *PCZPCWSTR;
typedef const __unaligned WCHAR *LPCUWSTR, *PCUWSTR;
typedef WCHAR                   *PZZWSTR;
typedef const WCHAR             *PCZZWSTR;
typedef __unaligned WCHAR       *PUZZWSTR;
typedef const __unaligned WCHAR *PCUZZWSTR;
typedef WCHAR                   *PNZWCH;
typedef const WCHAR             *PCNZWCH;
typedef __unaligned WCHAR       *PUNZWCH;
typedef const __unaligned WCHAR *PCUNZWCH;
typedef WCHAR                   *LPWCH;
typedef const WCHAR             *LPCWCH;
typedef long long               LONGLONG;
typedef INT_PTR                 (WINAPI* FARPROC)();
typedef sz                      SIZE_T;
typedef DWORD                   COLORREF, *LPCOLORREF;
typedef WORD                    LANGID;
typedef HANDLE                  HKEY, *PHKEY, *LPHKEY;
typedef LONG                    LSTATUS;
typedef DWORD                   ACCESS_MASK;
typedef ACCESS_MASK             REGSAM;
typedef LONG                    FXPT2DOT30;
typedef LONG                    SECURITY_STATUS;
typedef ULONG_PTR               HCRYPTPROV;
typedef UINT                    MMRESULT;

#if defined(_WIN64)
    typedef UINT_PTR            SOCKET;
#else
    typedef UINT                SOCKET;
#endif

#if _WINVER >= WIN32_2000
    typedef HANDLE HMONITOR;
#endif

union GuID {
    struct {
        u32 Data1;
        u16 Data2;
        u16 Data3;
        u8  Data4[8];
    };

    u8 V[16];
};

typedef GuID           GUID;
typedef HANDLE         HCERTSTORE;

typedef struct _TASKDIALOG_BUTTON {
    int    nButtonID;
    PCWSTR pszButtonText;
} TASKDIALOG_BUTTON;

typedef enum _MINIDUMP_TYPE {
    MiniDumpNormal                          = 0x00000000,
    MiniDumpWithDataSegs                    = 0x00000001,
    MiniDumpWithFullMemory                  = 0x00000002,
    MiniDumpWithHandleData                  = 0x00000004,
    MiniDumpFilterMemory                    = 0x00000008,
    MiniDumpScanMemory                      = 0x00000010,
    MiniDumpWithUnloadedModules             = 0x00000020,
    MiniDumpWithIndirectlyReferencedMemory  = 0x00000040,
    MiniDumpFilterModulePaths               = 0x00000080,
    MiniDumpWithProcessThreadData           = 0x00000100,
    MiniDumpWithPrivateReadWriteMemory      = 0x00000200,
    MiniDumpWithoutOptionalData             = 0x00000400,
    MiniDumpWithFullMemoryInfo              = 0x00000800,
    MiniDumpWithThreadInfo                  = 0x00001000,
    MiniDumpWithCodeSegs                    = 0x00002000,
    MiniDumpWithoutAuxiliaryState           = 0x00004000,
    MiniDumpWithFullAuxiliaryState          = 0x00008000,
    MiniDumpWithPrivateWriteCopyMemory      = 0x00010000,
    MiniDumpIgnoreInaccessibleMemory        = 0x00020000,
    MiniDumpWithTokenInformation            = 0x00040000,
    MiniDumpWithModuleHeaders               = 0x00080000,
    MiniDumpFilterTriage                    = 0x00100000,
    MiniDumpWithAvxXStateContext            = 0x00200000,
    MiniDumpWithIptTrace                    = 0x00400000,
    MiniDumpScanInaccessiblePartialPages    = 0x00800000,
    MiniDumpFilterWriteCombinedMemory,
    MiniDumpValidTypeFlags                  = 0x01FFFFFF,
    MiniDumpNoIgnoreInaccessibleMemory,
    MiniDumpValidTypeFlagsEx
} MINIDUMP_TYPE;

typedef enum _FILE_INFO_BY_HANDLE_CLASS {
    FileBasicInfo,
    FileStandardInfo,
    FileNameInfo,
    FileRenameInfo,
    FileDispositionInfo,
    FileAllocationInfo,
    FileEndOfFileInfo,
    FileStreamInfo,
    FileCompressionInfo,
    FileAttributeTagInfo,
    FileIdBothDirectoryInfo,
    FileIdBothDirectoryRestartInfo,
    FileIoPriorityHintInfo,
    FileRemoteProtocolInfo,
    FileFullDirectoryInfo,
    FileFullDirectoryRestartInfo,
    FileStorageInfo,
    FileAlignmentInfo,
    FileIdInfo,
    FileIdExtdDirectoryInfo,
    FileIdExtdDirectoryRestartInfo,
    FileDispositionInfoEx,
    FileRenameInfoEx,
    FileCaseSensitiveInfo,
    FileNormalizedNameInfo,
    MaximumFileInfoByHandleClass
} FILE_INFO_BY_HANDLE_CLASS, *PFILE_INFO_BY_HANDLE_CLASS;

typedef enum _GET_FILEEX_INFO_LEVELS {
    GetFileExInfoStandard,
    GetFileExMaxInfoLevel
} GET_FILEEX_INFO_LEVELS;

typedef enum _FINDEX_INFO_LEVELS {
    FindExInfoStandard,
    FindExInfoBasic,
    FindExInfoMaxInfoLevel
} FINDEX_INFO_LEVELS;

typedef enum _FINDEX_SEARCH_OPS {
    FindExSearchNameMatch,
    FindExSearchLimitToDirectories,
    FindExSearchLimitToDevices,
    FindExSearchMaxSearchOp
} FINDEX_SEARCH_OPS;

typedef enum _EXCEPTION_DISPOSITION {
    ExceptionContinueExecution,
    ExceptionContinueSearch,
    ExceptionNestedException,
    ExceptionCollidedUnwind
} EXCEPTION_DISPOSITION;

typedef enum _MONITOR_DPI_TYPE {
    MDT_EFFECTIVE_DPI = 0,
    MDT_ANGULAR_DPI   = 1,
    MDT_RAW_DPI       = 2,
    MDT_DEFAULT
} MONITOR_DPI_TYPE;

typedef enum _DWMNCRENDERINGPOLICY {
    DWMNCRP_USEWINDOWSTYLE,
    DWMNCRP_DISABLED,
    DWMNCRP_ENABLED,
    DWMNCRP_LAST
} DWMNCRENDERINGPOLICY;

typedef enum {
    BI_RGB       = 0x0000,
    BI_RLE8      = 0x0001,
    BI_RLE4      = 0x0002,
    BI_BITFIELDS = 0x0003,
    BI_JPEG      = 0x0004,
    BI_PNG       = 0x0005,
    BI_CMYK      = 0x000B,
    BI_CMYKRLE8  = 0x000C,
    BI_CMYKRLE4  = 0x000D
} Compression;

typedef enum {
    DIB_RGB_COLORS  = 0x00,
    DIB_PAL_COLORS  = 0x01,
    DIB_PAL_INDICES = 0x02
} DIBColors;

typedef enum _DWMWINDOWATTRIBUTE {
    DWMWA_NCRENDERING_ENABLED,
    DWMWA_NCRENDERING_POLICY,
    DWMWA_TRANSITIONS_FORCEDISABLED,
    DWMWA_ALLOW_NCPAINT,
    DWMWA_CAPTION_BUTTON_BOUNDS,
    DWMWA_NONCLIENT_RTL_LAYOUT,
    DWMWA_FORCE_ICONIC_REPRESENTATION,
    DWMWA_FLIP3D_POLICY,
    DWMWA_EXTENDED_FRAME_BOUNDS,
    DWMWA_HAS_ICONIC_BITMAP,
    DWMWA_DISALLOW_PEEK,
    DWMWA_EXCLUDED_FROM_PEEK,
    DWMWA_CLOAK,
    DWMWA_CLOAKED,
    DWMWA_FREEZE_REPRESENTATION,
    DWMWA_PASSIVE_UPDATE_MODE,
    DWMWA_USE_HOSTBACKDROPBRUSH,
    DWMWA_USE_IMMERSIVE_DARK_MODE       = 20,
    DWMWA_WINDOW_CORNER_PREFERENCE      = 33,
    DWMWA_BORDER_COLOR,
    DWMWA_CAPTION_COLOR,
    DWMWA_TEXT_COLOR,
    DWMWA_VISIBLE_FRAME_BORDER_THICKNESS,
    DWMWA_SYSTEMBACKDROP_TYPE,
    DWMWA_LAST
} DWMWINDOWATTRIBUTE;

typedef enum _RIO_NOTIFICATION_COMPLETION_TYPE {
    RIO_EVENT_COMPLETION = 1,
    RIO_IOCP_COMPLETION = 2
} RIO_NOTIFICATION_COMPLETION_TYPE, *PRIO_NOTIFICATION_COMPLETION_TYPE;

typedef enum _ADDRESS_MODE {
    ADDRESS_MODE_16_16,
    ADDRESS_MODE_16_32,
    ADDRESS_MODE_REAL,
    ADDRESS_MODE_FLAT,
} ADDRESS_MODE;

typedef enum _SYM_TYPE {
    SymNone = 0,
    SymCoff,
    SymCv,
    SymPdb,
    SymExport,
    SymDeferred,
    SymSym,
    SymDia,
    SymVirtual
} SYM_TYPE;

typedef i32 TASKDIALOG_FLAGS;
enum _TASKDIALOG_FLAGS {
    TDF_ENABLE_HYPERLINKS               = 0x0001,
    TDF_USE_HICON_MAIN                  = 0x0002,
    TDF_USE_HICON_FOOTER                = 0x0004,
    TDF_ALLOW_DIALOG_CANCELLATION       = 0x0008,
    TDF_USE_COMMAND_LINKS               = 0x0010,
    TDF_USE_COMMAND_LINKS_NO_ICON       = 0x0020,
    TDF_EXPAND_FOOTER_AREA              = 0x0040,
    TDF_EXPANDED_BY_DEFAULT             = 0x0080,
    TDF_VERIFICATION_FLAG_CHECKED       = 0x0100,
    TDF_SHOW_PROGRESS_BAR               = 0x0200,
    TDF_SHOW_MARQUEE_PROGRESS_BAR       = 0x0400,
    TDF_CALLBACK_TIMER                  = 0x0800,
    TDF_POSITION_RELATIVE_TO_WINDOW     = 0x1000,
    TDF_RTL_LAYOUT                      = 0x2000,
    TDF_NO_DEFAULT_RADIO_BUTTON         = 0x4000,
    TDF_CAN_BE_MINIMIZED                = 0x8000,
    TDF_NO_SET_FOREGROUND               = 0x00010000,
    TDF_SIZE_TO_CONTENT                 = 0x01000000
};

typedef enum _TASKDIALOG_COMMON_BUTTON_FLAGS {
    TDCBF_OK_BUTTON       = 0x0001,                         // IDOK
    TDCBF_YES_BUTTON      = 0x0002,                         // IDYES
    TDCBF_NO_BUTTON       = 0x0004,                         // IDNO
    TDCBF_CANCEL_BUTTON   = 0x0008,                         // IDCANCEL
    TDCBF_RETRY_BUTTON    = 0x0010,                         // IDRETRY
    TDCBF_CLOSE_BUTTON    = 0x0020                          // IDCLOSE
} TASKDIALOG_COMMON_BUTTON_FLAGS;

typedef struct _POINTL {
    LONG x;
    LONG y;
} POINTL, *PPOINTL;

typedef struct _POINT {
    LONG x;
    LONG y;
} POINT, *PPOINT, *NPPOINT, *LPPOINT;

typedef struct _CONSOLE_READCONSOLE_CONTROL {
    ULONG nLength;
    ULONG nInitialChars;
    ULONG dwCtrlWakeupMask;
    ULONG dwControlKeyState;
} CONSOLE_READCONSOLE_CONTROL, *PCONSOLE_READCONSOLE_CONTROL;

#if defined(NC_CC_CLANG)
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wmicrosoft-anon-tag"
#endif

typedef HRESULT (WINAPI* PFTASKDIALOGCALLBACK)(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, LONG_PTR lpRefData);

typedef struct _TASKDIALOGCONFIG {
    UINT                           cbSize;
    HWND                           hwndParent;
    HINSTANCE                      hInstance;
    TASKDIALOG_FLAGS               dwFlags;
    TASKDIALOG_COMMON_BUTTON_FLAGS dwCommonButtons;
    PCWSTR                         pszWindowTitle;

    union {
        HICON  hMainIcon;
        PCWSTR pszMainIcon;
    };

    PCWSTR                         pszMainInstruction;
    PCWSTR                         pszContent;
    UINT                           cButtons;
    const TASKDIALOG_BUTTON        *pButtons;
    int                            nDefaultButton;
    UINT                           cRadioButtons;
    const TASKDIALOG_BUTTON        *pRadioButtons;
    int                            nDefaultRadioButton;
    PCWSTR                         pszVerificationText;
    PCWSTR                         pszExpandedInformation;
    PCWSTR                         pszExpandedControlText;
    PCWSTR                         pszCollapsedControlText;

    union {
        HICON  hFooterIcon;
        PCWSTR pszFooterIcon;
    };

    PCWSTR                         pszFooter;
    PFTASKDIALOGCALLBACK           pfCallback;
    LONG_PTR                       lpCallbackData;
    UINT                           cxWidth;
} TASKDIALOGCONFIG;

typedef struct sockaddr {
    USHORT  sa_family;
    CHAR    sa_data[14];
} SOCKADDR, *PSOCKADDR, *LPSOCKADDR;

typedef struct in_addr {
    union {
        struct {
            UCHAR s_b1;
            UCHAR s_b2;
            UCHAR s_b3;
            UCHAR s_b4;
        } S_un_b;

        struct {
            USHORT s_w1;
            USHORT s_w2;
        } S_un_w;

        ULONG S_addr;
    } S_un;
} IN_ADDR, *PIN_ADDR, *LPIN_ADDR;

typedef struct in6_addr {
    union {
        UCHAR  Byte[16];
        USHORT Word[8];
    } u;
} IN6_ADDR, *PIN6_ADDR, FAR *LPIN6_ADDR;

typedef struct sockaddr_in {
    SHORT          sin_family;
    USHORT         sin_port;
    struct in_addr sin_addr;
    CHAR           sin_zero[8];
} SOCKADDR_IN, *PSOCKADDR_IN, *LPSOCKADDR_IN;

typedef struct sockaddr_storage {
    SHORT   ss_family;
    CHAR    __ss_pad1[6];                                                       // NOTE: 48-bit padding
    __int64 __ss_align;
    CHAR    __ss_pad2[112];                                                     // NOTE: 14-byte padding
} SOCKADDR_STORAGE, *PSOCKADDR_STORAGE;
STATIC_ASSERT(sizeof(SOCKADDR_STORAGE) == 128, SOCKADDR_STORAGESizeCheck);

typedef struct addrinfo {
    INT              ai_flags;
    INT              ai_family;
    INT              ai_socktype;
    INT              ai_protocol;
    SIZE_T           ai_addrlen;
    CHAR*            ai_canonname;
    struct sockaddr* ai_addr;
    struct addrinfo* ai_next;
} ADDRINFOA, *PADDRINFOA;

typedef struct addrinfoW {
    INT               ai_flags;
    INT               ai_family;
    INT               ai_socktype;
    INT               ai_protocol;
    SIZE_T            ai_addrlen;
    PWSTR             ai_canonname;
    struct sockaddr*  ai_addr;
    struct addrinfoW* ai_next;
} ADDRINFOW, *PADDRINFOW;

struct sockaddr_in6 {
    SHORT    sin6_family;
    USHORT   sin6_port;
    ULONG    sin6_flowinfo;
    IN6_ADDR sin6_addr;
};

typedef struct _SYSTEM_INFO {
    union _SYSTEM_INFO_DUMMYUNION {
        DWORD dwOemId;

        struct _SYSTEM_INFO_DUMMYSTRUCT {
            WORD wProcessorArchitecture;
            WORD wReserved;
        };
    };

    DWORD       dwPageSize;
    LPVOID      lpMinimumApplicationAddress;
    LPVOID      lpMaximumApplicationAddress;
    DWORD_PTR   dwActiveProcessorMask;
    DWORD       dwNumberOfProcessors;
    DWORD       dwProcessorType;
    DWORD       dwAllocationGranularity;
    WORD        wProcessorLevel;
    WORD        wProcessorRevision;
} SYSTEM_INFO, *LPSYSTEM_INFO;

#pragma pack(push, 1)
typedef struct _RGBTRIPLE {
    BYTE rgbtBlue;
    BYTE rgbtGreen;
    BYTE rgbtRed;
} RGBTRIPLE;
#pragma pack(pop)

typedef struct _RGBQUAD {
    BYTE rgbBlue;
    BYTE rgbGreen;
    BYTE rgbRed;
    BYTE rgbReserved;
} RGBQUAD;

#pragma pack(push, 2)
typedef struct _BITMAPFILEHEADER {
    WORD  bfType;
    DWORD bfSize;
    WORD  bfReserved1;
    WORD  bfReserved2;
    DWORD bfOffBits;
} BITMAPFILEHEADER;
#pragma pack(pop)

typedef struct _BITMAPINFOHEADER {
    DWORD biSize;
    LONG  biWidth;
    LONG  biHeight;
    WORD  biPlanes;
    WORD  biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    LONG  biXPelsPerMeter;
    LONG  biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
} BITMAPINFOHEADER;

typedef struct _BITMAPINFO {
    BITMAPINFOHEADER bmiHeader;
    RGBQUAD          bmiColors[1];
} BITMAPINFO, *PBITMAPINFO, *LPBITMAPINFO;

typedef struct _BITMAPCOREHEADER {
    DWORD bcSize;
    WORD  bcWidth;
    WORD  bcHeight;
    WORD  bcPlanes;
    WORD  bcBitCount;
} BITMAPCOREHEADER;

typedef struct _CIEXYZ {
    FXPT2DOT30 ciexyzX;
    FXPT2DOT30 ciexyzY;
    FXPT2DOT30 ciexyzZ;
} CIEXYZ;

typedef struct _ICEXYZTRIPLE {
    CIEXYZ ciexyzRed;
    CIEXYZ ciexyzGreen;
    CIEXYZ ciexyzBlue;
} CIEXYZTRIPLE;

#if _WINVER >= WIN32_NT4
typedef struct {
    DWORD        bV4Size;
    LONG         bV4Width;
    LONG         bV4Height;
    WORD         bV4Planes;
    WORD         bV4BitCount;
    DWORD        bV4Compression;
    DWORD        bV4SizeImage;
    LONG         bV4XPelsPerMeter;
    LONG         bV4YPelsPerMeter;
    DWORD        bV4ClrUsed;
    DWORD        bV4ClrImportant;
    DWORD        bV4RedMask;
    DWORD        bV4GreenMask;
    DWORD        bV4BlueMask;
    DWORD        bV4AlphaMask;
    DWORD        bV4CSType;
    CIEXYZTRIPLE bV4Endpoints;
    DWORD        bV4GammaRed;
    DWORD        bV4GammaGreen;
    DWORD        bV4GammaBlue;
} BITMAPV4HEADER;
#endif

#if _WINVER >= WIN32_2000
typedef struct {
    DWORD        bV5Size;
    LONG         bV5Width;
    LONG         bV5Height;
    WORD         bV5Planes;
    WORD         bV5BitCount;
    DWORD        bV5Compression;
    DWORD        bV5SizeImage;
    LONG         bV5XPelsPerMeter;
    LONG         bV5YPelsPerMeter;
    DWORD        bV5ClrUsed;
    DWORD        bV5ClrImportant;
    DWORD        bV5RedMask;
    DWORD        bV5GreenMask;
    DWORD        bV5BlueMask;
    DWORD        bV5AlphaMask;
    DWORD        bV5CSType;
    CIEXYZTRIPLE bV5Endpoints;
    DWORD        bV5GammaRed;
    DWORD        bV5GammaGreen;
    DWORD        bV5GammaBlue;
    DWORD        bV5Intent;
    DWORD        bV5ProfileData;
    DWORD        bV5ProfileSize;
    DWORD        bV5Reserved;
} BITMAPV5HEADER;
#endif

typedef struct _BITMAP {
    LONG   bmType;
    LONG   bmWidth;
    LONG   bmHeight;
    LONG   bmWidthBytes;
    WORD   bmPlanes;
    WORD   bmBitsPixel;
    LPVOID bmBits;
} BITMAP;

typedef struct _DIBSECTION {
    BITMAP           dsBm;
    BITMAPINFOHEADER dsBmih;
    DWORD            dsBitfields[3];
    HANDLE           dshSection;
    DWORD            dsOffset;
} DIBSECTION, *LPDIBSECTION, *PDIBSECTION;

typedef union _LARGE_INTEGER {
    struct {
        DWORD LowPart;
        LONG HighPart;
    };

    struct u {
        DWORD LowPart;
        LONG HightPart;
    };

    LONGLONG QuadPart;
} LARGE_INTEGER, *LPLARGE_INTEGER, *PLARGE_INTEGER;

typedef struct _OVERLAPPED {
    ULONG_PTR Internal;
    ULONG_PTR InternalHigh;

    union {
        struct {
            DWORD Offset;
            DWORD OffsetHigh;
        };

        PVOID Pointer;
    };

    HANDLE hEvent;
} OVERLAPPED, *LPOVERLAPPED;

typedef struct _devicemodeW {
    WCHAR dmDeviceName[CCHDEVICENAME];
    WORD  dmSpecVersion;
    WORD  dmDriverVersion;
    WORD  dmSize;
    WORD  dmDriverExtra;
    DWORD dmFields;

    union _DUMMYUNIONNAME {
        struct _DUMMYSTRUCTNAME {
            short dmOrientation;
            short dmPaperSize;
            short dmPaperLength;
            short dmPaperWidth;
            short dmScale;
            short dmCopies;
            short dmDefaultSource;
            short dmPrintQuality;
        };

        POINTL dmPosition;

        struct _DUMMYSTRUCTNAME2 {
          POINTL dmPosition;
          DWORD  dmDisplayOrientation;
          DWORD  dmDisplayFixedOutput;
        };
    };

    short dmColor;
    short dmDuplex;
    short dmYResolution;
    short dmTTOption;
    short dmCollate;
    WCHAR dmFormName[CCHFORMNAME];
    WORD  dmLogPixels;
    DWORD dmBitsPerPel;
    DWORD dmPelsWidth;
    DWORD dmPelsHeight;

    union _DUMMYUNIONNAME2 {
        DWORD dmDisplayFlags;
        DWORD dmNup;
    };

    DWORD dmDisplayFrequency;
    DWORD dmICMMethod;
    DWORD dmICMIntent;
    DWORD dmMediaType;
    DWORD dmDitherType;
    DWORD dmReserved1;
    DWORD dmReserved2;
    DWORD dmPanningWidth;
    DWORD dmPanningHeight;
} DEVMODEW, *PDEVMODEW, *NPDEVMODEW, *LPDEVMODEW;

typedef struct ALIGNAS(16) _M128A {
    ULONGLONG Low;
    LONGLONG High;
} M128A, *PM128A;

typedef struct ALIGNAS(16) _XSAVE_FORMAT {
    WORD   ControlWord;
    WORD   StatusWord;
    BYTE  TagWord;
    BYTE  Reserved1;
    WORD   ErrorOpcode;
    DWORD ErrorOffset;
    WORD   ErrorSelector;
    WORD   Reserved2;
    DWORD DataOffset;
    WORD   DataSelector;
    WORD   Reserved3;
    DWORD MxCsr;
    DWORD MxCsr_Mask;
    M128A FloatRegisters[8];

#if defined(NC_CPU_X86_64)
    M128A XmmRegisters[16];
    BYTE  Reserved4[96];
#else
    M128A XmmRegisters[8];
    BYTE  Reserved4[224];
#endif
} XSAVE_FORMAT, *PXSAVE_FORMAT;

typedef XSAVE_FORMAT XMM_SAVE_AREA32, *PXMM_SAVE_AREA32;

typedef struct ALIGNAS(16) _CONTEXT {
    DWORD64 P1Home;
    DWORD64 P2Home;
    DWORD64 P3Home;
    DWORD64 P4Home;
    DWORD64 P5Home;
    DWORD64 P6Home;
    DWORD   ContextFlags;
    DWORD   MxCsr;
    WORD    SegCs;
    WORD    SegDs;
    WORD    SegEs;
    WORD    SegFs;
    WORD    SegGs;
    WORD    SegSs;
    DWORD   EFlags;
    DWORD64 Dr0;
    DWORD64 Dr1;
    DWORD64 Dr2;
    DWORD64 Dr3;
    DWORD64 Dr6;
    DWORD64 Dr7;
    DWORD64 Rax;
    DWORD64 Rcx;
    DWORD64 Rdx;
    DWORD64 Rbx;
    DWORD64 Rsp;
    DWORD64 Rbp;
    DWORD64 Rsi;
    DWORD64 Rdi;
    DWORD64 R8;
    DWORD64 R9;
    DWORD64 R10;
    DWORD64 R11;
    DWORD64 R12;
    DWORD64 R13;
    DWORD64 R14;
    DWORD64 R15;
    DWORD64 Rip;

    union {
        XMM_SAVE_AREA32 FltSave;
        struct {
            M128A Header[2];
            M128A Legacy[8];
            M128A Xmm0;
            M128A Xmm1;
            M128A Xmm2;
            M128A Xmm3;
            M128A Xmm4;
            M128A Xmm5;
            M128A Xmm6;
            M128A Xmm7;
            M128A Xmm8;
            M128A Xmm9;
            M128A Xmm10;
            M128A Xmm11;
            M128A Xmm12;
            M128A Xmm13;
            M128A Xmm14;
            M128A Xmm15;
        };
    };

    M128A   VectorRegister[26];
    DWORD64 VectorControl;
    DWORD64 DebugControl;
    DWORD64 LastBranchToRip;
    DWORD64 LastBranchFromRip;
    DWORD64 LastExceptionToRip;
    DWORD64 LastExceptionFromRip;
} CONTEXT, *PCONTEXT;

typedef struct _MINIDUMP_MEMORY_INFO {
    ULONG64 BaseAddress;
    ULONG64 AllocationBase;
    ULONG32 AllocationProtect;
    ULONG32 __alignment1;
    ULONG64 RegionSize;
    ULONG32 State;
    ULONG32 Protect;
    ULONG32 Type;
    ULONG32 __alignment2;
} MINIDUMP_MEMORY_INFO, *PMINIDUMP_MEMORY_INFO;

typedef struct _MINIDUMP_CALLBACK_OUTPUT {
    union {
        ULONG   ModuleWriteFlags;
        ULONG   ThreadWriteFlags;
        ULONG   SecondaryFlags;

        struct {
            ULONG64 MemoryBase;
            ULONG   MemorySize;
        };

        struct {
            BOOL CheckCancel;
            BOOL Cancel;
        };

        HANDLE  Handle;

        struct {
            MINIDUMP_MEMORY_INFO VmRegion;
            BOOL                 Continue;
        };

        struct {
            HRESULT              VmQueryStatus;
            MINIDUMP_MEMORY_INFO VmQueryResult;
        };

        struct {
            HRESULT VmReadStatus;
            ULONG   VmReadBytesCompleted;
        };

        HRESULT Status;
    };
} MINIDUMP_CALLBACK_OUTPUT, *PMINIDUMP_CALLBACK_OUTPUT;

typedef struct _RIO_NOTIFICATION_COMPLETION {
    RIO_NOTIFICATION_COMPLETION_TYPE Type;

    union {
        struct {
            HANDLE EventHandle;
            BOOL   NotifyReset;
        } Event;

        struct {
            HANDLE IocpHandle;
            PVOID  CompletionKey;
            PVOID  Overlapped;
        } Iocp;
    };
} RIO_NOTIFICATION_COMPLETION, *PRIO_NOTIFICATION_COMPLETION;

#if defined(NC_CC_CLANG)
    #pragma clang diagnostic pop
#endif

typedef struct _SYSTEMTIME {
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
} SYSTEMTIME, *PSYSTEMTIME, *LPSYSTEMTIME;

typedef struct _CRITICAL_SECTION {
    BYTE _[40];
} CRITICAL_SECTION, *LPCRITICAL_SECTION, *PCRITICAL_SECTION;

typedef struct _SRWLOCK {
    PVOID Ptr;
} SRWLOCK, *PSRWLOCK;

typedef struct _CONDITION_VARIABLE {
    PVOID Ptr;
} CONDITION_VARIABLE, *PCONDITION_VARIABLE;

typedef struct _SYNCHRONIZATION_BARRIER {
    DWORD     Reserved1;
    DWORD     Reserved2;
    ULONG_PTR Reserved3[2];
    DWORD     Reserved4;
    DWORD     Reserved5;
} SYNCHRONIZATION_BARRIER, *PSYNCHRONIZATION_BARRIER, *LPSYNCHRONIZATION_BARRIER;

typedef struct _SECURITY_ATTRIBUTES {
    DWORD  nLength;
    LPVOID lpSecurityDescriptor;
    BOOL   bInheritHandle;
} SECURITY_ATTRIBUTES, *PSECURITY_ATTRIBUTES, *LPSECURITY_ATTRIBUTES;

typedef struct _FILETIME {
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
} FILETIME, *PFILETIME, *LPFILETIME;

typedef struct _BY_HANDLE_FILE_INFORMATION {
    DWORD    dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    DWORD    dwVolumeSerialNumber;
    DWORD    nFileSizeHigh;
    DWORD    nFileSizeLow;
    DWORD    nNumberOfLinks;
    DWORD    nFileIndexHigh;
    DWORD    nFileIndexLow;
} BY_HANDLE_FILE_INFORMATION, *PBY_HANDLE_FILE_INFORMATION, *LPBY_HANDLE_FILE_INFORMATION;

typedef struct _WIN32_FIND_DATAW {
    DWORD    dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    DWORD    nFileSizeHigh;
    DWORD    nFileSizeLow;
    DWORD    dwReserved0;
    DWORD    dwReserved1;
    WCHAR    cFileName[MAX_PATH];
    WCHAR    cAlternateFileName[14];
    DWORD    dwFileType;                                                        // WARN: Obsolete. Do not use.
    DWORD    dwCreatorType;                                                     // WARN: Obsolete. Do not use
    WORD     wFinderFlags;                                                      // WARN: Obsolete. Do not use
} WIN32_FIND_DATAW, *PWIN32_FIND_DATAW, *LPWIN32_FIND_DATAW;

typedef struct _FILE_ALLOCATION_INFO {
    LARGE_INTEGER AllocationSize;
} FILE_ALLOCATION_INFO, *PFILE_ALLOCATION_INFO;

typedef struct _WIN32_FILE_ATTRIBUTE_DATA {
    DWORD    dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    DWORD    nFileSizeHigh;
    DWORD    nFileSizeLow;
} WIN32_FILE_ATTRIBUTE_DATA, *LPWIN32_FILE_ATTRIBUTE_DATA;

typedef struct _STARTUPINFOW {
    DWORD  cb;
    LPWSTR lpReserved;
    LPWSTR lpDesktop;
    LPWSTR lpTitle;
    DWORD  dwX;
    DWORD  dwY;
    DWORD  dwXSize;
    DWORD  dwYSize;
    DWORD  dwXCountChars;
    DWORD  dwYCountChars;
    DWORD  dwFillAttribute;
    DWORD  dwFlags;
    WORD   wShowWindow;
    WORD   cbReserved2;
    LPBYTE lpReserved2;
    HANDLE hStdInput;
    HANDLE hStdOutput;
    HANDLE hStdError;
} STARTUPINFOW, *LPSTARTUPINFOW;

typedef struct _PROCESS_INFORMATION {
    HANDLE hProcess;
    HANDLE hThread;
    DWORD  dwProcessId;
    DWORD  dwThreadId;
} PROCESS_INFORMATION, *PPROCESS_INFORMATION, *LPPROCESS_INFORMATION;

typedef struct _RECT {
    LONG left;
    LONG top;
    LONG right;
    LONG bottom;
} RECT, *PRECT, *NPRECT, *LPRECT;

typedef struct _WINDOWPLACEMENT {
    UINT  length;
    UINT  flags;
    UINT  showCmd;
    POINT ptMinPosition;
    POINT ptMaxPosition;
    RECT  rcNormalPosition;
    RECT  rcDevice;
} WINDOWPLACEMENT;

typedef struct _MINMAXINFO {
    POINT ptReserved;
    POINT ptMaxSize;
    POINT ptMaxPosition;
    POINT ptMinTrackSize;
    POINT ptMaxTrackSize;
} MINMAXINFO, *PMINMAXINFO, *LPMINMAXINFO;

typedef struct _COPYDATASTRUCT {
    ULONG_PTR dwData;
    DWORD     cbData;
    PVOID     lpData;
} COPYDATASTRUCT, *PCOPYDATASTRUCT;

#if(WINVER >= WIN32_NT4)
    typedef struct _MDINEXTMENU {
        HMENU hmenuIn;
        HMENU hmenuNext;
        HWND  hwndNext;
    } MDINEXTMENU, * PMDINEXTMENU, * LPMDINEXTMENU;
#endif

typedef struct {
    GUID  PowerSetting;
    DWORD DataLength;
    UCHAR Data[1];
} POWERBROADCAST_SETTING, *PPOWERBROADCAST_SETTING;

typedef struct _SecHandle {
    ULONG_PTR dwLower;
    ULONG_PTR dwUpper;
} SecHandle, *PSecHandle;

typedef SecHandle  CredHandle;
typedef PSecHandle PCredHandle;
typedef SecHandle  CtxHandle;
typedef PSecHandle PCtxHandle;

typedef struct _SecPkgContext_StreamSizes {
    unsigned long cbHeader;
    unsigned long cbTrailer;
    unsigned long cbMaximumMessage;
    unsigned long cBuffers;
    unsigned long cbBlockSize;
} SecPkgContext_StreamSizes, *PSecPkgContext_StreamSizes;

typedef struct _SecBuffer {
    ULONG cbBuffer;
    ULONG BufferType;
    void* pvBuffer;
} SecBuffer;

typedef struct _SecBufferDesc {
    ULONG      ulVersion;
    ULONG      cBuffers;
    SecBuffer* pBuffers;
} SecBufferDesc;

typedef struct _CRYPTOAPI_BLOB {
    DWORD cbData;
    BYTE* pbData;
} CRYPT_INTEGER_BLOB, 
 *PCRYPT_INTEGER_BLOB, 
  CRYPT_UINT_BLOB, 
 *PCRYPT_UINT_BLOB, 
  CRYPT_OBJID_BLOB, 
 *PCRYPT_OBJID_BLOB, 
  CERT_NAME_BLOB, 
  CERT_RDN_VALUE_BLOB, 
 *PCERT_NAME_BLOB, 
 *PCERT_RDN_VALUE_BLOB, 
  CERT_BLOB, 
 *PCERT_BLOB, 
  CRL_BLOB, 
 *PCRL_BLOB, 
  DATA_BLOB, 
 *PDATA_BLOB, 
  CRYPT_DATA_BLOB, 
 *PCRYPT_DATA_BLOB, 
  CRYPT_HASH_BLOB, 
 *PCRYPT_HASH_BLOB, 
  CRYPT_DIGEST_BLOB, 
 *PCRYPT_DIGEST_BLOB, 
  CRYPT_DER_BLOB, 
  PCRYPT_DER_BLOB, 
  CRYPT_ATTR_BLOB, 
 *PCRYPT_ATTR_BLOB;

typedef struct _CRYPT_BIT_BLOB {
    DWORD cbData;
    BYTE  *pbData;
    DWORD cUnusedBits;
} CRYPT_BIT_BLOB, *PCRYPT_BIT_BLOB;

typedef struct _CRYPT_ALGORITHM_IDENTIFIER {
    LPSTR            pszObjId;
    CRYPT_OBJID_BLOB Parameters;
} CRYPT_ALGORITHM_IDENTIFIER, *PCRYPT_ALGORITHM_IDENTIFIER;

typedef struct _CERT_PUBLIC_KEY_INFO {
    CRYPT_ALGORITHM_IDENTIFIER Algorithm;
    CRYPT_BIT_BLOB             PublicKey;
} CERT_PUBLIC_KEY_INFO, *PCERT_PUBLIC_KEY_INFO;

typedef struct _CERT_EXTENSION {
    LPSTR            pszObjId;
    BOOL             fCritical;
    CRYPT_OBJID_BLOB Value;
} CERT_EXTENSION, *PCERT_EXTENSION;

typedef struct _CERT_INFO {
    DWORD                      dwVersion;
    CRYPT_INTEGER_BLOB         SerialNumber;
    CRYPT_ALGORITHM_IDENTIFIER SignatureAlgorithm;
    CERT_NAME_BLOB             Issuer;
    FILETIME                   NotBefore;
    FILETIME                   NotAfter;
    CERT_NAME_BLOB             Subject;
    CERT_PUBLIC_KEY_INFO       SubjectPublicKeyInfo;
    CRYPT_BIT_BLOB             IssuerUniqueId;
    CRYPT_BIT_BLOB             SubjectUniqueId;
    DWORD                      cExtension;
    PCERT_EXTENSION            rgExtension;
} CERT_INFO, *PCERT_INFO;

typedef struct _CERT_CONTEXT {
    DWORD      dwCertEncodingType;
    BYTE*      pbCertEncoded;
    DWORD      cbCertEncoded;
    PCERT_INFO pCertInfo;
    HCERTSTORE hCertStore;
} CERT_CONTEXT, *PCERT_CONTEXT;

typedef CERT_CONTEXT const* PCCERT_CONTEXT;

typedef struct _SCHANNEL_CRED {
    DWORD           dwVersion;
    DWORD           cCreds;
    PCCERT_CONTEXT* paCred;
    HCERTSTORE      hRootStore;
    DWORD           cMappers;
    void**          aphMappers;
    DWORD           cSupportedAlgs;
    void*           palgSupportedAlgs;
    DWORD           grbitEnabledProtocols;
    DWORD           dwMinimumCipherStrength;
    DWORD           dwMaximumCipherStrength;
    DWORD           dwSessionLifespan;
    DWORD           dwFlags;
    DWORD           dwCredFormat;
} SCHANNEL_CRED;

typedef struct _PAINTSTRUCT {
    HDC  hdc;
    BOOL fErase;
    RECT rcPaint;
    BOOL fRestore;
    BOOL fIncUpdate;
    BYTE rgbReserved[32];
} PAINTSTRUCT, *PPAINTSTRUCT, *NPPAINTSTRUCT, *LPPAINTSTRUCT;

typedef struct _MARGINS {
    int cxLeftWidth;
    int csRightWifth;
    int cyTopHeight;
    int cyBottomHeight;
} MARGINS, *PMARGINS;

typedef struct _WINDOWPOS {
    HWND hwnd;
    HWND hwndInsertAfter;
    int  x;
    int  y;
    int  cx;
    int  cy;
    UINT flags;
} WINDOWPOS, *LPWINDOWPOS, *PWINDOWPOS;

typedef struct _NCCALCSIZE_PARAMS {
    RECT       rgrc[3];
    PWINDOWPOS lppos;
} NCCALCSIZE_PARAMS, *LPNCCALCSIZE_PARAMS;

typedef LRESULT (*WNDPROC)(HWND unnamedParam1, UINT unnamedParam2, WPARAM unnamedParam3, LPARAM unnamedParam4);

typedef struct _WNDCLASSEXW {
    UINT      cbSize;
    UINT      style;
    WNDPROC   lpfnWndProc;
    int       cbClsExtra;
    int       cbWndExtra;
    HINSTANCE hInstance;
    HICON     hIcon;
    HCURSOR   hCursor;
    HBRUSH    hbrBackground;
    LPCWSTR   lpszMenuName;
    LPCWSTR   lpszClassName;
    HICON     hIconSm;
} WNDCLASSEXW, *PWNDCLASSEXW, *NPWNDCLASSEXW, *LPWNDCLASSEXW;

typedef struct _DISPLAY_DEVICEW {
    DWORD cb;
    WCHAR DeviceName[32];
    WCHAR DeviceString[128];
    DWORD StateFlags;
    WCHAR DeviceID[128];
    WCHAR DeviceKey[128];
} DISPLAY_DEVICEW, *PDISPLAY_DEVICEW, *LPDISPLAY_DEVICEW;

typedef struct _MONITORINFO {
    DWORD cbSize;
    RECT rcMonitor;
    RECT rcWork;
    DWORD dwFlags;
} MONITORINFO, *LPMONITORINFO;

typedef struct _MONITORINFOEXW {
    DWORD cbSize;
    RECT rcMonitor;
    RECT rcWork;
    DWORD dwFlags;
    WCHAR szDevice[CCHDEVICENAME];
} MONITORINFOEXW, *LPMONITORINFOEXW;

typedef struct _MSG {
    HWND   hwnd;
    UINT   message;
    WPARAM wParam;
    LPARAM lParam;
    DWORD  time;
    POINT  pt;
    DWORD  lPrivate;
} MSG, *PMSG, *NPMSG, *LPMSG;

typedef UINT_PTR (WINAPI* LPOFNHOOKPROC)(HWND unnamedParam1, UINT unnamedParam2, WPARAM unnamedParam3, LPARAM unnamedParam4);

typedef struct _OFNW {
    DWORD         lStructSize;
    HWND          hwndOwner;
    HINSTANCE     hInstance;
    LPCWSTR       lpstrFilter;
    LPWSTR        lpstrCustomFilter;
    DWORD         nMaxCustFilter;
    DWORD         nFilterIndex;
    LPWSTR        lpstrFile;
    DWORD         nMaxFile;
    LPWSTR        lpstrFileTitle;
    DWORD         nMaxFileTitle;
    LPCWSTR       lpstrInitialDir;
    LPCWSTR       lpstrTitle;
    DWORD         Flags;
    WORD          nFileOffset;
    WORD          nFileExtension;
    LPCWSTR       lpstrDefExt;
    LPARAM        lCustData;
    LPOFNHOOKPROC lpfnHook;
    LPCWSTR       lpTemplateName;
    void*         pvReserved;
    DWORD         dwReserved;
    DWORD         FlagsEx;
} OPENFILENAMEW, *LPOPENFILENAMEW;

typedef struct _SHITEMID {
    USHORT cb;
    BYTE   abID[1];
} SHITEMID;

typedef struct _ITEMIDLIST {
    SHITEMID mkid;
} ITEMIDLIST, *PITEMIDLIST, *LPITEMIDLIST;

typedef struct _IMAGE_TLS_DIRECTORY64 {
    ULONGLONG   StartAddressOfRawData;
    ULONGLONG   EndAddressOfRawData;
    ULONGLONG   AddressOfIndex;
    ULONGLONG   AddressOfCallBacks;
    DWORD       SizeOfZeroFill;
    DWORD       Characteristics;
} IMAGE_TLS_DIRECTORY64, *PIMAGE_TLS_DIRECTORY64;

typedef struct _IMAGE_LOAD_CONFIG_CODE_INTEGRITY {
    WORD  Flags;
    WORD  Catalog;
    DWORD CatalogOffset;
    DWORD Reserved;
} IMAGE_LOAD_CONFIG_CODE_INTEGRITY, *PIMAGE_LOAD_CONFIG_CODE_INTEGRITY;

typedef struct _IMAGE_LOAD_CONFIG_DIRECTORY64 {
    DWORD                            Size;
    DWORD                            TimeDateStamp;
    WORD                             MajorVersion;
    WORD                             MinorVersion;
    DWORD                            GlobalFlagsClear;
    DWORD                            GlobalFlagsSet;
    DWORD                            CriticalSectionDefaultTimeout;
    ULONGLONG                        DeCommitFreeBlockThreshold;
    ULONGLONG                        DeCommitTotalFreeThreshold;
    ULONGLONG                        LockPrefixTable;
    ULONGLONG                        MaximumAllocationSize;
    ULONGLONG                        VirtualMemoryThreshold;
    ULONGLONG                        ProcessAffinityMask;
    DWORD                            ProcessHeapFlags;
    WORD                             CSDVersion;
    WORD                             DependentLoadFlags;
    ULONGLONG                        EditList;
    ULONGLONG                        SecurityCookie;
    ULONGLONG                        SEHandlerTable;
    ULONGLONG                        SEHandlerCount;
    ULONGLONG                        GuardCFCheckFunctionPointer;
    ULONGLONG                        GuardCFDispatchFunctionPointer;
    ULONGLONG                        GuardCFFunctionTable;
    ULONGLONG                        GuardCFFunctionCount;
    DWORD                            GuardFlags;
    IMAGE_LOAD_CONFIG_CODE_INTEGRITY CodeIntegrity;
    ULONGLONG                        GuardAddressTakenIatEntryTable;
    ULONGLONG                        GuardAddressTakenIatEntryCount;
    ULONGLONG                        GuardLongJumpTargetTable;
    ULONGLONG                        GuardLongJumpTargetCount;
    ULONGLONG                        DynamicValueRelocTable;
    ULONGLONG                        CHPEMetadataPointer;
    ULONGLONG                        GuardRFFailureRoutine;
    ULONGLONG                        GuardRFFailureRoutineFunctionPointer;
    DWORD                            DynamicValueRelocTableOffset;
    WORD                             DynamicValueRelocTableSection;
    WORD                             Reserved2;
    ULONGLONG                        GuardRFVerifyStackPointerFunctionPointer;
    DWORD                            HotPatchTableOffset;
    DWORD                            Reserved3;
    ULONGLONG                        EnclaveConfigurationPointer;
    ULONGLONG                        VolatileMetadataPointer;
    ULONGLONG                        GuardEHContinuationTable;
    ULONGLONG                        GuardEHContinuationCount;
    ULONGLONG                        GuardXFGCheckFunctionPointer;
    ULONGLONG                        GuardXFGDispatchFunctionPointer;
    ULONGLONG                        GuardXFGTableDispatchFunctionPointer;
    ULONGLONG                        CastGuardOsDeterminedFailureMode;
    ULONGLONG                        GuardMemcpyFunctionPointer;
    ULONGLONG                        TlsDirectoryAddress;
} IMAGE_LOAD_CONFIG_DIRECTORY64, *PIMAGE_LOAD_CONFIG_DIRECTORY64;

typedef struct _EXCEPTION_RECORD32 {
    DWORD ExceptionCode;
    DWORD ExceptionFlags;
    DWORD ExceptionRecord;
    DWORD ExceptionAddress;
    DWORD NumberParameters;
    DWORD ExceptionInformation[EXCEPTION_MAXIMUM_PARAMETERS];
} EXCEPTION_RECORD32, *PEXCEPTION_RECORD32;

typedef struct _EXCEPTION_RECORD64 {
    DWORD   ExceptionCode;
    DWORD   ExceptionFlags;
    DWORD64 ExceptionRecord;
    DWORD64 ExceptionAddress;
    DWORD   NumberParameters;
    DWORD   __unusedAlignment;
    DWORD64 ExceptionInformation[EXCEPTION_MAXIMUM_PARAMETERS];
} EXCEPTION_RECORD64, *PEXCEPTION_RECORD64;

typedef struct _EXCEPTION_RECORD {
    DWORD ExceptionCode;
    DWORD ExceptionFlags;
    struct _EXCEPTION_RECORD* ExceptionRecord;
    PVOID ExceptionAddress;
    DWORD NumberParameters;
    ULONG_PTR ExceptionInformation[EXCEPTION_MAXIMUM_PARAMETERS];
} EXCEPTION_RECORD, *PEXCEPTION_RECORD;

typedef struct _EXCEPTION_POINTERS {
    PEXCEPTION_RECORD ExceptionRecord;
    PCONTEXT ContextRecord;
} EXCEPTION_POINTERS, *PEXCEPTION_POINTERS;

typedef struct _MINIDUMP_EXCEPTION_INFORMATION {
    DWORD ThreadId;
    PEXCEPTION_POINTERS ExceptionPointers;
    BOOL ClientPointers;
} MINIDUMP_EXCEPTION_INFORMATION, *PMINIDUMP_EXCEPTION_INFORMATION;

typedef struct _MINIDUMP_USER_STREAM {
    ULONG32 Type;
    ULONG   BufferSize;
    PVOID   Buffer;
} MINIDUMP_USER_STREAM, *PMINIDUMP_USER_STREAM;

typedef struct _MINIDUMP_USER_STREAM_INFORMATION {
    ULONG                 UserStreamCount;
    PMINIDUMP_USER_STREAM UserStreamArray;
} MINIDUMP_USER_STREAM_INFORMATION, *PMINIDUMP_USER_STREAM_INFORMATION;

typedef struct _MINIDUMP_THREAD_EX_CALLBACK {
    ULONG   ThreadId;
    HANDLE  ThreadHandle;
    ULONG   Pad;
    CONTEXT Context;
    ULONG   SizeOfContext;
    ULONG64 StackBase;
    ULONG64 StackEnd;
    ULONG64 BackingStoreBase;
    ULONG64 BackingStoreEnd;
} MINIDUMP_THREAD_EX_CALLBACK, *PMINIDUMP_THREAD_EX_CALLBACK;

typedef struct _VS_FIXEDFILEINFO {
    DWORD dwSignature;
    DWORD dwStrucVersion;
    DWORD dwFileVersionMS;
    DWORD dwFileVersionLS;
    DWORD dwProductVersionMS;
    DWORD dwProductVersionLS;
    DWORD dwFileFlagsMask;
    DWORD dwFileFlags;
    DWORD dwFileOS;
    DWORD dwFileType;
    DWORD dwFileSubtype;
    DWORD dwFileDateMS;
    DWORD dwFileDateLS;
} VS_FIXEDFILEINFO;

typedef struct _MINIDUMP_MODULE_CALLBACK {
    WCHAR*           FullPath;
    ULONG64          BaseOfImage;
    ULONG            SizeOfImage;
    ULONG            CheckSum;
    ULONG            TimeDateStamp;
    VS_FIXEDFILEINFO VersionInfo;
    PVOID            CvRecord;
    ULONG            SizeOfCvRecord;
    PVOID            MiscRecord;
    ULONG            SizeOfMiscRecord;
} MINIDUMP_MODULE_CALLBACK, *PMINIDUMP_MODULE_CALLBACK;

typedef struct _MINIDUMP_THREAD_CALLBACK {
    ULONG   ThreadId;
    HANDLE  ThreadHandle;
    ULONG   Pad;
    CONTEXT Context;
    ULONG   SizeOfContext;
    ULONG64 StackBase;
    ULONG64 StackEnd;
} MINIDUMP_THREAD_CALLBACK, *PMINIDUMP_THREAD_CALLBACK;

typedef struct _MINIDUMP_IO_CALLBACK {
    HANDLE  Handle;
    ULONG64 Offset;
    PVOID   Buffer;
    ULONG   BufferBytes;
} MINIDUMP_IO_CALLBACK, *PMINIDUMP_IO_CALLBACK;

typedef struct _MINIDUMP_INCLUDE_MODULE_CALLBACK {
    ULONG64 BaseOfImage;
} MINIDUMP_INCLUDE_MODULE_CALLBACK, *PMINIDUMP_INCLUDE_MODULE_CALLBACK;

typedef struct _MINIDUMP_INCLUDE_THREAD_CALLBACK {
    ULONG ThreadId;
} MINIDUMP_INCLUDE_THREAD_CALLBACK, *PMINIDUMP_INCLUDE_THREAD_CALLBACK;

typedef struct _MINIDUMP_READ_MEMORY_FAILURE_CALLBACK {
    ULONG64 Offset;
    ULONG   Bytes;
    HRESULT FailureStatus;
} MINIDUMP_READ_MEMORY_FAILURE_CALLBACK, *PMINIDUMP_READ_MEMORY_FAILURE_CALLBACK;

typedef struct _MINIDUMP_VM_QUERY_CALLBACK {
    ULONG64 Offset;
} MINIDUMP_VM_QUERY_CALLBACK;

typedef struct _MINIDUMP_VM_PRE_READ_CALLBACK {
    ULONG64 Offset;
    PVOID   Buffer;
    DWORD   Size;
} MINIDUMP_VM_PRE_READ_CALLBACK;

typedef struct _MINIDUMP_VM_POST_READ_CALLBACK {
    ULONGLONG Offset;
    PVOID     Buffer;
    DWORD     Size;
    DWORD     Completed;
    HRESULT   Status;
} MINIDUMP_VM_POST_READ_CALLBACK;

typedef struct _MINIDUMP_CALLBACK_INPUT {
    ULONG  ProcessId;
    HANDLE ProcessHandle;
    ULONG  CallbackType;

    union {
        HRESULT                               Status;
        MINIDUMP_THREAD_CALLBACK              Thread;
        MINIDUMP_THREAD_EX_CALLBACK           ThreadEx;
        MINIDUMP_MODULE_CALLBACK              Module;
        MINIDUMP_INCLUDE_THREAD_CALLBACK      IncludeThread;
        MINIDUMP_INCLUDE_MODULE_CALLBACK      IncludeModule;
        MINIDUMP_IO_CALLBACK                  Io;
        MINIDUMP_READ_MEMORY_FAILURE_CALLBACK ReadMemoryFailure;
        ULONG                                 SecondaryFlags;
        MINIDUMP_VM_QUERY_CALLBACK            VmQuery;
        MINIDUMP_VM_PRE_READ_CALLBACK         VmPreRead;
        MINIDUMP_VM_POST_READ_CALLBACK        VmPostRead;
    };
} MINIDUMP_CALLBACK_INPUT, *PMINIDUMP_CALLBACK_INPUT;

typedef BOOL (WINAPI* MINIDUMP_CALLBACK_ROUTINE)(PVOID CallbackParam, PMINIDUMP_CALLBACK_INPUT CallbackInput, PMINIDUMP_CALLBACK_OUTPUT CallbackOutput);

typedef struct _MINIDUMP_CALLBACK_INFORMATION {
    MINIDUMP_CALLBACK_ROUTINE CallbackRoutine;
    PVOID                     CallbackParam;
} MINIDUMP_CALLBACK_INFORMATION, *PMINIDUMP_CALLBACK_INFORMATION;

typedef struct _ADDRESS64 {
    DWORD64      Offset;
    WORD         Segment;
    ADDRESS_MODE Mode;
} ADDRESS64, *LPADDRESS64;

typedef struct _KDHELP64 {
    DWORD64 Thread;
    DWORD   ThCallbackStack;
    DWORD   ThCallbackBStore;
    DWORD   NextCallback;
    DWORD   FramePointer;
    DWORD64 KiCallUserMode;
    DWORD64 KeUserCallbackDispatcher;
    DWORD64 SystemRangeStart;
    DWORD64 KiUserExceptionDispatcher;
    DWORD64 StackBase;
    DWORD64 StackLimit;
    DWORD   BuildVersion;
    DWORD   RetpolineStubFunctionTableSize;
    DWORD64 RetpolineStubFunctionTable;
    DWORD   RetpolineStubOffset;
    DWORD   RetpolineStubSize;
    DWORD64 Reserved0[2];
} KDHELP64, *PKDHELP64;

typedef struct _STACKFRAME64 {
    ADDRESS64 AddrPC;
    ADDRESS64 AddrReturn;
    ADDRESS64 AddrFrame;
    ADDRESS64 AddrStack;
    ADDRESS64 AddrBStore;
    PVOID     FuncTableEntry;
    DWORD64   Params[4];
    BOOL      Far;
    BOOL      Virtual;
    DWORD64   Reserved[3];
    KDHELP64  KdHelp;
} STACKFRAME64, *LPSTACKFRAME64;

typedef struct _SYMBOL_INFOW {
    ULONG   SizeOfStruct;
    ULONG   TypeIndex;
    ULONG64 Reserved[2];
    ULONG   Index;
    ULONG   Size;
    ULONG64 ModBase;
    ULONG   Flags;
    ULONG64 Value;
    ULONG64 Address;
    ULONG   Register;
    ULONG   Scope;
    ULONG   Tag;
    ULONG   NameLen;
    ULONG   MaxNameLen;
    WCHAR   Name[1];
} SYMBOL_INFOW, *PSYMBOL_INFOW;

#if !defined(_IMAGEHLP_SOURCE_) && defined(_IMAGEHLP64)
    #define IMAGEHLP_LINE  IMAGEHLP_LINE64
    #define PIMAGEHLP_LINE PIMAGEHLP_LINE64
#else
    typedef struct _IMAGEHLP_LINE {
        DWORD    SizeOfStruct; 
        PVOID    Key; 
        DWORD    LineNumber; 
        PCHAR    FileName; 
        DWORD    Address; 
    } IMAGEHLP_LINE, *PIMAGEHLP_LINE;

    typedef struct _IMAGEHLP_LINEW {
        DWORD    SizeOfStruct; 
        PVOID    Key; 
        DWORD    LineNumber; 
        PCHAR    FileName; 
        DWORD64  Address; 
    } IMAGEHLP_LINEW, *PIMAGEHLP_LINEW;
#endif

typedef struct _IMAGEHLP_LINEW64 {
    DWORD   SizeOfStruct;
    PVOID   Key;
    DWORD   LineNumber;
    PWSTR   FileName;
    DWORD64 Address;
} IMAGEHLP_LINEW64, *PIMAGEHLP_LINEW64;

#if !defined(_IMAGEHLP_SOURCE_) && defined(_IMAGEHLP64)
    #define IMAGEHLP_MODULE   IMAGEHLP_MODULE64
    #define PIMAGEHLP_MODULE  PIMAGEHLP_MODULE64
    #define IMAGEHLP_MODULEW  IMAGEHLP_MODULEW64
    #define PIMAGEHLP_MODULEW PIMAGEHLP_MODULEW64
#else
    typedef struct _IMAGEHLP_MODULE {
        DWORD    SizeOfStruct;
        DWORD    BaseOfImage; 
        DWORD    ImageSize; 
        DWORD    TimeDateStamp; 
        DWORD    CheckSum; 
        DWORD    NumSyms; 
        SYM_TYPE SymType; 
        CHAR     ModuleName[32];  
        CHAR     ImageName[256]; 
        CHAR     LoadedImageName[256]; 
    } IMAGEHLP_MODULE, *PIMAGEHLP_MODULE;

    typedef struct _IMAGEHLP_MODULEW {
        DWORD    SizeOfStruct;  
        DWORD    BaseOfImage; 
        DWORD    ImageSize;  
        DWORD    TimeDateStamp; 
        DWORD    CheckSum; 
        DWORD    NumSyms; 
        SYM_TYPE SymType; 
        WCHAR    ModuleName[32]; 
        WCHAR    ImageName[256]; 
        WCHAR    LoadedImageName[256]; 
    } IMAGEHLP_MODULEW, *PIMAGEHLP_MODULEW;
#endif

typedef struct _IMAGEHLP_MODULEW64 {
    DWORD    SizeOfStruct;
    DWORD64  BaseOfImage;
    DWORD    ImageSize;
    DWORD    TimeDateStamp;
    DWORD    CheckSum;
    DWORD    NumSyms;
    SYM_TYPE SymType;
    WCHAR    ModuleName[32];
    WCHAR    ImageName[256];
    WCHAR    LoadedImageName[256];
    WCHAR    LoadedPdbName[256];
    DWORD    CVSig;
    WCHAR    *CVData[MAX_PATH * 3];
    DWORD    PdbSig;
    GUID     PdbSig70;
    DWORD    PdbAge;
    BOOL     PdbUnmatched;
    BOOL     DbgUnmatched;
    BOOL     LineNumbers;
    BOOL     GlobalSymbols;
    BOOL     TypeInfo;
    BOOL     SourceIndexed;
    BOOL     Publics;
    DWORD    MachineType;
    DWORD    Reserved;
} IMAGEHLP_MODULEW64, *PIMAGEHLP_MODULEW64;

typedef LARGE_INTEGER LUID, *PLUID;

typedef struct _LUID_AND_ATTRIBUTES {
    LUID  Luid;
    DWORD Attributes;
} LUID_AND_ATTRIBUTES, *PLUID_AND_ATTRIBUTES;

typedef struct _TOKEN_PRIVILEGES {
    DWORD               PrivilegeCount;
    LUID_AND_ATTRIBUTES Privileges[ANYSIZE_ARRAY];
} TOKEN_PRIVILEGES, *PTOKEN_PRIVILEGES;

typedef struct tagPIXELFORMATDESCRIPTOR {
    WORD  nSize;
    WORD  nVersion;
    DWORD dwFlags;
    BYTE  iPixelType;
    BYTE  cColorBits;
    BYTE  cRedBits;
    BYTE  cRedShift;
    BYTE  cGreenBits;
    BYTE  cGreenShift;
    BYTE  cBlueBits;
    BYTE  cBlueShift;
    BYTE  cAlphaBits;
    BYTE  cAlphaShift;
    BYTE  cAccumBits;
    BYTE  cAccumRedBits;
    BYTE  cAccumGreenBits;
    BYTE  cAccumBlueBits;
    BYTE  cAccumAlphaBits;
    BYTE  cDepthBits;
    BYTE  cStencilBits;
    BYTE  cAuxBuffers;
    BYTE  iLayerType;
    BYTE  bReserved;
    DWORD dwLayerMask;
    DWORD dwVisibleMask;
    DWORD dwDamageMask;
} PIXELFORMATDESCRIPTOR, *PPIXELFORMATDESCRIPTOR, *LPPIXELFORMATDESCRIPTOR;

typedef struct _INITCOMMONCONTROLSEX {
    DWORD dwSize;
    DWORD dwICC;
} INITCOMMONCONTROLSEX, *LPINITCOMMONCONTROLSEX;

typedef struct WSAData {
    WORD                    wVersion;
    WORD                    wHighVersion;
#if defined(NC_CPU_X86_64)
    unsigned short          iMaxSockets;
    unsigned short          iMaxUdpDg;
    char FAR*               lpVendorInfo;
    char                    szDescription[WSADESCRIPTION_LEN + 1];
    char                    szSystemStatus[WSASYS_STATUS_LEN + 1];
#else
    char                    szDescription[WSADESCRIPTION_LEN+1];
    char                    szSystemStatus[WSASYS_STATUS_LEN+1];
    unsigned short          iMaxSockets;
    unsigned short          iMaxUdpDg;
    char FAR *              lpVendorInfo;
#endif
} WSADATA;
typedef WSADATA FAR *LPWSADATA;

typedef struct _WSAOVERLAPPED {
    DWORD    Internal;
    DWORD    InternalHigh;
    DWORD    Offset;
    DWORD    OffsetHigh;
    WSAEVENT hEvent;
} WSAOVERLAPPED, *LPWSAOVERLAPPED;

typedef struct _ACTCTXW {
    ULONG   cbSize;
    DWORD   dwFlags;
    LPCWSTR lpSource;
    USHORT  wProcessorArchitecture;
    LANGID  wLangId;
    LPCWSTR lpAssemblyDirectory;
    LPCWSTR lpResourceName;
    LPCWSTR lpApplicationName;
    HMODULE hModule;
} ACTCTXW, *PACTCTXW;

typedef const ACTCTXW* PCACTCTXW;

typedef struct RIO_BUFFERID_t* RIO_BUFFERID, **PRIO_BUFFERID;
typedef struct RIO_CQ_t* RIO_CQ, **PRIO_CQ;
typedef struct RIO_RQ_t* RIO_RQ, **PRIO_RQ;

typedef struct _RIO_BUF {
    RIO_BUFFERID BufferId;
    ULONG        Offset;
    ULONG        Length;
} RIO_BUF, *PRIO_BUF;

typedef struct _RIORESULT {
    LONG      Status;
    ULONG     BytesTransferred;
    ULONGLONG SocketContext;
    ULONGLONG RequestContext;
} RIORESULT, *PRIORESULT;

typedef BOOL         (WINAPI* LPFN_RIORECEIVE)               (RIO_RQ SocketQueue, PRIO_BUF pData, ULONG DataBufferCount, DWORD Flags, PVOID RequestContext);
typedef VOID         (WINAPI* LPFN_RIOCLOSECOMPLETIONQUEUE)  (RIO_CQ CQ);
typedef RIO_CQ       (WINAPI* LPFN_RIOCREATECOMPLETIONQUEUE) (DWORD QueueSize, PRIO_NOTIFICATION_COMPLETION NotificationCompletion);
typedef RIO_RQ       (WINAPI* LPFN_RIOCREATEREQUESTQUEUE)    (SOCKET Socket, ULONG MaxOutstandingReceive, ULONG MaxReceiveDataBuffers, ULONG MaxOutstandingSend, ULONG MaxSendDataBuffers, RIO_CQ ReqceiveCQ, RIO_CQ SendCQ, PVOID SocketContext);
typedef ULONG        (WINAPI* LPFN_RIODEQUEUECOMPLETION)     (RIO_CQ CQ, PRIORESULT Array, ULONG ArraySize);
typedef VOID         (WINAPI* LPFN_RIODEREGISTERBUFFER)      (RIO_BUFFERID BufferId);
typedef INT          (WINAPI* LPFN_RIONOTIFY)                (RIO_CQ CQ);
typedef INT          (WINAPI* LPFN_RIORECEIVEEX)             (RIO_RQ SocketQueue, PRIO_BUF pData, ULONG DataBufferCount, PRIO_BUF pLocalAddress, PRIO_BUF pRemoteAddress, PRIO_BUF pControlContext, PRIO_BUF pFlags, DWORD Flags, PVOID RequestContext);
typedef RIO_BUFFERID (WINAPI* LPFN_RIOREGISTERBUFFER)        (PCHAR DataBuffer, DWORD DataLength);
typedef BOOL         (WINAPI* LPFN_RIORESIZECOMPLETIONQUEUE) (RIO_CQ CQ, DWORD QueueSize);
typedef BOOL         (WINAPI* LPFN_RIORESIZEREQUESTQUEUE)    (RIO_RQ RQ, DWORD MaxOutstandingReceive, DWORD MaxOutstandingSend);
typedef BOOL         (WINAPI* LPFN_RIOSEND)                  (RIO_RQ SocketQueue, PRIO_BUF pData, ULONG DataBufferCount, DWORD Flags, PVOID RequestContext);
typedef BOOL         (WINAPI* LPFN_RIOSENDEX)                (RIO_RQ SocketQueue, PRIO_BUF pData, ULONG DataBufferCount, PRIO_BUF pLocalAddress, PRIO_BUF pRemoteAddress, PRIO_BUF pControlContext, PRIO_BUF pFlags, DWORD Flags, PVOID RequestContext);

typedef struct _RIO_EXTENSION_FUNCTION_TABLE {
    DWORD                         cbSize;
    LPFN_RIORECEIVE               RIOReceive;
    LPFN_RIORECEIVEEX             RIOReceiveEx;
    LPFN_RIOSEND                  RIOSend;
    LPFN_RIOSENDEX                RIOSendEx;
    LPFN_RIOCLOSECOMPLETIONQUEUE  RIOCloseCompletionQueue;
    LPFN_RIOCREATECOMPLETIONQUEUE RIOCreateCompletionQueue;
    LPFN_RIOCREATEREQUESTQUEUE    RIOCreateRequestQueue;
    LPFN_RIODEQUEUECOMPLETION     RIODequeueCompletion;
    LPFN_RIODEREGISTERBUFFER      RIODeregisterBuffer;
    LPFN_RIONOTIFY                RIONotify;
    LPFN_RIOREGISTERBUFFER        RIORegisterBuffer;
    LPFN_RIORESIZECOMPLETIONQUEUE RIOResizeCompletionQueue;
    LPFN_RIORESIZEREQUESTQUEUE    RIOResizeRequestQueue;
} RIO_EXTENSION_FUNCTION_TABLE, *PRIO_EXTENSION_FUNCTION_TABLE;

typedef ITEMIDLIST const* PCITEMIDLIST, *LPCITEMIDLIST;
typedef PCITEMIDLIST      PIDLIST_ABSOLUTE, PCIDLIST_ABSOLUTE;
typedef LPITEMIDLIST      PITEMID_CHILD;
typedef LPCITEMIDLIST*    PCUITEMID_CHILD_ARRAY;
typedef struct IBindCtx   IBindCtx;                                                 // NOTE: this works, somehow???

typedef DWORD   (WINAPI* LPTHREAD_START_ROUTINE)(LPVOID lpThreadParameter);
typedef BOOL    (WINAPI* MONITORENUMPROC)(HMONITOR unnamedParam1, HDC unnamedParam2, LPRECT unnamedParam3, LPARAM unnamedParam4);
typedef VOID    (NTAPI*  PIMAGE_TLS_CALLBACK)(PVOID DllHandle, DWORD Reason, PVOID Reserved);
typedef LONG    (WINAPI* LPTOP_LEVEL_EXCEPTION_FILTER)(PEXCEPTION_POINTERS lpExceptionPointers);
typedef VOID    (WINAPI* LPWSAOVERLAPPED_COMPLETION_ROUTINE)(DWORD dwError, DWORD cbTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags);
typedef BOOL    (WINAPI* PREAD_PROCESS_MEMORY_ROUTINE64)(HANDLE hProcess, DWORD64 qwBaseAddress, PVOID lpBuffer, DWORD nSize, LPDWORD lpNumberOfBytesRead);
typedef PVOID   (WINAPI* PFUNCTION_TABLE_ACCESS_ROUTINE64)(HANDLE ahProcess, DWORD64 AddrBase);
typedef DWORD64 (WINAPI* PGET_MODULE_BASE_ROUTINE64)(HANDLE hProcess, DWORD64 Address);
typedef DWORD64 (WINAPI* PTRANSLATE_ADDRESS_ROUTINE64)(HANDLE hProcess, HANDLE hThread, LPADDRESS64 lpaddr);
typedef void    (WINAPI* WINHTTP_STATUS_CALLBACK)(HINTERNET hInternet, DWORD_PTR dwContext, DWORD dwInternetStatus, LPVOID lpvStatusInformation, DWORD dwStatusInformationLength);

// @functions__________________________________________________________________
EXTERN_C_LINK_BEGIN
DLL_IMPORT                    VOID                         WINAPI  GetSystemInfo(LPSYSTEM_INFO lpSystemInfo);
DLL_IMPORT                    LPVOID                       WINAPI  VirtualAlloc(void *lpAddress, sz dwSize, u32 flAllocationType, u32 flProtect);
DLL_IMPORT                    i32                          WINAPI  VirtualFree(void *lpAddress, sz dwSize, u32 dwFreeType);
DLL_IMPORT                    VOID                         WINAPI  GetSystemTime(LPSYSTEMTIME lpSystemTime);
DLL_IMPORT                    VOID                         WINAPI  GetLocalTime(LPSYSTEMTIME lpSystemTime);
DLL_IMPORT                    VOID                         WINAPI  Sleep(DWORD dwMilliseconds);
DLL_IMPORT                    VOID                         WINAPI  InitializeCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
DLL_IMPORT                    VOID                         WINAPI  EnterCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
DLL_IMPORT                    VOID                         WINAPI  LeaveCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
DLL_IMPORT                    VOID                         WINAPI  DeleteCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
DLL_IMPORT                    HANDLE                       WINAPI  CreateFileMappingW(HANDLE hFile, LPSECURITY_ATTRIBUTES lpFileMappingAttributes, DWORD flProtect, DWORD dwMaximumSizeHigh, DWORD dwMaximumSizeLow, LPCWSTR lpName);
DLL_IMPORT                    HANDLE                       WINAPI  OpenFileMappingW(DWORD dwDesiredAccess, BOOL bInheritHandle, LPCWSTR lpName);
DLL_IMPORT                    BOOL                         WINAPI  CloseHandle(HANDLE hObject);
DLL_IMPORT                    LPVOID                       WINAPI  MapViewOfFile(HANDLE hFileMappingObject, DWORD dwDesiredAccess, DWORD dwFileOffsetHigh, DWORD dwFileOffsetLow, SIZE_T dwNumberOfBytesToMap);
DLL_IMPORT                    BOOL                         WINAPI  UnmapViewOfFile(LPCVOID lpBaseAddress);
DLL_IMPORT                    BOOL                         WINAPI  QueryPerformanceFrequency(LARGE_INTEGER* lpFrequency);
DLL_IMPORT                    BOOL                         WINAPI  QueryPerformanceCounter(LARGE_INTEGER* lpPerformanceCount);
DLL_IMPORT                    VOID                         WINAPI  ExitProcess(DWORD dwExitCode);
DLL_IMPORT                    DWORD                        WINAPI  GetCurrentDirectoryW(DWORD dwMaxResultLength, LPCWSTR lpResultBuffer);
DLL_IMPORT                    BOOL                         WINAPI  FileTimeToSystemTime(FILETIME const* lpFileTime, LPSYSTEMTIME lpSystemTime);
DLL_IMPORT                    HANDLE                       WINAPI  CreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
DLL_IMPORT                    BOOL                         WINAPI  GetFileSizeEx(HANDLE hFile, PLARGE_INTEGER lpFileSize);
DLL_IMPORT                    BOOL                         WINAPI  ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberofBytesToRead, LPDWORD lpNumberofBytesRead, LPOVERLAPPED lpOverlapped);
DLL_IMPORT                    BOOL                         WINAPI  WriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped);
DLL_IMPORT                    BOOL                         WINAPI  SystemTimeToFileTime(LPSYSTEMTIME lpSystemTime, LPFILETIME lpFileTime);
DLL_IMPORT                    BOOL                         WINAPI  SetFileTime(HANDLE hFile, FILETIME const *lpCreationTime, FILETIME const *lpLastAccessTime, FILETIME const *lpLastWriteTime);
DLL_IMPORT                    BOOL                         WINAPI  GetFileInformationByHandleEx(HANDLE hFile, FILE_INFO_BY_HANDLE_CLASS FileInformationClass, LPVOID lpFileInformation, DWORD dwBufferSize);
DLL_IMPORT                    BOOL                         WINAPI  GetFileInformationByHandle(HANDLE hFile, LPBY_HANDLE_FILE_INFORMATION lpFileInformation);
DLL_IMPORT                    HANDLE                       WINAPI  FindFirstFileW(LPCWSTR lpFileName, LPWIN32_FIND_DATAW lpFindFileData);
DLL_IMPORT                    DWORD                        WINAPI  GetLogicalDriveStringsW(DWORD nBufferLength, LPWSTR lpBuffer);
DLL_IMPORT                    BOOL                         WINAPI  FindClose(HANDLE hFindFile);
DLL_IMPORT                    BOOL                         WINAPI  SetFileInformationByHandle(HANDLE hFile, FILE_INFO_BY_HANDLE_CLASS FileInformationClass, LPVOID lpFileInformation, DWORD dwBufferSize);
DLL_IMPORT                    BOOL                         WINAPI  DeleteFileW(LPCWSTR lpFileName);
DLL_IMPORT                    DWORD                        WINAPI  GetFullPathNameW(LPCWSTR lpFileName, DWORD nBufferLength, LPWSTR lpBuffer, LPWSTR* lpFilePart);
DLL_IMPORT                    DWORD                        WINAPI  GetFileAttributesW(LPCWSTR lpFileName);
DLL_IMPORT                    BOOL                         WINAPI  GetFileAttributesExW(LPCWSTR lpFileName, GET_FILEEX_INFO_LEVELS fInfoLevelId, LPVOID lpFileInformation);
DLL_IMPORT                    HANDLE                       WINAPI  CreateFileMappingA(HANDLE hFile, LPSECURITY_ATTRIBUTES lpFileMappingAttributes, DWORD flProtect, DWORD dwMaximumSizeHigh, DWORD dwMaximumSizeLow, LPCSTR lpName);
DLL_IMPORT                    HANDLE                       WINAPI  FindFirstFileExW(LPCWSTR lpFileName, FINDEX_INFO_LEVELS fInfoLevelId, LPVOID lpFindFileData, FINDEX_SEARCH_OPS fSearchOp, LPVOID lpSearchFilter, DWORD dwAdditionalFlags);
DLL_IMPORT                    BOOL                         WINAPI  FindNextFileW(HANDLE hFindFile, LPWIN32_FIND_DATAW lpFindFileData);
DLL_IMPORT                    HMODULE                      WINAPI  LoadLibraryW(LPCWSTR lpLibFileName);
DLL_IMPORT                    HMODULE                      WINAPI  LoadLibraryA(LPCSTR lpLibFileName);
DLL_IMPORT                    BOOL                         WINAPI  FreeLibrary(HMODULE hLibModule);
DLL_IMPORT                    FARPROC                      WINAPI  GetProcAddress(HMODULE hModule, LPCSTR lpProcName);
DLL_IMPORT                    BOOL                         WINAPI  CreateDirectoryW(LPCWSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes);
DLL_IMPORT                    BOOL                         WINAPI  CopyFileW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, BOOL bFailIfExists);
DLL_IMPORT                    DWORD                        WINAPI  GetCurrentThreadId(VOID);
DLL_IMPORT                    HANDLE                       WINAPI  GetCurrentThread(VOID);
DLL_IMPORT                    VOID                         WINAPI  RaiseException(DWORD dwExceptionCode, DWORD dwExceptionFlags, DWORD nNumberOfArguments, ULONG_PTR const* lpArguments);
DLL_IMPORT                    HANDLE                       WINAPI  CreateThread(LPSECURITY_ATTRIBUTES lpThreadAttributes, SIZE_T dwStackSize, LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId);
DLL_IMPORT                    DWORD                        WINAPI  WaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds);
DLL_IMPORT                    VOID                         WINAPI  InitializeSRWLock(PSRWLOCK SRWLock);
DLL_IMPORT                    VOID                         WINAPI  AcquireSRWLockShared(PSRWLOCK SRWLock);
DLL_IMPORT                    VOID                         WINAPI  ReleaseSRWLockShared(PSRWLOCK SRWLock);
DLL_IMPORT                    VOID                         WINAPI  AcquireSRWLockExclusive(PSRWLOCK SRWLock);
DLL_IMPORT                    VOID                         WINAPI  ReleaseSRWLockExclusive(PSRWLOCK SRWLock);
DLL_IMPORT                    VOID                         WINAPI  InitializeConditionVariable(PCONDITION_VARIABLE ConditionVariable);
DLL_IMPORT                    BOOL                         WINAPI  SleepConditionVariableCS(PCONDITION_VARIABLE ConditionVariable, PCRITICAL_SECTION CriticalSection, DWORD dwMilliseconds);
DLL_IMPORT                    BOOL                         WINAPI  SleepConditionVariableSRW(PCONDITION_VARIABLE ConditionVariable, PSRWLOCK SRWLock, DWORD dwMilliseconds, ULONG Flags);
DLL_IMPORT                    VOID                         WINAPI  WakeConditionVariable(PCONDITION_VARIABLE ConditionVariable);
DLL_IMPORT                    VOID                         WINAPI  WakeAllConditionVariable(PCONDITION_VARIABLE ConditionVariable);
DLL_IMPORT                    HANDLE                       WINAPI  CreateSemaphoreW(LPSECURITY_ATTRIBUTES lpSemaphoreAttributes, LONG lInitialCount, LONG lMaximumCount, LPCWSTR lpName);
DLL_IMPORT                    HANDLE                       WINAPI  OpenSemaphoreW(DWORD dwDesiredAccess, BOOL bInheritHandle, LPCWSTR lpName);
DLL_IMPORT                    BOOL                         WINAPI  ReleaseSemaphore(HANDLE hSemaphore, LONG lReleaseCount, LPLONG lpPreviousCount);
DLL_IMPORT                    BOOL                         WINAPI  CreateProcessW(LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandle, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation);
DLL_IMPORT                    BOOL                         WINAPI  GetExitCodeProcess(HANDLE hProcess, LPDWORD lpExitCode);
DLL_IMPORT                    BOOL                         WINAPI  TerminateProcess(HANDLE hProcess, UINT uExitCode);
DLL_IMPORT                    INT                          WINAPI  ReleaseDC(HWND hWnd, HDC hDC);
DLL_IMPORT                    BOOL                         WINAPI  DestroyWindow(HWND hWnd);
DLL_IMPORT                    LRESULT                      WINAPI  DefWindowProcW(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
DLL_IMPORT                    HDC                          WINAPI  BeginPaint(HWND hWnd, LPPAINTSTRUCT lpPaint);
DLL_IMPORT                    BOOL                         WINAPI  EndPaint(HWND hWnd, PAINTSTRUCT const* lpPaint);
DLL_IMPORT                    BOOL                         WINAPI  ReleaseCapture(VOID);
DLL_IMPORT                    HWND                         WINAPI  SetCapture(HWND hWnd);
DLL_IMPORT                    BOOL                         WINAPI  ScreenToClient(HWND hWnd, LPPOINT lpPoint);
DLL_IMPORT                    BOOL                         WINAPI  ClientToScreen(HWND hWnd, LPPOINT lpPoint);
DLL_IMPORT                    LONG                         WINAPI  GetWindowLongW(HWND hWnd, int nIndex);
DLL_IMPORT                    HCURSOR                      WINAPI  SetCursor(HCURSOR hCursor);
DLL_IMPORT                    BOOL                         WINAPI  DragQueryPoint(HDROP hDrop, POINT* ppt);
DLL_IMPORT                    UINT                         WINAPI  DragQueryFileW(HDROP hDrop, UINT iFile, LPWSTR lpszFile, UINT cch);
DLL_IMPORT                    UINT                         WINAPI  DragQueryFileA(HDROP hDrop, UINT iFile, LPSTR lpszFile, UINT cch);
DLL_IMPORT                    VOID                         WINAPI  DragFinish(HDROP hDrop);
DLL_IMPORT                    HRESULT                      WINAPI  DwmIsCompositionEnabled(BOOL *pfEnabled);
DLL_IMPORT                    HRESULT                      WINAPI  DwmExtendFrameIntoClientArea(HWND hWnd, MARGINS const* pMarInset);
DLL_IMPORT                    HRESULT                      WINAPI  DwmSetWindowAttribute(HWND hWnd, DWORD dwAttribute, LPCVOID pvAttribute, DWORD cbAttribute);
DLL_IMPORT                    LONG_PTR                     WINAPI  SetWindowLongPtrW(HWND hWnd, int nIndex, LONG_PTR dwNewLong);
DLL_IMPORT                    LONG_PTR                     WINAPI  GetWindowLongPtrW(HWND hWnd, int nIndex);
DLL_IMPORT                    int                          WINAPI  GetSystemMetrics(int nIndex);
DLL_IMPORT                    BOOL                         WINAPI  IsZoomed(HWND hWnd);
DLL_IMPORT                    HMODULE                      WINAPI  GetModuleHandleW(LPCWSTR lpModuleName);
DLL_IMPORT                    HMODULE                      WINAPI  GetModuleHandleA(LPCSTR lpModuleName);
DLL_IMPORT                    BOOL                         WINAPI  SetProcessDPIAware(VOID);
DLL_IMPORT                    HCURSOR                      WINAPI  LoadCursorA(HINSTANCE hInstance, LPCSTR lpCursorName);
DLL_IMPORT                    HCURSOR                      WINAPI  LoadCursorW(HINSTANCE hInstance, LPCWSTR lpCursorName);
DLL_IMPORT                    HICON                        WINAPI  LoadIconA(HINSTANCE hInstance, LPCSTR lpIconName);
DLL_IMPORT                    HICON                        WINAPI  LoadIconW(HINSTANCE hInstance, LPCWSTR lpIconName);
DLL_IMPORT                    ATOM                         WINAPI  RegisterClassExW(WNDCLASSEXW const* unnamedParam1);
DLL_IMPORT                    UINT                         WINAPI  GetDoubleClickTime(VOID);
DLL_IMPORT                    UINT                         WINAPI  GetCaretBlinkTime(VOID);
DLL_IMPORT                    BOOL                         WINAPI  EnumDisplayDevicesW(LPCWSTR lpDevice, DWORD iDevNum, PDISPLAY_DEVICEW lpDisplayDevice, DWORD dwFlags);
DLL_IMPORT                    BOOL                         WINAPI  EnumDisplaySettingsW(LPCWSTR lpszDeviceName, DWORD iModeNum, DEVMODEW *lpDevMode);
DLL_IMPORT                    BOOL                         WINAPI  OpenClipboard(HWND hWndNewOwner);
DLL_IMPORT                    BOOL                         WINAPI  EmptyClipboard(VOID);
DLL_IMPORT DECLSPEC_ALLOCATOR HGLOBAL                      WINAPI  GlobalAlloc(UINT uFlags, SIZE_T dwBytes);
DLL_IMPORT                    LPVOID                       WINAPI  GlobalLock(HGLOBAL hMem);
DLL_IMPORT                    BOOL                         WINAPI  GlobalUnlock(HGLOBAL hMem);
DLL_IMPORT                    HANDLE                       WINAPI  SetClipboardData(UINT uFormat, HANDLE hMem);
DLL_IMPORT                    BOOL                         WINAPI  CloseClipboard(VOID);
DLL_IMPORT                    BOOL                         WINAPI  IsClipboardFormatAvailable(UINT format);
DLL_IMPORT                    HANDLE                       WINAPI  GetClipboardData(UINT uFormat);
DLL_IMPORT                    HWND                         WINAPI  CreateWindowExW(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);
DLL_IMPORT                    VOID                         WINAPI  DragAcceptFiles(HWND hWnd, BOOL fAccept);
DLL_IMPORT                    HDC                          WINAPI  GetDC(HWND hWnd);
DLL_IMPORT                    BOOL                         WINAPI  SetWindowTextW(HWND hWnd, LPCWSTR lpString);
DLL_IMPORT                    BOOL                         WINAPI  ShowWindow(HWND hWnd, int nCmdShow);
DLL_IMPORT                    BOOL                         WINAPI  SetForegroundWindow(HWND hWnd);
DLL_IMPORT                    HWND                         WINAPI  SetFocus(HWND hWnd);
DLL_IMPORT                    HWND                         WINAPI  GetActiveWindow(VOID);
DLL_IMPORT                    BOOL                         WINAPI  GetWindowPlacement(HWND hWnd, WINDOWPLACEMENT* lpwndpl);
DLL_IMPORT                    BOOL                         WINAPI  GetMonitorInfoW(HMONITOR hMonitor, LPMONITORINFO lpmi);
DLL_IMPORT                    HMONITOR                     WINAPI  MonitorFromWindow(HWND hWnd, DWORD dwFlags);
DLL_IMPORT                    LONG                         WINAPI  SetWindowLongW(HWND hWnd, int nIndex, LONG dwNewLong);
DLL_IMPORT                    BOOL                         WINAPI  SetWindowPos(HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags);
DLL_IMPORT                    BOOL                         WINAPI  SetWindowPlacement(HWND hWnd, WINDOWPLACEMENT const* lpwndpl);
DLL_IMPORT                    BOOL                         WINAPI  IsIconic(HWND hWnd);
DLL_IMPORT                    BOOL                         WINAPI  BringWindowToTop(HWND hWnd);
DLL_IMPORT                    BOOL                         WINAPI  GetWindowRect(HWND hWnd, LPRECT lpRect);
DLL_IMPORT                    BOOL                         WINAPI  GetClientRect(HWND hWnd, LPRECT lpRect);
DLL_IMPORT                    BOOL                         WINAPI  EnumDisplayMonitors(HDC hdc, LPRECT lprcClip, MONITORENUMPROC lpfnEnum, LPARAM dwData);
DLL_IMPORT                    HMONITOR                     WINAPI  MonitorFromPoint(POINT pt, DWORD dwFlags);
DLL_IMPORT                    BOOL                         WINAPI  PostThreadMessageA(DWORD idThread, UINT Msg, WPARAM wParam, LPARAM lParam);
DLL_IMPORT                    BOOL                         WINAPI  GetMessageW(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax);
DLL_IMPORT                    BOOL                         WINAPI  GetMessageA(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax);
DLL_IMPORT                    BOOL                         WINAPI  PeekMessageA(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg);
DLL_IMPORT                    LRESULT                      WINAPI  DispatchMessageW(MSG const* lpMsg);
DLL_IMPORT                    LRESULT                      WINAPI  DispatchMessageA(MSG const* lpMsg);
DLL_IMPORT                    BOOL                         WINAPI  TranslateMessage(MSG const* lpMsg);
DLL_IMPORT                    SHORT                        WINAPI  GetKeyState(int nVirtKey);
DLL_IMPORT                    BOOL                         WINAPI  GetCursorPos(LPPOINT lpPoint);
DLL_IMPORT                    BOOL                         WINAPI  PostMessageW(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
DLL_IMPORT                    BOOL                         WINAPI  SetCursorPos(int X, int Y);
DLL_IMPORT                    INT                          WINAPI  MessageBoxW(HWND hWnd, LPCWSTR lptext, LPCWSTR lpCaption, UINT uType);
DLL_IMPORT                    BOOL                         WINAPI  GetOpenFileNameW(LPOPENFILENAMEW unnamedParam1);
DLL_IMPORT                    VOID                         WINAPI  CoTaskMemFree(LPVOID pv);
DLL_IMPORT                    HINSTANCE                    WINAPI  ShellExecuteW(HWND hWnd, LPCWSTR lpOperation, LPCWSTR lpFile, LPCWSTR lpParameters, LPCWSTR lpDirectory, INT nShowCmd);
SHSTDAPI                                                           SHParseDisplayName(PCWSTR pszName, IBindCtx* pbc, PIDLIST_ABSOLUTE* ppidl, SFGAOF sfgaoIn, SFGAOF* psfgaoOut);
SHSTDAPI                                                           SHOpenFolderAndSelectItems(PCIDLIST_ABSOLUTE pidlFolder, UINT cidl, PCUITEMID_CHILD_ARRAY apidl, DWORD dwFlags);
DLL_IMPORT                    SIZE_T                       WINAPI  GetLargePageMinimum(VOID);
DLL_IMPORT                    SIZE_T                       WINAPI  GetCurrentProcessId(VOID);
DLL_IMPORT                    BOOL                         WINAPI  GetComputerNameA(LPSTR lpBuffer, LPDWORD nSize);
DLL_IMPORT                    DWORD                        WINAPI  GetModuleFileNameW(HMODULE hModule, LPWSTR lpFilename, DWORD nSize);
SHFOLDERAPI                                                        SHGetFolderPathW(HWND hwnd, int csidl, HANDLE hToken, DWORD dwFlags, LPWSTR pszPath);
DLL_IMPORT                    LPWCH                        WINAPI  GetEnvironmentStringsW(VOID);
DLL_IMPORT                    HANDLE                       WINAPI  GetStdHandle(DWORD nStdHandle);
DLL_IMPORT                    DWORD                        WINAPI  TlsAlloc(VOID);
DLL_IMPORT                    BOOL                         WINAPI  TlsFree(DWORD dwTlsIndex);
DLL_IMPORT                    LPVOID                       WINAPI  TlsGetValue(DWORD dwTlsIndex);
DLL_IMPORT                    BOOL                         WINAPI  TlsSetValue(DWORD dwTlsIndex, LPVOID lpTlsValue);
DLL_IMPORT DECLSPEC_ALLOCATOR HLOCAL                       WINAPI  LocalAlloc(UINT uFlags, SIZE_T uBytes);
DLL_IMPORT                    HLOCAL                       WINAPI  LocalFree(HLOCAL hMem);
DLL_IMPORT                    LPWSTR                       WINAPI  GetCommandLineW(VOID);
DLL_IMPORT                    LPWSTR*                      WINAPI  CommandLineToArgvW(LPCWSTR lpCmdLine, int* pNumArgs);
DLL_IMPORT                    VOID                         WINAPI  OutputDebugStringW(LPCWSTR lpOutputString);
DLL_IMPORT                    BOOL                         WINAPI  AllocConsole(VOID);
DLL_IMPORT                    BOOL                         WINAPI  AttachConsole(DWORD dwProcessId);
DLL_IMPORT                    HANDLE                       WINAPI  GetCurrentProcess(VOID);
DLL_IMPORT                    BOOL                         WINAPI  PathAppendW(LPWSTR pszPath, LPCWSTR pszMore);
DLL_IMPORT                    INT                          WINAPIV wsprintfW(LPWSTR unnamedParam1, LPCWSTR unnamedParam2, ...);
DLL_IMPORT                    BOOL                         WINAPI  MiniDumpWriteDump(HANDLE hProcess, DWORD ProcessId, HANDLE hFile, MINIDUMP_TYPE DumpType, PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam, PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam, PMINIDUMP_CALLBACK_INFORMATION CallbackParam);
DLL_IMPORT                    BOOL                         WINAPI  OpenProcessToken(HANDLE hProcess, DWORD dwDesiredAccess, PHANDLE phToken);
DLL_IMPORT                    BOOL                         WINAPI  LookupPrivilegeValueW(LPCWSTR lpSystemName, LPCWSTR lpName, PLUID lpLuid);
DLL_IMPORT                    BOOL                         WINAPI  AdjustTokenPrivileges(HANDLE TokenHandle, BOOL DisableAllPrivileges, PTOKEN_PRIVILEGES NewState, DWORD BufferLength, PTOKEN_PRIVILEGES PreviousState, PDWORD ReturnLength);
DLL_IMPORT                    LPTOP_LEVEL_EXCEPTION_FILTER WINAPI  SetUnhandledExceptionFilter(LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter);
DLL_IMPORT                    INT                          WINAPI  WSAStartup(WORD wVersionRequired, LPWSADATA lpWSAData);
DLL_IMPORT                    SOCKET                       WINAPI  socket(INT af, INT type, INT protocol);
DLL_IMPORT                    INT                          WSAAPI  WSAIoctl(SOCKET s, DWORD dwIoControlCode, LPVOID lpvInBuffer, DWORD chInBuffer, LPVOID lpvOutBuffer, DWORD cbOutBuffer, LPDWORD lpcbBytesReturned, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);
DLL_IMPORT                    INT                          WSAAPI  closesocket(SOCKET s);
DLL_IMPORT                    INT                          WINAPI  ChoosePixelFormat(HDC hdc, PIXELFORMATDESCRIPTOR const* ppfd);
DLL_IMPORT                    BOOL                         WINAPI  SetPixelFormat(HDC hdc, INT format, PIXELFORMATDESCRIPTOR const* ppfd);
DLL_IMPORT                    HGLRC                        WINAPI  wglCreateContext(HDC unnamedParam1);
DLL_IMPORT                    BOOL                         WINAPI  wglMakeCurrent(HDC unnamedParam1, HGLRC unnamedParam2);
DLL_IMPORT                    BOOL                         WINAPI  SwapBuffers(HDC unnamedParam1);
DLL_IMPORT                    INT                          WINAPI  ShowCursor(BOOL bShow);
DLL_IMPORT                    BOOL                         WINAPI  SetStdHandle(DWORD nStdHandle, HANDLE hHandle);
DLL_IMPORT                    BOOL                         WINAPI  PathRemoveFileSpecW(LPWSTR pszPath);
DLL_IMPORT                    LPCWSTR                      WINAPI  PathFindFileNameW(LPCWSTR pszPath);
DLL_IMPORT                    LPCSTR                       WINAPI  PathFindFileNameA(LPCSTR pszPath);
DLL_IMPORT                    HRESULT                      WINAPI  TaskDialogIndirect(TASKDIALOGCONFIG const* pTaskConfig, int* pnButton, int* pnRadioButton, BOOL* pfVerificationFlagChecked);
DLL_IMPORT                    INT                          WINAPI  wnsprintfW(PWSTR pszDest, int cchDest, PCWSTR pszFmt, ...);
DLL_IMPORT                    INT                          WINAPI  wnsprintfA(PSTR pszDest, int cchDest, PCSTR pszFmt, ...);
DLL_IMPORT                    BOOL                         WINAPI  ActivateActCtx(HANDLE hActCtx, ULONG_PTR* lpCookie);
DLL_IMPORT                    HANDLE                       WINAPI  CreateActCtxW(PCACTCTXW pActCtx);
DLL_IMPORT                    VOID                         WINAPI  ReleaseActCtx(HANDLE hActCtx);
DLL_IMPORT                    DWORD                        WINAPI  GetLastError();
DLL_IMPORT                    BOOL                         WINAPI  InitCommonControlsEx(INITCOMMONCONTROLSEX const* pice);
DLL_IMPORT                    BOOL                         WINAPI  FreeConsole(VOID);
DLL_IMPORT                    VOID                         WINAPI  GetSystemTimeAsFileTime(LPFILETIME lpSystemTimeAsFileTime);
DLL_IMPORT                    BOOL                         WINAPI  SetConsoleMode(HANDLE hConsoleHandle, DWORD dwMode);
DLL_IMPORT                    BOOL                         WINAPI  GetConsoleMode(HANDLE hConsoleHandle, LPDWORD lpMode);
DLL_IMPORT                    HANDLE                       WINAPI  GetProcessHeap(VOID);
DLL_IMPORT DECLSPEC_ALLOCATOR LPVOID                       WINAPI  HeapAlloc(HANDLE hHeap, DWORD dwFlags, SIZE_T dwBytes);
DLL_IMPORT                    BOOL                         WINAPI  HeapFree(HANDLE hHeap, DWORD dwFlags, LPVOID lpMem);
DLL_IMPORT                    DWORD                        WINAPI  CommDlgExtendedError(VOID);
DLL_IMPORT                    BOOL                         WINAPI  InitializeSynchronizationBarrier(LPSYNCHRONIZATION_BARRIER lpBarrier, LONG lTotalThreads, LONG lSpinCount);
DLL_IMPORT                    BOOL                         WINAPI  EnterSynchronizationBarrier(LPSYNCHRONIZATION_BARRIER lpBarrier, DWORD dwFlags);
DLL_IMPORT                    BOOL                         WINAPI  DeleteSynchronizationBarrier(LPSYNCHRONIZATION_BARRIER lpBarrier);
DLL_IMPORT                    UINT                         WINAPI  GetWindowsDirectoryA(LPSTR lpBuffer, UINT uSize);
DLL_IMPORT                    LSTATUS                      WINAPI  RegOpenKeyExW(HKEY hKey, LPCWSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult);
DLL_IMPORT                    LSTATUS                      WINAPI  RegEnumValueA(HKEY hKey, DWORD dwIndex, LPSTR lpValueName, LPDWORD lpcchValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData);
DLL_IMPORT                    HGDIOBJ                      WINAPI  GetStockObject(int i);
DLL_IMPORT                    HGDIOBJ                      WINAPI  SelectObject(HDC hdc, HGDIOBJ h);
DLL_IMPORT                    COLORREF                     WINAPI  SetDCPenColor(HDC hdc, COLORREF color);
DLL_IMPORT                    COLORREF                     WINAPI  SetDCBrushColor(HDC hdc, COLORREF color);
DLL_IMPORT                    BOOL                         WINAPI  Rectangle(HDC hdc, INT left, INT top, INT right, INT bottom);
DLL_IMPORT                    HGDIOBJ                      WINAPI  GetCurrentObject(HDC hdc, UINT type);
DLL_IMPORT                    INT                          WINAPI  GetObjectA(HANDLE h, INT c, LPVOID pv);
DLL_IMPORT                    INT                          WINAPI  GetObjectW(HANDLE h, INT c, LPVOID pv);
DLL_IMPORT                    BOOL                         WINAPI  SetProcessWorkingSetSize(HANDLE hProcess, SIZE_T dwMinimumWorkingSetSize, SIZE_T dwMaximumWorkingSetSize);
DLL_IMPORT                    INT                          WINAPI  GetDIBits(HDC hdc, HBITMAP hbm, UINT start, UINT cLines, LPVOID lpvBits, LPBITMAPINFO lpbmi, UINT usage);
DLL_IMPORT                    BOOL                         WINAPI  SetConsoleOutputCP(UINT wCodePageID);
DLL_IMPORT                    BOOL                         WINAPI  SetConsoleCP(UINT wCodePageID);
DLL_IMPORT                    BOOL                         WINAPI  ReadConsoleA(HANDLE hConsoleInput, LPVOID lpBuffer, DWORD nNumberofCharsToRead, LPDWORD lpNumberofCharsRead, LPVOID pInputControl);
DLL_IMPORT                    VOID                         WSAAPI  freeaddrinfo(PADDRINFOA pAddrInfo);
DLL_IMPORT                    INT                          WSAAPI  getaddrinfo(PCSTR pNodeName, PCSTR pServiceName, ADDRINFOA const* pHints, PADDRINFOA* ppResult);
DLL_IMPORT                    INT                          WSAAPI  getaddrinfoW(PCWSTR pNodeName, PCWSTR pServiceName, ADDRINFOW const* pHints, PADDRINFOW* ppResult);
DLL_IMPORT                    INT                          WSAAPI  recvfrom(SOCKET s, CHAR* buf, INT len, INT flags, SOCKADDR* from, INT* fromlen);
DLL_IMPORT                    INT                          WSAAPI  recv(SOCKET s, CHAR* buf, INT len, INT flags);
DLL_IMPORT                    INT                          WSAAPI  sendto(SOCKET s, CHAR const* buf, INT len, INT flags, SOCKADDR const* to, INT tolen);
DLL_IMPORT                    INT                          WSAAPI  send(SOCKET s, CHAR const* buf, INT len, INT flags);
DLL_IMPORT                    INT                          WSAAPI  shutdown(SOCKET s, INT how);
DLL_IMPORT                    INT                          WSAAPI  connect(SOCKET s, SOCKADDR const* name, INT namelen);
DLL_IMPORT                    SOCKET                       WSAAPI  accept(SOCKET s, SOCKADDR* addr, INT* addrlen);
DLL_IMPORT                    INT                          WSAAPI  listen(SOCKET s, INT backlog);
DLL_IMPORT                    INT                          WSAAPI  bind(SOCKET s, SOCKADDR const* name, INT namelen);
DLL_IMPORT                    INT                          WSAAPI  setsockopt(SOCKET s, INT level, INT optname, CHAR const* optval, INT optlen);
DLL_IMPORT                    USHORT                       WSAAPI  htons(USHORT hostshort);
DLL_IMPORT                    USHORT                       WSAAPI  ntohs(USHORT networkshort);
DLL_IMPORT                    INT                          WSAAPI  WSACleanup(VOID);
DLL_IMPORT                    INT                          WSAAPI  ioctlsocket(SOCKET s, LONG cmd, ULONG* argp);
DLL_IMPORT                    BOOL                         WINAPI  IsDebuggerPresent(VOID);
DLL_IMPORT                    SECURITY_STATUS              WINAPI  AcquireCredentialsHandleW(LPWSTR pszPrincipal, LPWSTR pszPackage, ULONG fCredentialUse, void* pvLoginID, void* pAuthData, void* pGetKeyFn, void* pvGetKeyArgument, CredHandle* phCredential, void* ptsExpiry);
DLL_IMPORT                    SECURITY_STATUS              WINAPI  InitializeSecurityContextW(CredHandle* phCredential, CtxHandle* phContext, LPWSTR pszTargetName, ULONG fContextReq, ULONG Reserved1, ULONG TargetDataRep, SecBufferDesc* pInput, ULONG Reserved2, CtxHandle* phNewContext, SecBufferDesc* pOutput, ULONG* pfContextAttr, void* ptsExpiry);
DLL_IMPORT                    SECURITY_STATUS              WINAPI  AcceptSecurityContext(CredHandle* phCredential, CtxHandle* phContext, SecBufferDesc* pInput, ULONG fContextReq, ULONG TargetDataRep, CtxHandle* phNewContext, SecBufferDesc* pOutput, ULONG* pfContextAttr, void* ptsExpiry);
DLL_IMPORT                    SECURITY_STATUS              WINAPI  FreeCredentialsHandle(CredHandle* phCredential);
DLL_IMPORT                    SECURITY_STATUS              WINAPI  DeleteSecurityContext(CtxHandle* phContext);
DLL_IMPORT                    SECURITY_STATUS              WINAPI  FreeContextBuffer(void* pvContextBuffer);
DLL_IMPORT                    SECURITY_STATUS              WINAPI  ApplyControlToken(CtxHandle* phContext, SecBufferDesc* pInput);
DLL_IMPORT                    SECURITY_STATUS              WINAPI  QueryContextAttributesW(CtxHandle* phContext, ULONG ulAttribute, void* pBuffer);
DLL_IMPORT                    SECURITY_STATUS              WINAPI  QueryContextAttributesA(CtxHandle* phContext, ULONG ulAttribute, void* pBuffer);
DLL_IMPORT                    SECURITY_STATUS              WINAPI  EncryptMessage(CtxHandle* phContext, ULONG fQOP, SecBufferDesc* pMessage, ULONG MessageSeqNo);
DLL_IMPORT                    SECURITY_STATUS              WINAPI  DecryptMessage(CtxHandle* phContext, SecBufferDesc* pMessage, ULONG MessageSeqNo, ULONG* fQOP);
DLL_IMPORT                    HCERTSTORE                   WINAPI  PFXImportCertStore(CRYPT_DATA_BLOB* pPFX, LPCWSTR szPassword, DWORD dwFlags);
DLL_IMPORT                    HCERTSTORE                   WINAPI  CertOpenStore(LPCSTR lpszStoreProvider, DWORD dwEncodingType, HCRYPTPROV hCryptProv, DWORD dwFlags, void const* pvPara);
DLL_IMPORT                    BOOL                         WINAPI  CertCloseStore(HCERTSTORE hCertStore, DWORD dwFlags);
DLL_IMPORT                    PCCERT_CONTEXT               WINAPI  CertFindCertificateInStore(HCERTSTORE hCertStore, DWORD dwCertEncodingType, DWORD dwFindFlags, DWORD dwFindType, void const* pvFindPara, PCCERT_CONTEXT pPrevCertContext);
DLL_IMPORT                    PCCERT_CONTEXT               WINAPI  CertEnumCertificatesInStore(HCERTSTORE hCertStore, PCCERT_CONTEXT pPrevCertContext);
DLL_IMPORT                    BOOL                         WINAPI  CertAddEncodedCertificateToStore(HCERTSTORE hCertStore, DWORD dwCertEncodingType, BYTE const* pbCertEncoded, DWORD cbCertEncoded, DWORD dwAddDisposition, PCCERT_CONTEXT* ppCertContext);
DLL_IMPORT                    BOOL                         WINAPI  CertAddCertificateContextToStore(HCERTSTORE hCertStore, PCCERT_CONTEXT pCertContext, DWORD dwAddDisposition, PCCERT_CONTEXT* ppStoreContext);
DLL_IMPORT                    BOOL                         WINAPI  CertAddEncodedCRLToStore(HCERTSTORE hCertStore, DWORD dwCertEncodingType, BYTE const* pbCrlEncoded, DWORD cbCrlEncoded, DWORD dwAddDisposition, void** ppCrlContext);
DLL_IMPORT                    BOOL                         WINAPI  CertFreeCertificateContext(PCCERT_CONTEXT pCertContext);
DLL_IMPORT                    DWORD                        WINAPI  CertGetNameStringA(PCCERT_CONTEXT pCertContext, DWORD dwType, DWORD dwFlags, void* pvTypePara, LPSTR pszNameString, DWORD cchNameString);
DLL_IMPORT                    BOOL                         WINAPI  CryptHashCertificate2(LPCWSTR pwszCNGHashAlgid, DWORD dwFlags, void* pvReserved, BYTE const* pbEncoded, DWORD cbEncoded, BYTE* pbComputedHash, DWORD* pcbComputedHash);
DLL_IMPORT                    HINTERNET                            WinHttpOpen(LPCWSTR pszAgentW, DWORD dwAccessType, LPCWSTR pszProxyW, LPCWSTR pszProxyBypassW, DWORD dwFlags);
DLL_IMPORT                    WINHTTP_STATUS_CALLBACK              WinHttpSetStatusCallback(HINTERNET hInternet, WINHTTP_STATUS_CALLBACK lpfnInternetCallback, DWORD dwNotificationFlags, DWORD_PTR dwReserved);
DLL_IMPORT                    HINTERNET                            WinHttpConnect(HINTERNET hSession, LPCWSTR pswzServerName, INTERNET_PORT nServerPort, DWORD dwReserved);
DLL_IMPORT                    HINTERNET                            WinHttpOpenRequest(HINTERNET hConnect, LPCWSTR pwszVerb, LPCWSTR pwszObjectName, LPCWSTR pwszVersion, LPCWSTR pwszReferrer, LPCWSTR* ppwszAcceptTypes, DWORD dwFlags);
DLL_IMPORT                    BOOL                                 WinHttpSendRequest(HINTERNET hRequest, LPCWSTR lpszHeaders, DWORD dwHeadersLength, LPVOID lpOptional, DWORD dwOptionalLength, DWORD dwTotalLength, DWORD_PTR dwContext);
DLL_IMPORT                    BOOL                                 WinHttpReceiveResponse(HINTERNET hRequest, LPVOID lpReserved);
DLL_IMPORT                    BOOL                                 WinHttpQueryHeaders(HINTERNET hRequest, DWORD dwInfoLevel, LPCWSTR pwszName, LPVOID lpBuffer, LPDWORD lpdwBufferLength, LPDWORD lpdwIndex);
DLL_IMPORT                    BOOL                                 WinHttpQueryDataAvailable(HINTERNET hRequest, LPDWORD lpdwNumberOfBytesAvailable);
DLL_IMPORT                    BOOL                                 WinHttpReadData(HINTERNET hRequest, LPVOID lpBuffer, DWORD dwNumberOfBytesToRead, LPDWORD lpdwNumberOfBytesRead);
DLL_IMPORT                    BOOL                                 WinHttpCloseHandle(HINTERNET hInternet);
DLL_IMPORT                    MMRESULT                     WINAPI  timeBeginPeriod(UINT uPeriod);
DLL_IMPORT                    MMRESULT                     WINAPI  timeEndPeriod(UINT uPeriod);
EXTERN_C_LINK_END

// @runtime____________________________________________________________________
#if defined(UNICODE) || defined(_UNICODE)
    #define TEXT(X) (LPCWSTR) L##X
    #define _T(X)   (LPCWSTR) L##X
    #define _tcslen wcslen

    #define CreateFileMapping      CreateFileMappingW
    #define GetWindowLong          GetWindowLongW
    #define DragQueryFile          DragQueryFileW
    #define GetModuleHandle        GetModuleHandleW
    #define LoadCursor             LoadCursorW
    #define LoadIcon               LoadIconW
    #define GetMonitorInfo         GetMonitorInfoW
    #define SetWindowLong          SetWindowLongW
    #define PostMessage            PostMessageW
    #define DispatchMessage        DispatchMessageW
    #define GetMessage             GetMessageW
    #define LookupPrivilegeValue   LookupPrivilegeValueW
    #define MAKEINTRESOURCE        MAKEINTRESOURCEW
    #define CreateFile             CreateFileW
    #define GetObject              GetObjectW
    #define QueryContextAttributes QueryContextAttributesW
#else
    #define TEXT(X) (LPCSTR) X
    #define _T(X)   (LPCSTR) X
    #define _tcslen strlen

    #define CreateFileMapping    CreateFileMappingA
    #define DragQueryFile        DragQueryFileA
    #define GetModuleHandle      GetModuleHandleA
    #define LoadCursor           LoadCursorA
    #define LoadIcon             LoadIconA
    #define DispatchMessage      DispatchMessageA
    #define GetMessage           GetMessageA
    #define MAKEINTRESOURCE      MAKEINTRESOURCEA
    #define CreateFile           CreateFileA
    #define GetObject            GetObjectA
    #define QueryContextAttributes QueryContextAttributesA
#endif

#define IsMaximized IsZoomed

constexpr GUID WSAID_MULTIPLE_RIO = {
    0x8509e081,
    0x96dd,
    0x4005,
    {
        0xb1, 
        0x65, 
        0x9e, 
        0x2e, 
        0xe8, 
        0xc7, 
        0x9e, 
        0x3f
    }
};

typedef HRESULT WinSetThreadDescriptionFuncType(HANDLE hThread, PCWSTR lpThreadDescription);

global WinSetThreadDescriptionFuncType* WinSetThreadDescriptionFunc = NULL;
global RIO_EXTENSION_FUNCTION_TABLE WinRIOFunctions = {};

// @c_runtime_definitions______________________________________________________
EXTERN_C_LINK_BEGIN
extern VOID NTAPI tls_callback(HMODULE hModule, DWORD dwReason, LPVOID dwReserved);

#pragma section(".tls$XAA", read, write)
#pragma section(".tls$XCC", read, write)
#pragma section(".tls$XZZ", read, write)

ULONG _tls_index = 0;
int _fltused = 0x9875;           // NOTE(nathan): linker needs this to be happy as long as we also supply ftol2 and ftol2_sse (see below)

#if defined(NC_CPU_X86_32)
    NAKED void _ftol2()
    {
        __asm {
            fistp qword ptr             ; convert f32 to i64
            mov edx,                    ; return 32 high bits in edx
            mov eax,                    ; return 32 low bits in eax
            ret
        }
    }

    NAKED void _ftol2_sse()
    {
        __asm {
            fistp dword ptr
            mov eax,
            ret
        }
    }
#endif

#pragma data_seg(".tls$XCC")
PIMAGE_TLS_CALLBACK p_thread_callback[] = {tls_callback, NULL};

#pragma data_seg(".tls$XAA")
__declspec(allocate(".tls$XAA")) ULONG _tls_start = 0;

#pragma data_seg(".tls$XZZ")
__declspec(allocate(".tls$XZZ")) ULONG _tls_end = 0;
#pragma data_seg()

#pragma const_seg(".rdata$T")
IMAGE_TLS_DIRECTORY64 _tls_used = {
    (ULONG_PTR) &_tls_start,
    (ULONG_PTR) &_tls_end,
    (ULONG_PTR) &_tls_index,
    (ULONG_PTR) &p_thread_callback,
    0,
    0
};
#pragma const_seg()

typedef void (*_PVFV)(void);
typedef int (*_PIFV)(void);

#pragma section(".CRT$XIA",long,read)
__declspec(allocate(".CRT$XIA")) _PIFV __xi_a[] = { 0 };

#pragma section(".CRT$XIZ",long,read)
__declspec(allocate(".CRT$XIZ")) _PIFV __xi_z[] = { 0 };

#pragma section(".CRT$XCA",long,read)
__declspec(allocate(".CRT$XCA")) _PVFV __xc_a[] = { 0 };

#pragma section(".CRT$XCZ",long,read)
__declspec(allocate(".CRT$XCZ")) _PVFV __xc_z[] = { 0 };

#pragma section(".CRT$XPA",long,read)
__declspec(allocate(".CRT$XPA")) _PVFV __xp_a[] = { 0 };

#pragma section(".CRT$XPZ",long,read)
__declspec(allocate(".CRT$XPZ")) _PVFV __xp_z[] = { 0 };

#pragma section(".CRT$XTA",long,read)
__declspec(allocate(".CRT$XTA")) _PVFV __xt_a[] = { 0 };

#pragma section(".CRT$XTZ",long,read)
__declspec(allocate(".CRT$XTZ")) _PVFV __xt_z[] = { 0 };

EXTERN_C_LINK_END

internal void
__WIN32CRTCall(_PIFV* _A, _PIFV* _B)
{
    while (_A != _B) {
        if (*_A) {
            if ((**_A)()) {
                if (IsDebuggerPresent())
                    __debugbreak();

                ExitProcess(0);
            }
        }

        ++_A;
    }
}

internal void
__WIN32CRTCall(_PVFV* _A, _PVFV* _B)
{
    while (_A != _B) {
        if (*_A)
            (**_A)();

        ++_A;
    }
}

internal void
__WIN32CRTInit(void)
{
    __WIN32CRTCall(__xi_a, __xi_z);
    __WIN32CRTCall(__xc_a, __xc_z);
}

internal void
__WIN32CRTDone(void)
{
    __WIN32CRTCall(__xp_a, __xp_z);
    __WIN32CRTCall(__xt_a, __xt_z);
}

#endif // __WIN32_PLATFORM_H__
