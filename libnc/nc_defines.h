#if !defined(__NC_DEFINES_H__)
#define __NC_DEFINES_H__

// @compiler___________________________________________________________________
#if defined(_MSC_VER)
    #define NC_CC_MSVC
    #if defined(__MINGW32__)
        #define NC_CC_MINGW
        #define NC_CC_PREFIX MingW
    #else
        #define NC_CC_PREFIX
    #endif
    #if _MSC_VER >= 1930
        #define NC_CC_MSVC_YEAR 2022
        #define NC_CC_NAME NC_CC_PREFIX Microsoft Visual C++ 2022
    #elif _MSC_VER >= 1920
        #define NC_CC_MSVC_YEAR 2019
        #define NC_CC_NAME NC_CC_PREFIX Microsoft Visual C++ 2019
    #elif _MSC_VER >= 1910
        #define NC_CC_MSVC_YEAR 2017
        #define NC_CC_NAME NC_CC_PREFIX Microsoft Visual C++ 2017
    #elif _MSC_VER >= 1900
        #define NC_CC_MSVC_YEAR 2015
        #define NC_CC_NAME NC_CC_PREFIX Microsoft Visual C++ 2015
    #elif _MSC_VER >= 1800
        #define NC_CC_MSVC_YEAR 2013
        #define NC_CC_NAME NC_CC_PREFIX Microsoft Visual C++ 2013
    #elif _MSC_VER >= 1700
        #define NC_CC_MSVC_YEAR 2012
        #define NC_CC_NAME NC_CC_PREFIX Microsoft Visual C++ 2012
    #elif _MSC_VER >= 1600
        #define NC_CC_MSVC_YEAR 2010
        #define NC_CC_NAME NC_CC_PREFIX Microsoft Visual C++ 2010
    #elif _MSC_VER >= 1500
        #define NC_CC_MSVC_YEAR 2008
        #define NC_CC_NAME NC_CC_PREFIX Microsoft Visual C++ 2008
    #elif _MSC_VER >= 1400
        #define NC_CC_MSVC_YEAR 2005
        #define NC_CC_NAME NC_CC_PREFIX Microsoft Visual C++ 2005
    #else
        #define NC_CC_MSVC_YEAR 0
        #define NC_CC_NAME NC_CC_PREFIX Microsoft Visual C++ (Old Version)
    #endif
    #if defined(__INTEL_COMPILER)
        #define NC_CC_INTEL
        #if !defined(NC_CC_NAME)
            #define NC_CC_NAME Intel C++ (MSVC compatible)
        #endif
    #endif
    #if defined(__clang__)
        #define NC_CC_CLANG
        #if defined(__EMSCRIPTEN__)
            #define NC_CC_EMSCRIPTEN
            #if !defined(NC_CC_NAME)
                #define NC_CC_NAME Emscripten (Clang compatible)
            #endif
        #else
            #if !defined(NC_CC_NAME)
                #define NC_CC_NAME Clang (MSVC compatible)
            #endif
        #endif
    #endif
    #if defined(__INTEL_CLANG_COMPILER) || defined(__INTEL_LLVM_COMPILER)
        #define NC_CC_INTELX
        #if !defined(NC_CC_NAME)
            #define NC_CC_NAME Intel LLVM (MSVC compatible)
        #endif
    #endif
#elif defined(__GNUC__)
    #define NC_CC_GNU
    #define NC_CC_NAME GCC-__VERSION__
    #if defined(__MINGW32__)
        #define NC_CC_MINGW
        #if !defined(NC_CC_NAME)
            #define NC_CC_NAME MinGW GCC-__VERSION__
        #endif
    #endif
    #if defined(__INTEL_COMPILER)
        #define NC_CC_INTEL
        #if !defined(NC_CC_NAME)
            #define NC_CC_NAME Intel C++ (GCC compatible)
        #endif
    #endif
    #if defined(__INTEL_CLANG_COMPILER) || defined(__INTEL_LLVM_COMPILER)
        #define NC_CC_INTELX
        #if !defined(NC_CC_NAME)
            #define NC_CC_NAME Intel LLVM (GCC compatible)
        #endif
    #endif
    #if defined(__clang__)
        #if defined(NC_CC_NAME)
            #undef NC_CC_NAME
        #endif
        #define NC_CC_CLANG
        #define NC_CC_NAME Clang-__clang_version__
    #endif
    #if defined(__PGI)
        #define NC_CC_PGI
        #define NC_CC_NAME PGI Compiler
    #endif
    #if defined(_CRAYC)
        #define NC_CC_CRAY
        #define NC_CC_NAME Cray Compiler
    #endif
    #if defined(__LCC__)
        #define NC_CC_LCC
        #define NC_CC_NAME LCC Compiler
    #endif
#elif defined(__SUNPRO_C) || defined(__SUNPRO_CC)
    #define NC_CC_SUN
    #define NC_CC_NAME Sun/Oracle C++ Compiler
#elif defined(__xlC__) || defined(__xlc__)
    #define NC_CC_XLC
    #define NC_CC_NAME IBM XL C/C++
    #if defined(__ibmxl__)
        #define NC_CC_IXLC
        #define NC_CC_CLANG
        #if !defined(NC_CC_NAME)
            #define NC_CC_NAME IBM Clang-based XL C/C++
        #endif
    #endif
#elif defined(__ibmxl__)
    #define NC_CC_IXLC
    #define NC_CC_CLANG
    #define NC_CC_NAME IBM Clang-based XL C/C++
#elif defined(__open_xl__)
    #define NC_CC_OXLC
    #define NC_CC_CLANG
    #define NC_CC_NAME Open XL C/C++
#elif defined(__HP_cc) || defined(__HP_aCC)
    #define NC_CC_HP
    #define NC_CC_NAME HP aC++ Compiler
#elif defined(__DECC) || defined(__DECCXX)
    #define NC_CC_DEC
    #define NC_CC_NAME DEC C++ Compiler
#elif defined(__VMS) || defined(__clang__)
    #define NC_CC_DEC
    #define NC_CC_CLANG
    #define NC_CC_NAME DEC C++ (Clang-based)
#elif (defined(__sgi) || defined(sgi)) && (defined(_COMPILER_VERSION) || defined(_SGI_COMPILER_VERSION))
    #define NC_CC_MIPS
    #define NC_CC_NAME MIPSpro Compiler
