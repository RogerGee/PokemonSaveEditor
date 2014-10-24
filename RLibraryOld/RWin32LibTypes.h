//RWin32LibTypes.h
#ifndef RWIN32LIBTYPES_H
#define RWIN32LIBTYPES_H

namespace rtypes
{
    namespace rwin32
    {
        struct Point
        {
            Point() : x(0), y(0) {}
            Point(int X,int Y) : x(X), y(Y) {}
            Point& operator +=(const Point&);
            Point& operator -=(const Point&);
            bool operator ==(const Point& p) const { return x==p.x && y==p.y; }
            bool operator !=(const Point& p) const { return ! this->operator ==(p); }
            int x, y;
        };
        struct Size
        {
            Size() : width(0), height(0) {}
            Size(int Width,int Height) : width(Width), height(Height) {}
            Size& operator +=(const Size&);
            Size& operator -=(const Size&);
            bool operator ==(const Size& s) const { return width==s.width && height==s.height; }
            bool operator !=(const Size& s) const { return ! this->operator ==(s); }
            int width, height;
        };
        struct Rect : Point, Size
        {
            Rect() {}
            Rect(int X,int Y,int Width,int Height) : Point(X,Y), Size(Width,Height) {}
            bool overlaps(const Point&) const;
            bool overlaps(const Rect&) const;
            Rect& operator +=(const Point&);
            Rect& operator -=(const Point&);
            Rect& operator +=(const Size&);
            Rect& operator -=(const Size&);
            bool operator ==(const Rect& r) const { return Point::operator==(r) && Size::operator==(r); }
            bool operator !=(const Rect& r) const { return Point::operator!=(r) && Size::operator!=(r); }
        };
        // operator overloads for the above types
        const Point operator +(const Point&,const Point&);
        const Point operator -(const Point&,const Point&);
        const Size operator +(const Size&,const Size&);
        const Size operator -(const Size&,const Size&);
        
        struct SimpleResource
        {
            SimpleResource() : resourceID(0), useAppModule(true) {}
            SimpleResource(int ID,bool FromAppModule) : resourceID(ID), useAppModule(FromAppModule) {}
            int resourceID;
            bool useAppModule;
        };

        typedef void (*RoutinePtr)(void);

        struct EventData;
        typedef void (*EventFunct)(const EventData&);
        enum EventType
        {
            SubClassed, // event for a sub-class window object (control)
            /*
                Note on SubClassed event type: it is used by multiple EventData derivations,
                    so it is no sure way to assume the derived type of an EventData object.
                    Implement your system such that the type can be assumed by the context.
            */
            Normal, // a generic event that doesn't pass arguments
            WinChng, // the window size or location has changed
            Mouse, // the mouse has been clicked or moved in some way
            Paint, // the window needs to be redrawn
            Key, // a key was pressed/released
        };
        struct EventData
        {
            EventData() : tag(0), _functCall(0) {}
            EventData(int _Data,EventFunct _FunctCall) : tag(_Data), _functCall(_FunctCall) {}
            mutable int tag; // the user can use this in any context
            virtual EventType MyType() const { return Normal; } // helps to id the EventData
            void SetFunctCall(EventFunct ef) { _functCall = ef; }
            void RaiseEvent()
            { (*_functCall)(*this); }
        protected:
            EventFunct _functCall;
        };
        struct EventUserData : EventData
        {
            EventUserData() {}
            EventUserData(int _Data,EventFunct _FunctCall) : EventData(_Data,_FunctCall) {}
            virtual EventType MyType() const { return SubClassed; }
        };
        struct MouseEventData : EventData
        {
            enum MouseButton
            {
                NoButton,
                Left,
                Right,
                Middle
            };
            enum MouseEventType
            {
                ClickUp,
                ClickDown,
                Move,
            };
            MouseEventData() : _t(ClickUp), _b(NoButton) {}
            MouseEventData(MouseEventType Type,
                MouseButton BtnState) : _t(Type), _b(BtnState) {}
            void SetButtonState(MouseButton mb) { _b = mb; }
            void SetMouseEventType(MouseEventType t) { _t = t; }
            MouseButton MyButton() const { return _b; }
            MouseEventType MyClickType() const { return _t; }
            virtual EventType MyType() const { return Mouse; }
            Point clickLocation;
        private:
            MouseEventType _t;
            MouseButton _b;
        };
        struct PaintEventData : EventData
        {
            enum PaintEventType
            {
                PaintUpdate, // generic draw event
                // windows implement this differently and by different notifications
                // so provide seperate event types for them
                BackgroundDraw, // the background of a window needs to be drawn
                ForegroundDraw, // the foreground of a window needs to be drawn
            };
            PaintEventData() : _t(PaintUpdate) {}

