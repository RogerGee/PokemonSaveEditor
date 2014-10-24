#include "RStream.h"
using namespace rtypes;

void stream_buffer::_pushBackOutputString(const str& s)
{
    for (dword i = 0;i<s.size();i++)
        _pushBackOutput(s[i]);
}
bool stream_buffer::_popInput(char& var)
{
    // check for input
    if (_hasInput())
    {
        var = _bufIn.pop_back();
        return true;
    }
    //else
    if (_inDevice()) // request more input
        return _popInput(var); // get next char from input
    //else no input was available from source
    return false;
}
bool stream_buffer::_peekInput(char& var) const
{
    if (_hasInput())
    {
        var = _bufIn.peek(); // doesn't advance the iterator
        return true;
    }
    //else
    if (_inDevice()) // check for input
        return _peekInput(var); // get next char from input
    //else no input was available from source
    return false;
}