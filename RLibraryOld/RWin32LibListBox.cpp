#include "RWin32LibMaster.h"
using namespace rtypes;
using namespace rtypes::rwin32;

ListBox::ListBox()
{
    _sortState = false;
    _delegate = 0;
    EvntSelChanged = 0;
    EvntSelCanceled = 0;
}
ListBox::ListBox(bool SortItems)
{
    _sortState = SortItems;
}
bool ListBox::CreateListBox(const Window* ParentWin)
{
    static const int DEFAULT_LBOX_WIDTH = 120;
    static const int DEFAULT_LBOX_HEIGHT = 95;
    bool success = Create(-1,-1,DEFAULT_LBOX_WIDTH,DEFAULT_LBOX_HEIGHT,ParentWin);
    if (success)
    {
        // add border attrib by default
        SetAttribute(Border);
        // format height for 7 items (default)
        FormatLBoxHeight(7);
    }
    return success;
}
void ListBox::OnSelChanged(const ListBoxEventData&)
{
    /* do nothing */
}
void ListBox::OnSelCanceled(const ListBoxEventData&)
{
    /* do nothing */
}
str ListBox::GetItem(int Index) const
{
    str buf(GetItemLength(Index)+1);
    // let Windows copy the listbox item into the buffer
    if (SendMessage(_hWnd,LB_GETTEXT,Index,(LPARAM)&buf[0])==LB_ERR)
        return str();
    return buf;
}
sized_container<str> ListBox::GetItems() const
{
    unsigned int size = unsigned int(GetNumberOfItems());
    sized_container<str> items(size);
    for (unsigned i = 0;i<size;i++)
        items[i] = GetItem(i);
    return items;
}
void ListBox::FormatLBoxHeight(int NumberOfItems)
{
    if (NumberOfItems<=0 && *this)
        return;
    HDC hDc = GetDC(_hWnd);
    if (hDc)
    {
        TEXTMETRIC info;
        // check the mapping mode
        if (GetMapMode(hDc)!=MM_TEXT)
            SetMapMode(hDc,MM_TEXT); // this way it operates in pixels
        // get font metrics
        GetTextMetrics(hDc,&info);
        // calc. and change height; keep width the same
        SetSize(-1,NumberOfItems*(info.tmAscent+2)); // add 2 for display purposes
        ReleaseDC(_hWnd,hDc);
    }
}
const char* ListBox::_getWinClass() const
{
    return WC_LISTBOX;
}
dword ListBox::_getWinStyle() const
{
    return WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_VISIBLE | LBS_NOTIFY | (_sortState ? LBS_SORT : NULL);
}
void ListBox::_raiseSubEvents(EventData& data)
{
    // 'data' is a ListBoxEventData object in this context
    ListBoxEventData& lbox_data = static_cast<ListBoxEventData&> (data);
    // find which event the data refers to
    switch (lbox_data.MyEventType())
    {
    case ListBoxEventData::SelChanged:
        // this is the main event for a listbox
        if (_delegate)
        {
            // find the parent window
            Window* par = const_cast<Window*> (GetParentWin());
            if (par)
                // the listbox has a parent
                (par->*_delegate)(this);
        }
        if (EvntSelChanged)
        {
            data.SetFunctCall(EvntSelChanged);
            data.RaiseEvent();
        }
        OnSelChanged(lbox_data);
        break;
    case ListBoxEventData::SelCanceled:
        if (EvntSelCanceled)
        {
            data.SetFunctCall(EvntSelCanceled);
            data.RaiseEvent();
        }
        OnSelCanceled(lbox_data);
        break;
    }
}
Point ListBox::_getDefaultWinLocation(int Width,int Height) const
{
    return Point();
}

MultiSelListBox::MultiSelListBox()
{
}
MultiSelListBox::MultiSelListBox(bool SortItems)
    : ListBox(SortItems)
{
}
bool MultiSelListBox::SelectRange(int StartIndex,int EndIndex,bool Deselect) const
{
    // the high-word of indexData is the end index; the low-word is the start index
    dword indexData = (word) EndIndex;
    indexData <<= 16;
    indexData += (word) StartIndex;
    return SendMessage(_hWnd,LB_SELITEMRANGE,(Deselect ? TRUE : FALSE),indexData)!=LB_ERR;
}
void MultiSelListBox::RemoveSelection()
{
    // remove all indexes in case they are selected
    int cnt = GetNumberOfItems();
    for (int i = 0;i<cnt;i++)
        RemoveSelectedIndex(i);
}
sized_container<int> MultiSelListBox::GetSelectedIndeces() const
{
    unsigned int itemCnt = unsigned int(GetNumberOfSelItems());
    sized_container<int> selItems(itemCnt);
    // get the complete number of sel items from the box
    if (SendMessage(_hWnd,LB_GETSELITEMS,itemCnt,(LPARAM)&selItems[0])==LB_ERR)
        selItems.resize(0); // dealloc array
    return selItems;
}
bool MultiSelListBox::SetSelectedIndex(const str& Item,int StartSearchIndex)
{
    int Index = GetItemIndex(Item,StartSearchIndex);
    return SetSelectedIndex(Index);
}
dword MultiSelListBox::_getWinStyle() const
{
    return WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_VISIBLE | LBS_NOTIFY | LBS_MULTIPLESEL | LBS_EXTENDEDSEL | (_sortState ? LBS_SORT : NULL);
}