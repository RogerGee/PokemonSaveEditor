//RWin32LibWindow.h
#ifndef RWIN32LIBWINDOW_H
#define RWIN32LIBWINDOW_H

namespace rtypes
{
    namespace rwin32
    {
        enum WindowType
        {
            // all frame windows MUST use these two types
            FrameWindowObject, // super-class is rwin32::Window
            DialogWindowObject, // super-class is rwin32::DialogWindow
            // all control window types must be enumerated with values greater than
            //  FrameWindowObject and DialogWindowObject

            // standard controls
            PushButtonControl,
            RadioButtonControl,
            CheckBoxControl,
            EditControl,
            RichEditControl,
            ListBoxControl,
            ComboBoxControl,
            StaticControl,

            // custom controls

            UserDefinedControl
        };

        enum WindowAttribute
        {
            NullAttrib,
            TitleBar,
            SystemMenu,
            MaximizeBox,
            MinimizeBox,
            Enabled,
            HorizontileScroll,
            VerticalScroll,
            Border,
            Transparent,
            Sizable
        };

        enum WindowShowState
        {
            // these states make a window active (it gets focus)
            ShowDefaultActive = 10,
            ShowNormalActive = 1,
            ShowActive = 5,
            ShowMinimizedActive = 2,
            ShowMaximizedActive = 3,
            
            // these states simply change the show state of a window
            Show = 8,
            ShowMinimized = 7
        };

        enum WindowFlashFlag
        {
            FlashAll = 3,
            FlashCaption = 1,
            FlashStop = 0,
            FlashTimer = 4,
            FlashUntilRestored = 12,
            FlashTaskBarButton = 2
        };

        class Window
        {// a frame window (by default)
        public:
            Window();
            Window(const Window&);
            Window(HWND);
            ~Window();

            bool Create(int X = -1,int Y = -1,int Width = 0,int Height = 0,const Window* ParentWin = NULL); // this will fire OnCreate (if successful)
            bool Show(int ShowOption = SW_SHOWDEFAULT) const;
            bool Show(WindowShowState State) const { return Show((int) State); }
            // 'Close' destroys a window
            bool Close(bool ForceDestroy = false,bool NeedMessage = true); // !!ForceDestory will not raise a Close event!!

            // "set" functions
            bool SetText(const str& Text);
            bool SetLocation(int X,int Y);
            bool SetLocation(const Point& Loc);
            bool SetSize(int Width,int Height) { return SetSize( Size(Width,Height) ); }
            bool SetSize(Size Sz);
            bool SetAttribute(WindowAttribute Attrib,bool Remove = false); //returns true if a change happened to the window, false otherwise
            //      any value <= for a size limit means "no size limit"
            void SetMaxSize(Size Sz);
            void SetMaxSize(int Width,int Height) { SetMaxSize( Size(Width,Height) ); }
            void SetMinSize(Size Sz);
            void SetMinSize(int Width,int Height) { SetMinSize( Size(Width,Height) ); }

            // "get" functions
            str GetText() const;
            virtual Point GetLocation() const; // control windows reimplement this function
            Size GetSize() const;
            Size GetClientSize() const;
            Size GetMaxSize() const { return _sizeLimitsMax; }
            Size GetMinSize() const { return _sizeLimitsMin; }
            Rect GetWinRect() const;
            bool GetIfAttributeSet(WindowAttribute Attrib) const;
            int GetTextLength() const { return (int) SendMessage(_hWnd,WM_GETTEXTLENGTH,NULL,NULL); }
            HMENU GetMenuBarHandle() const { return GetMenu(_hWnd); }
            HMENU GetSystemMenuHandle() const { return GetSystemMenu(_hWnd,FALSE); }
            HMENU GetContextMenuHandle() const { return NULL; }
            bool GetFullscreenState() const { return _fullscreenState!=_fullscreenOffState; }
            virtual WNDPROC GetWndProc() const;

            //parent/child window functions
            bool HasParent() const;
            const Window* GetParentWin() const { return _parentWin; }
            const Window* GetParentWinFromOS(bool Load = false); // if Load, then get the parent from the OS and remember it again
            dword GetNumberOfChildWindows() const { return _children.size(); }
            const Window* GetChildWindow(dword Offset) const { return _children[Offset]; }