#elif defined(__USLC__) && defined(__SCO_VERSION__)
    #define NC_CC_USLC
    #define NC_CC_NAME USL C Compiler
#elif defined(__WATCOMC__)
    #define NC_CC_WATCOM
    #define NC_CC_NAME Watcom C++ Compiler
#elif defined(__BORLANDC__)
    #define NC_CC_BORLAND
    #define NC_CC_NAME Borland C++ Compiler
#elif defined(__BORLANDC__)
    #define NC_CC_BORLAND
    #define NC_CC_NAME Borland C++ Compiler
#elif defined(__INTEL_COMPILER) || defined(__IBMCPP__)
    #define NC_CC_INTEL
    #define NC_CC_NAME Intel C++ Compiler
#elif defined(__INTEL_CLANG_COMPILER) || defined(__INTEL_LLVM_COMPILER)
    #define NC_CC_INTELX
    #define NC_CC_NAME Intel LLVM Compiler
#elif defined(__PGI)
    #define NC_CC_PGI
    #define NC_CC_NAME PGI Compiler
#elif defined(_CRAYC)
    #define NC_CC_CRAY
    #define NC_CC_NAME Cray Compiler
#elif defined(__LCC__)
    #define NC_CC_LCC
    #define NC_CC_NAME LCC Compiler
#elif defined(__clang__)
    #define NC_CC_CLANG
    #define NC_CC_NAME Clang-__clang_version__
#endif
#if !defined(NC_CC_NAME)
    #define NC_CC_NAME Unknown Compiler
#endif

// @os_________________________________________________________________________
#if defined(__APPLE__) && (defined(__GNUC__) || defined(__xlC__) || defined(__xlc__))
    #define NC_OS_DARWIN
    #define NC_OS_BSD4
    #if defined(__LP64__)
        #define NC_OS_DARWIN64
    #else
        #define NC_OS_DARWIN32
    #endif
#elif defined(__APPLE__) && defined(__MACH__)
    #define NC_OS_MAC_OSX
#elif defined(Macintosh) || defined(macintosh)
    #define NC_OS_MAC9
#elif defined(__MSYS__)
    #define NC_OS_MSYS
#elif defined(__CYGWIN__)
    #define NC_OS_CYGWIN
#elif defined(_WIN64) || defined(_M_X64) || defined(_M_AMD64)
    #define NC_OS_WIN64
#elif defined(__WIN32__) || defined(_WIN32) || defined(WIN32)
    #define NC_OS_WIN32
#elif defined(__ANDROID__)
    #define NC_OS_ANDROID
    #define NC_OS_LINUX
#elif defined(__linux) || defined(__linux__)
    #define NC_OS_LINUX
#elif defined(__FreeBSD__)
    #define NC_OS_FREEBSD
    #define NC_OS_BSD4
#elif defined(__DragonFly__)
    #define NC_OS_DRAGONFLY
    #define NC_OS_BSD4
#elif defined(__NetBSD__)
    #define NC_OS_NETBSD
    #define NC_OS_BSD4
#elif defined(__OpenBSD__)
    #define NC_OS_OPENBSD
    #define NC_OS_BSD4
#elif defined(_AIX)
    #define NC_OS_AIX
#elif defined(hpux) || defined(__hpux)
    #define NC_OS_HPUX
#elif defined(__osf__) || defined(__osf)
    #define NC_OS_TRU64
#elif defined(__sun) || defined(sun) || defined(__SOLARIS9__)
    #define NC_OS_SOLARIS
#elif defined(__QNXNTO__)
    #if defined(__BLACKBERRY10__)
        #define NC_OS_BB10
    #else
        #define NC_OS_QNX6
    #endif
#elif defined(__QNX__)
    #define NC_OS_QNX
#elif defined(_SCO_DS)
    #define NC_OS_SCO
#elif defined(__USLC__) || defined(__UNIXWARE__)
    #define NC_OS_UNIXWARE
#elif defined(__svr4__) && defined(i386)
    #define NC_OS_UNIXWARE
#elif defined(__sgi) || defined(sgi)
    #define NC_OS_IRIX
#elif defined(__HAIKU__)
    #define NC_OS_HAIKU
#elif defined(__SYLLABLE__)
    #define NC_OS_SYLLABLE
#elif defined(__BEOS__)
    #define NC_OS_BEOS
#elif defined(__OS2__)
    #define NC_OS_OS2
#elif defiuned(VMS) || defined(__VMS)
    #define NC_OS_VMS
#elif defined(AMIGA) || defined(__amigaos__)
    #define NC_OS_AMIGA
#endif
#if defined(NC_OS_WIN64) || defined(NC_OS_WIN32)
    #define NC_OS_WIN
#endif
#if defined(NC_OS_DARWIN)
    #define NC_OS_MAC
    #if defined(NC_OS_DARWIN64)
        #define NC_OS_MAC64
    #elif defined(NC_OS_DARWIN32)
        #define NC_OS_MAC32
    #endif
#endif
#if defined(NC_OS_WIN) || defined(NC_OS_MAC_OSX) || defined(NC_OS_MAC9) || defined(NC_OS_HAIKU) || defined(NC_OS_BEOS) || defined(NC_OS_OS2) || defined(NC_OS_VMS) || defined(NC_OS_AMIGA)
    #undef NC_OS_UNIX
#elif !defined(NC_OS_UNIX)
    #define NC_OS_UNIX
#endif
#if defined(_WIN64) || defined(__x86_64__) || defined(_M_X64) || defined(__64BIT__) || defined(__powerpc64__) || defined(__ppc64__) || defined(__aarch64__) || (defined(__riscv) && __riscv_xlen == 64)
    #define NC_ARCH_64
#else
    #define NC_ARCH_32
#endif
#if defined(NC_OS_WIN)
    #define WIN32_NT4           0x0400
    #define WIN32_2000          0x0500
    #define WIN32_WINXP         0x0501
    #define WIN32_SERVER_2003   0x0502
    #define WIN32_VISTA         0x0600
    #define WIN32_SERVER_2008   0x0600
    #define WIN32_WIN7          0x0601
    #define WIN32_WIN8          0x0602
    #define WIN32_WIN8_1        0x0603
    #define WIN32_SERVER_2016   0x0605
    #define WIN32_WIN10         0x0A00
