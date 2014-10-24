// PokemonEditorConsole.h
#ifndef POKEMONEDITORCONSOLE_H
#define POKEMONEDITORCONSOLE_H

namespace PokemonEditorWindow
{
    namespace PokemonEditorConsole
    {
        bool IsConsoleInit();
        void InitConsole();
        void SetDataForConsole(PokemonEditor::PokemonSaveFile*,MainWindow*);
        void FocusConsole();
        void PackUpConsole();
        void ConsoleCleanup(); // call this before the program quits
    }
}

#endif