#include "MasterInclude.h"
using namespace rtypes;
using namespace rtypes::rwin32;
using namespace PokemonEditor;
using namespace PokemonEditorWindow;

namespace
{
    template<class T>
    T getNumericFromTBox(const TextBox& tBox)
    {
        str txt = tBox.GetText();
        rstringstream ss(txt);
        T r;
        if (!(ss >> r))
            r = 0;
        return r;
    }

    template<class T>
    void putNumericInTBox(TextBox& tBox,T val)
    {
        rstringstream ss;
        ss << val;
        tBox.SetText( ss.get_device() );
    }

    Button::ButtonState bstateCast_safe(bool b)
    {
        if (b)
            return Button::Checked;
        return Button::Unchecked;
    }

    // PokemonEditor_dialog info windows
    // These are static so that they can exist after the dialogs have closed
    list<PokemonInfo_dialog> infoDialogs;
    PokemonInfo_dialog& GetInActiveInfoDialogObject()
    {
        dword i = 0;
        while (i<infoDialogs.length() && infoDialogs[i].WinHandle()!=NULL)
            i++;
        if (i>=infoDialogs.length())
            infoDialogs.add();
        return infoDialogs[i];
    }

    // PokemonEditor_dialog dialog windows
    // The listing dialog keeps these stored internally, but other dialogs need them as well
    list<PokemonEditor_dialog> editorDialogs;
    PokemonEditor_dialog& GetInActiveEditorDialogObject()
    {
        dword i = 0;
        while (i<editorDialogs.length() && editorDialogs[i].WinHandle()!=NULL)
            i++;
        if (i>=editorDialogs.length())
            editorDialogs.add();
        return editorDialogs[i];
    }
}

// (don't worry about return values)
bool PokemonEditor_dialog::CloseActiveInfoDialogs()
{
    for (dword i = 0;i<::infoDialogs.length();i++)
        if (::infoDialogs[i].WinHandle()!=NULL)
            ::infoDialogs[i].Close();
    return true;
}
bool PokemonEditor_dialog::MinimizeActiveInfoDialogs()
{
    for (dword i = 0;i<::infoDialogs.length();i++)
        if (::infoDialogs[i].WinHandle()!=NULL)
            ::infoDialogs[i].Minimize();
    return true;
}
bool PokemonEditor_dialog::RestoreActiveInfoDialogs()
{
    for (dword i = 0;i<::infoDialogs.length();i++)
        if (::infoDialogs[i].WinHandle()!=NULL)
            ::infoDialogs[i].Restore();
    return true;
}

YesNoCancel_dialog::YesNoCancel_dialog(const rtypes::str& messageText,
    const rtypes::str& btnYesText,const rtypes::str& btnNoText,const rtypes::str btnCancelText)
{
    _titles[1] = messageText;
    _titles[2] = btnYesText;
    _titles[3] = btnNoText;
    _titles[4] = btnCancelText;
}
void YesNoCancel_dialog::OnCreate(const EventData&)
{
    SetAttribute(TitleBar,true);
    SetSize(325,195);
    SetText(_titles[0]);
    _lblMsg.Create(11,13,300,128,this);
    _btnYes.CreateButton(this);
    _btnNo.CreateButton(this);
    _btnCancel.CreateButton(this);
    _btnYes.SetSize(90,25);
    _btnNo.SetSize(90,25);
    _btnCancel.SetSize(90,25);
    _btnYes.SetLocation(10,150);
    _btnNo.SetLocation(115,150);
    _btnCancel.SetLocation(220,150);
    _btnYes.SetText( (_titles[2].length()>0 ? _titles[2] : str("Yes")) );
    _btnNo.SetText( (_titles[3].length()>0 ? _titles[3] : str("No")) );
    _btnCancel.SetText( (_titles[4].length()>0 ? _titles[4] : str("Cancel")) );
    _lblMsg.SetText(_titles[1]);
    _btnYes.SetClickEvent( static_cast<EventDelegate> (&YesNoCancel_dialog::_clkYes) );
    _btnNo.SetClickEvent( static_cast<EventDelegate> (&YesNoCancel_dialog::_clkNo) );
    _btnCancel.SetClickEvent( static_cast<EventDelegate> (&YesNoCancel_dialog::_clkCancel) );
}

void Message_dialog::OnCreate(const EventData&)
{
    SetAttribute(TitleBar,true);
    SetSize(325,195);
    //SetText("MessageDialog");
    _lblMsg.Create(11,13,300,128,this);
    _lblMsg.SetText(_message);
    _btnOK.CreateButton(this);
    _btnOK.SetLocation(220,150);
    _btnOK.SetSize(90,25);
    _btnOK.SetText("Okay");
    _btnOK.SetClickEvent( static_cast<EventDelegate> (&Message_dialog::_clkOK) );
}

void ListChooser_dialog::OnCreate(const EventData&)
{
    Size szClient;
    SetAttribute(TitleBar,true);
    SetSize(400,330);
    szClient = GetClientSize();
    _lblMsg.Create(0,0,szClient.width,100,this);
    _lblMsg.SetText(_messageText);
    theListBox.Create(0,100,szClient.width,170,this);
    // 170, 295
    _btnChoose.Create(170,295,50,21,this);
    _btnChoose.SetText("Choose");
    _btnChoose.SetClickEvent( static_cast<EventDelegate> (&ListChooser_dialog::_clkChoose) );
    _btnCancel.Create(225,295,50,21,this);
    _btnCancel.SetText("Cancel");
    _btnCancel.SetClickEvent( static_cast<EventDelegate> (&ListChooser_dialog::_clkCancel) );
}

HFONT baseDialog::_dialogFontSm = NULL;
HFONT baseDialog::_dialogFontLg = NULL;
void baseDialog::InitFonts()
{
    _dialogFontSm = CreateFont(13,0,0,0,375,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,FF_DONTCARE,"Consolas");
    _dialogFontLg = CreateFont(14,0,0,0,450,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,FF_DONTCARE,"Consolas");
}
void baseDialog::DestroyFonts()
{
    DeleteObject(_dialogFontSm);
    DeleteObject(_dialogFontLg);
    _dialogFontSm = NULL;
    _dialogFontLg = NULL;
}

bool baseDialog::Minimize()
{
    if (!_isMinimized && Show(SW_MINIMIZE))
    {
        _isMinimized = true;
        _itemMinimizeRestore.SetText("&Restore");
        _itemMinimizeRestoreToParent.SetText("Res&tore to parent");
        return true;
    }
    return false;
}
bool baseDialog::Restore()
{
    if (_isMinimized && Show(SW_RESTORE))
    {
        _isMinimized = false;
        _itemMinimizeRestore.SetText("Mi&nimize");
        _itemMinimizeRestoreToParent.SetText("Minimi&ze to parent");
        return true;
    }
    return false;
}
void baseDialog::callbackOnCreate()
{
    _setChildWindowFont();
    _addSystemMenuItems();
}
void baseDialog::callbackOnClose()
{
    if (_pSystemMenu!=NULL)
    {
        delete _pSystemMenu;
        _pSystemMenu = NULL;
    }
}
void baseDialog::_setChildWindowFont()
{
    for (dword i = 0;i<GetNumberOfChildWindows();i++)
    {
        const Window* child = GetChildWindow(i);
        HFONT hMonospaceFont = (_dialogFontSm==NULL || _dialogFontLg==NULL ? (HFONT) GetStockObject(SYSTEM_FIXED_FONT) : (child->MyType()==StaticControl ? _dialogFontSm : _dialogFontLg));
        SendMessage(child->WinHandle(),
            WM_SETFONT,
            (WPARAM)hMonospaceFont,
            FALSE);
    }
}
void baseDialog::_addSystemMenuItems()
{
    if (_pSystemMenu==NULL)
    {
        _pSystemMenu = new MenuBar( GetSystemMenuHandle() );
        _itemMinimizeRestore.SetText("Mi&nimize");
        _itemMinimizeRestore.SetEvent( static_cast<EventDelegate_id> (&baseDialog::_clkMinimizeRestore) );
        _itemMinimizeRestoreToParent.SetText("Minimi&ze to parent");
        _itemMinimizeRestoreToParent.SetEvent( static_cast<EventDelegate_id> (&baseDialog::_clkMinimizeRestore) );
        _pSystemMenu->InsertItem(_itemMinimizeRestore,0);
        _pSystemMenu->InsertItem(_itemMinimizeRestoreToParent,1);
        _pSystemMenu->InsertSeparator(2);
        _pSystemMenu->SetWindow(*this); // rwin32 needs this for the menu callback event
    }
}
void baseDialog::_clkMinimizeRestore(dword id)
{
    if (_isMinimized)
        Restore();
    else
        Minimize();
    if (id==_itemMinimizeRestoreToParent.GetID())
    {
        const Window* par = GetParentWin();
        if (par==NULL)
            par = GetParentWinFromOS(true);
        if (par!=NULL)
            SetLocation( par->GetLocation() );
    }
}

