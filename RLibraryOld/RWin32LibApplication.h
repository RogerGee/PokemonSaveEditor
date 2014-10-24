//RWin32LibApplication.h
#ifndef RWIN32LIBAPPLICATION_H
#define RWIN32LIBAPPLICATION_H

namespace rtypes
{
    namespace rwin32
    {
        class Application
        {
        public:
            Application();

            bool RegisterApplication(HINSTANCE hInstance,
                SimpleResource ResAppIcon,
                SimpleResource ResAppIconSmall,
                SimpleResource ResAppCursor); // defined in RWin32LibGlobalAppData.cpp

            bool SetAsMainWindow(const Window& MainWnd); // register a window as the main window for the current thread
            

            // begins the message loop for the main window on the current thread
            int RunWindow(int CmdShow = SW_SHOWDEFAULT);
            // ends the message loop for the main window on the current thread
            void QuitWindow(int ReturnVal);

            dword GetNumberOfCommandLineArgs() const { return _commandLine.size(); }
            str GetCommandLineArg(dword Index) const { return _commandLine[Index]; }
            str GetExecutablePath() const; // does not contain a terminating '\\'

            const Window* GetMainAppWindow() const;
        private:
            struct _mainWindow
            {
                _mainWindow() : threadID(0),wndPtr(NULL) {}
                _mainWindow(const Window*);
                _mainWindow(dword ID) : threadID(ID), wndPtr(NULL) {}
                dword threadID; // the thread associated with the window id
                const Window* wndPtr;
                // these operations are used with sorting and searching
                bool operator <(const _mainWindow& right) const { return threadID<right.threadID; }
                bool operator ==(const _mainWindow& right) const { return threadID==right.threadID; }
            };

            bool _init;
            HINSTANCE _hInst;
            dword _mainThreadID;
            SimpleResource _resAppIcon,
                _resAppIconSmall,
                _resAppCursor;
            int _showState;
            container<str> _commandLine;
            container<_mainWindow> _mainWindows; // keep this container sorted

            bool _removeMainWindow(); // remove the main app window for the current thread

            friend HINSTANCE global_app_data::GetAppInstanceModule();
        };

        // provide an extension to the 'global_app_data' namespace
        namespace global_app_data
        {
            // this function is defined in RWin32GlobalAppData.cpp
            Application* GetRegisteredApplication();
        }
    }
}

#endif