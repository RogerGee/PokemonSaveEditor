#include "MasterInclude.h"
using namespace rtypes;
using namespace PokemonEditor;

pok_char& pok_char::operator =(byte b)
{
    theChar = _translate(b,true);
    return *this;
}
byte pok_char::_translate(byte b,bool toPokemonChar) const
{
    byte r;
    if (toPokemonChar)
    {
        if (b>='A' && b<='Z')
            r = b+0xbb-'A';
        else if (b>='a' && b<= 'z')
            r = b+0xd5-'a';
        else
        {
            switch (b)
            {
            case '.':
                r = 0xad;
                break;
            case '-':
                r = 0xae;
                break;
            case '\"':
                r = 0xb1; // OR 0xb2
                break;
            case '\'':
                r = 0xb4; // OR 0xb3
                break;
            case ',':
                r = 0xb8;
                break;
            case '$':
                r = 0xb7;
                break;
            case '/':
                r = 0xba;
                break;
            case ':':
                r = 0xf0;
                break;
            case '=':
                r = 0xfa; // OR 0xfc
                break;
            case '*':
                r = 0xfb;
                break;
            case '@':
                r = 0xfd;
                break;
            case '+':
                r = 0xfe;
                break;
            case '!':
                r = 0xab;
                break;
            case '?':
                r = 0xac;
                break;
            case ' ':
                r = 0x00;
                break;
            default:
                r = 0xff; // BADCHAR; could use 0
                break;
            }
        }
        return r;
    }
    if (b>=0xa1 && b<=0xaa)
        r = '0'+b-0xa1;
    else if (b>=0xbb && b<=0xd4)
        r = 'A'+b-0xbb;
    else if (b>=0xd5 && b<=0xee)
        r = 'a'+b-0xd5;
    else
    {
        switch (b)
        {
        case 0xad:
            r = '.';
            break;
        case 0xae:
            r = '-';
            break;
        case 0xb1:
        case 0xb2:
            r = '\"';
            break;
        case 0xb3:
        case 0xb4:
            r = '\'';
            break;
        case 0xb8:
            r = ',';
            break;
        case 0xb7:
            r = '$';
            break;
        case 0xba:
            r = '/';
            break;
        case 0xf0:
            r = ':';
            break;
        case 0xfa:
        case 0xfc:
            r = '=';
            break;
        case 0xfb:
            r = '*';
            break;
        case 0xfd:
            r = '@';
            break;
        case 0xfe:
            r = '+';
            break;
        case 0xab:
            r = '!';
            break;
        case 0xac:
            r = '?';
            break;
        case 0x00:
            r = ' ';
            break;
        default:
            r = '~'; // bad char display character
            break;
        }
    }
    return r;
}

pok_string::pok_string(const char* cStr)
{
    dword i = 0;
    while (cStr[i])
    {
        push_back(cStr[i]);
        i++;
    }
}
pok_string& pok_string::operator =(const char* cStr)
{
    dword i = 0;
    clear();
    while (cStr[i])
    {
        push_back(cStr[i]);
        i++;
    }
    return *this;
}
str pok_string::to_string() const
{
    rstringstream ss;
    for (dword i = 0;i<length();i++)
    {
        char c = operator[](i).getASCII();
        if (c=='~')
            break;
        ss.put(c);
    }
    return ss.get_device();
}

void pok_mark::load_from(byte b)
{
    circle = (b&MARK_CIRCLE)!=0;
    square = (b&MARK_SQUARE)!=0;
    triangle = (b&MARK_TRIANGLE)!=0;
    heart = (b&MARK_HEART)!=0;
}
byte pok_mark::get_from() const
{
    byte r = 0;
    if (circle)
        r |= MARK_CIRCLE;
    if (square)
        r |= MARK_SQUARE;
    if (triangle)
        r |= MARK_TRIANGLE;
    if (heart)
        r |= MARK_HEART;
    return r;
}

