#include "RWin32LibMaster.h"
using namespace rtypes;
using namespace rtypes::rwin32;

Application::_mainWindow::_mainWindow(const Window* w)
{
    wndPtr = w;
    threadID = threading::Thread::GetCurrentThreadID();
}

Application::Application()
{
    _init = false;
    _hInst = NULL;
    // set the thread that made the application object
    // this thread is considered to be the application thread
    _mainThreadID = threading::Thread::GetCurrentThreadID();
    //set command line information
    str cmd_line = GetCommandLineA();
    for (dword i = 0;i<cmd_line.size();i++)
    {
        str next;
        for (byte quo = 0;i<cmd_line.size() && (quo || cmd_line[i]!=' ');i++)
            if (cmd_line[i]=='"')
                quo = ~quo;
            else
                next += cmd_line[i];
        _commandLine.push_back(next);
    }
}
bool Application::SetAsMainWindow(const Window& MainWnd)
{
    if (GetMainAppWindow() /* there is already a main app window for the thread */
         || MainWnd.MyType()!=FrameWindowObject /* only frame windows can become main windows */)
        return false;
    _mainWindows.push_back( _mainWindow(&MainWnd) );
    _mainWindows.insertion_sort(); // keep 'em sorted
    return true;
}
const Window* Application::GetMainAppWindow() const
{
    dword i;
    if (_mainWindows.binary_search( _mainWindow(threading::Thread::GetCurrentThreadID()),i ))
        return _mainWindows[i].wndPtr;
    return NULL;
}

int Application::RunWindow(int CmdShow)
{
    MSG msg;
    const Window* mainAppWindow = GetMainAppWindow();
    if (!_init || !mainAppWindow || !mainAppWindow->Show(CmdShow))
        return -1;
    while (GetMessage(&msg,NULL,0,0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    _removeMainWindow();
    if (_mainWindows.size() // there are other main windows that haven't stopped their message loops
        && threading::Thread::GetCurrentThreadID()==_mainThreadID) // only check for other threads if this is the main thread
    {
        // wait for each of the other threads to close to prevent leaking their memory
        while (_mainWindows.size())
        {
            HANDLE hThread = OpenThread(THREAD_ALL_ACCESS,FALSE,_mainWindows[0].threadID);
            if (!hThread)
            {
                _mainWindows.remove_at(0); // can't wait on it
                continue;
            }
            WaitForSingleObject(hThread,INFINITE);
        }
    }
    return (int) msg.wParam;
}
void Application::QuitWindow(int ReturnValue)
{
    PostQuitMessage(ReturnValue);
}
str Application::GetExecutablePath() const
{
    str path(MAX_PATH);
    dword sz, i;
    sz = GetModuleFileName(NULL,&path[0],MAX_PATH);
    path.truncate(sz); // do this to allow the string to accurately report size information
    i = path.size()-1;
    while (i>0 && path[i]!='\\')
        i--;
    // cut the string to include just the path, not the executable name
    path.truncate(i);
    return path; // the copy operation should trim the data usage down to include just the path
}
bool Application::_removeMainWindow()
{
    dword i;
    if (_mainWindows.binary_search( _mainWindow(threading::Thread::GetCurrentThreadID()),i ))
    {
        _mainWindows.remove_at(i);
        return true;
    }
    return false;
}