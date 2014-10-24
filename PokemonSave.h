//PokemonSave.h
#ifndef POKEMONSAVE_H
#define POKEMONSAVE_H

namespace PokemonEditor
{
    enum SaveFileState
    {
        CurrentState,
        PreviousState,
        UndeterminedState
    };

    struct PokemonEntry
    {
        Pokemon pok;
        rtypes::dword offset; // the offset in whatever buffer the pokemon was read from
        SaveFileState state;
    };

    bool operator ==(const PokemonEntry& left,const PokemonEntry& right);

    class PokemonSaveFile
    {
    public:
        enum LoadOperationReturn
        {
            LoadSuccess,
            LoadFailBadValidation, // the file had validation errors OR didn't have enough data to satisfy loading constraints
            LoadFailFileNotOpened // the file could not be opened: it may not exist or is being used and not shared by another process
        };
        enum EntryOperationReturn
        {
            EntryObtained,
            EntryOutstanding,
            EntryUnobtainable
        };

        PokemonSaveFile();

        LoadOperationReturn LoadFrom(const rtypes::str& File,PokemonGameFromFlag gameFrom = DemoJirachiDisk,bool SkipChecksumValidation = false); // load data from File; returns 
        bool SaveTo(const rtypes::str& File,bool SaveReadOnly = false); // save data to File; marks the file as saved
        void SaveTo(rtypes::rwin32::fileIO::FileBufferStream&); // save data to file buffer by stream
        void Clear();
        void ResetEntries() { _outstandingEntryOffsets.empty(); } // clears any record of outstanding entries
        bool WasAltered() const { return _altered; }
        //void FlagUnsaved() { _altered = true; }
        //void FlagSaved() { _altered = false; } 

        // Special save functions for ordered memory
        //      for the buffer overloads, if allocateBuffer = true, the buffer interfaced by the
        //      specified stream is re-allocated (if its size is not great enough) 
        //      and the stream is set to the beginning before writing the data
        bool SaveCombinedMemoryFile(const rtypes::str& File,bool SaveReadOnly = false);
        void SaveCombinedMemoryFile(rtypes::rwin32::fileIO::FileBufferStream&,bool allocateBuffer = false);
        bool SaveCurrentMemoryFile(const rtypes::str& File,bool SaveReadOnly = false);
        void SaveCurrentMemoryFile(rtypes::rwin32::fileIO::FileBufferStream&,bool allocateBuffer = false);
        bool SavePreviousMemoryFile(const rtypes::str& File,bool SaveReadOnly = false);
        void SavePreviousMemoryFile(rtypes::rwin32::fileIO::FileBufferStream&,bool allocateBuffer = false);

        // Pokemon operations
        //      (the occurance index marks the entry's position in relation to other entries of the same type)
        //      (this search is a specialized search that is more precise)
        //      (a shallow entry is a string that identifies an entry; it is not outstanding)
        rtypes::str GetShallowPokemonEntry(PokemonEntryType entryType,SaveFileState stateType,rtypes::dword occuranceIndex) const;
        EntryOperationReturn GetPokemonEntry(PokemonEntry& entryObject,PokemonEntryType entryType,SaveFileState stateType,rtypes::dword occuranceIndex,bool flagUnsaved = false) const; // returns false if the entry is outstanding or cannot be obtained
        //      (this search will find the next valid pokemon entry in the specified save; block number is used to determine entry type)
        PokemonEntry GetNextPokemonEntry(SaveFileState stateFrom,bool& passedStart_out,bool flagUnsaved = false) const; // will cycle through all pokemon in the file; if the out param. 'passedStart_out' is true, a entry wasn't successfully read and the end of data stream was reached
        bool PutPokemonEntry(const PokemonEntry& entryObject,bool flagUnsaved = true); // note: if the entry's state is undetermined, current is assumed; the return value indicates whether or not the entry was originally taken out - if false, the put operation failed as well

        // misc.
        rtypes::rbinstringstream ToBinaryStream(SaveFileState SaveState) const; // gets a binary stream of the data section for the specified save
        PokemonGameFromFlag GetGameFrom(bool& IsConclusive_out) const; // 'conclusive-ness' refers to if locations can be concluded, not version
        //  (these don't have options for previous/current since the data would most likely be the same either way!)
        Gender GetTrainerGender() const;
        pok_string GetTrainerName() const;
        rtypes::word GetTrainerRegularID() const;
        rtypes::word GetTrainerSecretID() const;

        // fun stuff
        void SetTrainerGender(Gender);
    private:
        static const rtypes::dword BUFFER_SIZE;
        struct _Block
        {
            byte data[3968];
            // padding: 116 bytes
            // signiture: 12 bytes
            byte blockID;
            byte padding;
            rtypes::word checksum;
            rtypes::dword validation;
            rtypes::dword saveID;
            // block location offset (for re-structuring)
            rtypes::dword locationOffset;

            // comparison operators
            bool operator <(const _Block&);
            bool operator >(const _Block&);
            bool operator ==(const _Block&);

            void output(rtypes::rwin32::fileIO::FileBufferStream&);
            bool input(rtypes::rwin32::fileIO::FileStream_binary&,bool performChecksumValidation);
            void calculate_checksum();
        };

        PokemonGameFromFlag _gameFrom; // this value won't be entirely precise; however it will suffice for the purposes of loading and saving pokemon entries
        bool _gameFromVerified; // if true, the game from flag is conclusive (applies for day care pok offsets)
        rtypes::list<_Block> _blocks; // blocks from 0 - 13 (current save); from 14-27 (previousSave); 28-31 (other)
        rtypes::rwin32::fileIO::FileBuffer _currentSave, _previousSave; // store copy of block data in buffers
        mutable rtypes::dword   _nextIter;
        mutable rtypes::set<rtypes::dword> _outstandingEntryOffsets;

        mutable bool _altered;
        
        rtypes::dword _getBlockWithOffset(rtypes::dword offset) const;
        void _loadBlocks(); // loads data from file buffers into blocks
        void _loadBuffers(); // loads data from blocks into file buffers
        void _determineGameFrom(); // determines and sets the game from flag for the save file
        rtypes::dword _getStartOffsetFor(PokemonEntryType,rtypes::dword) const; // note: a return of zero indicates lookup error
    };

    // this will re-allocate the buffer associated with the buffer stream and overwrite any contents in the save file range
    rtypes::rwin32::fileIO::FileBufferStream& operator <<(rtypes::rwin32::fileIO::FileBufferStream&,PokemonSaveFile&);
    // read an entry starting at the buffer stream's input iterator offset
    rtypes::rwin32::fileIO::FileBufferStream& operator >>(rtypes::rwin32::fileIO::FileBufferStream&,PokemonEntry&);
}

#endif