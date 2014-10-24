#include "MasterInclude.h"
using namespace rtypes;
using namespace rwin32::fileIO;
using namespace PokemonEditor;

bool PokemonEditor::operator ==(const PokemonEntry& left,const PokemonEntry& right)
{
    // compare PokemonEntry objects
    return left.offset == right.offset;
}

// I/O functions for PokemonSaveFile and its sub-types, PokemonEntry
FileBufferStream& PokemonEditor::operator <<(FileBufferStream& stream,PokemonSaveFile& saveFile)
{
    saveFile.SaveTo(stream);
    return stream;
}
FileBufferStream& PokemonEditor::operator >>(FileBufferStream& stream,PokemonEntry& entry)
{
    // the procedure that called this one should set the pokemon object's entry flag
    entry.state = UndeterminedState;
    entry.offset = stream.get_input_iter_offset();
    stream >> entry.pok;
    return stream;
}

// member functions of type PokemonSaveFile::_Block
void PokemonSaveFile::_Block::output(FileBufferStream& binStream)
{
    FileBuffer& buffer = binStream.get_device();
    dword i = binStream.get_output_iter_offset();
    if (buffer.GetBufferSize()<SAVE_BLOCK_SIZE+i)
        return; // can't fit block into buffer at specified index
    // add data segment
    for (word cnt = 0;cnt<SAVE_BLOCK_DATA_SIZE;cnt++,i++)
        buffer[i] = data[cnt];
    // add padding
    for (byte cnt = 0;cnt<116;cnt++,i++)
        buffer[i] = 0;
    binStream.set_output_iter(i);
    binStream << blockID << padding << checksum << validation << saveID;
}
bool PokemonSaveFile::_Block::input(FileStream_binary& fileStream,bool performChecksumValidation)
{
    File& saveFile = fileStream.get_device();
    locationOffset = fileStream.get_input_iter_offset();
    saveFile.SetReadPtr(locationOffset);
    if (!saveFile.Read(data,SAVE_BLOCK_DATA_SIZE))
        return false;
    fileStream.seek_input_iter(4084); // move past data and padding
    // read footer
    fileStream >> blockID >> padding >> checksum
        >> validation >> saveID;
    // perform some simple validation on the block to avoid potential errors
    dword check = checksum;
    calculate_checksum();
    if ((check!=checksum&&performChecksumValidation) || blockID>13 || padding!=0 || (validation!=SAVE_VALIDATION && validation!=0)
        || fileStream==0/* if zero, the last input operation was unsuccessful */)
        return false;
    return true;
}
void PokemonSaveFile::_Block::calculate_checksum()
{
    /*
        The checksum for a save block is somewhat less than
        straight-forward. It is the sum of the high and low words
        of a 32-bit sum taken of all dwords within the data section.
    */
    dword dSum = 0, i = 0;
    while (i<SAVE_BLOCK_DATA_SIZE)
    {
        dword part;
        rbinstringstream bss;
        for (byte c = 0;c<4;c++,i++)
            bss.put(data[i]);
        bss >> part;
        dSum += part;
    }
    checksum = word(dSum) + word(dSum>>16);
}

// These comparison operators are defined such that blocks are ordered by ID
// and by save ID. The greatest save id takes precedence and should go with the
// "lower ordered" blocks; this way the current save is always ordered first.
bool PokemonSaveFile::_Block::operator <(const PokemonSaveFile::_Block& right)
{
    return (blockID < right.blockID && saveID >= right.saveID) || saveID > right.saveID || (validation==SAVE_VALIDATION && right.validation!=SAVE_VALIDATION);
}
bool PokemonSaveFile::_Block::operator >(const PokemonSaveFile::_Block& right)
{
    return (blockID > right.blockID && saveID <= right.saveID) || saveID < right.saveID || validation!=SAVE_VALIDATION;
}
bool PokemonSaveFile::_Block::operator ==(const PokemonSaveFile::_Block& right)
{
    return blockID == right.blockID && saveID == right.saveID && validation==right.validation;
}

