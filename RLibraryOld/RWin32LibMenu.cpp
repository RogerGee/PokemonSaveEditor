#include "RWin32LibMaster.h"
#include <map>
using namespace std;
using namespace rtypes;
using namespace rtypes::rwin32;

namespace
{
    map<HMENU,MenuItem*> LoadedItems; /* I do bother to remove key-value pairs that have expired, since I leave access to menu
                                                command processing open to users.
                                        */
}

bool MenuItem::ProcessMenuCommand(HMENU hMenu)
{
    if (LoadedItems.count(hMenu)==1)
    {// the menu handle is registered in our system
        LoadedItems[hMenu]->_invokeDelegate();
        return true;
    }
    return false;
}

dword MenuItem::_menuIDCount = 0;
MenuItem::MenuItem()
{
    _hSubMenu = CreatePopupMenu();
    // add pair to LoadedItems to reference this menu item object by its menu handle value
    LoadedItems[_hSubMenu] = this;
    _myPos = 0;
    _parentState = true;
    _parentBar = 0; // this means the menu item is by default a top-level item on a menu bar
    _popupState = false;
    _checkState = false;
    _enabledState = true;
    _delegate = 0; // or routine = 0
    _eventState = true;
    _allocState = false;
    _radioState = false;
    // set unique menu id
    _myID = _menuIDCount++; // I don't bother to recycle id's, seeing as no one's going to create 0xffffffff menu items
}
// this c-stor is private and is used to load a pre-existing menu item and its children
MenuItem::MenuItem(HMENU hMenu)
{
    _hSubMenu = hMenu; // this is all the information we can get from ourselves
    // whatever is creating us can set other information, we just need to worry about our children
    int itemCount = GetMenuItemCount(_hSubMenu);
    _popupState = itemCount>0; // if we have children, we're a popup menu
    _eventState = false;
    _delegate = 0;
    for (int i = 0;i<itemCount;i++)
    {
        MENUITEMINFO info;
        ZeroMemory(&info,sizeof(MENUITEMINFO));
        info.cbSize = sizeof(MENUITEMINFO);
        info.fMask = MIIM_ID | MIIM_STRING | MIIM_STATE | MIIM_TYPE;
        // first call to get menu item HMENU and menu item string size
        GetMenuItemInfo(_hSubMenu,i,TRUE,&info);
        // create MenuItem object
        MenuItem* item = new MenuItem((HMENU) info.wID);
        item->_myPos = i;
        item->_allocState = true; // flag this item as being dynamically allocated and as not owned by our menu system
        item->_parentState = true; // this context is the parent and is a menu item
        item->_parentItem = this; // assign ourself as the item's parent
        item->_checkState = (info.fState&MFS_CHECKED) == MFS_CHECKED;
        item->_enabledState = (info.fState&MFS_DISABLED) != MFS_DISABLED;
        item->_radioState = (info.fType&MFT_RADIOCHECK)==MFT_RADIOCHECK && item->_checkState;
        // create a buffer for the menu item's text
        item->_text = str(++info.cch); // add 1 to account for the null terminating character
        info.dwTypeData = &item->_text[0];
        // second call to get menu item string
        info.fMask = MIIM_STRING;
        GetMenuItemInfo(_hSubMenu,i,TRUE,&info);
        // add the menu item
        _children.push_back(item);
    }
}
MenuItem::MenuItem(const MenuItem& obj)
{// unimplemented, since I want to deny copying a menu item
}
MenuItem::~MenuItem()
{
    for (dword i = 0;i<_children.size();i++)
    {
        if (_children[i]->_allocState)
        {
            // the item object was created dynamically from a pre-existing menu handle
            delete _children[i];
            _children.remove_at(i--);
        }
        else
        {//remove our children to tell them we're being destroyed
            RemoveItem(i--);
        }
    }
    //remove item from its parent
    if (_parentBar && !_allocState) // if we're dynamically allocated, then our parent must already be removing us
    {
        if (_parentState)
        {// we're a child of another sub menu
            _parentItem->RemoveItem(*this);
        }
        else
        {// we're top level on a menu bar
            _parentBar->RemoveItem(*this);
        }
    }
    else if (_allocState)
        return; // we don't own the menu handle, we just use it
    // destroy the menu item that the object refers to
    DestroyMenu(_hSubMenu); // this function may or may not fail
    // remove the expired pair from the reference system
    if (LoadedItems.count(_hSubMenu))
        LoadedItems.erase(_hSubMenu);
}
void MenuItem::SetEvent(EventDelegate_id Delegate)
{
    _delegate = Delegate;
    _eventState = true;
}
void MenuItem::SetEvent(RoutinePtr Routine)
{
    _routine = Routine;
    _eventState = false;
}
void MenuItem::SetText(const str& s)
{
    if (s.size()==0)
        // we cannot have a string with no size
        _text = "Item";
    else
        _text = s; // store text in our own buffer in anticipation that the item is going to be added to another item or bar
    if (_parentItem!=0) // (I can use either _parentItem or _parentBar since they both refer to the same memory address)
    {// the item has a parent, so we need to call an API routine to change the text
        HMENU hTopLevel; // not the absolute top level, but a level above this menu
        hTopLevel = (_parentState ? _parentItem->_hSubMenu : _parentBar->GetMenuHandle());
        // modify text only
        ModifyMenu(hTopLevel,*this,MF_BYCOMMAND | MF_STRING,*this,&_text[0]);
    }
    // if the item doesn't have a parent, then this item's text will be added when the item is added to 
}
bool MenuItem::SetCheckState(bool On)
{
    if (!_parentState || IsSeparator()) // can't render this on items whose next top level is a menu bar or seps
        return false;
    HMENU hTopLevel = _parentItem->_hSubMenu;
    if (_text.size()==0)
        return false; // need text set for this operation
    //if (ModifyMenu(hTopLevel,*this,MF_BYCOMMAND | (On ? MF_CHECKED : MF_UNCHECKED),*this,&_text[0]))
    //{
    //  _checkState = On;
    //  return true;
    //}
    //return false;
    MENUITEMINFO info;
    ZeroMemory(&info,sizeof(MENUITEMINFO));
    info.cbSize = sizeof(MENUITEMINFO);
    info.fMask = MIIM_FTYPE | MIIM_STATE;
    if (!GetMenuItemInfo(hTopLevel,(UINT)_hSubMenu,FALSE,&info))
        return false;
    if (On)
    {
        if (_radioState)
        {
            // change the check state of other items in the radio group to which this item belongs 
            container<MenuItem*> parentChildrenSorted = _parentItem->_getChildrenSortedByIndex();
            for (dword i = 0;i<_parentItem->_radioGroups.size();i++)
            {
                Point& p = _parentItem->_radioGroups[i];
                if (_myPos>=dword(p.x)/*start*/ && _myPos<=dword(p.y)/*end*/)
                    for (dword start = p.x;start<=dword(p.y);start++)
                        if (start==_myPos)
                            continue;
                        else
                            parentChildrenSorted[start]->SetCheckState(false);
            }
            // add radio check flag to this item
            info.fType |= MFT_RADIOCHECK;
        }
        info.fState |= MFS_CHECKED;
    }
    else
    {
        if (_radioState)
            info.fType &= ~MFT_RADIOCHECK;
        info.fState &= ~MFS_CHECKED; // MFS_UNCHECKED (val==0x00) doesn't work
    }
    _checkState = On;
    return SetMenuItemInfo(hTopLevel,(UINT)_hSubMenu,FALSE,&info)!=FALSE;
}
bool MenuItem::ToggleCheckState()
{
    return SetCheckState(!_checkState);
}
bool MenuItem::AddItem(MenuItem& Item)
{
    //make sure 'Item' already hasn't been added
    dword dummy;
    if (_children.binary_search(&Item,dummy))
        return false; // already added
    // append the menu using the api
    if (!AppendMenu(_hSubMenu,MF_STRING,Item,Item._text.c_str()))
        return false;
    // set parent ptr
    Item._parentItem = this;
    Item._parentState = true;
    // set item pos
    Item._myPos = _children.size();
    // modify popup flag if needed
    if (!_popupState)
    {// this menu item is becoming a popup menu, so its state needs to be modified
        HMENU hTopLevel;
        // find the top level menu
        if (!_parentItem)
            return false; // the item needed to have been added to either an item or a bar
        else
            hTopLevel = (_parentState ? _parentItem->_hSubMenu : _parentBar->GetMenuHandle());
        if (!ModifyMenu(hTopLevel,*this,MF_BYCOMMAND | MF_POPUP,*this,&_text[0]))
            return false;
        _popupState = true;
    }
    _children.push_back(&Item);
    //sort the container
    _children.insertion_sort();
    _findUpdate();
    return true;
}
bool MenuItem::AddSeparator()
{
    MenuItem* itemSep = new MenuItem;
    itemSep->_allocState = true; // flag this as dynamically allocated so that it will be deleted later
    // append the menu as a separator
    if (!AppendMenu(_hSubMenu,MF_SEPARATOR,*itemSep,NULL))
    {
        delete itemSep;
        return false;
    }
    // set parent ptrs
    itemSep->_parentItem = this;
    itemSep->_parentState = true;
    // set item pos
    itemSep->_myPos = _children.size();
    // modify popup flag if needed
    if (!_popupState)
    {// this menu item is becoming a popup menu, so its state needs to be modified
        HMENU hTopLevel;
        // find the top level menu
        if (!_parentItem)
        {
            delete itemSep;
            return false; // the item needed to have been added to either an item or a bar
        }
        else
            hTopLevel = (_parentState ? _parentItem->_hSubMenu : _parentBar->GetMenuHandle());
        if (!ModifyMenu(hTopLevel,*this,MF_BYCOMMAND | MF_POPUP,*this,&_text[0]))
        {
            delete itemSep;
            return false;
        }
        _popupState = true;
    }
    _children.push_back(itemSep);
    _children.insertion_sort();
    _findUpdate();
    return true;
}
bool MenuItem::InsertItem(MenuItem& Item,dword PositionIndex)
{
    // make sure the item hasn't already been added
    dword dummy;
    if (_children.binary_search(&Item,dummy))
        return false;
    // insert the menu item
    if (!InsertMenu(_hSubMenu,PositionIndex,MF_BYPOSITION|MF_STRING,Item,Item._text.c_str()))
        return false;
    // set parent ptr
    Item._parentItem = this;
    Item._parentState = true;
    // set item pos
    Item._myPos = PositionIndex;
    // inform other children that their position index has changed
    _chngChildPos(PositionIndex,true);
    // modify popup flag if needed
    if (!_popupState)
    {// this menu is about to become a popup menu, so its state needs to be modified
        HMENU hTopLevel;
        // find the top level menu
        if (!_parentItem)
            return false; // the item needed to have been added the either an item or a bar
        else
            hTopLevel = (_parentState ? _parentItem->_hSubMenu : _parentBar->GetMenuHandle());
        if (!ModifyMenu(hTopLevel,*this,MF_BYCOMMAND | MF_POPUP,*this,&_text[0]))
            return false;
        _popupState = true;
    }
    // add to child list
    _children.push_back(&Item);
    // sort the container
    _children.insertion_sort();
    _findUpdate();
    return true;
}
bool MenuItem::InsertSeparator(dword PositionIndex)
{
    MenuItem* itemSep = new MenuItem;
    itemSep->_allocState = true; // this will let us delete this later
    // insert the menu item as a separator
    if (!InsertMenu(_hSubMenu,PositionIndex,MF_BYPOSITION|MF_SEPARATOR,*itemSep,NULL))
    {
        delete itemSep;
        return false;
    }
    // set parent ptrs
    itemSep->_parentItem = this;
    itemSep->_parentState = true;
    // set item pos
    itemSep->_myPos = PositionIndex;
    // inform other children that their positions have changed
    _chngChildPos(PositionIndex,true);
    // modify popup flag (if needed)
    if (!_popupState)
    {// this menu item is becoming a popup menu, so its state needs to be modified
        HMENU hTopLevel;
        // find the top level menu
        if (!_parentItem)
        {
            delete itemSep;
            return false; // the item needed to have been added to either an item or a bar
        }
        else
            hTopLevel = (_parentState ? _parentItem->_hSubMenu : _parentBar->GetMenuHandle());
        if (!ModifyMenu(hTopLevel,*this,MF_BYCOMMAND | MF_POPUP,*this,&_text[0]))
        {
            delete itemSep;
            return false;
        }
        _popupState = true;
    }
    _children.push_back(itemSep);
    _children.insertion_sort();
    _findUpdate();
    return true;
}
void MenuItem::RemoveAllItems()
{
    dword lastSize = _children.size();
    for (dword i = 0;i<_children.size();i++)
    {
        RemoveItem(i);
        i -= lastSize-_children.size();
        lastSize = _children.size();
    }
}
bool MenuItem::RemoveItem(MenuItem& item)
{
    HMENU hRemoveMe = item._hSubMenu;
    // find index of occurance
    dword i;
    if (!_children.binary_search(&item,i) || i>=_children.size() || _children[i]->_allocState)
        return false; // can't remove
    // inform the child that it no longer has a parent
    _children[i]->_parentItem = 0;
    // remove from our children
    _children.remove_at(i);
    // tell other children that their positions have changed
    _chngChildPos(item._myPos,false);
    //tell Windows to remove the menu item from the parent
    if (!RemoveMenu(_hSubMenu,(UINT)hRemoveMe,NULL)) // looks at the identifier, not the position
        return false;
    //change menu item parameters if no longer a drop down
    if (_popupState && !_children.size() && _parentItem)
    {
        HMENU hTopLevel = (_parentState ? _parentItem->_hSubMenu : _parentBar->GetMenuHandle());
        // I don't know how (or if) this is possible
    }
    //update window menu bar
    _findUpdate();
    return true;
}
bool MenuItem::RemoveItem(dword Index)
{
    if (Index>=_children.size() || _children[Index]->_allocState)
        return false;
    HMENU hRemoveMe = GetItem(Index)->_hSubMenu;
    // inform the child that it no longer has a parent
    _children[Index]->_parentItem = 0;
    // remove from our children list
    _children.remove_at(Index);
    // tell other children that their positions have changed
    _chngChildPos(Index,false);
    //tell Windows to remove the menu item from the parent
    if (!RemoveMenu(_hSubMenu,(UINT)hRemoveMe,NULL)) // looks at the identifier, not the position
        return false;
    //change menu item parameters if no longer a drop down
    if (_popupState && !_children.size() && _parentItem)
    {
        HMENU hTopLevel = (_parentState ? _parentItem->_hSubMenu : _parentBar->GetMenuHandle());
        // I don't know how (or if) this is possible
    }
    //update window menu bar
    _findUpdate();
    return true;
}
bool MenuItem::EnableMenuItem()
{
    UINT flags = MF_BYCOMMAND | MF_ENABLED;
    HMENU hTopLevel;
    // find the top level menu
    if (!_parentItem)
        return false; // the item needed to have been added the either an item or a bar
    else
        hTopLevel = (_parentState ? _parentItem->_hSubMenu : _parentBar->GetMenuHandle());
    bool success = ::EnableMenuItem(hTopLevel,*this,flags) != -1;
    if (success)
        _enabledState = true;
    return success;
}
bool MenuItem::DisableMenuItem()
{
    UINT flags = MF_BYCOMMAND | MF_DISABLED;
    HMENU hTopLevel;
    // find the top level menu
    if (!_parentItem)
        return false; // the item needed to have been added the either an item or a bar
    else
        hTopLevel = (_parentState ? _parentItem->_hSubMenu : _parentBar->GetMenuHandle());
    bool success = ::EnableMenuItem(hTopLevel,*this,flags) != -1;
    if (success)
        _enabledState = false;
    return success;
}
bool MenuItem::GrayOutMenuItem()
{
    UINT flags = MF_BYCOMMAND | MF_GRAYED;
    HMENU hTopLevel;
    // find the top level menu
    if (!_parentItem)
        return false; // the item needed to have been added the either an item or a bar
    else
        hTopLevel = (_parentState ? _parentItem->_hSubMenu : _parentBar->GetMenuHandle());
    bool success = ::EnableMenuItem(hTopLevel,*this,flags) != -1;
    if (success)
        _enabledState = false;
    return success;
}
bool MenuItem::CreateRadioGroup(dword PositionFirst,
    dword PositionLast,
    dword PositionSelected)
{
    if (PositionLast<PositionFirst || PositionFirst>=_children.size() 
        || PositionLast>=_children.size() || PositionSelected<PositionFirst 
        || PositionSelected>PositionLast
        || _text.size()==0/* text needs to already have been set */
        || PositionFirst==PositionLast /* needs to have at least two items */)
        return false;
    container<MenuItem*> itemsSorted = _getChildrenSortedByIndex();
    // check to see if items in the range are part of another radio group
    for (dword i = PositionFirst;i<=PositionLast;i++)
        if (itemsSorted[i]->_radioState)
            return false;
    bool success = CheckMenuRadioItem(_hSubMenu,
        PositionFirst,
        PositionLast,
        PositionSelected,
        MF_BYPOSITION)!=FALSE;
    if (success)
    {
        // set state flags for our reference
        Point range;
        range.x = PositionFirst;
        for (range.y = range.x;range.y<=(int)PositionLast;range.y++)
            itemsSorted[range.y]->_radioState = true;
        // the radioed item needs to be marked as checked
        itemsSorted[PositionSelected]->_checkState = true;
        _radioGroups.push_back(range);
    }
    return success;
}
bool MenuItem::RemoveRadioGroup(dword PositionFirst,dword PositionLast)
{
    container<MenuItem*> childrenSorted = _getChildrenSortedByIndex();
    for (dword i = 0;i<_radioGroups.size();i++)
    {
        if (_radioGroups[i]==Point(PositionFirst,PositionLast))
        {
            for (dword start = _radioGroups[i].x;start<=dword(_radioGroups[i].y);start++)
            {
                if (childrenSorted[start]->_checkState)
                    childrenSorted[start]->SetCheckState(false);
                childrenSorted[start]->_radioState = false;
            }
            _radioGroups.remove_at(i);
            return true;
        }
    }
    return false;
}
bool MenuItem::RemoveRadioGroup(const MenuItem& item)
{
    container<MenuItem*> childrenSorted = _getChildrenSortedByIndex();
    for (dword i = 0;i<_radioGroups.size();i++)
    {
        bool found = false;
        for (dword start = _radioGroups[i].x;start<=dword(_radioGroups[i].y);start++)
        {
            if (found)
            {
                if (childrenSorted[start]->_checkState)
                    childrenSorted[start]->SetCheckState(false);
                childrenSorted[start]->_radioState = false;
            }
            else if (childrenSorted[start]==&item)
            {
                // restart the loop to close children
                start = _radioGroups[i].x;
                found = true;
                continue;
            }
        }
        if (found)
        {
            _radioGroups.remove_at(i);
            return true;
        }
    }
    return false;
}
void MenuItem::RemoveAllRadioGroups()
{
    _radioGroups.remove_all();
    for (dword i = 0;i<_children.size();i++)
        if (_children[i]->_radioState)
        {
            // remove radio functionality
            if (_children[i]->_checkState)
                _children[i]->SetCheckState(false);
            _children[i]->_radioState = false;
        }
}
bool MenuItem::IsSeparator() const
{
    MENUITEMINFO info;
    HMENU hTopLevel;
    // find the top level menu
    if (!_parentItem)
        return false; // the item needed to have been added the either an item or a bar
    else
        hTopLevel = (_parentState ? _parentItem->_hSubMenu : _parentBar->GetMenuHandle());
    ZeroMemory(&info,sizeof(MENUITEMINFO));
    info.cbSize = sizeof(MENUITEMINFO);
    info.fMask = MIIM_FTYPE;
    GetMenuItemInfo(hTopLevel,*this,FALSE,&info);
    return (info.fType&MFT_SEPARATOR) == MFT_SEPARATOR;
}
void MenuItem::_invokeDelegate()
{
    _defaultProcessing(); // default process
    if (!_delegate)
        return; // no event delegate (or routine) set
    if (_eventState)
    {//inkove the delegate member function
        MenuBar* top_level; Window* parent_wnd;
        // find the top level menu bar and get the window object
        top_level = _findTopLevel();
        if (!top_level) return; // no menu bar parent at top of heirarchy
        parent_wnd = const_cast<Window*> (top_level->GetBoundWindowPtr());
        if (!parent_wnd)
            return; // just in case
        // invoke the delegate
        (parent_wnd->*_delegate)( _myID ); // the id can identify the menu item, in case the routine is being used for multiple items
    }
    else
    {// invoke non-member routine
        (*_routine)();
    }
}
void MenuItem::_defaultProcessing()
{
    // toggle radio menu items
    if (_radioState && !_checkState)
    {
        // turn this item's radio check on
        SetCheckState(true);
    }
}
void MenuItem::_findUpdate() const
{
    MenuBar* top_level;
    // find the top level menu bar and get the window object
    top_level = _findTopLevel();
    if (!top_level) return;
    //call for update
    top_level->Update();
}
void MenuItem::_chngChildPos(dword ChangePos,bool Insertion)
{
    // change position members for children
    for (dword i = 0;i<_children.size();i++)
    {
        if (_children[i]->_myPos>ChangePos)
        {
            if (Insertion)
                _children[i]->_myPos++;
            else
                _children[i]->_myPos--;
        }
    }
}
container<MenuItem*> MenuItem::_getChildrenSortedByIndex() const
{
    container<MenuItem*> elems = _children;
    for (dword i = 1;i<elems.size();i++)
    {
        MenuItem* elem = elems[i];
        dword ind = i;
        while (ind>0 && elem->_myPos<elems[ind-1]->_myPos)
        {
            elems[ind] = elems[ind-1];
            ind--;
        }
        if (ind!=i)
            elems[ind] = elem;
    }
    return elems;
}
MenuBar* MenuItem::_findTopLevel() const
{
    MenuBar* top_level = 0;
    if (_parentState)
    {
        MenuItem* p = _parentItem;
        bool barState = p->_parentState; // (the default bar state is false)
        while (barState)
        {
            p = p->_parentItem;
            barState = p->_parentState;
        }
        top_level = p->_parentBar;
    }
    else
        top_level = _parentBar;
    return top_level;
}
MenuItem::operator UINT_PTR() const
{
    return (UINT_PTR)_hSubMenu;
}

