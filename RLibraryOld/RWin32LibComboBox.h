//RWin32LibComboBox.h
#ifndef RWIN32LIBCOMBOBOX_H
#define RWIN32LIBCOMBOBOX_H

namespace rtypes
{
    namespace rwin32
    {
        class ComboBox : public Window
        {
        public:
            ComboBox();

            bool CreateComboBox(const Window* ParentWin) { return Create(-1,-1,defaults::DEFAULT_TBOX_WIDTH,defaults::DEFAULT_TBOX_HEIGHT,ParentWin); }

            // operations
            bool AddItemFromBox(bool CheckIfExists = false); // adds the current string in the combo box's edit control to the listbox; returns if the item was added
            void AddItem(const str& Item,bool SetExtent = false);
            void AddItemRange(const char** ItemArray,dword Size);
            void AddItemRange(const str ItemArray[],dword Size);
            void AddItemRange(const container<str>& ItemArray);
            bool InsertItemFromBox(bool CheckIfExists = false);
            void InsertItem(const str& Item,dword InsertionIndex,bool SetExtent = false);
            void InsertItemRange(const char** ItemArray,dword Size,dword InsertionIndex);
            void InsertItemRange(const str ItemArray[],dword Size,dword InsertionIndex);
            void InsertItemRange(const container<str>& ItemArray,dword InsertionIndex);
            bool RemoveItem(dword IndexAt) { return SendMessage(_hWnd,CB_DELETESTRING,IndexAt,NULL)!=CB_ERR; }
            bool RemoveItemRange(dword IndexStart,dword IndexEnd); // return value indicates if 1 or more remove ops failed
            bool SearchItem(dword &IndexFound_out,const str& ItemText,int StartIndex = -1);
            bool SearchItemExact(dword &IndexFound_out,const str& ItemText,int StartIndex = -1);
            bool SearchSelect(const str& ItemText,int StartIndex = -1) { return SendMessage(_hWnd,CB_SELECTSTRING,StartIndex,(LPARAM) ItemText.c_str())!=CB_ERR; } // returns if an element was found; selects the element if found
            void ClearSelection() { SetSelectedIndex(-1); }
            void ResetListBox() { SendMessage(_hWnd,CB_RESETCONTENT,NULL,NULL); }
            void LimitText(dword CharCount) { SendMessage(_hWnd,CB_LIMITTEXT,CharCount,NULL); }

            //events
            //      delegate events
            void SetSelectionChangedEvent(EventDelegate Delegate) { _delSelChng = Delegate; }
            void SetSelectionAffirmedEvent(EventDelegate Delegate) { _delSelAffirm = Delegate; }
            void SetSelectionCanceledEvent(EventDelegate Delegate) { _delSelCancel = Delegate; }
            //
            EventFunct EvntSelectionChanged;
            EventFunct EvntSelectionAffirmed;
            EventFunct EvntSelectionCanceled;
            EventFunct EvntTextChanged;
            EventFunct EvntTextUpdated;
            virtual void OnSelectionChanged(const ComboBoxEventData&);
            virtual void OnSelectionAffirmed(const ComboBoxEventData&);
            virtual void OnSelectionCanceled(const ComboBoxEventData&);
            virtual void OnTextChanged(const ComboBoxEventData&);
            virtual void OnTextUpdated(const ComboBoxEventData&);

            // "get" methods
            str GetItem(dword Index) const;
            dword GetItemLength(dword Index) const { return SendMessage(_hWnd,CB_GETLBTEXTLEN,Index,NULL); }
            dword GetNumberOfItems() const { return SendMessage(_hWnd,CB_GETCOUNT,NULL,NULL); }
            dword GetSelectedIndex() const { return SendMessage(_hWnd,CB_GETCURSEL,NULL,NULL); }
            wstr GetCueBanner() const;
            //      GetSelectionInfo doesn't work on drop down lists
            dword GetSelectionInfo(dword& SelStart,dword& SelEnd) const; // gets selection information about the edit control - returns selection length
            int GetItemHeight() const { return SendMessage(_hWnd,CB_GETITEMHEIGHT,0,NULL); }
            int GetMinViewableItems() const { return SendMessage(_hWnd,CB_GETMINVISIBLE,NULL,NULL); }
            dword GetTopVisibleIndex() const { return SendMessage(_hWnd,CB_GETTOPINDEX,NULL,NULL); } // gets the first viewable item index

            // "set" methods
            // (SetCueBanner limits banner text to defaults::BANNER_BUFFER_SIZE characters)
            bool SetCueBanner(wstr BannerText);
            bool SetSelectionInfo(dword SelStart,dword SelEnd); // note: doesn't work on drop down lists
            bool SetSelectedIndex(dword Index) { return SendMessage(_hWnd,CB_SETCURSEL,Index,NULL)!=CB_ERR; }
            bool SetTopVisibleIndex(dword Index) { return SendMessage(_hWnd,CB_SETTOPINDEX,Index,NULL)!=CB_ERR; }
            bool SetItemHeight(int Height) { return SendMessage(_hWnd,CB_SETITEMHEIGHT,0,Height)!=CB_ERR; }
            bool SetMinViewableItems(int ItemCount) const { return SendMessage(_hWnd,CB_SETMINVISIBLE,ItemCount,NULL)==TRUE; }

            // misc.
            bool IsItemSelected() const { return GetSelectedIndex()!=CB_ERR; }

            // other
            virtual bool IsControl() const { return true; }
            virtual WindowType MyType() const { return ComboBoxControl; }
        protected:
            virtual const char* _getWinClass() const;
            virtual bool _isSystemClass() const { return true; }
            virtual dword _getWinStyle() const;
            virtual void _raiseSubEvents(EventData&);
            void _addItem(const char* c) { SendMessage(_hWnd,CB_ADDSTRING,NULL,(LPARAM) c); }
            void _insertItem(const char* c,dword at) { SendMessage(_hWnd,CB_INSERTSTRING,at,(LPARAM) c); }
        private:
            EventDelegate   _delSelChng,
                            _delSelAffirm,
                            _delSelCancel;
        };

        class DropDownComboBox : public ComboBox
        {
        public:
            bool IsDroppedDown() const { return SendMessage(_hWnd,CB_GETDROPPEDSTATE,NULL,NULL)==TRUE; }
            void DropDown(bool Show) const { SendMessage(_hWnd,CB_SHOWDROPDOWN,Show,NULL); }
        protected:
            virtual dword _getWinStyle() const;
        };

        class DropDownListComboBox : public DropDownComboBox
        {// a drop down, but with no edit functionality
        protected:
            virtual dword _getWinStyle() const;
        };


    }
}

#endif