// PokemonEditorDialogs.h
#ifndef POKEMONEDITORDIALOGS_H
#define POKEMONEDITORDIALOGS_H

namespace PokemonEditorWindow
{
    class _MultilineLabel : public rtypes::rwin32::StaticLabel
    {
    protected:
        virtual rtypes::dword _getWinStyle() const { return WS_CHILD | WS_BORDER | SS_NOTIFY | SS_EDITCONTROL | WS_VISIBLE; };
    };

    class YesNoCancel_dialog : public rtypes::rwin32::DialogWindow
    {
    public:
        YesNoCancel_dialog(const rtypes::str& messageText,const rtypes::str& btnYesText = "",
            const rtypes::str& btnNoText = "",const rtypes::str btnCancelText = "");
        void ResetMessage(const rtypes::str& messageText) { _titles[1] = messageText; }
    protected:
        rtypes::str _titles[5]; // 0:DialogTitle 1:LabelText 2:yesBtn 3:noBtn 4:cancelBtn
        _MultilineLabel _lblMsg;
        rtypes::rwin32::Button _btnYes,_btnNo,_btnCancel;

        virtual void OnCreate(const rtypes::rwin32::EventData&);

        void _clkYes(rtypes::rwin32::Window*) { CloseDlg(Yes); }
        void _clkNo(rtypes::rwin32::Window*) { CloseDlg(No); }
        void _clkCancel(rtypes::rwin32::Window*) { CloseDlg(Cancel); }
    };

    class Message_dialog : public rtypes::rwin32::DialogWindow
    {
    public:
        Message_dialog(const rtypes::str& Message) : _message(Message) {}
    protected:
        rtypes::str _message;
        _MultilineLabel _lblMsg;
        rtypes::rwin32::Button _btnOK;

        virtual void OnCreate(const rtypes::rwin32::EventData&);

        void _clkOK(rtypes::rwin32::Window*) { CloseDlg(Cancel); }
    };

    class ListChooser_dialog : public rtypes::rwin32::DialogWindow
    {
    public:
        ListChooser_dialog(const char* Message) : _messageText(Message) {}
        rtypes::rwin32::ListBox theListBox;
    protected:
        rtypes::str _messageText;
        _MultilineLabel _lblMsg;
        rtypes::rwin32::Button _btnChoose,_btnCancel;

        virtual void OnCreate(const rtypes::rwin32::EventData&);

        void _clkChoose(rtypes::rwin32::Window*) { CloseDlg(); /* close w/ YES */ }
        void _clkCancel(rtypes::rwin32::Window*) { CloseDlg(Cancel); }
    };

    class baseDialog : public rtypes::rwin32::DialogWindow
    {
    public: 
        bool IsMinimized() const { return _isMinimized; }
        bool Minimize(); // returns true if the operation happened
        bool Restore(); // returns true if the operation happened

        // load/destroy calls for resources used by any object that derives baseDialog
        static void InitFonts();
        static void DestroyFonts();
        // provided so that other controls can set their font
        static void SetWindowFont(Window& wnd,bool useLargeFont) { SendMessage(wnd.WinHandle(),WM_SETFONT,(WPARAM)(useLargeFont?_dialogFontLg:_dialogFontSm),FALSE); }
    protected:
        baseDialog() : _isMinimized(false),_pSystemMenu(NULL) {}
        ~baseDialog() { callbackOnClose(); /* needed in case of force quits, which do not send close messages */ }

        void callbackOnCreate();
        void callbackOnClose();

        // if these are overloaded, it is the derived class's responsibility to call them at create and at close time
        virtual void OnCreate(const rtypes::rwin32::EventData&) { callbackOnCreate(); }
        virtual void OnClose(const rtypes::rwin32::EventData&) { callbackOnClose(); }
    private:
        static HFONT _dialogFontSm, _dialogFontLg;

        bool _isMinimized;

        rtypes::rwin32::MenuBar*        _pSystemMenu;
        rtypes::rwin32::MenuItem     _itemMinimizeRestore,
                                    _itemMinimizeRestoreToParent;

        void _setChildWindowFont();
        void _addSystemMenuItems();
        void _clkMinimizeRestore(rtypes::dword);
    };

    class PokemonEditor_dialog : public baseDialog
    {
    public:
        PokemonEditor_dialog(PokemonEditor::PokemonSaveFile* pSaveFile,const PokemonEditor::PokemonEntry& Entry);
        PokemonEditor_dialog(const char* pSaveFile,const PokemonEditor::PokemonEntry& Entry);
        
