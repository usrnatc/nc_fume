@echo off
setlocal enabledelayedexpansion

set BuildMode=debug
set SubSystem=/SUBSYSTEM:CONSOLE
set EntryPoint=/ENTRY:wmainCRTStartup

for %%a in (%*) do (
    set Arg=%%~a

    if /I "!Arg!"=="release" set BuildMode=release
    if /I "!Arg!"=="dev"     set BuildMode=dev
)

if not exist build mkdir build
pushd build

if not exist msvcrt.lib (
    echo.[33m____generating_msvcrt.lib____[0m

    set MsvcrtDef=msvcrt.def
    set MsvcrtDllPath=%SystemRoot%\System32\msvcrt.dll

    echo EXPORTS>!MsvcrtDef!
    for /f "skip=19 tokens=4" %%i in ('DUMPBIN /EXPORTS "!MsvcrtDllPath!" ^| findstr /r "^^  *[0-9][0-9]* "') do (
        echo %%i>>!MsvcrtDef!
    )
    lib.exe /nologo /DEF:!MsvcrtDef! /OUT:msvcrt.lib /MACHINE:x64

    if %ERRORLEVEL% neq 0 (
        echo. \e[31mFailed to generate msvcrt.lib. \e[0m
        popd
        exit /b 1
    )
)

set CHKSTKASMSSourceFiles=..\libnc\internal\win32\win32_chkstk.asm
set BSWAPASMSourceFiles=..\libnc\internal\win32\win32_bswap.asm
set MATHASMSourceFiles=..\libnc\internal\win32\win32_math.asm
set INTRINASMSourceFiles=..\libnc\internal\win32\win32_intrin.asm

set ASMFlags=/c /nologo /quiet

set CHKSTKASMOutputObj=win32_chkstk.obj
set BSWAPASMOutputObj=win32_bswap.obj
set MATHASMOutputObj=win32_math.obj
set INTRINASMOutputObj=win32_intrin.obj

set SourceFiles=..\fume.cpp

set OutputExe=fume.exe
set OutputPdb=fume.pdb
set OutputMap=fume.map

set SharedCompilerFlags=^
    /nologo /FC /WX /W4 /GR- /Gm- /EHa- /EHsc- /Zl ^
    /arch:AVX2 ^
    /fp:strict /fp:except- ^
    /GS- /guard:cf- /guard:ehcont- ^
    /analyze- ^
    /diagnostics:caret /MP32 ^
    /I"..\libnc" /I"..\libnc\external" /I"..\libnc\internal" /I"..\libnc\cache"^
    /DUNICODE /D_NO_CRT_STDIO_INLINE /D_CRT_SECURE_NO_WARNINGS ^
    /wd4018 /wd4100 /wd4189 /wd4200 /wd4201 /wd4211 /wd4244 ^
    /wd4245 /wd4334 /wd4456 /wd4505 /wd4714 /wd4146 /wd4716

set SharedLinkerFlags=^
    /MANIFEST:EMBED %SubSystem% /NOEXP /NODEFAULTLIB ^
    /MANIFESTDEPENDENCY:"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595B64144CCF1DF' language='*'" ^
    %EntryPoint% /time /INCLUDE:_tls_used ^
    /alternatename:memset=MemSet /alternatename:memcpy=MemCpy ^
    /ALIGN:64 /MERGE:.CRT=.rdata /MERGE:.rdata=.text ^
    kernel32.lib user32.lib gdi32.lib shell32.lib ole32.lib ^
    comdlg32.lib dwmapi.lib msvcrt.lib ws2_32.lib advapi32.lib shlwapi.lib ^
    dbghelp.lib d3dcompiler.lib d3d11.lib comctl32.lib dwrite.lib ^
    crypt32.lib secur32.lib winhttp.lib winmm.lib

set ConfigCompilerFlags=
set ConfigLinkerFlags=
set PostBuildCmd=

if /I "%BuildMode%"=="debug"   goto :build_debug
if /I "%BuildMode%"=="release" goto :build_release
if /I "%BuildMode%"=="dev"     goto :build_dev
goto :invalid_mode

REM  /D_DEBUG /DNC_DEBUG=1 /Zi

:build_debug
    echo.[33m____building_debug___________[0m
    set ConfigCompilerFlags=/Od /Oy- /Zi /d2Zi+ /D_DEBUG /DNC_DEBUG=1 /DNC_PROFILE=1
    set ConfigLinkerFlags=/DEBUG:FULL /INCREMENTAL:NO
    set PostBuildCmd=%OutputExe%
    goto :build

:build_release
    echo.[33m____building_release_________[0m
    set ConfigCompilerFlags=/O2 /Oi /Os /GL /Gy /GF /GA /Ob3 /Qpar /Qvec-report:2 /DNDEBUG
    set ConfigLinkerFlags=/LTCG /NOCOFFGRPINFO /EMITTOOLVERSIONINFO:NO /OPT:REF /OPT:ICF /INCREMENTAL:NO
    set PostBuildCmd=%OutputExe%
    goto :build

:build_dev
    echo.[33m____building_dev__debug______[0m
    set ConfigCompilerFlags=/Od /Oy- /Zi /d2Zi+ /D_DEBUG
    set ConfigLinkerFlags=/DEBUG:FULL /INCREMENTAL:NO
    set PostBuildCmd=devenv /DebugExe %OutputExe%
    goto :build

:build
    echo.[33m____deleting_build_data______[0m
    del %OutputPdb%> NUL 2> NUL
    del %OutputExe%> NUL 2> NUL
    del *.obj> NUL 2> NUL
    del nc_log.txt> NUL 2> NUL

    echo.[33m____assembling_win32_asms____[0m
    ml64.exe %ASMFlags% %CHKSTKASMSSourceFiles% /Fo%CHKSTKASMOutputObj%

    if not exist %CHKSTKASMOutputObj% (
        echo.[31mFailed to generate win32_chkstk.obj.[0m
        popd
        exit /b 1
    )

    ml64.exe %ASMFlags% %BSWAPASMSourceFiles% /Fo%BSWAPASMOutputObj%

    if not exist %BSWAPASMOutputObj% (
        echo.[31mFailed to generate win32_bswap.obj.[0m
        popd
        exit /b 1
    )

    ml64.exe %ASMFlags% %MATHASMSourceFiles% /Fo%MATHASMOutputObj%

    if not exist %MATHASMOutputObj% (
        echo.[31mFailed to generate win32_math.obj.[0m
        popd
        exit /b 1
    )

    ml64.exe %ASMFlags% %INTRINASMSourceFiles% /Fo%INTRINASMOutputObj%

    if not exist %INTRINASMOutputObj% (
        echo.[31mFailed to generate win32_intrin.obj.[0m
        popd
        exit /b 1
    )

    echo.[33m____compiling_and_linking____[0m
    cl.exe %SharedCompilerFlags% %ConfigCompilerFlags% %SourceFiles% %CHKSTKASMOutputObj% %BSWAPASMOutputObj% %MATHASMOutputObj% %INTRINASMOutputObj% /Fm%OutputMap% /link %SharedLinkerFlags% %ConfigLinkerFlags% /OUT:%OutputExe% /PDB:%OutputPdb%

    set LastError=%ERRORLEVEL%
    if not %LastError% == 0 (
        echo.
        echo.[31mBUILD FAILED. [0m
        popd 
        exit /b %LastError%
    )

    echo.[32m____finished_________________[0m

    if not "%PostBuildCmd%"=="" (
        echo.[33m____running_post_build_______[0m
        %PostBuildCmd%
    )
    goto :end

:invalid_mode
    echo.[31mInvalid build mode: %BuildMode% [0m
    echo.[31mValid modes are any ONE ^(1^) of: debug, release, dev [0m
    popd
    exit /b 1

:end
popd
endlocal
