// Pokemon.h
#ifndef POKEMON_H
#define POKEMON_H

namespace PokemonEditor
{
    struct pok_char
    {
        pok_char() : theChar(0) {}
        pok_char(byte b) : theChar(_translate(b,true)) {} // performs a translation on the byte (from ASCII to pokemon)
        pok_char& operator =(byte); // performs a translation on the byte (from ASCII to pokemon)
        operator byte() const { return theChar; }
        byte theChar; // assign here directly to bypass character translation
        char getASCII() const { return (char) _translate(theChar,false); }
    private:
        byte _translate(byte b,bool toPokemonChar) const;
    };

    struct pok_mark
    {
        pok_mark() : circle(false),square(false),triangle(false),heart(false) {}
        pok_mark(byte b) { load_from(b); }

        bool circle, square, triangle, heart;

        void load_from(byte);
        byte get_from() const;
    };

    struct pp_ups
    {
        pp_ups() : move1Ups(0),move2Ups(0),move3Ups(0),move4Ups(0) {}
        pp_ups(byte b) { load_from(b); }

        // Note: pp ups have a maximum of 3
        int move1Ups, move2Ups, move3Ups, move4Ups;

        void load_from(byte);
        byte get_from() const;
    };

    /*
        Note -
            At any time, a pok_string object should store the actual
        pokemon data structure char data, not ASCII characters.
    */
    class pok_string : public rtypes::rtype_string<pok_char>
    {
    public:
        pok_string() {}
        pok_string(const char*);

        pok_string& operator =(const char*);

        rtypes::str to_string() const;
    };

    template<class T>
    rtypes::rstream<T>& operator <<(rtypes::rstream<T>& stream,const pok_string& s)
    {
        // get ASCII representation
        for (rtypes::dword i = 0;i<s.length();i++)
        {
            char c = s[i].getASCII();
            if (c=='~') 
                break; // stop at bad char, which most likely terminates the string
            stream.put(c);
        }
        return stream;
    }
    template<class T>
    rtypes::rbinstream<T>& operator <<(rtypes::rbinstream<T>& binStream,const pok_string& s)
    {
        // keep pokemon string representation
        for (rtypes::dword i = 0;i<s.length();i++)
            binStream.put(s[i].theChar); // need to put all characters
        return binStream;
    }

    enum PokemonStat
    {
        /*
            This defines the standard order for pokemon stats
                (like in an array/game data). Don't change it!
        */
        HP,
        Attack,
        Defense,
        Speed,
        SpcAttack,
        SpcDefense
    };

    enum ContestStat
    {
        /*
            This defines the standard order for pokemon contest stats
                (like in an array/game data). Don't change their values!
        */
        Coolness,
        Beauty,
        Cuteness,
        Smartness,
        Toughness,
        Feel
    };

    enum PokemonNature
    {
        /*
            This defines the standard order for pokemon natures
                and should not be modified!
        */
        Hardy,
        Lonely,
        Brave,
        Adament,
        Naughty,
        Bold,
        Docile,
        Relaxed,
        Impish,
        Lax,
        Timid,
        Hasty,
        Serious,
        Jolly,
        Naive,
        Modest,
        Mild,
        Quiet,
        Bashful,
        Rash,
        Calm,
        Gentle,
        Sassy,
        Careful,
        Quirky
    };

    enum Gender
    {
        Male,
        Female,
        Unknown // I was really tempted to call this bender...
    };

    enum Ball
    {
        /*
            This enumeration defines the standard value order
                for poke-balls used by the game. Don't change them!
        */
        MasterBall = 1,
        UltraBall,
        GreatBall,
        PokeBall,
        SafariBall,
        NetBall,
        DiveBall,
        NestBall,
        RepeatBall,
        TimerBall,
        LuxeryBall,
        PremierBall
    };

    enum PokemonStatusCondition
    {
        /*
            The values in this enumeration correspond to the
                game values for status - so don't change them!
        */
        OK,
        Sleep1, // SleepN, where N is the number of sleep turns
        Sleep2,
        Sleep3,
        Sleep4,
        Sleep5,
        Sleep6,
        MaxSleep,
        Poisoned,
        Burned = 16,
        Frozen = 32,
        Paralysis = 64,
        BadPoison = 128
    };

    enum PokemonGameFromFlag
    {
        /*
            Don't change the values of this enumeration!
        */
        DemoJirachiDisk, // what the heck is this?
        PokemonSapphire,
        PokemonRuby,
        PokemonEmerald,
        PokemonFireRed,
        PokemonLeafGreen,
        PokemonColosseum = 15
    };

    enum PokemonEntryType
    {
        /*
            Don't modify the order of the values in this enumeration!
        */
        ErrorPok = -1,
        NullPok, // an invalid, null pokemon object state; should read into a NullPok though not required
        NewPok, // a brand new pokemon that hasn't been placed in a location; should refer to a valid pokemon
        PartyPok,
        DayCarePok,
        PCPok,
    };

