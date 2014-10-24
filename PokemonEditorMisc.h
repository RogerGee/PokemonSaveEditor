//PokemonEditorMisc.h
#ifndef POKEMONEDITORMISC_H
#define POKEMONEDITORMISC_H

namespace PokemonEditor
{
    namespace PokMath
    {
        // helper types
        class StatCalculator
        {
        public:
            StatCalculator(const Pokemon& p) : _pok(&p) {}
            rtypes::word CalcLevel() const;
            rtypes::word CalcStat(PokemonStat stat) const;
        private:
            bool _natureDoesIncrease(PokemonStat) const;
            bool _natureDoesDecrease(PokemonStat) const;
            const Pokemon* _pok;
        };

        // random number functions
        int GetRandomInt();
        void SeedRandomGenerator();
        template<class Numeric>
        Numeric GetRandom()
        {
            Numeric random = 0;
            for (int b = 0;b<sizeof(Numeric);b++)
            {
                for (int bit = 0;bit<8;bit++)   
                {
                    int r = GetRandomInt();
                    if (r%2==0)
                        random |= 1;
                    random <<= 1;
                }
            }
            return random;
        }

        // misc. mathematical functions
        template<class Numeric>
        Numeric power(Numeric nBase,Numeric nPower)
        {
            if (nPower==0)
                return 1;
            if (nPower==1)
                return nBase;
            return nBase*power(nBase,nPower-1);
        }
        double round(double);
    }

    // "to string" functions for enumerated constants
    const char* ToString_PokemonStat(PokemonStat);
    const char* ToString_PokemonStat_abbr(PokemonStat);
    const char* ToString_ContestStat(ContestStat);
    const char* ToString_ContestStat_abbr(ContestStat);
    const char* ToString_PokemonNature(PokemonNature);
    const char* ToString_Gender(Gender);
    const char* ToString_Ball(Ball);
    const char* ToString_PokemonStatusCondition(PokemonStatusCondition);
    const char* ToString_PokemonGameFromFlag(PokemonGameFromFlag);
    const char* ToString_PokemonGameFromFlag_abbr(PokemonGameFromFlag);

    namespace Misc
    {
        // useful conversion routines
        rtypes::wstr to_wide_char_str(const rtypes::str&);
        void to_lower(rtypes::str&);
    }

}

#endif