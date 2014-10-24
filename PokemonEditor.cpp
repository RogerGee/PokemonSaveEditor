#include "MasterInclude.h"
#include "resource.h"
using namespace rtypes;
using namespace rtypes::rwin32;
using namespace PokemonEditor;
using namespace PokemonEditorWindow;

#define VDEBUG

#ifdef VDEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

struct com_arg // wraps a string
{
    str value;
};

rstringstream& operator >>(rstringstream& stream,com_arg& arg);

int __stdcall WinMain(  HINSTANCE hInst,
                        HINSTANCE hPrevInst,
                        LPSTR CmdLine,
                        int CmdShow )
{
#ifdef VDEBUG
    _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
    //_CrtSetBreakAlloc(16205);
#endif
    Application app;
    int rVal;
    
    app.RegisterApplication(hInst,
        SimpleResource(IDI_ICON1,true),
        SimpleResource(IDI_ICON1,true),
        SimpleResource());

    // load calls
    PokMath::SeedRandomGenerator();
    LoadPokemonDataConstants();
    baseDialog::InitFonts();

    // create main window
    MainWindow theWindow;
    app.SetAsMainWindow(theWindow); // make this the main window on the main thread

    // parse command line
    /*
        unless a switch (/) was specified the command-line is assumed to be
            <load-file-non-opt_filename> [save-kind-opt_Cur|Prev] [view-entry-kind_EntryType:view-entry-occur_EntryOccur]

            load-file-non-opt: a battery file to load
            save-kind-opt: a flag; "cur" OR "prev" designating current or previous save
            view-entry-kind: a specified entry type to load
                -EntryType: can be "party" "daycare" or "pcbox"
            view-entry-occur: the specified entry occurance to load of the entry type (all indexes are 1-based for user convinience)
                -EntryOccur: can be a numeric index value or "pcbox:#:#" for pc entries (if no "pcbox:#" is specified for PC entries, an index still is expected; it will be relative to all pokemon in the pc)

            example command lines
                PokemonEditor battery.sav cur party:1
                PokemonEditor battery.sav prev pcbox:

            (switches - tokens following switches must be separated by space OR a colon)
            /? - display help
            /help - same as /?
            /entry <load-file-non-opt:filename> - load a startup entry that isn't associated with a battery file
            /item:<item-name-non-opt:itemName OR item-index-non-opt:itemIndex> - display either the item index or name associated with the name or index respectively
            /pok:<pok-name-non-opt:pokName OR pok-index-non-opt:pokIndex> - display either the pokemon index or name associated with the name or index respectively
            /move <move-name-non-opt:
    */
    rstringstream argParser;
    argParser << CmdLine;
    if (argParser.has_input())
    {
        com_arg arg, otherArg;
        if (argParser.get_device()[0]=='/')
        {// is a switch
            argParser.add_extra_delimiter('/');
            argParser.add_extra_delimiter(':');
            argParser >> arg;
            Misc::to_lower(arg.value);
            if (arg.value=="?" || arg.value=="help")
            {
                // send a help click to the main window
                // to start up documentation
                
            }
            else if (arg.value=="entry")
            {
                argParser >> otherArg; // read filename

            }
        }
        else
        {// try loading battery file
            argParser >> arg;
            argParser.add_extra_delimiter(':');
            argParser >> otherArg;
            Misc::to_lower(otherArg.value);
            theWindow.SetFileState( otherArg.value=="prev" ? PreviousState : CurrentState );
            if (!theWindow.LoadBatteryFile(arg.value))
            {
                YesNoCancel_dialog dialog("Loading failed. Do you still want to launch PokemonEditor?");
                DialogWindow::DialogResult r = dialog.ShowDlg();
                if (r!=DialogWindow::Yes)
                    return 1;
            }
            else if (otherArg.value.length()>0)
            {// check other elements of the command-line (they are all optional)
                dword occur;
                PokemonEntryType t = ErrorPok;
                bool error = false;
                rstringstream numericParser;
                argParser >> arg; // read in occur index
                Misc::to_lower(arg.value);
                numericParser.open(arg.value);
                if ( (numericParser >> occur) ) // attempt to read numeric from
                {
                    --occur; // 'zero' the occur index
                    if (otherArg.value=="party")
                        t = PartyPok;
                    else if (otherArg.value=="daycare")
                        t = DayCarePok;
                    else if (otherArg.value=="pcbox")
                    {// format: pcbox:box#:boxentry#
                        dword var;
                        t = PCPok;
                        argParser >> otherArg; // read in other occur index part
                        occur *= 30; // 30 poks per PC box
                        numericParser.set_input_iter(0);
                        if ( numericParser >> var )
                            occur += var-1;
                        else
                            error = true;
                    }
                    // try to open the entry
                    if (!error)
                        error = !theWindow.LoadEntry(t,occur);
                }
                else
                    error = true;
                if (error)
                {
                    Message_dialog dialog("An entry to load was specified on the command-line, however the syntax was either incomplete or incorrect.");
                    dialog.ShowDlg();
                }
            }
        }
    }

    // run main window
    rVal = app.RunWindow(CmdShow);

    // destroy calls
    baseDialog::DestroyFonts();
    PokemonEditorConsole::ConsoleCleanup(); // safely quit the message thread if it is running

    return rVal;
}

rstringstream& operator >>(rstringstream& stream,com_arg& arg)
{// get the logical commnad-line argument (enclosed in quotes, else by whitespace)
    if (stream.peek()=='"')
    {
        arg.value.clear(); // do this here just in case (else clause is handled by rstringstream)
        stream.get();
        while (true)
        {
            char c = stream.get();
            if (c=='"')
                break;
            arg.value.push_back(c);
        }
    }
    else
        stream >> arg.value;
    return stream;
}