void pp_ups::load_from(byte b)
{
    move1Ups = b & 0x03;
    move2Ups = (b & 0x0c) >> 2;
    move3Ups = (b & 0x30) >> 4;
    move4Ups = (b & 0xc0) >> 6;
}
byte pp_ups::get_from() const
{
    byte r = 0;
    if (move1Ups>=0 && move1Ups<=3)
        r |= byte(move1Ups);
    if (move2Ups>=0 && move2Ups<=3)
        r |= byte(move2Ups) << 2;
    if (move3Ups>=0 && move3Ups<=3)
        r |= byte(move3Ups) << 4;
    if (move4Ups>=0 && move4Ups<=3)
        r |= byte(move4Ups) << 6;
    return r;
}

Pokemon::_PokDataBlock::_PokDataBlock()
{
    ppBonus = 0;
    for (int i = 0;i<6;i++)
    {
        evs[i] = 0;
        contestStats[i] = 0;
        if (i>=4)
            continue;
        pp[i] = 0;
        attacks[i] = 0;
    }
    friendship = 0;
    pokerusStatus = 0;
    locationCaught = 0;
    levelCaught = 0;
    pokeBall_trainerGender = 0;
    species = 0;
    heldItem = 0;
    padding1 = 0;
    experience = 0;
    ivs = 0;
    ribbons = 0;
}
rbinstringstream Pokemon::_PokDataBlock::getDataStream() const
{
    rbinstringstream bss;
    // read all data into the stream
    bss << species << heldItem << experience
        << ppBonus << friendship << padding1;
    for (int i = 0;i<4;i++)
        bss << attacks[i];
    for (int i = 0;i<4;i++)
        bss << pp[i];
    for (PokemonStat stat = HP;stat<=SpcDefense;((int&)stat)++)
        bss << evs[stat];
    for (ContestStat stat = Coolness;stat<=Feel;((int&)stat)++)
        bss << contestStats[stat];
    bss << pokerusStatus << locationCaught << levelCaught << pokeBall_trainerGender
        << ivs << ribbons;
    return bss;
}
rbinstringstream Pokemon::_PokDataBlock::getDataStreamWithOrder(int dataBlockForm) const
{
    rbinstringstream bss;
    for (int i = 0;i<4;i++)
    {
        switch (DATA_BLOCK_ORDERS[dataBlockForm][i])
        {
        case SUB_G:
            bss << species << heldItem
                << experience << ppBonus << friendship
                << padding1;
            break;
        case SUB_A:
            for (int j = 0;j<4;j++)
                bss << attacks[j];
            for (int j = 0;j<4;j++)
                bss << pp[j];
            break;
        case SUB_E:
            for (PokemonStat stat = HP;stat<=SpcDefense;((int&)stat)++)
                bss << evs[stat];
            for (ContestStat stat = Coolness;stat<=Feel;((int&)stat)++)
                bss << contestStats[stat];
            break;
        case SUB_M:
            bss << pokerusStatus << locationCaught 
                << levelCaught << pokeBall_trainerGender
                << ivs << ribbons;
            break;
        }
    }
    return bss;
}
void Pokemon::_PokDataBlock::encryptDecrypt(const dword encryptKey)
{
    dword counter;
    rbinstringstream bss = getDataStream(); // get data as a binary stream
    str* ps = &bss.get_device(); // get a pointer to the data string
    // decrypt data in string object pointed to by ps
    counter = 0;
    while (counter+3<ps->length())
    {
        dword var;
        str s;
        rbinstringstream binStream(s); // opens for appending (in output mode)
        for (int cnt = 0;cnt<4;cnt++)
            s.push_back( (*ps)[counter+cnt] );
        binStream >> var;
        var ^= encryptKey;
        binStream.set_output_iter(0); // set back to beginnning
        binStream << var;
        for (int cnt = 0;cnt<4;cnt++)
            (*ps)[counter+cnt] = s[cnt];
        counter += 4;
    }
    // put data back in struct fields from the binary stream
    bss >> species >> heldItem >> experience >> ppBonus >> friendship >> padding1;
    for (int i = 0;i<4;i++)
        bss >> attacks[i];
    for (int i = 0;i<4;i++)
        bss >> pp[i];
    for (PokemonStat stat = HP;stat<=SpcDefense;((int&)stat)++)
        bss >> evs[stat];
    for (ContestStat stat = Coolness;stat<=Feel;((int&)stat)++)
        bss >> contestStats[stat];
    bss >> pokerusStatus >> locationCaught >> levelCaught >> pokeBall_trainerGender
        >> ivs >> ribbons;
}

