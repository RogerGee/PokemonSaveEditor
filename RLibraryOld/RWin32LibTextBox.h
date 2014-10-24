//RWin32LibTextBox.h
#ifndef RWIN32LIBTEXTBOX_H
#define RWIN32LIBTEXTBOX_H

namespace rtypes
{
    namespace rwin32
    {
        enum TextBoxAttribute
        {
            NumericInputOnly,
            LowerCaseOnly, // don't use with rich edits
            UpperCaseOnly, // don't use with rich edits
            Password,
            ReadOnly,
            SendReturnChar // I want to say this only effects edits who are children of dialog boxes
        };

        enum ScrollType
        {
            ScrollLineDown,
            ScrollLineUp,
            ScrollPageDown,
            ScrollPageUp
        };

        struct ParserElement
        {
            ParserElement() : indexBegin(-1),indexEnd(-1) {}
            str element;
            int indexBegin, indexEnd; // inclusive values
            bool Spans(int Index) const { return Index>=indexBegin && Index<=indexEnd; }
        };

        class textBoxBase : public Window
        {
        public:
            bool CreateTextBox(const Window* ParentWin);

            //events
            //  main event
            void SetTextChangedEvent(EventDelegate Delegate) { _delegateTextChng = Delegate; }
            //
            EventFunct EvntTextChanged;
            EventFunct EvntTextUpdated;
            EventFunct EvntTextHScroll;
            EventFunct EvntTextVScroll;
            virtual void OnTextChanged(const TextBoxEventData&);
            virtual void OnTextUpdated(const TextBoxEventData&);
            virtual void OnTextHScrolled(const TextBoxEventData&);
            virtual void OnTextVScrolled(const TextBoxEventData&);

            // operator wrappers
            textBoxBase& operator +=(const str&);
            textBoxBase& operator +=(const textBoxBase&);

            // text operations
            void AppendText(const str& Text);
            void InsertText(const str& Text,int At = -1); // if At==-1 then insert at selection start
            void Cut() { SendMessage(_hWnd,WM_CUT,NULL,NULL); }
            void Copy() const { SendMessage(_hWnd,WM_COPY,NULL,NULL); }
            void Paste() { SendMessage(_hWnd,WM_PASTE,NULL,NULL); }
            str ParseElement(const str& Delimiters = " \t\r\n",int SelStart = -1/* means cur sel */) const; // get an element around where the user is typing, unless otherwise specified
            ParserElement ParseElementEx(const str& Delimiters = " \t\r\n",int SelStart = -1/* means cur sel */) const; // get an element around where the user is typing, unless otherwise specified
            str ParseElements(container<str>& outElements,const str& Delimiters = " \t\r\n") const; // parse all elements in text, return element around where the user is typing
            ParserElement ParseElementsEx(container<ParserElement>& outElements,const str& Delimiters = " \t\r\n") const; // parser all elements in text, return element around where the user is typing
            //  these parsers only work for multiline edits,_bufferLines==true
            //  they work for a given line and return false if a bad line was specified or if no elements were found
            bool ParseLineElements(int LineIndex,container<str>& outElements,const str& Delimiters = " \t\r\n") const;
            bool ParseLineElementsEx(int LineIndex,container<ParserElement>& outElements,const str& Delimiters = " \t\r\n") const;
            void Delete(); // delete a single character
            void DeleteSelectedText() { SendMessage(_hWnd,WM_CLEAR,NULL,NULL); }
            bool Undo() const { return SendMessage(_hWnd,EM_UNDO,NULL,NULL)!=FALSE; }