        void InitializeEntry(PokemonEditor::PokemonSaveFile* pSaveFile,const PokemonEditor::PokemonEntry& Entry);
        void InitializeEntry(const char* pSaveFile,const PokemonEditor::PokemonEntry& Entry);

        PokemonEditor::PokemonEntry GetEntry() const { return entry; } // get a copy of the entry that the dialog handles
        rtypes::dword GetEntryOffset() const { return entry.offset; }

        bool WasDataAltered() const { return dataAltered; }
        void FlagSaved() { dataAltered = false; }
        void FlagUnsaved() { dataAltered = true; }

        static bool CloseActiveInfoDialogs();
        static bool MinimizeActiveInfoDialogs();
        static bool RestoreActiveInfoDialogs();
    protected:
        virtual void OnCreate(const rtypes::rwin32::EventData&);
        virtual void OnClose(const rtypes::rwin32::EventData&);
    private:
        PokemonEditor_dialog() {} // rtypes::list<PokemonEditor_dialog> uses this for default init.

        bool dataAltered;
        bool cancelFlag; // if true, cancel putting the entry back into the file
        bool levelExpFlag, genericUpdateFlag;
        PokemonEditor::PokemonSaveFile* saveFile;
        rtypes::str saveFileName;
        PokemonEditor::PokemonEntry entry;

        PokemonEditor::Pokemon temp;

        // controls
        //      group boxes
        rtypes::rwin32::GroupBox    gboxGeneral,
                                    gboxStats,gboxModifiers,
                                    gboxMoves,
                                    gboxMisc;
        //      buttons
        rtypes::rwin32::Button  btnDone,
                                btnCancel,
                                btnView;
        //      lables
        rtypes::rwin32::StaticLabel lblOriginalTrainerRegularID,lblOriginalTrainerSecretID,
                                    lblOriginalTrainerName,
                                    lblOriginalTrainerGender,
                                    lblMark,
                                    lblSpecies,lblSpeciesIndex,
                                    lblNickname,
                                    lblBallCaughtWith,
                                    lblGameFrom,
                                    lblHeldItem,
                                    lblPPUpsTitle, /*lblPPUps[4],*/
                                    lblPPTitle, /*lblPPMove[4],*/
                                    lblEVsTitle,
                                    lblIVsTitle, /*lblIVStat[6],*/
                                    lblModStat[6], // use these for general stat modifier labels
                                    lblLevel,
                                    lblLevelCaught,
                                    lblPartyLevel,
                                    lblLocationCaught,
                                    lblPartyStatTitle,
                                    lblContestStatTitle, /*lblContestStats[6],*/
                                    lblNature,
                                    lblGender,
                                    lblFriendship;
        _MultilineLabel lblStats/*used for displaying actual calculated stats*/;
        //      comboBoxes
        rtypes::rwin32::DropDownComboBox    cboxSpecies,
                                            cboxBallCaughtWith, // conv. from Ball to index = Ball-1
                                            cboxGameFrom,
                                            cboxHeldItem,
                                            cboxLocationCaught,
                                            cboxNature,
                                            cboxTechs[4];
        //      textBoxes
        rtypes::rwin32::TextBox txtOriginalTrainerRegularID,txtOriginalTrainerSecretID,
                                txtOriginalTrainerName,
                                txtNickname,
                                txtPPUps[4],
                                txtPPMove[4],
                                txtEVStat[6],
                                txtIVStat[6],
                                txtLevel,
                                txtLevelCaught,
                                txtPartyStat[6],
                                txtContestStat[6],
                                txtFriendship,
                                txtExperience,
                                txtPartyLevel;
        rtypes::rwin32::MultilineTextBox    txtInfoDisplay;
        //      checkBoxes
        rtypes::rwin32::CheckBoxButton  chkbxMarkCircle, chkbxMarkSquare, chkbxMarkTriangle, chkbxMarkHeart,
                                        chkbxObeyBit,
                                        chkbxShiny;
        //      radioButtons
        rtypes::rwin32::RadioButton rbtnGenderMale, // pokemon gender
                                    rbtnGenderFemale,
                                    rbtnGenderUnknown,
                                    rbtnTGenderMale, // trainer gender
                                    rbtnTGenderFemale;

        void _initControls();
        void _positionControls();

        // event handlers
        void _genericUpdate(Window*); // callback for control info updated
        void _clkDone(Window*);
        void _clkCancel(Window*);
        void _clkInfo(Window*);
        void _selchngSpecies(Window*);
        void _levelUpdate(Window*); // callback for level info updated
        void _expUpdate(Window*); // callback for experience info updated
        void _statUpdate(Window*);

