#include "MasterInclude.h"
#include <ctime>
#include <cstdlib>
using namespace std;
using namespace rtypes;
using namespace PokemonEditor;
using namespace PokemonEditor::PokMath;

double PokMath::round(double d)
{
    double asInt = (int) d;
    if (d-asInt >= 0.5)
        return asInt+1.00;
    return asInt;
}

bool StatCalculator::_natureDoesIncrease(PokemonStat stat) const
{
    PokemonNature n = _pok->GetNature();
    return ((stat==Attack && n>=Hardy && n<=Naughty)
        ||(stat==Defense && n>=Bold && n<=Lax)
            ||(stat==Speed && n>=Timid && n<=Naive)
                ||(stat==SpcAttack && n>=Modest && n<=Rash)
                    || (stat==SpcDefense && n>=Calm && n<=Quirky));
}
bool StatCalculator::_natureDoesDecrease(PokemonStat stat) const
{
    PokemonNature n = _pok->GetNature();
    return ((n-Hardy+1==stat && n>=Hardy && n<=Naughty)
        ||(n-Bold+1==stat && n>=Bold && n<=Lax)
            ||(n-Timid+1==stat && n>=Timid && n<=Naive)
                ||(n-Modest+1==stat && n>=Modest && n<=Rash)
                    || (n-Calm+1==stat && n>=Calm && n<=Quirky));
}

word StatCalculator::CalcLevel() const
{
    word levelIndex;
    ExpGroup group = _pok->GetSpeciesBase()->expGroup;
    dword exp = _pok->GetExperience();
    // go through the lookup tables for the exp group and find the current level
    if (group!=erratic && group!=fast && group!=medium_fast && group!=medium_slow && group!=slow && group!=fluctuating)
        return 0; // bad group or none
    for (levelIndex = 0;levelIndex<100;levelIndex++)
        if (EXP_LOOKUP_TABLE[group][levelIndex]>exp)
            break;
    return levelIndex; // yields level
}
word StatCalculator::CalcStat(PokemonStat stat) const
{
    // special cases
    if (_pok->GetSpecies()==0)
        return 0;
    if (_pok->GetSpecies()==303 && stat==HP)
        return 1; // Shedinja
    //
    const PokemonBase* pokBase = _pok->GetSpeciesBase();
    word value, level = CalcLevel();
    // hp uses a different formula then other stats
    if (stat==HP)
        value = (_pok->GetIV(stat) + 2*pokBase->baseStats[stat] + _pok->GetEV(stat)/4 + 100) * level / 100 + 10;
    else
    {
        value = (_pok->GetIV(stat) + 2*pokBase->baseStats[stat] + _pok->GetEV(stat)/4) * level / 100 + 5;
        // nature is factored into non-hp stats
        bool dec = _natureDoesDecrease(stat), inc = _natureDoesIncrease(stat);
        if ((!dec || !inc) && (dec || inc))
        {// the nature applies
            if (dec)
            {
                value *= 9; // 90% of original
                value /= 10;
            }
            else if (inc)
            {
                value *= 11; // 110% of original
                value /= 10;
            }
        }
    }
    return value;
}

void PokMath::SeedRandomGenerator()
{
    srand( (unsigned int) time(NULL) );
}
int PokMath::GetRandomInt()
{// wrapper for c++ std. lib. random # generator
    return rand();
}