            // "set" functions
            //  overload SetAttribute for text box attribs (bring old SetAttribute into scope to prevent shadowing)
            using Window::SetAttribute; bool SetAttribute(TextBoxAttribute Attrib,bool Remove = false);
            void SetPasswordChar(char pChar) { SendMessage(_hWnd,EM_SETPASSWORDCHAR,(WPARAM) pChar,NULL); } // this turns password chars on for the box
            /*
                Selection setting notes
                    -SetSelection will set and highlight the selection from Start to End, but not include
                End in the selection. If Start==End, no highlight will be made; the cursor will simply
                change position.
                    -SetSelectionStart is a wrapper that sets the cursor location only with no highlighting.
                    -SetHighlightSelection treats Start and End as inclusive indeces for the sake of highlighting.
                If End==Start-1, no highlight will be made (the selection size will be zero).
            */
            void SetSelection(int Start,int End) const { SendMessage(_hWnd,EM_SETSEL,(WPARAM) Start,(LPARAM) End); }
            void SetSelectionStart(int Index) const { SetSelection(Index,Index); }
            void SetHighlightSelection(int Start,int End) const { SetSelection(Start,End+1); }
            void SetSelectionText(const str& Text) { SendMessage(_hWnd,EM_REPLACESEL,TRUE,(LPARAM) &Text[0]); }
            virtual void SetTextLimit(dword Limit) { SendMessage(_hWnd,EM_SETLIMITTEXT,Limit,NULL); }

            // "get" functions
            bool GetIfAttributeSet(TextBoxAttribute Attrib) const;
            char GetPasswordChar() const { return (char) SendMessage(_hWnd,EM_GETPASSWORDCHAR,NULL,NULL); }
            virtual str GetSelectedText() const;
            // (these are unsigned because that's what the API implementation expects)
            dword GetSelectionInformation(dword &Start,dword &End) const; // combines all of the 3 operations below; returns length; 'End' is inclusive
            dword GetSelectionLength() const;
            dword GetSelectionStart() const;
            dword GetSelectionEnd() const;
            dword GetSelectionEndInclusive() const; // use this to get last char index in highlighted selection
            int GetNumberOfLines() const { return (int) SendMessage(_hWnd,EM_GETLINECOUNT,NULL,NULL); }
            const char* GetLineText(int LineIndex,int &outLineLength) const;
            const char* GetCurrentLineText(int &outLineLength) const { return GetLineText( GetLineIndex(GetCurrentLineCharacterIndex()),outLineLength ); }
            int GetCurrentLineCharacterIndex() const { return GetCharacterIndex(-1); } // EM_LINEINDEX implementation specific 
            int GetCharacterIndex(int LineIndex) const { return (int) SendMessage(_hWnd,EM_LINEINDEX,LineIndex,NULL); }
            int GetLineIndex(int CharacterIndex) const { return (int) SendMessage(_hWnd,EM_LINEFROMCHAR,CharacterIndex,NULL); }
            int GetFirstVisibleLineIndex() const;

            //misc.
            void SelectAll() const;
            void DeselectSelection() const;
            int ScrollText(ScrollType ByType) const; // returns number of lines scrolled
            void ScrollToCaret() const { SendMessage(_hWnd,EM_SCROLLCARET,NULL,NULL); }
            bool HasUndoQueue() const { return SendMessage(_hWnd,EM_CANUNDO,NULL,NULL)!=0; }
            virtual bool IsControl() const { return true; }
        protected:
            explicit textBoxBase(bool BufferLines);
            virtual bool _isSystemClass() const { return true; }
            virtual void _raiseSubEvents(EventData&);
            virtual void _raiseSpecificSubEvents(TextBoxEventData&) = 0; // implement any other notifications
        private:
            EventDelegate _delegateTextChng;
            bool _multilineState;
            bool _bufferLines; // if true, then _lineBuf is allocated for use
            mutable str _lineBuf;
        };

        class TextBox : public textBoxBase
        {// represents a single-line edit control that left-orients text
        public:
            TextBox();

            // "get" methods
            wstr GetBannerCue() const;

            // "set" methods
            // (SetBannerCue depends on version 6 of the common control library)
            // (SetBannerCue limits the size of BannerText to defaults::BANNER_BUFFER_SIZE)
            bool SetBannerCue(wstr BannerText,bool ShowWhenFocused = false);
            bool SetTextBoxSize(int Width); // height is set based on the font