        // helpers
        rtypes::str _getStatString() const;
        void _downloadEntry(); // load the entry to the control interface
        void _uploadEntry(); // load the control interface data into the entry
        void _uploadTemp(Window*);
        void _updateInfoDisplay();

        friend class rtypes::list<PokemonEditor_dialog>;
    };

    // note: all dialogs should close opened pokemon by putting the entries back into the save file object
    class PokemonListing_dialog : public baseDialog
    {
    public:
        PokemonListing_dialog(PokemonEditor::PokemonSaveFile* pSaveFile);

        /*
            This method allows open PokemonEditor_dialog objects to close normally and perform
            closing actions. The listing dialog itself does not have to techically be running
            since it passes all of its PokemonEditor_dialogs to its parent window.
                Return values:
                    true - all dialogs closed normally,
                    false - one or more dialogs canceled the close action
        */
        bool CloseActiveDialogs();
        
        bool MinimizeActiveDialogs();
        bool RestoreActiveDialogs();
        void SetSaveStateType(PokemonEditor::SaveFileState state) { saveStateType = state; }
        void ResetListing();
    protected:
        virtual void OnCreate(const rtypes::rwin32::EventData&);
    private:
        PokemonEditor::SaveFileState saveStateType;
        PokemonEditor::PokemonSaveFile* saveFile;
        rtypes::list<rtypes::str> entries; // "shallow entries" that aren't outstanding
        rtypes::list<PokemonEditor_dialog> dialogs;
        rtypes::rwin32::ListBox lboxLoadedEntries;
        rtypes::rwin32::Button btnRefreshEntries, btnOpenPokemon;

        // event handling routines
        void _clkRefreshEntries(Window*);
        void _clkOpenPokemon(Window*);

        // helpers
        void _refreshEntries();
    };
    
    class PokemonInfo_dialog : public baseDialog
    {
    public:
        PokemonInfo_dialog() { InitializeDialog(NULL,NULL); }

        void InitializeDialog(const char* title,const char* txtContents);
    protected:
        virtual void OnCreate(const rtypes::rwin32::EventData&); // needs to call baseDialog::callbackOnCreate
    private:
        rtypes::rwin32::MultilineTextBox box;
        rtypes::rwin32::Button btnDone;
        const char* title, *txt; // this pointer should be set to NULL after the text box loads the data it points to

        void _clkDone(rtypes::rwin32::Window*);
    };

    class NewPokemon_dialog : public baseDialog
    {// this dialog is responsible for obtaining infomation for a new pokemon and starting an unrouted PokemonEditor_dialog object for the Pokemon
    public:
        NewPokemon_dialog(PokemonEditor::PokemonSaveFile* ptrSaveFile,PokemonEditor::SaveFileState stateType) : pSaveFile(ptrSaveFile),saveStateType(stateType) {}

        static bool MinimizeActiveDialogs();
        static bool RestoreActiveDialogs();
        static bool CloseActiveDialogs();
    protected:
        virtual void OnCreate(const rtypes::rwin32::EventData&);
    private:
        PokemonEditor::PokemonSaveFile* pSaveFile;
        PokemonEditor::SaveFileState saveStateType;

        rtypes::rwin32::GroupBox        gboxDestination;
        _MultilineLabel lblDescript;
        rtypes::rwin32::StaticLabel     lblSpecies,
                                        lblGameFrom,
                                        lblTrainerName,
                                        lblTrainerIDReg,
                                        lblTrainerIDSec,
                                        lblTrainerGender,
                                        lblLocationCaught,
                                        lblNickname,
                                        lblIniLevel,
                                        lblPath;
        rtypes::rwin32::DropDownComboBox    cboxSpecies,
                                            cboxGameFrom,
                                            cboxLocationCaught;
        rtypes::rwin32::RadioButton     rbtnTrainerGenderMale,
                                        rbtnTrainerGenderFemale,
                                        rbtnBatteryFile,
                                        rbtnExternalFile;
        rtypes::rwin32::CheckBoxButton  chkbxIsEgg;
        rtypes::rwin32::TextBox     boxTrainerName,
                                    boxTrainerIDReg,
                                    boxTrainerIDSec,
                                    boxNickname,
                                    boxIniLevel,
                                    boxAddress;
        rtypes::rwin32::Button  btnBrowse;
        rtypes::rwin32::Button  btnCreate,
                                btnCancel;

        void _initControls();
        void _clkCreate(rtypes::rwin32::Window*);
        void _clkCancel(rtypes::rwin32::Window*) { CloseDlg(No); }
        void _clkBrowse(rtypes::rwin32::Window*);
        void _clkRBtnFileType(rtypes::rwin32::Window*);
    };
}

#endif