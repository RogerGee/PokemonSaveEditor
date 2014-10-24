#include "RWin32LibMaster.h"
using namespace rtypes;
using namespace rtypes::rwin32;
using namespace rtypes::rwin32::graphics;

DeviceContext::DeviceContext()
{
    _device = NULL;

}
DeviceContext::DeviceContext(HDC Device)
{
    SetDevice(Device);

}
bool DeviceContext::DrawLine(Point p1,Point p2)
{
    if (!*this || !::MoveToEx(_device,p1.x,p1.y,NULL) || !::LineTo(_device,p2.x,p2.y))
        return false;
    return true;
}