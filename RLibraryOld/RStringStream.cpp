#include "RStringStream.h"
using namespace rtypes;

// link with RStream.cpp

bool rstringstream::_openID(const char* DeviceID)
{
    // Create own string with DeviceID value.
    *_device = DeviceID;
    return true;
}
void rstringstream::_outDevice()
{
    // route all data from the output buffer to the string object
    // route data to string offset
    if (_odeviceIter>_device->size())
        _odeviceIter = _device->size();
    while (!_bufOut.is_empty())
    {
        if (_odeviceIter>=_device->size())
            _device->push_back( _bufOut.pop_back() );
        else
            _device->operator [](_odeviceIter) = _bufOut.pop_back();
        _odeviceIter++;
    }
}
bool rstringstream::_inDevice() const
{// return the state of the input device
    if (_ideviceIter<_deviceSize())
    {
        // route all available data from the device to the input buffer
        while (_ideviceIter<_device->size())
        {
            _bufIn.push_back( _device->operator [](_ideviceIter) );
            _ideviceIter++;
        }
        return true;
    }
    return false; // no input
}

bool rbinstringstream::_openID(const char* DeviceID)
{
    // Create own string with DeviceID value
    *_device = DeviceID;
    return true;
}
void rbinstringstream::_outDevice()
{
    // route all data from the output buffer to the string object
    // route data to string offset
    if (_odeviceIter>_device->size())
        _odeviceIter = _device->size();
    while (!_bufOut.is_empty())
    {
        if (_odeviceIter>=_device->size())
            _device->push_back( _bufOut.pop_back() );
        else
            _device->operator [](_odeviceIter) = _bufOut.pop_back();
        _odeviceIter++;
    }
}
bool rbinstringstream::_inDevice() const
{
    if (_ideviceIter<_deviceSize())
    {
        // route all available data from the device to the input buffer
        while (_ideviceIter<_device->size())
        {
            _bufIn.push_back( _device->operator [](_ideviceIter) );
            _ideviceIter++;
        }
        return true;
    }
    return false; // no input available from source
}

////test
//#include <iostream> // test
//using namespace std;
//
//int main(int argc,const char* args[])
//{
//  if (argc<=1)
//  {
//      cerr << "Enter an address." << endl;
//      return 1;
//  }
//  int i = 0;
//  int bytes[5] = {0};
//  str argies = args[1];
//  for (int i = 2;i<argc;i++)
//      argies += args[i];
//  rstringstream ss(argies);
//
//  ss.add_extra_delimiter('.');
//  ss.add_extra_delimiter('/');
//
//  while (ss>>bytes[i++]);
//
//  cout << "Ip address bytes:\n";
//  for (int i = 0;i<4;i++)
//      cout << bytes[i] << endl;
//
//  if (bytes[4])
//  {
//      dword subnet_mask = 0;
//      for (dword i = 0;i<bytes[4];i++)
//          subnet_mask |= 1<<(31-i);
//      cout << "Subnet mask bytes:\n";
//      for (int i = 0;i<4;i++)
//      {
//          cout << ((subnet_mask >> (24-i*8)) & 0xff) << endl;
//      }
//  }
//
//  return 0;
//}