#endif

// @cpu________________________________________________________________________
#if defined(__alpha__) || defined(__alpha) || defined(_M_ALPHA)
    #define NC_CPU_ALPHA
#elif defined(__arm__) || defined(__TARGET_ARCH_ARM) || defined(_ARM) || defined(_M_ARM) || defined(_M_ARM64) || defined(__arm) || defined(__aarch64__) || defined(__ARM64__) || defined(__arm64__) || defined(__arm64)
    #if defined(__aarch64__) || defined(_M_ARM64) || defined(__ARM64__) || defined(__arm64__) || defined(__arm64)
        #define NC_CPU_ARM64
        #define NC_CPU_CACHE_LINE_SIZE 64
    #else
        #define NC_CPU_ARM32
        #define NC_CPU_CACHE_LINE_SIZE 32
    #endif
    #if defined(__ARM_ARCH) && __ARM_ARCH > 1
        #define NC_CPU_ARM __ARM_ARCH
    #elif defined(__TARGET_ARCH_ARM) && __TARGET_ARCH_ARM > 1
        #define NC_CPU_ARM __TARGET_ARCH_ARM
    #elif defined(_M_ARM) && _M_ARM > 1
        #define NC_CPU_ARM _M_ARM
    #elif defined(__ARM_ARCH_9__)
        #define NC_CPU_ARM 9
    #elif defined(__ARM64_ARCH_8__) || defined(__ARM_ARCH_8__) || defined(__ARM_ARCH_8A__) || defined(__aarch64__) || defined(__ARMv8__) || defined(__ARMv8_A__) || defined(_M_ARM64) || defined(__CORE_CORTEXAV8__)
        #define NC_CPU_ARM 8
    #elif defined(__ARM_ARCH_7__) || defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7S__) || defined(_ARM_ARCH_7) || defined(__CORE_CORTEXA__)
        #define NC_CPU_ARM 7
    #elif defined(__ARM_ARCH_6__) || defined(__ARM_ARCH_6J__) || defined(__ARM_ARCH_6T2__) || defined(__ARM_ARCH_6Z__) || defined(__ARM_ARCH_6K__) || defined(__ARM_ARCH_6ZK__) || defined(__ARM_ARCH_6M__)
        #define NC_CPU_ARM 6
    #elif defined(__ARM_ARCH_5__) || defined(__ARM_ARCH_5E__) || defined(__ARM_ARCH_5T__) || defined(__ARM_ARCH_5TE__) || defined(__ARM_ARCH_5TEJ__)
        #define NC_CPU_ARM 5
    #elif defined(__ARM_ARCH_4__) || defined(__ARM_ARCH_4T__)
        #define NC_CPU_ARM 4
    #elif defined(__ARM_ARCH_3__) || defined(__ARM_ARCH_3M__)
        #define NC_CPU_ARM 3
    #elif defined(__ARM_ARCH_2__)
        #define NC_CPU_ARM 2
    #else
        #define NC_CPU_ARM 0
    #endif
    #if NC_CPU_ARM == 9
        #define NC_CPU_ARM_V9
    #elif NC_CPU_ARM == 8
        #define NC_CPU_ARM_V8
    #elif NC_CPU_ARM == 7
        #define NC_CPU_ARM_V7
    #elif NC_CPU_ARM == 6
        #define NC_CPU_ARM_V6
    #elif NC_CPU_ARM == 5
        #define NC_CPU_ARM_V5
    #elif NC_CPU_ARM == 4
        #define NC_CPU_ARM_V4
    #elif NC_CPU_ARM == 3
        #define NC_CPU_ARM_V3
    #elif NC_CPU_ARM == 2
        #define NC_CPU_ARM_V2
    #else
        #error "ARM architecture is unknown or too old"
    #endif
#elif defined(__i386__) || defined(__i386) || defined(_M_IX86)
    #define NC_CPU_X86_32
    #define NC_CPU_CACHE_LINE_SIZE 32
    #if defined(_M_IX86)
        #if (_M_IX86 >= 300 && _M_IX86 <= 600)
            #define NC_CPU_X86 (_M_IX86 / 100)
        #else
            #define NC_CPU_X86 6
        #endif
    #elif defined(__i686__) || defined(__athlon__) || defined(__SSE__) || defined(__pentiumpro__)
        #define NC_CPU_X86 6
    #elif defined(__i586__) || defined(__k6__) || defined(__pentium__)
        #define NC_CPU_X86 5
    #elif defined(__i486__) || defined(__80486__)
        #define NC_CPU_X86 4
    #else
        #define NC_CPU_X86 3
    #endif
#elif defined(__x86_64__) || defined(__x86_64) || defined(__amd64__) || defined(__amd64) || defined(_M_X64) || defined(_M_AMD64)
    #define NC_CPU_X86_64
    #define NC_CPU_CACHE_LINE_SIZE 64
    #define NC_CPU_X86 6
#elif defined(__ia64__) || defined(__ia64) || defined(_M_IA64)
    #define NC_CPU_IA64