Pokemon::_PokPartyStats::_PokPartyStats()
{
    currentHP = 0;
    for (int i = 0;i<6;i++)
        stats[i] = 0;
}
rbinstringstream Pokemon::_PokPartyStats::getDataStream() const
{
    rbinstringstream bss;
    bss << currentHP;
    for (int i = 0;i<6;i++)
        bss << stats[i];
    return bss;
}

// non-member operators for Pokemon

/* 
    This input operator overload makes the following assumptions:
        About the binary input stream -
            -the input iterator points to the start of a pokemon entry
        About the Pokemon object -
            -the object's entry type is set
                -if the entry flag is ErrorPok, the function does not attempt to load pokemon data
    This function will perform validation on the pokemon; if the validation fails,
        the pok's entry flag is set to ErrorPok; else success, the pok's flag is not modified.
*/
rwin32::fileIO::FileBufferStream& PokemonEditor::operator >>(rwin32::fileIO::FileBufferStream& binaryStream,Pokemon& pok)
{
    if (pok.GetEntryType()==ErrorPok)
        return binaryStream;
    dword dataBlockForm;
    binaryStream >> pok._pid >> pok._originalTrainerID;
    for (int i = 0;i<10;i++) // get nickname string
    {
        pok_char c;
        c.theChar = binaryStream.get();
        pok._nickname.push_back(c);
    }
    binaryStream >> pok._languageID;
    for (int i = 0;i<7;i++) // get OT name string
    {
        pok_char c;
        c.theChar = binaryStream.get();
        pok._originalTrainerName.push_back(c);
    }
    binaryStream >> pok._mark >> pok._checksum;
    binaryStream >> pok._padding1;
    // read big data block (_PokDataBlock)
    dataBlockForm = pok._pid%DATA_BLOCK_ORDERS_COUNT;
    //  read each of the 4 sub blocks in the prescribed order for the pokemon
    for (int i = 0;i<4;i++)
    {
        switch ( DATA_BLOCK_ORDERS[dataBlockForm][i] )
        {
        case SUB_G:
            binaryStream >> pok._mainData.species >> pok._mainData.heldItem
                >> pok._mainData.experience >> pok._mainData.ppBonus >> pok._mainData.friendship
                >> pok._mainData.padding1;
            break;
        case SUB_A:
            for (int j = 0;j<4;j++)
                binaryStream >> pok._mainData.attacks[j];
            for (int j = 0;j<4;j++)
                binaryStream >> pok._mainData.pp[j];
            break;
        case SUB_E:
            for (PokemonStat stat = HP;stat<=SpcDefense;((int&)stat)++)
                binaryStream >> pok._mainData.evs[stat];
            for (ContestStat stat = Coolness;stat<=Feel;((int&)stat)++)
                binaryStream >> pok._mainData.contestStats[stat];
            break;
        case SUB_M:
            binaryStream >> pok._mainData.pokerusStatus >> pok._mainData.locationCaught 
                >> pok._mainData.levelCaught >> pok._mainData.pokeBall_trainerGender
                >> pok._mainData.ivs >> pok._mainData.ribbons;
            break;
        }
    }
    if (pok.GetEntryType()==PartyPok)
    {// read data specific to the pokemon being a party pokemon
        binaryStream >> pok._status >> pok._level >> pok._pokerusRemaining >> pok._calculatedStats.currentHP;
        for (PokemonStat stat = HP;stat<=SpcDefense;((int&)stat)++)
            binaryStream >> pok._calculatedStats.stats[stat];
        pok._statsCalculated = true;
    }
    else
        pok._statsCalculated = false;
    // perform final operations on the newly read data
    pok._mainData.encryptDecrypt( pok._getEncryptKey() );
    // perform validation on the newly read data
    dword check = pok._checksum;
    pok._calcChecksum();
    if (!binaryStream /*check to see if the last input operation was successful*/
        || check!=pok._checksum
        || pok._mainData.species==0 || pok._mainData.species>MAX_POKEMON_INDEX)
    {
        if (check==0)
            pok._entryFlag = NullPok;
        else
            pok._entryFlag = ErrorPok;
    }
    return binaryStream;
}
//