PokemonListing_dialog::PokemonListing_dialog(PokemonEditor::PokemonSaveFile* pSaveFile)
{
    saveFile = pSaveFile;
    _refreshEntries();
}
bool PokemonListing_dialog::CloseActiveDialogs()
{
    bool r = true;
    for (dword i = 0;i<dialogs.size();i++)
    {
        if (dialogs[i].WinHandle()!=NULL)
        {
            dialogs[i].FlashWindow(FlashCaption,5);
            if (!dialogs[i].Close())
                r = false;
        }
    }
    return r;
}
bool PokemonListing_dialog::MinimizeActiveDialogs()
{
    bool b = true;
    for (dword i = 0;i<dialogs.length();i++)
    {
        if (dialogs[i].WinHandle()!=NULL && !dialogs[i].Minimize())
            b = false;
    }
    return b;
}
bool PokemonListing_dialog::RestoreActiveDialogs()
{
    bool b = true;
    for (dword i = 0;i<dialogs.length();i++)
    {
        if (dialogs[i].WinHandle()!=NULL && !dialogs[i].Restore())
            b = false;
    }
    return b;
}
void PokemonListing_dialog::ResetListing()
{
    entries.empty();
    if (WinHandle()!=NULL)
        lboxLoadedEntries.RemoveAllItems();
}
void PokemonListing_dialog::OnCreate(const EventData& data)
{
    int i;
    Size mySize(400,600);
    SetText("Pokemon Search Listing");
    SetSize(mySize);
    mySize = GetClientSize();
    lboxLoadedEntries.CreateListBox(this);
    btnRefreshEntries.CreateButton(this);
    btnOpenPokemon.CreateButton(this);
    lboxLoadedEntries.SetAttribute(HorizontileScroll);
    lboxLoadedEntries.SetAttribute(VerticalScroll);
    // tell the dialog base to perform some default init. on the dialog and its child windows
    callbackOnCreate();
    // set locations and sizes
    btnRefreshEntries.SetLocation(10,5);
    btnOpenPokemon.SetText("Open Pokemon");
    btnOpenPokemon.FitWidthToText();
    btnOpenPokemon.SetLocation(mySize.width-btnOpenPokemon.GetSize().width-10,5);
    btnRefreshEntries.SetText("Refresh");
    btnRefreshEntries.SetClickEvent( static_cast<EventDelegate> (&PokemonListing_dialog::_clkRefreshEntries) );
    btnOpenPokemon.SetClickEvent( static_cast<EventDelegate> (&PokemonListing_dialog::_clkOpenPokemon) );
    i = btnRefreshEntries.GetSize().height;
    i += i/2;
    lboxLoadedEntries.SetLocation(0,i);
    lboxLoadedEntries.SetSize(mySize.width,mySize.height-i);
    if (entries.size()==0)
        _refreshEntries(); // initial loading failed because battery file wasn't loaded
    for (dword i = 0;i<entries.size();i++)
        lboxLoadedEntries.AddItem( entries.get_elem(i) );
}
void PokemonListing_dialog::_clkRefreshEntries(Window*)
{
    _refreshEntries();
    lboxLoadedEntries.RemoveAllItems();
    for (dword i = 0;i<entries.size();i++)
        lboxLoadedEntries.AddItem( entries.get_elem(i) );
}
void PokemonListing_dialog::_clkOpenPokemon(Window*)
{
    PokemonEntry entry;
    PokemonEntryType type;
    PokemonSaveFile::EntryOperationReturn r;
    dword i,occurance;
    int selInd = lboxLoadedEntries.GetSelectedIndex();
    if (selInd<0)
        return;
    if (selInd<6)
    {
        type = PartyPok;
        occurance = selInd;
    }
    else
    {
        dword top;
        bool isConcl;
        PokemonGameFromFlag gameFrom;
        gameFrom = saveFile->GetGameFrom(isConcl);
        top = (gameFrom==PokemonFireRed || gameFrom==PokemonLeafGreen ? 8 : 7);
        if (dword(selInd)<=top)
        {
            type = DayCarePok;
            occurance = selInd-6;
        }
        else
        {
            type = PCPok;
            occurance = selInd-top-1;
        }
    }
    r = saveFile->GetPokemonEntry(entry,type,saveStateType,occurance);
    if (r==PokemonSaveFile::EntryUnobtainable)
    {
        Message_dialog dialog("The specified entry was unobtainable.");
        dialog.ShowDlg(this);
        return;
    }
    else if (r==PokemonSaveFile::EntryOutstanding)
    {
        Message_dialog dialog("The specified entry is outstanding. This means that another service within PokemonEditor is currently using the entry. Close the entry and try again.");
        dialog.ShowDlg(this);
        return;
    }
    i = 0;
    // find the first unused dialog object
    while (i<dialogs.size() && dialogs[i].WinHandle()!=NULL)
        i++;
    if (i>=dialogs.size()) // there were no unused dialog objects, so, create a new one
        dialogs.add();
    PokemonEditor_dialog& newDialog = dialogs[i];
    newDialog.InitializeEntry(saveFile,entry);
    newDialog.CreateDlg(GetParentWin());
    newDialog.Show();
}
void PokemonListing_dialog::_refreshEntries()
{
    bool b;
    PokemonGameFromFlag gameFrom = saveFile->GetGameFrom(b);
    if (gameFrom==DemoJirachiDisk)
        return;
    /* load up the shallow entries
        order
            party poks
            day care poks
            pc poks
    */
    entries.empty();
    for (dword i = 0;i<6;i++)
    {
        rstringstream ss;
        ss << "Party" << i+1 << " - "
            << saveFile->GetShallowPokemonEntry(PartyPok,saveStateType,i);
        entries.add(ss.get_device());
    }
    for (dword i = 0;i<2;i++)
    {
        rstringstream ss;
        str shallowEntry = saveFile->GetShallowPokemonEntry(DayCarePok,saveStateType,i);
        ss << "DayCare" << i+1 << " - " << (shallowEntry.size()==0 ? "UNVERIFIED_LOCATION" : shallowEntry);
        entries.add(ss.get_device());
    }
    if (gameFrom==PokemonFireRed || gameFrom==PokemonLeafGreen)
    {
        str s = "DayCareLoner - " + saveFile->GetShallowPokemonEntry(DayCarePok,saveStateType,2);
        entries.add(s);
    }
    for (dword box = 0;box<14;box++)
    {
        for (dword pokNum = 0;pokNum<30;pokNum++)
        {
            rstringstream ss;
            ss << "PC Box" << box+1 << " Entry" << pokNum+1 << " - " << saveFile->GetShallowPokemonEntry(PCPok,saveStateType,box*30+pokNum);
            entries.add(ss.get_device());
        }
    }
}