    class Pokemon
    {
    public:
        // constructor for blank pokemon
        Pokemon();
        Pokemon(PokemonEntryType);

        // constructor for a new usable pokemon
        Pokemon(rtypes::word Species,
            PokemonGameFromFlag GameFrom,
            const pok_string& TrainerName,
            rtypes::word TrainerID,
            rtypes::word TrainerSecretID,
            Gender TrainerGender,
            bool IsPokEgg = false,
            const PokemonLocation* LocationCaught = NULL,
            const char* Nickname = NULL,
            byte DefaultLevel = 1);

        // operations
        void CalculateStats(); // calculate the party stats for a pokemon
        void Heal() { _calculatedStats.currentHP = GetStat(HP); } // for party poks

        // "fun" operations
        void RandomizeMoves();

        // "get" methods
        //  (note - these methods return all byte values as word values)
        const PokemonBase* GetSpeciesBase() const;
        PokemonEntryType GetEntryType() const { return _entryFlag; }
        rtypes::dword GetPID() const { return _pid; }
        rtypes::word GetOriginalTrainerID() const { return _originalTrainerID; }
        rtypes::word GetOriginalTrainerSecretID() const { return (rtypes::word) (_originalTrainerID>>16); }
        pok_string GetOriginalTrainerName() const { return _originalTrainerName; }
        Gender GetOriginalTrainerGender() const { return (Gender) (_mainData.pokeBall_trainerGender>>7); }
        pok_mark GetMark() const { return _mark; }
        rtypes::word GetSpecies() const { return _mainData.species; }
        pok_string GetNickname() const;
        Ball GetBallCaughtWith() const { return (Ball) ((_mainData.pokeBall_trainerGender>>3)&0x0f); }
        PokemonGameFromFlag GetGameFrom() const;
        const PokemonItem* GetHeldItem() const;
        pp_ups GetPPUps() const { return _mainData.ppBonus; }
        rtypes::word GetPP(int MoveNumber) const { return (rtypes::word) _mainData.pp[MoveNumber-1]; }
        rtypes::word GetEV(PokemonStat stat) const { return (rtypes::word) _mainData.evs[stat]; }
        rtypes::word GetIV(PokemonStat stat) const;
        rtypes::word GetLevel() const;
        rtypes::word GetPartyLevel() const { return _level; }
        rtypes::word GetLevelCaught() const { return _mainData.levelCaught&0x7f; } // if 0 then Level 5 (egg)
        const PokemonLocation* GetLocationCaught() const;
        rtypes::word GetStat(PokemonStat stat) const;
        rtypes::word GetPartyStat(PokemonStat stat) const { return _calculatedStats.stats[stat]; }
        rtypes::word GetContestStat(ContestStat stat) const { return (rtypes::word) _mainData.contestStats[stat]; }
        Gender GetGender() const;
        PokemonNature GetNature() const { return  (PokemonNature) (_pid%0x19); }
        rtypes::word GetFriendship() const { return _mainData.friendship; }
        rtypes::dword GetExperience() const { return _mainData.experience; }
        PokemonStatusCondition GetStatusCondition() const { return (PokemonStatusCondition) _status; }
        //  (specific to party poks)
        rtypes::word GetCurrentHP() const { return _calculatedStats.currentHP; }
        const PokemonMove* GetMove(int MoveNumber) const;

        // "to" methods
        rtypes::rbinstringstream ToBinaryStream(bool RecalcStats = true) const;
        rtypes::str ToString() const;
        rtypes::str ToBytesString(bool RecalcStats = true) const;
        
        // "predicates"
        bool HasIndeterminateStatus() const { return _entryFlag<PartyPok || _entryFlag>PCPok; } 
        bool HasAbilityFlagOn() const { return (_mainData.ivs&0x80000000) != 0; }
        bool IsEgg() const { return (_mainData.ivs&0x40000000) != 0; /*return (_mainData.ivs/0x40000000%2) == 1;*/ };
        bool WasCaughtAsEgg() const { return _mainData.levelCaught==0; }
        bool IsObeySet() const { return (_mainData.ribbons&0x80000000) != 0; }
        bool IsShiny() const;

