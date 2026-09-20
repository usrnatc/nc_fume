#if !defined(__LINUX_PLATFORM_H__)
#define __LINUX_PLATFORM_H__

#include "../nc_types.h"

#include <dirent.h>
#include <dlfcn.h>
#include <dlfcn.h>
#include <errno.h>
#include <execinfo.h>
#include <fcntl.h>
#include <features.h>
#include <linux/limits.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <spawn.h>
#include <stdio.h>
#include <linux/limits.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/random.h>
#include <sys/resource.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <time.h>
#include <unistd.h>

// // @defines____________________________________________________________________
// #if defined(NC_CPU_X86_64)
//     #define SYSCALL_READ           0x000   // (char __user* buf, size_t count)
//     #define SYSCALL_WRITE          0x001   // (const char __user* buf, size_t count)
//     #define SYSCALL_OPEN           0x002   // (int flags, umode_t mode)
//     #define SYSCALL_CLOSE          0x003   // (void)
//     #define SYSCALL_STAT           0x004
//     #define SYSCALL_FSTAT          0x005
//     #define SYSCALL_LSEEK          0x008
//     #define SYSCALL_MMAP           0x009   // (unsigned long len, int prot, int flags, int fd, long off)
//     #define SYSCALL_IOCTL          0x010
//     #define SYSCALL_MPROTECT       0x00A   // (size_t len, unsigned long prot)
//     #define SYSCALL_MUNMAP         0x00B   // (size_t len)
//     #define SYSCALL_PREAD64        0x010   // (char __user* buf, size_t count, loff_t pos)
//     #define SYSCALL_PWRITE64       0x011   // (const char __user* buf, size_t count, loff_t pos)
//     #define SYSCALL_SCHED_YIELD    0x017   // (void)
//     #define SYSCALL_MREMAP         0x018   // (unsigned long old_len, unsigned long new_len, unsigned long flags, unsigned long new_addr)
//     #define SYSCALL_MSYNC          0x019   // (size_t len, int flags)
//     #define SYSCALL_MADVISE        0x01B   // (size_t len, int behaviour)
//     #define SYSCALL_NANOSLEEP      0x022   // (struct timespec __user* rmtp)
//     #define SYSCALL_GETPID         0x026   // (void)
//     #define SYSCALL_CLONE          0x038
//     #define SYSCALL_FORK           0x039
//     #define SYSCALL_EXECVE         0x03B
//     #define SYSCALL_EXIT           0x03C
//     #define SYSCALL_WAIT4          0x03D
//     #define SYSCALL_KILL           0x03E
//     #define SYSCALL_FCNTL          0x048
//     #define SYSCALL_FTRUNCATE      0x04D
//     #define SYSCALL_GETDENTS64     0x0D9
//     #define SYSCALL_GETCWD         0x04F
//     #define SYSCALL_CHDIR          0x050
//     #define SYSCALL_MKDIR          0x053
//     #define SYSCALL_RMDIR          0x054
//     #define SYSCALL_UNLINK         0x057
//     #define SYSCALL_RENAME         0x052
//     #define SYSCALL_CLOCK_GETTIME  0x0E4
//     #define SYSCALL_EXIT_GROUP     0x0E7
//     #define SYSCALL_FUTEX          0x0CA
//     #define SYSCALL_OPENAT         0x101
//     #define SYSCALL_NEWFSTATAT     0x106
//     #define SYSCALL_GETRANDOM      0x13E
//     #define SYSCALL_RT_SIGACTION   0x00D
//     #define SYSCALL_RT_SIGPROCMASK 0x00E
//     #define SYSCALL_RT_SIGRETURN   0x00F
// #elif defined(NC_CPU_X86_32)
//     #error "no support for 32-bit linux"
// #endif

