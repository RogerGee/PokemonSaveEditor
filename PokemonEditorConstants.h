// PokemonEditorConstants.h
#ifndef POKEMONEDITORCONSTANTS_H
#define POKEMONEDITORCONSTANTS_H

#define SUB_G 0
#define SUB_A 1
#define SUB_E 2
#define SUB_M 3

namespace PokemonEditor
{
    // types for constants
    enum ExpGroup
    {
        /*
            This defines the standard order for exp group flags and needs to be maintained!
        */
        erratic,
        fast,
        medium_fast,
        medium_slow,
        slow,
        fluctuating,
        none
    };

    struct PokemonBase
    {
        PokemonBase(const char* Name,
            rtypes::word IndexNo,
            rtypes::word BaseHP,
            rtypes::word BaseAttack,
            rtypes::word BaseDefense,
            rtypes::word BaseSpcAttack,
            rtypes::word BaseSpcDefense,
            rtypes::word BaseSpeed,
            ExpGroup ExperienceGroup,
            byte EggCycleCount,
            byte GenderThreshold);

        rtypes::str speciesName;
        rtypes::word speciesIndexNo;
        rtypes::word baseStats[6];
        ExpGroup expGroup;
        byte eggCycles;
        byte genderThreshold;
    };

    struct PokemonMove
    {
        PokemonMove(const char* Name,
            rtypes::word Value) : moveName(Name),moveValue(Value) {}
        rtypes::str moveName;
        rtypes::word moveValue;
    };

    struct PokemonLocation
    {
        PokemonLocation(const char* Name,
            rtypes::word Value) : locationName(Name),locationValue(Value) {}
        rtypes::str locationName;
        rtypes::word locationValue;
    };

    struct PokemonItem
    {
        PokemonItem(const char* Name,
            rtypes::word Value) : itemName(Name),itemValue(Value) {}
        rtypes::str itemName;
        rtypes::word itemValue;
    };
    
    extern const rtypes::word LANGUAGE_ID;

    extern const rtypes::dword DATA_BLOCK_ORDERS_COUNT;
    extern const byte DATA_BLOCK_ORDERS[24][4];

    // mark constants
    extern const byte MARK_CIRCLE;
    extern const byte MARK_SQUARE;
    extern const byte MARK_TRIANGLE;
    extern const byte MARK_HEART;

    // pokemon data constants
    //      base info (lookup is POKEMON_BASE_DATA[PokemonSpeciesIndexNo])
    extern const rtypes::word MAX_POKEMON_INDEX;
    extern const PokemonBase POKEMON_BASE_DATA[412/*MAX_POKEMON_INDEX+1*/];
    extern const rtypes::word POKEMON_DEFAULT_MOVES[412][4];
    //      move info (lookup is POKEMON_MOVE_DATA[MoveNo])
    extern const rtypes::word MAX_POKEMON_MOVE;
    extern const PokemonMove POKEMON_MOVE_DATA[357/*MAX_POKEMON_MOVE+1*/];
    //      location info (lookup is POKEMON_LOCATION_LOOKUP[LocationNo])
    bool IsValidPokemonLocationIndex(rtypes::dword IndexNo);
    extern const PokemonLocation POKEMON_LOCATION_DATA[215];
    extern const rtypes::irregular_array<const PokemonLocation*> POKEMON_LOCATION_LOOKUP;
    //      item info (lookup is POKEMON_ITEM_LOOKUP[ItemNo])
    bool IsValidPokemonItemIndex(rtypes::dword ItemNo);
    extern const PokemonItem POKEMON_ITEM_DATA[311];
    extern const rtypes::irregular_array<const PokemonItem*> POKEMON_ITEM_LOOKUP;
    
    //exp data - lookup is EXP_LOOKUP_TABLE[ExpGroup][Level-1]
    extern const rtypes::dword EXP_LOOKUP_TABLE[6][100];

    extern const byte EGG_NICKNAME_BYTES[10]; // every egg appears to have this nickname
    extern const rtypes::word EGG_LANGUAGE_ID; // every egg has these two bytes as language id

    // function to load constants - should be called at program entry
    void LoadPokemonDataConstants();

    // constants application to save files
    extern const rtypes::dword SAVE_VALIDATION;
    extern const rtypes::dword SAVE_FILE_SIZE;
    extern const rtypes::dword SAVE_BLOCK_SIZE;
    extern const rtypes::dword SAVE_BLOCK_DATA_SIZE;
    extern const rtypes::dword SAVE_PC_MAX_COUNT; // the number of pokemon that can be stored in the PC
}

#endif