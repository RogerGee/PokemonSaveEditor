#include "RWin32LibMaster.h"
#include "RQueue.h"
using namespace rtypes;
using namespace rtypes::rwin32;
using namespace rtypes::rwin32::threading;

namespace
{
    // thread tracking system
    priority_queue<HANDLE,int> openThreads;
    dword DestroyThreadResources( void* vp )
    {
        // wait for each of the threads in the queue
        while ( !openThreads.is_empty() )
        {
            HANDLE hThread = openThreads.pop_back();
            WaitForSingleObject(hThread,INFINITE);
            CloseHandle(hThread);
        }
        return 0;
    }
    Thread threadTrack;

    struct ProcessHandles
    {
        ProcessHandles() : hProcess(INVALID_HANDLE_VALUE),hThread(INVALID_HANDLE_VALUE) {}
        ProcessHandles(HANDLE handleProcess,HANDLE handleThread) : hProcess(handleProcess),hThread(handleThread) {}
        HANDLE hProcess,hThread;
    };
    queue<ProcessHandles> openProcesses;
    dword DestroyProcessResources( void* vp )
    {
        while ( !openProcesses.is_empty() )
        {
            ProcessHandles processHandles = openProcesses.pop_back();
            WaitForSingleObject(processHandles.hProcess,INFINITE);
            CloseHandle(processHandles.hProcess);
            CloseHandle(processHandles.hThread);
        }
        return 0;
    }
    Thread processTrack;
}

const dword threading::ExitNormal = 0;
const dword threading::ExitForced = -1;
const dword threading::ExitError = -2;

Thread::Thread()
{
    _hThread = NULL;
    _handleID = 0;
    _myCompletionPriority = 0;
    autoKill = false;
}
Thread::Thread(HANDLE hThread,dword ThreadID,int CompletionPriority)
{
    _hThread = hThread;
    if (ThreadID==0)
        _handleID = ::GetThreadId(hThread);
    else
        _handleID = ThreadID;
    _myCompletionPriority = CompletionPriority;
    autoKill= false;
}
Thread::~Thread()
{
    if (autoKill)
    {
        if (IsActive())
            Kill();
        Close();
    }
    else if (IsActive() && _myCompletionPriority!=-1)
    {
        // the object is passing out of scope and the thread is running
        // start the thread tracking system so that thread resources can be disposed of
        openThreads.push_back(_hThread,_myCompletionPriority);
        if (!threadTrack.IsActive())
        {
            // create the thread with a completion priority of -1,
            // which means that the thread is not to be added to the thread track system
            threadTrack.Create(&DestroyThreadResources,-1);
            threadTrack.Run();
        }
    }
    else if (_hThread)
        Close(); // we can safely dispose of the thread object
}
bool Thread::Create(ThreadProcedure EntryPoint,int ThreadPriority)
{
    if (IsActive())
        return false; // can't start another thread with this object
    else if (_hThread)
        // clear object
        Close();
    // create the thread
    _hThread = CreateThread(NULL,
        NULL,
        (LPTHREAD_START_ROUTINE) EntryPoint,
        NULL,
        CREATE_SUSPENDED, // create suspended so the user can start it up later
        &_handleID);
    if (_hThread)
        // remember the completion priority
        _myCompletionPriority = ThreadPriority;
    return _hThread!=NULL;
}
bool Thread::Run()
{
    if (_hThread==NULL)
        return false;
    while ( ResumeThread(_hThread)>0 ) // suspend count of zero means running
        if (ResumeThread(_hThread)==-1)
            return false;
    if (ResumeThread(_hThread)==-1)
        return false;
    return true;
}
bool Thread::RunWait(dword &ExitCode_out)
{
    if (!Run())
        return false;
    else if (WaitForSingleObject(_hThread,INFINITE)==WAIT_FAILED)
    {
        Close();
        return false;
    }
    Close(); // the thread is spent and needs to be closed
    return true;
}
bool Thread::Close()
{
    if (CloseHandle(_hThread))
    {
        _hThread = NULL;
        _handleID = 0;
        return true;
    }
    return false;
}
bool Thread::IsActive() const
{
    if (_hThread==NULL)
        return false;
    DWORD code;
    if (GetExitCodeThread(_hThread,&code) && code==STILL_ACTIVE)
        return true;
    return false;
}
dword Thread::GetThreadExitCode() const
{
    DWORD code;
    if (GetExitCodeThread(_hThread,&code))
        return code;
    return NULL;
}

