//RStream.h - provides very basic stream functionality; this header shouldn't have to be included directly
#ifndef RSTREAM_H
#define RSTREAM_H
#include "RTypesTypes.h"
#include "RSet.h"
#include "RQueue.h"
#include "RStack.h"
#include "RString.h"

namespace rtypes
{
    /*          ABOUT ENDLINE ENCODING
        The IO virtual functions for stream_buffer (_outDevice and _inDevice) need to enforce proper end-line encoding, despite whatever platform
        they are implemented on. Use only the '\n' character to indicate an endline. You may or may not have to add other endline characters,
        such as '\r' depending on your platform. Simply ensure that _outDevice and _inDevice translate appropriately from the device data to the
        rstream expected format.
        Example:
            '\r\n' Windows file => _inDevice => '\n' => rstream
    */
    const str endline("\n");
    class stream_buffer
    {
    protected:
        stream_buffer() {}
        // output
        virtual void _outDevice() = 0; // route output buffer data to device; depending on implementation, this method may or may not route output at each invocation
        //          note: _outDevice should route data to the device at offset _odeviceIter (declared in rstreamBase)
        void _pushBackOutput(char c) { _bufOut.push_back(c); }
        void _pushBackOutputString(const str&);
        // input
        virtual bool _inDevice() const = 0; // route device data to input buffer; this should route all available input to the buffer; this should return true if input is now available in the buffer (regardless of if new input was put there)
        //          note: _inDevice should route data starting from offset _ideviceIter (declared in rstreamBase)
        bool _popInput(char&); // seeks the iterator
        bool _peekInput(char&) const;
        bool _hasInput() const { return !_bufIn.is_empty(); }
        //
        void _resetBufOut() { _bufOut.reset(); }
        void _resetBufIn() { _bufIn.reset(); }
        void _clearBufOut() { _bufOut.clear(); }
        void _clearBufIn() { _bufIn.clear(); }
        queue<char> _bufOut;
        mutable queue<char> _bufIn;
    };

