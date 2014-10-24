//RWin32LibListBox.h
#ifndef RWIN32LIBLISTBOX_H
#define RWIN32LIBLISTBOX_H

namespace rtypes
{
    namespace rwin32
    {
        class ListBox : public Window
        {
        public:
            ListBox();
            ListBox(bool SortItems);

            bool CreateListBox(const Window* ParentWin);

            // events
            //      main event
            void SetSelChangedEvent(EventDelegate Delegate) { _delegate = Delegate; }
            //
            EventFunct EvntSelChanged;
            EventFunct EvntSelCanceled;
            virtual void OnSelChanged(const ListBoxEventData&);
            virtual void OnSelCanceled(const ListBoxEventData&);


            // "get" functions
            str GetItem(int Index) const;
            sized_container<str> GetItems() const;
            int GetItemLength(int Index) const { return (int) SendMessage(_hWnd,LB_GETTEXTLEN,Index,NULL); }
            int GetNumberOfItems() const { return (int) SendMessage(_hWnd,LB_GETCOUNT,NULL,NULL); }
            int GetSelectedIndex() const { return (int) SendMessage(_hWnd,LB_GETCURSEL,NULL,NULL); }
            bool GetSelectState(int Index) const { return SendMessage(_hWnd,LB_GETSEL,Index,NULL)!=LB_ERR; }
            //      gets index of first occurance (not an exact search); err case == -1
            int GetItemIndex(const str& Item,int StartSearchIndex = 0) const { return (int) SendMessage(_hWnd,LB_FINDSTRING,StartSearchIndex-1,(LPARAM)Item.c_str()); }
            //      gets index of first occurance (exact search); err case == -1
            int GetExactItemIndex(const str& Item,int StartSearchIndex = 0) const { return (int) SendMessage(_hWnd,LB_FINDSTRINGEXACT,StartSearchIndex-1,(LPARAM)Item.c_str()); }
            int GetItemHeight() const { return (int) SendMessage(_hWnd,LB_GETITEMHEIGHT,NULL/*no owner drawn l-box*/,NULL); }

            // "set" functions
            virtual bool SetSelectedIndex(int Index) { return SendMessage(_hWnd,LB_SETCURSEL,Index,NULL)!=LB_ERR; }
            virtual bool SetSelectedIndex(const str& Item,int StartSearchIndex = 0) { return SendMessage(_hWnd,LB_SELECTSTRING,StartSearchIndex-1,(LPARAM)Item.c_str())!=LB_ERR; }
            void SetScrollableWidth(int PixelWidth) { SendMessage(_hWnd,LB_SETHORIZONTALEXTENT,PixelWidth,NULL); }

            //misc./operations
            void FormatLBoxHeight(int NumberOfItems);
            bool AddItem(const str& Item) { return SendMessage(_hWnd,LB_ADDSTRING,NULL,(WPARAM)Item.c_str())!=LB_ERR; }
            bool InsertItem(const str& Item,int Index) { return SendMessage(_hWnd,LB_INSERTSTRING,Index,(LPARAM)Item.c_str())!=LB_ERR; }
            bool RemoveItem(int Index) const { return SendMessage(_hWnd,LB_DELETESTRING,Index,NULL)!=LB_ERR; }
            bool RemoveItem(const str& Item,int StartSearchIndex = 0) const { return RemoveItem(GetItemIndex(Item,StartSearchIndex)); }
            void RemoveAllItems() const { SendMessage(_hWnd,LB_RESETCONTENT,NULL,NULL); }
            virtual void RemoveSelection() { SetSelectedIndex(-1); }

            virtual bool IsControl() const { return true; }
            virtual WindowType MyType() const { return ListBoxControl; }
        protected:
            virtual const char* _getWinClass() const;
            virtual bool _isSystemClass() const { return true; }
            virtual dword _getWinStyle() const;
            virtual void _raiseSubEvents(EventData&);
            virtual Point _getDefaultWinLocation(int Width = 0,int Height = 0) const;
            bool _sortState;
        private:
            EventDelegate _delegate;
        };

        class MultiSelListBox : public ListBox
        {
        public:
            MultiSelListBox();
            MultiSelListBox(bool SortItems);

            // "get" functions
            int GetAnchorIndex() const { return (int) SendMessage(_hWnd,LB_GETANCHORINDEX,NULL,NULL); }
            int GetCarentIndex() const { return (int) SendMessage(_hWnd,LB_GETCARETINDEX,NULL,NULL); }
            sized_container<int> GetSelectedIndeces() const;
            int GetNumberOfSelItems() const { return (int) SendMessage(_hWnd,LB_GETSELCOUNT,NULL,NULL); }

            // "set" functions
            virtual bool SetSelectedIndex(int Index) { return SendMessage(_hWnd,LB_SETSEL,TRUE,Index)!=LB_ERR; }
            virtual bool SetSelectedIndex(const str& Item,int StartSearchIndex = 0);
            bool SetAnchorIndex(int NewIndex) const { return SendMessage(_hWnd,LB_SETANCHORINDEX,NewIndex,NULL)!=LB_ERR; }
            bool SetCaretIndex(int NewIndex) const { return SendMessage(_hWnd,LB_SETCARETINDEX,NewIndex,FALSE)!=LB_ERR; }

            // operations
            bool SelectRange(int StartIndex,int EndIndex,bool Deselect = false) const;
            virtual void RemoveSelection();
            bool RemoveSelectedIndex(int Index) { return SendMessage(_hWnd,LB_SETSEL,FALSE,Index)!=LB_ERR; }
        protected:
            virtual dword _getWinStyle() const;
        };
    }
}

#endif