// #define SIGHUP     1
// #define SIGINT     2
// #define SIGQUIT    3
// #define SIGILL     4
// #define SIGTRAP    5
// #define SIGABRT    6
// #define SIGBUS     7
// #define SIGFPE     8
// #define SIGKILL    9
// #define SIGUSR1   10
// #define SIGSEGV   11
// #define SIGUSR2   12
// #define SIGPIPE   13
// #define SIGALRM   14
// #define SIGTERM   15
// #define SIGCHLD   17
// #define SIGCONT   18
// #define SIGSTOP   19
// #define SIGTSTP   20
// #define SIGTTIN   21
// #define SIGTTOU   22
// #define SIGURG    23
// #define SIGXCPU   24
// #define SIGXFSZ   25
// #define SIGVTALRM 26
// #define SIGPROF   27
// #define SIGWINCH  28
// #define SIGIO     29
// #define SIGPWR    30
// #define SIGSYS    31

// #define SIG_BLOCK   0
// #define SIG_UNBLOCK 1
// #define SIG_SETMASK 2

// #define SA_NOCLDSTOP  0x00000001
// #define SA_NOCLDWAIT  0x00000002
// #define SA_SIGINFO    0x00000004
// #define SA_RESTORER   0x04000000
// #define SA_ONSTACK    0x08000000
// #define SA_RESTART    0x10000000
// #define SA_NODEFER    0x40000000
// #define SA_RESETHAND  0x80000000

// #define PROT_NONE   0x0
// #define PROT_READ   0x1
// #define PROT_WRITE  0x2
// #define PROT_EXEC   0x4

// #define MAP_SHARED     0x01
// #define MAP_PRIVATE    0x02
// #define MAP_FIXED      0x10
// #define MAP_ANONYMOUS  0x20
// #define MAP_NORESERVE  0x4000
// #define MAP_POPULATE   0x8000

// #define MREMAP_MAYMOVE  1
// #define MREMAP_FIXED    2

// #define O_RDONLY    0x0000
// #define O_WRONLY    0x0001
// #define O_RDWR      0x0002
// #define O_CREAT     0x0040
// #define O_EXCL      0x0080
// #define O_TRUNC     0x0200
// #define O_APPEND    0x0400
// #define O_NONBLOCK  0x0800
// #define O_DIRECTORY 0x10000
// #define O_CLOEXEC   0x80000

// #define AT_FDCWD    -100

// #define FUTEX_WAIT          0
// #define FUTEX_WAKE          1
// #define FUTEX_PRIVATE_FLAG  128
// #define FUTEX_WAIT_PRIVATE  (FUTEX_WAIT | FUTEX_PRIVATE_FLAG)
// #define FUTEX_WAKE_PRIVATE  (FUTEX_WAKE | FUTEX_PRIVATE_FLAG)

// #define CLONE_VM              0x00000100
// #define CLONE_FS              0x00000200
// #define CLONE_FILES           0x00000400
// #define CLONE_SIGHAND         0x00000800
// #define CLONE_THREAD          0x00010000
// #define CLONE_SYSVSEM         0x00040000
// #define CLONE_SETTLS          0x00080000
// #define CLONE_PARENT_SETTID   0x00100000
// #define CLONE_CHILD_CLEARTID  0x00200000

// #define DT_UNKNOWN  0
// #define DT_FIFO     1
// #define DT_CHR      2
// #define DT_DIR      4
// #define DT_BLK      6
// #define DT_REG      8
// #define DT_LNK     10
// #define DT_SOCK    12

// #define SEGV_MAPERR 1
// #define SEGV_ACCERR 2

// #define FPE_INTDIV 1
// #define FPE_INTOVF 2
// #define FPE_FLTDIV 3
// #define FPE_FLTOVF 4
// #define FPE_FLTUND 5
// #define FPE_FLTRES 6
// #define FPE_FLTINV 7
// #define FPE_FLTSUB 8

// #define ILL_ILLOPC 1
// #define ILL_ILLOPN 2
// #define ILL_ILLADR 3
// #define ILL_ILLTRP 4
// #define ILL_PRVOPC 5
// #define ILL_PRVREG 6
// #define ILL_COPROC 7
// #define ILL_BADSTK 8