const char* PokemonEditor::ToString_Ball(Ball b)
{
    switch (b)
    {
    case MasterBall:
        return "Master Ball";
    case UltraBall:
        return "Ultra Ball";
    case GreatBall:
        return "Great Ball";
    case PokeBall:
        return "Poke Ball";
    case SafariBall:
        return "Safari Ball";
    case NetBall:
        return "Net Ball";
    case DiveBall:
        return "Dive Ball";
    case NestBall:
        return "Nest Ball";
    case RepeatBall:
        return "Repeat Ball";
    case TimerBall:
        return "Timer Ball";
    case LuxeryBall:
        return "Luxery Ball";
    case PremierBall:
        return "Premier Ball";
    default:
        return "BAD_BALL";
    }
}
const char* PokemonEditor::ToString_ContestStat(ContestStat s)
{
    switch (s)
    {
    case Coolness:
        return "Coolness";
    case Beauty:
        return "Beauty";
    case Cuteness:
        return "Cuteness";
    case Smartness:
        return "Smartness";
    case Toughness:
        return "Toughness";
    case Feel:
        return "Feel";
    default:
        return "BAD_CSTAT";
    }
}
const char* PokemonEditor::ToString_ContestStat_abbr(ContestStat s)
{
    switch (s)
    {
    case Coolness:
        return "Cool";
    case Beauty:
        return "Beau";
    case Cuteness:
        return "Cute";
    case Smartness:
        return "Smar";
    case Toughness:
        return "Tness";
    case Feel:
        return "Feel";
    default:
        return "BADC";
    }
}
const char* PokemonEditor::ToString_Gender(Gender g)
{
    switch (g)
    {
    case Male:
        return "Male";
    case Female:
        return "Female";
    case Unknown:
        return "Unknown";
    default:
        return "BAD_GEN";
    }
}
const char* PokemonEditor::ToString_PokemonNature(PokemonNature n)
{
    switch (n)
    {
    case Hardy:
        return "Hardy";
    case Lonely:
        return "Lonely";
    case Brave:
        return "Brave";
    case Adament:
        return "Adament";
    case Naughty:
        return "Naughty";
    case Bold:
        return "Bold";
    case Docile:
        return "Docile";
    case Relaxed:
        return "Relaxed";
    case Impish:
        return "Impish";
    case Lax:
        return "Lax";
    case Timid:
        return "Timid";
    case Hasty:
        return "Hasty";
    case Serious:
        return "Serious";
    case Jolly:
        return "Jolly";
    case Naive:
        return "Naive";
    case Modest:
        return "Modest";
    case Mild:
        return "Mild";
    case Quiet:
        return "Quiet";
    case Bashful:
        return "Bashful";
    case Rash:
        return "Rash";
    case Calm:
        return "Calm";
    case Gentle:
        return "Gentle";
    case Sassy:
        return "Sassy";
    case Careful:
        return "Careful";
    case Quirky:
        return "Quirky";
    default:
        return "BAD_NATURE";
    }
}
const char* PokemonEditor::ToString_PokemonStat(PokemonStat s)
{
    switch (s)
    {
    case HP:
        return "HP";
    case Attack:
        return "Attack";
    case Defense:
        return "Defense";
    case Speed:
        return "Speed";
    case SpcAttack:
        return "Special Attack";
    case SpcDefense:
        return "Special Defense";
    default:
        return "BAD_STAT";
    }
}
const char* PokemonEditor::ToString_PokemonStat_abbr(PokemonStat s)
{
    switch (s)
    {
    case HP:
        return "HP";
    case Attack:
        return "Atck";
    case Defense:
        return "Defn";
    case Speed:
        return "Spd";
    case SpcAttack:
        return "SpcA";
    case SpcDefense:
        return "SpcD";
    default:
        return "BADS";
    }
}
const char* PokemonEditor::ToString_PokemonStatusCondition(PokemonStatusCondition c)
{
    switch (c)
    {
    case OK:
        return "OK";
    case Sleep1:
        return "Sleep (1 turn)";
    case Sleep2:
        return "Sleep (2 turns)";
    case Sleep3:
        return "Sleep (3 turns)";
    case Sleep4:
        return "Sleep (4 turns)";
    case Sleep5:
        return "Sleep (5 turns)";
    case Sleep6:
        return "Sleep (6 turns)";
    case MaxSleep:
        return "Sleep (max)";
    case Poisoned:
        return "Poison";
    case Burned:
        return "Burn";
    case Frozen:
        return "Freeze";
    case Paralysis:
        return "Paralysis";
    case BadPoison:
        return "Bad Poison";
    default:
        return "BAD_STATCON";
    }
}
const char* PokemonEditor::ToString_PokemonGameFromFlag(PokemonGameFromFlag flag)
{
    switch (flag)
    {
    case DemoJirachiDisk:
        return "Demo Jirachi Disk";
    case PokemonSapphire:
        return "Pokemon Sapphire Version";
    case PokemonRuby:
        return "Pokemon Ruby Version";
    case PokemonEmerald:
        return "Pokemon Emerald Version";
    case PokemonFireRed:
        return "Pokemon Fire Red Version";
    case PokemonLeafGreen:
        return "Pokemon Leaf Green Version";
    case PokemonColosseum:
        return "Pokemon Colosseum";
    default:
        return "BAD_GAMEFROM";
    }
}
const char* PokemonEditor::ToString_PokemonGameFromFlag_abbr(PokemonGameFromFlag flag)
{
    // abbrieviated for save file game from approximations
    switch (flag)
    {
    case DemoJirachiDisk:
        return "Demo";
    case PokemonSapphire:
    case PokemonRuby:
        return "RS";
    case PokemonEmerald:
        return "Emer";
    case PokemonFireRed:
    case PokemonLeafGreen:
        return "FrLg";
    default:
        return "BadGF";
    }
}

wstr PokemonEditor::Misc::to_wide_char_str(const str& s)
{
    wstr r;
    for (dword i = 0;i<s.length();i++)
        r.push_back( (wchar_t) s[i] );
    return r;
}
void PokemonEditor::Misc::to_lower(str& s)
{
    for (dword i = 0;i<s.size();i++)
        if (s[i]>='A' && s[i]<='Z')
        {
            s[i] -= 'A';
            s[i] += 'a';
        }
}