#elif defined(__mips__) || defined(__mips) || defined(_M_MRX000)
    #define NC_CPU_MIPS
    #define NC_CPU_CACHE_LINE_SIZE 64
    #if defined(_M_MRX000)
        #if _M_MRX000 >= 10000
            #define NC_CPU_MIPS_IV
        #else
            #define NC_CPU_MIPS_III
        #endif
    #endif
    #if defined(_MIPS_ARCH_MIPS64) || defined(__mips64) || (defined(__mips) && __mips - 0 >= 64) || (defined(_MIPS_ISA) && defined(_MIPS_ISA_MIPS64) && __MIPS_ISA - 0 >= _MIPS_ISA_MIPS64)
        #define NC_CPU_MIPS_64
    #elif defined(_MIPS_ARCH_MIPS32) || defined(__mips32) || (defined(__mips) && __mips - 0 >= 32) || (defined(_MIPS_ISA) && defined(_MIPS_ISA_MIPS32) && __MIPS_ISA - 0 >= _MIPS_ISA_MIPS32)
        #define NC_CPU_MIPS_32
    #elif defined(_MIPS_ARCH_MIPS5) || (defined(__mips) && __mips - 0 >= 5) || (defined(_MIPS_ISA) && defined(_MIPS_ISA_MIPS5) && __MIPS_ISA - 0 >= _MIPS_ISA_MIPS5)
        #define NC_CPU_MIPS_V
    #elif defined(_MIPS_ARCH_MIPS4) || (defined(__mips) && __mips - 0 >= 4) || (defined(_MIPS_ISA) && defined(_MIPS_ISA_MIPS4) && __MIPS_ISA - 0 >= _MIPS_ISA_MIPS4)
        #define NC_CPU_MIPS_IV
    #elif defined(_MIPS_ARCH_MIPS3) || (defined(__mips) && __mips - 0 >= 3) || (defined(_MIPS_ISA)&& defined(_MIPS_ISA_MIPS3) && __MIPS_ISA - 0 >= _MIPS_ISA_MIPS3)
        #define NC_CPU_MIPS_III
    #elif defined(_MIPS_ARCH_MIPS2) || (defined(__mips) && __mips - 0 >= 2) || (defined(_MIPS_ISA) && defined(_MIPS_ISA_MIPS2) && __MIPS_ISA - 0 >= _MIPS_ISA_MIPS2)
        #define NC_CPU_MIPS_II
    #elif defined(_MIPS_ARCH_MIPS1) || (defined(__mips) && __mips - 0 >= 1) || (defined(_MIPS_ISA) && defined(_MIPS_ISA_MIPS1) && __MIPS_ISA - 0 >= _MIPS_ISA_MIPS1)
        #define NC_CPU_MIPS_I
    #endif
    #if defined(NC_CPU_MIPS_64)
        #define NC_CPU_MIPS_V
    #endif
    #if defined(NC_CPU_MIPS_V)
        #define NC_CPU_MIPS_IV
    #endif
    #if defined(NC_CPU_MIPS_IV)
        #define NC_CPU_MIPS_III
    #endif
    #if defined(NC_CPU_MIPS_32) || defined(NC_CPU_MIPS_III)
        #define NC_CPU_MIPS_II
    #endif
    #if defined(NC_CPU_MIPS_II)
        #define NC_CPU_MIPS_I
    #endif
#elif defined(__powerpc__) || defined(__powerpc) || defined(__ppc__) || defined(__ppc) || defined(_ARCH_PPC) || defined(_ARCH_PWR) || defined(_ARCH_COM) || defined(_M_PPC) || defined(_M_MPPC)
    #define NC_CPU_POWER
    #define NC_CPU_CACHE_LINE_SIZE 128
    #if defined(__powerpc64__) || defined(__powerpc64) || defined(__ppc64__) || defined(__ppc64) || defined(__64BIT__) || defined(__LP64__) || defined(_LP64)
        #define NC_CPU_POWER_64
    #else
        #define NC_CPU_POWER_32
    #endif
#elif defined(__sparc__) || defined(__sparc)
    #define NC_CPU_SPARC
    #if defined(__sparc_v9__) || defined(__sparcv9)
        #define NC_CPU_SPARC_V9
    #elif defined(__sparc_v8__) || defined(__sparcv8)
        #define NC_CPU_SPARC_V8
    #endif
#elif defined(__hppa__) || defined(__hppa)
    #define NC_CPU_HPPA
    #if defined(_PA_RISC2_0) || defined(__RISC2_0__) || defined(__HPPA20__) || defined(__PA8000__)
        #define NC_CPU_HPPA_64
    #else
        #define NC_CPU_HPPA_32
    #endif
#elif defined(__s390__)
    #define NC_CPU_S390
    #if defined(__s390x__)
        #define NC_CPU_S390X
    #endif
#elif defined(__riscv) || defined(__riscv__)
    #define NC_CPU_RISCV
    #if defined(__riscv64) || (defined(__riscv_xlen) && (__riscv_xlen == 64))
        #define NC_CPU_RISCV_64
        #define NC_CPU_CACHE_LINE_SIZE 64
    #else
        #define NC_CPU_RISCV_32
        #define NC_CPU_CACHE_LINE_SIZE 32
    #endif
#elif defined(__loongarch__)
    #define NC_CPU_LOONGARCH
    #if defined(__loongarch64) || (__loongarch_grlen == 64)
        #define NC_CPU_LOONGARCH_64
    #else
        #define NC_CPU_LOONGARCH_32
    #endif
#elif defined(__e2k__)
    #define NC_CPU_E2K
#endif

// @simd_______________________________________________________________________
#if defined(__AVX512F__)
    #define NC_CPU_AVX512F
#endif
#if defined(__AVX2__)
    #define NC_CPU_AVX2
#endif
#if defined(__AVX__)
    #define NC_CPU_AVX
#endif
#if defined(__SSE4_2__)
    #define NC_CPU_SSE4_2
#endif
#if defined(__SSE4_1__)
    #define NC_CPU_SSE4_1
#endif
#if defined(__SSSE3__)
    #define NC_CPU_SSSE3
#endif
#if defined(__SSE3__)
    #define NC_CPU_SSE3
#endif
#if defined(__SSE2__) || defined(_M_X64) || defined(_M_AMD64) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2)
    #define NC_CPU_SSE2
    #if defined(__x86_64__) || defined(_M_X64) || defined(_M_AMD64)
        #define NC_CPU_SSE2_64
    #else
        #define NC_CPU_SSE2_32
    #endif
#endif
#if defined(__SSE__) || (defined(_M_IX86_FP) && _M_IX86_FP >= 1)
    #define NC_CPU_SSE
#endif
#if defined(__ARM_NEON) || defined(__ARM_NEON__)
    #define NC_CPU_NEON
    #if defined(__aarch64__) || defined(_M_ARM64)
        #define NC_CPU_NEON_64
    #else
        #define NC_CPU_NEON_32
    #endif
#endif
#if defined(__ARM_FEATURE_SVE)
    #define NC_CPU_SVE
#endif
#if defined(__ALTIVEC__) || defined(__VEC__)
    #define NC_CPU_ALTIVEC
#endif
#if defined(__wasm_simd128__)
    #define NC_CPU_WASM_SIMD
#endif
#if defined(__riscv_vector)
    #define NC_CPU_RISCV_V