    template<class T>
    class rstreamBase : protected stream_buffer
    {
    public:
        rstreamBase()
        {
            _lastSuccess = true;
            _isBuffering = false;
            _device = new T;
            _owned = true;
            _ideviceIter = 0;
            _odeviceIter = 0;
        }
        rstreamBase(const rstreamBase& obj)
        {
            _lastSuccess = obj._lastSuccess;
            _isBuffering = obj._isBuffering;
            _owned = obj._owned;
            if (_owned)
            {
                _device = new T; // copy the device if owned
                *_device = *obj._device;
            }
            else
                _device = obj._device;
            _ideviceIter = obj._ideviceIter;
            _odeviceIter = obj._odeviceIter;
        }
        ~rstreamBase()
        {
            if (_owned)
                delete _device;
        }
        // copy operator
        rstreamBase& operator =(const rstreamBase& obj)
        {
            if (&obj==this)
                return *this;
            _lastSuccess = obj._lastSuccess;
            _isBuffering = obj._isBuffering;
            close(); // sets _owned to true and allocates a new device
            if (obj._owned)
                *_device = *obj._device;
            else
                open(*obj._device);
            _ideviceIter = obj._ideviceIter;
            _odeviceIter = obj._odeviceIter;
            return *this;
        }
        // basic stream operations
        bool open(const char* DeviceID)
        {
            // we must own the device first
            if (_deviceSize()>0 || !_owned)
                return false; // stream hasn't been closed
            bool success = _openID(DeviceID);
            if (success)
                _odeviceIter = _deviceSize(); // set the output iterator for appending (default)
            return success;
        }
        bool open(T& Device)
        {
            // we must own the device first
            if (!_owned)
                return false; // stream hasn't been closed
            if (_owned)
                // free memory before overwritting the pointer
                delete _device;
            _device = &Device;
            _odeviceIter = _deviceSize(); // set the output iterator for appending (default)
            _owned = false;
            return true;
        }
        void clear() // remove all data used by the device and its stream
        {
            _clearDevice();
            // I use these because they don't do a deallocation
            _clearBufOut();
            reset_input_iter(); //_clearBufIn() is called through this
            reset_output_iter();
        }
        bool close() // clear the stream to start handling a new device (forget the old device)
        {
            if (_owned && !_deviceSize())
                return false;
            if (_owned)
                _clearDevice(); // just clear the device; there's no sense in calling _outDevice (since we have to clear the device)
            else
            {// the device had been specified by the user
                _outDevice(); // route any data in buffer to device
                _device = new T;
                _owned = true;
            }
            // reset buffers so I/O ops don't read/write to/from them AND reset iterators
            _clearBufOut();
            reset_input_iter(); //_clearBufIn() is called through this
            reset_output_iter();
            return true;
        }
        void reset_input_iter() // reset device input reads to the beginning of the stream
        {
            _ideviceIter = 0;
            _clearBufIn();
        }
        bool set_input_iter(dword Offset) // set device input reads to the beginning of the stream
        {
            if (Offset>=_deviceSize())
                return false;
            _ideviceIter = Offset;
            _clearBufIn(); // have to reset input buffer for new offset
            return true;
        }
        bool seek_input_iter(dword Amount,bool is_neg = false)
        {
            if (_ideviceIter+Amount>=_deviceSize())
                return false;
            // have to get input iter minus data size of input buffer
            // so make a call to get_input_iter_offset
            if (is_neg)
                _ideviceIter = get_input_iter_offset()-Amount;
            else
                _ideviceIter = get_input_iter_offset()+Amount;
            _clearBufIn(); // have to reset input buffer for new offset
            return true;
        }
        void reset_output_iter() { _odeviceIter = 0; }
        bool set_output_iter(dword Offset)
        {
            if (_odeviceIter>_deviceSize())
                return false;
            _odeviceIter = Offset;
            return true;
        }
        bool seek_output_iter(dword Amount,bool is_neg = false)
        {
            if (_odeviceIter+Amount>_deviceSize())
                return false;
            if (is_neg)
                _odeviceIter -= Amount;
            else
                _odeviceIter += Amount;
            return true;
        }
        void flush_output()
        {
            _outDevice();
        }
        void start_buffering_output()
        {
            _isBuffering = true;
            _outDevice(); // just in case there is output in the buffer
        }
        void stop_buffering_output()
        {
            _isBuffering = false;
            _outDevice(); // flush output buffer
        }
        bool is_buffering_output() const
        {
            return _isBuffering;
        }
        // basic operations
        void put(char c)
        {
            _pushBackOutput(c);
            if (!_isBuffering)
                _outDevice();
        }
        char get() // get the next char from the stream
        {
            char c = 0;
            _popInput(c);
            return c;
        }
        char peek() const
        {
            char c = 0;
            _peekInput(c);
            return c;
        }
        // misc. operations
        T& get_device() const { return *_device; }
        dword get_input_iter_offset() const { return _ideviceIter-_bufIn.size(); } // this will get the position in the device that is the offset of the next popped char (or end of stream)
        dword get_output_iter_offset() const { return _odeviceIter; }
        virtual bool has_input() const { return _ideviceIter<_deviceSize() /* input in device */ || _hasInput() /* input in buffer */; } // should determine if the device has input or not
        operator void*() const { return (void*) _lastSuccess; } // this state applies to the input stream
    protected:
        T* _device;
        bool _lastSuccess, // state of last input operation
            _isBuffering; // if true, all data is buffered in the stream buffer until stop_buffering() is called or flush(), so impl. shouldn't call _outDevice if true
        mutable dword _ideviceIter; // _inDevice should use this to keep track of device input
        mutable dword _odeviceIter; // _outDevice should use this to keep track of output for the device
        bool _isOwned() const { return _owned; }
        virtual bool _openID(const char*) = 0; // set/configure _device such that it responds to a device ID (or string)
        virtual void _clearDevice() = 0; // release all data from the device
        virtual dword _deviceSize() const = 0; // should return the number of bytes of data within the object pointed to by _device
    private:
        bool _owned;
    };

