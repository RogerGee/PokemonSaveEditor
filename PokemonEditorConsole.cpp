#include "MasterInclude.h"
using namespace rtypes;
using namespace rtypes::rwin32;
using namespace rtypes::rwin32::console;
using namespace PokemonEditor;
using namespace PokemonEditorWindow;

namespace // globals
{
    // forward declares for functions in this anon-namespace
    dword PokemonEditorConsoleMessageLoop(void*);

    // global data
    bool isInit = false;
    ConsoleBuffer conBuf;
    PokemonSaveFile* pSaveFile = NULL;
    MainWindow* pMainWnd = NULL;
    threading::Thread consoleThread;

    volatile bool ForceQuitConsoleMessageThread = false;
    dword PokemonEditorConsoleMessageLoop( void* pParam )
    {
        dword rCode = 0;
        Window conWnd = ::GetConsoleWindow();
        str prompt = "<>"; // print a space after the prompt
        do
        {
            rstringstream lineStream;
            str commandLine, command;
            // print the prompt
            theConsole << prompt << ' ';
            // read command line
            theConsole.getline(commandLine);
            lineStream.open(commandLine);
            lineStream.add_extra_delimiter(':');
            lineStream.add_extra_delimiter('.');
            lineStream.add_extra_delimiter('=');
            lineStream >> command;
            Misc::to_lower(command);
            if (command=="hide")
            {
                if (pMainWnd!=NULL)
                    pMainWnd->SimulateClick(HideConsoleClick);
            }
            else if (command=="minimize")
                conWnd.Show(SW_MINIMIZE);
            else if (command=="maximize")
                conWnd.Show(SW_MAXIMIZE);
            else if (command=="restore")
                conWnd.Show(SW_RESTORE);
            else if (command=="cls" || command=="clear")
            {
                theConsole.clear();
                conBuf.SetCursorLocation(0,0);
            }
            else if (command=="quit")
            {
                if (pMainWnd!=NULL)
                    pMainWnd->SimulateClick(RequestCloseClick);
                Sleep(100); // give the window a chance to quit
                if (::IsWindow(conWnd.WinHandle()))
                { // the window didn't close, most likely because the main window or one of its children canceled the quit operation
                    theConsole << "The main window canceled the quit operation.\n";
                }
            }
            
            else
                theConsole << "The specified command-line could not be interpreted." << endline;
        } while (!ForceQuitConsoleMessageThread);
        return rCode;
    }
}

bool PokemonEditorConsole::IsConsoleInit()
{
    return isInit;
}
void PokemonEditorConsole::InitConsole()
{
    static const Size BUF_SIZE(100,2000);
    BOOL allocResult = ::AllocConsole();
    Window consoleWindow;
    if (!isInit && allocResult)
    {
        consoleWindow = ::GetConsoleWindow();
        consoleWindow.SetText("PokemonEditor Command-Line Window");
        consoleWindow.SetAttribute(SystemMenu,true);
        // destroy the buffers that Windows gave us and use our own
        // conBuf most likely has a bad buffer handle, seeing as a console was not allocated when its
        // constructor ran; so, I create a temporary buffer and assign it to conBuf, conBuf will create a 
        // new buffer modeled after the temporary one. It's simply a workaround.
        conBuf = ConsoleBuffer();
        conBuf.SetBufferSize(BUF_SIZE);
        conBuf.Show();
        CloseHandle( GetStdHandle(STD_OUTPUT_HANDLE) );
        CloseHandle( GetStdHandle(STD_ERROR_HANDLE) );
        SetStdHandle(STD_OUTPUT_HANDLE,conBuf.GetConsoleBufferHandle());
        SetStdHandle(STD_ERROR_HANDLE,conBuf.GetConsoleBufferHandle());
        theConsole.get_device().ReloadFromStdHandles();
        // start up the console thread
        consoleThread.Create(&::PokemonEditorConsoleMessageLoop);
        consoleThread.Run();
    }
    else
        consoleWindow = ::GetConsoleWindow();
    if (consoleWindow) // console has already been allocated, simply change state
    {
        isInit = true;
        consoleWindow.Show();
    }
}
void PokemonEditorConsole::SetDataForConsole(PokemonSaveFile* pSave,MainWindow* pWnd)
{
    pSaveFile = pSave;
    pMainWnd = pWnd;
}
void PokemonEditorConsole::FocusConsole()
{
    if (isInit)
    {
        Window consoleWindow = ::GetConsoleWindow();
        if (consoleWindow)
        {
            consoleWindow.Show(SW_RESTORE);
            ::BringWindowToTop(consoleWindow.WinHandle());
        }
    }
}
void PokemonEditorConsole::PackUpConsole() // just hide the console until the user calls init again
{
    if (isInit)
    {
        Window consoleWindow = ::GetConsoleWindow();
        consoleWindow.Show(SW_HIDE);
        isInit = false;
    }
}
void PokemonEditorConsole::ConsoleCleanup()
{
    if (consoleThread.IsActive())
    {
        ForceQuitConsoleMessageThread = true;
        ::FreeConsole(); // this cancels pending io operations
        ::WaitForSingleObject(consoleThread.GetThreadHandle(),INFINITE);
    }
}