#endif
#define NC_SIMD_ALIGN128 16
#define NC_SIMD_ALIGN256 32
#define NC_SIMD_ALIGN512 64
#if defined(NC_CPU_AVX512F)
    #define NC_SIMD_WIDTH 512
    #define NC_SIMD_ALIGN NC_SIMD_ALIGN512
    #define NC_SIMD_LEVEL 8
#elif defined(NC_CPU_AVX2)
    #define NC_SIMD_WIDTH 256
    #define NC_SIMD_ALIGN NC_SIMD_ALIGN256
    #define NC_SIMD_LEVEL 7
#elif defined(NC_CPU_AVX)
    #define NC_SIMD_WIDTH 256
    #define NC_SIMD_ALIGN NC_SIMD_ALIGN256
    #define NC_SIMD_LEVEL 6
#elif defined(NC_CPU_SSE4_2) || defined(NC_CPU_SSE4_1)
    #define NC_SIMD_WIDTH 128
    #define NC_SIMD_ALIGN NC_SIMD_ALIGN128
    #define NC_SIMD_LEVEL 5
#elif defined(NC_CPU_SSSE3) || defined(NC_CPU_SSE3)
    #define NC_SIMD_WIDTH 128
    #define NC_SIMD_ALIGN NC_SIMD_ALIGN128
    #define NC_SIMD_LEVEL 4
#elif defined(NC_CPU_SSE2) || defined(NC_CPU_NEON) || defined(NC_CPU_ALTIVEC) || defined(NC_CPU_WASM_SIMD)
    #define NC_SIMD_WIDTH 128
    #define NC_SIMD_ALIGN NC_SIMD_ALIGN128
    #define NC_SIMD_LEVEL 3
#elif defined(NC_CPU_SSE)
    #define NC_SIMD_WIDTH 128
    #define NC_SIMD_ALIGN NC_SIMD_ALIGN128
    #define NC_SIMD_LEVEL 2
#elif defined(NC_CPU_SVE) || defined(NC_CPU_RISCV_V)
    #define NC_SIMD_WIDTH 0  // Variable width
    #define NC_SIMD_ALIGN NC_SIMD_ALIGN128
    #define NC_SIMD_LEVEL 1
#else
    #define NC_CPU_SCALAR
    #define NC_SIMD_WIDTH 0
    #define NC_SIMD_ALIGN 1
    #define NC_SIMD_LEVEL 0
#endif
#define NC_SIMD_PACK_COUNT (NC_SIMD_WIDTH / NC_SIMD_ALIGN)
#if !defined(NC_CPU_CACHE_LINE_SIZE)
    #warning Cache line size not known for CPU
    #define NC_CPU_CACHE_LINE_SIZE 64
#endif

// @lang_______________________________________________________________________
#if defined(__cplusplus)
    #define NC_LANG_CPP
    #if defined(NC_CC_MSVC)
        #if defined(_MSVC_LANG)
            #if _MSVC_LANG <= 199711L
                #define NC_LANG_CPP_VERSION 98
            #elif _MSVC_LANG <= 201103L
                #define NC_LANG_CPP_VERSION 11
            #elif _MSVC_LANG <= 201402L
                #define NC_LANG_CPP_VERSION 14
            #elif _MSVC_LANG <= 201703L
                #define NC_LANG_CPP_VERSION 17
            #elif _MSVC_LANG <= 202002L
                #define NC_LANG_CPP_VERSION 20
            #else
                #define NC_LANG_CPP_VERSION 23
            #endif
        #else
            #if NC_CC_MSVC_YEAR <= 2010
                #define NC_LANG_CPP_VERSION 98
            #elif NC_CC_MSVC_YEAR <= 2015
                #define NC_LANG_CPP_VERSION 11
            #else
                #define NC_LANG_CPP_VERSION 17
            #endif
        #endif
    #else
        #if __cplusplus <= 199711L
            #define NC_LANG_CPP_VERSION 98
        #elif __cplusplus <= 201103L
            #define NC_LANG_CPP_VERSION 11
        #elif __cplusplus <= 201402L
            #define NC_LANG_CPP_VERSION 14
        #elif __cplusplus <= 201703L
            #define NC_LANG_CPP_VERSION 17
        #elif __cplusplus <= 202002L
            #define NC_LANG_CPP_VERSION 20
        #else
            #define NC_LANG_CPP_VERSION 23
        #endif
    #endif
    #define NC_LANG_NAME C++NC_LANG_CPP_VERSION
#elif defined(__OBJC__)
    #define NC_LANG_OBJC
    #define NC_LANG_NAME "Objective-C"
#else
    #define NC_LANG_C
    #define NC_LANG_NAME "C"
#endif
#if !defined(NC_LANG_NAME)
    #define NC_LANG_NAME "Unknown Language"
#endif

// @macros_____________________________________________________________________
#define internal static
#define global   static
#define local    static
#define IN
#define OUT
#define _GLUE(X, Y) X##Y
#define GLUE(X, Y) _GLUE(X, Y)
#define _STRINGIFY(X) #X
#define STRINGIFY(X) _STRINGIFY(X)
#if defined(NC_CC_MSVC)
    #define STATIC_MSG(X) __pragma(message(STRINGIFY(X)))
#elif (defined(NC_CC_GNU) && __GNUC__ >= 4) || defined(NC_CC_CLANG)
    #define STATIC_MSG(X) _Pragma(STRINGIFY(message(X)))
#else
    #define STATIC_MSG(X)