    // This I/O stream is for text.
    template<class T> // where T represents some I/O device
    class rstream : public rstreamBase<T>
    {
    public:
        rstream() : _repFlag(decimal) {}
        // delimit operations
        void add_extra_delimiter(char c) { _delimits.insert(c); } // add a new delimiter to the basic whitespace set
        void remove_extra_delimiter(char c) { _delimits.remove(c); }
        void clear_extra_delimiters() { _delimits.empty(); } // reset delimiters to whitespace only
        // output operators for basic types
        rstream& operator <<(char c)
        {
            _pushBackOutput(c);
            if (!_isBuffering)
                _outDevice();
            return *this;
        }
        rstream& operator <<(byte b)
        {
            _pushBackOutput(b);
            if (!_isBuffering)
                _outDevice();
            return *this;
        }
        rstream& operator <<(short s)
        {
            _pushBackOutputString( _toString(s,s<0) );
            if (!_isBuffering)
                _outDevice();
            return *this;
        }
        rstream& operator <<(word w)
        {
            _pushBackOutputString( _toString(w,false) );
            if (!_isBuffering)
                _outDevice();
            return *this;
        }
        rstream& operator <<(int i)
        {
            _pushBackOutputString( _toString(i,i<0) );
            if (!_isBuffering)
                _outDevice();
            return *this;
        }
        rstream& operator <<(dword d)
        {
            _pushBackOutputString( _toString(d,false) );
            if (!_isBuffering)
                _outDevice();
            return *this;
        }
        rstream& operator <<(const long long& l)
        {
            _pushBackOutputString( _toString(l,l<0) );
            if (!_isBuffering)
                _outDevice();
            return *this;
        }
        rstream& operator <<(const qword& q)
        {
            _pushBackOutputString( _toString(q,false) );
            if (!_isBuffering)
                _outDevice();
            return *this;
        }
        rstream& operator <<(const str& s)
        {
            _pushBackOutputString(s);
            if (!_isBuffering)
                _outDevice();
            return *this;
        }
        rstream& operator <<(rlib_numeric_rep_flag flag)
        {
            _repFlag = flag;
            return *this;
        }
        // input operators for basic types
        rstream& operator >>(char& var)
        {
            char input;
            do
            {
                if (!_popInput(input))
                {
                    _lastSuccess = false;
                    return *this; // no available input
                }
            } while (_isWhitespace(input));
            var = input;
            _lastSuccess = true;
            return *this;
        }
        rstream& operator >>(byte& var)
        {
            char input;
            do
            {
                if (!_popInput(input))
                {
                    _lastSuccess = false;
                    return *this; // no available input
                }
            } while (_isWhitespace(input));
            var = input;
            _lastSuccess = true;
            return *this;
        }
        rstream& operator >>(short& var)
        {
            str s;
            char input;
            while (true)
            {
                if (!_popInput(input))
                    break;
                else if (_isWhitespace(input))
                {
                    if (s.size()>0)
                        break;
                    else
                        continue;
                }
                s.push_back(input);
            }
            if (!s.size())
            {
                _lastSuccess = false;
                return *this;
            }
            var = _fromString<short> (s,_lastSuccess);
            return *this;
        }
        rstream& operator >>(word& var)
        {
            str s;
            char input;
            while (true)
            {
                if (!_popInput(input))
                    break;
                else if (_isWhitespace(input))
                {
                    if (s.size()>0)
                        break;
                    else
                        continue;
                }
                s.push_back(input);
            }
            if (!s.size())
            {
                _lastSuccess = false;
                return *this;
            }
            var = _fromString<word> (s,_lastSuccess);
            return *this;
        }
        rstream& operator >>(int& var)
        {
            str s;
            char input;
            while (true)
            {
                if (!_popInput(input))
                    break;
                else if (_isWhitespace(input))
                {
                    if (s.size()>0)
                        break;
                    else
                        continue;
                }
                s.push_back(input);
            }
            if (!s.size())
            {
                _lastSuccess = false;
                return *this;
            }
            var = _fromString<int> (s,_lastSuccess);
            return *this;
        }
        rstream& operator >>(dword& var)
        {
            str s;
            char input;
            while (true)
            {
                if (!_popInput(input))
                    break;
                else if (_isWhitespace(input))
                {
                    if (s.size()>0)
                        break;
                    else
                        continue;
                }
                s.push_back(input);
            }
            if (!s.size())
            {
                _lastSuccess = false;
                return *this;
            }
            var = _fromString<dword> (s,_lastSuccess);
            return *this;
        }
        rstream& operator >>(long long& var)
        {
            str s;
            char input;
            while (true)
            {
                if (!_popInput(input))
                    break;
                else if (_isWhitespace(input))
                {
                    if (s.size()>0)
                        break;
                    else
                        continue;
                }
                s.push_back(input);
            }
            if (!s.size())
            {
                _lastSuccess = false;
                return *this;
            }
            var = _fromString<long long> (s,_lastSuccess);
            return *this;
        }
        rstream& operator >>(qword& var)
        {
            str s;
            char input;
            while (true)
            {
                if (!_popInput(input))
                    break;
                else if (_isWhitespace(input))
                {
                    if (s.size()>0)
                        break;
                    else
                        continue;
                }
                s.push_back(input);
            }
            if (!s.size())
            {
                _lastSuccess = false;
                return *this;
            }
            var = _fromString<qword> (s,_lastSuccess);
            return *this;
        }
        rstream& operator >>(str& var)
        {
            var.clear(); // overwrite var
            char input;
            while (true)
            {
                if (!_popInput(input))
                    break;
                else if (_isWhitespace(input))
                {
                    if (var.size()>0)
                        break;
                    else
                        continue;
                }
                var.push_back(input);
            }
            _lastSuccess = var.size()>0; // success if any characters could be read
            return *this;
        }
        // other output operations
        void putline(const str& var)
        {
            _pushBackOutputString(var);
            _pushBackOutput('\n'); //add a newline
            _outDevice();
        }
        // other input operations
        void getline(str& var)
        {
            var.clear();
            char input;
            while (true)
            {
                if (!_popInput(input) || input=='\n')
                    break;
                //else if (input=='\r') // newline encoding is not supposed to use this with stream buffer data
                //  continue;
                var.push_back(input);
            }
            _lastSuccess = var.size()>0;
        }
    protected:
        // import names; preserve access modifiers
        // names from stream_buffer
        using stream_buffer::_outDevice;
        using stream_buffer::_pushBackOutput;
        using stream_buffer::_pushBackOutputString;
        using stream_buffer::_inDevice;
        using stream_buffer::_popInput;
        using stream_buffer::_peekInput;
        using stream_buffer::_hasInput;
        using stream_buffer::_resetBufOut;
        using stream_buffer::_resetBufIn;
        using stream_buffer::_clearBufOut;
        using stream_buffer::_clearBufIn;
        using stream_buffer::_bufOut;
        using stream_buffer::_bufIn;
        // names from rstreamBase<T>
        using rstreamBase<T>::_device;
        using rstreamBase<T>::_lastSuccess;
        using rstreamBase<T>::_isBuffering;
        using rstreamBase<T>::_ideviceIter;
        using rstreamBase<T>::_isOwned;
        using rstreamBase<T>::_openID;
        using rstreamBase<T>::_clearDevice;
        using rstreamBase<T>::_deviceSize;
    private:
        set<char> _delimits;
        rlib_numeric_rep_flag _repFlag;
        template<class Numeric>
        str _toString(Numeric n,bool is_neg)
        {
            if (is_neg && _repFlag!=decimal)
            {// occurs for signed types only - this preserves byte values for non-decimal reps
                qword mask = 0; // will mask bits not needed by the size of Numeric
                qword q = n; // treat n (Numeric) as an unsigned value
                for (dword i = 0;i<sizeof(Numeric);i++)
                {
                    mask <<= 8;
                    mask |= 0xff;
                }
                q &= mask; // apply mask
                return _toString(q,false); // recursively call _toString to get negative rep for non-decimal rep number
            }
            stack<char> digitChars;
            str r;
            if (is_neg)
            {// only true if Numeric is a signed type
                n *= -1;
                digitChars.push_back('-');
            }
            else if (n==0)
                digitChars.push_back('0');
            while (n>0)
            {
                char off = (char) (n%(Numeric) _repFlag);
                if (off<=9)
                    digitChars.push_back('0'+off);
                else
                    digitChars.push_back('A'+(off-10));
                n /= (Numeric) _repFlag;
            }
            while ( !digitChars.is_empty() )
                r.push_back( digitChars.pop_back() );
            //Numeric copy = n;
            //dword digitCnt = (is_neg || n==0 ? 1 : 0);
            //while (copy!=0)
            //{
            //  digitCnt++;
            //  copy /= (Numeric) _repFlag;
            //}
            //str r(digitCnt+1); // add one for the terminator
            //if (is_neg/* && _repFlag==decimal */)
            //{// only true if Numeric is a signed type
            //  // (only use negative signs with decimal representation)
            //  n *= -1;
            //  r[0] = '-';
            //}
            //else if (n==0)
            //  r[0] = '0';
            //for (int i = (int) (digitCnt-1);n;i--,n/=(Numeric) _repFlag)
            //{
            //  char off = (char) (n%(Numeric) _repFlag);
            //  if (off<=9)
            //      r[i] = '0'+off;
            //  else
            //  {
            //      off -= 10;
            //      r[i] = 'A'+off;
            //  }
            //}
            return r;
        }
        template<class Numeric>
        static Numeric _fromString(const str& s,bool& success)
        {
            Numeric r = 0;
            bool isNeg = (s[0]=='-');
            for (dword i = (isNeg || s[0]=='+' ? 1 : 0);i<s.size();i++)
            {
                if (s[i]>='0' && s[i]<='9')
                {
                    r *= 10;
                    r += s[i]-'0';
                }
                else
                {
                    success = false;
                    return r;
                }
            }
            if (isNeg)
                r *= -1;
            success = true;
            return r;
        }
        bool _isWhitespace(char c)
        {
            return c==' ' || c=='\n' || c=='\t' || c==0 || _delimits.contains(c);
        }
    };

