#include "RWin32LibMaster.h"
using namespace rtypes;
using namespace rtypes::rwin32;
using namespace rtypes::rwin32::fileIO;

FileAttributes::FileAttributes()
{
    archived = false;
    compressed = false;
    isDirectory = false;
    encrypted = false;
    hidden = false;
    readOnly = false;
    systemFile = false;
    temporaryFile = false;
    fileSize = 0;
}
void FileAttributes::_setFrom(const WIN32_FILE_ATTRIBUTE_DATA* attribs)
{
    // set file attributes
    archived = (attribs->dwFileAttributes&FILE_ATTRIBUTE_ARCHIVE) == FILE_ATTRIBUTE_ARCHIVE;
    compressed = (attribs->dwFileAttributes&FILE_ATTRIBUTE_COMPRESSED) == FILE_ATTRIBUTE_COMPRESSED;
    isDirectory = (attribs->dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_COMPRESSED;
    encrypted = (attribs->dwFileAttributes&FILE_ATTRIBUTE_ENCRYPTED) == FILE_ATTRIBUTE_ENCRYPTED;
    hidden = (attribs->dwFileAttributes&FILE_ATTRIBUTE_HIDDEN) == FILE_ATTRIBUTE_HIDDEN;
    readOnly = (attribs->dwFileAttributes&FILE_ATTRIBUTE_READONLY) == FILE_ATTRIBUTE_READONLY;
    systemFile = (attribs->dwFileAttributes&FILE_ATTRIBUTE_SYSTEM) == FILE_ATTRIBUTE_SYSTEM;
    temporaryFile = (attribs->dwFileAttributes&FILE_ATTRIBUTE_TEMPORARY) == FILE_ATTRIBUTE_TEMPORARY;
    // set times
    SYSTEMTIME holder;
    if (FileTimeToSystemTime(&attribs->ftCreationTime,&holder))
        createTime = holder;
    if (FileTimeToSystemTime(&attribs->ftLastAccessTime,&holder))
        lastAccessTime = holder;
    if (FileTimeToSystemTime(&attribs->ftLastWriteTime,&holder))
        lastWriteTime = holder;
    if (!isDirectory) // docs say that the data has "no meaning" for dirs
    {
        // set high word
        fileSize = ((qword) attribs->nFileSizeHigh << 32);
        fileSize |= attribs->nFileSizeLow;
    }
}
void FileAttributes::_getAttribs(WIN32_FILE_ATTRIBUTE_DATA* attribs) const
{// I probably won't use this one, but here it is
    if (normal())
        attribs->dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
    else
    {
        attribs->dwFileAttributes = NULL;
        attribs->dwFileAttributes |= archived ? FILE_ATTRIBUTE_ARCHIVE : NULL;
        attribs->dwFileAttributes |= compressed ? FILE_ATTRIBUTE_COMPRESSED : NULL;
        attribs->dwFileAttributes |= isDirectory ? FILE_ATTRIBUTE_DIRECTORY : NULL;
        attribs->dwFileAttributes |= encrypted ? FILE_ATTRIBUTE_ENCRYPTED : NULL;
        attribs->dwFileAttributes |= hidden ? FILE_ATTRIBUTE_HIDDEN : NULL;
        attribs->dwFileAttributes |= readOnly ? FILE_ATTRIBUTE_READONLY : NULL;
        attribs->dwFileAttributes |= systemFile ? FILE_ATTRIBUTE_SYSTEM : NULL;
        attribs->dwFileAttributes |= temporaryFile ? FILE_ATTRIBUTE_TEMPORARY : NULL;
    }
    // file times and file size are controlled by the system, so I don't include them
    // this function merely sets a WIN32_FILE_ATTRIBUTE_DATA struct with the attrib data
}
dword FileAttributes::_getFileAttribs() const
{
    if (normal())
        return FILE_ATTRIBUTE_NORMAL;
    dword r = NULL;
    r |= archived ? FILE_ATTRIBUTE_ARCHIVE : NULL;
    r |= compressed ? FILE_ATTRIBUTE_COMPRESSED : NULL;
    r |= isDirectory ? FILE_ATTRIBUTE_DIRECTORY : NULL;
    r |= encrypted ? FILE_ATTRIBUTE_ENCRYPTED : NULL;
    r |= hidden ? FILE_ATTRIBUTE_HIDDEN : NULL;
    r |= readOnly ? FILE_ATTRIBUTE_READONLY : NULL;
    r |= systemFile ? FILE_ATTRIBUTE_SYSTEM : NULL;
    r |= temporaryFile ? FILE_ATTRIBUTE_TEMPORARY : NULL;
    return r;
}

FileFlags::FileFlags()
{
    ClearFlags();
}
void FileFlags::ClearFlags()
{
    flagAsynchronous = false;
    flagDeleteOnClose = false;
    flagNoBuffering = false;
    flagRandomAccess = false;
    flagSequentialScan = false;
    flagWriteThrough = false;
}

void FileFlags::_setFrom(dword flags)
{
    flagAsynchronous = (flags&FILE_FLAG_OVERLAPPED)==FILE_FLAG_OVERLAPPED;
    flagDeleteOnClose = (flags&FILE_FLAG_DELETE_ON_CLOSE)==FILE_FLAG_DELETE_ON_CLOSE;
    flagNoBuffering = (flags&FILE_FLAG_NO_BUFFERING)==FILE_FLAG_NO_BUFFERING;
    flagRandomAccess = (flags&FILE_FLAG_RANDOM_ACCESS)==FILE_FLAG_RANDOM_ACCESS;
    flagSequentialScan = (flags&FILE_FLAG_SEQUENTIAL_SCAN)==FILE_FLAG_SEQUENTIAL_SCAN;
    flagWriteThrough = (flags&FILE_FLAG_WRITE_THROUGH)==FILE_FLAG_WRITE_THROUGH;
}
dword FileFlags::_getFileFlags() const
{
    dword flags = 0;
    if (flagAsynchronous)
        flags |= FILE_FLAG_OVERLAPPED;
    if (flagDeleteOnClose)
        flags |= FILE_FLAG_DELETE_ON_CLOSE;
    if (flagNoBuffering)
        flags |= FILE_FLAG_NO_BUFFERING;
    if (flagRandomAccess)
        flags |= FILE_FLAG_RANDOM_ACCESS;
    if (flagSequentialScan)
        flags |= FILE_FLAG_SEQUENTIAL_SCAN;
    if (flagWriteThrough)
        flags |= FILE_FLAG_WRITE_THROUGH;
    return flags;
}

bool File::IsValidName(const str& fileName)
{
    // only a name (directory, file, or extension) should be specified, not a full file path and name
    for (dword i = 0;i<fileName.size();i++)
    {
        switch (fileName[i])
        {
        case '<':
        case '>':
        case ':':
        case '\"':
        case '/':
        case '|':
        case '?':
        case '*':
            return false;
        }
        if (fileName[i]>=1 && fileName[i]<=31)
            return false;
    }
    return true;
}

File::File()
{
    _hFile = INVALID_HANDLE_VALUE;
    _flags = NULL;
    _readPtr.QuadPart = 0;
    _writePtr.QuadPart = 0;
    _myAccess = GenericAll;
    _byteCntDummy = 0;
    _readState = true;
    SetCurrentDirectoryPath_parser();
}
File::File(const str& FileName)
{
    _hFile = INVALID_HANDLE_VALUE;
    _flags = NULL;
    _readPtr.QuadPart = 0;
    _writePtr.QuadPart = 0;
    _myAccess = GenericAll;
    _byteCntDummy = 0;
    _readState = true;
    // open the file
    Open(FileName);
}
File::~File()
{
    if (_hFile!=INVALID_HANDLE_VALUE)
    {// close the file handle
        CloseHandle(_hFile);
    }
}
bool File::Open(FileAccess AccessRights,
                    ShareAccess Sharing,
                    bool OpenOnlyIfExists,
                    bool Overwrite,
                    const FileAttributes& Attribs,
                    const FileFlags& Flags)
{
    if (_hFile!=INVALID_HANDLE_VALUE)
        return false; // a file is already open
    // use the file path and name parts to construct a file name
    return Open( GetFilePathName_parser(),AccessRights,Sharing,OpenOnlyIfExists,Overwrite,Attribs,Flags );
}
bool File::Open(const str& FileName,
                    FileAccess AccessRights,
                    ShareAccess Sharing,
                    bool OpenOnlyIfExists,
                    bool Overwrite,
                    const FileAttributes& Attribs,
                    const FileFlags& Flags)
{
    if (_hFile!=INVALID_HANDLE_VALUE)
        return false; // a file is already open
    // check attribs
    //      make sure the user isn't passing a directory
    if (Attribs.isDirectory)
        return false; // maintain logical separation between files and directories
    DWORD creationDisposition;
    DWORD attribsFlags;
    // get file attribs
    attribsFlags = Attribs._getFileAttribs() | Flags._getFileFlags();
    // conclude correct creation disposition
    if (Overwrite)
        if (OpenOnlyIfExists)
            creationDisposition = TRUNCATE_EXISTING;
        else
            creationDisposition = CREATE_ALWAYS;
    else
        if (OpenOnlyIfExists)
            creationDisposition = OPEN_EXISTING;
        else
            creationDisposition = OPEN_ALWAYS; // default
    // create a new file on disk
    _hFile = CreateFile(FileName.c_str(),
            (DWORD) AccessRights,
            (DWORD) Sharing,
            NULL, // assign a default "security descriptor"
            creationDisposition,
            attribsFlags,
            NULL);
    // return file creation state
    if (_hFile!=INVALID_HANDLE_VALUE)
    {
        // assign full (final) file name associated with the open handle
        //      figure out the size needed
        dword sz = GetFinalPathNameByHandle(_hFile,NULL,NULL,FILE_NAME_NORMALIZED);
        _fileFromOpen = str(++sz); // allocate buffer; add 1 to account for the null terminator (Despite the docs, I think I need to add 1)
        sz = GetFinalPathNameByHandle(_hFile,&_fileFromOpen[0],sz,FILE_NAME_NORMALIZED);
        // these strings have a prefix (\\?\) that indicates a (potentially) long path name; cut it off
        if (sz) // sz will be non-zero if success
        {
            _fileFromOpen = &_fileFromOpen[4];
            // set file name to members
            SetFilePathName_parser(_fileFromOpen);
        }
        else
            // try with the user's name
            SetFilePathName_parser(FileName);
        // assign file pointers for read and write operations
        _readPtr.QuadPart = 0; // just in case
        _writePtr.QuadPart = Overwrite ? 0 : GetFileSize();
        // assign access for easy later reference
        _myAccess = AccessRights;
        // assign flags
        _flags = Flags._getFileFlags();
        return true;
    }
    return false;
}
bool File::Close()
{
    if (_hFile!=INVALID_HANDLE_VALUE)
    {// close the file handle
        _fileFromOpen.clear();
        if (CloseHandle(_hFile))
        {
            _hFile = INVALID_HANDLE_VALUE; // reset handle
            return true;
        }
    }
    return false;
}
bool File::Truncate(const qword& FileSize)
{
    if (_hFile==INVALID_HANDLE_VALUE || FileSize>=GetFileSize() || _myAccess==GenericRead)
        return false;
    LARGE_INTEGER holder;
    holder.QuadPart = FileSize;
    _setPtr(&holder); // move file pointer to truncate point
    if (SetEndOfFile(_hFile))
    {
        _writePtr.QuadPart = FileSize; // write at end of new file
        _readPtr.QuadPart = 0; // set reading back to the beginning
        return true;
    }
    return false;
}
bool File::Write(byte b)
{
    if (_hFile==INVALID_HANDLE_VALUE || _myAccess==GenericRead)
        return false;
    // set file pointer to current write location
    _setPtr(&_writePtr);
    // write file data, return success
    if (WriteFile(_hFile,&b,1,&_byteCntDummy,NULL))
    {
        // seek ptr
        _writePtr.QuadPart += _byteCntDummy; // to ensure if writting happened
        return true;
    }
    return false;
}
bool File::Write(const byte* PtrToData,dword Length)
{
    if (_hFile==INVALID_HANDLE_VALUE || _myAccess==GenericRead)
        return false;
    // set file pointer to current write location
    _setPtr(&_writePtr);
    // write data, return success
    if (WriteFile(_hFile,PtrToData,Length,&_byteCntDummy,NULL))
    {
        // seek ptr
        _writePtr.QuadPart += _byteCntDummy;
        return true;
    }
    return false;
}
bool File::WriteString(const str& s)
{
    if (_hFile==INVALID_HANDLE_VALUE || _myAccess==GenericRead)
        return false;
    // set file pointer to current write location
    _setPtr(&_writePtr);
    // write data, return success
    if (WriteFile(_hFile,s.c_str(),s.size(),&_byteCntDummy,NULL))
    {
        // seek ptr
        _writePtr.QuadPart += _byteCntDummy;
        return true;
    }
    return false;
}
bool File::Read(byte& b)
{
    if (_hFile==INVALID_HANDLE_VALUE || _myAccess==GenericWrite)
        return false;
    // set file pointer to current read location
    _setPtr(&_readPtr);
    // read data, return success
    _readState = false;
    if (ReadFile(_hFile,&b,1,&_byteCntDummy,NULL))
    {
        if (!_byteCntDummy)
            return false;
        // seek ptr
        _readPtr.QuadPart += _byteCntDummy;
        _readState = true;
    }
    return _readState;
}
bool File::Read(byte* PtrToData,dword Length)
{
    if (_hFile==INVALID_HANDLE_VALUE || _myAccess==GenericWrite || !Length)
        return false;
    // set file pointer to current read location
    _setPtr(&_readPtr);
    // read data, return success
    _readState = false;
    if (ReadFile(_hFile,PtrToData,Length,&_byteCntDummy,NULL))
    {
        if (!_byteCntDummy)
            return false;
        // seek ptr
        _readPtr.QuadPart += _byteCntDummy;
        _readState = true;
    }
    return _readState;
}
bool File::ReadString(str& s,dword Length)
{
    if (_hFile==INVALID_HANDLE_VALUE || _myAccess==GenericWrite || !Length)
        return false;
    // set file pointer to current read location
    _setPtr(&_readPtr);
    // allocate enough space in s to buffer input
    s.resize(Length);
    // read data, return success
    _readState = false;
    if (ReadFile(_hFile,&s[0],Length,&_byteCntDummy,NULL))
    {
        if (!_byteCntDummy)
            return false;
        // truncate string to appropriate size
        s.truncate(_byteCntDummy);
        // seek ptr
        _readPtr.QuadPart += _byteCntDummy;
        _readState = true;
    }
    return _readState;
}
bool File::ReadToEnd(str& s)
{// from current location to end
    if (_hFile==INVALID_HANDLE_VALUE || _myAccess==GenericWrite)
        return false;
    dword length = (dword) (GetFileSize()-_readPtr.QuadPart);
    // allocate enough space in s
    s.resize(length);
    // set pointers to current location
    _setPtr(&_readPtr);
    // read data, return success
    _readState = false;
    if (ReadFile(_hFile,&s[0],length,&_byteCntDummy,NULL))
    {
        if (!_byteCntDummy)
            return false;
        _readPtr.QuadPart += _byteCntDummy;
        _readState = true;
    }
    return _readState;
}
bool File::ReadAllBytes(str& s)
{
    if (_hFile==INVALID_HANDLE_VALUE || _myAccess==GenericWrite)
        return false;
    dword length = (dword) GetFileSize();
    // allocate enough space in s
    s.resize(length);
    // set pointer to beginning of file
    _readPtr.QuadPart = 0; _setPtr(&_readPtr);
    // read data, return success
    _readState = false;
    if (ReadFile(_hFile,&s[0],length,&_byteCntDummy,NULL))
    {
        if (!_byteCntDummy)
            return false;
        _readPtr.QuadPart += _byteCntDummy;
        _readState = true;
    }
    return _readState;
}
str File::GetFilePathName_parser() const
{
    str file;
    if (fullPath.size())
    {
        file += fullPath;
        if (fullPath[fullPath.size()-1]!='\\' && fullPath[fullPath.size()-1]!=':') // a colon indicates the current directory; don't add a backslash in order to maintain this
            file += '\\';
    }
    file += fileName;
    if (extension.size())
    {
        if (extension[0]!='.')
            file += '.';
        file += extension;
    }
    return file;
}
str File::GetSafeName_parser(bool IncludeExtension) const
{
    str file = fileName;
    if (IncludeExtension && extension.size())
    {
        if (extension[0]!='.')
            file.push_back('.');
        file += extension;
    }
    return file;
}
void File::GetFilePathNameComponents_parser(container<str> &Components) const
{
    // "components" includes directories and the filename with extension
    str part;
    if (fullPath.size()>0)
    {
        for (dword i = 0;i<fullPath.size()+1;i++)
        {
            if (i>=fullPath.size() || fullPath[i]=='\\')
            {
                if (part.size()==0 && i==0 || (part.size()>0 && part[part.size()-1]==':' && fullPath[i]=='\\'))
                    part.push_back('\\'); // relative root directory OR requires a '\\' because it's a drive spec.
                else if (part.size()==0)
                    continue; // bad file spec. caused this (or a server name)
                Components.push_back(part);
                part.clear();
                continue;
            }
            part.push_back(fullPath[i]);
        }
    }
    Components.push_back( GetSafeName_parser() );
}
FileAttributes File::GetFileAttribs() const
{
    FileAttributes f;
    if (_hFile==INVALID_HANDLE_VALUE)
        return f;
    WIN32_FILE_ATTRIBUTE_DATA data;
    if (GetFileAttributesEx(_fileFromOpen.c_str(),GetFileExInfoStandard,&data))
        f._setFrom(&data);
    return f;
}
qword File::GetFileSize() const
{
    if (_hFile==INVALID_HANDLE_VALUE)
        return 0;
    DWORD low,high;
    low = ::GetFileSize(_hFile,&high);
    return ((qword) high<<32) + low;
}
void File::SetCurrentDirectoryOnDrive_parser(char DriveLetter)
{
    // C:dirName
    fullPath.clear();
    fullPath.push_back(DriveLetter);
    fullPath.push_back(':');
}
void File::SetFilePathName_parser(const str& file)
{
    dword i, j;
    // clear any old data
    fullPath.clear();
    fileName.clear();
    extension.clear();
    // get the file name and extension
    for (i = file.size()-1;i>0 && file[i]!='\\' && file[i]!=':';i--);
    for (dword iter = (file[i]!='\\' && file[i]!=':' ? i : i+1); // add 1 to move past '\\' or ':' (if there is a '\\' or ':')
            iter<file.size();
                iter++)
        fileName.push_back(file[iter]);
    // separate out the extension from the file name
    for (j = fileName.size()-1;j>0 && fileName[j]!='.';j--);
    if (fileName[j]=='.')
    {// the file has an extension
        for (dword iter = j+1; // add 1 to move past '.'
                iter<fileName.size();
                    iter++)
            extension.push_back(fileName[iter]);
        fileName.truncate(j); // cut off extension from file name
    }
    // the rest of the string 'file' is the full (or relative) path
    if (file[0]=='\\' // special case: current drive root directory short-hand
        || file[1]==':')
        i++; // move past it so as to include the '\\' or ':'
    for (dword iter = 0;iter<i;iter++) // "iter<1" to exclude '\\'
        fullPath.push_back(file[iter]);
}
void File::SetFileAttribs(const FileAttributes& Attribs)
{
    if (_hFile==INVALID_HANDLE_VALUE)
        return;
    SetFileAttributes(_fileFromOpen.c_str(),Attribs._getFileAttribs());
}

FileBuffer::FileBuffer()
{
    _data = 0;
    _sz = 0;
}
FileBuffer::~FileBuffer()
{
    delete[] _data;
}
bool FileBuffer::LoadFromFile(const str& FileName)
{
    File fileObj;
    fileObj.Open(FileName,GenericAll,FileNoShare,true);
    if (!fileObj)
        return false;
    Resize((dword) fileObj.GetFileSize());
    if (!fileObj.Read(_data,_sz))
    {
        Clear();
        return false;
    }
    _loadedFile = fileObj.GetOpenedFile();
    return true;
}
bool FileBuffer::LoadFromFile(File& FileObj)
{
    if (!FileObj)
        return false;
    Resize((dword) FileObj.GetFileSize());
    if (!FileObj.Read(_data,_sz))
    {
        Clear();
        return false;
    }
    _loadedFile = FileObj.GetOpenedFile();
    return true;
}
void FileBuffer::Resize(dword AllocSize)
{
    delete[] _data;
    _sz = AllocSize;
    if (_sz)
        _data = new byte[_sz];
    else
        _data = 0;
}
bool FileBuffer::Save()
{
    File fileObj(_loadedFile);
    return fileObj.Write(_data,_sz);
}
bool FileBuffer::Save(const str& NewFile)
{
    File fileObj(NewFile);
    return fileObj.Write(_data,_sz);
}

dword FileStream::get_number_of_lines() const
{
    FileStream& thisNonConst = const_cast<FileStream&> (*this);
    str dummy;
    dword oldIter = thisNonConst.get_input_iter_offset();
    dword lineCnt = 0;
    while (has_input())
    {
        thisNonConst.getline(dummy);
        lineCnt++;
    }
    thisNonConst.set_input_iter(oldIter);
    return lineCnt;
}
void FileStream::_outDevice()
{
    // route all data from the output buffer to the File object
    // set write ptr
    if (_odeviceIter>_deviceSize())
        _odeviceIter = _deviceSize();
    _device->SetWritePtr(_odeviceIter);
    while (!_bufOut.is_empty())
    {
        dword len = 0;
        const char* data = &_bufOut.peek(); // this is defined by rtypes::queue
        while (len<_bufOut.size() && data[len]!='\n')
            len++;
        if (len>0)
        {
            if (!_device->Write((const byte*) data,len))
                return; // keep data in buffer
            _odeviceIter += len; // seek stream iterator
        }
        if (len<_bufOut.size() && data[len]=='\n')
        {
            if (!_device->WriteString("\r\n")) // replace \n with \r\n
                return; // keep data in buffer
            _odeviceIter += 2; // seek stream iterator
            len++; // increment this so that the newline is dequeued
        }
        _bufOut.pop_range(len);
    }
    _resetBufOut(); // we don't need this data any more
}
bool FileStream::_inDevice() const
{// return the state of input from the device
    if (_ideviceIter<_deviceSize())
    {
        // put 8 dwords into the input buffer
        str s;
        _device->SetReadPtr(_ideviceIter); // just in case
        if (!_device->ReadString(s,32))
            return false;
        for (dword i = 0;i<s.length();i++,_ideviceIter++)
        {
            if (s[i]=='\r')
                continue; // strip '\r''s out of newlines
            _bufIn.push_back( s[i] );
        }
        return true;
    }
    return false; // no more input
}

void FileStream_binary::_outDevice()
{
    // route all data from the output buffer to the File object
    if (_odeviceIter>_deviceSize())
        _odeviceIter = _deviceSize();
    _device->SetWritePtr(_odeviceIter); // set write pos
    // write data
    //for (dword i = 0;i<_bufOut.size();i++,_odeviceIter++)
    //  if (!_device->Write( _bufOut[i] ))
    //  {// if 1 fails then following ones might
    //      return; // keep output in buffer
    //  }
    dword len = _bufOut.size();
    if (_device->Write( (const byte*) &_bufOut.peek(),len ))
    {
        _resetBufOut(); // we don't need this data any more
        _bufOut.pop_range(len);
        _odeviceIter += len;
    }
}
bool FileStream_binary::_inDevice() const
{// return the state of input from the device
    if (_ideviceIter<_deviceSize())
    {
        // put 8 dwords into the input buffer
        str s;
        _device->SetReadPtr(_ideviceIter); // just in case
        if (!_device->ReadString(s,32))
            return false;
        for (dword i = 0;i<s.length();i++,_ideviceIter++)
            _bufIn.push_back( s[i] );
        return true;
    }
    return false; // no more input
}

FileBufferStream::FileBufferStream(const char* DeviceID)
    : rbinstream<FileBuffer>(little)
{
    open(DeviceID);
    _odeviceIter = 0;
}
FileBufferStream::FileBufferStream(FileBuffer& Device)
    : rbinstream<FileBuffer>(little)
{
    open(Device);
    _odeviceIter = 0;
}
void FileBufferStream::_outDevice()
{
    if (_odeviceIter>=_deviceSize())
        return; // there is no appending in a file buffer
    FileBuffer* fileBuf = &get_device();
    while (!_bufOut.is_empty() && _odeviceIter<_deviceSize())
        (*fileBuf)[_odeviceIter++] = (byte) _bufOut.pop_back();
    _resetBufOut();
}
bool FileBufferStream::_inDevice() const
{
    if (_ideviceIter>=_deviceSize())
        return false; // no input
    // put 8 dwords (32 bytes) into the input buffer (if possible)
    const FileBuffer* fileBuf = &get_device();
    for (dword i = 0;i<32;i++)
    {
        _bufIn.push_back( (*fileBuf)[_ideviceIter++] );
        if (_ideviceIter>=_deviceSize())
            break;
    }
    return true;
}