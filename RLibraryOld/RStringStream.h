//RStringStream.h
#ifndef RSTRINGSTREAM_H
#define RSTRINGSTREAM_H
#include "RStream.h"

namespace rtypes
{
    // rstringstream expects strings with normal ('\n') end line encoding
    class rstringstream : public rstream<str>
    {
    public:
        rstringstream() {}
        rstringstream(const char* DeviceID) { open(DeviceID); }
        rstringstream(str& Device) { open(Device); }
        ~rstringstream() { if (!_isOwned()) flush_output(); /* Just in case (this is a close action) */ }
    protected:
        virtual bool _openID(const char*);
        virtual dword _deviceSize() const { return _device->size(); }
        virtual void _outDevice();
        virtual bool _inDevice() const;
        virtual void _clearDevice() { _device->clear(); }
    };

    // rbinstream by default implements big endian byte orders
    class rbinstringstream : public rbinstream<str>
    {
    public:
        rbinstringstream() : rbinstream<str>(little) {}
        rbinstringstream(const char* DeviceID) : rbinstream<str>(little) { open(DeviceID); }
        rbinstringstream(str& Device) : rbinstream<str>(little) { open(Device); }
        rbinstringstream(endianness E) : rbinstream<str>(E) {} // provide a means of specifying the endianess
        ~rbinstringstream() { if (!_isOwned()) flush_output(); /* Just in case (this is a close action) */ }
    protected:
        virtual bool _openID(const char*);
        virtual dword _deviceSize() const { return _device->size(); }
        virtual void _outDevice();
        virtual bool _inDevice() const;
        virtual void _clearDevice() { _device->clear(); }
    };
}

#endif