        // "set" methods
        void SetEntryType(PokemonEntryType t) { _entryFlag = t; }
        void SetOriginalTrainerID(rtypes::word ID,rtypes::word SecretID);
        void SetOriginalTrainerName(const pok_string& s);
        void SetOriginalTrainerGender(Gender g);
        void SetMark(const pok_mark& m) { _mark = m.get_from(); }
        void SetSpecies(const PokemonBase* pBase) { _mainData.species = pBase->speciesIndexNo; }
        void SetSpecies(rtypes::word IndexNo) { _mainData.species = IndexNo; }
        void SetNickname(const pok_string&);
        void SetNickname(const PokemonBase* pBase);
        void SetBallCaughtWith(Ball b);
        void SetGameFrom(PokemonGameFromFlag);
        void SetHeldItem(const PokemonItem* pItemBase) { _mainData.heldItem = pItemBase->itemValue; }
        void SetHeldItem(rtypes::word itemIndex) { _mainData.heldItem = itemIndex; }
        void SetPPUps(const pp_ups& ups) { _mainData.ppBonus = ups.get_from(); }
        void SetPP(int MoveNumber,byte PPAmount);
        void SetEV(PokemonStat stat,byte evVal);
        void SetIV(PokemonStat stat,byte ivVal);
        void SetMaxIVs() { _mainData.ivs |= 0x3fffffff; }
        void SetMove(int MoveNumber,const PokemonMove* pMove) { _mainData.attacks[MoveNumber-1] = pMove->moveValue; }
        void SetMove(int MoveNumber,rtypes::word moveVal) { _mainData.attacks[MoveNumber-1] = moveVal; }
        void SetExperience(rtypes::dword Exp);
        bool SetLevel(byte Level);
        void SetPartyLevel(byte Level) { _level = Level; }
        void SetLevelCaught(byte Level) { _mainData.levelCaught = Level; }
        void SetLocationCaught(const PokemonLocation* pLoc) { _mainData.locationCaught = (byte) pLoc->locationValue; }
        void SetLocationCaught(byte loc) { _mainData.locationCaught = loc; }
        void SetPartyStat(PokemonStat stat,rtypes::word statVal) { _calculatedStats.stats[stat] = statVal; }
        void SetContestStat(ContestStat stat,byte conVal) { _mainData.contestStats[stat] = conVal; }
        void SetNature(PokemonNature Nature);
        void SetGender(Gender Gender);
        void SetFriendship(byte value) { _mainData.friendship = value; }
        void SetObey(bool On);
        void SetShiny(bool On);
    private:
        struct _PokDataBlock
        { // main pokemon data block - 48 bytes
            _PokDataBlock();

            byte            ppBonus,
                            pp[4],
                            evs[6],
                            contestStats[6],
                            friendship,
                            pokerusStatus,
                            locationCaught,
                            levelCaught, // interpret as a 7-bit signed value
                            pokeBall_trainerGender;
            rtypes::word    species,
                            heldItem,
                            attacks[4],
                            padding1;
            rtypes::dword   experience,
                            ivs, // 5-bit ivs from 0-29; bit 30 is egg flag; bit 31 is ability flag
                            ribbons; // bit 31 is the obey bit; I don't know what bit 30 is for...

            rtypes::rbinstringstream getDataStream() const;
            rtypes::rbinstringstream getDataStreamWithOrder(int dataBlockForm) const;
            // toggles the encryption state of the data
            void encryptDecrypt(const rtypes::dword encryptKey);
        };
        struct _PokPartyStats
        { // pokemon stats calculated by game for quick lookup in party)
            _PokPartyStats();

            rtypes::word    currentHP;
            rtypes::word    stats[6]; // order of stats is defined in PokemonStat

            rtypes::rbinstringstream getDataStream() const;
        };

        // Pokemon data (100 bytes)
        //  (80)
        rtypes::dword   _pid;
        rtypes::dword   _originalTrainerID; // high word contains "secret ID"; low word contains regular ID
        pok_string      _nickname; // 10 bytes
        rtypes::word    _languageID;
        pok_string      _originalTrainerName; // 7 bytes
        byte    _mark;
        rtypes::word    _checksum;
        rtypes::word    _padding1;
        _PokDataBlock   _mainData;
        // (if the pokemon is in the party, it will have these fields)
        //  (20)
        rtypes::dword   _status;
        byte    _level;
        byte    _pokerusRemaining;
        _PokPartyStats  _calculatedStats;

        // misc. flag data not associated with any game data
        PokemonEntryType _entryFlag;
        bool _statsCalculated; // if true, _calculatedStats holds correct stat values

        // helper functions
        void _calcChecksum();
        rtypes::dword _getEncryptKey() const { return _pid^_originalTrainerID; }

        // friends of Pokemon
        friend rtypes::rwin32::fileIO::FileBufferStream& operator >>(rtypes::rwin32::fileIO::FileBufferStream&,Pokemon&);
    };

    // operators for Pokemon
    rtypes::rwin32::fileIO::FileBufferStream& operator >>(rtypes::rwin32::fileIO::FileBufferStream&,Pokemon&);

    template<class T>
    rtypes::rstream<T>& operator <<(rtypes::rstream<T>& stream,const Pokemon& p)
    {
        stream << p.ToString();
        return stream;
    }

    template<class T>
    rtypes::rbinstream<T>& operator <<(rtypes::rbinstream<T>& binaryStream,const Pokemon& p)
    {
        // take data from one stream and send it to another
        binaryStream << p.ToBinaryStream().get_device();
        return binaryStream;
    }
}

#endif