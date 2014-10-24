#include "RWin32LibMaster.h"
using namespace rtypes;
using namespace rtypes::rwin32;

ComboBox::ComboBox()
{
    _delSelChng = 0;
    _delSelAffirm = 0;
    _delSelCancel = 0;
    EvntSelectionChanged = 0;
    EvntSelectionAffirmed = 0;
    EvntSelectionCanceled = 0;
    EvntTextChanged = 0;
    EvntTextUpdated = 0;
}
bool ComboBox::AddItemFromBox(bool CheckIfExists)
{
    str s = GetText();
    dword dummy;
    if (CheckIfExists && SearchItemExact(dummy,s))
        return false;
    AddItem(s);
    return true;
}
bool ComboBox::InsertItemFromBox(bool CheckIfExists)
{
    str s = GetText();
    dword dummy;
    if (CheckIfExists && SearchItemExact(dummy,s))
        return false;
    AddItem(s);
    return true;
}
void ComboBox::AddItem(const str& Item,bool SetExtent)
{
    if (!_hWnd)
        return;
    if (SetExtent)
    {
        HDC hDc = GetDC(_hWnd);
        if (hDc)
        {
            // automatically manage horizontile extent for scroll bar functionality
            SIZE sz;
            if (GetTextExtentPoint32(hDc,Item.c_str(),(int) Item.size(),&sz))
            {
                int myWidth = GetSize().width;
                int curExtent = SendMessage(_hWnd,CB_GETHORIZONTALEXTENT,NULL,NULL);
                if (sz.cx > curExtent && sz.cx > myWidth)
                    // set the horizontile text limit to show the long element
                    SendMessage(_hWnd,CB_SETHORIZONTALEXTENT,sz.cx,NULL);
            }
        }
    }
    _addItem(Item.c_str());
}
void ComboBox::InsertItem(const str& Item,dword InsertionIndex,bool SetExtent)
{
    if (!_hWnd)
        return;
    if (SetExtent)
    {
        HDC hDc = GetDC(_hWnd);
        if (hDc)
        {
            // automatically manage horizontile extent for scroll bar functionality
            SIZE sz;
            if (GetTextExtentPoint32(hDc,Item.c_str(),(int) Item.size(),&sz))
            {
                int myWidth = GetSize().width;
                int curExtent = SendMessage(_hWnd,CB_GETHORIZONTALEXTENT,NULL,NULL);
                if (sz.cx > curExtent && sz.cx > myWidth)
                    // set the horizontile text limit to show the long element
                    SendMessage(_hWnd,CB_SETHORIZONTALEXTENT,sz.cx,NULL);
            }
        }
    }
    _insertItem(Item.c_str(),InsertionIndex);
}
void ComboBox::AddItemRange(const char** ItemArray,dword Size)
{
    for (dword i = 0;i<Size;i++)
        _addItem(ItemArray[i]);
}
void ComboBox::InsertItemRange(const char** ItemArray,dword Size,dword InsertionIndex)
{
    for (dword i = 0;i<Size;i++,InsertionIndex++)
        _insertItem(ItemArray[i],InsertionIndex);
}
void ComboBox::AddItemRange(const str ItemArray[],dword Size)
{
    for (dword i = 0;i<Size;i++)
        AddItem(ItemArray[i]);
}
void ComboBox::InsertItemRange(const str ItemArray[],dword Size,dword InsertionIndex)
{
    for (dword i = 0;i<Size;i++,InsertionIndex++)
        InsertItem(ItemArray[i],InsertionIndex);
}
void ComboBox::AddItemRange(const container<str>& ItemArray)
{
    for (dword i = 0;i<ItemArray.size();i++)
        AddItem(ItemArray[i]);
}
void ComboBox::InsertItemRange(const container<str>& ItemArray,dword InsertionIndex)
{
    for (dword i = 0;i<ItemArray.size();i++,InsertionIndex++)
        InsertItem(ItemArray[i],InsertionIndex);
}
bool ComboBox::RemoveItemRange(dword IndexStart,dword IndexEnd)
{
    bool hadFailure = false;
    while (IndexStart<=IndexEnd)
    {
        if (!RemoveItem(IndexStart))
            hadFailure = true;
        IndexStart++;
    }
    return !hadFailure;
}
bool ComboBox::SearchItem(dword &IndexFound_out,const str& ItemText,int StartIndex)
{
    dword indexResult;
    indexResult = SendMessage(_hWnd,CB_FINDSTRING,StartIndex,(LPARAM) ItemText.c_str());
    if (indexResult!=CB_ERR)
    {
        IndexFound_out = indexResult;
        return true;
    }
    return false;
}
bool ComboBox::SearchItemExact(dword &IndexFound_out,const str& ItemText,int StartIndex)
{
    dword indexResult;
    indexResult = SendMessage(_hWnd,CB_FINDSTRINGEXACT,StartIndex,(LPARAM) ItemText.c_str());
    if (indexResult!=CB_ERR)
    {
        IndexFound_out = indexResult;
        return true;
    }
    return false;
}
void ComboBox::OnSelectionChanged(const ComboBoxEventData&)
{
    /* do nothing */
}
void ComboBox::OnSelectionAffirmed(const ComboBoxEventData&)
{
    /* do nothing */
}
void ComboBox::OnSelectionCanceled(const ComboBoxEventData&)
{
    /* do nothing */
}
void ComboBox::OnTextChanged(const ComboBoxEventData&)
{
    /* do nothing */
}
void ComboBox::OnTextUpdated(const ComboBoxEventData&)
{
    /* do nothing */
}
bool ComboBox::SetCueBanner(wstr BannerText)
{
    BannerText.truncate(defaults::BANNER_BUFFER_SIZE);
    return SendMessage(_hWnd,CB_SETCUEBANNER,NULL,(LPARAM) BannerText.c_str())==1; 
}
bool ComboBox::SetSelectionInfo(dword SelStart,dword SelEnd)
{
    dword lParam = (word) SelStart;
    lParam |= ((word) SelEnd)<<16;
    return SendMessage(_hWnd,CB_SETEDITSEL,NULL,lParam)==TRUE;
}
str ComboBox::GetItem(dword Index) const
{
    str s(GetItemLength(Index)+1); // add 1 for the null character
    SendMessage(_hWnd,CB_GETLBTEXT,Index,(LPARAM) &s[0]);
    return s;
}
wstr ComboBox::GetCueBanner() const
{
    wstr bannerText(defaults::BANNER_BUFFER_SIZE+1);
    if (SendMessage(_hWnd,CB_GETCUEBANNER,(WPARAM) &bannerText[0],defaults::BANNER_BUFFER_SIZE+1)!=1)
        return wstr();
    // set the string size for accurate reporting
    dword size = 0;
    while (size<defaults::BANNER_BUFFER_SIZE && bannerText[size])
        size++;
    bannerText.truncate(size);
    return bannerText;
}
dword ComboBox::GetSelectionInfo(dword &SelStart,dword &SelEnd) const
{
    SendMessage(_hWnd,CB_GETEDITSEL,(WPARAM) &SelStart,(LPARAM) &SelEnd);
    // make the selection end index inclusive
    SelEnd--;
    return SelEnd-SelStart+1;   
}
const char* ComboBox::_getWinClass() const
{
    return WC_COMBOBOX;
}
dword ComboBox::_getWinStyle() const
{
    return WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | CBS_SIMPLE;
}
void ComboBox::_raiseSubEvents(EventData& data)
{
    // data must be a ComboBoxEventData object
    ComboBoxEventData& cb_data = static_cast<ComboBoxEventData&> (data);
    Window* par = const_cast<Window*> (GetParentWin());
    switch (cb_data.MyEventType())
    {
    case ComboBoxEventData::SelChanged:
        // raise the delegate
        if (_delSelChng && par)
            (par->*_delSelChng)(this);
        // raise other events
        OnSelectionChanged(cb_data);
        if (EvntSelectionChanged)
        {
            cb_data.SetFunctCall(EvntSelectionChanged);
            cb_data.RaiseEvent();
        }
        break;
    case ComboBoxEventData::SelAffirmed:
        // raise the delegate
        if (_delSelAffirm && par)
            (par->*_delSelAffirm)(this);
        // raise other events
        OnSelectionAffirmed(cb_data);
        if (EvntSelectionAffirmed)
        {
            cb_data.SetFunctCall(EvntSelectionAffirmed);
            cb_data.RaiseEvent();
        }
        break;
    case ComboBoxEventData::SelCanceled:
        // raise the delegate
        if (_delSelCancel && par)
            (par->*_delSelCancel)(this);
        // raise other events
        OnSelectionCanceled(cb_data);
        if (EvntSelectionCanceled)
        {
            cb_data.SetFunctCall(EvntSelectionCanceled);
            cb_data.RaiseEvent();
        }
        break;
    case ComboBoxEventData::TextChanged:
        OnTextChanged(cb_data);
        if (EvntTextChanged)
        {
            cb_data.SetFunctCall(EvntTextChanged);
            cb_data.RaiseEvent();
        }
        break;
    case ComboBoxEventData::TextUpdated:
        OnTextChanged(cb_data);
        if (EvntTextUpdated)
        {
            cb_data.SetFunctCall(EvntTextUpdated);
            cb_data.RaiseEvent();
        }
        break;
    }
}

dword DropDownComboBox::_getWinStyle() const
{
    return WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | CBS_DROPDOWN;
}

dword DropDownListComboBox::_getWinStyle() const
{
    return WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | CBS_DROPDOWNLIST;
}