            // ** Events **
            // event function pointers (make sure they're initialized to 0 in c-tor)
            EventFunct EvntCreate;
            EventFunct EvntShown;
            EventFunct EvntClose;
            EventFunct EvntDestroy;
            EventFunct EvntGotFocus;
            EventFunct EvntKillFocus;
            EventFunct EvntMouseUp;
            EventFunct EvntMouseDown;
            EventFunct EvntMouseMove;
            EventFunct EvntMoved;
            EventFunct EvntResized;
            EventFunct EvntKeyDown;
            EventFunct EvntKeyUp;
            EventFunct EvntHScroll;
            EventFunct EvntVScroll;

            // onEvent virtual methods (called if event functs are set to 0)
            virtual void OnCreate(const EventData&);
            virtual void OnShown(const EventData&);
            virtual void OnClose(const EventData&);
            virtual void OnDestroy(const EventData&);
            virtual void OnGotFocus(const EventData&);
            virtual void OnKillFocus(const EventData&);
            virtual void OnMouseUp(const MouseEventData&);
            virtual void OnMouseDown(const MouseEventData&);
            virtual void OnMouseMove(const MouseEventData&);
            virtual void OnMoved(const WinChngEventData&);
            virtual void OnResized(const WinChngEventData&);
            virtual void OnKeyDown(const KeyEventData&);
            virtual void OnKeyUp(const KeyEventData&);
            virtual void OnHScroll(const EventData&);
            virtual void OnVScroll(const EventData&);

            // ** **

            //operator overloads
            operator void *() const;
            Window& operator =(const Window&);
            Window& operator =(HWND);

            //misc.
            virtual WindowType MyType() const { return FrameWindowObject; }
            void Focus() const { SetFocus(_hWnd); }
            HWND WinHandle() const { return _hWnd; }
            void SizeToText(str Text = "");
            void FitWidthToText(str Text = "");
            void MakeFullScreen(bool Undo = false);  // works with Frame windows
            bool ToggleFullScreen(); // works with Frame windows; returns fullscreen state
            bool FlashWindow(WindowFlashFlag flag,dword FlashCount,dword FlashTimeout = 0) const;

            //misc. predicates
            virtual bool IsControl() const { return false; }
            bool IsHandleOwner() const { return !_doNotClose; }
            bool HasFocus() const { return GetFocus()==_hWnd; }

            // non-instance methods
            static WNDPROC GetMasterWndProc() { return &_wndProc; }
        protected:
            static bool _containsStyle(dword winStyle,dword style);
            static POINT _maxTrackSize;
            virtual const char* _getWinClass() const;
            virtual bool _isSystemClass() const { return false; /* frame windows use either a specified or generic class */ }
            virtual dword _getWinStyle() const;
            virtual Point _getDefaultWinLocation(int Width = 0,int Height = 0) const;
            virtual void _raiseSubEvents(EventData&); // called by _raiseEvents(...); should process subclassed events
            void _raiseEvents(EventData&); // default event processing
            void _destroyChildren() const; // make this constant to fit with children mutability
            HWND _hWnd;
        private:
            static const Rect _fullscreenOffState;
            static LRESULT CALLBACK _wndProc(HWND,UINT,WPARAM,LPARAM);
            bool _doNotClose; // if true, the window is a copy of another window that the object doesn't own
            mutable container<Window*> _children;
            const Window* _parentWin;
            Rect _fullscreenState;
            Size _sizeLimitsMax, _sizeLimitsMin; // max limits for the window

            friend class DialogWindow;
        };

        class DialogWindow : public Window
        {
        public:
            enum DialogResult
            {
                DialogError1 = -1,
                DialogError2 = 0,
                Yes = 1,
                No,
                Cancel
            };
            DialogWindow();

            // Do not call Window::Create on a dialog
            // creates a modeless dialog - call this instead of Window::Create
            bool CreateDlg(const Window* Parent = 0,int X = -1,int Y = -1,int Width = 0,int Height = 0);
            // creates and runs a modal dialog
            DialogResult ShowDlg(); // show with MainAppWindow
            DialogResult ShowDlg(const Window* Parent); // show with Parent
            // closes only modal dialogs - use Window::Close to close both modal and modeless dialogs
            bool CloseDlg(DialogResult Result = Yes);

            WindowType MyType() const { return DialogWindowObject; }
            bool IsModal() const { return _isModal; }
        protected:
            virtual dword _getWinStyle() const;
            Point _getDefaultWinLocation(int Width = 0,int Height = 0) const;
        private:
            static INT_PTR CALLBACK _dialogProc(HWND,UINT,WPARAM,LPARAM);
            bool _isModal;

        };

        typedef void (Window::* EventDelegate)(Window* senderWindow);
        typedef void (Window::* EventDelegate_void)( void );
        typedef void (Window::* EventDelegate_id)(dword ID);
    }
}

#endif