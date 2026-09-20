#include "nc_cli.h"
#include "nc_arena.h"

API_INTERNAL Str8List
StrListFromCLIArgs(Arena* MemPool, int ArgC, char** ArgV)
{
    Str8List Result = {};

    for (u32 I = 0; I < ArgC; ++I) {
        Str8 String = CStr8(ArgV[I]);

        ListPush(MemPool, &Result, String);
    }

    return Result;
}

API_INTERNAL CommandLineOption**
CommandLineSlotFromStr(CommandLine* CLI, Str8 String)
{
    CommandLineOption** Slot = NULL;

    if (CLI->OptionTableSize) {
        u64 StringHash = Hash(String);
        u64 Bucket = StringHash % CLI->OptionTableSize;

        Slot = &CLI->OptionTable[Bucket];
    }

    return Slot;
}

API_INTERNAL CommandLineOption*
CommandLineOptionFromSlot(CommandLineOption** Slot, Str8 String)
{
    CommandLineOption* Result = NULL;

    for (CommandLineOption* Opt = *Slot; Opt; Opt = Opt->HashNext) {
        if (StrMatch(String, Opt->String, 0)) {
            Result = Opt;
            break;
        }
    }

    return Result;
}

API_INTERNAL void
CommandLinePushOption(CommandLineOptionList* List, CommandLineOption* Opt)
{
    SLL_QUEUE_PUSH(List->Head, List->Tail, Opt);
    ++List->Count;
}

API_INTERNAL CommandLineOption*
CommandLineInsertOption(Arena* MemPool, CommandLine* CLI, Str8 String, Str8List Opts)
{
    CommandLineOption* Opt = NULL;
    CommandLineOption** Slot = CommandLineSlotFromStr(CLI, String);
    CommandLineOption* FoundOpt = CommandLineOptionFromSlot(Slot, String);

    if (FoundOpt) {
        Opt = FoundOpt;
    } else {
        Opt = ArenaPushArrayZero(MemPool, CommandLineOption, 1);
        Opt->HashNext = *Slot;
        Opt->Hash = Hash(String);
        Opt->String = ArenaPushStrCpy(MemPool, String);
        Opt->ValueStrings = Opts;

        Str8JoinPart JoinPart = {};

        JoinPart.Pre = Str8Lit("");
        JoinPart.Sep = Str8Lit(",");
        JoinPart.Post = Str8Lit("");
        Opt->ValueString = StrListJoin(MemPool, &Opt->ValueStrings, &JoinPart);
        *Slot = Opt;
        CommandLinePushOption(&CLI->Options, Opt);
    }

    return Opt;
}

API_INTERNAL CommandLine
CommandLineFromStrList(Arena* MemPool, Str8List Arguments)
{
    CommandLine Result = {};

    Result.ProgramName = Arguments.Head->String;
    Result.OptionTableSize = 64;
    Result.OptionTable = ArenaPushArrayZero(MemPool, CommandLineOption*, Result.OptionTableSize);

    b32 AfterPassthroughOption = FALSE;
    b32 FirstPassthrough = TRUE;

    for (
        Str8Node* Node = Arguments.Head->Next, *Next = NULL; 
        Node; 
        Node = Next
    ) {
        Next = Node->Next;

        b32 IsOption = FALSE;
        Str8 OptionName = Node->String;

        if (!AfterPassthroughOption) {
            IsOption = TRUE;

            if (StrMatch(Node->String, Str8Lit("--"), 0)) {
                AfterPassthroughOption = TRUE;
                IsOption = FALSE;
            } else if (StrMatch(StrPrefix(Node->String, 2), Str8Lit("--"), 0)) {
                OptionName = StrSkip(OptionName, 2);
            } else if (StrMatch(StrPrefix(Node->String, 1), Str8Lit("-"), 0)) {
                OptionName = StrSkip(OptionName, 1);
        #if defined(NC_OS_WIN)
            } else if (StrMatch(StrPrefix(Node->String, 1), Str8Lit("/"), 0)) {
                OptionName = StrSkip(OptionName, 1);
        #endif
            } else {
                IsOption = FALSE;
            }
        }

        if (IsOption) {
            b32 HasValues = FALSE;
            u64 ValueSignifierPosition1 = StrFindSubStr(OptionName, Str8Lit(":"), 0, 0);
            u64 ValueSignifierPosition2 = StrFindSubStr(OptionName, Str8Lit("="), 0, 0);
            u64 ValueSignifierPosition = MIN(ValueSignifierPosition1, ValueSignifierPosition2);
            Str8 ValuePortion = StrSkip(OptionName, ValueSignifierPosition + 1);

            if (ValueSignifierPosition < OptionName.Size)
                HasValues = TRUE;

            OptionName = StrPrefix(OptionName, ValueSignifierPosition);

            Str8List Values = {};

            if (HasValues) {
                for (Str8Node* SNode = Node; SNode; SNode = SNode->Next) {
                    Next = SNode->Next;

                    Str8 String = SNode->String;

                    if (SNode == Node)
                        String = ValuePortion;

                    u8 Splits[] = { ',' };
                    Str8List ValuesInString = StrSplit(MemPool, String, Splits, ARRAY_COUNT(Splits), FALSE);

                    for (Str8Node* SubVal = ValuesInString.Head; SubVal; SubVal = SubVal->Next)
                        ListPush(MemPool, &Values, SubVal->String);

                    if (!StrMatch(StrPostfix(SNode->String, 1), Str8Lit(","), 0) && (SNode != Node || ValuePortion.Size)) {
                        break;
                    }
                }
            }

            CommandLineInsertOption(MemPool, &Result, OptionName, Values);
        } else if (!StrMatch(Node->String, Str8Lit("--"), 0) || !FirstPassthrough) {
            ListPush(MemPool, &Result.Inputs, Node->String);
            FirstPassthrough = FALSE;
        }
    }

    Result.ArgC = Arguments.Count;
    Result.ArgV = ArenaPushArrayZero(MemPool, char*, Result.ArgC);

    {
        u64 I = 0;

        for (Str8Node* Node = Arguments.Head; Node; Node = Node->Next) {
            Result.ArgV[I++] = (char*) ArenaPushStrCpy(MemPool, Node->String).Str;
        }
    }

    return Result;
}

API_INTERNAL CommandLineOption*
CommandLineOptionFromStr(CommandLine* CLI, Str8 Name)
{
    return CommandLineOptionFromSlot(CommandLineSlotFromStr(CLI, Name), Name);
}

API_INTERNAL Str8List
CommandLineStrings(CommandLine* CLI, Str8 Name)
{
    Str8List Result = {};
    CommandLineOption* Opt = CommandLineOptionFromStr(CLI, Name);

    if (Opt)
        Result = Opt->ValueStrings;

    return Result;
}

API_INTERNAL Str8
CommandLineString(CommandLine* CLI, Str8 Name)
{
    Str8 Result = {};
    CommandLineOption* Opt = CommandLineOptionFromStr(CLI, Name);

    if (Opt)
        Result = Opt->ValueString;

    return Result;
}

API_INTERNAL b32
CommandLineHasFlag(CommandLine* CLI, Str8 Name)
{
    CommandLineOption* Opt = CommandLineOptionFromStr(CLI, Name);

    return !!Opt;
}

API_INTERNAL b32
CommandLineHasArgument(CommandLine* CLI, Str8 Name)
{
    CommandLineOption* Opt = CommandLineOptionFromStr(CLI, Name);

    return (!!Opt && Opt->ValueStrings.Count);
}
