//RWin32Menu.h
#ifndef RWIN32LIBMENU_H
#define RWIN32LIBMENU_H

namespace rtypes
{
    namespace rwin32
    {
        class MenuBar;

        class MenuItem
        {
        public:
            MenuItem();
            ~MenuItem();

            // "set" functions
            void SetEvent(RoutinePtr Routine);
            void SetEvent(EventDelegate_id Delegate);
            void SetText(const str& Text);
            bool SetCheckState(bool On);
            bool ToggleCheckState(); // returns the check state that resulted from the operation

            // "get" functions
            dword GetItemIndex() const { return _myPos; }
            dword GetNumberOfSubItems() const { return _children.size(); }
            MenuItem* GetItem(dword At) const { return _children[At]; }
            str GetText() const { return _text; }
            bool GetCheckState() const { return _checkState; }
            bool IsEnabled() const { return _enabledState; }
            bool IsRadioItem() const { return _radioState; }
            HMENU GetMenuHandle() const { return _hSubMenu; }
            dword GetID() const { return _myID; }

            // operations
            bool AddItem(MenuItem&);
            bool AddSeparator();
            bool InsertItem(MenuItem& Item,dword PositionIndex);
            bool InsertSeparator(dword PositionIndex);
            void RemoveAllItems();
            bool RemoveItem(MenuItem&);
            bool RemoveItem(dword Index);
            bool EnableMenuItem();
            bool DisableMenuItem();
            bool GrayOutMenuItem();
            /*
                CreateRadioGroup requires that 'this' be a popup menu item (has children).
                    The radio state applies to a sequential group of its children.
                    PositionIndexFirst - index of first item in radio group
                    PositionIndexLast - index of last item in radio group
                    PositionSelected - index of item (within group) to be selected
                CreateRadioGroup requires that the items in the new group not be included in another group
            */
            bool CreateRadioGroup(dword PositionFirst,
                dword PositionLast,
                dword PositionSelected);
            bool RemoveRadioGroup(dword PositionFirst,dword PositionLast); // return indicates if the range was found
            bool RemoveRadioGroup(const MenuItem&); // return indicates if the menu item was a part of any radio group; if so, the group was removed
            void RemoveAllRadioGroups();

            // misc.
            bool IsDropDown() const { return _popupState; }
            bool IsSeparator() const;
            bool HasEventDelegate() const { return _delegate==0; }

            //statics
            static bool ProcessMenuCommand(HMENU); // call this to send a menu item clicked message to the correct menu object
        private:
            static dword _menuIDCount;
            HMENU _hSubMenu;
            dword _myPos, _myID;
            union // (I prefer this to casting unrelated pointer types)
            {// note: these are both 4 bytes long
                EventDelegate_id _delegate;
                RoutinePtr _routine;
            };
            bool _eventState; // if true, then delegate, else routine
            union // (I prefer this to casting unrelated pointer types)
            {// a menu item's parent is either another item or a menu bar
                MenuItem* _parentItem;
                MenuBar* _parentBar;
            };
            bool _parentState; // if true, item parent, else bar parent
            bool _popupState;
            bool _checkState,// check state can mean radio checked or normal checked
                _enabledState,
                _radioState; // _radioState should mean if the item is part of a radio group, though if a menu structure was copied this could be inaccurate
            bool _allocState; // if true, the parent dynamically allocated this item
            container<MenuItem*> _children;
            str _text;
            container<Point> _radioGroups; // Point represents a group range [x=start,y=end]

            // deny copying
            MenuItem(const MenuItem&);
            MenuItem& operator =(const MenuItem&) { return *this; };
            // deny this type of c-stor - I want users to have to go through MenuBar to do this - this ensures the object will be completely set up
            MenuItem(HMENU); // load the menu item and its children from the system
            MenuBar* _findTopLevel() const;
            void _invokeDelegate(); // attempt to invoke the delegate
            void _defaultProcessing();
            void _findUpdate() const;
            void _chngChildPos(dword ChangePos,bool Insertion); // an item was either inserted or deleted at RemovePos; changes children pos members accordingly
            container<MenuItem*> _getChildrenSortedByIndex() const;
            operator UINT_PTR() const;

            friend class MenuBar;
        };

        class MenuBar
        {
        public:
            MenuBar();
            MenuBar(HMENU);
            ~MenuBar();

            void LoadFromHandle(HMENU);

            // menu bar operations
            bool Bind(const Window& Wnd); // adds the menu to the specified window as its menu bar
            /*
            //  Similar to Bind, but doesn't add the window as a menu bar.
                Use this if you want the bar's items to process events and invoke delegates, 
                but not be the main menu bar on the window.
                You can only add a window if there isn't one currently added. Invoke Remove to 
                remove a bar from a window.
            */
            bool SetWindow(const Window& Wnd);
            const Window* GetBoundWindowPtr() const { return _wnd; }
            bool ToggleHidden(bool ForceShown = false) const; // returns the current state of the menu bar after the operation
            bool Remove(); // removes menu from current window

            // menu item operations
            bool AddItem(MenuItem&);
            bool AddSeparator();
            bool InsertItem(MenuItem& Item,dword PositionIndex);
            bool InsertSeparator(dword PositionIndex);
            dword GetItemCount() const { return _items.size(); }
            MenuItem* GetItem(dword At) const { return _items[At]; }
            void RemoveAllItems();
            bool RemoveItem(MenuItem&); // removes specified element if it exists
            bool RemoveItem(dword Index); // removes the element at the specified index

            //misc.
            void Update() const;
            HMENU GetMenuHandle() const { return _hMenu; }
        private:
            // deny copying to users - create a copy by invoking the MenuBar(HMENU) c-stor
            MenuBar(const MenuBar&);
            MenuBar& operator =(const MenuBar&) { return *this; }
            HMENU _hMenu; // handle to menu bar
            mutable bool _showState;
            bool _handleOwned;
            const Window* _wnd;
            container<MenuItem*> _items;
            void _chngChildPos(dword ChangePos,bool Insertion);
        };
    }
}

#endif