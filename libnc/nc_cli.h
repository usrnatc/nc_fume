#if !defined(__NC_CLI_H__)
#define __NC_CLI_H__

#include "nc_types.h"
#include "nc_string.h"

// @types______________________________________________________________________
struct CommandLineOption {
    CommandLineOption* Next;
    CommandLineOption* HashNext;
    u64 Hash;
    Str8 String;
    Str8List ValueStrings;
    Str8 ValueString;
};

struct CommandLineOptionList {
    CommandLineOption* Head;
    CommandLineOption* Tail;
    u64 Count;
};

struct CommandLine {
    Str8                  ProgramName;
    CommandLineOptionList Options;
    Str8List              Inputs;
    u64                   OptionTableSize;
    CommandLineOption**   OptionTable;
    u64                   ArgC;
    char**                ArgV;
};

// @functions__________________________________________________________________
Str8List StrListFromCLIArgs(Arena* MemPool, int ArgC, char** ArgV);
CommandLineOption** CommandLineSlotFromStr(CommandLine* CLI, Str8 String);
CommandLineOption* CommandLineOptionFromSlot(CommandLineOption** Slot, Str8 String);
void CommandLinePushOption(CommandLineOptionList* List, CommandLineOption* Opt);
CommandLineOption* CommandLineInsertOption(Arena* MemPool, CommandLine* CLI, Str8 String, Str8List Opts);
CommandLine CommandLineFromStrList(Arena* MemPool, Str8List Arguments);
CommandLineOption* CommandLineOptionFromStr(CommandLine* CLI, Str8 Name);
Str8List CommandLineStrings(CommandLine* CLI, Str8 Name);
Str8 CommandLineString(CommandLine* CLI, Str8 Name);
b32 CommandLineHasFlag(CommandLine* CLI, Str8 Name);
b32 CommandLineHasArgument(CommandLine* CLI, Str8 Name);

#endif // __NC_CLI_H__