Process::Process()
{
    ZeroMemory(&_startupInfo,sizeof(STARTUPINFO));
    ZeroMemory(&_processInfo,sizeof(PROCESS_INFORMATION));
    _startupInfo.cb = sizeof(STARTUPINFO);
    _hRedirectOutput = INVALID_HANDLE_VALUE;
    _hRedirectError = INVALID_HANDLE_VALUE;
    _hRedirectInput = INVALID_HANDLE_VALUE;
}
Process::Process(const str& ApplicationName,const str& CommandLine)
{
    ZeroMemory(&_startupInfo,sizeof(STARTUPINFO));
    ZeroMemory(&_processInfo,sizeof(PROCESS_INFORMATION));
    _startupInfo.cb = sizeof(STARTUPINFO);
    _hRedirectOutput = INVALID_HANDLE_VALUE;
    _hRedirectError = INVALID_HANDLE_VALUE;
    _hRedirectInput = INVALID_HANDLE_VALUE;
    _applicationName = ApplicationName;
    _commandLine = CommandLine;
}
Process::~Process()
{
    if (IsProcessRunning())
    {
        // add the process handle to the process handle track system
        // so that its resources can be closed later on
        openProcesses.push_back( ProcessHandles(GetProcessHandle(),GetProcessMainThreadHandle()) );
        if (!processTrack.IsActive())
        {
            // start the process track thread
            processTrack.Create(&DestroyProcessResources,-1);
            processTrack.Run();
        }
    }
    else if ( GetProcessHandle() )
    {
        // close the process handle and its primary thread handle
        CloseHandle( GetProcessHandle() );
        CloseHandle( GetProcessMainThreadHandle() );
    }
}
void Process::RedirectHandles(HANDLE hOut,HANDLE hErr,HANDLE hIn)
{
    _hRedirectOutput = hOut;
    _hRedirectError = hErr;
    _hRedirectInput = hIn;
}
bool Process::RunProcess(bool InheritHandles)
{
    if (IsProcessRunning())
        return false;
    else if (_processInfo.hProcess)
    {
        CloseHandle( GetProcessHandle() );
        CloseHandle( GetProcessMainThreadHandle() );
        // overwrite old process info and startup info
        ZeroMemory(&_processInfo,sizeof(PROCESS_INFORMATION));
        ZeroMemory(&_startupInfo,sizeof(STARTUPINFO));
    }
    // create startup info
    _startupInfo.cb = sizeof(STARTUPINFO);
    if (InheritHandles && (_hRedirectOutput!=INVALID_HANDLE_VALUE || _hRedirectError!=INVALID_HANDLE_VALUE || _hRedirectInput!=INVALID_HANDLE_VALUE))
    {
        _startupInfo.dwFlags = STARTF_USESTDHANDLES;
        _startupInfo.hStdError = _hRedirectError;
        _startupInfo.hStdInput = _hRedirectInput;
        _startupInfo.hStdOutput = _hRedirectOutput;
    }
    // try to create and run the process
    fileIO::File parser; // use a file object to parse safe name
    parser.SetFilePathName_parser(_applicationName);
    str command = parser.GetSafeName_parser(false);
    command.push_back(' ');
    command += _commandLine;
    if (!CreateProcess(_applicationName.c_str(),
        &command[0],
        NULL,
        NULL,
        TRUE,
        0,
        NULL,
        NULL,
        &_startupInfo,
        &_processInfo))
        return false;
    return true;
}
bool Process::RunProcessWait(int &ReturnCode_out,bool InheritHandles)
{
    if (IsProcessRunning())
        return false;
    else if (_processInfo.hProcess)
    {
        CloseHandle( GetProcessHandle() );
        CloseHandle( GetProcessMainThreadHandle() );
        // overwrite old process info and startup info
        ZeroMemory(&_processInfo,sizeof(PROCESS_INFORMATION));
        ZeroMemory(&_startupInfo,sizeof(STARTUPINFO));
    }
    // create startup info
    _startupInfo.cb = sizeof(STARTUPINFO);
    if (InheritHandles && (_hRedirectOutput!=INVALID_HANDLE_VALUE || _hRedirectError!=INVALID_HANDLE_VALUE || _hRedirectInput!=INVALID_HANDLE_VALUE))
    {
        _startupInfo.dwFlags = STARTF_USESTDHANDLES;
        _startupInfo.hStdError = _hRedirectError;
        _startupInfo.hStdInput = _hRedirectInput;
        _startupInfo.hStdOutput = _hRedirectOutput;
    }
    // try to create and run the process
    fileIO::File parser; // use a File object to parse safe name
    parser.SetFilePathName_parser(_applicationName);
    str command = parser.GetSafeName_parser(false); // first element in command line is the application name
    command.push_back(' ');
    command += _commandLine;
    if (!CreateProcess(_applicationName.c_str(),
        &command[0],
        NULL,
        NULL,
        TRUE,
        0,
        NULL,
        NULL,
        &_startupInfo,
        &_processInfo))
            return false;
    DWORD r = WaitForSingleObject(GetProcessHandle(),INFINITE);
    ReturnCode_out = GetProcessReturnCode();
    // close handles
    CloseHandle(_processInfo.hProcess);
    CloseHandle(_processInfo.hThread);
    // overwrite old process info and startup info
    ZeroMemory(&_processInfo,sizeof(PROCESS_INFORMATION));
    ZeroMemory(&_startupInfo,sizeof(STARTUPINFO));
    return r!=WAIT_FAILED;
}
bool Process::KillProcess(int ReturnCode)
{
    return false;
}
bool Process::IsProcessRunning() const
{
    if (!_processInfo.hProcess)
        return false;
    DWORD exitCode;
    if (GetExitCodeProcess(_processInfo.hProcess,&exitCode) && exitCode==STILL_ACTIVE)
        return true;
    return false;
}
dword Process::GetProcessReturnCode() const
{
    DWORD rVal;
    if (!GetExitCodeProcess(_processInfo.hProcess,&rVal))
        rVal = -1;
    return rVal;
}