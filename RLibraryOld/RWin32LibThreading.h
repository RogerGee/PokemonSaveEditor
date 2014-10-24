//RWin32LibThreading.h - contains wrapper types for creating threads and processes
#ifndef RWIN32LIBTHREADING_H
#define RWIN32LIBTHREADING_H

namespace rtypes
{
    namespace rwin32
    {
        namespace threading
        {
            typedef dword (*ThreadProcedure)( void* );

            // thread return states
            extern const dword ExitNormal;
            extern const dword ExitForced;
            extern const dword ExitError;

            class Thread
            {
            public:
                Thread();
                Thread(HANDLE,dword ThreadID = 0/* 0 is guarenteed as an invalid thread id*/,int CompletionPriority = -1);
                ~Thread();

                bool Create(ThreadProcedure EntryPoint,int CompletionPriority = 0);
                bool Run();
                bool RunWait(dword &ExitCode_out);
                bool Suspend() { return SuspendThread(_hThread)!=-1; }
                bool Close();
                // !!you preferably don't want to have to do this in C++!!
                void Kill() { TerminateThread(_hThread,ExitForced); /* this is a very bad thing to do */ }

                bool IsActive() const;
                bool IsCreated() const { return _hThread!=NULL; }
                dword GetThreadID() const { return _handleID; }
                HANDLE GetThreadHandle() const { return _hThread; }
                dword GetThreadExitCode() const; // -1 indicates error; STILL_ACTIVE means the thread is still running

                bool autoKill; // kills its thread when the Thread object passes out of scope; default is false
                static dword GetCurrentThreadID() { return ::GetCurrentThreadId(); }
            private:
                HANDLE _hThread;
                DWORD _handleID;
                int _myCompletionPriority;
            };

            class Process
            {
            public:
                Process();
                Process(const str& ApplicationName,
                    const str& CommandLine);
                ~Process();

                void SetApplicationName(const str& Name) { _applicationName = Name; }
                void SetCommandLine(const str& CmdLine) { _commandLine = CmdLine; }
                void RedirectHandles(HANDLE hOut,HANDLE hErr,HANDLE hIn);

                bool RunProcess(bool InheritHandles = true); // run the process
                bool RunProcessWait(int &ReturnCode_out,bool InheritHandles = true); // run the process and wait for it to quit
                bool KillProcess(int ReturnCode); // it is probably not a good idea for you to call this

                // methods that get information about a running process
                bool IsProcessRunning() const;
                dword GetProcessReturnCode() const;
                HANDLE GetProcessHandle() const { return _processInfo.hProcess; }
                dword GetProcessID() const { return _processInfo.dwProcessId; }
                Thread GetProcessMainThread() const { return Thread(_processInfo.hThread,_processInfo.dwThreadId); }
                HANDLE GetProcessMainThreadHandle() const { return _processInfo.hThread; }
                dword GetProcessMainThreadID() const { return _processInfo.dwThreadId; }
                str GetProcessApplicationName() const { return _applicationName; }
            private:
                STARTUPINFO _startupInfo;
                PROCESS_INFORMATION _processInfo;
                str _applicationName;
                str _commandLine;
                HANDLE  _hRedirectOutput,
                        _hRedirectError,
                        _hRedirectInput;
            };
        }
    }
}

#endif