const dword PokemonSaveFile::BUFFER_SIZE = SAVE_BLOCK_DATA_SIZE*14;
PokemonSaveFile::PokemonSaveFile()
{
    _gameFrom = DemoJirachiDisk; // used as a null value
    _gameFromVerified = false;
    _nextIter = 0;
    _currentSave.Resize(BUFFER_SIZE);
    _previousSave.Resize(BUFFER_SIZE);
    _currentSave[0] = 0xff;
    _previousSave[0] = 0xff;
    _altered = false;
}
PokemonSaveFile::LoadOperationReturn PokemonSaveFile::LoadFrom(const str& file,PokemonGameFromFlag gameFrom,bool SkipChecksumValidation)
{
    File saveFile;
    FileStream_binary fileStream;
    if (!saveFile.Open(file,
            GenericRead,
            FileReadShare,
            true))
        return LoadFailFileNotOpened;
    fileStream.open(saveFile);
    _blocks.empty();
    for (dword i = 0, rm = 0;i<32;i++)
    {
        _blocks.add();
        _Block& block = _blocks[i-rm];
        if (!block.input(fileStream,!SkipChecksumValidation))
        {
            /*
                Unfortunately, the current block couldn't be loaded properly; there are
                several reasons why this could be:
                    1.) the file isn't really a Pokemon save file and it failed validation
                    2.) the file is truncated, which some emulators unfortunately do; 3rd gen.
                        games should be saved with 128 KB of data, yet sometimes the second
                        memory bank isn't saved resulting in a 64 KB file.
                Solution: if the file is bad, we inform the caller by returning false; if the file 
                has usable data, we take what we can get, assume its the current save, and manufacture a copy
                and call it the previous save (decrementing the save ID).
            */
            if (i>=14) // we've read at least enough to have a state
            {
                for (dword j = 0;j<14;j++)
                {
                    _Block b = _blocks[j];
                    if (b.saveID==0)
                        b.saveID = 1;
                    else
                        b.saveID--;
                    b.locationOffset += 57344; // move to the other side of the battery file
                    _blocks.add(b);
                }
                break;
            }
            return LoadFailBadValidation;
        }
    }
    _blocks.quicksort();
    // load data into buffers
    _loadBuffers();
    // determine the game from (approximately) unless specified
    if (gameFrom==DemoJirachiDisk)
        _determineGameFrom();
    else
    {
        _gameFrom = gameFrom;
        _gameFromVerified = true; // an assumption
    }
    _altered = false;
    return LoadSuccess;
}
bool PokemonSaveFile::SaveTo(const str& file,bool saveReadOnly)
{
    File f;
    FileBuffer saveFile(SAVE_FILE_SIZE);
    FileBufferStream fileBufStream(saveFile);
    SaveTo(fileBufStream);
    if (!f.Open(file,GenericAll,FileNoShare,false,true)) // ensure overwritting
        return false;
    if (saveReadOnly)
    {
        FileAttributes attribs = f.GetFileAttribs();
        attribs.readOnly = true;
        f.SetFileAttribs(attribs);
    }
    if (saveFile.Save(f))
    {
        _altered = false;
        return true;
    }
    return false;
}
void PokemonSaveFile::SaveTo(FileBufferStream& stream)
{
    FileBuffer& buf = stream.get_device();
    if (buf.GetBufferSize()<SAVE_FILE_SIZE)
        buf.Resize(SAVE_FILE_SIZE);
    stream.set_output_iter(0x00); // set iterator to the beginning for accuracy
    // repack the save file
    _loadBlocks();
    while (stream.get_output_iter_offset()<SAVE_FILE_SIZE)
    {
        dword ind = _getBlockWithOffset(stream.get_output_iter_offset());
        if (ind>=_blocks.size())
            break; // just in case
        _blocks[ind].output(stream);
    }
}
void PokemonSaveFile::Clear()
{
    // the whole point of this function is to persist the object
    // but lower memory usage (which really isn't a problem)
    _currentSave.Clear();
    _previousSave.Clear();
    _blocks.empty();
    ResetEntries();
    _gameFrom = DemoJirachiDisk;
    _altered = false;
}
bool PokemonSaveFile::SaveCombinedMemoryFile(const str& file,bool saveReadOnly)
{
    File saveFile;
    if (!saveFile.Open(file,
        GenericAll,
        FileNoShare,
        false,
        true))
        return false;
    if (saveReadOnly)
    {
        FileAttributes attribs = saveFile.GetFileAttribs();
        attribs.readOnly = true;
        saveFile.SetFileAttribs(attribs);
    }
    return _currentSave.Save(saveFile) && _previousSave.Save(saveFile);
}
void PokemonSaveFile::SaveCombinedMemoryFile(FileBufferStream& stream,bool allocateBuffer)
{
    if (allocateBuffer)
    {
        FileBuffer& buffer = stream.get_device();
        if (buffer.GetBufferSize()<BUFFER_SIZE)
            buffer.Resize(BUFFER_SIZE);
        stream.set_output_iter(0);
    }
    SaveCurrentMemoryFile(stream);
    SavePreviousMemoryFile(stream);
}
bool PokemonSaveFile::SaveCurrentMemoryFile(const str& file,bool saveReadOnly)
{
    File saveFile;
    if (!saveFile.Open(file,
        GenericAll,
        FileNoShare,
        false,
        true))
        return false;
    if (saveReadOnly)
    {
        FileAttributes attribs = saveFile.GetFileAttribs();
        attribs.readOnly = true;
        saveFile.SetFileAttribs(attribs);
    }
    return _currentSave.Save(saveFile);
}
void PokemonSaveFile::SaveCurrentMemoryFile(FileBufferStream& stream,bool allocateBuffer)
{
    if (allocateBuffer)
    {
        FileBuffer& buffer = stream.get_device();
        if (buffer.GetBufferSize()<BUFFER_SIZE)
            buffer.Resize(BUFFER_SIZE);
        stream.set_output_iter(0);
    }
    bool state = stream.is_buffering_output();
    if (!state)
        stream.start_buffering_output();
    for (dword i = 0;i<_currentSave.GetBufferSize();i++)
        stream.put(_currentSave[i]);
    stream.stop_buffering_output(); // flush
    if (state) // preserve state
        stream.start_buffering_output();
}
bool PokemonSaveFile::SavePreviousMemoryFile(const str& file,bool saveReadOnly)
{
    File saveFile;
    if (!saveFile.Open(file,
        GenericAll,
        FileNoShare,
        false,
        true))
        return false;
    if (saveReadOnly)
    {
        FileAttributes attribs = saveFile.GetFileAttribs();
        attribs.readOnly = true;
        saveFile.SetFileAttribs(attribs);
    }
    return _previousSave.Save(saveFile);
}
void PokemonSaveFile::SavePreviousMemoryFile(FileBufferStream& stream,bool allocateBuffer)
{
    bool state = stream.is_buffering_output();
    if (!state)
        stream.start_buffering_output();
    for (dword i = 0;i<_previousSave.GetBufferSize();i++)
        stream.put(_previousSave[i]);
    stream.stop_buffering_output(); // flush
    if (state) // preserve state
        stream.start_buffering_output();
}
str PokemonSaveFile::GetShallowPokemonEntry(PokemonEntryType entryType,SaveFileState state,dword occuranceIndex) const
{
    str entry;
    dword offset;
    FileBufferStream stream;
    PokemonSaveFile* _this = const_cast<PokemonSaveFile*> (this);
    // open a stream to the save data
    if (state==CurrentState)
        stream.open(_this->_currentSave);
    else if (state==PreviousState)
        stream.open(_this->_previousSave);
    offset = _getStartOffsetFor(entryType,occuranceIndex);
    if (offset>0)
    {
        Pokemon p;
        p.SetEntryType(entryType);
        stream.set_input_iter(offset);
        stream >> p;
        entry = "POK=";
        if (p.GetEntryType()==NullPok)
            entry += "NullPokemon";
        else if (p.GetEntryType()==ErrorPok)
            entry += "BadPokemon";
        else
            entry += p.GetNickname().to_string();
        entry += " - SaveState=";
        entry += (state==CurrentState ? "Current" : "Previous");
    }
    return entry;
}
PokemonSaveFile::EntryOperationReturn PokemonSaveFile::GetPokemonEntry(PokemonEntry& entryObject,PokemonEntryType entryType,SaveFileState state,dword occuranceIndex,bool flagUnsaved) const
{
    FileBufferStream stream;
    PokemonSaveFile* _this = const_cast<PokemonSaveFile*> (this);
    // open a stream to the save data
    if (state==CurrentState)
        stream.open(_this->_currentSave);
    else if (state==PreviousState)
        stream.open(_this->_previousSave);
    entryObject.pok.SetEntryType(entryType); // set this for proper IO later on 
    entryObject.offset = _getStartOffsetFor(entryType,occuranceIndex);
    if (entryObject.offset==0)
        return EntryUnobtainable;
    if ( _outstandingEntryOffsets.contains(entryObject.offset) )
        return EntryOutstanding;
    _outstandingEntryOffsets.insert(entryObject.offset);
    stream.set_input_iter(entryObject.offset);
    stream >> entryObject.pok;
    entryObject.state = state;
    if (flagUnsaved)
        _altered = true;
    return EntryObtained;
}
PokemonEntry PokemonSaveFile::GetNextPokemonEntry(SaveFileState stateFrom,bool &passedStart_out,bool flagUnsaved) const
{
    FileBufferStream stream;
    PokemonSaveFile* _this = const_cast<PokemonSaveFile*> (this);
    if (stateFrom==CurrentState)
        stream.open(_this->_currentSave);
    else if (stateFrom==PreviousState)
        stream.open(_this->_previousSave);
    stream.set_input_iter(_nextIter);
    if (!stream.has_input())
    {
        _nextIter = 0;
        stream.set_input_iter(_nextIter);
    }
    while (stream.has_input())
    {
        PokemonEntry entry;
        entry.offset = stream.get_input_iter_offset();
        stream >> entry.pok;
        if (entry.pok.GetEntryType()!=ErrorPok)
        {
            _nextIter = stream.get_input_iter_offset();
            if ( !_outstandingEntryOffsets.contains(entry.offset) )
            {
                _outstandingEntryOffsets.insert(entry.offset);
                entry.state = stateFrom;
                passedStart_out = false;
                return entry;
            }
        }
        stream.set_input_iter(entry.offset+1);
    }
    _nextIter = stream.get_input_iter_offset();
    passedStart_out = true;
    if (flagUnsaved)
        _altered = true;
    return PokemonEntry();
}
bool PokemonSaveFile::PutPokemonEntry(const PokemonEntry& entryObject,bool flagUnsaved)
{
    if ( !_outstandingEntryOffsets.contains(entryObject.offset) )
        return false;
    FileBufferStream stream;
    if (entryObject.state==CurrentState)
        stream.open(_currentSave);
    else if (entryObject.state==PreviousState)
        stream.open(_previousSave);
    stream.set_output_iter(entryObject.offset);
    stream << entryObject.pok;
    _outstandingEntryOffsets.remove(entryObject.offset);
    if (flagUnsaved)
        _altered = true;
    return true;
}
rbinstringstream PokemonSaveFile::ToBinaryStream(SaveFileState SaveState) const
{
    rbinstringstream bss;
    const FileBuffer* stateData;
    if (SaveState==CurrentState)
        stateData = &_currentSave;
    else if (SaveState==PreviousState)
        stateData = &_previousSave;
    else
        return bss;
    for (dword i = 0;i<stateData->GetBufferSize();i++)
        bss.put( stateData->operator[](i) );
    return bss;
}
PokemonGameFromFlag PokemonSaveFile::GetGameFrom(bool& IsConclusive_out) const
{
    IsConclusive_out = _gameFromVerified;
    return _gameFrom;
}
Gender PokemonSaveFile::GetTrainerGender() const
{
    if (_currentSave[0]!=0xff)
        return _currentSave[0x8]!=0 ? Female : Male;
    return Unknown;
}
pok_string PokemonSaveFile::GetTrainerName() const
{
    // the trainer name is the first 7 bytes of the ordered memory file
    FileBufferStream stream( const_cast<PokemonSaveFile*> (this)->_currentSave);
    pok_string name;
    if (stream.peek()==-1)
        return name;
    for (byte b = 0;b<10;b++)
    {
        pok_char c;
        c.theChar = stream.get();
        name.push_back(c);
    }
    return name;
}
word PokemonSaveFile::GetTrainerRegularID() const
{
    FileBufferStream stream( const_cast<PokemonSaveFile*> (this)->_currentSave);
    word id = 0;
    if (stream.peek()==-1)
        return id;
    stream.set_input_iter(0xa);
    stream >> id;
    return id;
}
word PokemonSaveFile::GetTrainerSecretID() const
{
    FileBufferStream stream( const_cast<PokemonSaveFile*> (this)->_currentSave);
    word id = 0;
    if (stream.peek()==-1)
        return id;
    stream.set_input_iter(0xc);
    stream >> id;
    return id;
}
void PokemonSaveFile::SetTrainerGender(Gender g)
{
    if (g==Male)
        _currentSave[0x8] = 0;
    else
        _currentSave[0x8] = 1;
    _altered = true;
}
dword PokemonSaveFile::_getBlockWithOffset(dword offset) const
{
    // have to linear search through the blocks
    for (dword i = 0;i<_blocks.size();i++)
        if (_blocks[i].locationOffset==offset)
            return i;
    return _blocks.size();
}
void PokemonSaveFile::_loadBlocks()
{
    // copy data from save buffers into blocks
    // update each block's checksum
    //  copy current file
    dword i = 0;
    while (i<_currentSave.GetBufferSize())
    {
        dword index = i/SAVE_BLOCK_DATA_SIZE;
        if (index>=_blocks.size())
            break;
        _Block& block = _blocks[index];
        for (dword cnt = 0;cnt<SAVE_BLOCK_DATA_SIZE;cnt++,i++)
            block.data[cnt] = _currentSave[i];
        block.calculate_checksum();
    }
    //  copy previous file
    i = 0;
    while (i<_previousSave.GetBufferSize())
    {
        dword index = i/SAVE_BLOCK_DATA_SIZE+14;
        if (index>=_blocks.size())
            break;
        _Block& block = _blocks[index];
        for (dword cnt = 0;cnt<SAVE_BLOCK_DATA_SIZE;cnt++,i++)
            block.data[cnt] = _previousSave[i];
        block.calculate_checksum();
    }
}
void PokemonSaveFile::_loadBuffers()
{
    if (_blocks.size()<28)
        return; // not enough data in block list to fill buffers
    // resize save buffers to 55552 bytes each (just in case they've changed; the constructor did this earlier)
    if (_currentSave.GetBufferSize()!=BUFFER_SIZE)
        _currentSave.Resize(BUFFER_SIZE);
    if (_previousSave.GetBufferSize()!=BUFFER_SIZE)
        _previousSave.Resize(BUFFER_SIZE);
    // copy data into current and previous save buffers
    for (dword i = 0,offset = 0;i<14;i++) 
    {
        for (dword cnt = 0;cnt<SAVE_BLOCK_DATA_SIZE;cnt++,offset++)
        {
            _currentSave[offset] = _blocks[i].data[cnt];
            _previousSave[offset] = _blocks[i+14].data[cnt];
        }
    }
}
void PokemonSaveFile::_determineGameFrom()
{
    dword cnt;
    Pokemon dummy(PartyPok);
    FileBufferStream testStream(_currentSave);
    // test for Fire and Leaf
    testStream.set_input_iter(0xfb4);
    testStream >> cnt;
    if (cnt<=6 && cnt>0) // party count check
    {
        testStream >> dummy;
        if (dummy.GetEntryType()!=ErrorPok)
        {
            _gameFrom = PokemonFireRed; // this works for both fire-red and leaf-green
            _gameFromVerified = true;
            return;
        }
    }
    // test for Emerald, Ruby, and Sapphire
    testStream.set_input_iter(0x11b4);
    testStream >> cnt;
    if (cnt<=6 && cnt>0)
    {
        dummy.SetEntryType(PartyPok);
        testStream >> dummy;
        if (dummy.GetEntryType()!=ErrorPok)
        {
            _gameFrom = PokemonEmerald; // this will cover all party poks in this set of games
            testStream.set_input_iter(0x3fb0);
            // test for day care poks to deduce whether this is (Ruby OR Sapphire) OR Emerald
            dummy.SetEntryType(DayCarePok);
            testStream >> dummy;
            if (dummy.GetEntryType()==ErrorPok)
            {
                testStream.set_input_iter(0x3f1c);
                dummy.SetEntryType(DayCarePok);
                testStream >> dummy;
                if (dummy.GetEntryType()!=ErrorPok)
                {
                    _gameFrom = PokemonRuby; // this suffices for Pokemon Sapphire as well
                    _gameFromVerified = true;
                }
                else
                    _gameFromVerified = false;
            }
            return;
        }
    }
    _gameFrom = DemoJirachiDisk;
    _gameFromVerified = false;
}
dword PokemonSaveFile::_getStartOffsetFor(PokemonEntryType flag,dword occurIndex) const
{
    // Here, I conditionally select offsets for the specified entry
    if (_gameFrom==DemoJirachiDisk) // the battery file isn't loaded
        return 0;
    dword start, entrySize = 80, entryExtra = 0;
    switch (flag)
    {
    case PartyPok:
        if (occurIndex>=6)
            return 0;
        if (_gameFrom>=PokemonFireRed && _gameFrom<=PokemonLeafGreen)
            start = 0xfb8;
        else
            start = 0x11b8;
        entrySize += 20; // party poks have an extra 20 bytes
        break;
    case DayCarePok:
        if (occurIndex>=2)
        {
            if (occurIndex==2 && _gameFrom>=PokemonFireRed && _gameFrom<=PokemonLeafGreen)
                return 0xc418; // the lone day care south of cerulean
            else
                return 0;
        }
        entryExtra = (_gameFrom!=PokemonRuby && _gameFrom!=PokemonSapphire ? 60 : 0); // 60 bytes between day care entries (For games past Ruby and Sapphire)
        if (_gameFrom>=PokemonFireRed && _gameFrom<=PokemonLeafGreen)
            start = 0x3f00;
        else if (_gameFromVerified)
        {
            if (_gameFrom==PokemonRuby || _gameFrom==PokemonSapphire)
                start = 0x3f1c;
            else if (_gameFrom==PokemonEmerald)
                start = 0x3fb0;
            else
                return 0;
        }
        else
            return 0;
        break;
    case PCPok: // thankfully, PC pokemon appear at the same start offset across all games in generation 3
        if (occurIndex>=SAVE_PC_MAX_COUNT)
            return 0;
        start = 0x4d84;
        break;
    default:
        return 0;
    }
    return start+(entrySize+entryExtra)*occurIndex;
}