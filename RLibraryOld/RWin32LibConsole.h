//RWin32LibConsole.h
#ifndef RWIN32LIBCONSOLE_H
#define RWIN32LIBCONSOLE_H

namespace rtypes
{
    namespace rwin32
    {
        namespace console
        {
            struct ConsoleChar
            {
                ConsoleChar() : character(0),characterColor(colors::White),backColor(colors::Black),characterBold(false),characterReversed(false),characterUnderscored(false) {}
                ConsoleChar(char c) : character(c),characterColor(colors::White),backColor(colors::Black),characterBold(false),characterReversed(false),characterUnderscored(false) {}
                ConsoleChar(char c,const Color& clrText,const Color& clrBack) : character(c),characterColor(clrText),backColor(clrBack),characterBold(false),characterReversed(false),characterUnderscored(false) {}
                ConsoleChar(char c,const Color& clrText,const Color& clrBack,bool bold,bool reversed,bool underscored)
                    : character(c),characterColor(clrText),backColor(clrBack),characterBold(bold),characterReversed(reversed),characterUnderscored(underscored) {}
                char character;
                Color characterColor,backColor; // this color may be simplified 
                bool characterBold,
                    characterReversed, // the character cell is reversed
                    characterUnderscored;
                ConsoleChar& operator =(char c);
                void GetCharInfo(PCHAR_INFO) const;
                void SetFromCharInfo(const CHAR_INFO*);
            };

            class ConsoleString : public rtype_string<ConsoleChar>
            {
            public:
                ConsoleString() {}
                ConsoleString(const str& s);
                /*
                    Applies formatting from the specified character to the entire string.
                    The character attribute of the ConsoleChar object is not copied, just
                        the formatting
                */
                void apply_formatting(const ConsoleChar& Formatting);
                void apply_formatting_to_range(const ConsoleChar& Formatting,int Start,int Stop);

                ConsoleString& operator =(const str& s);
                ConsoleString& operator +=(const str& s);

                str to_plain_string() const;
            };

            class ConsoleBuffer // technically, this is a buffer for a buffer!
            {
            public:
                ConsoleBuffer();
                ConsoleBuffer(HANDLE); // create with pre-existing buffer
                ConsoleBuffer(const ConsoleBuffer&); // create a new buffer as a copy
                ConsoleBuffer(const Size&);
                ConsoleBuffer(const ConsoleBuffer&,const Size&); // create a copy, but maintain specified size
                ~ConsoleBuffer();

                // operations
                bool PresentBuffer(); // write buffer contents to video buffer
                void UpdateBufferFromSource() { _initFromSource(); }
                void ClearBuffer();
                bool Show();
                static bool UnShow(); // "un-shows" the most current buffer

                // operators
                operator void* () const { return (void*) (_hConsole==INVALID_HANDLE_VALUE); }
                CHAR_INFO* operator [](int Row) { return _charData+Row*_info.dwSize.X; } // 2D
                const CHAR_INFO* operator [](int Row) const { return _charData+Row*_info.dwSize.X; }
                CHAR_INFO& operator ()(int i) { return _charData[i]; } // linear
                const CHAR_INFO& operator ()(int i) const { return _charData[i]; }
                ConsoleChar operator ()(int Column,int Row) const; // get console char
                void operator ()(int Column,int Row,const ConsoleChar& charObj) { charObj.GetCharInfo( &(this->operator[](Row)[Column]) ); } // specify console char
                void operator ()(int Column,int Row,const ConsoleString&,bool Wrap = false); // specify console string at specified location
                ConsoleBuffer& operator =(HANDLE); // overwrite with pre-existing buffer
                ConsoleBuffer& operator =(const ConsoleBuffer&); // create a copy

                // "get" methods
                HANDLE GetConsoleBufferHandle() const { return _hConsole; }
                Size GetBufferSize() const { return Size(_info.dwSize.X,_info.dwSize.Y); }
                int GetBufferLength() const { return _info.dwSize.X*_info.dwSize.Y; }
                Point GetCursorLocation() const { return Point(_info.dwCursorPosition.X,_info.dwCursorPosition.Y); }

                // "set" methods
                bool SetBufferSize(const Size&,bool SaveOld = true); // the size is in character units
                //      resizes the buffer to ensure it is at least as big as the specified size
                bool EnsureBufferSize(Size,bool SaveOld = true); //returns true if the buffer needed to be resized
                bool SetCursorLocation(const Point& location);
                bool SetCursorLocation(int column,int row) { return SetCursorLocation(Point(column,row)); }

                // other
                bool IsShown() const { return _shown; }
            private:
                static const COORD ORIGIN;
                CONSOLE_SCREEN_BUFFER_INFO _info; // keep '_info' updated throughout object lifetime
                HANDLE _hConsole;
                CHAR_INFO* _charData;
                bool _handleOwned;
                mutable bool _shown;
                mutable bool _inShowStack;
                void _updateInfo();
                void _copyFrom(CHAR_INFO*,int colCount,int rowCount);
                void _copyTo(CHAR_INFO*,int colCount,int rowCount);
                void _initFromSource();
            };

            class Console
            {
            public:
                Console();
                Console(HANDLE hStdOut,HANDLE hStdIn,HANDLE hStdErr);

