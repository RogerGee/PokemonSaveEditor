// PokemonEditorMainWindow.h
#ifndef POKEMONEDITORMAINWINDOW_H
#define POKEMONEDITORMAINWINDOW_H

namespace PokemonEditorWindow
{
    extern const char* const PokemonEditorTitle;

    enum MainWindowClickSimulation
    {
        HideConsoleClick,
        RequestCloseClick,
        SaveBatteryFileClick
    };

    class MainWindow : public rtypes::rwin32::Window
    {
    public:
        MainWindow();
        ~MainWindow();

        bool LoadBatteryFile(const rtypes::str& fileName);
        bool LoadEntryFile(const rtypes::str& fileName);
        bool LoadEntry(PokemonEditor::PokemonEntryType,rtypes::dword);
        void SetFileState(PokemonEditor::SaveFileState state) { saveFileState = state; }
        void SimulateClick(MainWindowClickSimulation);
    protected:
        virtual void OnClose(const rtypes::rwin32::EventData&);
        virtual void OnResized(const rtypes::rwin32::WinChngEventData&);

    private:
        void _initControls();

        // save file information
        PokemonEditor::PokemonSaveFile saveFile;
        PokemonEditor::SaveFileState saveFileState; // note: when this is changed, it must be reset in the listing dialog object
        rtypes::str saveFileName;

        // dialogs
        PokemonListing_dialog listingDialog;
        rtypes::list<PokemonEditor_dialog> activeEntryDialogs;
        PokemonEditor_dialog& _getInActiveEntryDialog();

        // menu objects
        rtypes::rwin32::MenuBar menuBar;
        rtypes::rwin32::MenuItem
            itemFile,
                itemOpenBatteryFile,
                itemOpenElement,
                // sep
                itemSaveBatteryFile,
                itemSaveAll,
                itemSaveBatteryFileAs,
                itemSaveElementAs,
                // sep
                itemCloseBatteryFile,
                // sep
                itemExit,
            itemEdit,
                itemCopy,
                itemPaste,
                itemCut,
                itemDelete,
                // sep
                itemFind,
            itemView,
                itemPokemonListing,
                itemBatteryFileProperties,
                
            itemPokemon,
                itemNewPokemon,
                // A lot of menu items go here!!
                itemPartySub,
                itemPartyPoks[6],
                itemDayCareSub,
                itemDayCarePoks[3],
                itemPCSub,
                itemPCBoxSub[14],
                itemPCPoks[14][30],
            itemOps,
            itemWindow,
                itemConsole,
                itemHideConsole,
                //sep
                itemNewWindow,
                itemCloseActiveWindows,
            itemHelp;

        // event handles for menu events
        void _clkOpenBatteryFile(rtypes::dword);
        void _clkOpenElement(rtypes::dword);
        void _clkSaveBatteryFile(rtypes::dword);
        void _clkSaveBatteryFileAs(rtypes::dword);
        void _clkSaveElementAs(rtypes::dword);
        void _clkCloseBatteryFile(rtypes::dword);
        void _clkExit(rtypes::dword);
        void _clkPokemonListing(rtypes::dword);
        void _clkNewPokemon(rtypes::dword);
        void _clkPokemonEntry(rtypes::dword);
        void _clkConsole(rtypes::dword);
        void _clkHideConsole(rtypes::dword);
        void _clkNewWindow(rtypes::dword);
        void _clkCloseActiveWindows(rtypes::dword);

        // misc. helpers
        bool _closeActiveDialogs(); // returns false if one window requested to stall termination
        bool _minimizeActiveDialogs();
        bool _restoreActiveDialogs();

    };
}

#endif