// #define EPERM           0x01	
// #define ENOENT          0x02	
// #define ESRCH           0x03	
// #define EINTR           0x04	
// #define EIO             0x05	
// #define ENXIO           0x06	
// #define E2BIG           0x07	
// #define ENOEXEC         0x08	
// #define EBADF           0x09	
// #define ECHILD          0x0A	
// #define EAGAIN          0x0B	
// #define EWOULDBLOCK     0x0B	
// #define ENOMEM          0x0C	
// #define EACCES          0x0D	
// #define EFAULT          0x0E	
// #define ENOTBLK         0x0F	
// #define EBUSY           0x10	
// #define EEXIST          0x11	
// #define EXDEV           0x12	
// #define ENODEV          0x13	
// #define ENOTDIR         0x14	
// #define EISDIR          0x15	
// #define EINVAL          0x16	
// #define ENFILE          0x17	
// #define EMFILE          0x18	
// #define ENOTTY          0x19	
// #define ETXTBSY         0x1A	
// #define EFBIG           0x1B	
// #define ENOSPC          0x1C	
// #define ESPIPE          0x1D	
// #define EROFS           0x1E	
// #define EMLINK          0x1F	
// #define EPIPE           0x20	
// #define EDOM            0x21	
// #define ERANGE          0x22	
// #define EDEADLK         0x23	
// #define EDEADLOCK       0x23	
// #define ENAMETOOLONG    0x24	
// #define ENOLCK          0x25	
// #define ENOSYS          0x26	
// #define ENOTEMPTY       0x27	
// #define ELOOP           0x28	
// #define ENOMSG          0x2A	
// #define EIDRM           0x2B	
// #define ECHRNG          0x2C	
// #define EL2NSYNC        0x2D	
// #define EL3HLT          0x2E	
// #define EL3RST          0x2F	
// #define ELNRNG          0x30	
// #define EUNATCH         0x31	
// #define ENOCSI          0x32	
// #define EL2HLT          0x33	
// #define EBADE           0x34	
// #define EBADR           0x35	
// #define EXFULL          0x36	
// #define ENOANO          0x37	
// #define EBADRQC         0x38	
// #define EBADSLT         0x39	
// #define EBFONT          0x3B	
// #define ENOSTR          0x3C	
// #define ENODATA         0x3D	
// #define ETIME           0x3E	
// #define ENOSR           0x3F	
// #define ENONET          0x40	
// #define ENOPKG          0x41	
// #define EREMOTE         0x42	
// #define ENOLINK         0x43	
// #define EADV            0x44	
// #define ESRMNT          0x45	
// #define ECOMM           0x46	
// #define EPROTO          0x47	
// #define EMULTIHOP       0x48	
// #define EDOTDOT         0x49	
// #define EBADMSG         0x4A	
// #define EOVERFLOW       0x4B	
// #define ENOTUNIQ        0x4C	
// #define EBADFD          0x4D	
// #define EREMCHG         0x4E	
// #define ELIBACC         0x4F	
// #define ELIBBAD         0x50	
// #define ELIBSCN         0x51	
// #define ELIBMAX         0x52	
// #define ELIBEXEC        0x53	
// #define EILSEQ          0x54	
// #define ERESTART        0x55	
// #define ESTRPIPE        0x56	
// #define EUSERS          0x57	
// #define ENOTSOCK        0x58	
// #define EDESTADDRREQ    0x59	
// #define EMSGSIZE        0x5A	
// #define EPROTOTYPE      0x5B	
// #define ENOPROTOOPT     0x5C	
// #define EPROTONOSUPPORT 0x5D	
// #define ESOCKTNOSUPPORT 0x5E	
// #define EOPNOTSUPP      0x5F	
// #define ENOTSUP         0x5F	
// #define EPFNOSUPPORT    0x60	
// #define EAFNOSUPPORT    0x61	
// #define EADDRINUSE      0x62	
// #define EADDRNOTAVAIL   0x63	
// #define ENETDOWN        0x64	
// #define ENETUNREACH     0x65	
// #define ENETRESET       0x66	
// #define ECONNABORTED    0x67	
// #define ECONNRESET      0x68	
// #define ENOBUFS         0x69	
// #define EISCONN         0x6A	
// #define ENOTCONN        0x6B	
// #define ESHUTDOWN       0x6C	
// #define ETOOMANYREFS    0x6D	
// #define ETIMEDOUT       0x6E	
// #define ECONNREFUSED    0x6F	
// #define EHOSTDOWN       0x70	
// #define EHOSTUNREACH    0x71	
// #define EALREADY        0x72	
// #define EINPROGRESS     0x73	
// #define ESTALE          0x74	
// #define EUCLEAN         0x75	
// #define ENOTNAM         0x76	
// #define ENAVAIL         0x77	
// #define EISNAM          0x78	
// #define EREMOTEIO       0x79	
// #define EDQUOT          0x7A	
// #define ENOMEDIUM       0x7B	
// #define EMEDIUMTYPE     0x7C	
// #define ECANCELED       0x7D	
// #define ENOKEY          0x7E	
// #define EKEYEXPIRED     0x7F	
// #define EKEYREVOKED     0x80	
// #define EKEYREJECTED    0x81	
// #define EOWNERDEAD      0x82	
// #define ENOTRECOVERABLE 0x83	
// #define ERFKILL         0x84	
// #define EHWPOISON       0x85	