#endif
#define NC_BYTE_ORDER_LITTLE 0
#define NC_BYTE_ORDER_BIG    1
#if !defined(NC_BYTE_ORDER)
    #if defined(__BYTE_ORDER__)
        #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
            #define NC_BYTE_ORDER NC_BYTE_ORDER_LITTLE
        #elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN
            #define NC_BYTE_ORDER NC_BYTE_ORDER_BIG
        #else
            #error Could not determine byte ordering for system
        #endif
    #elif defined(__GLIBC__)
        #include <endian.h>
        #if __BYTE_ORDER == __LITTLE_ENDIAN
            #define NC_BYTE_ORDER NC_BYTE_ORDER_LITTLE
        #elif __BYTE_ORDER == __BIG_ENDIAN
            #define NC_BYTE_ORDER NC_BYTE_ORDER_BIG
        #else
            #error Could not determine byte ordering for system
        #endif
    #elif defined(_LITTLE_ENDIAN) && !defined(_BIG_ENDIAN)
        #define NC_BYTE_ORDER NC_BYTE_ORDER_LITTLE
    #elif defined(_BIG_ENDIAN) && !defined(_LITTLE_ENDIAN)
        #define NC_BYTE_ORDER NC_BYTE_ORDER_BIG
    #elif defined(NC_CPU_SPARC) || defined(NC_CPU_POWER) || defined(NC_CPU_HPPA) || defined(NC_CPU_MIPS) || defined(NC_CPU_S390)
        #define NC_BYTE_ORDER NC_BYTE_ORDER_BIG
    #elif defined(NC_CPU_X86) || defined(NC_CPU_ALPHA)
        #define NC_BYTE_ORDER NC_BYTE_ORDER_LITTLE
    #elif defined(NC_CC_MSVC) && defined(NC_CPU_ARM)
        #define NC_BYTE_ORDER NC_BYTE_ORDER_LITTLE
    #else
        #define NC_BYTE_ORDER NC_BYTE_ORDER_LITTLE
        STATIC_MSG(Could not determine byte ordering for system - assuming LE);
    #endif
#endif
#define ARRAY_COUNT(X) (sizeof((X)) / sizeof((X)[0]))
#define SWAP(T, X, Y) do { T _X = (X); (X) = (Y); (Y) = _X; } while (FALSE)
#if defined(NC_CC_MSVC) || (defined(NC_CC_CLANG) && defined(NC_OS_WIN))
    #pragma section(".rdata$", read)
    #define readonly __declspec(allocate(".rdata$"))
#elif defined(NC_CC_CLANG) || defined(NC_CC_GNU)
    #define readonly const // __attribute__((section(".rodata.nc")))
#else
    #define readonly
    STATIC_MSG(readonly not defined on your system)
#endif
#if defined(NC_CC_MSVC)
    #define force_inline __forceinline
#elif defined(NC_CC_CLANG) || defined(NC_CC_GNU)
    #define force_inline __attribute__((always_inline)) inline
#else
    STATIC_MSG(force_inline not defined for compiler)
    #define force_inline inline
#endif
#if defined(NC_CC_MSVC)
    #define no_inline __declspec(noinline)
#elif defined(NC_CC_CLANG) || defined(NC_CC_GNU)
    #define no_inline __attribute__((__noinline__))
#endif
#if !defined(no_inline)
    #if defined(NC_LANG_CPP) && NC_LANG_CPP_VERSION >= 11
        #define no_inline [[gnu::noinline]]
    #else
        STATIC_MSG(no_inline not defined for compiler)
        #define no_inline
    #endif
#endif
#if defined(NC_CC_MSVC)
    #define DLL_IMPORT __declspec(dllimport)
    #define DLL_EXPORT __declspec(dllexport)
#elif defined(NC_CC_CLANG) || defined(NC_CC_GNU)
    #define DLL_IMPORT
    #define DLL_EXPORT __attribute__((visibility("default")))
#else
    #define DLL_IMPORT
    #define DLL_EXPORT
#endif
#define INTERNAL internal force_inline
#define API_INTERNAL force_inline
#if defined(NC_LANG_CPP)
    #define EXTERN_C_LINK extern "C"
    #define EXTERN_C_LINK_BEGIN extern "C" {
    #define EXTERN_C_LINK_END }
#else
    #define EXTERN_C_LINK
    #define EXTERN_C_LINK_BEGIN
    #define EXTERN_C_LINK_END
#endif
#if defined(NC_CC_MSVC) || (defined(NC_CC_CLANG) && defined(NC_OS_WIN))
    #define PACKED_STRUCT_BEGIN __pragma(pack(push, 1))
    #define PACKED_STRUCT_END __pragma(pack(pop))
    #define PACKED
#elif (defined(NC_CC_GNU) && __GNUC__ >= 4) || defined(NC_CC_CLANG)
    #define PACKED_STRUCT_BEGIN
    #define PACKED_STRUCT_END
    #define PACKED __attribute__((__packed__))
#else
    #define PACKED_STRUCT_BEGIN
    #define PACKED_STRUCT_END
    #define PACKED
    STATIC_MSG("PACKED_STRUCTs not defined for compiler");
#endif
#if defined(NC_CC_MSVC)
    #define THREAD_LOCAL __declspec(thread)
#elif (defined(NC_CC_GNU) && __GNUC__ >= 4) || defined(NC_CC_CLANG)
    #define THREAD_LOCAL __thread
#else
    #define THREAD_LOCAL
    STATIC_MSG("THREAD_LOCAL not defined for compiler");
#endif
#if defined(NC_CC_MSVC)
    #define ALIGNAS(X) __declspec(align(X))
#elif (defined(NC_CC_GNU) && __GNUC__ >= 4) || defined(NC_CC_CLANG)
    #define ALIGNAS(X) alignas(X)
#else
    #error "ALIGNAS not defined for compiler"
#endif
#if defined(NC_CC_MSVC) || (defined(NC_CC_CLANG) && defined(NC_OS_WIN))
    #define __ALIGNED(X) ALIGNAS(X)
#elif (defined(NC_CC_GNU) && __GNUC__ >= 4) || defined(NC_CC_CLANG)
    #define __ALIGNED(X) __attribute__((aligned(X)))
#else
    #error "__ALIGNED not defined for compiler"
#endif
#if defined(NC_CPU_X86_64)
    #define CACHE_LINE_SIZE 64
#elif defined(NC_CPU_X86_32)
    #define CACHE_LINE_SIZE 32
#else
    #define CACHE_LINE_SIZE 64
    #warning "CACHE_LINE_SIZE not defined for architecture, assuming 64"
#endif
#define CACHE_ALIGN ALIGNAS(CACHE_LINE_SIZE)
#if defined(NC_OS_WIN)
    #define CACHE_ALIGN_STRUCT CACHE_ALIGN
#elif defined(NC_OS_LINUX)
    #define CACHE_ALIGN_STRUCT __attribute__((aligned(CACHE_LINE_SIZE)))