            //misc.
            virtual WindowType MyType() const { return EditControl; }
        protected:
            virtual const char* _getWinClass() const;
            virtual dword _getWinStyle() const;
            virtual Point _getDefaultWinLocation(int Width = 0,int Height = 0) const;
            virtual void _raiseSpecificSubEvents(TextBoxEventData&);
        };

        class MultilineTextBox : public textBoxBase
        {// multiline edit control with no word wrap
        public:
            MultilineTextBox();

            virtual WindowType MyType() const { return EditControl; }
        protected:
            virtual const char* _getWinClass() const;
            virtual dword _getWinStyle() const;
            virtual Point _getDefaultWinLocation(int Width = 0,int Height = 0) const;
            virtual void _raiseSpecificSubEvents(TextBoxEventData&);
        };

        class RichTextBox; // forward declaration for RichCharData

        struct RichCharAttrib
        {
            RichCharAttrib();
            long height;
            long offset;
            Color textColor;
            str fontFamily; // the name string is all that is needed
            bool bold,italic,strikeout,underline; // text styles
            bool protectedCharacter;
            void SetChanges(bool ChangeHeight,
                bool ChangeOffset,
                bool ChangeColor,
                bool ChangeFontFamily,
                bool ChangeTextStyle,
                bool ChangeProtected);
        private:
            void _fillCharFormatStruct(CHARFORMAT* strct) const; // 'strct' is a pointer to a valid CHARFORMAT struct
            void _setFromCharFormatStruct(const CHARFORMAT* strct);
            // change states
            bool _changeHeight;
            bool _changeOffset;
            bool _changeColor;
            bool _changeFontFamily;
            bool _changeTextStyle;
            bool _changeProtected;
            friend class RichTextBox;
        };

        struct DroppedFileInfo
        {
            DroppedFileInfo();
            long characterPosition;
            Point mousePosition;
            sized_container<str> fileNames;
        private:
            void _setFromDropFilesStruct(const ENDROPFILES* strct);
            friend class RichTextBox;
        };

        // provide a special delegate type for file drops
        typedef void (Window::* FileDroppedEventDelegate)(const DroppedFileInfo&);
        
        class RichTextBox : public textBoxBase
        {// represents and wraps a multiline rich edit control with no word wrap
        public:
            RichTextBox();

            // events
            //      main events
            void SetSelectionChangedEvent(EventDelegate Delegate) { _delegateSelChng = Delegate; }
            void SetFileDroppedEvent(FileDroppedEventDelegate Delegate) { _delegateFileDrop = Delegate; }
            //
            EventFunct EvntSelectionChanged;
            EventFunct EvntFileDropped;
            virtual void OnSelectionChanged(const TextBoxEventData&);
            virtual void OnFileDropped(const DroppedFileInfo&);

            // operations
            void ShowSelectionHighlight() const { SendMessage(_hWnd,EM_HIDESELECTION,FALSE,NULL); }
            void HideSelectionHightlight() const { SendMessage(_hWnd,EM_HIDESELECTION,TRUE,NULL); }
            bool FormatAllText(const RichCharAttrib& Formating);
            //      if no text is selected, this changes the formatting at the insertion point
            bool FormatSelectedText(const RichCharAttrib& Formating);
            void PastePlainText() { SendMessage(_hWnd,EM_PASTESPECIAL,CF_TEXT,NULL); }

            // "get" functions
            RichCharAttrib GetTextFormatting() const;
            RichCharAttrib GetSelectedTextFormatting() const;
            virtual str GetSelectedText() const;

            // "set" functions
            virtual void SetTextLimit(dword Limit) { SendMessage(_hWnd,EM_EXLIMITTEXT,NULL,Limit); }
            
            // misc.
            virtual WindowType MyType() const { return RichEditControl; }
        protected:
            virtual const char* _getWinClass() const;
            virtual dword _getWinStyle() const;
            virtual Point _getDefaultWinLocation(int Width = 0,int Height = 0) const;
            virtual void _raiseSpecificSubEvents(TextBoxEventData&);
        private:
            EventDelegate _delegateSelChng;
            FileDroppedEventDelegate _delegateFileDrop;
            virtual void OnCreate(const EventData&);
        };
    }
}

#endif