// #define CLOCK_REALTIME      0
// #define CLOCK_MONOTONIC     1
// #define CLOCK_MONOTONIC_RAW 4

// typedef void (*sighandler_t)(int);

// typedef uptr dev_t;
// typedef iptr time_t;
// typedef iptr pid_t;
// typedef uptr uid_t;
// typedef uptr gid_t;
// typedef iptr id_t;
// typedef uptr sigset_t;

// #define SIGMASK(sig) ((int) (1U << ((sig) - 1)))

// #define _NSIG_WORDS 1

// // @types______________________________________________________________________
// struct timespec {
//     iptr sec;
//     uptr nsec;
// };

// struct stat_info {
//     dev_t    dev;
//     uptr     ino;

// #if defined(NC_CPU_X86_64)
//     uptr     nlink;
//     u32      mode;
//     u32      uid;
//     u32      gid;
//     u32      __pad0;
// #elif defined(NC_CPU_X86_32)
//     u16      mode;
//     u16      nlink;
//     u16      uid;
//     u16      gid;
// #endif

//     dev_t    rdev;
//     uptr     size;
//     uptr     blksize;
//     uptr     blocks;
//     timespec atim;
//     timespec mtim;
//     timespec ctim;

// #if defined(NC_CPU_X86_64)
//     i64      __unused[3];
// #elif defined(NC_CPU_X86_32)
//     u32      __unused4;
//     u32      __unused5;
// #endif
// };

// union sigval {
//     int   sival_int;
//     void* sival_ptr;
// };

// struct siginfo_t {
//     int    si_signo;
//     int    si_code;
//     sigval si_value;
//     int    si_errno;
//     pid_t  si_pid;
//     uid_t  si_uid;
//     void*  si_addr;
//     int    si_status;
//     int    si_band;
// };

// struct sigaction_t {
//     union {
//         sighandler_t sa_handler;
//         void       (*sa_sigaction)(int, siginfo_t*, void*);
//     };

//     unsigned long    sa_flags;
//     void           (*sa_restorer)(void);
//     sigset_t         sa_mask;
// };

// struct linux_dirent64 {
//     u64  d_ino;
//     i64  d_off;
//     u16  d_reclen;
//     u8   d_type;
//     char d_name[];
// };

// struct kernel_sigset_t {
//     unsigned long sig[_NSIG_WORDS];
// };

// struct fpstate_t {
//     u16 cwd;
//     u16 swd;
//     u16 twd;
//     u16 fop;
//     u16 rip;
//     u16 rdp;
//     u32 mxcsr;
//     u32 mxcsr_mask;
//     u32 st_space[32];
//     u32 xmm_space[64];
//     u32 __reserved0[12];
//     u32 __reserved1[12];
// };