    enum endianness
    {
        little, // least significant byte first
        big // most significant byte first
    };

    // This file stream is for binary data.
    template<class T>
    class rbinstream : public rstreamBase<T>
    {
    public:
        rbinstream(endianness EndianFlag) : _endianFlag(EndianFlag) {}
        //output operations
        rbinstream& operator <<(char c)
        {
            _pushBackOutput(c);
            if (!_isBuffering)
                _outDevice();
            return *this;
        }
        rbinstream& operator <<(byte b)
        {
            _pushBackOutput(b);
            if (!_isBuffering)
                _outDevice();
            return *this;
        }
        rbinstream& operator <<(short s)
        {
            _pushBackOutputString( _toString(s) );
            if (!_isBuffering)
                _outDevice();
            return *this;
        }
        rbinstream& operator <<(word w)
        {
            _pushBackOutputString( _toString(w) );
            if (!_isBuffering)
                _outDevice();
            return *this;
        }
        rbinstream& operator <<(int i)
        {
            _pushBackOutputString( _toString(i) );
            if (!_isBuffering)
                _outDevice();
            return *this;
        }
        rbinstream& operator <<(dword d)
        {
            _pushBackOutputString( _toString(d) );
            if (!_isBuffering)
                _outDevice();
            return *this;
        }
        rbinstream& operator <<(const long long& l)
        {
            _pushBackOutputString( _toString(l) );
            if (!_isBuffering)
                _outDevice();
            return *this;
        }
        rbinstream& operator <<(const qword& q)
        {
            _pushBackOutputString( _toString(q) );
            if (!_isBuffering)
                _outDevice();
            return *this;
        }
        rbinstream& operator <<(const str& s)
        {
            _pushBackOutputString(s);
            if (!_isBuffering)
                _outDevice();
            return *this;
        }
        //input operations
        rbinstream& operator >>(char& var)
        {
            if (!_popInput(var))
                _lastSuccess = false;
            _lastSuccess = true;
            return *this;
        }
        rbinstream& operator >>(byte& var)
        {
            if (!_popInput((char&) var))
                _lastSuccess = false;
            _lastSuccess = true;
            return *this;
        }
        rbinstream& operator >>(short& var)
        {
            str data(3); // add 1 for null terminator
            // load data
            for (int i = 0;i<2;i++)
                if (!_popInput(data[i]))
                {
                    _lastSuccess = false;
                    return *this;
                }
            // calc. value
            var = _fromString<short>(data,_lastSuccess);
            return *this;
        }
        rbinstream& operator >>(word& var)
        {
            str data(3); // add 1 for null terminator
            // load data
            for (int i = 0;i<2;i++)
                if (!_popInput(data[i]))
                {
                    _lastSuccess = false;
                    return *this;
                }
            // calc. value
            var = _fromString<word>(data,_lastSuccess);
            return *this;
        }
        rbinstream& operator >>(int& var)
        {
            str data(5); // add 1 for null terminator
            // load data
            for (int i = 0;i<4;i++)
                if (!_popInput(data[i]))
                {
                    _lastSuccess = false;
                    return *this;
                }
            // calc. value
            var = _fromString<int>(data,_lastSuccess);
            return *this;
        }
        rbinstream& operator >>(dword& var)
        {
            str data(5); // add 1 for null terminator
            // load data
            for (int i = 0;i<4;i++)
                if (!_popInput(data[i]))
                {
                    _lastSuccess = false;
                    return *this;
                }
            // calc. value
            var = _fromString<dword>(data,_lastSuccess);
            return *this;
        }
        rbinstream& operator >>(long long& var)
        {
            str data(9); // add 1 for null terminator
            // load data
            for (int i = 0;i<8;i++)
                if (!_popInput(data[i]))
                {
                    _lastSuccess = false;
                    return *this;
                }
            // calc. value
            var = _fromString<long long>(data,_lastSuccess);
            return *this;
        }
        rbinstream& operator >>(qword& var)
        {
            str data(9); // add 1 for null term
            // load data
            for (int i = 0;i<8;i++)
                if (!_popInput(data[i]))
                {
                    _lastSuccess = false;
                    return *this;
                }
            // calc. value
            var = _fromString<qword>(data,_lastSuccess);
            return *this;
        }
    protected:
        // names from stream_buffer
        using stream_buffer::_outDevice;
        using stream_buffer::_pushBackOutput;
        using stream_buffer::_pushBackOutputString;
        using stream_buffer::_inDevice;
        using stream_buffer::_popInput;
        using stream_buffer::_peekInput;
        using stream_buffer::_hasInput;
        using stream_buffer::_resetBufOut;
        using stream_buffer::_clearBufOut;
        using stream_buffer::_clearBufIn;
        using stream_buffer::_bufOut;
        using stream_buffer::_bufIn;
        // names from rstreamBase<T>
        using rstreamBase<T>::_device;
        using rstreamBase<T>::_lastSuccess;
        using rstreamBase<T>::_isBuffering;
        using rstreamBase<T>::_ideviceIter;
        using rstreamBase<T>::_isOwned;
        using rstreamBase<T>::_openID;
        using rstreamBase<T>::_clearDevice;
        using rstreamBase<T>::_deviceSize;
    private:
        endianness _endianFlag;
        template<class Numeric>
        str _toString(Numeric n) // divide n into its bytes; put in a string obj.
        {// assume endianness from _endianFlag
            str bytes;
            int byteCnt = sizeof(Numeric);
            if (_endianFlag==big)
            {// get most significant to least significant
                for (int i = byteCnt-1;i>=0;i--)
                    bytes.push_back( (n>>(8*i))&0xff );
            }
            else if (_endianFlag==little)
            {// get least significant to most significant
                for (int i = 0;i<byteCnt;i++)
                    bytes.push_back( (n>>(8*i))&0xff );
            }       
            return bytes;
        }
        template<class Numeric>
        Numeric _fromString(const str& s,bool& success)
        {// assume endianness from _endianFlag
            Numeric val = 0;
            dword byteCnt = sizeof(Numeric);
            if (s.size()!=byteCnt)
            {
                success = false;
                return val;
            }
            if (_endianFlag==big)
            {// bytes are ordered most significant to least significant
                for (int i = int(byteCnt)-1;i>=0;i--)
                    val |= (((Numeric) s[int(byteCnt)-1-i] & 0xff)<<(8*i));
                success = true;
            }
            else if (_endianFlag==little)
            {// bytes are ordered least significant to most significant
                for (int i = 0;i<int(byteCnt);i++)
                    val |= (((Numeric) s[i] & 0xff)<<(8*i));
                success = true;
            }
            else
                success = false;
            return val;
        }
    };
}

#endif