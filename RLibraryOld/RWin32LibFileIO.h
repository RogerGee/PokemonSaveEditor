//RWin32LibFileIO.h
#ifndef RWIN32LIBFILEIO_H
#define RWIN32LIBFILEIO_H

namespace rtypes
{
    namespace rwin32
    {
        namespace fileIO
        {
            const str WindowsEndLine("\r\n");
            enum FileAccess
            {
                NoAccess = 0, // read file attrib info
                GenericAll = GENERIC_READ|GENERIC_WRITE,
                GenericRead = GENERIC_READ,
                GenericWrite = GENERIC_WRITE
            };

            enum ShareAccess
            {
                FileNoShare = 0,
                FileDeleteShare = 4,
                FileReadShare = 1,
                FileWriteShare  = 2
            };

            // forward declarations
            class Directory;
            class File;

            struct FileAttributes
            {
                FileAttributes();
                // file attrib states - true==on, false==off
                bool normal() const // "no attributes set"
                { return !archived && !compressed && !isDirectory && !encrypted && !hidden && !readOnly && !systemFile && !temporaryFile; }
                bool archived,
                    compressed,
                    isDirectory,
                    encrypted,
                    hidden,
                    readOnly,
                    systemFile,
                    temporaryFile;
                // (these members are ignored when setting attribs, since they are controlled by the system)
                SystemTime createTime,lastAccessTime,lastWriteTime;
                qword fileSize;
            private:
                void _setFrom(const WIN32_FILE_ATTRIBUTE_DATA* attribs);
                void _getAttribs(WIN32_FILE_ATTRIBUTE_DATA* attribs) const;
                dword _getFileAttribs() const;

                friend class Directory;
                friend class File;
            };

            struct FileFlags
            {
                FileFlags();
                void ClearFlags();
                // these flags are used when creating a file
                bool flagAsynchronous,
                    flagDeleteOnClose,
                    flagNoBuffering,
                    flagRandomAccess,
                    flagSequentialScan,
                    flagWriteThrough;
            private:
                void _setFrom(dword);
                dword _getFileFlags() const;

                friend class Directory;
                friend class File;
            };

            class Directory
            {
            public:
                Directory();
                Directory(const str&);
                ~Directory();

                bool Open(const str& DirPath);

            private:
                HANDLE _hDir;
                str _pathOpened; // the string used to create the directory
            };

            class File
            {
            public:
                File();
                File(const str&);
                ~File();

                // file operations
                bool Open(FileAccess AccessRights = GenericAll, // use file name from this file object's parser
                    ShareAccess Sharing = FileNoShare,
                    bool OpenOnlyIfExists = false,
                    bool Overwrite = false,
                    const FileAttributes& Attribs = FileAttributes(),
                    const FileFlags& Flags = FileFlags());
                bool Open(const str&,
                    FileAccess AccessRights = GenericAll,
                    ShareAccess Sharing = FileNoShare,
                    bool OpenOnlyIfExists = false,
                    bool Overwrite = false,
                    const FileAttributes& Attribs = FileAttributes(),
                    const FileFlags& Flags = FileFlags());
                bool Close();
                bool Truncate(const qword& FileSize); // sets read ptr back to beginning, write ptr to end of new file
                bool Erase() { return Truncate(0); }
                // write ops - return true if data was written
                bool Write(byte);
                bool Write(const byte* PtrToData,dword Length);
                bool WriteString(const str&);
                bool WriteLine(const str& s) { return WriteString(s) && WriteString(WindowsEndLine); }
                bool WriteLine(const str& s,str CustomEndLine) { return WriteString(s) && WriteString(CustomEndLine); }
                // read ops - return true if data was read and copied
                bool Read(byte&);
                bool Read(byte* PtrToData,dword Length); // read an array of bytes, Length bytes long
                bool ReadString(str&,dword Length); // read Length bytes into string object s
                bool ReadToEnd(str&); // read from current location to end
                bool ReadAllBytes(str&); // read from beginning of file to end; doesn't seek read ptr

                // misc. methods
                bool FileReadState() const { return _readState; }
                bool CanRead() const { return _myAccess==GenericAll || _myAccess==GenericRead; }
                bool CanWrite() const { return _myAccess==GenericAll || _myAccess==GenericWrite; }

                // operators (note - this class doesn't provide a stream interface)
                operator void *() const { return (void*) (_hFile!=INVALID_HANDLE_VALUE); } // indicates whether a valid file is loaded
                File& operator <<(byte b) { Write(b); return *this; }
                File& operator <<(const str& s) { WriteString(s); return *this; }
                
                // "get" methods
                str GetOpenedFile() const { return _fileFromOpen; } // it's better to use this to get an opened file name
                str GetFilePathName_parser() const; // use this to get the fully qualified name of the parse
                str GetSafeName_parser(bool IncludeExtension = true) const; // file name with no path
                void GetFilePathNameComponents_parser(container<str>& Components) const;
                FileAttributes GetFileAttribs() const;
                qword GetFileSize() const;
                qword GetReadPointer() const { return _readPtr.QuadPart; }
                qword GetWritePointer() const { return _writePtr.QuadPart; }