// struct mcontext_t {
//     u64        r8;
//     u64        r9;
//     u64        r10;
//     u64        r11;
//     u64        r12;
//     u64        r13;
//     u64        r14;
//     u64        r15;
//     u64        rdi;
//     u64        rsi;
//     u64        rbp;
//     u64        rbx;
//     u64        rdx;
//     u64        rax;
//     u64        rcx;
//     u64        rsp;
//     u64        rip;
//     u64        eflags;
//     u16        cs;
//     u16        gs;
//     u16        fs;
//     u16        __padding0;
//     u64        err;
//     u64        trapno;
//     u64        oldmask;
//     u64        cr2;
//     fpstate_t* fpstate;
//     u64        __reserved0[8];
// };

// struct ucontext_t {
//     unsigned long   uc_flags;
//     ucontext_t*     uc_link;

//     struct {
//         void*       ss_sp;
//         int         ss_flags;
//         uptr        ss_size;
//     } uc_stack;

//     mcontext_t      uc_mcontext;
//     kernel_sigset_t uc_sigmask;
// };

// struct stack_frame {
//     stack_frame* rbp;
//     void*        rip;
// };

// struct Dl_info {
//     char const* dli_fname;
//     void*       dli_fbase;
//     char const* dli_sname;
//     void*       dli_saddr;
// };

// // @runtime____________________________________________________________________
INTERNAL iptr
SYSCALL0(uptr Num)
{
    iptr Result;

    __asm__ __volatile__(
        "syscall\n"
        : "=a" (Result)
        : "a" (Num)
        : "rcx", "r11", "memory"
    );

    return Result;
}

INTERNAL iptr 
SYSCALL1(uptr Num, uptr Param)
{
    iptr Result;

    __asm__ __volatile__(
        "syscall\n"
        : "=a" (Result)
        : "a" (Num), "D" (Param)
        : "rcx", "r11", "memory"
    );

    return Result;
}

INTERNAL iptr 
SYSCALL2(uptr Num, uptr Param0, uptr Param1)
{
    iptr Result;

    __asm__ __volatile__(
        "syscall\n"
        : "=a" (Result)
        : "a" (Num), "D" (Param0), "S" (Param1)
        : "rcx", "r11", "memory"
    );

    return Result;
}

INTERNAL iptr 
SYSCALL3(uptr Num, uptr Param0, uptr Param1, uptr Param2)
{
    iptr Result;

    __asm__ __volatile__(
        "syscall\n"
        : "=a" (Result)
        : "a" (Num), "D" (Param0), "S" (Param1), "d" (Param2)
        : "rcx", "r11", "memory"
    );

    return Result;
}

INTERNAL iptr 
SYSCALL4(uptr Num, uptr Param0, uptr Param1, uptr Param2, uptr Param3)
{
    iptr Result;
    register uptr __r10 __asm__("r10") = Param3;

    __asm__ __volatile__(
        "syscall\n"
        : "=a" (Result)
        : "a" (Num), "D" (Param0), "S" (Param1), "d" (Param2), "r" (__r10)
        : "rcx", "r11", "memory"
    );

    return Result;
}

INTERNAL iptr 
SYSCALL5(
    uptr Num, 
    uptr Param0, 
    uptr Param1, 
    uptr Param2, 
    uptr Param3, 
    uptr Param4
) {
    iptr Result;
    register uptr __r10 __asm__("r10") = Param3;
    register uptr __r8 __asm__("r8") = Param4;

    __asm__ __volatile__(
        "syscall\n"
        : "=a" (Result)
        : "a" (Num)
        , "D" (Param0)
        , "S" (Param1)
        , "d" (Param2)
        , "r" (__r10)
        , "r" (__r8)
        : "rcx", "r11", "memory"
    );

    return Result;
}

INTERNAL iptr 
SYSCALL6(
    uptr Num, 
    uptr Param0, 
    uptr Param1, 
    uptr Param2, 
    uptr Param3, 
    uptr Param4, 
    uptr Param5
) {
    iptr Result;
    register uptr __r10 __asm__("r10") = Param3;
    register uptr __r8 __asm__("r8") = Param4;
    register uptr __r9 __asm__("r9") = Param5;

    __asm__ __volatile__(
        "syscall\n"
        : "=a" (Result)
        : "a" (Num)
        , "D" (Param0)
        , "S" (Param1)
        , "d" (Param2)
        , "r" (__r10)
        , "r" (__r8)
        , "r" (__r9)
        : "rcx", "r11", "memory"
    );

    return Result;
}

