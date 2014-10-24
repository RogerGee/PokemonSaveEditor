#include "RWin32LibMaster.h"
#include <map>
using namespace std;
using namespace rtypes;
using namespace rtypes::rwin32;

// I use global data to prevent exposing c++ std lib. types.
namespace
{
    map<HWND,Window*> LoadedWindows; /* (I don't bother to remove pairs after a window handle was destroyed, since
                                         if the next load has a preexisting handle, the window object ptr value
                                         will be overridden.)       */
    Window* LoadingObject = 0; // global "communication" variable

    map<HWND,DialogWindow*> LoadedDialogs;
    DialogWindow* LoadingDialog = 0;
}

// ** This procedure handles all windows of our main app window class (frame window class)**
//      Its purpose is to call event functions of corresponding window objects.
//      This procedure calls events to the window refered to by "hWnd"
//          AND any control windows passed in through either "wParam" or "lParam".
LRESULT CALLBACK Window::_wndProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
    // get window object
    Window* win_obj = 0;
    if (LoadedWindows.count(hWnd)==1)
        // see if the object exists, so we don't add "unmapped" keys
        win_obj = LoadedWindows[hWnd];
    if (win_obj==0 && LoadingObject!=0)
    {
        if (!LoadingObject->_hWnd)
            // set _hWnd property so that we can handle "early" messages, like WM_CREATE
            LoadingObject->_hWnd = hWnd;
        LoadedWindows[hWnd] = LoadingObject;
        win_obj = LoadingObject;
        LoadingObject = 0; // we're finished with this, so reset it
    }
    if (!win_obj)
        // !!the window isn't owned by our system and we have no idea what to do with it!!
        return DefWindowProc(hWnd,msg,wParam,lParam);
    // process messages and call appropriate Window object methods
    /*  If processDefault==true, then the our window procedure thought it might have needed to process
            a message, but actually didn't. So, instead of returning 0, we return the result of DefWindowProc.
    */
    bool processDefault = false;
    switch (msg)
    {
    case WM_CREATE:
        {
            EventData d(WM_CREATE,NULL);
            win_obj->_raiseEvents(d);
        }
        break;
    case WM_VSCROLL:
        {
            EventData d(WM_VSCROLL,NULL);
            win_obj->_raiseEvents(d);
        }
        break;
    case WM_HSCROLL:
        {
            EventData d(WM_HSCROLL,NULL);
            win_obj->_raiseEvents(d);
        }
        break;
    case WM_SHOWWINDOW:
        {
            // include show state (wParam) in tag
            EventData d(WM_SHOWWINDOW | wParam,NULL);
            win_obj->_raiseEvents(d);
        }
        break;
    case WM_SETFOCUS:
        {
            EventData d(WM_SETFOCUS,NULL);
            win_obj->_raiseEvents(d);
        }
        break;
    case WM_KILLFOCUS:
        {
            EventData d(WM_KILLFOCUS,NULL);
            win_obj->_raiseEvents(d);
        }
        break;
    case WM_MOVE:
        {
            WinChngEventData d;
            d.SetWinChngEventType(WinChngEventData::Moved);
            d.winSize = win_obj->GetClientSize(); // fill this for consistency
            //use lParam to get location (even thought win_obj->GetLocation() would work)
            d.winLocation.x = lParam&0xffff;
            d.winLocation.y = (lParam>>16)&0xffff;
            win_obj->_raiseEvents(d);
        }
        break;
    case WM_GETMINMAXINFO:
        {
            static bool gotMaxSize = false;
            MINMAXINFO *info = (PMINMAXINFO) lParam;
            if (!gotMaxSize)
            {
                _maxTrackSize = info->ptMaxTrackSize; // save the max track size for later use
                gotMaxSize = true;
            }
            // set minimums
            info->ptMinTrackSize.x = win_obj->_sizeLimitsMin.width;
            info->ptMinTrackSize.y = win_obj->_sizeLimitsMin.height;
            // set maximums
            info->ptMaxTrackSize.x = (win_obj->_sizeLimitsMax.width==0 ? _maxTrackSize.x : win_obj->_sizeLimitsMax.width);
            info->ptMaxTrackSize.y = (win_obj->_sizeLimitsMax.height==0 ? _maxTrackSize.y : win_obj->_sizeLimitsMax.height);
        }
        break;
    case WM_SIZE:
        // occurs after the resizing has occurred
        {
            WinChngEventData d;
            d.SetWinChngEventType(WinChngEventData::Resized);
            d.winLocation = win_obj->GetLocation();
            //use lParam to get size
            d.winSize.width = lParam&0xffff;
            d.winSize.height = (lParam>>16)&0xffff;
            //set tag data (minimized or maximized)
            d.tag = wParam;
            win_obj->_raiseEvents(d);
        }
        break;
    case WM_SYSCOMMAND:
        processDefault = !MenuItem::ProcessMenuCommand((HMENU) wParam);
        break;
    case WM_COMMAND:
        {
            word highWord = (wParam>>16)&0xffff, lowWord = wParam&0xffff;
            if (!lParam)
            {// a menu or accelerator is sending the command
                HMENU hMenu = (HMENU) wParam;
                processDefault = !MenuItem::ProcessMenuCommand(hMenu); // send command notification to the menu item element
            }
            else
            {// a control is sending the command
                HWND hControl = (HWND) lParam;
                //find the wrapping Window object for the control handle
                //by checking through the current window's children
                Window* controlWindow = 0;
                for (dword i = 0;i<win_obj->_children.size();i++)
                    // children are sorted by Window object address, not HWND, so we have to linear search the children
                    if (win_obj->_children[i]->WinHandle()==hControl)
                    {
                        controlWindow = win_obj->_children[i];
                        break;
                    }
                if (controlWindow && controlWindow->MyType()>DialogWindowObject) // the control handle matched a wrapper object
                {
                    /*  Handle individual control messages -
                            Some of these events are subclassed events, among which some are main control events;
                            However, all events are routed through the Window::_raiseEvents method, even if they
                            are subclassed events.
                    */
                    switch (controlWindow->MyType())
                    {
                    case PushButtonControl:
                    case RadioButtonControl:
                    case CheckBoxControl:
                        {
                            if (highWord==BN_CLICKED)
                            {
                                ButtonEventData d(ButtonEventData::Clicked);
                                controlWindow->_raiseEvents(d);
                            }
                            else if (highWord==BN_DOUBLECLICKED)
                            {
                                ButtonEventData d(ButtonEventData::DblClicked);
                                controlWindow->_raiseEvents(d);
                            }
                            else if (highWord==BN_SETFOCUS)
                            {
                                EventData d(WM_SETFOCUS,NULL); // this flag invokes OnGotFocus
                                controlWindow->_raiseEvents(d);
                            }
                            else if (highWord==BN_KILLFOCUS)
                            {
                                EventData d(WM_KILLFOCUS,NULL); // this flag invokes OnKillFocus
                                controlWindow->_raiseEvents(d);
                            }
                            else if (highWord==BN_PAINT)
                            {//raise paint for the control window
                            
                            }
                        }
                        break;
                    case RichEditControl: // shares notifications with EditControl
                    case EditControl:
                        {
                            if (highWord==EN_CHANGE)
                            {
                                TextBoxEventData d(TextBoxEventData::TextChanged);
                                controlWindow->_raiseEvents(d);
                            }
                            else if (highWord==EN_UPDATE)
                            {
                                TextBoxEventData d(TextBoxEventData::TextUpdated);
                                controlWindow->_raiseEvents(d);
                            }
                            else if (highWord==EN_HSCROLL)
                            {
                                TextBoxEventData d(TextBoxEventData::HScrolled);
                                controlWindow->_raiseEvents(d);
                            }
                            else if (highWord==EN_VSCROLL)
                            {
                                TextBoxEventData d(TextBoxEventData::VScrolled);
                                controlWindow->_raiseEvents(d);
                            }
                            else if (highWord==EN_SETFOCUS)
                            {
                                EventData d(WM_SETFOCUS,NULL); // this flag invokes OnGotFocus
                                controlWindow->_raiseEvents(d);
                            }
                            else if (highWord==EN_KILLFOCUS)
                            {
                                EventData d(WM_KILLFOCUS,NULL); // this flag invokes OnKillFocus
                                controlWindow->_raiseEvents(d);
                            }
                        }
                        break;
                    case ListBoxControl:
                        {
                            if (highWord==LBN_SELCHANGE)
                            {
                                ListBoxEventData d(ListBoxEventData::SelChanged);
                                controlWindow->_raiseEvents(d);
                            }
                            else if (highWord==LBN_SELCANCEL)
                            {
                                ListBoxEventData d(ListBoxEventData::SelCanceled);
                                controlWindow->_raiseEvents(d);
                            }
                            else if (highWord==LBN_SETFOCUS)
                            {
                                EventData d(WM_SETFOCUS,NULL); // this flag invokes OnGotFocus
                                controlWindow->_raiseEvents(d);
                            }
                            else if (highWord==LBN_KILLFOCUS)
                            {
                                EventData d(WM_KILLFOCUS,NULL); // this flag invokes OnKillFocus
                                controlWindow->_raiseEvents(d);
                            }
                        }
                        break;
                    case ComboBoxControl:
                        {
                            ComboBoxEventData cd;
                            EventData d;
                            if (highWord==CBN_SELCHANGE)
                            {
                                cd.SetComboBoxEventType(ComboBoxEventData::SelChanged);
                                controlWindow->_raiseEvents(cd);
                            }
                            else if (highWord==CBN_SELENDOK)
                            {
                                cd.SetComboBoxEventType(ComboBoxEventData::SelAffirmed);
                                controlWindow->_raiseEvents(cd);
                            }
                            else if (highWord==CBN_SELENDCANCEL)
                            {
                                cd.SetComboBoxEventType(ComboBoxEventData::SelCanceled);
                                controlWindow->_raiseEvents(cd);
                            }
                            else if (highWord==CBN_EDITUPDATE)
                            {
                                cd.SetComboBoxEventType(ComboBoxEventData::TextUpdated);
                                controlWindow->_raiseEvents(cd);
                            }
                            else if (highWord==CBN_EDITCHANGE)
                            {
                                cd.SetComboBoxEventType(ComboBoxEventData::TextChanged);
                                controlWindow->_raiseEvents(cd);
                            }
                            else if (highWord==CBN_SETFOCUS)
                            {
                                d.tag = WM_SETFOCUS;
                                controlWindow->_raiseEvents(d);
                            }
                            else if (highWord==CBN_KILLFOCUS)
                            {
                                d.tag = WM_KILLFOCUS;
                                controlWindow->_raiseEvents(d);
                            }
                        }
                        break;
                    case StaticControl:
                        {
                            EventUserData d;
                            d.tag = highWord;
                            controlWindow->_raiseEvents(d);
                        }
                        break;
                    default: // assume user defined, pass event data down
                        {
                            // Include wParam as the high word, lParam as the low word
                            //  this restriction is part of the implementation
                            EventUserData d((wParam<<16) & lParam,NULL);
                            controlWindow->_raiseEvents(d);
                        }
                        break;
                    }
                }
                else
                {
                    // can't do anything with the notification message
                    processDefault = true;
                }
            }
        }
        break;
    case WM_NOTIFY:
        {
            NMHDR *notify = (NMHDR*) lParam;
            Window* controlWindow = 0;
            // find the control window object by linear-searching through its parent's list of children
            for (dword i = 0;i<win_obj->_children.size();i++)
                if (win_obj->_children[i]->WinHandle()==notify->hwndFrom)
                {
                    controlWindow = win_obj->_children[i];
                    break;
                }
            if (controlWindow)
            {// found the control for which the notification is valid
                // process nofitications based on control type
                if (controlWindow->MyType()==RichEditControl)
                {
                    if (notify->code==EN_MSGFILTER)
                    {// handle key and mouse event messages
                        MSGFILTER *filterInfo = (MSGFILTER*) lParam;
                        switch (filterInfo->msg)
                        {
                        case WM_MOUSEMOVE:
                            {
                                MouseEventData::MouseButton button = MouseEventData::NoButton;
                                if (filterInfo->wParam==MK_LBUTTON)
                                    button = MouseEventData::Left;
                                else if (filterInfo->wParam==MK_RBUTTON)
                                    button = MouseEventData::Right;
                                else if (filterInfo->wParam==MK_MBUTTON)
                                    button = MouseEventData::Middle;
                                MouseEventData d(MouseEventData::Move,button);
                                d.clickLocation.x = filterInfo->lParam&0xffff;
                                d.clickLocation.y = (filterInfo->lParam>>16)&0xffff;
                                controlWindow->_raiseEvents(d);
                            }
                            break;
                        case WM_LBUTTONDOWN:
                            {
                                MouseEventData d(MouseEventData::ClickDown,MouseEventData::Left);
                                d.clickLocation.x = filterInfo->lParam&0xffff;
                                d.clickLocation.y = (filterInfo->lParam>>16)&0xffff;                                
                                controlWindow->_raiseEvents(d);
                            }
                            break;
                        case WM_LBUTTONUP:
                            {
                                MouseEventData d(MouseEventData::ClickUp,MouseEventData::Left);
                                d.clickLocation.x = filterInfo->lParam&0xffff;
                                d.clickLocation.y = (filterInfo->lParam>>16)&0xffff;
                                controlWindow->_raiseEvents(d);
                            }
                            break;
                        case WM_RBUTTONDOWN:
                            {
                                MouseEventData d(MouseEventData::ClickDown,MouseEventData::Right);
                                d.clickLocation.x = filterInfo->lParam&0xffff;
                                d.clickLocation.y = (filterInfo->lParam>>16)&0xffff;
                                controlWindow->_raiseEvents(d);
                            }
                            break;
                        case WM_RBUTTONUP:
                            {
                                MouseEventData d(MouseEventData::ClickUp,MouseEventData::Right);
                                d.clickLocation.x = filterInfo->lParam&0xffff;
                                d.clickLocation.y = (filterInfo->lParam>>16)&0xffff;
                                controlWindow->_raiseEvents(d);
                            }
                            break;
                        }
                    }
                    else if (notify->code==EN_SELCHANGE)
                    {
                        TextBoxEventData d(TextBoxEventData::SelectionChanged);
                        controlWindow->_raiseEvents(d);
                    }
                    else if (notify->code==EN_DROPFILES)
                    {
                        TextBoxEventData d(TextBoxEventData::FileDropped);
                        d.tag = lParam; // set the ENDROPFILES structure ptr to the tag
                        controlWindow->_raiseEvents(d);
                    }
                }
            }
            else
            {
                // can't do anything with the message
                processDefault = true;
            }
        }
        break;
    case WM_LBUTTONDOWN:
        {
            MouseEventData d(MouseEventData::ClickDown,MouseEventData::Left);
            d.clickLocation.x = lParam&0xffff;
            d.clickLocation.y = (lParam>>16)&0xffff;
            win_obj->_raiseEvents(d);
        }
        break;
    case WM_RBUTTONDOWN:
        {
            MouseEventData d(MouseEventData::ClickDown,MouseEventData::Right);
            d.clickLocation.x = lParam&0xffff;
            d.clickLocation.y = (lParam>>16)&0xffff;
            win_obj->_raiseEvents(d);
        }
        break;
    case WM_LBUTTONUP:
        {
            MouseEventData d(MouseEventData::ClickUp,MouseEventData::Left);
            d.clickLocation.x = lParam&0xffff;
            d.clickLocation.y = (lParam>>16)&0xffff;
            win_obj->_raiseEvents(d);
        }
        break;
    case WM_RBUTTONUP:
        {
            MouseEventData d(MouseEventData::ClickUp,MouseEventData::Right);
            d.clickLocation.x = lParam&0xffff;
            d.clickLocation.y = (lParam>>16)&0xffff;
            win_obj->_raiseEvents(d);
        }
        break;
    case WM_MOUSEMOVE:
        {
            MouseEventData::MouseButton button = MouseEventData::NoButton;
            // find out what buttons are down
            if (wParam==MK_LBUTTON)
                button = MouseEventData::Left;
            else if (wParam==MK_RBUTTON)
                button = MouseEventData::Right;
            else if (wParam==MK_MBUTTON)
                button = MouseEventData::Middle;
            // create the event data
            MouseEventData d(MouseEventData::Move,button);
            d.clickLocation.x = lParam&0xffff;
            d.clickLocation.y = (lParam>>16)&0xffff;
            win_obj->_raiseEvents(d);
        }
        break;
    case WM_KEYDOWN:
        {
            KeyEventData keyData;
            keyData.virtualKey = (int) wParam;
            keyData.repeatCount = lParam&0xffff; // bits 0 - 15
            keyData.isExtended = (lParam&0x1000000) == 0x1000000; // see if bit 24 is on
            keyData.wasDown = (lParam&0x40000000) == 0x40000000; // see if bit 30 is on
            // raise key down event
            keyData.SetKeyEventType(KeyEventData::KeyDown);
            win_obj->_raiseEvents(keyData);
        }
        break;
    case WM_KEYUP:
        {
            KeyEventData keyData;
            keyData.virtualKey = (int) wParam;
            keyData.repeatCount = lParam&0xffff; // bits 0 - 15
            keyData.isExtended = (lParam&0x1000000) == 0x1000000; // see if bit 24 is on
            keyData.wasDown = (lParam&0x40000000) == 0x40000000; // see if bit 30 is on
            // raise key down event
            keyData.SetKeyEventType(KeyEventData::KeyUp);
            win_obj->_raiseEvents(keyData);
        }
        break;
    case WM_CLOSE:
        {
            // We expect the window object to send this message; this way, we know that the proper clean up occurred
            //  A window object always sends a WM_CLOSE message with an LPARAM that's non-zero (spc. 1)
            if (!lParam) // the system generated this message
            {// call the window obj's close method for it
                win_obj->Close();
                return 1;
            }
            EventData d(WM_CLOSE,NULL);
            if (!wParam) // a force quit does not call WM_CLOSE
                win_obj->_raiseEvents(d);
            if (d.tag || wParam) // wParam hold a value that, if non-zero, indicates that the window MUST be destroyed
            {
                // if tag is non-zero, the user wants to continue with the close
                DestroyWindow(hWnd);
                win_obj->_hWnd = NULL; // let the object know we killed its window (the window object should have done this)
            }
            // else, cancel the close by not calling DestroyWindow
            else return 1; // notify sender that the window did not close
        }
        break;
    case WM_DESTROY: // the window is destroyed
        {
            Application* app = global_app_data::GetRegisteredApplication();
            HWND hMainWindow = global_app_data::GetMainWindow();
            EventData d(WM_DESTROY,NULL);
            // seeing as the window is destroyed, we should mark the handle as NULL
            // for code running in an OnDestroy routine
            win_obj->_hWnd = NULL;
            win_obj->_raiseEvents(d);
            if (hWnd==hMainWindow) // this window is the main app window for this thread
                app->QuitWindow( threading::ExitNormal );
            else if (!hMainWindow)
                // application error - there should be a main app window for the thread
                app->QuitWindow( threading::ExitError );
        }
        break;
    default:
        // message is not handled by this system
        return DefWindowProc(hWnd,msg,wParam,lParam);
    }
    if (processDefault)
        // somewhere within our handling (the switch statement) we needed to not process
        // a message but instead need to let the DefWindowProc handle it
        return DefWindowProc(hWnd,msg,wParam,lParam);
    return 0;
}

