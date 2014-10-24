#include "RWin32LibMaster.h"
#include "RStack.h"
using namespace rtypes;
using namespace rtypes::rwin32;
using namespace rtypes::rwin32::console;

namespace
{
    stack<const ConsoleBuffer*> stackedBuffers;
}

ConsoleChar& ConsoleChar::operator =(char c)
{
    character = c;
    return *this;
}
void ConsoleChar::GetCharInfo(PCHAR_INFO pcharInf) const
{
    pcharInf->Char.AsciiChar = character;
    pcharInf->Attributes = (characterBold ? FOREGROUND_INTENSITY : NULL) |
        (characterReversed ? COMMON_LVB_REVERSE_VIDEO : NULL) |
        (characterUnderscored ? COMMON_LVB_UNDERSCORE : NULL);
    if (characterColor.GetBlue()>127)
        pcharInf->Attributes |= FOREGROUND_BLUE;
    if (characterColor.GetGreen()>127)
        pcharInf->Attributes |= FOREGROUND_GREEN;
    if (characterColor.GetRed()>127)
        pcharInf->Attributes |= FOREGROUND_RED;
    if (backColor.GetBlue()>127)
        pcharInf->Attributes |= BACKGROUND_BLUE;
    if (backColor.GetGreen()>127)
        pcharInf->Attributes |= BACKGROUND_GREEN;
    if (backColor.GetRed()>127)
        pcharInf->Attributes |= BACKGROUND_RED;
}
void ConsoleChar::SetFromCharInfo(const CHAR_INFO* pcharInf)
{
    character = pcharInf->Char.AsciiChar;
    
}

ConsoleString::ConsoleString(const str& s)
{
    _virtAlloc(s.length()+1);
    for (dword i = 0;i<s.length();i++)
        (*this)[i] = s[i];
    _nullTerm();
}
void ConsoleString::apply_formatting(const ConsoleChar& c)
{
    for (dword i = 0;i<length();i++)
    {
        ConsoleChar& theChar = (*this)[i];
        char hold = theChar.character;
        theChar = c;
        theChar.character = hold;
    }
}
void ConsoleString::apply_formatting_to_range(const ConsoleChar& c,int Start,int Stop)
{
    while (Start<=Stop)
    {
        ConsoleChar& theChar = (*this)[Start++];
        char hold = theChar.character;
        theChar = c;
        theChar.character = hold;
    }
}
ConsoleString& ConsoleString::operator =(const str& s)
{
    _virtAlloc(s.length()+1);
    for (dword i = 0;i<s.length();i++)
        (*this)[i] = s[i];
    _nullTerm();
    return *this;
}
ConsoleString& ConsoleString::operator +=(const str& s)
{
    dword stringEnd = size();
    _virtAlloc(stringEnd+s.length()+1);
    for (dword i = 0;i<s.length();i++)
        (*this)[stringEnd+i] = s[i];
    _nullTerm();
    return *this;
}
str ConsoleString::to_plain_string() const
{
    str r;
    for (dword i = 0;i<size();i++)
        r.push_back( (*this)[i].character );
    return r;
}