// __attribute__((naked))
// INTERNAL void 
// signal_restorer(void)
// {
//     __asm__ __volatile__(
//         "mov %0, %%rax\n"
//         "syscall\n"
//         "ud2\n"
//         : : "i" (SYSCALL_RT_SIGRETURN) : "memory"
//     );
// }

// INTERNAL void
// SigSetEmpty(kernel_sigset_t* Set)
// {
//     Set->sig[0] = 0;
// }

// INTERNAL void
// SigSetFill(kernel_sigset_t* Set)
// {
//     Set->sig[0] = ~0UL;
// }

// INTERNAL void
// SigSetAdd(kernel_sigset_t* Set, int SigNum)
// {
//     Set->sig[0] |= (1UL << (SigNum - 1));
// }

// INTERNAL void
// SigSetDelete(kernel_sigset_t* Set, int SigNum)
// {
//     Set->sig[0] &= ~(1UL << (SigNum - 1));
// }

// INTERNAL b32
// SigSetIsMember(kernel_sigset_t* Set, int SigNum)
// {
//     return (Set->sig[0] & (1UL << (SigNum - 1)));
// }

// INTERNAL iptr
// sigaction(int SigNum, sigaction_t* Action, sigaction_t* PrevAction)
// {
//     iptr Result;

//     if (Action) {
//         Action->sa_flags |= SA_RESTORER;
//         Action->sa_restorer = signal_restorer;
//     }

//     Result = SYSCALL4(
//         SYSCALL_RT_SIGACTION,
//         (uptr) SigNum,
//         (uptr) Action,
//         (uptr) PrevAction,
//         sizeof(kernel_sigset_t)
//     );

//     return Result;
// }

// INTERNAL iptr
// sigprocmask(int How, kernel_sigset_t* Set, kernel_sigset_t* PrevSet)
// {
//     return SYSCALL4(
//         SYSCALL_RT_SIGPROCMASK,
//         (uptr) How,
//         (uptr) Set,
//         (uptr) PrevSet,
//         sizeof(kernel_sigset_t)
//     );
// }

// INTERNAL void
// sleep(u64 NSecs)
// {
//     timespec TimeToSleep = {};

//     TimeToSleep.sec = NSecs / BILLION(1);
//     TimeToSleep.nsec = NSecs % BILLION(1);

//     while (
//         SYSCALL2(
//             SYSCALL_NANOSLEEP, 
//             (uptr) &TimeToSleep, 
//             (uptr) &TimeToSleep
//         ) == -EINTR
//     );
// }

// INTERNAL void
// yield(void)
// {
//     SYSCALL0(SYSCALL_SCHED_YIELD);
// }

// INTERNAL u64
// clock_gettime(void)
// {
//     u64 Result = 0;
//     timespec TimeSpec;

//     SYSCALL2(
//         SYSCALL_CLOCK_GETTIME,
//         CLOCK_MONOTONIC,
//         (uptr) &TimeSpec
//     );
//     Result = (u64) TimeSpec.sec * BILLION(1) + TimeSpec.nsec;

//     return Result;
// }

// INTERNAL void
// _exit(int Code)
// {
//     SYSCALL1(SYSCALL_EXIT_GROUP, (uptr) Code);
//     UNREACHABLE();
// }

// INTERNAL int
// backtrace(void** Buffer, int Max)
// {
//     int Result = 0;
//     stack_frame* Frame;

//     __asm__ __volatile__(
//         "mov %%rbp, %0"
//         : "=r" (Frame)
//     );

//     while (Frame && Result < Max) {
//         if ((uptr) Frame & 7)
//             break;

//         if ((uptr) Frame < 0x1000)
//             break;

//         if (!Frame->rip)
//             break;

//         Buffer[Result++] = Frame->rip;

//         if (Frame->rbp <= Frame)
//             break;

//         Frame = Frame->rbp;
//     }

//     return Result;
// }

// // @functions__________________________________________________________________

#endif // __LINUX_PLATFORM_H__