                // "set" methods
                void SetCurrentDirectoryPath_parser() { fullPath = "."; } // specifies the current directory
                void SetCurrentDirectoryOnDrive_parser(char DriveLetter); // specifies the current directory on the specified drive
                void SetParentDirectory_parser() { fullPath = ".."; } // specifies the directory above the current directory
                void SetFilePathName_parser(const str&);
                void SetFileAttribs(const FileAttributes& Attribs);
                void SetReadPtr(qword Index) { _readPtr.QuadPart = Index; }
                void SetWritePtr(qword Index) { _writePtr.QuadPart = Index; }

                // these members simply help with parsing a file name
                // these are automatically set after a file's been opened
                str fileName;
                str extension;
                str fullPath;

                static bool IsValidName(const str&); // returns false if the string contains any illegal characters for names
            private:
                HANDLE _hFile;
                dword _flags;
                FileAccess _myAccess;
                str _fileFromOpen; // the filename we used to open the file
                LARGE_INTEGER _readPtr; // offset of next read operation
                LARGE_INTEGER _writePtr; // offset of next write operation
                DWORD _byteCntDummy;
                bool _readState;
                void _setPtr(PLARGE_INTEGER Ptr) { SetFilePointerEx(_hFile,*Ptr,NULL,FILE_BEGIN); }
            };

            class FileBuffer // fixed size buffer for file editing
            {
            public:
                FileBuffer();
                FileBuffer(const str& FileName) : _data(NULL) { LoadFromFile(FileName); }
                FileBuffer(const char* FileName) : _data(NULL) { LoadFromFile(FileName); }
                explicit FileBuffer(dword AllocSize) : _data(NULL),_sz(0) { Resize(AllocSize); }
                ~FileBuffer();
                
                // operations
                //      the following ops will overwrite pre-existing data
                bool LoadFromFile(const str& FileName);
                bool LoadFromFile(File& FileObj); // reads all data from file object
                void Resize(dword AllocSize);
                void Clear() { Resize(0); }
                //
                bool Save();
                bool Save(const str& NewFile); // save data in specified file
                bool Save(File& FileObj) { return FileObj.Write(_data,_sz); } // save data in pre-configured file object for appending/overwriting

                // operators
                byte& operator [](dword i) { return _data[i]; }
                const byte& operator [](dword i) const { return _data[i]; }
                //      operation for placement of binary sequences
                template<class numeric_integer>
                void operator ()(numeric_integer Data,dword Offset,/*rtypes::*/endianness Endianness = big) // start at offset, and fill in sizeof(numeric) bytes
                {
                    // use a binary string stream to convert
                    str s;
                    rbinstringstream bss(Endianness);
                    bss.open(s);
                    bss << Data;
                    bss.close();
                    for (dword i = 0;i<s.size();i++)
                        _data[i+Offset] = s[i];
                }
                
                // "get" methods
                dword GetBufferSize() const { return _sz; }
                str GetLoadedFile() const { return _loadedFile; }
                bool HasLoadedFile() const { return _loadedFile.length()>0; }

            private:
                str _loadedFile;
                byte* _data;
                dword _sz;
            };

            class FileStream : public rstream<File>
            {
            public:
                FileStream() {}
                FileStream(const char* DeviceID) { open(DeviceID); }
                FileStream(File& Device) { open(Device); }
                dword get_number_of_lines() const;
                ~FileStream() { if (!_isOwned()) flush_output(); }
            protected:
                virtual bool _openID(const char* p) { return _device->Open(p); }
                virtual dword _deviceSize() const { return (dword) _device->GetFileSize(); }
                virtual void _outDevice();
                virtual bool _inDevice() const;
                virtual void _clearDevice() { _device->Erase(); }
            };

            class FileStream_binary : public rbinstream<File>
            {
            public:
                // these c-strs implement big endian byte order
                FileStream_binary() : rbinstream<File>(little) {}
                FileStream_binary(const char* DeviceID) : rbinstream<File>(little) { open(DeviceID); }
                FileStream_binary(File& Device) : rbinstream<File>(little) { open(Device); }
                // provide a means of specifying endianess
                FileStream_binary(endianness E) : rbinstream<File>(E) {}
                ~FileStream_binary() { if (!_isOwned()) flush_output(); }
            protected:
                virtual bool _openID(const char* p) { return _device->Open(p); }
                virtual dword _deviceSize() const { return (dword) _device->GetFileSize(); }
                virtual void _outDevice();
                virtual bool _inDevice() const;
                virtual void _clearDevice() { _device->Erase(); }
            };

            // a FileBufferStream only comes in a binary version
            class FileBufferStream : public rbinstream<FileBuffer>
            {
            public:
                // these c-strs implement big endian byte order
                //  they also set the output device iter to zero, since appending on a FileBuffer is not allowed
                FileBufferStream() : rbinstream<FileBuffer>(little) {}
                FileBufferStream(const char* DeviceID);
                FileBufferStream(FileBuffer& Device);
                // provide a means of specifying endianess
                FileBufferStream(endianness E) : rbinstream<FileBuffer>(E) { _odeviceIter = 0; }
                ~FileBufferStream() { if (!_isOwned()) flush_output(); }
            protected:
                virtual bool _openID(const char* p) { return _device->LoadFromFile(p); }
                virtual dword _deviceSize() const { return _device->GetBufferSize(); }
                virtual void _outDevice();
                virtual bool _inDevice() const;
                virtual void _clearDevice() { _device->Clear(); }
            };
        }
    }
}

#endif