PokemonEditor_dialog::PokemonEditor_dialog(PokemonSaveFile* pSaveFile,const PokemonEntry& Entry)
{
    InitializeEntry(pSaveFile,Entry);
}
PokemonEditor_dialog::PokemonEditor_dialog(const char* pSaveFile,const PokemonEntry& Entry)
{
    InitializeEntry(pSaveFile,Entry);
}
void PokemonEditor_dialog::InitializeEntry(PokemonSaveFile* pSaveFile,const PokemonEntry& Entry)
{
    cancelFlag = true; // default to true so that control box X will NOT attempt save
    levelExpFlag = false;
    genericUpdateFlag = false;
    saveFile = pSaveFile;
    saveFileName.clear(); // so that the dialog isn't flagged as being routed to a file as opposed to the battery file
    entry = Entry;
    temp = Entry.pok;
}
void PokemonEditor_dialog::InitializeEntry(const char* pSaveFile,const PokemonEntry& Entry)
{
    cancelFlag = true; // default to true so that control box X will NOT attempt save
    levelExpFlag = false;
    genericUpdateFlag = false;
    saveFile = NULL;
    saveFileName = pSaveFile;
    entry = Entry;
    temp = Entry.pok;
}
void PokemonEditor_dialog::OnCreate(const EventData&)
{
    str title = "POK=";
    // init controls before downloading pokemon data
    _initControls(); // calls callbackOnCreate for baseDialog
    _positionControls();
    if (entry.pok.GetEntryType()==NullPok && entry.pok.GetNickname().length()==0)
        title += "Null Pokemon";
    else if (entry.pok.GetEntryType()==ErrorPok)
        title += "Bad Pokemon";
    else
    {
        title += entry.pok.GetNickname().to_string();
        _downloadEntry(); // only download the entry if it is valid
    }
    SetText(title);
    SetSize(470,670);
    dataAltered = false;
}
void PokemonEditor_dialog::OnClose(const EventData& data)
{
    callbackOnClose(); // needed call for base to destroy memory
    if (dataAltered && cancelFlag)
    {
        if (IsMinimized())
            Restore();
        YesNoCancel_dialog warning("Warning - the Pokemon data has been altered and not saved. Do you want to save the changes you've made?");
        DialogResult r = warning.ShowDlg(this);
        if (r==Yes)
            cancelFlag = false;
        else if (r==Cancel)
        {
            data.tag = FALSE;
            return;
        }
    }
    if (dataAltered && !cancelFlag)
        _uploadEntry();
    if (!cancelFlag && saveFile==NULL)
    {// user didn't cancel and the data might be destined for a file (as opposed to the save battery file object)
        if (saveFileName.length()==0)
        {
            if (IsMinimized())
                Restore();
            YesNoCancel_dialog dialog("This pokemon entry's data has no destination. Click 'Save' to choose how to save the data; click 'Don't save' to forget and lose the data; click 'Cancel' to return to the editing dialog.",
                "Save","Don't Save");
            DialogResult r = dialog.ShowDlg(this);
            if (r==Yes)
                ; // let the user choose how to save the data
            else if (r==Cancel)
            {
                data.tag = FALSE;
                return;
            }
        }
        else
        {
            // overwrite file with pokemon data
            str bytes = entry.pok.ToBinaryStream().get_device();
            // (open even if the file doesn't exist and overwrite contents)
            do
            {
                fileIO::File theFile;
                if (!theFile.Open(saveFileName,fileIO::GenericWrite,fileIO::FileNoShare,false,true) || !theFile.Write((const byte*) bytes.c_str(),bytes.length())) 
                {
                    if (IsMinimized())
                        Restore();
                    YesNoCancel_dialog dialog("File error with '" + saveFileName + "'. Check to make sure that the location is not read-only and that it still exists. Click 'Try Again' to attempt the save again. Click 'Browse' to choose a new save location. Click 'Cancel' to return to the editing dialog.",
                        "Try Again","Browse");
                    DialogResult r = dialog.ShowDlg(this);
                    if (r==No) // browse for new file
                    {
                        common_dialogs::SaveFileDialog saveDialog;
                        saveDialog.AddFilter("pokentry","PokemonEditor Entry File");
                        saveDialog.SetDefaultFileName(saveFileName);
                        saveDialog.SetAllFileFilter();
                        saveFileName = saveDialog.RunDialog(*this);
                        if (saveFileName.length()==0)
                            break; // cancel save and lose the data; the user canceled the save dialog
                    }
                    else if (r==Cancel)
                    {
                        data.tag = FALSE; // cancel save and return to editing dialog
                        return;
                    }
                }
                else
                    break;
            } while (true);
        }
    }
    if (saveFile!=NULL) // the destination is the save battery file object
        // if cancelFlag then don't modify altered state within battery file (!cancelFlag value)
        saveFile->PutPokemonEntry(entry,!cancelFlag && dataAltered); // we must put the entry back
    // reset object in case it can be reused (InitializeEntry should be called again)
    saveFile = NULL; // just in case
    saveFileName.clear(); // just in case
    entry = PokemonEntry();
}
void PokemonEditor_dialog::_initControls()
{
    // Create controls
    gboxGeneral.CreateButton(this);
    gboxStats.CreateButton(this);
    gboxModifiers.CreateButton(this);
    gboxMoves.CreateButton(this);
    gboxMisc.CreateButton(this);
    btnDone.CreateButton(this);
    btnCancel.CreateButton(this);
    btnView.CreateButton(this);
    lblOriginalTrainerRegularID.Create(0,0,100,21,this);
    lblOriginalTrainerSecretID.Create(0,0,100,21,this);
    lblOriginalTrainerName.Create(0,0,100,21,this);
    lblOriginalTrainerGender.Create(0,0,100,21,this);
    lblMark.Create(0,0,100,21,this);
    lblSpecies.Create(0,0,100,21,this);
    lblSpeciesIndex.Create(0,0,100,21,this);
    lblNickname.Create(0,0,100,21,this);
    lblBallCaughtWith.Create(0,0,100,21,this);
    lblGameFrom.Create(0,0,100,21,this);
    lblHeldItem.Create(0,0,100,21,this);
    lblPPUpsTitle.Create(0,0,100,21,this);
    lblPPTitle.Create(0,0,100,21,this);
    lblEVsTitle.Create(0,0,100,21,this);
    for (byte i = 0;i<6;i++)
        lblModStat[i].Create(0,0,100,15,this);
    lblIVsTitle.Create(0,0,100,21,this);
    lblLevel.Create(0,0,100,21,this);
    lblLevelCaught.Create(0,0,100,21,this);
    lblPartyLevel.Create(0,0,100,21,this);
    lblLocationCaught.Create(0,0,100,21,this);
    lblPartyStatTitle.Create(0,0,100,21,this);
    lblContestStatTitle.Create(0,0,100,21,this);
    lblNature.Create(0,0,100,21,this);
    lblGender.Create(0,0,100,21,this);
    lblFriendship.Create(0,0,100,21,this);
    lblStats.Create(0,0,100,21,this);
    cboxSpecies.CreateComboBox(this);
    cboxBallCaughtWith.CreateComboBox(this);
    cboxGameFrom.CreateComboBox(this);
    cboxHeldItem.CreateComboBox(this);
    cboxLocationCaught.CreateComboBox(this);
    cboxNature.CreateComboBox(this);
    for (byte i = 0;i<4;i++)
        cboxTechs[i].CreateComboBox(this);
    txtOriginalTrainerRegularID.CreateTextBox(this);
    txtOriginalTrainerSecretID.CreateTextBox(this);
    txtOriginalTrainerName.CreateTextBox(this);
    txtNickname.CreateTextBox(this);
    for (byte i = 0;i<6;i++)
    {
        if (i<4)
        {
            txtPPUps[i].CreateTextBox(this);
            txtPPMove[i].CreateTextBox(this);
        }
        txtEVStat[i].CreateTextBox(this);
        txtIVStat[i].CreateTextBox(this);
        txtPartyStat[i].CreateTextBox(this);
        txtContestStat[i].CreateTextBox(this);
    }
    txtLevel.CreateTextBox(this);
    txtLevelCaught.CreateTextBox(this);
    txtFriendship.CreateTextBox(this);
    txtExperience.CreateTextBox(this);
    txtPartyLevel.CreateTextBox(this);
    txtInfoDisplay.CreateTextBox(this);
    chkbxMarkCircle.CreateButton(this);
    chkbxMarkSquare.CreateButton(this);
    chkbxMarkTriangle.CreateButton(this);
    chkbxMarkHeart.CreateButton(this);
    chkbxObeyBit.CreateButton(this);
    chkbxShiny.CreateButton(this);
    rbtnGenderMale.CreateRadioButton(this,true);
    rbtnGenderFemale.CreateRadioButton(this);
    rbtnGenderUnknown.CreateRadioButton(this);
    rbtnTGenderMale.CreateRadioButton(this,true); // create new radio group
    rbtnTGenderFemale.CreateRadioButton(this);
    // tell the dialog base to perform some default init. on the dialog and its items
    callbackOnCreate();
    // Set control content and (for some) size
    gboxGeneral.SetText("General");
    gboxStats.SetText("Stats");
    gboxModifiers.SetText("Modifiers");
    gboxMoves.SetText("Move Data");
    gboxMisc.SetText("Misc.");
    btnDone.SetText("Save");
    btnCancel.SetText("Cancel");
    btnView.SetText("View Info");
    lblOriginalTrainerRegularID.SetText("OT ID:");
    lblOriginalTrainerRegularID.FitWidthToText();
    lblOriginalTrainerSecretID.SetText("OT ID (Secret):");
    lblOriginalTrainerSecretID.FitWidthToText();
    lblOriginalTrainerName.SetText("OT Name:");
    lblOriginalTrainerName.FitWidthToText();
    lblOriginalTrainerGender.SetText("-OT Gender-");
    lblOriginalTrainerGender.FitWidthToText();
    lblMark.SetText("-Mark Configuration-");
    lblMark.FitWidthToText();
    lblSpecies.SetText("Species:");
    lblSpecies.FitWidthToText();
    lblSpeciesIndex.FitWidthToText("Index=WWW");
    lblNickname.SetText("Nickname:");
    lblNickname.FitWidthToText();
    lblBallCaughtWith.SetText("PokeBall:");
    lblBallCaughtWith.FitWidthToText();
    lblGameFrom.SetText("Game From:");
    lblGameFrom.FitWidthToText();
    lblHeldItem.SetText("Held Item:");
    lblHeldItem.FitWidthToText();
    lblPPUpsTitle.SetText("-PP Ups-");
    lblPPUpsTitle.FitWidthToText();
    lblPPTitle.SetText("-Current PP-");
    lblPPTitle.FitWidthToText();
    lblEVsTitle.SetText("-Effort-");
    lblEVsTitle.FitWidthToText();
    lblIVsTitle.SetText("-Indv. Values-");
    lblIVsTitle.FitWidthToText();
    lblPartyStatTitle.SetText("-Party Stats-");
    lblPartyStatTitle.FitWidthToText();
    for (PokemonStat stat = HP;stat<=SpcDefense;((int&)stat)++)
    {
        int index = stat-HP;
        lblModStat[index].SetText( ToString_PokemonStat(stat)+str(":") );
        lblModStat[index].FitWidthToText();
    }
    lblLevel.SetText("-Level/Experience-");
    lblLevel.FitWidthToText();
    lblLevelCaught.SetText("Level Caught:");
    lblLevelCaught.FitWidthToText();
    lblPartyLevel.SetText("Party Level:");
    lblPartyLevel.FitWidthToText();
    lblLocationCaught.SetText("Location Caught:");
    lblLocationCaught.FitWidthToText();
    lblContestStatTitle.SetText("-Contest Stats-");
    lblContestStatTitle.FitWidthToText();
    lblNature.SetText("-Nature-");
    lblNature.FitWidthToText();
    lblGender.SetText("-Gender-");
    lblGender.FitWidthToText();
    if (entry.pok.IsEgg())
        lblFriendship.SetText("Hatch Cycles:");
    else
        lblFriendship.SetText("Friendship:");
    lblFriendship.FitWidthToText();
    lblStats.SetText(_getStatString()); // set initial string
    lblStats.SetSize(165,95);
    cboxSpecies.SetCueBanner(L"Species");
    for (dword i = 0;i<=PokemonEditor::MAX_POKEMON_INDEX;i++)
        cboxSpecies.AddItem(PokemonEditor::POKEMON_BASE_DATA[i].speciesName);
    cboxSpecies.SizeToText("WWWWWWW");
    cboxBallCaughtWith.SetCueBanner(L"PokeBall");
    cboxBallCaughtWith.FitWidthToText("Master Ball    ");
    for (Ball b = MasterBall;b<=PremierBall;((int&)b)++)
        cboxBallCaughtWith.AddItem( ToString_Ball(b) );
    cboxGameFrom.SetCueBanner(L"Game From Flag");
    cboxGameFrom.SizeToText("WWWWWWWWWWWWWW");
    for (PokemonGameFromFlag flag = DemoJirachiDisk;flag<=PokemonColosseum;((int&)flag)++)
        cboxGameFrom.AddItem( ToString_PokemonGameFromFlag(flag) );
    cboxHeldItem.SetCueBanner(L"Held Item");
    for (dword i = 0;i<POKEMON_ITEM_LOOKUP.actual_size();i++)
        cboxHeldItem.AddItem(POKEMON_ITEM_LOOKUP[ POKEMON_ITEM_LOOKUP.get_virtual_index(i) ]->itemName);
    cboxHeldItem.SizeToText("WWWWWWWWWWWW");
    cboxLocationCaught.SetCueBanner(L"Location Caught");
    cboxLocationCaught.SizeToText("WWWWwWWWWWWW");
    for (dword i = 0;i<POKEMON_LOCATION_LOOKUP.actual_size()-1;i++) // subtract 1 to exclude BAD_LOC
        cboxLocationCaught.AddItem(POKEMON_LOCATION_LOOKUP[ POKEMON_LOCATION_LOOKUP.get_virtual_index(i) ]->locationName);
    cboxNature.SetCueBanner(L"Nature");
    for (PokemonNature n = Hardy;n<=Quirky;((int&)n)++)
        cboxNature.AddItem( ToString_PokemonNature(n) );
    for (short i = 0;i<4;i++)
    {
        rstringstream ss;
        ss << (i+1);
        cboxTechs[i].SetCueBanner(L"Tech. #"+Misc::to_wide_char_str(ss.get_device()));
        cboxTechs[i].FitWidthToText("MoveMoveMov");
    }
    for (dword i = 0;i<=MAX_POKEMON_MOVE;i++)   
    {
        for (byte j = 0;j<4;j++)
            cboxTechs[j].AddItem(POKEMON_MOVE_DATA[i].moveName);
    }
    txtOriginalTrainerRegularID.SetBannerCue(L"OT ID");
    txtOriginalTrainerRegularID.FitWidthToText("55555");
    txtOriginalTrainerRegularID.SetTextLimit(5);
    txtOriginalTrainerSecretID.SetBannerCue(L"OT SID");
    txtOriginalTrainerSecretID.FitWidthToText("55555");
    txtOriginalTrainerSecretID.SetTextLimit(5);
    txtOriginalTrainerName.SetBannerCue(L"Trainer Name");
    txtOriginalTrainerName.FitWidthToText("WWWWWWW");
    txtOriginalTrainerName.SetTextLimit(7);
    txtNickname.SetBannerCue(L"Nickname");
    txtNickname.FitWidthToText("WWWWWWWW");
    txtNickname.SetTextLimit(10);
    for (int i = 0;i<4;i++) 
    {
        wstr cue;
        rstringstream ss;
        ss << "Move " << i+1;
        cue = Misc::to_wide_char_str(ss.get_device());
        txtPPUps[i].SetBannerCue(cue);
        txtPPMove[i].SetBannerCue(cue);
        txtPPUps[i].FitWidthToText("111223");
        txtPPMove[i].FitWidthToText("111223");
        txtPPUps[i].SetTextLimit(1);
        txtPPMove[i].SetTextLimit(3);
    }
    for (PokemonStat stat = HP;stat<=SpcDefense;((int&)stat)++)
    {
        int index = stat-HP;
        wstr cue = Misc::to_wide_char_str( ToString_PokemonStat_abbr(stat) );
        txtEVStat[index].SetBannerCue(cue);
        txtEVStat[index].SetTextLimit(3);
        txtEVStat[index].SetTextBoxSize(45);
        txtIVStat[index].SetBannerCue(cue);
        txtIVStat[index].SetTextLimit(2);
        txtIVStat[index].SetTextBoxSize(45);
        txtPartyStat[index].SetBannerCue(cue);
        txtPartyStat[index].SetTextLimit(3);
        txtPartyStat[index].SetTextBoxSize(40);
    }
    txtLevel.SetBannerCue(L"Level");
    txtLevel.SetTextLimit(3);
    txtLevel.FitWidthToText("WWW");
    txtLevelCaught.SetBannerCue(L"Lvl");
    txtLevelCaught.FitWidthToText("555");
    txtLevelCaught.SetTextLimit(3);
    for (ContestStat stat = Coolness;stat<=Feel;((int&)stat)++)
    {
        int index = stat-Coolness;
        wstr cue = Misc::to_wide_char_str( ToString_ContestStat_abbr(stat) );
        txtContestStat[index].SetBannerCue(cue);
        txtContestStat[index].SetTextBoxSize(40);
        txtContestStat[index].SetTextLimit(3);
    }
    if (entry.pok.IsEgg())
        txtFriendship.SetBannerCue(L"Hatch Cycles:");
    else
        txtFriendship.SetBannerCue(L"Friendship");
    txtFriendship.FitWidthToText("Friendship");
    txtFriendship.SetTextLimit(3);
    txtExperience.SetBannerCue(L"Experience");
    txtExperience.SetTextLimit(7);
    txtExperience.FitWidthToText("999999910");
    txtPartyLevel.SetBannerCue(L"Party Lvl");
    txtPartyLevel.SetTextLimit(3);
    txtPartyLevel.SetTextBoxSize(75);
    txtInfoDisplay.SetAttribute(SendReturnChar);
    txtInfoDisplay.SetAttribute(VerticalScroll);
    txtInfoDisplay.SetAttribute(HorizontileScroll);
    txtInfoDisplay.SetAttribute(ReadOnly);
    chkbxMarkCircle.SetText("Circle");
    chkbxMarkTriangle.SetText("Triangle");
    chkbxMarkHeart.SetText("Heart");
    chkbxMarkSquare.SetText("Square");
    chkbxShiny.SetText("Is Shiny");
    chkbxObeyBit.SetText("Obey Bit");
    rbtnGenderMale.SetText("Male");
    rbtnGenderMale.FitWidthToText("Male   ");
    rbtnGenderFemale.SetText("Female");
    rbtnGenderFemale.FitWidthToText("Female   ");
    rbtnGenderUnknown.SetText("Unknown");
    rbtnGenderUnknown.FitWidthToText("Unknown   ");
    rbtnTGenderMale.SetText("Male");
    rbtnTGenderMale.FitWidthToText("Male   ");
    rbtnTGenderFemale.SetText("Female");
    rbtnTGenderFemale.FitWidthToText("Female   ");
    // set events
    EventDelegate   genericUpdate = static_cast<EventDelegate> (&PokemonEditor_dialog::_genericUpdate),
                    statUpdate = static_cast<EventDelegate> (&PokemonEditor_dialog::_statUpdate);
    btnDone.SetClickEvent(static_cast<EventDelegate> (&PokemonEditor_dialog::_clkDone) );
    btnCancel.SetClickEvent(static_cast<EventDelegate> (&PokemonEditor_dialog::_clkCancel) );
    btnView.SetClickEvent(static_cast<EventDelegate> (&PokemonEditor_dialog::_clkInfo) );
    cboxSpecies.SetSelectionChangedEvent( static_cast<EventDelegate> (&PokemonEditor_dialog::_selchngSpecies) );
    txtLevel.SetTextChangedEvent( static_cast<EventDelegate> (&PokemonEditor_dialog::_levelUpdate) );
    txtExperience.SetTextChangedEvent( static_cast<EventDelegate> (&PokemonEditor_dialog::_expUpdate) );
    cboxBallCaughtWith.SetSelectionChangedEvent(genericUpdate);
    cboxGameFrom.SetSelectionChangedEvent(genericUpdate);
    cboxHeldItem.SetSelectionChangedEvent(genericUpdate);
    cboxLocationCaught.SetSelectionChangedEvent(genericUpdate);
    cboxNature.SetSelectionChangedEvent(statUpdate);
    for (byte i = 0;i<4;i++)
        cboxTechs[i].SetSelectionChangedEvent(genericUpdate);
    txtOriginalTrainerRegularID.SetTextChangedEvent(genericUpdate);
    txtOriginalTrainerSecretID.SetTextChangedEvent(genericUpdate);
    txtOriginalTrainerName.SetTextChangedEvent(genericUpdate);
    txtNickname.SetTextChangedEvent(genericUpdate);
    for (byte i = 0;i<6;i++)
    {
        if (i<4)
        {
            txtPPUps[i].SetTextChangedEvent(genericUpdate);
            txtPPMove[i].SetTextChangedEvent(genericUpdate);
        }
        txtEVStat[i].SetTextChangedEvent(statUpdate);
        txtIVStat[i].SetTextChangedEvent(statUpdate);
        txtPartyStat[i].SetTextChangedEvent(genericUpdate);
        txtContestStat[i].SetTextChangedEvent(genericUpdate);
    }
    txtLevelCaught.SetTextChangedEvent(genericUpdate);
    txtFriendship.SetTextChangedEvent(genericUpdate);
    txtPartyLevel.SetTextChangedEvent(genericUpdate);
    chkbxMarkCircle.SetClickEvent(genericUpdate);
    chkbxMarkSquare.SetClickEvent(genericUpdate);
    chkbxMarkTriangle.SetClickEvent(genericUpdate);
    chkbxMarkHeart.SetClickEvent(genericUpdate);
    chkbxObeyBit.SetClickEvent(genericUpdate);
    chkbxShiny.SetClickEvent(genericUpdate);
    rbtnGenderMale.SetClickEvent(genericUpdate);
    rbtnGenderFemale.SetClickEvent(genericUpdate);
    rbtnGenderUnknown.SetClickEvent(genericUpdate);
    rbtnTGenderMale.SetClickEvent(genericUpdate);
    rbtnTGenderFemale.SetClickEvent(genericUpdate);
    // set horizontile extents for combo boxes (that require it)
    SendMessage(cboxGameFrom.WinHandle(),CB_SETHORIZONTALEXTENT,225,NULL);
    SendMessage(cboxLocationCaught.WinHandle(),CB_SETHORIZONTALEXTENT,225,NULL);
    for (byte i = 0;i<4;i++)
        SendMessage(cboxTechs[i].WinHandle(),CB_SETHORIZONTALEXTENT,125,NULL);
}
void PokemonEditor_dialog::_positionControls()
{
    Size s;
    Point p(2,15), temp; // group box origen (not yet implemented in RWin32)
    //  "general" group box
    gboxGeneral.SetLocation(8,0);
    gboxGeneral.SetSize(452,160);
    gboxGeneral(lblNickname,p);
    p.x += lblNickname.GetSize().width-20;
    gboxGeneral(txtNickname,p);
    p.x += txtNickname.GetSize().width+20;
    gboxGeneral(lblSpecies,p);
    p.x += lblSpecies.GetSize().width;
    gboxGeneral(cboxSpecies,p);
    s = cboxSpecies.GetSize();
    p.x += s.width;
    gboxGeneral(lblSpeciesIndex,p);
    p.x = 2;
    p.y += 2+s.height;
    gboxGeneral(lblOriginalTrainerName,p);
    p.x += lblOriginalTrainerName.GetSize().width-10;
    gboxGeneral(txtOriginalTrainerName,p);
    p.x += txtOriginalTrainerName.GetSize().width+10;
    gboxGeneral(lblOriginalTrainerRegularID,p);
    p.x += lblOriginalTrainerRegularID.GetSize().width;
    gboxGeneral(txtOriginalTrainerRegularID,p);
    p.x += txtOriginalTrainerRegularID.GetSize().width+10;
    gboxGeneral(lblOriginalTrainerSecretID,p);
    s = lblOriginalTrainerSecretID.GetSize();
    p.x += s.width;
    gboxGeneral(txtOriginalTrainerSecretID,p);
    p.x = 2;
    p.y += 2+s.height;
    gboxGeneral(lblOriginalTrainerGender,p);
    p.x += lblOriginalTrainerGender.GetSize().width+10;
    gboxGeneral(lblGender,p);
    s = lblGender.GetSize();
    temp = p;
    gboxGeneral(rbtnGenderMale,temp+Point(6,s.height));
    gboxGeneral(rbtnGenderFemale,temp+Point(6,rbtnGenderMale.GetSize().height+s.height));
    gboxGeneral(rbtnGenderUnknown,temp+Point(6,s.height+rbtnGenderFemale.GetSize().height+rbtnGenderMale.GetSize().height));
    p.x = 8;
    p.y += 2+lblOriginalTrainerGender.GetSize().height;
    gboxGeneral(rbtnTGenderMale,p);
    p.y += 2+rbtnTGenderMale.GetSize().height;
    gboxGeneral(rbtnTGenderFemale,p);
    p = temp;
    p.x += rbtnGenderUnknown.GetSize().width;
    temp = p;
    gboxGeneral(lblGameFrom,p);
    p.x += lblGameFrom.GetSize().width-20;
    gboxGeneral(cboxGameFrom,p);
    p.x = temp.x-5;
    p.y += 2+cboxGameFrom.GetSize().height;
    gboxGeneral(lblBallCaughtWith,p);
    p.x += -5+lblBallCaughtWith.GetSize().width;
    gboxGeneral(cboxBallCaughtWith,p);
    p.x += cboxBallCaughtWith.GetSize().width;
    gboxGeneral(lblLevelCaught,p);
    p.x += lblLevelCaught.GetSize().width-10;
    gboxGeneral(txtLevelCaught,p);
    p.x = temp.x;
    p.y += 2+cboxBallCaughtWith.GetSize().height;
    gboxGeneral(lblLocationCaught,p);
    p.x += lblLocationCaught.GetSize().width-20;
    gboxGeneral(cboxLocationCaught,p);
    p.x = temp.x+5;
    p.y +=cboxLocationCaught.GetSize().height;
    gboxGeneral(lblHeldItem,p);
    p.x += lblHeldItem.GetSize().width;
    gboxGeneral(cboxHeldItem,p);
    // "stats" group box
    p = Point(2,15);
    temp = p;
    gboxStats.SetLocation(8,165);
    gboxStats.SetSize(450,225);
    gboxStats(lblStats,p);
    gboxStats(lblPartyLevel,Point(2,lblStats.GetSize().height+17));
    gboxStats(txtPartyLevel,Point(lblPartyLevel.GetSize().width,lblStats.GetSize().height+17));
    gboxStats(gboxModifiers,Point(p.x+lblStats.GetSize().width+10,p.y));
    p.y += lblStats.GetSize().height+25;
    gboxStats(lblPartyStatTitle,p);
    p.x += lblPartyStatTitle.GetSize().width-6;
    gboxStats(lblContestStatTitle,p);
    p.x = 2;
    p.y += lblPartyStatTitle.GetSize().height-5;
    for (byte i = 0;i<6;i+=2)
    {
        gboxStats(txtPartyStat[i],p);
        p.x += txtPartyStat[i].GetSize().width;
        gboxStats(txtPartyStat[i+1],p);
        p.x += txtPartyStat[i+1].GetSize().width+6;
        gboxStats(txtContestStat[i],p);
        p.x += txtContestStat[i].GetSize().width;
        gboxStats(txtContestStat[i+1],p);
        if (i!=4) // keep p after last control positioned here
        {
            p.x = 2;
            p.y += txtPartyStat[i].GetSize().height;
        }
    }
    p.x += txtContestStat[5].GetSize().width+11;
    p.y -= 15;
    gboxStats(lblLevel,p);
    p.x += 5;
    s.height = p.y;
    p.y += lblLevel.GetSize().height-5;
    gboxStats(txtLevel,p);
    p.x += txtLevel.GetSize().width;
    gboxStats(txtExperience,p);
    p.x += txtExperience.GetSize().width;
    s.width = p.x+5;
    p.x += 30;
    p.y = s.height;
    gboxStats(lblNature,p);
    p.x = s.width;
    p.y += lblNature.GetSize().height-5;
    gboxStats(cboxNature,p);
    gboxModifiers.SetSize(432-lblStats.GetSize().width,205);
    temp.x += lblModStat[5].GetSize().width;
    gboxModifiers(lblEVsTitle,temp);
    temp.x += lblEVsTitle.GetSize().width;
    gboxModifiers(lblIVsTitle,temp);
    temp.x = 2;
    temp.y += lblModStat[5].GetSize().height;
    for (byte i = 0;i<6;i++)
    {
        s = lblModStat[i].GetSize();
        gboxModifiers(lblModStat[i],temp);
        temp.x += s.width+(lblModStat[5].GetSize().width-s.width);
        gboxModifiers(txtEVStat[i],temp);
        temp.x += txtEVStat[i].GetSize().width+20;
        gboxModifiers(txtIVStat[i],temp);
        temp.x = 2;
        temp.y += txtEVStat[i].GetSize().height;
    }
    // "move data" group box
    p = Point(2,20);
    temp = p;
    s = cboxTechs[0].GetSize();
    gboxMoves.SetLocation(8,395);
    gboxMoves.SetSize(225,122);
    for (byte i = 0;i<4;i++,p.y+=s.height)
        gboxMoves(cboxTechs[i],p);
    temp.x += s.width+10;
    temp.y -= 10;
    gboxMoves(lblPPTitle,temp);
    temp.x -= 10;
    temp.y += lblPPTitle.GetSize().height-8;
    s.width = temp.x;
    for (byte i = 0;i<4;i++)
    {
        gboxMoves(txtPPMove[i],temp);
        if (i%2!=0)
        {
            temp.x = s.width;
            temp.y += txtPPMove[i].GetSize().height;
        }
        else
            temp.x += txtPPMove[i].GetSize().width;
    }
    gboxMoves(lblPPUpsTitle,Point(temp.x+10,temp.y));
    temp.y += lblPPUpsTitle.GetSize().height-8;
    for (byte i = 0;i<4;i++)
    {
        gboxMoves(txtPPUps[i],temp);
        if (i%2!=0)
        {
            temp.x = s.width;
            temp.y += txtPPUps[i].GetSize().height;
        }
        else
            temp.x += txtPPUps[i].GetSize().width;
    }

    // "misc." group box
    p = Point(10,15);
    gboxMisc.SetLocation(230,395);
    gboxMisc.SetSize(225,122);
    gboxMisc(lblMark,p);
    p.x += 20;
    p.y += lblMark.GetSize().height-5;
    temp = p;
    gboxMisc(chkbxMarkCircle,p);
    p.x += chkbxMarkCircle.GetSize().width;
    gboxMisc(chkbxMarkSquare,p);
    temp.y += chkbxMarkCircle.GetSize().height;
    gboxMisc(chkbxMarkTriangle,temp);
    temp.x += chkbxMarkTriangle.GetSize().width;
    gboxMisc(chkbxMarkHeart,temp);
    p = Point(10,73);
    gboxMisc(chkbxShiny,p);
    p.x += chkbxShiny.GetSize().width;
    gboxMisc(chkbxObeyBit,p);
    p = Point(10,95);
    gboxMisc(lblFriendship,p);
    p.x += lblFriendship.GetSize().width;
    gboxMisc(txtFriendship,p);
    txtInfoDisplay.SetLocation(8,520);
    txtInfoDisplay.SetSize(450,85);
    btnView.SetLocation(8,610);
    btnDone.SetLocation(280,610);
    btnCancel.SetLocation(285+btnDone.GetSize().width,610);
}
void PokemonEditor_dialog::_genericUpdate(Window* pWnd)
{
    if (!genericUpdateFlag)
    {
        /*
            I hate to do this (the upcasting in the solution below), but Windows is sending notification for radio button clicks whenever the window gets focus!
                (!!! I don't know why this is happening !!!)
                The notifications are sent from unchecked radio buttons and throw off the dataAltered flag.
            So, if the window identifies itself as a radio button, I'll see if it's checked, if not, then ignore the message.
        */
        if (pWnd->MyType()==RadioButtonControl && ( static_cast<RadioButton*> (pWnd) ->GetCheckState() == RadioButton::Unchecked ))
            return;
        _uploadTemp(pWnd);
        _updateInfoDisplay();
        dataAltered = true;
    }
}
void PokemonEditor_dialog::_clkDone(Window*)
{
    cancelFlag = false;
    Close();
}
void PokemonEditor_dialog::_clkCancel(Window*)
{
    cancelFlag = true;
    Close();
}
void PokemonEditor_dialog::_clkInfo(Window*)
{
    str title = GetText(), text = entry.pok.ToString(), text2; // the dialog object knows to forget points to this data after it is created
    PokemonInfo_dialog& dialog = ::GetInActiveInfoDialogObject();
    for (dword i = 0;i<text.length();i++)
    {
        if (text[i]=='\n')
            text2.push_back('\r');
        text2.push_back(text[i]);
    }
    dialog.InitializeDialog(title.c_str(),text2.c_str());
    dialog.CreateDlg(this->GetParentWin()); // pass it along to the parent window so it can last after this (PokemonEditor_dialog) dialog closes
    /* dialog object forgets pointers to local variable data */
    dialog.Show();
}
void PokemonEditor_dialog::_selchngSpecies(Window* pWnd)
{
    rstringstream ss("Index=");
    ss << cboxSpecies.GetSelectedIndex();
    lblSpeciesIndex.SetText(ss.get_device());
    _genericUpdate(pWnd);
}
void PokemonEditor_dialog::_levelUpdate(Window* wObj)
{
    if (temp.GetSpecies()==0)
        return; // bad pokemon
    if (levelExpFlag)
    {// the call is coming from the update to the exp box
        levelExpFlag = false;
        return;
    }
    const PokemonBase* base = entry.pok.GetSpeciesBase();
    word lvl = getNumericFromTBox<word>(txtLevel);
    levelExpFlag = true;
    putNumericInTBox<dword>(txtExperience,EXP_LOOKUP_TABLE[base->expGroup][lvl-1]);
    //_genericUpdate(wObj);
    _statUpdate(wObj); // calls _genericUpdate
}
void PokemonEditor_dialog::_expUpdate(Window* wObj)
{
    if (temp.GetSpecies()==0)
        return; // bad pokemon
    if (levelExpFlag)
    {
        levelExpFlag = false;
        return;
    }
    const PokemonBase* base = entry.pok.GetSpeciesBase();
    dword exp = getNumericFromTBox<dword>(txtExperience), levelIndex = 0;
    while (levelIndex<100 && exp>EXP_LOOKUP_TABLE[base->expGroup][levelIndex])
        levelIndex++;
    levelExpFlag = true;
    if (exp>EXP_LOOKUP_TABLE[base->expGroup][99])
    {
        putNumericInTBox<dword>(txtExperience,EXP_LOOKUP_TABLE[base->expGroup][99]);
        levelExpFlag = true;
    }
    putNumericInTBox<dword>(txtLevel,levelIndex);
    //_genericUpdate(wObj);
    _statUpdate(wObj); // calls _genericUpdate
}
void PokemonEditor_dialog::_statUpdate(Window* wObj)
{
    _genericUpdate(wObj);
    lblStats.SetText( _getStatString() );
}
str PokemonEditor_dialog::_getStatString() const
{
    rstringstream ss;
    ss << "Calculated Stats -\r\n";
    for (PokemonStat stat = HP;stat<=SpcDefense;((int&)stat)++)
    {
        word statVal = temp.GetStat(stat);
        ss << '-';
        ss << ToString_PokemonStat(stat) << ": ";
        if (statVal!=0)
            ss << statVal;
        else
            ss << "N/A";
        ss << "\r\n";
    }
    return ss.get_device();
}
void PokemonEditor_dialog::_downloadEntry()
{
    genericUpdateFlag = true;
    Gender g = entry.pok.GetGender();
    pp_ups ppUps = entry.pok.GetPPUps();
    pok_mark marks = entry.pok.GetMark();
    // put data into control objects
    cboxSpecies.SetSelectedIndex( entry.pok.GetSpecies() ); _selchngSpecies(&cboxSpecies);
    cboxBallCaughtWith.SetSelectedIndex( entry.pok.GetBallCaughtWith()-1 );
    cboxGameFrom.SetSelectedIndex( entry.pok.GetGameFrom() );
    cboxHeldItem.SetSelectedIndex( POKEMON_ITEM_LOOKUP.get_actual_index(entry.pok.GetHeldItem()->itemValue) );
    cboxLocationCaught.SetSelectedIndex( entry.pok.GetLocationCaught()->locationValue );
    cboxNature.SetSelectedIndex( entry.pok.GetNature() );
    for (byte i = 1;i<=4;i++)
        cboxTechs[i-1].SetSelectedIndex( entry.pok.GetMove(i)->moveValue );
    putNumericInTBox(txtOriginalTrainerRegularID,entry.pok.GetOriginalTrainerID());
    putNumericInTBox(txtOriginalTrainerSecretID,entry.pok.GetOriginalTrainerSecretID());
    txtOriginalTrainerName.SetText( entry.pok.GetOriginalTrainerName().to_string() );
    txtNickname.SetText( entry.pok.GetNickname().to_string() );
    putNumericInTBox(txtPPUps[0],ppUps.move1Ups);
    putNumericInTBox(txtPPUps[1],ppUps.move2Ups);
    putNumericInTBox(txtPPUps[2],ppUps.move3Ups);
    putNumericInTBox(txtPPUps[3],ppUps.move4Ups);
    for (byte i = 1;i<=4;i++)
        putNumericInTBox(txtPPMove[i-1],entry.pok.GetPP(i));
    for (PokemonStat stat = HP;stat<=SpcDefense;((int&)stat)++)
    {
        word statVal = entry.pok.GetPartyStat(stat);
        if (statVal!=0)
            putNumericInTBox(txtPartyStat[stat-HP],statVal);
        putNumericInTBox(txtEVStat[stat-HP],entry.pok.GetEV(stat));
        putNumericInTBox(txtIVStat[stat-HP],entry.pok.GetIV(stat));
    }
    levelExpFlag = true; // stop an automatic update
    putNumericInTBox(txtLevel,entry.pok.GetLevel());
    if (entry.pok.GetEntryType()==PartyPok)
        putNumericInTBox(txtPartyLevel,entry.pok.GetPartyLevel());
    putNumericInTBox(txtLevelCaught,entry.pok.GetLevelCaught());
    for (ContestStat stat = Coolness;stat<=Feel;((int&)stat)++)
        putNumericInTBox(txtContestStat[stat-Coolness],entry.pok.GetContestStat(stat));
    putNumericInTBox(txtFriendship,entry.pok.GetFriendship());
    levelExpFlag = true; // stop an automatic update
    putNumericInTBox(txtExperience,entry.pok.GetExperience());
    chkbxMarkCircle.SetCheckState( bstateCast_safe(marks.circle) );
    chkbxMarkSquare.SetCheckState( bstateCast_safe(marks.square) );
    chkbxMarkTriangle.SetCheckState( bstateCast_safe(marks.triangle) );
    chkbxMarkHeart.SetCheckState( bstateCast_safe(marks.heart) );
    chkbxObeyBit.SetCheckState( bstateCast_safe(entry.pok.IsObeySet()) );
    chkbxShiny.SetCheckState( bstateCast_safe(entry.pok.IsShiny()) );
    if (entry.pok.GetOriginalTrainerGender()==Female)
        rbtnTGenderFemale.SetCheckState(Button::Checked);
    else
        rbtnTGenderMale.SetCheckState(Button::Checked);
    if (g==Male)
        rbtnGenderMale.SetCheckState(Button::Checked);
    else if (g==Female)
        rbtnGenderFemale.SetCheckState(Button::Checked);
    else
        rbtnGenderUnknown.SetCheckState(Button::Checked);
    _updateInfoDisplay();
    genericUpdateFlag = false;
}
void PokemonEditor_dialog::_uploadEntry()
{
    pp_ups ups;
    pok_mark mark;
    // put data into entry object from control objects
    entry.pok.SetSpecies(cboxSpecies.GetSelectedIndex());
    entry.pok.SetBallCaughtWith( (Ball) (cboxBallCaughtWith.GetSelectedIndex()+1) );
    entry.pok.SetGameFrom( (PokemonGameFromFlag) cboxGameFrom.GetSelectedIndex() );
    entry.pok.SetHeldItem( POKEMON_ITEM_LOOKUP.get_virtual_index(cboxHeldItem.GetSelectedIndex()) );
    entry.pok.SetLocationCaught( POKEMON_LOCATION_LOOKUP.get_actual_index(cboxLocationCaught.GetSelectedIndex()) );
    entry.pok.SetNature( (PokemonNature) cboxNature.GetSelectedIndex() );
    for (byte i = 0;i<4;i++)
        entry.pok.SetMove( i+1,cboxTechs[i].GetSelectedIndex() );
    entry.pok.SetOriginalTrainerID( getNumericFromTBox<word>(txtOriginalTrainerRegularID),getNumericFromTBox<word>(txtOriginalTrainerSecretID) );
    entry.pok.SetOriginalTrainerName(txtOriginalTrainerName.GetText().c_str());
    entry.pok.SetNickname(txtNickname.GetText().c_str());
    ups.move1Ups = getNumericFromTBox<int>(txtPPUps[0]);
    ups.move2Ups = getNumericFromTBox<int>(txtPPUps[1]);
    ups.move3Ups = getNumericFromTBox<int>(txtPPUps[2]);
    ups.move4Ups = getNumericFromTBox<int>(txtPPUps[3]);
    entry.pok.SetPPUps(ups);
    for (byte i = 0;i<4;i++)
        entry.pok.SetPP(i+1,byte(getNumericFromTBox<word>(txtPPMove[i])));
    for (PokemonStat stat = HP;stat<=SpcDefense;((int&)stat)++)
    {
        entry.pok.SetPartyStat(stat,getNumericFromTBox<word>(txtPartyStat[stat-HP]));
        entry.pok.SetEV(stat,byte(getNumericFromTBox<word>(txtEVStat[stat-HP])));
        entry.pok.SetIV(stat,byte(getNumericFromTBox<word>(txtIVStat[stat-HP])));
    }
    entry.pok.SetPartyLevel(byte(getNumericFromTBox<word>(txtPartyLevel)));
    // don't set level, set exp. only
    entry.pok.SetExperience(getNumericFromTBox<dword>(txtExperience));
    entry.pok.SetLevelCaught(byte(getNumericFromTBox<word>(txtLevelCaught)));
    for (ContestStat stat = Coolness;stat<=Feel;((int&)stat)++)
        entry.pok.SetContestStat(stat,byte(getNumericFromTBox<word>(txtContestStat[stat-HP])));
    entry.pok.SetFriendship(byte(getNumericFromTBox<word>(txtFriendship)));
    mark.circle = chkbxMarkCircle.GetCheckState()==Button::Checked; 
    mark.square = chkbxMarkSquare.GetCheckState()==Button::Checked;
    mark.triangle = chkbxMarkTriangle.GetCheckState()==Button::Checked;
    mark.heart = chkbxMarkHeart.GetCheckState()==Button::Checked;
    entry.pok.SetMark(mark);
    entry.pok.SetObey( chkbxObeyBit.GetCheckState()==Button::Checked );
    entry.pok.SetShiny( chkbxShiny.GetCheckState()==Button::Checked );
    if (rbtnTGenderFemale.GetCheckState()==Button::Checked)
        entry.pok.SetOriginalTrainerGender(Female);
    else if (rbtnTGenderMale.GetCheckState()==Button::Checked)
        entry.pok.SetOriginalTrainerGender(Male);
    if (rbtnGenderFemale.GetCheckState()==Button::Checked)
        entry.pok.SetGender(Female);
    else if (rbtnGenderMale.GetCheckState()==Button::Checked)
        entry.pok.SetGender(Male);
    else if (rbtnGenderUnknown.GetCheckState()==Button::Checked)
        entry.pok.SetGender(PokemonEditor::Unknown);
}
void PokemonEditor_dialog::_uploadTemp(Window* wChng)
{
    /*
        I just bother reading data from the control that was updated.
            The data is updated into the temp pokemon object
    */
    pok_mark mark;
    // put data into entry object from control objects
    if (wChng->WinHandle()==cboxSpecies.WinHandle())
        temp.SetSpecies(cboxSpecies.GetSelectedIndex());
    else if (wChng->WinHandle()==cboxBallCaughtWith.WinHandle())
        temp.SetBallCaughtWith( (Ball) cboxBallCaughtWith.GetSelectedIndex() );
    else if (wChng->WinHandle()==cboxGameFrom.WinHandle())
        temp.SetGameFrom( (PokemonGameFromFlag) cboxGameFrom.GetSelectedIndex() );
    else if (wChng->WinHandle()==cboxHeldItem.WinHandle())
        temp.SetHeldItem( POKEMON_ITEM_LOOKUP.get_actual_index(cboxHeldItem.GetSelectedIndex()) );
    else if (wChng->WinHandle()==cboxLocationCaught.WinHandle())
        temp.SetLocationCaught( POKEMON_LOCATION_LOOKUP.get_actual_index(cboxLocationCaught.GetSelectedIndex()) );
    else if (wChng->WinHandle()==cboxNature.WinHandle())
        temp.SetNature( (PokemonNature) cboxNature.GetSelectedIndex() );
    else if (wChng->WinHandle()==cboxTechs[0].WinHandle())
        temp.SetMove( 1,cboxTechs[0].GetSelectedIndex() );
    else if (wChng->WinHandle()==cboxTechs[1].WinHandle())
        temp.SetMove( 2,cboxTechs[1].GetSelectedIndex() );
    else if (wChng->WinHandle()==cboxTechs[2].WinHandle())
        temp.SetMove( 3,cboxTechs[2].GetSelectedIndex() );
    else if (wChng->WinHandle()==cboxTechs[3].WinHandle())
        temp.SetMove( 4,cboxTechs[3].GetSelectedIndex() );
    //for (byte i = 0;i<4;i++)
    //  temp.SetMove( i+1,cboxTechs[i].GetSelectedIndex() );
    else if (wChng->WinHandle()==txtOriginalTrainerRegularID.WinHandle() || wChng->WinHandle()==txtOriginalTrainerSecretID.WinHandle())
        temp.SetOriginalTrainerID( getNumericFromTBox<word>(txtOriginalTrainerRegularID),getNumericFromTBox<word>(txtOriginalTrainerSecretID) );
    else if (wChng->WinHandle()==txtOriginalTrainerName.WinHandle())
        temp.SetOriginalTrainerName(txtOriginalTrainerName.GetText().c_str());
    else if (wChng->WinHandle()==txtNickname.WinHandle())
        temp.SetNickname(txtNickname.GetText().c_str());
    else if (wChng->WinHandle()==txtPPUps[0].WinHandle() || wChng->WinHandle()==txtPPUps[1].WinHandle() || wChng->WinHandle()==txtPPUps[2].WinHandle() || wChng->WinHandle()==txtPPUps[3].WinHandle())
    {// upload all of them
        pp_ups ups;
        ups.move1Ups = getNumericFromTBox<int>(txtPPUps[0]);
        ups.move2Ups = getNumericFromTBox<int>(txtPPUps[1]);
        ups.move3Ups = getNumericFromTBox<int>(txtPPUps[2]);
        ups.move4Ups = getNumericFromTBox<int>(txtPPUps[3]);
        temp.SetPPUps(ups);
    }
    else if (wChng->WinHandle()==txtPPMove[0].WinHandle())
        temp.SetPP(1,byte(getNumericFromTBox<word>(txtPPMove[0])));
    else if (wChng->WinHandle()==txtPPMove[1].WinHandle())
        temp.SetPP(2,byte(getNumericFromTBox<word>(txtPPMove[1])));
    else if (wChng->WinHandle()==txtPPMove[2].WinHandle())
        temp.SetPP(3,byte(getNumericFromTBox<word>(txtPPMove[2])));
    else if (wChng->WinHandle()==txtPPMove[3].WinHandle())
        temp.SetPP(4,byte(getNumericFromTBox<word>(txtPPMove[3])));
    else if (wChng->WinHandle()==txtExperience.WinHandle())
        temp.SetExperience(getNumericFromTBox<dword>(txtExperience));
    else if (wChng->WinHandle()==txtLevel.WinHandle())
        temp.SetLevel(byte(getNumericFromTBox<word>(txtLevel)));
    else if (wChng->WinHandle()==txtPartyLevel.WinHandle())
        temp.SetPartyLevel(byte(getNumericFromTBox<word>(txtPartyLevel)));
    else if (wChng->WinHandle()==txtLevelCaught.WinHandle())
        temp.SetLevelCaught(byte(getNumericFromTBox<word>(txtLevelCaught)));
    else if (wChng->WinHandle()==txtFriendship.WinHandle())
        temp.SetFriendship(byte(getNumericFromTBox<word>(txtFriendship)));
    else if (wChng->WinHandle()==chkbxMarkCircle.WinHandle() || wChng->WinHandle()==chkbxMarkSquare.WinHandle() || wChng->WinHandle()==chkbxMarkTriangle.WinHandle() || wChng->WinHandle()==chkbxMarkHeart.WinHandle())
    {
        mark.circle = chkbxMarkCircle.GetCheckState()==Button::Checked; 
        mark.square = chkbxMarkSquare.GetCheckState()==Button::Checked;
        mark.triangle = chkbxMarkTriangle.GetCheckState()==Button::Checked;
        mark.heart = chkbxMarkHeart.GetCheckState()==Button::Checked;
        temp.SetMark(mark);
    }
    else if (wChng->WinHandle()==chkbxObeyBit.WinHandle())
        temp.SetObey( chkbxObeyBit.GetCheckState()==Button::Checked );
    else if (wChng->WinHandle()==chkbxShiny.WinHandle())
        temp.SetShiny( chkbxShiny.GetCheckState()==Button::Checked );
    else if (wChng->WinHandle()==rbtnTGenderFemale.WinHandle() || wChng->WinHandle()==rbtnTGenderMale.WinHandle())
    {
        if (rbtnTGenderFemale.GetCheckState()==Button::Checked)
            temp.SetOriginalTrainerGender(Female);
        else if (rbtnTGenderMale.GetCheckState()==Button::Checked)
            temp.SetOriginalTrainerGender(Male);
    }
    else if (wChng->WinHandle()==rbtnGenderFemale.WinHandle() || wChng->WinHandle()==rbtnGenderMale.WinHandle() || wChng->WinHandle()==rbtnGenderUnknown.WinHandle())
    {
        if (rbtnGenderFemale.GetCheckState()==Button::Checked)
            temp.SetGender(Female);
        else if (rbtnGenderMale.GetCheckState()==Button::Checked)
            temp.SetGender(Male);
        else if (rbtnGenderUnknown.GetCheckState()==Button::Checked)
            temp.SetGender(PokemonEditor::Unknown);
    }
    else
    {
        for (PokemonStat stat = HP;stat<=SpcDefense;((int&)stat)++)
        {
            temp.SetPartyStat(stat,byte(getNumericFromTBox<word>(txtPartyStat[stat-HP])));
            temp.SetEV(stat,byte(getNumericFromTBox<word>(txtEVStat[stat-HP])));
            temp.SetIV(stat,byte(getNumericFromTBox<word>(txtIVStat[stat-HP])));
        }
        for (ContestStat stat = Coolness;stat<=Feel;((int&)stat)++)
            temp.SetContestStat(stat,byte(getNumericFromTBox<word>(txtContestStat[stat-HP])));
    }
}
void PokemonEditor_dialog::_updateInfoDisplay()
{
    // display temp data
    str text = temp.ToBytesString(), text2;
    for (dword i = 0;i<text.length();i++)
    {
        if (text[i]=='\n')
            text2.push_back('\r');
        text2.push_back(text[i]);
    }
    txtInfoDisplay.SetText(text2);
}

