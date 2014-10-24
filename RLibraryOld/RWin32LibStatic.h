//RWin32LibStatic.h
#ifndef RWIN32LIBSTATIC_H
#define RWIN32LIBSTATIC_H

namespace rtypes
{
    namespace rwin32
    {
        class StaticLabel : public Window
        {
        public:
            StaticLabel();

            // events
            EventFunct EvntStaticClicked;
            EventFunct EvntStaticDblClicked;
            virtual void OnStaticClicked(const EventUserData&);
            virtual void OnStaticDblClicked(const EventUserData&);

            // misc.
            virtual bool IsControl() const { return true; }
            virtual WindowType MyType() const { return StaticControl; }
        protected:
            virtual const char* _getWinClass() const;
            virtual bool _isSystemClass() const { return true; }
            virtual dword _getWinStyle() const;
            virtual void _raiseSubEvents(EventData&);
        private:

        };
    }
}

#endif