#else
    #error "CACHE_ALIGN_STRUCT not defined for operating system"
#endif
#define ALIGNTO(X, Y) (((X) + ((Y) - 1)) & ~((Y) - 1))
#if (defined(NC_CC_MSVC) || defined(NC_CC_CLANG))
    #define ALIGNOF(X) __alignof(X)
#elif defined(NC_CC_GNU)
    #define ALIGNOF(X) __alignof__(X)
#else
    #error "ALIGNOF not defined for compiler"
#endif
#define MEMBER(X, Y) (((X*) 0)->Y)
#define OFFSETOF(X, Y) ((u64) (&MEMBER(X, Y)))
#define MEMBER_FROM_OFFSET(X, Y, Z) (X) ((((u8*) Y) + (Z)))
#define CAST_FROM_MEMBER(X, Y, Z) (X*) (((u8*) Z) - OFFSETOF(X, Y))
#if defined(NC_CC_CLANG)
    #define EXPECT(X, Y) __builtin_expect((X), (Y))
#else
    #define EXPECT(X, Y) (X)
#endif
#define LIKELY(X) EXPECT(X, TRUE)
#define UNLIKELY(X) EXPECT(X, FALSE)
#define UNUSED(X) ((void) (X))
#if defined(__has_include)
    #define IS_INCLUDED(X) __has_include(X)
#else
    #define IS_INCLUDED(X) FALSE
#endif
#if defined(__has_builtin)
    #define IS_BUILTIN(X) __has_builtin(__builtin_##X)
#else
    #define IS_BUILTIN(X) FALSE
#endif
#if IS_BUILTIN(trap)
    #define TRAP() __builtin_trap()
#elif defined(NC_CC_MSVC) && !defined(NC_CC_CLANG)
    #if defined(NC_CPU_X86)
        // #define TRAP() __ud2()
        #define TRAP() __debugbreak()
    #else
        #define TRAP() __fastfail(7)
    #endif
#elif defined(NC_CC_GNU) || defined(NC_CC_CLANG)
    #if defined(NC_CPU_ARM_32)
        #define TRAP() __asm__ __volatile__("udf #0xFE")
    #elif defined(NC_CPU_ARM_64)
        #define TRAP() __asm__ __volatile__("udf #0xFDEE")
    #elif defined(NC_CPU_POWER)
        #define TRAP() __asm__ __volatile__(".word 0x0")
    #elif defined(NC_CPU_MIPS)
        #define TRAP() __asm__ __volatile__(".word 0x3D")
    #elif defined(NC_CPU_RISCV)
        #define TRAP() __asm__ __volatile__("unimp")
    #elif defined(NC_CPU_S390X)
        #define TRAP() __asm__ __volatile__("j 0x2")
    #elif defined(NC_CPU_SPARC)
        #define TRAP() __asm__ __volatile__("illtrap")
    #elif defined(NC_CPU_X86)
        #define TRAP() __asm__ __volatile__("ud2")
    #endif
#else
    #define TRAP()
    STATIC_MSG("TRAP not defined for your machine");
#endif
#define ASSERT_ALWAYS(X) do { if (!(X)) { TRAP(); }} while (FALSE)
#if defined(NC_DEBUG)
    #define ASSERT(X) ASSERT_ALWAYS(X)
#else
    #define ASSERT(X) UNUSED(X)
#endif
#define STATIC_ASSERT(X, Y) global u8 GLUE(Y, __LINE__)[(X) ? 1 : -1]
#define UNREACHABLE() ASSERT_ALWAYS(!"Unreachable code")
// NOTE(nc): X is ^2 => X should only have a single bit set
#define IS_POW2(X) ((X) != 0 && ((X) & ((X) - 1)) == 0)
#define ASSERT_POW2(X) ASSERT_ALWAYS(IS_POW2(X))
#define DEFER(X, Y) for (i64 __I = ((X), FALSE); !__I; ++__I, (Y))
#define DEFER_SAFE(X, Y) for (i64 __I = 2 * !(X); (__I == 2 ? ((Y), FALSE) : !__I); ++__I, (Y))
#define MIN(X, Y)       (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y)       (((X) > (Y)) ? (X) : (Y))
#define CLAMP_TOP(X, Y) MIN(X, Y)
#define CLAMP_BOT(X, Y) MAX(X, Y)
#define CLAMP(X, Y, Z)  CLAMP_BOT(X, CLAMP_TOP(Y, Z))
#if defined(NC_CC_MSVC)
    #if defined(__SANITIZE_ADDRESS__)
        #define ASAN_ENABLED 1
        #define NO_ASAN __declspec(no_sanitize_address)
    #else
        #define NO_ASAN
    #endif
#elif defined(NC_CC_CLANG)
    #if defined(__has_feature)
        #if __has_feature(address_sanitizer) || defined(__SANITIZE_ADDRESS__)
            #define ASAN_ENABLED 1
        #endif
    #endif
    #define NO_ASAN __attribute__((no_sanitize("address")))
#else
    #define NO_ASAN
#endif
#if ASAN_ENABLED
    #pragma comment(lib, "clang_rt.asan_dynamic-x86_64.lib")
    EXTERN_C_LINK void __asan_poison_memory_region(void const volatile* addr, size_t size);
    EXTERN_C_LINK void __asan_unpoison_memory_region(void const volatile* addr, size_t size);
    #define ASAN_POISON(Addr, Size) __asan_poison_memory_region((Addr), (Size))
    #define ASAN_UNPOISON(Addr, Size) __asan_unpoison_memory_region((Addr), (Size))
#else
    #define ASAN_POISON(...)
    #define ASAN_UNPOISON(...)
#endif
#if defined(NC_CC_MSVC)
    #define NAKED __declspec(naked)
#elif defined(NC_CC_GNU) || defined(NC_CC_CLANG)
    #define NAKED __attribute__((naked))
