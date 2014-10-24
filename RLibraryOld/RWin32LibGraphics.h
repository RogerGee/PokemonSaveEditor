//RWin32LibGraphics.h
#ifndef RWIN32LIBGRAPHICS_H
#define RWIN32LIBGRAPHICS_H

namespace rtypes
{
    namespace rwin32
    {
        namespace graphics
        {
            class DeviceContext
            {
            public:
                DeviceContext();
                DeviceContext(HDC Device);

                bool DrawLine(Point P1,Point P2);

                // "set" methods
                void SetDevice(HDC Device) { _device = Device; }

                // "get" methods
                HDC GetDevice() const { return _device; }

                // operators
                operator void* () const { return (void*) (_device!=NULL); }
            private:
                HDC _device;
            };
        }
    }
}

#endif