Pokemon::Pokemon()
{
    _pid = 0;
    _originalTrainerID = 0;
    _languageID = 0;
    _mark = 0;
    _checksum = 0;
    _padding1 = 0;
    _status = 0;
    _level = 0;
    _pokerusRemaining = 0;
    _entryFlag = NullPok;
    _statsCalculated = false;
}
Pokemon::Pokemon(PokemonEntryType flag)
{
    _pid = 0;
    _originalTrainerID = 0;
    _languageID = 0;
    _mark = 0;
    _checksum = 0;
    _padding1 = 0;
    _status = 0;
    _level = 0;
    _pokerusRemaining = 0;
    _entryFlag = flag;
    _statsCalculated = false;
}
Pokemon::Pokemon(word Species,
            PokemonGameFromFlag GameFrom,
            const pok_string& TrainerName,
            word TrainerID,
            word TrainerSecretID,
            Gender TrainerGender,
            bool IsPokEgg,
            const PokemonLocation* LocationCaught,
            const char* Nickname,
            byte DefaultLevel)
{
    const PokemonBase* speciesBase;
    _statsCalculated = false;
    _pid = PokMath::GetRandom<dword>(); // get a random PID
    SetOriginalTrainerName(TrainerName);
    SetOriginalTrainerID(TrainerID,TrainerSecretID);
    _mark = 0;
    _checksum = 0;
    _padding1 = 0;
    _status = 0;
    _pokerusRemaining = 0xff;
    // set all data for a viable pokemon
    _mainData.species = Species;
    speciesBase = GetSpeciesBase();
    _mainData.ivs |= PokMath::GetRandom<dword>()&0x3FFFFFFF; // get random ivs
    if (!IsPokEgg)
    {
        // set to level 1 by default
        if (!SetLevel(DefaultLevel))
            SetLevel(1);
        // set nickname
        if (Nickname!=NULL)
            SetNickname(Nickname);
        else
            // set a default nickname using the pokemon's species name
            SetNickname(speciesBase);
        _mainData.friendship = 35; // I figure this is a pretty decent default friendship value
        _languageID = LANGUAGE_ID;
    }
    else
    {
        _mainData.ivs |= 0x40000000; // turn on egg flag bit
        SetLevel(5); // all eggs should be set to level 5 (the game enforces this)
        // set nickname to default egg nickname
        for (int i = 0;i<10;i++)
        {
            pok_char c;
            c.theChar = EGG_NICKNAME_BYTES[i];
            _nickname.push_back(c);
        }
        _mainData.friendship = speciesBase->eggCycles; // set default egg cycles
        _languageID = EGG_LANGUAGE_ID;
    }
    _mainData.levelCaught = (IsPokEgg ? 0/* special code for hatched; set just in case */ : _level&0x7f);
    CalculateStats();
    _calculatedStats.currentHP = _calculatedStats.stats[HP];
    SetGameFrom(GameFrom);
    SetOriginalTrainerGender(TrainerGender);
    SetBallCaughtWith(PokeBall);
    if (LocationCaught!=NULL)
        SetLocationCaught(LocationCaught);
    for (int i = 0;i<4;i++) // set default moves; I do this for even non-eggs
        _mainData.attacks[i] = POKEMON_DEFAULT_MOVES[speciesBase->speciesIndexNo][i];
    _entryFlag = NewPok;
}
void Pokemon::CalculateStats()
{
    PokMath::StatCalculator calculator(*this);
    _level = (byte) GetLevel();
    for (PokemonStat stat = HP;stat<=SpcDefense;((int&)stat)++)
        _calculatedStats.stats[stat] = calculator.CalcStat(stat);
    _statsCalculated = true;
}
void Pokemon::RandomizeMoves()
{
    for (int i = 0;i<4;i++)
        _mainData.attacks[i] = PokMath::GetRandom<word>()%MAX_POKEMON_MOVE + 1;
}
const PokemonBase* Pokemon::GetSpeciesBase() const
{
    if (_mainData.species>MAX_POKEMON_INDEX)
        return &POKEMON_BASE_DATA[0];
    return &POKEMON_BASE_DATA[_mainData.species];
}
const PokemonMove* Pokemon::GetMove(int MoveNumber) const
{
    word attackNo = _mainData.attacks[MoveNumber-1];
    if (attackNo>MAX_POKEMON_MOVE)
        return &POKEMON_MOVE_DATA[355]; // this is a bad move element
    return &POKEMON_MOVE_DATA[attackNo];
}
const PokemonLocation* Pokemon::GetLocationCaught() const
{
    if (!IsValidPokemonLocationIndex(_mainData.locationCaught))
        return &POKEMON_LOCATION_DATA[214]; // this is a bad location element
    return POKEMON_LOCATION_LOOKUP[_mainData.locationCaught];
}
const PokemonItem* Pokemon::GetHeldItem() const
{
    if (!IsValidPokemonItemIndex(_mainData.heldItem))
        return &POKEMON_ITEM_DATA[310]; // this is a bad item element
    return POKEMON_ITEM_LOOKUP[_mainData.heldItem];
}
pok_string Pokemon::GetNickname() const
{
    if (IsEgg())
    {
        // return egg string
        return "EGG";
    }
    return _nickname;
}
PokemonGameFromFlag Pokemon::GetGameFrom() const
{
    byte b = _mainData.levelCaught >> 7;
    b |= (_mainData.pokeBall_trainerGender & 0x07) << 1;
    return (PokemonGameFromFlag) b;
}
word Pokemon::GetIV(PokemonStat stat) const
{
    word iv;
    dword bitPattern = 31 << (5*stat);
    iv = (_mainData.ivs&bitPattern) >> (5*stat);
    return iv;
}
word Pokemon::GetLevel() const
{
    if (_statsCalculated)
        return _level;
    PokMath::StatCalculator calc(*this);
    return calc.CalcLevel();
}
word Pokemon::GetStat(PokemonStat stat) const
{
    if (_statsCalculated)
        return _calculatedStats.stats[stat];
    // calculate the stat value
    PokMath::StatCalculator calculator(*this);
    return calculator.CalcStat(stat);
}
Gender Pokemon::GetGender() const
{
    byte threshold = GetSpeciesBase()->genderThreshold;
    if (threshold==0xff)
        return Unknown;
    if ((_pid&0xff) >= threshold)
        return Male;
    return Female;
}
rbinstringstream Pokemon::ToBinaryStream(bool RecalcStats) const
{
    /*
        Note: when I invoke the output operator with a
        right-hand operand of type 'rtypes::str' the 
        stream will use the object's internal size rather than
        pushing all characters until null. There may be null characters
        in a 'str' object that need to be written to the binary stream.

        Note: all bytes are outputed here, even if the pokemon is not a party pokemon.
    */
    rbinstringstream bss;
    _PokDataBlock copy = _mainData; // copy main data for encryption
    // I cheat here with these non-const methods
    if (RecalcStats)
        const_cast<Pokemon*>(this)->CalculateStats(); // update party stats for party pokemon
    const_cast<Pokemon*>(this)->_calcChecksum(); // update checksum
    copy.encryptDecrypt( _getEncryptKey() );
    bss << _pid << _originalTrainerID << _nickname
        << _languageID << _originalTrainerName << _mark
        << _checksum << _padding1 << copy.getDataStreamWithOrder( _pid%DATA_BLOCK_ORDERS_COUNT ).get_device();
    if (_entryFlag==PartyPok)
        bss << _status << _level << _pokerusRemaining << _calculatedStats.getDataStream().get_device();
    return bss;
}
str Pokemon::ToString() const
{
    /*
        Text presentation of a Pokemon. For hexadecimal values, prepend a '0x'
            for added clarity.
    */
    rstringstream ss;
    pok_mark mark = GetMark();
    pp_ups ups = GetPPUps();
    // I cheat here on these non-const methods
    const_cast<Pokemon*>(this)->CalculateStats();
    const_cast<Pokemon*>(this)->_calcChecksum();
    ss.start_buffering_output();
    ss << "Pokemon Information for POK=" << GetNickname() << endline // call GetNickname for egg name support
        << "\tPIDNo.: 0x" << hexadecimal << _pid << decimal << endline
        << "\tSpecies: " << GetSpeciesBase()->speciesName << " [#" << GetSpeciesBase()->speciesIndexNo << ']' << endline
        << "\tOT: " << _originalTrainerName << " [" << ToString_Gender(GetOriginalTrainerGender()) << ']' << endline
        << "\tOT IDno.: " << GetOriginalTrainerID() << endline
        << "\tOT Secret IDno.: " << GetOriginalTrainerSecretID() << endline
        << "\tGame From: " << ToString_PokemonGameFromFlag(GetGameFrom()) << endline
        << "\tHeld Item: " << GetHeldItem()->itemName << endline
        << "\tGender: " << ToString_Gender(GetGender()) << endline
        << "\tNature: " << ToString_PokemonNature(GetNature()) << endline
        << "\tLocation Caught: " << GetLocationCaught()->locationName;
    if (GetLevelCaught()==0)
        ss << " [as egg]\n";
    else
        ss << " [at level " << GetLevelCaught() << ']' << endline;
    ss << "\tPokeBall Type: " << ToString_Ball(GetBallCaughtWith()) << endline
        << "\tIsShiny: " << (IsShiny() ? "True" : "False") << endline
        << (IsEgg() ? "\t[IsEgg]\n\tNumber of cycles until hatch: " : "\tFriendship Status: ") << GetFriendship() << "/255 [" << (word) PokMath::round(_mainData.friendship/255.00*100.00) << "%]" << endline;
    if (IsEgg())
        ss << "\t\tSteps until hatch: " << GetFriendship()*255 << endline;
    ss << "\tStatus Condition: " << (_entryFlag==PartyPok ? ToString_PokemonStatusCondition(GetStatusCondition()) : (_entryFlag==DayCarePok ? "In Day Care" : (_entryFlag==PCPok ? "In PC" : "Indeterminate"))) << endline
        << "\tMark: " << (!mark.circle && !mark.square && !mark.triangle && !mark.heart ? "Not Set" : "") << endline
        << (mark.circle ? "\t\t+circle\n" : "") << (mark.square ? "\t\t+square\n" : "") << (mark.triangle ? "\t\t+triangle\n" : "") << (mark.heart ? "\t\t+heart\n" : "")

        << "\tExperience: " << _mainData.experience << endline
        << "\tLevel: " << GetLevel() << endline

        << "\tEffort Values:\n";
    for (PokemonStat stat = HP;stat<=SpcDefense;((int&)stat)++)
        ss << "\t\t" << ToString_PokemonStat(stat) << " [" << GetEV(stat) << ']' << endline;
    ss << "\tIndividual Values:\n";
    for (PokemonStat stat = HP;stat<=SpcDefense;((int&)stat)++)
        ss << "\t\t" << ToString_PokemonStat(stat) << " [" << GetIV(stat) << ']' << endline;
    ss << "\tStats:\n";
    for (PokemonStat stat = HP;stat<=SpcDefense;((int&)stat)++)
        ss << "\t\t" << ToString_PokemonStat(stat) << " [" << GetStat(stat) << ']' << endline;
    ss << "\tMoveset:\n\t\t" << GetMove(1)->moveName << " (pp) " << GetPP(1) << " (pp ups) " << ups.move1Ups << "/3" << endline
        << "\t\t" << GetMove(2)->moveName << " (pp) " << GetPP(2) << " (pp ups) " << ups.move2Ups << "/3" << endline
        << "\t\t" << GetMove(3)->moveName << " (pp) " << GetPP(3) << " (pp ups) " << ups.move3Ups << "/3" << endline
        << "\t\t" << GetMove(4)->moveName << " (pp) " << GetPP(4) << " (pp ups) " << ups.move4Ups << "/3" << endline
        << endline;
    ss.stop_buffering_output();
    return ss.get_device();
}
str Pokemon::ToBytesString(bool RecalcStats) const
{
    rstringstream ss;
    rbinstringstream data = ToBinaryStream(RecalcStats);
    ss << hexadecimal;
    while (data.has_input())
    {
        byte b;
        data >> b;
        // rstream doesn't have width manipulation (yet)
        ss << (b<0x10 ? "0" : "") << (word)b << (data.get_input_iter_offset()%16==0 ? endline : " ");
    }
    return ss.get_device();
}
bool Pokemon::IsShiny() const
{
    const word idPart = GetOriginalTrainerID() ^ GetOriginalTrainerSecretID();
    word pidPart = (_pid&0xffff) ^ ((_pid>>16)&0xffff);
    word result = idPart ^ pidPart;
    return result<8;
}
void Pokemon::SetOriginalTrainerID(word ID,word SecretID)
{
    _originalTrainerID = dword(SecretID<<16);
    _originalTrainerID |= ID;
}
void Pokemon::SetNickname(const pok_string& s)
{
    dword sz;
    _nickname = s;
    sz = _nickname.size();
    _nickname.resize(10); // ensure length of 10
    for (dword i = sz;i<_nickname.size();i++)
        _nickname[i].theChar = 0xff; // blank out unused characters
}
void Pokemon::SetNickname(const PokemonBase* pBase)
{
    dword length = pBase->speciesName.length();
    const char* Nick = pBase->speciesName.c_str();
    _nickname.clear();
    for (dword i = 0;i<10;i++)
    {
        pok_char c;
        if (i<length)
        {
            char upper = Nick[i];
            if (upper>='a' && upper<='z')
            {// make uppercase (in keeping with the games' style)
                upper -= 'a';
                upper += 'A';
            }
            c = upper; // translates from ASCII to pokemon char
        }
        else
            c.theChar = 0xff;
        _nickname.push_back(c);
    }
}
void Pokemon::SetBallCaughtWith(Ball b)
{
    byte pattern = _mainData.pokeBall_trainerGender & 0x87;
    _mainData.pokeBall_trainerGender = b << 3;
    _mainData.pokeBall_trainerGender |= pattern;
}
void Pokemon::SetOriginalTrainerName(const pok_string& name)
{
    dword sz;
    _originalTrainerName = name;
    sz = _originalTrainerName.size();
    _originalTrainerName.resize(7); // ensure proper size
    for (dword i = sz;i<_originalTrainerName.size();i++)
        _originalTrainerName[i].theChar = 0xff; // blank the unused characters
}
void Pokemon::SetOriginalTrainerGender(Gender g)
{
    byte pattern = _mainData.pokeBall_trainerGender & 0x7f;
    _mainData.pokeBall_trainerGender = g<<7;
    _mainData.pokeBall_trainerGender |= pattern;
}
void Pokemon::SetGameFrom(PokemonGameFromFlag flag)
{
    byte pattern;
    pattern = _mainData.levelCaught & 0x7f;
    _mainData.levelCaught = ((flag&0x01)<<7);
    _mainData.levelCaught |= pattern;
    pattern = _mainData.pokeBall_trainerGender & 0xf8;
    _mainData.pokeBall_trainerGender = (flag>>1);
    _mainData.pokeBall_trainerGender |= pattern;
}
void Pokemon::SetPP(int MoveNumber,byte PPAmount)
{
    if (MoveNumber<1 || MoveNumber>4)
        return;
    _mainData.pp[MoveNumber-1] = PPAmount;
}
void Pokemon::SetEV(PokemonStat stat,byte evVal)
{
    _mainData.evs[stat] = evVal;
    _statsCalculated = false;
}
void Pokemon::SetIV(PokemonStat stat,byte ivVal)
{
    dword pattern = ((ivVal&0x1f)<<(stat*5));
    pattern |= _mainData.ivs & ~(0x1f<<(stat*5));
    _mainData.ivs = pattern;
    _statsCalculated = false;
}
void Pokemon::SetExperience(dword Exp)
{
    _mainData.experience = Exp;
    _statsCalculated = false;
}
bool Pokemon::SetLevel(byte Level)
{
    // I allow level 1 pokemon, level 0 would be valid too, but is non-standard
    // (or at least more non-standard than level 1!)
    if (Level<1 || Level>100)
        return false;
    _mainData.experience = EXP_LOOKUP_TABLE[GetSpeciesBase()->expGroup][Level-1];
    _level = Level;
    _statsCalculated = false;
    return true;
}
void Pokemon::SetNature(PokemonNature Nature)
{

    _statsCalculated = false;
}
void Pokemon::SetGender(Gender g)
{
    if (GetGender()==g || g==Unknown)
        return;

}
void Pokemon::SetObey(bool On)
{
    if (On)
        _mainData.ribbons |= 0x80000000;
    else
        _mainData.ribbons &= 0x7fffffff; // ~0x80000000
}
void Pokemon::SetShiny(bool On)
{
    const word idPart = GetOriginalTrainerID() ^ GetOriginalTrainerSecretID();
    word pidHigh = ((_pid>>16)&0xffff);
    const word pidLow = (_pid&0xffff);
    word pidPart = pidLow ^ pidHigh;
    word result = idPart ^ pidPart;
    if ((result<8 && On) || (result>=8 && !On))
        return; // already configured
    if (On)
    {
        // change bits in pid high to make shiny
        //  don't change gender portion of pid (pidLow)
        //  don't change trainer id (idPart)
        for (byte bitOffset = 0;bitOffset<13;bitOffset++) // note: bit offset is from bit 3
        {
            word pattern = 8; // prepare the bit pattern
            for (byte cnt = 0;cnt<bitOffset;cnt++)
                pattern <<= 1;
            if (result&pattern)
            {// the bit is on in the result; it needs to work out to be off
                //toggle the bit for pidHigh, which changes the XOR outcome for the current bit
                pidHigh ^= pattern;
            }
        }
    }
    else
    {
        pidHigh ^= 8;
    }
    // set pidHigh to pokemon's pid value
    _pid = (_pid&0xffff) | (pidHigh<<16);
}
void Pokemon::_calcChecksum()
{
    rbinstringstream bss = _mainData.getDataStream();
    _checksum = 0;
    while (bss.has_input())
    {
        word w;
        bss >> w;
        _checksum += w;
    }
}