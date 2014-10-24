#include "RWin32LibMaster.h"
using namespace rtypes;
using namespace rtypes::rwin32;

StaticLabel::StaticLabel()
{
    EvntStaticClicked = 0;
    EvntStaticDblClicked = 0;
}
void StaticLabel::OnStaticClicked(const EventUserData&)
{
    // do nothing
}
void StaticLabel::OnStaticDblClicked(const EventUserData&)
{
    // do nothing
}
const char* StaticLabel::_getWinClass() const
{
    return WC_STATIC;
}
dword StaticLabel::_getWinStyle() const
{
    return WS_CHILD | WS_VISIBLE | SS_SIMPLE | SS_NOTIFY;
}
void StaticLabel::_raiseSubEvents(EventData& _data)
{
    EventUserData& data = static_cast<EventUserData&> (_data);
    switch (data.tag)
    {
    case STN_CLICKED:
        OnStaticClicked(data);
        if (EvntStaticClicked)
        {
            data.SetFunctCall(EvntStaticClicked);
            data.RaiseEvent();
        }
        break;
    case STN_DBLCLK:
        OnStaticDblClicked(data);
        if (EvntStaticDblClicked)
        {
            data.SetFunctCall(EvntStaticDblClicked);
            data.RaiseEvent();
        }
        break;
    }
}