bool Window::_containsStyle(dword winStyle,dword style)
{// a little helper function for determining if a style is turned on
    return ((winStyle & style) == style);
}

POINT Window::_maxTrackSize;
const Rect Window::_fullscreenOffState(-1,-1,-1,-1);
Window::Window()
{
    _doNotClose = false;
    _hWnd = NULL;
    _parentWin = 0;
    _fullscreenState = _fullscreenOffState; // this means not in full screen
    EvntCreate = 0;
    EvntShown = 0;
    EvntClose = 0;
    EvntDestroy = 0;
    EvntGotFocus = 0;
    EvntKillFocus = 0;
    EvntMouseUp = 0;
    EvntMouseDown = 0;
    EvntMouseMove = 0;
    EvntMoved = 0;
    EvntResized = 0;
    EvntKeyDown = 0;
    EvntKeyUp = 0;
    EvntHScroll = 0;
    EvntVScroll = 0;
}
Window::Window(const Window& Wnd)
{
    _hWnd = Wnd._hWnd;
    _doNotClose = true;
    _parentWin = Wnd._parentWin;
    //copy children
    _children = Wnd._children;
    //copy other stuff - like event function pointers

    _fullscreenState = Wnd._fullscreenState;
}
Window::Window(HWND hWnd)
{
    _hWnd = hWnd;
    _doNotClose = true;
    _fullscreenState = _fullscreenOffState;
    GetParentWinFromOS(true); // see if the window has a parent
}
Window::~Window()
{
    /*
        This destructor is in place for every window object in case a user misuses
        a window object and loses track of a window handle. This could be a scoping problem
        or can be used to implicitly close windows as the execution passes from scope to scope.
        Other systems besides this destructor are in place to properly close a window. For example,
        a window object with children will automatically close all of its child windows before it closes.
    */
    // only close if we are the owner of the window handle AND if the window handle is valid
    if (!_doNotClose && _hWnd)
        /*
            the window hasn't been destroyed and the handling object is about to be destroyed,
            therefore we need to destroy the window
        */
        Close(true); // destroy the window no matter what
}
bool Window::Create(int X,int Y,int Width,int Height,const Window* ParentWin)
{
    if (_hWnd || MyType()==DialogWindowObject)
        return false;
    // allow master win proc to add our window object to the window lookup map
    if (MyType()==FrameWindowObject)
        // the main window procedure is for frame windows
        LoadingObject = this;
    // create window
    Point loc = _getDefaultWinLocation(Width,Height);
    if (X>=0)
        loc.x = X;
    if (Y>=0)
        loc.y = Y;
    // find a parent window, if any
    HWND hParent = NULL;
    if (IsControl() && !ParentWin)
        // it should have a parent specified
        return false;
    if (ParentWin)
        hParent = ParentWin->WinHandle(); // assign parent handle so CreateWindowEx
    // (_hWnd gets assigned in the window procedure too, but needs to be assigned here for control windows)
    dword extendedStyle = NULL;
    _hWnd = CreateWindowEx(extendedStyle,
        _getWinClass(),
        "",
        _getWinStyle(),
        loc.x,
        loc.y,
        (Width==0 ? CW_USEDEFAULT : Width),
        (Height==0 ? CW_USEDEFAULT : Height),
        hParent,
        NULL, // menu
        global_app_data::GetAppInstanceModule(),
        NULL); // (this window handle is released by the master window procedure)
    // check window creation success
    if (!_hWnd)
    {
        LoadingObject = 0;
        return false;
    }
    // add us as a child of our parent/handle fonts
    if (ParentWin)
    {
        ParentWin->_children.push_back(this);
        // sort 'em
        ParentWin->_children.insertion_sort();
        // set font to parent font

    }
    else
        // set font to default gui font
        SendMessage(_hWnd,WM_SETFONT,(WPARAM) GetStockObject(DEFAULT_GUI_FONT),TRUE);
    // attempt to set parent win
    GetParentWinFromOS(true); // could assign 'ParentWin', however this method checks to ensure this window is a child
    // control event
    if (IsControl())
    {
        EventData d(WM_CREATE,0);
        OnCreate(d);
        if (EvntCreate)
        {
            d.SetFunctCall(EvntCreate);
            d.RaiseEvent();
        }
    }
    return true;
}
bool Window::Show(int ShowOption) const
{
    if (!_hWnd)
        return false;
    // show and update window
    ShowWindow(_hWnd,ShowOption);
    UpdateWindow(_hWnd); // force the window to process WM_PAINT
    return true;
}
bool Window::Close(bool ForceDestroy,bool NeedMessage)
{
    // ** The Master Window Proc. calls this for each window that disposes;
    //      it will, however, destroy the window without having been called by the Master Window Proc. **
    if (_hWnd)
    {
        // !!Must get window parent before window is destroyed!!
        const Window* par = GetParentWin();
        // close window works for dialog boxes too
        if (NeedMessage)
        {// the window might be destroyed automatically by its parent
            /* !!Pass 1 as LPARAM, this means the window object is sending the close message!! */
            int rVal = SendMessage(_hWnd,WM_CLOSE,(WPARAM)ForceDestroy,1);
            if (MyType()==DialogWindowObject) 
                rVal = GetWindowLong(_hWnd,/*DWL_MSGRESULT*/0);
            if (rVal!=0)
                return false; // the window didn't close
        }
        // if we have a parent, tell them that we're being destroyed
        if (par)
        {
            dword i;
            if (par->_children.binary_search(this,i))
                par->_children.remove_at(i);
        }
        _parentWin = NULL; // set our parent win reference to null
        // if we are a parent, destroy our children (sounds terrible, but needs to be done)
        // Windows automatically destroys the window handles to child objects
        // We just have to tell the wrapper objects that their windows were destroyed
        _destroyChildren();
        // mark that we're destroyed (may have been done already, doesn't hurt to do it again)
        _hWnd = NULL;
        // control event
        if (IsControl())
        {// we have to handle this event here
            EventData d(WM_DESTROY,0);
            OnDestroy(d);
            if (EvntDestroy)
            {
                d.SetFunctCall(EvntDestroy);
                d.RaiseEvent();
            }
        }
        return true;
    }
    else return false;
}
bool Window::SetText(const str& Text)
{
    if (!_hWnd)
        return false;
    return SetWindowText(_hWnd,&Text[0])==TRUE;
}
bool Window::SetLocation(int X,int Y)
{
    if (!_hWnd)
        return false;
    return SetWindowPos(_hWnd,NULL,X,Y,NULL,NULL,SWP_NOSIZE | SWP_NOZORDER)==TRUE;
}
bool Window::SetLocation(const Point& Loc)
{
    if (!_hWnd)
        return false;
    return SetWindowPos(_hWnd,NULL,Loc.x,Loc.y,NULL,NULL,SWP_NOSIZE | SWP_NOZORDER)==TRUE;
}
bool Window::SetSize(Size Sz)
{
    if (!_hWnd)
        return false;
    if (MyType()>DialogWindowObject)
    {// not a frame or dialog window, enforce window size limits
        // check for min size limit violations
        if (Sz.width<_sizeLimitsMax.width && _sizeLimitsMax.width>0)
            Sz.width = _sizeLimitsMax.width;
        if (Sz.height<_sizeLimitsMax.height && _sizeLimitsMax.height>0)
            Sz.height = _sizeLimitsMax.height;
        // check for max size limit violations
        if (Sz.width>_sizeLimitsMax.width && _sizeLimitsMax.width>0)
            Sz.width = _sizeLimitsMax.width;
        if (Sz.height>_sizeLimitsMax.height && _sizeLimitsMax.height>0)
            Sz.height = _sizeLimitsMax.height;
    } // else, this is supported by frame/dialog windows through handling a message in the win. proc.
    if (Sz.width<0 || Sz.height<0)
    {// if a negative number is passed, use the pre-existing size value
        Size sz = GetSize();
        if (Sz.width<0)
            Sz.width = sz.width;
        if (Sz.height<0)
            Sz.height = sz.height;
    }
    return SetWindowPos(_hWnd,NULL,NULL,NULL,Sz.width,Sz.height,SWP_NOMOVE | SWP_NOZORDER)==TRUE;
}
void Window::SetMaxSize(Size Sz)
{
    if ( !(*this) )
        return;
    Size mySize;
    _sizeLimitsMax = Sz;
    // check current size
    mySize = GetSize();
    if ((mySize.width>_sizeLimitsMax.width && _sizeLimitsMax.width>0) 
        || (mySize.height>_sizeLimitsMax.height && _sizeLimitsMax.height>0))
    {
        if (mySize.width>_sizeLimitsMax.width && _sizeLimitsMax.width>0)
            mySize.width = _sizeLimitsMax.width;
        if (mySize.height>_sizeLimitsMax.height && _sizeLimitsMax.height>0)
            mySize.height = _sizeLimitsMax.height;
        SetSize(mySize); // enforce max size
    }
}
void Window::SetMinSize(Size Sz)
{
    if ( !(*this) )
        return;
    Size mySize;
    _sizeLimitsMin = Sz;
    // check current size
    mySize = GetSize();
    if ((mySize.width<_sizeLimitsMax.width && _sizeLimitsMax.width>0) 
        || (mySize.height<_sizeLimitsMax.height && _sizeLimitsMax.height>0))
    {
        if (mySize.width<_sizeLimitsMax.width && _sizeLimitsMax.width>0)
            mySize.width = _sizeLimitsMax.width;
        if (mySize.height<_sizeLimitsMax.height && _sizeLimitsMax.height>0)
            mySize.height = _sizeLimitsMax.height;
        SetSize(mySize); // enforce max size
    }
}
bool Window::SetAttribute(WindowAttribute Attrib,bool Remove)
{
    if (!_hWnd)
        return false;
    dword style = GetWindowLong(_hWnd,GWL_STYLE);
    dword attrib = NullAttrib;
    /* Find the right attrib to modify; check to see if prerequisites are set.
            This way, we can force the user to set attribs in the correct order.
        This is a stricter implementation, however, it will insure a window's style
        refers to every applicable trait.
                                        */
    switch (Attrib)
    {
    case NullAttrib:
        return true;
    case TitleBar:
        attrib = WS_CAPTION;
        break;
    case SystemMenu:
        if (!Remove && !_containsStyle(style,WS_CAPTION))
            return false;
        attrib = WS_SYSMENU;
        break;
    case MaximizeBox:
        if (!Remove && !_containsStyle(style,WS_CAPTION) && !_containsStyle(style,WS_SYSMENU))
            // the prerequisite window attribs aren't on
            return false;
        attrib = WS_MAXIMIZEBOX;
        break;
    case MinimizeBox:
        if (!Remove && !_containsStyle(style,WS_CAPTION) && !_containsStyle(style,WS_SYSMENU))
            // the prerequisite window attribs aren't on
            return false;
        attrib = WS_MINIMIZEBOX;
        break;
    case Enabled: // this is a special case
        EnableWindow(_hWnd, (Remove ? FALSE : TRUE));
        return true;
    case HorizontileScroll:
        attrib = WS_HSCROLL;
        break;
    case VerticalScroll:
        attrib = WS_VSCROLL;
        break;
    case Border:
        attrib = WS_BORDER;
        break;
    case Transparent:
        // special case
        {
            dword exStyle = GetWindowLong(_hWnd,GWL_EXSTYLE);
            exStyle |= (!Remove ? WS_EX_LAYERED : ~WS_EX_LAYERED);
            SetWindowLong(_hWnd,GWL_EXSTYLE,exStyle);
            if (!Remove)
            {
                // set layered styles for color key only; key color is solid white
                if (GetLastError() || !SetLayeredWindowAttributes(_hWnd,RGB(255,255,255),NULL,LWA_COLORKEY))
                    return false;
            }
        }
        return true;
    case Sizable:
        attrib = WS_THICKFRAME;
        break;
    default:
        return false; // unrecognized window attribute
    }
    // perform the add/remove operation
    if (Remove && _containsStyle(style,attrib))
        // bits are on, turn them off
        style ^= attrib; // could do style &= ~attrib, but I need to go to the else case if the style isn't set
    else if (!Remove && !_containsStyle(style,attrib))
        // bits are off, turn them on
        style |= attrib;
    else
        // the bits are already properly configured
        return false;
    // reset window style
    SetWindowLong(_hWnd,GWL_STYLE,style);
    // make the changes happen
    if (GetLastError() || !SetWindowPos(_hWnd,NULL,NULL,NULL,NULL,NULL,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED))
        return false;
    return true;
}
str Window::GetText() const
{
    if (!_hWnd) return "";
    int len = GetWindowTextLength(_hWnd)+1; // add 1 for null term.
    str r_buf((dword) len);
    GetWindowText(_hWnd,&r_buf[0],len);
    return r_buf;
}
Point Window::GetLocation() const
{
    RECT r;
    if (!GetWindowRect(_hWnd,&r))
        return Rect();
    if (MyType()>DialogWindowObject && _parentWin)
        ScreenToClient(_parentWin->WinHandle(),(LPPOINT)&r); // RECT and POINT types are similar (order of LONG members)
    return Point(r.left,r.top);
}
Size Window::GetSize() const
{
    RECT r;
    if (!GetWindowRect(_hWnd,&r))
        return Size();
    return Size(r.right-r.left,r.bottom-r.top);
}
Size Window::GetClientSize() const
{
    RECT r;
    if (!GetClientRect(_hWnd,&r))
        return Size();
    return Size(r.right,r.bottom);
}
Rect Window::GetWinRect() const
{
    Rect r;
    Point* p = &r; Size* s = &r;
    *p = GetLocation();
    *s = GetSize();
    return r;
}
bool Window::HasParent() const
{
    if (!_hWnd)
        return false;
    return GetParent(_hWnd)!=NULL;
}
const Window* Window::GetParentWinFromOS(bool Load)
{
    HWND hParent = GetParent(_hWnd);
    const Window* pWin;
    if (LoadedWindows.count(hParent))
        pWin = LoadedWindows[hParent];
    else if (LoadedDialogs.count(hParent))
        pWin = static_cast<const Window*> (LoadedDialogs[hParent]);
    else
        pWin = 0;
    if (Load)
        _parentWin = pWin;
    return pWin;
}
bool Window::GetIfAttributeSet(WindowAttribute Attrib) const
{
    //unimplemented until I determine all window attributes
    return false;
}
WNDPROC Window::GetWndProc() const
{
    WNDCLASSEX wndclaInfo;
    wndclaInfo.cbSize = sizeof(WNDCLASSEX);
    if (!::GetClassInfoEx( (_isSystemClass() ? NULL : global_app_data::GetAppInstanceModule()),
        _getWinClass(),
        &wndclaInfo))
        return NULL;
    return wndclaInfo.lpfnWndProc;
}
void Window::SizeToText(str def)
{
    HDC hDc = GetDC(_hWnd);
    if (hDc)
    {
        SIZE s;
        str Text = (def.size()==0 ? GetText() : def);
        int len = (int) Text.size();
        //get the text dimensions
        GetTextExtentPoint32(hDc,Text.c_str(),len,&s);
        // extend window size in all directions (for display purposes)
        s.cx += s.cx/len;
        s.cy += s.cy/4;
        SetSize(s.cx,s.cy);
        ReleaseDC(_hWnd,hDc);
    }
}
void Window::FitWidthToText(str def)
{
    HDC hDc = GetDC(_hWnd);
    if (hDc)
    {
        SIZE s;
        if (def.size()==0)
            def = GetText();
        int len = (int) def.size();
        if (len<=0) // control might not exist and gets a bogus HDC
            return;
        //get text dimensions
        GetTextExtentPoint32(hDc,def.c_str(),len,&s);
        //extend the width for display purposes
        s.cx += s.cx/len;
        SetSize(s.cx,-1); // pass -1 as height to preserve old height
        ReleaseDC(_hWnd,hDc);
    }
}
void Window::MakeFullScreen(bool Undo)
{// the user doesn't probably know the state of fullscreen-ness
    if (!_hWnd || MyType()!=FrameWindowObject)
        return;
    bool state = GetFullscreenState();
    SetAttribute(TitleBar,!Undo);
    SetAttribute(Sizable,!Undo);
    if (!Undo && !state)
    {
        Window desktop = GetDesktopWindow();
        Point myLoc = GetLocation(); Size mySize = GetSize();
        // preserve old size
        _fullscreenState.x = myLoc.x;
        _fullscreenState.y = myLoc.y;
        _fullscreenState.width = mySize.width;
        _fullscreenState.height = mySize.height;
        // set to fullscreen size
        SetSize(desktop.GetSize());
        // set in upper left corner
        SetLocation( Point() );
    }
    else if (state)
    {
        SetSize( _fullscreenState );
        SetLocation( _fullscreenState );
        _fullscreenState = _fullscreenOffState;
    }
    // at this point the window should be in whatever fullscreen state the user wanted
}
bool Window::ToggleFullScreen()
{
    if (!_hWnd || MyType()!=FrameWindowObject)
        return false;
    bool state = GetFullscreenState();
    // set/remove fullscreen window styles
    SetAttribute(TitleBar,!state);
    SetAttribute(Sizable,!state);
    if (state)
    {// is already fullscreen
        // set back to old window settings
        SetSize( _fullscreenState );
        SetLocation( _fullscreenState );
        _fullscreenState = _fullscreenOffState;
    }
    else
    {// needs to become fullscreen
        Window desktop = GetDesktopWindow();
        Point myLoc = GetLocation(); Size mySize = GetSize();
        // preserve old size
        _fullscreenState.x = myLoc.x;
        _fullscreenState.y = myLoc.y;
        _fullscreenState.width = mySize.width;
        _fullscreenState.height = mySize.height;
        // set to fullscreen size
        SetSize(desktop.GetSize());
        // set in upper left corner
        SetLocation( Point() );
    }
    return state;
}
bool Window::FlashWindow(WindowFlashFlag flag,dword FlashCount,dword FlashTimeout) const
{
    if (MyType()>DialogWindowObject || !_hWnd)
        return false; // not a flashable window
    static FLASHWINFO fwinfo;
    fwinfo.cbSize = sizeof(FLASHWINFO);
    fwinfo.hwnd = _hWnd;
    fwinfo.uCount = FlashCount;
    fwinfo.dwTimeout = FlashTimeout;
    fwinfo.dwFlags = flag;
    return FlashWindowEx(&fwinfo)!=FALSE;
}
Window::operator void *() const
{
    // used to determine if the window handle is valid
    return (void*) _hWnd;
}
Window& Window::operator =(const Window& Wnd)
{
    if (this==&Wnd)
        return *this; // don't assign to self, come now
    //close if necessary
    Close(true);
    //assign and mark that this object is simply a reference (copy)
    _doNotClose = true;
    _hWnd = Wnd._hWnd;
    _parentWin = Wnd._parentWin;
    //copy children
    _children = Wnd._children;
    //copy other stuff - like event function pointers

    _fullscreenState = Wnd._fullscreenState;
    return *this;
}
Window& Window::operator =(HWND hWnd)
{
    //close if necessary
    Close(true);
    //mark that this object is a reference copy
    _doNotClose = true;
    _hWnd = hWnd;
    _fullscreenState = _fullscreenOffState;
    GetParentWinFromOS(true);
    // unfortunately we cannot determine any child windows
    return *this;
}
const char* Window::_getWinClass() const
{
    return global_app_data::GetGenericMainWindowClass();
}
dword Window::_getWinStyle() const
{
    // dfeault main app window style
    return WS_OVERLAPPEDWINDOW;
}
Point Window::_getDefaultWinLocation(int Width,int Height) const
{
    return Point(CW_USEDEFAULT,CW_USEDEFAULT);
}
//default event handlers do nothing
void Window::OnCreate(const EventData&)
{
    /* do nothing... */
}
void Window::OnShown(const EventData&)
{
    /* do nothing... */
}
void Window::OnClose(const EventData&)
{
    /* do nothing... */
}
void Window::OnDestroy(const EventData&)
{
    /* do nothing... */
}
void Window::OnGotFocus(const EventData&)
{
    /* do nothing... */
}
void Window::OnKillFocus(const EventData&)
{
    /* do nothing... */
}
void Window::OnMouseUp(const MouseEventData&)
{
    /* do nothing... */
}
void Window::OnMouseDown(const MouseEventData&)
{
    /* do nothing... */
}
void Window::OnMouseMove(const MouseEventData&)
{
    /* do nothing... */
}
void Window::OnMoved(const WinChngEventData&)
{
    /* do nothing... */
}
void Window::OnResized(const WinChngEventData&)
{
    /* do nothing... */
}
void Window::OnKeyDown(const KeyEventData&)
{
    /* do nothing... */
}
void Window::OnKeyUp(const KeyEventData&)
{
    /* do nothing... */
}
void Window::OnHScroll(const EventData&)
{
    /* do nothing... */
}
void Window::OnVScroll(const EventData&)
{
    /* do nothing... */
}
void Window::_raiseSubEvents(EventData& data)
{
    /* do nothing... */
}
void Window::_raiseEvents(EventData& data)
{// the master window procedure should call this when an event message is recieved
    if (_doNotClose)
        return; // this is not the correct Window object to invoke events for the window
    //raise generic window events that we handle
    /*
        EventData may have been obtained from the Master Win Proc; however, the event data for
            some events need to be assigned here. In that case, "data" is used to id the type of event only.
        We pass the same pointer/reference that the Master Win Proc created to all event functions. This library is only single-threaded,
            so this is a safe operation.
                */
    switch (data.MyType())
    {
    case Normal: // the generic event data; event message is stored in data tag
        if (data.tag==WM_CREATE)
        {
            OnCreate(data);
            if (EvntCreate)
            {
                data.SetFunctCall(EvntCreate);
                data.RaiseEvent();
            }
        }
        else if (data.tag==WM_CLOSE)
        {
            OnClose(data);
            if (EvntClose)
            {
                data.SetFunctCall(EvntClose);
                data.RaiseEvent();
            }
        }
        else if (data.tag==WM_DESTROY)
        {
            OnDestroy(data);
            if (EvntDestroy)
            {
                data.SetFunctCall(EvntDestroy);
                data.RaiseEvent();
            }
        }
        else if ( (data.tag & 0xfffffffe) == WM_SHOWWINDOW)
        {// the tag holds two data items: the WM_SHOWWINDOW ID and the show state
            // the least sig. bit holds the show state
            // erase all bits except show bit
            data.tag &= 0x01;
            OnShown(data);
            if (EvntShown)
            {
                data.SetFunctCall(EvntShown);
                data.RaiseEvent();
            }
        }
        else if (data.tag==WM_SETFOCUS)
        {
            OnGotFocus(data);
            if (EvntGotFocus)
            {
                data.SetFunctCall(EvntGotFocus);
                data.RaiseEvent();
            }
        }
        else if (data.tag==WM_KILLFOCUS)
        {
            OnKillFocus(data);
            if (EvntKillFocus)
            {
                data.SetFunctCall(EvntKillFocus);
                data.RaiseEvent();
            }
        }
        else if (data.tag==WM_HSCROLL)
        {
            OnHScroll(data);
            if (EvntHScroll)
            {
                data.SetFunctCall(EvntHScroll);
                data.RaiseEvent();
            }
        }
        else if (data.tag==WM_VSCROLL)
        {
            OnVScroll(data);
            if (EvntVScroll)
            {
                data.SetFunctCall(EvntVScroll);
                data.RaiseEvent();
            }
        }
        break;
    case Mouse:
        {
            /* For the Mouse event, the location data members and button type should be assigned. 
                    All we have to do here is assign the function pointer and call the onEvent method.
                    */
            // data actually is a MouseEventData object
            MouseEventData& mobj = static_cast<MouseEventData&> (data);
            if (mobj.MyClickType()==MouseEventData::ClickUp)
            {
                OnMouseUp(mobj);
                if (EvntMouseUp)
                {
                    mobj.SetFunctCall(EvntMouseUp);
                    mobj.RaiseEvent();
                }   
            }
            else if (mobj.MyClickType()==MouseEventData::ClickDown)
            {
                OnMouseDown(mobj);
                if (EvntMouseDown)
                {
                    mobj.SetFunctCall(EvntMouseDown);
                    mobj.RaiseEvent();
                }
            }
            else if (mobj.MyClickType()==MouseEventData::Move)
            {
                OnMouseMove(mobj);
                if (EvntMouseMove)
                {
                    mobj.SetFunctCall(EvntMouseMove);
                    mobj.RaiseEvent();
                }
            }
        }
        break;
    case WinChng:
        {
            /* For the WinChng event, the window has been moved or resized. 

                    */
            // data is actually a WinChngEventData object
            WinChngEventData& wcobj = static_cast<WinChngEventData&> (data);
            if (wcobj.MyEventType()==WinChngEventData::Moved)
            {
                OnMoved(wcobj);
                if (EvntMoved)
                {
                    wcobj.SetFunctCall(EvntMoved);
                    wcobj.RaiseEvent();
                }
            }
            else if (wcobj.MyEventType()==WinChngEventData::Resized)
            {
                OnResized(wcobj);
                if (EvntResized)
                {
                    wcobj.SetFunctCall(EvntResized);
                    wcobj.RaiseEvent();
                }
            }
        }
        break;
    case Key:
        {
            /*
                For Key events, the event is either key down or key up.
                    KeyDown = the key is pressed
                    KeyUp = the key was released
                    */
            KeyEventData& keyData = static_cast<KeyEventData&> (data);
            if (keyData.MyEventType()==KeyEventData::KeyDown)
            {// key down event
                OnKeyDown(keyData);
                if (EvntKeyDown)
                {
                    keyData.SetFunctCall(EvntKeyDown);
                    keyData.RaiseEvent();
                }
            }
            else if (keyData.MyEventType()==KeyEventData::KeyUp)
            {// key up event
                OnKeyUp(keyData);
                if (EvntKeyUp)
                {
                    keyData.SetFunctCall(EvntKeyUp);
                    keyData.RaiseEvent();
                }
            }
        }
        break;
    case SubClassed:
        //raise events that a sub-class might handle (that we don't)
        _raiseSubEvents(data);
        break;
    }
}
void Window::_destroyChildren() const
{
    dword lastSize = _children.size();
    for (dword i = 0;i<_children.size();i++)
    {
        _children[i]->Close(false,false);
        i -= lastSize-_children.size(); // a child will remove itself from a parent
        lastSize = _children.size(); // record last size
    }
}

