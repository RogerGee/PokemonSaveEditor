//RWin32LibButton.h
#ifndef RWIN32LIBBUTTON_H
#define RWIN32LIBBUTTON_H

namespace rtypes
{
    namespace rwin32
    {
        class Button : public Window
        {// represents a push button control and functionality common to Windows button types
        public:
            enum ButtonState
            {
                NullButtonState = -1,
                Checked = BST_CHECKED,
                Unchecked = BST_UNCHECKED,
                Indeterminate = BST_INDETERMINATE
            };
            Button();

            bool CreateButton(const Window* ParentWin);

            // "get" methods
            //  (for push buttons, this returns push state)
            ButtonState GetCheckState() const;

            // "set" methods
            void SetCheckState(ButtonState State) { SendMessage(_hWnd,BM_SETCHECK,(WPARAM)State,NULL); } // unimplemented for push buttons

            //events
            //  main event
            void SetClickEvent(EventDelegate Delegate) { _delegate = Delegate; }
            //
            EventFunct EvntClick;
            EventFunct EvntDoubleClick;
            virtual void OnClick(const ButtonEventData&);
            virtual void OnDoubleClick(const ButtonEventData&);

            //provide a method to simulate a button click
            //  calls this button's main event delegate, along with its OnClick and its EvntClick routines
            void Click(ButtonEventData&);

            //misc.
            virtual bool IsControl() const { return true; }
            virtual WindowType MyType() const { return PushButtonControl; }
        protected:
            virtual const char* _getWinClass() const;
            virtual bool _isSystemClass() const { return true; }
            virtual dword _getWinStyle() const;
            virtual void _raiseSubEvents(EventData&);
            virtual Point _getDefaultWinLocation(int Width = 0,int Height = 0) const;
        private:
            EventDelegate _delegate;
        };

        class RadioButton : public Button
        {
        public:
            RadioButton();

            /* 
               Users can create a radio button with this method or with CreateButton
               however, it's recommended that they call CreateRadioButton for added radio
               button functionality.
            */
            bool CreateRadioButton(const Window* ParentWin,bool IsLeader = false);

            virtual WindowType MyType() const { return RadioButtonControl; }
        protected:
            virtual void OnCreate(const EventData&);
            virtual void OnDestroy(const EventData&);
            virtual dword _getWinStyle() const;
        private:
            static bool _hasLeader;
            bool _isLeader;
        };

        class CheckBoxButton : public Button
        {
        public:

            virtual WindowType MyType() const { return CheckBoxControl; }
        protected:
            virtual dword _getWinStyle() const;
        };

        class CheckBox3StateButton : public CheckBoxButton
        {
        protected:
            virtual dword _getWinStyle() const;
        };

        class GroupBox : public Button
        {
        public:

            // CalcElementLocation will return a location on the parent window for
            // an element given a position relative to the group box control
            Point CalcElementLocation(const Point& PositionRelative) const;

            // Sets "ControlWindow" within the group box at the specified
            // relative position (within the client area of the group box)
            void operator ()(Window& ControlWindow,const Point& PositionRelative) const { ControlWindow.SetLocation( CalcElementLocation(PositionRelative) ); }
        protected:
            virtual dword _getWinStyle() const;
        };

        // if the common control library isn't version 6 or higher, a command link will behave
        // and look like a normal Button
        class CommandLink : public Button
        {
        public:
            bool SetNoteText(const wstr&);
        protected:
            virtual dword _getWinStyle() const;
        };
    }
}

#endif