#endif
#define EACH_ENUM(X, Y, Z) (X Y = (X) 0; (Y) < (Z); (Y) = (X) (Y + 1))
#define DLL_INSERT(Head, Tail, Pred, Node) do { \
    if (!(Head)) {                              \
        (Head) = (Tail) = (Node);               \
        (Node)->Next = (Node)->Prev = NULL;     \
    } else if (!(Pred)) {                       \
        (Node)->Next = (Head);                  \
        (Head)->Prev = (Node);                  \
        (Head) = (Node);                        \
        (Node)->Prev = NULL;                    \
    } else if ((Pred) == (Tail)) {              \
        (Tail)->Next = (Node);                  \
        (Node)->Prev = (Tail);                  \
        (Tail) = (Node);                        \
        (Node)->Next = NULL;                    \
    } else {                                    \
        if ((Pred) && (Pred)->Next) {           \
            (Pred)->Next->Prev = (Node);        \
        }                                       \
                                                \
        (Node)->Next = (Pred)->Next;            \
        (Pred)->Next = (Node);                  \
        (Node)->Prev = (Pred);                  \
    }                                           \
} while (FALSE)
#define DLL_INSERT_EX(Empty, Head, Tail, Pred, Node, _Next, _Prev) do { \
    if ((Head) == (Empty)) {                                            \
        (Head) = (Tail) = (Node);                                       \
        (Node)->_Next = (Node)->_Prev = (Empty);                        \
    } else if ((Pred) == (Empty)) {                                     \
        (Node)->_Next = (Head);                                         \
        (Head)->_Prev = (Node);                                         \
        (Head) = (Node);                                                \
        (Node)->_Prev = (Empty);                                        \
    } else if ((Pred) == (Tail)) {                                      \
        (Tail)->_Next = (Node);                                         \
        (Node)->_Prev = (Tail);                                         \
        (Tail) = (Node);                                                \
        (Node)->_Next = (Empty);                                        \
    } else {                                                            \
        if ((Pred) && (Pred)->_Next) {                                  \
            (Pred)->_Next->_Prev = (Node);                              \
        }                                                               \
                                                                        \
        (Node)->_Next = (Pred)->_Next;                                  \
        (Pred)->_Next = (Node);                                         \
        (Node)->_Prev = (Pred);                                         \
    }                                                                   \
} while (FALSE)
#define DLL_PUSH_BACK(Head, Tail, Node) DLL_INSERT(Head, Tail, Tail, Node)
#define DLL_PUSH_BACK_EX(Empty, Head, Tail, Node, _Next, _Prev) DLL_INSERT_EX(Empty, Head, Tail, Tail, Node, _Next, _Prev)
#define DLL_PUSH_FRONT(Head, Tail, Node) DLL_INSERT(Head, Tail, NULL, Node)
#define DLL_REMOVE(Head, Tail, Node) do {   \
    if ((Node) == (Head)) {                 \
        (Head) = (Node)->Next;              \
    }                                       \
                                            \
    if ((Node) == (Tail)) {                 \
        (Tail) = (Node)->Prev;              \
    }                                       \
                                            \
    if ((Node)->Prev) {                     \
        (Node)->Prev->Next = (Node)->Next;  \
    }                                       \
                                            \
    if ((Node)->Next) {                     \
        (Node)->Next->Prev = (Node)->Prev;  \
    }                                       \
} while (FALSE)
#define DLL_REMOVE_EX(Empty, Head, Tail, Node, _Next, _Prev) do {   \
    if ((Node) == (Head)) {                                         \
        (Head) = (Node)->_Next;                                     \
    }                                                               \
                                                                    \
    if ((Node) == (Tail)) {                                         \
        (Tail) = (Node)->_Prev;                                     \
    }                                                               \
                                                                    \
    if ((Node)->_Prev != (Empty)) {                                 \
        (Node)->_Prev->_Next = (Node)->_Next;                       \
    }                                                               \
                                                                    \
    if ((Node)->_Next != (Empty)) {                                 \
        (Node)->_Next->_Prev = (Node)->_Prev;                       \
    }                                                               \
} while (FALSE)
#define SLL_QUEUE_PUSH(Head, Tail, Node) do {   \
    if (!(Head)) {                              \
        (Head) = (Tail) = (Node);               \
        (Node)->Next = NULL;                    \
    } else {                                    \
        (Tail)->Next = (Node);                  \
        (Tail) = (Node);                        \
        (Node)->Next = NULL;                    \
    }                                           \
} while (FALSE)
#define SLL_QUEUE_PUSH_EX(Head, Tail, Node, _Next) do { \
    if (!(Head)) {                                      \
        (Head) = (Tail) = (Node);                       \
        (Node)->_Next = NULL;                           \
    } else {                                            \
        (Tail)->_Next = (Node);                         \
        (Tail) = (Node);                                \
        (Node)->_Next = NULL;                           \
    }                                                   \
} while (FALSE)
#define SLL_QUEUE_PUSH_EX_EMPTY(_NULL, Head, Tail, Node, _Next) do { \
    if ((Head) == (_NULL)) {                                         \
        (Head) = (Tail) = (Node);                                    \
        (Node)->_Next = (_NULL);                                     \
    } else {                                                         \
        (Tail)->_Next = (Node);                                      \
        (Tail) = (Node);                                             \
        (Node)->_Next = (_NULL);                                     \
    }                                                                \
} while (FALSE)
#define SLL_QUEUE_PUSH_FRONT(Head, Tail, Node) do { \
    if (!(Head)) {                                  \
        (Head) = (Tail) = (Node);                   \
        (Node)->Next = NULL;                        \
    } else {                                        \
        (Node)->Next = (Head);                      \
        (Head) = (Node);                            \
    }                                               \
} while (FALSE)
#define SLL_QUEUE_POP(Head, Tail) do {  \
    if ((Head) == (Tail)) {             \
        (Head) = (Tail) = NULL;         \
    } else {                            \
        (Head) = (Head)->Next;          \
    }                                   \
} while (FALSE)
#define SLL_STACK_PUSH(Head, Node) do { \
    (Node)->Next = (Head);              \
    (Head) = (Node);                    \
} while (FALSE)
#define SLL_STACK_PUSH_EX(Head, Node, _Next) do { \
    (Node)->_Next = (Head);                       \
    (Head) = (Node);                              \
} while (FALSE)
#define SLL_STACK_POP(Head) do {    \
    (Head) = (Head)->Next;          \
} while (FALSE)
#define SLL_STACK_POP_EX(Head, _Next) do { \
    (Head) = (Head)->_Next;                \
} while (FALSE)

#endif // __NC_DEFINES_H__