// ** Main Win Proc for dialog windows **
//      This procedure is responsable for identifing modal dialog windows.
INT_PTR CALLBACK DialogWindow::_dialogProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
    DialogWindow* win_obj = 0;
    if (LoadedDialogs.count(hWnd)==1)
        win_obj = LoadedDialogs[hWnd];
    if (win_obj==0 && LoadingDialog!=0)
    {// a dialog was created, add it to our dialog window reference system
        if (!LoadingDialog->_hWnd)
            /* the dialog object might be modal dialog that needs to have its window handle set,
                    or else is a modeless dialog that's being created (a WM_INITDIALOG is sent before CreateDialog
                        returns
                    */
            LoadingDialog->_hWnd = hWnd;
        LoadedDialogs[hWnd] = LoadingDialog;
        win_obj = LoadingDialog;
        LoadingDialog = 0; // reset so we don't add again
    }
    if (win_obj==0)
    {
        // !! we don't own this window; tell the sender we didn't process the message!!
        return FALSE;
    }
    switch (msg)
    {
    case WM_INITDIALOG:
        // call OnCreate for the dialog
        {
            // set default center location if isModal (Since we didn't have an HWND earlier)
            if (win_obj->_isModal)
            {
                // we provide this as a convinience (we do this for modeless boxes too)
                HWND hParent = GetParent(hWnd);
                Window WndPar = ( hParent ? hParent : GetDesktopWindow() ); // use the desktop if no parent
                Size sz; Point loc;
                sz = WndPar.GetSize();
                loc = WndPar.GetLocation();
                loc += win_obj->_getDefaultWinLocation(sz.width,sz.height);
                win_obj->SetLocation(loc.x,loc.y);
            }
            EventData d(WM_CREATE,NULL); // WM_CREATE acts as a code to raise the OnCreate method,etc.
            win_obj->_raiseEvents(d);
        }
        return TRUE;
    case WM_SHOWWINDOW:
        if (wParam)
        {// the window is being shown; raise the event
            //  include show state (wParam) in tag
            EventData d(WM_SHOWWINDOW | wParam,NULL);
            win_obj->_raiseEvents(d);
        }
        return TRUE;
    case WM_CLOSE:
        {
            if (!lParam)
            { // let the object send the close message so that proper clean-up can occur
                win_obj->Close();
                return TRUE;
            }
            EventData d(WM_CLOSE,NULL);
            if (!wParam) // don't raise Close event on force quit
                win_obj->_raiseEvents(d);
            if (d.tag || wParam) // if WParam!=0 then force quit
            {
                // if tag is non-zero, the user wants to continue with the close
                if (!win_obj->_isModal)
                    DestroyWindow(hWnd); // destroy modeless dialogs like this
                else
                    // you have to destroy modals like this
                    EndDialog(hWnd,No); // the user most likely clicked the X, so the the result should be No
                // destroy all child windows on the window, if any
                win_obj->_destroyChildren();
                // inform the win obj that we killed it's window
                win_obj->_hWnd = NULL;
                SetWindowLong(hWnd,/*DWL_MSGRESULT*/0,0);
            }
            // else, cancel the close by not calling DestroyWindow
            else
            {// notify sender that the window did not close
                // for dialog windows, specific return values must be handled seperately before TRUE is returned
                SetWindowLong(hWnd,/*DWL_MSGRESULT*/0,1); // Window::Close will check to see if it is a DialogWindow, if so it will get this value instead of the return from SendMessage
                //return 1;
            }
        }
        return TRUE;
    case WM_DESTROY:
        {
            EventData d(WM_DESTROY,0);
            win_obj->_raiseEvents(d);
        }
        return TRUE;
    case WM_SETFOCUS:
        {
            EventData d(WM_SETFOCUS,NULL);
            win_obj->_raiseEvents(d);
        }
        return TRUE;
    case WM_KILLFOCUS:
        {
            EventData d(WM_KILLFOCUS,NULL);
            win_obj->_raiseEvents(d);
        }
        return TRUE;
    case WM_MOVE:
        {
            WinChngEventData d;
            d.SetWinChngEventType(WinChngEventData::Moved);
            d.winSize = win_obj->GetClientSize(); // fill this for consistency
            //use lParam to get location (even thought win_obj->GetLocation() would work)
            d.winLocation.x = lParam&0xffff;
            d.winLocation.y = (lParam>>16)&0xffff;
            win_obj->_raiseEvents(d);
        }
        return TRUE;
    case WM_SIZE:
        // occurs after the resizing has occurred
        {
            WinChngEventData d;
            d.SetWinChngEventType(WinChngEventData::Resized);
            d.winLocation = win_obj->GetLocation();
            //use lParam to get size
            d.winSize.width = lParam&0xffff;
            d.winSize.height = (lParam>>16)&0xffff;
            //set tag data (minimized or maximized)
            d.tag = wParam;
            win_obj->_raiseEvents(d);
        }
        return FALSE;
    case WM_GETMINMAXINFO:
        {
            MINMAXINFO *info = (PMINMAXINFO) lParam;
            Size sizeLimitsMin = win_obj->GetMinSize(), sizeLimitsMax = win_obj->GetMaxSize();
            // set minimums
            info->ptMinTrackSize.x = sizeLimitsMin.width;
            info->ptMinTrackSize.y = sizeLimitsMin.height;
            // set maximums
            info->ptMaxTrackSize.x = (sizeLimitsMax.width==0 ? _maxTrackSize.x : sizeLimitsMax.width);
            info->ptMaxTrackSize.y = (sizeLimitsMax.height==0 ? _maxTrackSize.y : sizeLimitsMax.height);
        }
        return TRUE;
    case WM_SYSCOMMAND:
         return (!MenuItem::ProcessMenuCommand((HMENU) wParam) ? FALSE : TRUE);
    case WM_COMMAND:
        {
            word highWord = (wParam>>16)&0xffff, lowWord = wParam&0xffff;
            if (!lParam)
            {
                HMENU hMenu = (HMENU) wParam;
                MenuItem::ProcessMenuCommand(hMenu);
            }
            else
            {// a control sends the command
                HWND hControl = (HWND) lParam;
                //find the wrapping Window object for the control handle
                //by checking through the current window's children
                Window* controlWindow = 0;
                for (dword i = 0;i<win_obj->_children.size();i++)
                    // children are sorted by Window object address, not HWND, so we have to linear search the children
                    if (win_obj->_children[i]->WinHandle()==hControl)
                    {
                        controlWindow = win_obj->_children[i];
                        break;
                    }
                if (controlWindow && controlWindow->MyType()>DialogWindowObject) // the control handle matched a wrapper object
                {
                    /*  Handle individual control messages -
                            Some of these events are subclassed events, among which some are main control events;
                            However, all events are routed through the Window::_raiseEvents method, even if they
                            are subclassed events.
                    */
                    switch (controlWindow->MyType())
                    {
                    case PushButtonControl:
                    case RadioButtonControl:
                    case CheckBoxControl:
                        {
                            if (highWord==BN_CLICKED)
                            {
                                ButtonEventData d(ButtonEventData::Clicked);
                                controlWindow->_raiseEvents(d);
                            }
                            else if (highWord==BN_DOUBLECLICKED)
                            {
                                ButtonEventData d(ButtonEventData::DblClicked);
                                controlWindow->_raiseEvents(d);
                            }
                            else if (highWord==BN_SETFOCUS)
                            {
                                EventData d(WM_SETFOCUS,NULL); // this flag invokes OnGotFocus
                                controlWindow->_raiseEvents(d);
                            }
                            else if (highWord==BN_KILLFOCUS)
                            {
                                EventData d(WM_KILLFOCUS,NULL); // this flag invokes OnKillFocus
                                controlWindow->_raiseEvents(d);
                            }
                            else if (highWord==BN_PAINT)
                            {//raise paint for the control window
                            
                            }
                        }
                        break;
                    case RichEditControl: // shares notifications with EditControl
                    case EditControl:
                        {
                            if (highWord==EN_CHANGE)
                            {
                                TextBoxEventData d(TextBoxEventData::TextChanged);
                                controlWindow->_raiseEvents(d);
                            }
                            else if (highWord==EN_UPDATE)
                            {
                                TextBoxEventData d(TextBoxEventData::TextUpdated);
                                controlWindow->_raiseEvents(d);
                            }
                            else if (highWord==EN_HSCROLL)
                            {
                                TextBoxEventData d(TextBoxEventData::HScrolled);
                                controlWindow->_raiseEvents(d);
                            }
                            else if (highWord==EN_VSCROLL)
                            {
                                TextBoxEventData d(TextBoxEventData::VScrolled);
                                controlWindow->_raiseEvents(d);
                            }
                            else if (highWord==EN_SETFOCUS)
                            {
                                EventData d(WM_SETFOCUS,NULL); // this flag invokes OnGotFocus
                                controlWindow->_raiseEvents(d);
                            }
                            else if (highWord==EN_KILLFOCUS)
                            {
                                EventData d(WM_KILLFOCUS,NULL); // this flag invokes OnKillFocus
                                controlWindow->_raiseEvents(d);
                            }
                        }
                        break;
                    case ListBoxControl:
                        {
                            if (highWord==LBN_SELCHANGE)
                            {
                                ListBoxEventData d(ListBoxEventData::SelChanged);
                                controlWindow->_raiseEvents(d);
                            }
                            else if (highWord==LBN_SELCANCEL)
                            {
                                ListBoxEventData d(ListBoxEventData::SelCanceled);
                                controlWindow->_raiseEvents(d);
                            }
                            else if (highWord==LBN_SETFOCUS)
                            {
                                EventData d(WM_SETFOCUS,NULL); // this flag invokes OnGotFocus
                                controlWindow->_raiseEvents(d);
                            }
                            else if (highWord==LBN_KILLFOCUS)
                            {
                                EventData d(WM_KILLFOCUS,NULL); // this flag invokes OnKillFocus
                                controlWindow->_raiseEvents(d);
                            }
                        }
                        break;
                    case ComboBoxControl:
                        {
                            ComboBoxEventData cd;
                            EventData d;
                            if (highWord==CBN_SELCHANGE)
                            {
                                cd.SetComboBoxEventType(ComboBoxEventData::SelChanged);
                                controlWindow->_raiseEvents(cd);
                            }
                            else if (highWord==CBN_SELENDOK)
                            {
                                cd.SetComboBoxEventType(ComboBoxEventData::SelAffirmed);
                                controlWindow->_raiseEvents(cd);
                            }
                            else if (highWord==CBN_SELENDCANCEL)
                            {
                                cd.SetComboBoxEventType(ComboBoxEventData::SelCanceled);
                                controlWindow->_raiseEvents(cd);
                            }
                            else if (highWord==CBN_EDITUPDATE)
                            {
                                cd.SetComboBoxEventType(ComboBoxEventData::TextUpdated);
                                controlWindow->_raiseEvents(cd);
                            }
                            else if (highWord==CBN_EDITCHANGE)
                            {
                                cd.SetComboBoxEventType(ComboBoxEventData::TextChanged);
                                controlWindow->_raiseEvents(cd);
                            }
                            else if (highWord==CBN_SETFOCUS)
                            {
                                d.tag = WM_SETFOCUS;
                                controlWindow->_raiseEvents(d);
                            }
                            else if (highWord==CBN_KILLFOCUS)
                            {
                                d.tag = WM_KILLFOCUS;
                                controlWindow->_raiseEvents(d);
                            }
                        }
                        break;
                    default: // assume user defined, pass event data down
                        {
                            // Include wParam as the high word, lParam as the low word
                            //  this restriction is part of the implementation
                            EventUserData d((wParam<<16) & lParam,NULL);
                            controlWindow->_raiseEvents(d);
                        }
                        break;
                    }
                }
                else
                {
                    // can't do anything with the notification message
                    return FALSE;
                }
            }
        }
        return TRUE;
    case WM_NOTIFY:
        {
            NMHDR *notify = (NMHDR*) lParam;
            Window* controlWindow = 0;
            // find the control window object by linear-searching through its parent's list of children
            for (dword i = 0;i<win_obj->_children.size();i++)
                if (win_obj->_children[i]->WinHandle()==notify->hwndFrom)
                {
                    controlWindow = win_obj->_children[i];
                    break;
                }
            if (controlWindow)
            {// found the control for which the notification is valid
                // process nofitications based on control type
                if (controlWindow->MyType()==RichEditControl)
                {
                    if (notify->code==EN_MSGFILTER)
                    {// handle key and mouse event messages
                        MSGFILTER *filterInfo = (MSGFILTER*) lParam;
                        switch (filterInfo->msg)
                        {
                        case WM_MOUSEMOVE:
                            {
                                MouseEventData::MouseButton button = MouseEventData::NoButton;
                                if (filterInfo->wParam==MK_LBUTTON)
                                    button = MouseEventData::Left;
                                else if (filterInfo->wParam==MK_RBUTTON)
                                    button = MouseEventData::Right;
                                else if (filterInfo->wParam==MK_MBUTTON)
                                    button = MouseEventData::Middle;
                                MouseEventData d(MouseEventData::Move,button);
                                d.clickLocation.x = filterInfo->lParam&0xffff;
                                d.clickLocation.y = (filterInfo->lParam>>16)&0xffff;
                                controlWindow->_raiseEvents(d);
                            }
                            break;
                        case WM_LBUTTONDOWN:
                            {
                                MouseEventData d(MouseEventData::ClickDown,MouseEventData::Left);
                                d.clickLocation.x = filterInfo->lParam&0xffff;
                                d.clickLocation.y = (filterInfo->lParam>>16)&0xffff;                                
                                controlWindow->_raiseEvents(d);
                            }
                            break;
                        case WM_LBUTTONUP:
                            {
                                MouseEventData d(MouseEventData::ClickUp,MouseEventData::Left);
                                d.clickLocation.x = filterInfo->lParam&0xffff;
                                d.clickLocation.y = (filterInfo->lParam>>16)&0xffff;
                                controlWindow->_raiseEvents(d);
                            }
                            break;
                        case WM_RBUTTONDOWN:
                            {
                                MouseEventData d(MouseEventData::ClickDown,MouseEventData::Right);
                                d.clickLocation.x = filterInfo->lParam&0xffff;
                                d.clickLocation.y = (filterInfo->lParam>>16)&0xffff;
                                controlWindow->_raiseEvents(d);
                            }
                            break;
                        case WM_RBUTTONUP:
                            {
                                MouseEventData d(MouseEventData::ClickUp,MouseEventData::Right);
                                d.clickLocation.x = filterInfo->lParam&0xffff;
                                d.clickLocation.y = (filterInfo->lParam>>16)&0xffff;
                                controlWindow->_raiseEvents(d);
                            }
                            break;
                        }
                    }
                    else if (notify->code==EN_SELCHANGE)
                    {
                        TextBoxEventData d(TextBoxEventData::SelectionChanged);
                        controlWindow->_raiseEvents(d);
                    }
                    else if (notify->code==EN_DROPFILES)
                    {
                        TextBoxEventData d(TextBoxEventData::FileDropped);
                        d.tag = lParam; // set the ENDROPFILES structure ptr to the tag
                        controlWindow->_raiseEvents(d);
                    }
                }
            }
            else
            {
                // can't do anything with the message
                return FALSE;
            }
        }
        return TRUE;
    case WM_LBUTTONDOWN:
        {
            MouseEventData d(MouseEventData::ClickDown,MouseEventData::Left);
            d.clickLocation.x = lParam&0xffff;
            d.clickLocation.y = (lParam>>16)&0xffff;
            win_obj->_raiseEvents(d);
        }
        break;
    case WM_RBUTTONDOWN:
        {
            MouseEventData d(MouseEventData::ClickDown,MouseEventData::Right);
            d.clickLocation.x = lParam&0xffff;
            d.clickLocation.y = (lParam>>16)&0xffff;
            win_obj->_raiseEvents(d);
        }
        break;
    case WM_LBUTTONUP:
        {
            MouseEventData d(MouseEventData::ClickUp,MouseEventData::Left);
            d.clickLocation.x = lParam&0xffff;
            d.clickLocation.y = (lParam>>16)&0xffff;
            win_obj->_raiseEvents(d);
        }
        break;
    case WM_RBUTTONUP:
        {
            MouseEventData d(MouseEventData::ClickUp,MouseEventData::Right);
            d.clickLocation.x = lParam&0xffff;
            d.clickLocation.y = (lParam>>16)&0xffff;
            win_obj->_raiseEvents(d);
        }
        break;
    case WM_KEYDOWN:
        {
            KeyEventData keyData;
            keyData.virtualKey = (int) wParam;
            keyData.repeatCount = lParam&0xffff; // bits 0 - 15
            keyData.isExtended = (lParam&0x1000000) == 0x1000000; // see if bit 24 is on
            keyData.wasDown = (lParam&0x40000000) == 0x40000000; // see if bit 30 is on
            // raise key down event
            keyData.SetKeyEventType(KeyEventData::KeyDown);
            win_obj->_raiseEvents(keyData);
        }
        return TRUE;
    case WM_KEYUP:
        {
            KeyEventData keyData;
            keyData.virtualKey = (int) wParam;
            keyData.repeatCount = lParam&0xffff; // bits 0 - 15
            keyData.isExtended = (lParam&0x1000000) == 0x1000000; // see if bit 24 is on
            keyData.wasDown = (lParam&0x40000000) == 0x40000000; // see if bit 30 is on
            // raise key down event
            keyData.SetKeyEventType(KeyEventData::KeyUp);
            win_obj->_raiseEvents(keyData);
        }
        return TRUE;
    }
    return FALSE;
}

