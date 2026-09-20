#!/bin/bash

YELLOW='\033[1;33m'
RED='\033[0;31m'
GREEN='\033[0;32m'
RESET='\033[0;0m'

BuildMode="debug"

for arg in "$@"; do
    case "$(echo $arg | tr '[:upper:]' '[:lower:]')" in
        release)
            BuildMode="release"
            ;;
        dev)
            BuildMode="dev"
            ;;
    esac
done

mkdir -p build
pushd build > /dev/null

MATHASMSourceFiles="../libnc/internal/linux/linux_math.asm"
INTRINASMSourceFiles="../libnc/internal/linux/linux_intrin.asm"
ASMFlags="-f elf64"
MATHASMOutputObj=linux_math.o
INTRINASMOutputObj=linux_intrin.o

SourceFiles="../fume.cpp"
OutputBin="fume"

SharedCompilerFlags=(
    "-std=c++14"
    "-w"
    "-fno-rtti"
    "-fno-exceptions"
    "-fpermissive"
    "-mavx2"
    "-mssse3"
    "-msse4.1"
    "-msse4.2"
    "-mavx"
    "-msse"
    "-msse2"
    "-mbmi"
    "-mbmi2"
    "-fno-stack-protector"
    "-fno-strict-aliasing"
    "-fno-stack-protector"
    "-fcf-protection=none"
    "-fno-sanitize=all"
    "-fno-threadsafe-statics"
    "-fno-use-cxa-atexit"
    "-fno-implicit-templates"
    "-fno-asynchronous-unwind-tables"
    "-fno-unwind-tables"
    "-fno-exceptions"
    "-I../libnc/external"
    "-I../libnc/internal"
    "-I../libnc/cache"
    "-I../libnc"
    "-DUNICODE"
    "-D_GNU_SOURCE"
)

WarningFlags=(
    "-Wno-sign-compare"
    "-Wno-unused-parameter"
    "-Wno-unused-function"
    "-Wno-unused-parameter"
    "-Wno-unused-variable"
    "-Wno-unused-function"
    "-Wno-sign-compare"
    "-Wno-missing-field-initializers"
    "-Wno-missing-braces"
    "-Wno-unused-but-set-variable"
    "-Wno-type-limits"
    "-Wno-switch"
    "-Wno-return-type"
    "-Wno-parentheses"
    "-Wno-unknown-pragmas"
    "-Wno-attributes"
    "-Wno-write-strings"
)

SharedLinkerFlags=(
    "-lpthread"
    "-ldl"
    "-lm"
    "-static-libgcc"
    "-static-libstdc++"
    "-Wl,-Bstatic -lstdc++"
    "-Wl,-Bdynamic -ldl"
)

case "$BuildMode" in
    debug)
        echo -e "${YELLOW}____building_debug___________${RESET}"
        ConfigCompilerFlags="-O0 -g -D_DEBUG -DNC_DEBUG=1"
        PostBuildCmd="./$OutputBin"
        ;;
    release)
        echo -e "${YELLOW}____building_release_________${RESET}"
        ConfigCompilerFlags="-O3 -DNDEBUG"
        PostBuildCmd="./$OutputBin"
        ;;
    dev)
        echo -e "${YELLOW}____building_dev__debug______${RESET}"
        ConfigCompilerFlags="-O0 -g -pg -D_DEBUG -DNC_DEBUG=1"
        PostBuildCmd="gdb ./$OutputBin"
        ;;
    *)
        echo -e "${RED}Invalid build mode: $BuildMode${RESET}"
        popd > /dev/null
        exit 1
        ;;
esac

echo -e "${YELLOW}____deleting_build_data______${RESET}"
rm -f "$OutputBin"
rm -f *.o

echo -e "${YELLOW}____assembling_linux_asms____${RESET}"
nasm "$ASMFlags" "$MATHASMSourceFiles" -o "$MATHASMOutputObj"
nasm "$ASMFlags" "$INTRINASMSourceFiles" -o "$INTRINASMOutputObj"

echo -e "${YELLOW}____compiling_and_linking____${RESET}"
g++ ${SharedCompilerFlags[@]} ${WarningFlags[@]} $ConfigCompilerFlags \
    $SourceFiles $MATHASMOutputObj $INTRINASMOutputObj \
    -o $OutputBin \
    ${SharedLinkerFlags[@]}

LastError=$?
if [ $LastError -ne 0 ]; then
    echo -e "\n${RED}BUILD FAILED.${RESET}"
    popd > /dev/null
    exit $LastError
fi

echo -e "${GREEN}____finished_________________${RESET}"

if [ -n "$PostBuildCmd" ] && [ "$BuildMode" != "dev" ]; then
    echo -e "${YELLOW}____running_post_build_______${RESET}"
    $PostBuildCmd
elif [ "$BuildMode" == "dev" ]; then
    echo -e "${YELLOW}____dev mode: run 'gdb ./$OutputBin' to debug____${RESET}"
fi

popd > /dev/null