            void SetPaintType(PaintEventType t) { _t = t; }
            PaintEventType MyPaintType() const { return _t; }
            virtual EventType MyType() const { return Paint; }
        private:
            PaintEventType _t;
        };
        struct WinChngEventData : EventData
        {
            // tag object should store the size type (SIZE_MAXIMIZED,etc.) on a resize
            enum WinChngEventType
            {
                Moved,
                Resized
            };
            WinChngEventData() : _t(Moved) {}
            WinChngEventData(Point p,Size s)
                : winLocation(p),winSize(s) {}
            Point winLocation;
            Size winSize; // client area size
            void SetWinChngEventType(WinChngEventType t) { _t = t; }
            virtual EventType MyType() const { return WinChng; }
            WinChngEventType MyEventType() const { return _t; }
            bool WasMaximized() const { return tag==SIZE_MAXIMIZED; }
            bool WasMinimized() const { return tag==SIZE_MINIMIZED; }
        private:
            WinChngEventType _t;
        };
        struct KeyEventData : EventData
        {
            enum KeyEventType
            {
                KeyUp,
                KeyDown
            };
            KeyEventData() : virtualKey(0x00),_t(KeyDown) {}
            int virtualKey;
            bool isExtended;
            short repeatCount; // number of repeats on the event before a new one is fired
            bool wasDown; // if true, the key was down before the event happened (for KeyUp, this should always be true)
            virtual EventType MyType() const { return Key; }
            void SetKeyEventType(KeyEventType t) { _t = t; }
            KeyEventType MyEventType() const { return _t; }
        private:
            KeyEventType _t;
        };
        //control event types
        struct ButtonEventData : EventData
        {
            enum ButtonEventType
            {
                Clicked,
                DblClicked
            };
            ButtonEventData() {}
            ButtonEventData(ButtonEventType Type) : _t(Type) {}
            void SetButtonEventType(ButtonEventType t) { _t = t; }
            ButtonEventType MyEventType() const { return _t; }
            virtual EventType MyType() const { return SubClassed; }
        private:
            ButtonEventType _t;
        };
        struct TextBoxEventData : EventData
        {
            enum TextBoxEventType
            {
                TextChanged,
                TextUpdated,
                HScrolled,
                VScrolled,
                SelectionChanged,
                FileDropped // 'tag' is a pointer to a ENDROPFILES structure
            };
            TextBoxEventData() {}
            TextBoxEventData(TextBoxEventType Type) : _t(Type) {}
            void SetTextBoxEventType(TextBoxEventType t) { _t = t; }
            TextBoxEventType MyEventType() const { return _t; }
            virtual EventType MyType() const { return SubClassed; }
        private:
            TextBoxEventType _t;
        };
        struct ListBoxEventData : EventData
        {
            enum ListBoxEventType
            {
                SelChanged,
                SelCanceled
            };
            ListBoxEventData() : _t(SelChanged) {}
            ListBoxEventData(ListBoxEventType Type) : _t(Type) {}
            void SetListBoxEventType(ListBoxEventType t) { _t = t; }
            ListBoxEventType MyEventType() const { return _t; }
            virtual EventType MyType() const { return SubClassed; }
        private:
            ListBoxEventType _t;
        };
        struct ComboBoxEventData : EventData
        {
            enum ComboBoxEventType
            {
                SelChanged,
                SelAffirmed,
                SelCanceled,
                TextChanged,
                TextUpdated
            };
            ComboBoxEventData() : _t(SelChanged) {}
            ComboBoxEventData(ComboBoxEventType Type) : _t(Type) {}
            void SetComboBoxEventType(ComboBoxEventType t) { _t = t; }
            ComboBoxEventType MyEventType() const { return _t; }
            virtual EventType MyType() const { return SubClassed; }
        private:
            ComboBoxEventType _t;
        };

        struct SystemTime
        {
            enum Month
            {
                January = 1,
                February,
                March,
                April,
                May,
                June,
                July,
                August,
                September,
                October,
                November,
                December
            };
            enum DayOfWeek
            {
                Sunday,
                Monday,
                Tuesday,
                Wednesday,
                Thursday,
                Friday,
                Saturday
            };
            SystemTime() : year(0),day(0),hour(0),minute(0),second(0),milliseconds(0) {}
            SystemTime(const SYSTEMTIME&);
            SystemTime(PSYSTEMTIME);
            SystemTime& operator =(PSYSTEMTIME);
            SystemTime& operator =(const SYSTEMTIME&);
            word year,day,hour,minute,second,milliseconds;
            Month month;
            DayOfWeek dayOfWeek;
            str AsString() const;
            static str MonthToString(Month);
            static str DayOfWeekToString(DayOfWeek);
            static SystemTime GetCurrentSystemTime();
            static SystemTime GetCurrentLocalTime();
        };
    }
}

#endif