DialogWindow::DialogWindow()
{
    _isModal = false;
}
bool DialogWindow::CreateDlg(const Window* Parent,int X,int Y,int Width,int Height)
{// create a modeless dialog
    if (_hWnd)
        return false; // a window was already created
    // see if there is a main app window to set
    HWND hParent = ( Parent!=0 ? Parent->WinHandle() : global_app_data::GetMainWindow() );
    // set LoadingDialog so _procDialog can add this to the dialog window reference system
    LoadingDialog = this;
    _isModal = false;
    // we have to create the dialog, then apply the user attributes
    _hWnd = CreateDialog(global_app_data::GetAppInstanceModule(),
        MAKEINTRESOURCE(RWIN32_GENERIC_DIALOG),
        hParent,
        &_dialogProc);
    // check success
    if (!_hWnd)
    {
        LoadingDialog = 0; // reset if error
        return false;
    }
    // now, set the user's attributes
    if (Width>0 && Height>0) // do this first so that positioning is correct
        SetSize(Width,Height);
    if (X>=0 && Y>=0)
        SetLocation(X,Y);
    else
    {
        Point loc;
        Size sz;
        if (Parent==0)
        {
            if (LoadedWindows.count(hParent))
                Parent = LoadedWindows[hParent];
        }
        if (Parent!=0)
        {
            sz = Parent->GetSize();
            loc = Parent->GetLocation();
        }
        else
        {
            // use the desktop window dimensions for the resize
            RECT r;
            GetWindowRect(GetDesktopWindow(),&r);
            // (I do the subtraction for convention's sake)
            sz.width = r.right-r.left;
            sz.height = r.bottom-r.top;
            // keep 'loc' at {0,0} 
        }
        loc += _getDefaultWinLocation(sz.width,sz.height);
        SetLocation(loc.x,loc.y);
    }
    // else, use default size from dialog template
    return true;
}
DialogWindow::DialogResult DialogWindow::ShowDlg()
{// create a modal dialog window
    if (_hWnd)
        return DialogError1;
    // set LoadingDialog
    LoadingDialog = this;
    _isModal = true;
    // show the dialog and wait for a result
    DialogResult r = (DialogResult) DialogBox(global_app_data::GetAppInstanceModule(),
        MAKEINTRESOURCE(RWIN32_GENERIC_DIALOG),
        global_app_data::GetMainWindow(),
        &_dialogProc);
    // if error, reset LoadingDialog
    if (r<=DialogError2)
        LoadingDialog = 0;
    return r;
}
DialogWindow::DialogResult DialogWindow::ShowDlg(const Window* Parent)
{// create a modal dialog window
    if (_hWnd)
        return DialogError1;
    // set LoadingDialog
    LoadingDialog = this;
    _isModal = true;
    // show the dialog and wait for a result
    DialogResult r = (DialogResult) DialogBox(global_app_data::GetAppInstanceModule(),
        MAKEINTRESOURCE(RWIN32_GENERIC_DIALOG),
        Parent->WinHandle(),
        &_dialogProc);
    // if error, reset LoadingDialog
    if (r<=DialogError2)
        LoadingDialog = 0;
    return r;
}
bool DialogWindow::CloseDlg(DialogResult Result)
{// this is an alternative to Close (for modal dialogs) that allows the dialog result to be set
    if (_isModal)
    {// this does not raise a Close event
        if (!EndDialog(_hWnd,(INT_PTR) Result))
            return false;
        //destroy any children
        _destroyChildren();
        //reset win handle
        _hWnd = NULL;
        return true;
    }
    else return false;
}
dword DialogWindow::_getWinStyle() const
{// probably not going to use this, but here it is
    return DS_MODALFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU;
}
Point DialogWindow::_getDefaultWinLocation(int Width,int Height) const
{
    Point r;
    if (Width>0 && Height>0)
    {
        Size mySize = GetSize();
        r.x = Width/2-mySize.width/2;
        r.y = Height/2-mySize.height/2;
    }
    return r;
}