MenuBar::MenuBar()
{
    _hMenu = CreateMenu(); // create the menu on startup
    _wnd = 0;
    _showState = false;
    _handleOwned = true;
}
MenuBar::MenuBar(HMENU hMenu)
{
    int itemCount;
    _hMenu = hMenu;
    _handleOwned = false;
    _wnd = NULL; // not enough info to determine this; user needs to call Bind
    itemCount = GetMenuItemCount(_hMenu);
    for (int i = 0;i<itemCount;i++)
    {
        MENUITEMINFO info;
        ZeroMemory(&info,sizeof(MENUITEMINFO));
        info.cbSize = sizeof(MENUITEMINFO);
        info.fMask = MIIM_ID | MIIM_STRING | MIIM_STATE | MIIM_TYPE;
        // first call to get menu item HMENU and menu item string size
        GetMenuItemInfo(_hMenu,i,TRUE,&info);
        // create MenuItem object
        MenuItem* item = new MenuItem((HMENU) info.wID);
        item->_myPos = i;
        item->_allocState = true; // flag this item as being dynamically allocated and as not owned by our menu system
        item->_parentState = false; // this context is the parent and is a menu bar
        item->_parentBar = this; // assign ourself as the item's parent
        item->_checkState = (info.fState&MFS_CHECKED) == MFS_CHECKED;
        item->_enabledState = (info.fState&MFS_DISABLED) != MFS_DISABLED;
        item->_radioState = (info.fType&MFT_RADIOCHECK)==MFT_RADIOCHECK && item->_checkState;
        // create a buffer for the menu item's text
        item->_text = str(++info.cch); // add 1 to account for the null terminating character
        info.dwTypeData = &item->_text[0];
        // second call to get menu item string
        info.fMask = MIIM_STRING;
        GetMenuItemInfo(_hMenu,i,TRUE,&info);
        item->_text = "ASDF";
        // add the menu item
        _items.push_back(item);
    }
}
MenuBar::MenuBar(const MenuBar& bar)
{// this is technically unimplemented, since I want to deny copying menu bars
    _handleOwned = false;
    _hMenu = bar._hMenu;
    _wnd = bar._wnd;
    _showState = bar._showState;
    // don't copy items
}
MenuBar::~MenuBar()
{
    // remove all items
    for (dword i = 0;i<_items.size();i++)
    {
        if (_items[i]->_allocState)
        {//delete any free alloc. items
            delete _items[i];
            _items.remove_at(i--);
        }
        else
        {// inform stack objects that they're being removed
            RemoveItem(i--);
        }
    }
    /*  If a MenuBar object passes out of scope then 
            we need to destroy the menu memory unless it's bound to a window.
        A window will delete its menu for us, along with all of its children.
        A menu might be bound to a window (as far as this class's abstraction is concerned),
            yet it might be hidden, which means technically it's not bound to a window. In
            this case, we need to destroy the menu.
    */
    if ((!_wnd || !_showState) && _handleOwned)
        DestroyMenu(_hMenu);

    _items.delete_elements();
}
void MenuBar::LoadFromHandle(HMENU hMenu)
{
    // destroy/forget any old menu elements
    for (dword i = 0;i<_items.size();i++)
        if (_items[i]->_allocState)
            delete _items[i];
        else
            RemoveItem(i);
    if ((!_wnd || !_showState && _handleOwned))
        DestroyMenu(_hMenu);
    // load from new handle
    int itemCount;
    _hMenu = hMenu;
    _handleOwned = false;
    _wnd = NULL; // not enough info to determine this; user needs to call Bind
    itemCount = GetMenuItemCount(_hMenu);
    for (int i = 0;i<itemCount;i++)
    {
        MENUITEMINFO info;
        ZeroMemory(&info,sizeof(MENUITEMINFO));
        info.cbSize = sizeof(MENUITEMINFO);
        info.fMask = MIIM_ID | MIIM_STRING | MIIM_STATE | MIIM_TYPE;
        // first call to get menu item HMENU and menu item string size
        GetMenuItemInfo(_hMenu,i,TRUE,&info);
        // create MenuItem object
        MenuItem* item = new MenuItem((HMENU) info.wID);
        item->_myPos = i;
        item->_allocState = true; // flag this item as being dynamically allocated and as not owned by our menu system
        item->_parentState = false; // this context is the parent and is a menu bar
        item->_parentBar = this; // assign ourself as the item's parent
        item->_checkState = (info.fState&MFS_CHECKED) == MFS_CHECKED;
        item->_enabledState = (info.fState&MFS_DISABLED) != MFS_DISABLED;
        item->_radioState = (info.fType&MFT_RADIOCHECK)==MFT_RADIOCHECK && item->_checkState;
        // create a buffer for the menu item's text
        item->_text = str(++info.cch); // add 1 to account for the null terminating character
        info.dwTypeData = &item->_text[0]; // set buffer for item string
        // second call to get menu item string only
        info.fMask = MIIM_STRING;
        GetMenuItemInfo(_hMenu,i,TRUE,&info);
        // add the menu item
        _items.push_back(item);
    }
}
bool MenuBar::Bind(const Window& Wnd)
{
    if (_wnd)
        return false;
    _wnd = &Wnd; // set window obj.
    // add the menu to the window
    if (!SetMenu(_wnd->WinHandle(),_hMenu))
        return false;
    _showState = true;
    return true;
}
bool MenuBar::SetWindow(const Window& Wnd)
{
    if (_wnd)
        return false;
    _wnd = &Wnd;
    _showState = false;
    return true;
}
bool MenuBar::ToggleHidden(bool ForceShown) const
{
    if (!_wnd)
        return false;
    // if force shown, force a change to the shown state
    if (ForceShown)
        _showState = false;
    // change state
    if (_showState)
    {// "hide" the menu bar by removing it
        if (!SetMenu(_wnd->WinHandle(),NULL))
            return _showState;
        _showState = false;
    }
    else
    {
        if (!SetMenu(_wnd->WinHandle(),_hMenu))
            return _showState;
        _showState = true;
    }
    return _showState;
}
bool MenuBar::Remove()
{
    if (!_wnd)
        return false;
    SetMenu(_wnd->WinHandle(),NULL); // remove menu bar
    // forget the window
    _wnd = 0;
    _showState = false;
    return true;
}
bool MenuBar::AddItem(MenuItem& Item)
{
    //make sure the same item object hasn't already been added
    dword dummy;
    if (_items.binary_search(&Item,dummy))
        return false;
    if (!AppendMenu(_hMenu,MF_STRING,Item,Item._text.c_str()))
        return false;
    Item._parentBar = this; // set parent ptr
    Item._parentState = false;
    // set item pos
    Item._myPos = _items.size();
    _items.push_back(&Item);
    //ensure that the container is always sorted for later use
    _items.insertion_sort();
    return true;
}
bool MenuBar::AddSeparator()
{
    MenuItem* newItem = new MenuItem;
    newItem->_allocState = true; // we need to delete this later
    if (!AppendMenu(_hMenu,MF_SEPARATOR,*newItem,NULL))
    {
        delete newItem;
        return false;
    }
    newItem->_parentBar = this; // set parent ptr
    newItem->_parentState = false;
    // set item pos.
    newItem->_myPos = _items.size();
    _items.push_back(newItem);
    // ensure that the container is always sorted
    _items.insertion_sort();
    return true;
}
bool MenuBar::InsertItem(MenuItem& Item,dword PositionIndex)
{
    // make sure the item doesn't already exist in the item list
    dword dummy;
    if (_items.binary_search(&Item,dummy))
        return false;
    if (!InsertMenu(_hMenu,PositionIndex,MF_BYPOSITION|MF_STRING,Item,Item._text.c_str()))
        return false;
    Item._parentBar = this;
    Item._parentState = false;
    Item._myPos = PositionIndex;
    _chngChildPos(PositionIndex,true);
    _items.push_back(&Item);
    //ensure that the container is always sorted for later use
    _items.insertion_sort();
    return true;
}
bool MenuBar::InsertSeparator(dword PositionIndex)
{
    MenuItem* newItem = new MenuItem;
    newItem->_allocState = true; // we need to delete this later
    if (!InsertMenu(_hMenu,PositionIndex,MF_BYPOSITION|MF_SEPARATOR,*newItem,NULL))
    {
        delete newItem;
        return false;
    }
    newItem->_parentBar = this;
    newItem->_parentState = false;
    newItem->_myPos = _items.size();
    _chngChildPos(PositionIndex,true);
    _items.push_back(newItem);
    // ensure that the container is always sorted for later use
    _items.insertion_sort();
    return true;
}
void MenuBar::RemoveAllItems()
{
    dword lastSize = _items.size();
    for (dword i = 0;i<_items.size();i++)
    {
        RemoveItem(i);
        i -= lastSize-_items.size();
        lastSize = _items.size();
    }
}
bool MenuBar::RemoveItem(MenuItem& item)
{
    HMENU hRemoveMe = item._hSubMenu;
    // find index of occurance
    dword i;
    if (!_items.binary_search(&item,i))
        return false;
    if (_items[i]->_allocState)
        return false; // can't remove because we don't own that item
    //inform the item that it no longer has a parent
    _items[i]->_parentBar = 0;
    //inform other children of their changing positon
    _chngChildPos(_items[i]->_myPos,false);
    // remove from our children
    _items.remove_at(i);
    //tell Windows to remove the menu item from the parent
    if (!RemoveMenu(_hMenu,(UINT)hRemoveMe,NULL)) // looks at the identifier, not the position
        return false;
    //update window menu bar
    Update();
    return true;
}
bool MenuBar::RemoveItem(dword Index)
{
    if (Index>=_items.size())
        return false;
    HMENU hRemoveMe = _items[Index]->_hSubMenu;
    if (_items[Index]->_allocState)
        return false;
    //inform the item that it no longer has a parent
    _items[Index]->_parentBar = 0;
    //inform other children that their pos member is changing
    _chngChildPos(Index,false);
    // remove from our children
    _items.remove_at(Index);
    //tell Windows to remove the menu item from the parent
    if (!RemoveMenu(_hMenu,(UINT)hRemoveMe,NULL)) // looks at the identifier, not the position
        return false;
    //update window menu bar
    Update();
    return true;
}
void MenuBar::Update() const
{
    if (_showState && _wnd)
        DrawMenuBar(_wnd->WinHandle());
}
void MenuBar::_chngChildPos(dword ChangePos,bool Insertion)
{
    // change position members for children
    for (dword i = 0;i<_items.size();i++)
    {
        if (_items[i]->_myPos>ChangePos)
        {
            if (Insertion)
                _items[i]->_myPos++;
            else
                _items[i]->_myPos--;
        }
    }
}