const COORD ConsoleBuffer::ORIGIN = {0,0};
ConsoleBuffer::ConsoleBuffer()
{
    // create a new console screen buffer
    SECURITY_ATTRIBUTES secAttrib;
    ZeroMemory(&secAttrib,sizeof(SECURITY_ATTRIBUTES));
    secAttrib.bInheritHandle = TRUE;
    _hConsole = ::CreateConsoleScreenBuffer(GENERIC_READ|GENERIC_WRITE,NULL,&secAttrib,CONSOLE_TEXTMODE_BUFFER,NULL);
    ZeroMemory(&_info,sizeof(CONSOLE_SCREEN_BUFFER_INFO));
    _handleOwned = true;
    _shown = false;
    _inShowStack = false;
    _charData = NULL;
    _initFromSource();
}
ConsoleBuffer::ConsoleBuffer(HANDLE hConsole)
{
    _hConsole = hConsole;
    ZeroMemory(&_info,sizeof(CONSOLE_SCREEN_BUFFER_INFO));
    _handleOwned = false;
    _shown = false;
    _inShowStack = false;
    _charData = NULL;
    _initFromSource();
}
ConsoleBuffer::ConsoleBuffer(const ConsoleBuffer& obj)
{
    // create a new console screen buffer
    int numberOfChars = obj.GetBufferLength();
    SECURITY_ATTRIBUTES secAttrib;
    ZeroMemory(&secAttrib,sizeof(SECURITY_ATTRIBUTES));
    secAttrib.bInheritHandle = TRUE;
    _hConsole = ::CreateConsoleScreenBuffer(GENERIC_READ|GENERIC_WRITE,NULL,&secAttrib,CONSOLE_TEXTMODE_BUFFER,NULL);
    _handleOwned = true;
    _shown = false;
    _inShowStack = false;
    ZeroMemory(&_info,sizeof(CONSOLE_SCREEN_BUFFER_INFO));
    // copy object (this copies even "unpresented" data)
    if (_hConsole==INVALID_HANDLE_VALUE)
        // bad object/buffer creation
        return;
    _charData = NULL;
    if (numberOfChars==0 || !SetBufferSize(obj.GetBufferSize(),false))
    {
        _initFromSource();
        return;
    }
    for (int i = 0;i<numberOfChars;i++)
        _charData[i] = obj._charData[i]; // copy from obj's local buffer
    _info = obj._info;
    PresentBuffer(); // write data to win32 buffer
}
ConsoleBuffer::ConsoleBuffer(const Size& s)
{

}
ConsoleBuffer::ConsoleBuffer(const ConsoleBuffer& obj,const Size& s)
{
    // create a new console screen buffer
    SECURITY_ATTRIBUTES secAttrib;
    ZeroMemory(&secAttrib,sizeof(SECURITY_ATTRIBUTES));
    secAttrib.bInheritHandle = TRUE;
    _hConsole = ::CreateConsoleScreenBuffer(GENERIC_READ|GENERIC_WRITE,NULL,&secAttrib,CONSOLE_TEXTMODE_BUFFER,NULL);
    _handleOwned = true;
    _shown = false;
    _inShowStack = false;
    // copy object (this copies even "unpresented" data)
    if (_hConsole==INVALID_HANDLE_VALUE)
        // bad object/buffer creation
        return;

}
ConsoleBuffer::~ConsoleBuffer()
{
    while (_inShowStack)
        UnShow(); // unfortunately, I have to remove all screens from the show stack until this screen is no longer in the stack!
    if (_handleOwned)
        CloseHandle(_hConsole);
    delete[] _charData;
}
bool ConsoleBuffer::PresentBuffer()
{
    SMALL_RECT rect;
    rect.Top = 0; // x
    rect.Left = 0; // y
    rect.Right = _info.dwSize.X-1;
    rect.Bottom = _info.dwSize.Y-1;
    return WriteConsoleOutput(_hConsole,_charData,_info.dwSize,ORIGIN,&rect)==TRUE;
}
void ConsoleBuffer::ClearBuffer()
{
    int length = GetBufferLength();
    for (int i = 0;i<length;i++)
    {
        _charData[i].Attributes = _info.wAttributes;
        _charData[i].Char.AsciiChar = ' ';
    }
}
bool ConsoleBuffer::Show()
{
    if (!stackedBuffers.is_empty())
        stackedBuffers.peek()->_shown = false;
    stackedBuffers.push_back(this);
    this->_shown = true;
    this->_inShowStack = true;
    return ::SetConsoleActiveScreenBuffer(_hConsole)!=0;
}
bool ConsoleBuffer::UnShow()
{// static method
    // unshow the current screen and replace it with the next screen on the stack
    if (!stackedBuffers.is_empty())
    {
        HANDLE hNew;
        const ConsoleBuffer* buf = stackedBuffers.pop_back();
        buf->_shown = false;
        buf->_inShowStack = false;
        if (stackedBuffers.is_empty())
            hNew = GetStdHandle(STD_OUTPUT_HANDLE);
        else
            hNew = stackedBuffers.peek()->GetConsoleBufferHandle();
        return ::SetConsoleActiveScreenBuffer(hNew)!=0;
    }
    return false; // have reached final buffer (main startup buffer)
}
ConsoleChar ConsoleBuffer::operator ()(int Column,int Row) const
{
    ConsoleChar r;
    r.SetFromCharInfo( &(this->operator[](Row)[Column]) );
    return r;
}
void ConsoleBuffer::operator ()(int Column,int Row,const ConsoleString& stringObj,bool Wrap)
{
    // replace complete string
    for (dword i = 0;i<stringObj.length();i++)
    {
        if (Column>=_info.dwSize.X)
        {
            Column = 0;
            Row++;
        }
        if (Row>=_info.dwSize.Y)
            return; // no more space to write string
        (*this)(Column++,Row,stringObj[i]); // put char at point in buffer
    }
}
ConsoleBuffer& ConsoleBuffer::operator =(HANDLE hConsole)
{
    if (_handleOwned)
        CloseHandle(_hConsole);
    _hConsole = hConsole;
    _handleOwned = false;
    _initFromSource();
    return *this;
}
ConsoleBuffer& ConsoleBuffer::operator =(const ConsoleBuffer& obj)
{
    if (&obj==this)
        return *this;
    if (_handleOwned)
        CloseHandle(_hConsole);
    else
        _handleOwned = true;
    // create a new console screen buffer
    int numberOfChars = obj.GetBufferLength();
    SECURITY_ATTRIBUTES secAttrib;
    ZeroMemory(&secAttrib,sizeof(SECURITY_ATTRIBUTES));
    secAttrib.bInheritHandle = TRUE;
    _hConsole = ::CreateConsoleScreenBuffer(GENERIC_READ|GENERIC_WRITE,NULL,&secAttrib,CONSOLE_TEXTMODE_BUFFER,NULL);
    // copy object (this copies even "unpresented" data)
    if (_hConsole==INVALID_HANDLE_VALUE || numberOfChars==0)
        // bad object
        return *this;
    SetBufferSize(obj.GetBufferSize(),false);
    for (int i = 0;i<numberOfChars;i++)
        _charData[i] = obj._charData[i]; // copy from obj's local buffer
    _info = obj._info; // just in case
    return *this;
}
bool ConsoleBuffer::SetBufferSize(const Size& bufSize,bool SaveOld)
{
    if (bufSize!=GetBufferSize())
    {
        int length = bufSize.width*bufSize.height;
        COORD c;
        c.X = bufSize.width; // cols
        c.Y = bufSize.height; // rows
        // attempt to resize win32 screen buffer
        if (!SetConsoleScreenBufferSize(_hConsole,c) || /* just in case */ bufSize.width==0 || bufSize.height==0)
            return false;
        // resize our memory
        CHAR_INFO* newData = new CHAR_INFO[length];
        // ensure the buffer is blank
        for (int i = 0;i<length;i++)
        {
            newData[i].Char.AsciiChar = ' ';
            newData[i].Attributes = _info.wAttributes;
        }
        // copy what can be copied from old data (if specified)
        if (SaveOld)
            _copyTo(newData,bufSize.width,bufSize.height);
        // delete old ptr and copy new ptr
        delete[] _charData;
        _charData = newData;
        // update info for convienient lookup
        _updateInfo();
    }
    return true;
}
bool ConsoleBuffer::EnsureBufferSize(Size s,bool SaveOld)
{
    Size mySize = GetBufferSize();
    if (mySize.width<s.width || mySize.height<s.height)
    {
        if (mySize.width>s.width)
            s.width = mySize.width;
        else if (mySize.height>s.height)
            s.height = mySize.height;
        return SetBufferSize(s,SaveOld);
    }
    return false;
}
bool ConsoleBuffer::SetCursorLocation(const Point& p)
{
    COORD pos;
    pos.X = p.x;
    pos.Y = p.y;
    if (::SetConsoleCursorPosition(_hConsole,pos)==0)
        return false;
    // update for accurate lookup
    _updateInfo();
    return true;
}
void ConsoleBuffer::_updateInfo()
{
    ZeroMemory(&_info,sizeof(CONSOLE_SCREEN_BUFFER_INFO));
    GetConsoleScreenBufferInfo(_hConsole,&_info);
}
void ConsoleBuffer::_copyFrom(CHAR_INFO* data,int colCount,int rowCount)
{
    // copy all data that is possible from one buffer to the next
    int otherLength = colCount*rowCount;
    int myLength = GetBufferLength();
    for (int i = 0;i<otherLength && i<myLength;i++)
        _charData[i] = data[i];
}
void ConsoleBuffer::_copyTo(CHAR_INFO* data,int colCount,int rowCount)
{
    // copy all data that is possible from one buffer to the next
    int newLength = colCount*rowCount;
    int oldLength = GetBufferLength();
    for (int i = 0;i<newLength && i<oldLength;i++)
        data[i] = _charData[i];

}
void ConsoleBuffer::_initFromSource()
{
    SMALL_RECT readRect;
    int oldArea = GetBufferLength();
    _updateInfo(); // make sure information about the win32 buffer is up to date
    if (_info.dwSize.X*_info.dwSize.Y!=oldArea)
    {// allocation size has changed from previous
        delete[] _charData;
        _charData = NULL;
        if (_info.dwSize.X*_info.dwSize.Y==0)
            return;
        // allocate the buffer
        _charData = new CHAR_INFO[_info.dwSize.X*_info.dwSize.Y];
    }
    else if (oldArea==0)
        return;
    // read data from the win32 buffer to our local buffer
    // the operation fails on large buffer sizes, so I read 1 row at a time
    readRect.Left = 0; //x - read from beginning column...
    readRect.Top = 0; //y - start at top row
    readRect.Right = _info.dwSize.X-1; //x - ...to ending column
    readRect.Bottom = readRect.Top; //y - read one row per time
    for (int row = 0;row<_info.dwSize.Y;row++,readRect.Top++,readRect.Bottom++)
        ReadConsoleOutput(_hConsole,_charData+row*_info.dwSize.X,_info.dwSize,ORIGIN,&readRect);
}