void PokemonInfo_dialog::InitializeDialog(const char* pTitle,const char* pTxt)
{
    title = pTitle;
    txt = pTxt;
}
void PokemonInfo_dialog::OnCreate(const EventData&)
{
    Size s;
    if (title!=NULL)
        SetText(title);
    SetSize(425,425);
    btnDone.CreateButton(this);
    box.CreateTextBox(this);
    box.SetAttribute(Border,true);
    box.SetAttribute(HorizontileScroll);
    box.SetAttribute(VerticalScroll);
    s = GetClientSize();
    s.height -= btnDone.GetSize().height+4;
    box.SetSize(s);
    btnDone.SetText("Done");
    btnDone.SetLocation(305,box.GetSize().height+4);
    btnDone.SetClickEvent(static_cast<EventDelegate> (&PokemonInfo_dialog::_clkDone));
    if (txt!=NULL)
        box.SetText(txt);
    callbackOnCreate(); // needed call for baseDialog
    // the data pointed to may pass out of scope
    title = NULL;
    txt = NULL;
}
void PokemonInfo_dialog::_clkDone(Window*)
{
    Close();
}

bool NewPokemon_dialog::CloseActiveDialogs()
{
    bool r = true;
    for (dword i = 0;i<editorDialogs.size();i++)
    {
        if (editorDialogs[i].WinHandle()!=NULL)
        {
            editorDialogs[i].FlashWindow(FlashCaption,5);
            if (!editorDialogs[i].Close())
                r = false;
        }
    }
    return r;
}
bool NewPokemon_dialog::MinimizeActiveDialogs()
{
    bool b = true;
    for (dword i = 0;i<editorDialogs.length();i++)
    {
        if (editorDialogs[i].WinHandle()!=NULL && !editorDialogs[i].Minimize())
            b = false;
    }
    return b;
}
bool NewPokemon_dialog::RestoreActiveDialogs()
{
    bool b = true;
    for (dword i = 0;i<editorDialogs.length();i++)
    {
        if (editorDialogs[i].WinHandle()!=NULL && !editorDialogs[i].Restore())
            b = false;
    }
    return b;
}

