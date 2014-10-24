#include "RWin32LibMaster.h"
using namespace rtypes;
using namespace rtypes::rwin32;

#undef RWIN32LIB_ENABLE_VISUAL_STYLES

Button::Button()
{
    _delegate = 0;
    EvntClick = 0;
    EvntDoubleClick = 0;
}
Button::ButtonState Button::GetCheckState() const
{
    int r = SendMessage(_hWnd,BM_GETSTATE,NULL,NULL);
    if (_containsStyle(r,BST_CHECKED))
        return Checked;
    if (_containsStyle(r,BST_UNCHECKED))
        return Unchecked;
    if (_containsStyle(r,BST_INDETERMINATE))
        return Indeterminate;
    return NullButtonState;
}
bool Button::CreateButton(const Window* ParentWin)
{
    return Create(-1,-1,defaults::DEFAULT_BTN_WIDTH,defaults::DEFAULT_BTN_HEIGHT,ParentWin);
}
void Button::OnClick(const ButtonEventData&)
{
    //do nothing
}
void Button::OnDoubleClick(const ButtonEventData&)
{
    //do nothing
}
void Button::Click(ButtonEventData& d)
{
    Window* par = const_cast<Window*> ( GetParentWin() );
    if (par)
    {// means that the button has a parent
        if (_delegate)
            (par->*_delegate)(this);
    }
    if (EvntClick)
    {
        d.SetFunctCall(EvntClick);
        d.RaiseEvent();
    }
    OnClick(d);
}
const char* Button::_getWinClass() const
{
    return WC_BUTTON;
}
dword Button::_getWinStyle() const
{
    return WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_CENTER | BS_NOTIFY /* BS_NOTIFY sends more notifications */;
}
void Button::_raiseSubEvents(EventData& ev_data)
{
    // ev_data must be an object of type 'ButtonEventData'
    ButtonEventData& button_data = static_cast<ButtonEventData&> (ev_data);
    //find which event the data refers to
    switch (button_data.MyEventType())
    {
    case ButtonEventData::Clicked:
        // this is the main event for a Button
        Click(button_data); // this invokes all 'Click' routines
        break;
    case ButtonEventData::DblClicked:
        if (EvntDoubleClick)
        {
            ev_data.SetFunctCall(EvntDoubleClick);
            ev_data.RaiseEvent();
        }
        OnDoubleClick(button_data);
        break;
    }
}
Point Button::_getDefaultWinLocation(int Width,int Height) const
{
    return Point();
}

bool RadioButton::_hasLeader = false;
RadioButton::RadioButton()
{
    _isLeader = false;
}
bool RadioButton::CreateRadioButton(const Window* ParentWin,bool IsLeader)
{
    _isLeader = IsLeader || !_hasLeader;
    // set background color to parent window background color
     // might do this in Button::CreateButton
    return CreateButton(ParentWin);
}
void RadioButton::OnCreate(const EventData&)
{
    if (_isLeader)
        _hasLeader = true;
}
void RadioButton::OnDestroy(const EventData&)
{
    // this needs to be reset so new groups
    // aren't created without user approval
    _isLeader = false;
}
dword RadioButton::_getWinStyle() const
{
    // WS_GROUP|WS_TABSTOP ("leader" style) makes a new tab group with all subsequence radio box creations including the leader
    return WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | BS_LEFT | BS_NOTIFY | (_isLeader ? WS_GROUP|WS_TABSTOP : NULL);
}

dword CheckBoxButton::_getWinStyle() const
{
    return WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | BS_LEFT | BS_NOTIFY;
}

dword CheckBox3StateButton::_getWinStyle() const
{
    return WS_CHILD | WS_VISIBLE | BS_AUTO3STATE | BS_LEFT | BS_NOTIFY;
}

Point GroupBox::CalcElementLocation(const Point& PositionRelative) const
{
    Point r = GetLocation();
    r += PositionRelative;
    return r;
}
dword GroupBox::_getWinStyle() const
{
    return WS_CHILD | WS_VISIBLE | BS_GROUPBOX | BS_NOTIFY;
}

bool CommandLink::SetNoteText(const wstr& s)
{
    if (SendMessage(_hWnd,BCM_SETNOTE,NULL,(LPARAM) s.c_str())!=FALSE)
        return true;
    return false;
}
dword CommandLink::_getWinStyle() const
{
    return WS_CHILD | WS_VISIBLE | BS_COMMANDLINK | BS_NOTIFY;
}