Console::Console()
{
    // get the standard handles
    /*      If a call to GetStdHandle returns NULL, there is no redirected handle,
            so it is set to INVALID_HANDLE_VALUE so that this class can
            understand that it isn't used.
    */
    _hStdOut = ::GetStdHandle(STD_OUTPUT_HANDLE);
    if (_hStdOut==NULL)
        _hStdOut = INVALID_HANDLE_VALUE;
    _hStdErr = ::GetStdHandle(STD_ERROR_HANDLE);
    if (_hStdErr==NULL)
        _hStdErr = INVALID_HANDLE_VALUE;
    _hStdIn = ::GetStdHandle(STD_INPUT_HANDLE);
    if (_hStdIn==NULL)
        _hStdIn = INVALID_HANDLE_VALUE;
    // other initialization
    _charsRead = 0;
    _charsWrote = 0;
    _lastInputSuccess = true;
    _lastOutputSuccess = true;
}
Console::Console(HANDLE hStdOut,HANDLE hStdIn,HANDLE hStdErr)
{
    // set specified standard handles
    _hStdOut = hStdOut;
    _hStdIn = hStdIn;
    _hStdErr = hStdErr;
    // other initialization
    _charsRead = 0;
    _charsWrote = 0;
    _lastInputSuccess = true;
    _lastOutputSuccess = true;
}
void Console::ClearOutput()
{
    ConsoleBuffer buffer(_hStdOut);
    buffer.ClearBuffer();
    buffer.PresentBuffer();
}
bool Console::SetWriteCursor(const Point& p)
{
    COORD pos;
    pos.X = p.x;
    pos.Y = p.y;
    return ::SetConsoleCursorPosition(_hStdOut,pos)!=0;
}
bool Console::SetWriteCursorErr(const Point& p)
{
    COORD pos;
    pos.X = p.x;
    pos.Y = p.y;
    return ::SetConsoleCursorPosition(_hStdErr,pos)!=0;
}
void Console::WriteLine(const str& lineText)
{
    Write(lineText);
    Write("\r\n");
}
void Console::WriteLine(const char* pData,dword len)
{
    Write(pData,len);
    Write("\r\n");
}
void Console::WriteLineErr(const str& lineText)
{
    WriteErr(lineText);
    WriteErr("\r\n");
}
void Console::WriteLineErr(const char* pData,dword len)
{
    WriteErr(pData,len);
    WriteErr("\r\n");
}
char Console::Read()
{
    char input;
    _lastInputSuccess = ReadFile(_hStdIn,&input,1,&_charsRead,NULL)==TRUE && _charsRead>=1;
    return input;
}
str Console::ReadLine()
{
    str buffer;
    while (true)
    {
        char c = Read();
        if (!_lastInputSuccess)
            return buffer; // return what we got
        if (c=='\r')
        {
            c = Read(); // read past '\n'
            break;
        }
        buffer.push_back(c);
    }
    return buffer;
}
Console& Console::operator >>(HANDLE hOut)
{
    _hStdOut = hOut;
    return *this;
}
Console& Console::operator <<(HANDLE hIn)
{
    _hStdIn = hIn;
    return *this;
}
void Console::ReloadFromStdHandles()
{
    _hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    _hStdIn = GetStdHandle(STD_INPUT_HANDLE);
    _hStdErr = GetStdHandle(STD_ERROR_HANDLE);
}
void Console::RedirectHandles(HANDLE hOut,HANDLE hIn,HANDLE hErr)
{
    _hStdOut = hOut;
    _hStdIn = hIn;
    _hStdErr = hErr;
}