                // operations
                //      only call these methods if the output hasn't been redirected
                //      in other words, the handles should be console screen buffers
                void ClearOutput();
                void ClearInput() { ::FlushConsoleInputBuffer(_hStdIn); }
                bool SetWriteCursor(const Point& location);
                bool SetWriteCursor(int column,int row) { return SetWriteCursor(Point(column,row)); }
                bool SetWriteCursorErr(const Point& location);
                bool SetWriteCursorErr(int column,int row) { return SetWriteCursorErr(Point(column,row)); }
                //
                void Write(char c) { _lastOutputSuccess = WriteFile(_hStdOut,&c,1,&_charsWrote,NULL)==TRUE && _charsWrote>=1; }
                void Write(const str& s) { _lastOutputSuccess = WriteFile(_hStdOut,&s[0],s.size(),&_charsWrote,NULL)==TRUE && _charsWrote>=s.size(); }
                void Write(const char* pData,dword len) { _lastOutputSuccess = WriteFile(_hStdOut,pData,len,&_charsWrote,NULL)==TRUE && _charsWrote>=len; }
                void WriteLine(const str&);
                void WriteLine(const char* pData,dword len);
                void WriteErr(char c) { _lastOutputSuccess = WriteFile(_hStdErr,&c,1,&_charsWrote,NULL)==TRUE && _charsWrote>=1; }
                void WriteErr(const str& s) { _lastOutputSuccess = WriteFile(_hStdErr,&s[0],s.size(),&_charsWrote,NULL)==TRUE && _charsWrote>=s.size(); }
                void WriteErr(const char* pData,dword len) { _lastOutputSuccess = WriteFile(_hStdErr,pData,len,&_charsWrote,NULL)==TRUE && _charsWrote>=len; }
                void WriteLineErr(const str&);
                void WriteLineErr(const char* pData,dword len);
                char Read();
                str ReadLine();

                // operators/operations
                operator void* () const { return (void*) (_hStdOut!=INVALID_HANDLE_VALUE && _hStdErr!=INVALID_HANDLE_VALUE && _hStdIn!=INVALID_HANDLE_VALUE); }
                //      (redirect operators)
                Console& operator >>(HANDLE hOut);
                Console& operator <<(HANDLE hIn);
                void ReloadFromStdHandles();
                void RedirectError(HANDLE hErr) { _hStdErr = hErr; }
                void RedirectHandles(HANDLE hOut,HANDLE hIn,HANDLE hErr);

                // other
                // if true, the last operation was successful, or there was no preceding operation
                //  a successful operation is one where at least one character was read
                bool GetLastInputSuccess() const { return _lastInputSuccess; }
                bool GetLastOutputSuccess() const { return _lastOutputSuccess; }
                HANDLE GetStdOutputHandle() const { return _hStdOut; }
                HANDLE GetStdInputHandle() const { return _hStdIn; }
                HANDLE GetStdErrorHandle() const { return _hStdErr; }
            private:
                HANDLE  _hStdOut,
                        _hStdErr,
                        _hStdIn;
                DWORD _charsRead, _charsWrote;
                bool _lastInputSuccess, _lastOutputSuccess;
            };

            /*
                Note: unlike most rstreams, a ConsoleStream directs output to a
            different device than the one from which it gets input. This is wrapped in
            a Console object.

            */
            enum ConsoleStreamOutputFlag
            {
                StdOut,
                StdErr
            };

            class ConsoleStream : public rstream<Console>
            {
            public:
                ConsoleStream() : _outputFlag(StdOut),_iterChange(false) {}
                ConsoleStream(const char* DeviceID) : _outputFlag(StdOut),_iterChange(false) { open(DeviceID); }
                ConsoleStream(Console& Device) : _outputFlag(StdOut),_iterChange(false) { open(Device); }
                ~ConsoleStream() { if (!_isOwned()) flush_output(); }

                // Include a little low-level console functionality into the stream

                // introduce overloaded iterator methods to enhance rstream functionality
                //  to work with the two-dimensional buffer
                using rstream<Console>::set_output_iter; // (calling these won't work)
                using rstream<Console>::seek_output_iter;
                bool set_output_iter(dword Column,dword Row);
                bool set_output_iter(const Point& location) { return set_output_iter(location.x,location.y); }
                bool seek_output_iter(dword XAmount,dword YAmount,bool XIsNeg = false,bool YIsNeg = false);
                bool seek_output_iter(const Point& amount,bool XIsNeg = false,bool YIsNeg = false) { return seek_output_iter(amount.x,amount.y,XIsNeg,YIsNeg); }
            protected:
                virtual bool _openID(const char* p);
                virtual dword _deviceSize() const;
                virtual void _outDevice();
                virtual bool _inDevice() const;
                virtual void _clearDevice();
            private:
                mutable CONSOLE_SCREEN_BUFFER_INFO _info;
                ConsoleStreamOutputFlag _outputFlag;
                bool _iterChange; // if true, the iterator needs to be reset

                void _setCursorFromOIter();
                void _setOIterFromCursor();

                friend ConsoleStream& operator <<(ConsoleStream&,ConsoleStreamOutputFlag);
            };

            ConsoleStream& operator <<(ConsoleStream&,ConsoleStreamOutputFlag);

            extern ConsoleStream theConsole; // the rough equivilent of 'cout', 'cin', and 'cerr'
        }
    }
}

#endif