void NewPokemon_dialog::OnCreate(const EventData&)
{
    bool dummy;
    Gender g;
    PokemonGameFromFlag flag;
    SetText("Create New Pokemon...");
    SetSize(565,345);
    _initControls();
    callbackOnCreate(); // needed call
    // fill the controls with default data obtained from the battery file
    if (pSaveFile==NULL)
        return;
    boxTrainerName.SetText( pSaveFile->GetTrainerName().to_string() );
    putNumericInTBox(boxTrainerIDReg,pSaveFile->GetTrainerRegularID());
    putNumericInTBox(boxTrainerIDSec,pSaveFile->GetTrainerSecretID());
    g = pSaveFile->GetTrainerGender();
    if (g==Male)
        rbtnTrainerGenderMale.SetCheckState(RadioButton::Checked);
    else if (g==Female)
        rbtnTrainerGenderFemale.SetCheckState(RadioButton::Checked);
    flag = pSaveFile->GetGameFrom(dummy);
    if (flag!=DemoJirachiDisk)
        cboxGameFrom.SetSelectedIndex(flag);
}
void NewPokemon_dialog::_initControls()
{
    // set the radio button groups up
    rbtnTrainerGenderMale.CreateRadioButton(this,true);
    rbtnTrainerGenderFemale.CreateRadioButton(this);
    rbtnBatteryFile.CreateRadioButton(this,true);
    rbtnExternalFile.CreateRadioButton(this);
    //
    gboxDestination.Create(33,179,514,101,this);
    lblSpecies.Create(39,21,63,15,this);
    cboxSpecies.Create(108,21,153,19,this);
    lblGameFrom.Create(276,21,70,15,this);
    cboxGameFrom.Create(352,21,153,19,this);
    lblTrainerName.Create(39,55,161,15,this);
    boxTrainerName.Create(206,52,140,23,this);
    lblTrainerIDReg.Create(352,55,49,15,this);
    boxTrainerIDReg.Create(405,52,100,23,this);
    boxTrainerIDSec.Create(405,84,100,23,this);
    lblTrainerIDSec.Create(289,87,112,15,this);
    rbtnTrainerGenderMale.SetLocation(154,85);
    rbtnTrainerGenderMale.SetSize(53,19);
    rbtnTrainerGenderFemale.SetLocation(213,85);
    rbtnTrainerGenderFemale.SetSize(67,19);
    lblTrainerGender.Create(39,87,112,15,this);
    lblLocationCaught.Create(39,119,119,15,this);
    cboxLocationCaught.Create(164,117,182,19,this);
    lblIniLevel.Create(352,121,105,15,this);
    boxIniLevel.Create(458,116,47,23,this);
    lblNickname.Create(39,153,70,15,this);
    boxNickname.Create(108,150,125,23,this);
    chkbxIsEgg.Create(437,151,68,19,this);
    btnCreate.Create(335,286,75,23,this);
    btnCancel.Create(430,286,75,23,this);
    // these elements go inside the group box but are children of the main window so there events can be routed
    btnBrowse.Create(463,251,75,23,this);
    rbtnBatteryFile.SetLocation(382,198);
    rbtnBatteryFile.SetSize(137,17);
    rbtnExternalFile.SetLocation(382,220);
    rbtnExternalFile.SetSize(144,17);
    lblPath.Create(53,255,42,15,this);
    boxAddress.Create(108,251,349,23,this);
    lblDescript.Create(43,194,317,48,this);
    // set text for elements
    lblSpecies.SetText("Species:");
    lblGameFrom.SetText("GameFrom:");
    lblTrainerName.SetText("Original Trainer Name:");
    lblTrainerIDReg.SetText("OT ID:");
    lblTrainerIDSec.SetText("OT ID (secret):");
    rbtnTrainerGenderMale.SetText("Male");
    rbtnTrainerGenderFemale.SetText("Female");
    lblTrainerGender.SetText("Trainer Gender:");
    lblLocationCaught.SetText("Location Caught:");
    lblIniLevel.SetText("Initial Level:");
    lblNickname.SetText("Nickname:");
    chkbxIsEgg.SetText("Is Egg");
    rbtnBatteryFile.SetText("- Battery File -");
    rbtnExternalFile.SetText("- External File -");
    gboxDestination.SetText("Destination");
    lblPath.SetText("Path:");
    btnBrowse.SetText("Browse");
    btnCreate.SetText("Create");
    btnCancel.SetText("Cancel");
    lblDescript.SetText("Battery file paths - \r\n\tparty:SLOT#\tdaycare:SLOT#\r\n\tpcbox:BOX#:SLOT#");
    // set limits and info for elements
    for (dword i = 0;i<=PokemonEditor::MAX_POKEMON_INDEX;i++)
        cboxSpecies.AddItem(PokemonEditor::POKEMON_BASE_DATA[i].speciesName);
    for (PokemonGameFromFlag flag = DemoJirachiDisk;flag<=PokemonColosseum;((int&)flag)++)
        cboxGameFrom.AddItem( ToString_PokemonGameFromFlag(flag) );
    for (dword i = 0;i<POKEMON_LOCATION_LOOKUP.actual_size()-1;i++) // subtract 1 to exclude BAD_LOC
        cboxLocationCaught.AddItem(POKEMON_LOCATION_LOOKUP[ POKEMON_LOCATION_LOOKUP.get_virtual_index(i) ]->locationName);
    SendMessage(cboxGameFrom.WinHandle(),CB_SETHORIZONTALEXTENT,225,NULL);
    SendMessage(cboxLocationCaught.WinHandle(),CB_SETHORIZONTALEXTENT,225,NULL);
    boxNickname.SetTextLimit(10);
    boxIniLevel.SetTextLimit(3);
    boxTrainerIDReg.SetTextLimit(5);
    boxTrainerIDSec.SetTextLimit(5);
    boxTrainerName.SetTextLimit(7);
    btnBrowse.SetAttribute(Enabled,true);
    // set events
    rbtnBatteryFile.SetClickEvent( static_cast<EventDelegate> (&NewPokemon_dialog::_clkRBtnFileType) );
    rbtnExternalFile.SetClickEvent( static_cast<EventDelegate> (&NewPokemon_dialog::_clkRBtnFileType) );
    btnCreate.SetClickEvent( static_cast<EventDelegate> (&NewPokemon_dialog::_clkCreate) );
    btnCancel.SetClickEvent( static_cast<EventDelegate> (&NewPokemon_dialog::_clkCancel) );
    btnBrowse.SetClickEvent( static_cast<EventDelegate> (&NewPokemon_dialog::_clkBrowse) );
}
void NewPokemon_dialog::_clkCreate(Window*)
{
    // prelim. checks
    if (!cboxSpecies.IsItemSelected())
    {
        Message_dialog d("Choose a species for the new Pokemon.");
        d.ShowDlg(this);
        return;
    }
    if (!cboxGameFrom.IsItemSelected())
    {
        Message_dialog d("Choose a \"Game From\" flag for the new Pokemon.");
        d.ShowDlg(this);
        return;
    }
    if (!cboxLocationCaught.IsItemSelected())
    {
        Message_dialog d("Choose a \"Location Caught\" for the new Pokemon.");
        d.ShowDlg(this);
        return;
    }
    // make a pokemon entry using the specified information
    PokemonEntry entry;
    const char* errorMsg = NULL;
    str fileName; // if length>0 then route to file
    PokemonEntryType entryType = NewPok;
    if (rbtnBatteryFile.GetCheckState()==RadioButton::Checked)
    {
        rstringstream parser((const char*) boxAddress.GetText().c_str());
        str part; word index;
        parser.add_extra_delimiter(':');
        parser >> part >> index;
        Misc::to_lower(part);
        // Open the entries and mark the file as unsaved since these are new pokemon
        if (part=="party")
        {
            entryType = PartyPok;
            index--;
        }
        else if (part=="daycare")
        {
            entryType = DayCarePok;
            index--;
        }
        else if (part=="pcbox")
        {
            entryType = PCPok;
            word slot;
            if (! (parser >> slot) )
                errorMsg = "You must specifiy a PC slot number in addition to a box number";
            else
            {
                index *= 30;
                index += slot-1;
            }
        }
        else
            errorMsg = (boxAddress.GetTextLength()==0 ? "Specify a destination address." : "Bad token(s) in address.");
        if (errorMsg==NULL)
        {
            PokemonSaveFile::EntryOperationReturn r;
            r = pSaveFile->GetPokemonEntry(entry,entryType,saveStateType,index,true);
            if (r==PokemonSaveFile::EntryUnobtainable)
                errorMsg = "The specified entry is unobtainable.";
            else if (r==PokemonSaveFile::EntryOutstanding)
                errorMsg = "The specified entry is outstanding. Another service within PokemonEditor is currently using the entry.";
            else if (entry.pok.GetEntryType()==ErrorPok)
            {
                Message_dialog warning("Warning - PokemonEditor flagged the data in this entry as ErrorPok.");
                warning.ShowDlg(this);
            }
            else if (entry.pok.GetEntryType()!=NullPok)
            {
                YesNoCancel_dialog dialog("Warning - this entry already contains a valid Pokemon (POK=" + entry.pok.GetNickname().to_string() + "). Continuing will overwrite this entry's data. Do you want to continue?","Yes","No","Abort");
                DialogResult res = dialog.ShowDlg(this);
                if (res==No)
                    return;
                else if (res==Cancel)
                {
                    pSaveFile->PutPokemonEntry(entry,false);
                    CloseDlg(Cancel);
                    return;
                }
            }
        }
    }
    else if (rbtnExternalFile.GetCheckState()==RadioButton::Checked)
    {
        fileName = boxAddress.GetText();
        if (fileName.length()==0)
            errorMsg = "Provide a destination address on disk.";
    }
    else
        errorMsg = "Choose a save destination.";
    if (errorMsg==NULL)
    {
        // create the new pokemon and assign it to the entry's pok field; this will flag the pokemon as NewPok
        entry.pok = Pokemon(    (word) cboxSpecies.GetSelectedIndex(),
                                (PokemonGameFromFlag) cboxGameFrom.GetSelectedIndex(),
                                boxTrainerName.GetText().c_str(),
                                getNumericFromTBox<word> (boxTrainerIDReg),
                                getNumericFromTBox<word> (boxTrainerIDSec),
                                (rbtnTrainerGenderFemale.GetCheckState()==RadioButton::Checked ? Female : Male),
                                chkbxIsEgg.GetCheckState()==RadioButton::Checked,
                                &POKEMON_LOCATION_DATA[ POKEMON_LOCATION_LOOKUP.get_actual_index(cboxLocationCaught.GetSelectedIndex()) ],
                                (boxNickname.GetTextLength()>0 ? boxNickname.GetText().c_str() : NULL),
                                (boxIniLevel.GetTextLength()>0 ? (byte) getNumericFromTBox<word>(boxIniLevel) : 1)  );
        if (entryType==PartyPok || entryType==PCPok || entryType==DayCarePok)
            entry.pok.SetEntryType(entryType); // route the new pok to the appropriate entry.
    }
    if (errorMsg!=NULL)
    {
        Message_dialog dialog(errorMsg);
        dialog.ShowDlg(this);
        return;
    }
    // the entry passed
    PokemonEditor_dialog& editDialog = ::GetInActiveEditorDialogObject();
    if (fileName.length()==0)
        editDialog.InitializeEntry(pSaveFile,entry);
    else
        editDialog.InitializeEntry(fileName.c_str(),entry);
    editDialog.CreateDlg(this->GetParentWin());
    editDialog.Show();
    if (fileName.length()>0)
        editDialog.FlagUnsaved(); // new file hasn't been saved
    CloseDlg();
}
void NewPokemon_dialog::_clkBrowse(Window*)
{
    static dword cnt = 1;
    static rstringstream conv;
    common_dialogs::SaveFileDialog chooser;
    str file;
    chooser.AddFilter("pokentry","PokemonEditor Entry File");
    chooser.SetAllFileFilter();
    conv.clear();
    conv << "new_pok" << cnt;
    chooser.SetDefaultFileName(conv.get_device());
    file = chooser.RunDialog( *this );
    if (file.length()>0)
        boxAddress.SetText( file );
}
void NewPokemon_dialog::_clkRBtnFileType(Window*)
{
    if (rbtnBatteryFile.GetCheckState()==RadioButton::Checked)
        btnBrowse.SetAttribute(Enabled,true);
    else if (rbtnExternalFile.GetCheckState()==RadioButton::Checked)
        btnBrowse.SetAttribute(Enabled);
}