ConsoleStream rtypes::rwin32::console::theConsole;

bool ConsoleStream::set_output_iter(dword Column,dword Row)
{
    // linearize the cursor position
    HANDLE hConsole = NULL;
    if (_outputFlag==StdOut)
        hConsole = _device->GetStdOutputHandle();
    else if (_outputFlag==StdErr)
        hConsole = _device->GetStdErrorHandle();
    if (hConsole==NULL || ::GetConsoleScreenBufferInfo(hConsole,&_info)==0 || !set_output_iter( _info.dwSize.X*Row+Column ))
        return false;
    _iterChange = true;
    return true;
}
bool ConsoleStream::seek_output_iter(dword x_amt,dword y_amt,bool XIsNeg,bool YIsNeg)
{
    // linearize the amount, then call seek_output_iter(dword) to
    // seek the amount
    int dif; bool isNeg = false;
    HANDLE hConsole = NULL;
    if (_outputFlag==StdOut)
        hConsole = _device->GetStdOutputHandle();
    else if (_outputFlag==StdErr)
        hConsole = _device->GetStdErrorHandle();
    if (hConsole==NULL || ::GetConsoleScreenBufferInfo(hConsole,&_info)==0)
        return false;
    dif = _info.dwSize.X*y_amt * (YIsNeg ? -1 : 1);
    dif += XIsNeg ? -int(x_amt) : x_amt;
    if (dif<0)
    {
        dif *= -1;
        isNeg = true;
    }
    if (seek_output_iter(dword(dif),isNeg))
    {
        _iterChange = true;
        return true;
    }
    return false;
}
bool ConsoleStream::_openID(const char* p)
{

    return false;
}
dword ConsoleStream::_deviceSize() const
{
    // this will only work on console screen buffers,
    // which are the devices I should hope are being used with a console stream
    // note: I don't use ConsoleBuffer so that it doesn't allocate a local buffer
    //  (which wouldn't get used)
    HANDLE hConsole = NULL;
    if (_outputFlag==StdOut)
        hConsole = _device->GetStdOutputHandle();
    else if (_outputFlag==StdErr)
        hConsole = _device->GetStdErrorHandle();
    else if (hConsole==NULL || ::GetConsoleScreenBufferInfo(hConsole,&_info)==0)
        return 0;
    return _info.dwSize.X*_info.dwSize.Y;
}
void ConsoleStream::_outDevice()
{
    // set cursor
    _setCursorFromOIter();
    // print all chars in stream output buffer to the console output buffer
    while (!_bufOut.is_empty())
    {
        dword len = 0;
        const char* data = &_bufOut.peek(); // rtypes::queue is implemented for this purpose (on the other hand, rtypes::wrapped_queue isn't)
        while (len<_bufOut.size() && data[len]!='\n')
            len++;
        // increment len if an endline was found so that the '\n' is dequeued in the virtual range push
        if (_outputFlag==StdOut)
        {
            if (data[len]=='\n')
                _device->WriteLine( data,len++ );
            else
                _device->Write( data,len );
        }
        else if (_outputFlag==StdErr)
        {
            if (data[len]=='\n')
                _device->WriteLineErr( data,len++ );
            else
                _device->WriteErr( data,len );
        }
        _bufOut.pop_range(len);
    }
    // update output device iterator
    _setOIterFromCursor();
    // clear output buffer
    _clearBufOut();
}
bool ConsoleStream::_inDevice() const
{
    // read a line of input into the stream input buffer
    str s = _device->ReadLine();
    for (dword i = 0;i<s.length();i++)
        _bufIn.push_back(s[i]);
    // we need to put an endline into the input buffer so that the stream can
    // parse the input
    _bufIn.push_back('\n');
    return _device->GetLastInputSuccess();
}
void ConsoleStream::_clearDevice()
{
    _device->ClearInput();
    _device->ClearOutput();
}
void ConsoleStream::_setCursorFromOIter()
{
    HANDLE hConsole = NULL;
    if (_outputFlag==StdOut)
        hConsole = _device->GetStdOutputHandle();
    else if (_outputFlag==StdErr)
        hConsole = _device->GetStdErrorHandle();
    if (hConsole!=NULL && _iterChange) // only update iterator if a change is needed, else there's some overhead
    {
        if (::GetConsoleScreenBufferInfo(hConsole,&_info))
        {
            COORD pos;
            // take the output buffer iterator and multi-dimensionalize it
            pos.X = _odeviceIter % _info.dwSize.X;
            pos.Y = _odeviceIter / _info.dwSize.X;
            ::SetConsoleCursorPosition(hConsole,pos);
        }
        _iterChange = false; // assume the iterator will now match, since it should be automatically incremeneted by whatever console host is rendering the console
    }
}
void ConsoleStream::_setOIterFromCursor()
{
    HANDLE hConsole = NULL;
    if (_outputFlag==StdOut)
        hConsole = _device->GetStdOutputHandle();
    else if (_outputFlag==StdErr)
        hConsole = _device->GetStdErrorHandle();
    if (hConsole!=NULL && ::GetConsoleScreenBufferInfo(hConsole,&_info))
        _odeviceIter = _info.dwSize.X*_info.dwCursorPosition.Y+_info.dwCursorPosition.X;
}

ConsoleStream& console::operator <<(ConsoleStream& stream,ConsoleStreamOutputFlag flag)
{
    stream._outputFlag = flag;
    return stream;
}