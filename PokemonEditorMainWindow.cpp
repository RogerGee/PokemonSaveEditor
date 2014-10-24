#include "MasterInclude.h"
#include <Shlobj.h>
using namespace rtypes;
using namespace rtypes::rwin32;
using namespace PokemonEditor;
using namespace PokemonEditorWindow;

const char* const PokemonEditorWindow::PokemonEditorTitle = "PokemonEditor";

namespace
{
    struct IniData
    {
        str lastFile;
        int x, y;
        int width, height;
    };
    fileIO::FileStream& operator >>(fileIO::FileStream& stream,IniData& obj)
    {
        stream.getline(obj.lastFile);
        stream >> obj.x >> obj.y >> obj.width >> obj.height;
        return stream;
    }
    fileIO::FileStream& operator <<(fileIO::FileStream& stream,const IniData& obj)
    {
        stream.putline(obj.lastFile);
        stream << obj.x << ' ' << obj.y << endline
            << obj.width << ' ' << obj.height << endline;
        return stream;
    }
    const char* const INI_DATA_FILE_PATH = "\\pokconfig.ini";
    IniData iniData;
    bool LoadIniData()
    {
        str file(MAX_PATH);
        if (SUCCEEDED(::SHGetFolderPath(NULL,CSIDL_APPDATA,NULL,0,&file[0])))
        {
            fileIO::File f;
            dword i = 0;
            while (file[i] != 0)
                i++;
            file.resize(i);
            file += INI_DATA_FILE_PATH;
            if (!f.Open(file,fileIO::GenericRead,fileIO::FileNoShare,true))
                return false;
            fileIO::FileStream reader(f);
            reader >> iniData;
            return true;
        }
        return false;
    }
    void SaveIniData()
    {
        str file(MAX_PATH);
        if (SUCCEEDED(::SHGetFolderPath(NULL,CSIDL_APPDATA,NULL,0,&file[0])))
        {
            fileIO::File f;
            dword i = 0;
            while (file[i] != 0)
                i++;
            file.resize(i);
            file += INI_DATA_FILE_PATH;
            f.Open(file,fileIO::GenericWrite,fileIO::FileNoShare,false,true);
            fileIO::FileStream writer(f);
            writer << iniData;
        }
    }

    void QuickMessage(const char* msg,const Window* parent)
    {
        Message_dialog dialog(msg);
        dialog.ShowDlg(parent);
    }
}

MainWindow::MainWindow()
    : listingDialog(&saveFile)
{
    if (!LoadIniData())
        Create();
    else
        Create(iniData.x,iniData.y,iniData.width,iniData.height);
    SetText(PokemonEditorTitle);
    _initControls();
    saveFileState = CurrentState;
    listingDialog.SetSaveStateType(CurrentState);
}
MainWindow::~MainWindow()
{

    SaveIniData();
}
bool MainWindow::LoadBatteryFile(const str& fileName)
{
    PokemonSaveFile::LoadOperationReturn retVal;
    saveFileName = fileName;
    retVal = saveFile.LoadFrom(saveFileName);
    if (saveFileName.length()==0)
        return false;
    if (retVal!=PokemonSaveFile::LoadSuccess)
    {
        if (retVal==PokemonSaveFile::LoadFailFileNotOpened)
        {
            Message_dialog errorDialog("Could not open file '" + saveFileName + ".' Make sure that the file exists and isn't being used by another service within Windows.");
            errorDialog.ShowDlg(this);
        }
        else if (retVal==PokemonSaveFile::LoadFailBadValidation)
        {
            YesNoCancel_dialog errorDialog("Could not process file '" + saveFileName + ".' The file failed validation checks. (Sometimes, hacked ROMs generate battery files that don't conform to the standard battery file format. Click 'Try Again' to re-process the file ignoring checksum validation checks.)","Try Again","Return","Help");
            DialogWindow::DialogResult r = errorDialog.ShowDlg(this);
            if (r==DialogWindow::Yes) // reload without checksum validation checks
            {
                retVal = saveFile.LoadFrom(saveFileName,DemoJirachiDisk,true);
                if (retVal!=PokemonSaveFile::LoadSuccess)
                {
                    Message_dialog msg("Could not process file.");
                    msg.ShowDlg(this);
                }
            }
            else if (r==DialogWindow::Cancel) // start the help docs
                ;
        }
        if (retVal!=PokemonSaveFile::LoadSuccess)
            return false;
    }
    fileIO::File parser(saveFileName);
    SetText(parser.GetSafeName_parser()+" - "+PokemonEditorTitle);
    // change enable state of menu items
    itemOpenBatteryFile.DisableMenuItem();
    itemSaveBatteryFile.EnableMenuItem();
    itemSaveBatteryFileAs.EnableMenuItem();
    itemCloseBatteryFile.EnableMenuItem();
    return true;
}
bool MainWindow::LoadEntryFile(const str& fileName)
{

    return false;
}
bool MainWindow::LoadEntry(PokemonEntryType type,dword occurInd)
{
    PokemonEntry entry;
    if (saveFileName.length()>0 && saveFile.GetPokemonEntry(entry,type,saveFileState,occurInd)==PokemonSaveFile::EntryObtained)
    {
        PokemonEditor_dialog& dialog = _getInActiveEntryDialog();
        dialog.InitializeEntry(&saveFile,entry);
        dialog.CreateDlg(this);
        dialog.Show();
        return true;
    }
    return false;   
}
void MainWindow::SimulateClick(MainWindowClickSimulation simul)
{
    if (simul==HideConsoleClick)
        _clkHideConsole(itemHideConsole.GetID());
    else if (simul==RequestCloseClick)
        Close();
    else if (simul==SaveBatteryFileClick)
        _clkSaveBatteryFile(itemSaveBatteryFile.GetID());

}
void MainWindow::OnClose(const EventData& data)
{
    data.tag = (_closeActiveDialogs() ? TRUE : FALSE);
    if (data.tag!=FALSE)
    {
        if (saveFile.WasAltered())
        {
            YesNoCancel_dialog dialog("Warning - the save file has been altered but not saved. Do you want to save it now?");
            DialogWindow::DialogResult r = dialog.ShowDlg(this);
            if (r==DialogWindow::Yes)
                SimulateClick(SaveBatteryFileClick);
            else if (r==DialogWindow::Cancel)
                data.tag = FALSE;
        }
    }
    if (data.tag)
    {
        Point location;
        Size size;
        location = GetLocation();
        size = GetSize();
        iniData.x = location.x;
        iniData.y = location.y;
        iniData.width = size.width;
        iniData.height = size.height;
    }
}
void MainWindow::OnResized(const WinChngEventData& data)
{
    if (data.WasMinimized())
        _minimizeActiveDialogs();
    else
        _restoreActiveDialogs();
}
void MainWindow::_initControls()
{
    itemFile.SetText("&File");
    itemEdit.SetText("&Edit");
    itemView.SetText("&View");
    itemPokemon.SetText("&Pokemon");
    itemOps.SetText("&Operations");
    itemWindow.SetText("&Window");
    itemHelp.SetText("&Help");
    // Initialize sub menu items
    // file menu
    itemOpenBatteryFile.SetText("&Open Battery File...");
    itemOpenBatteryFile.SetEvent( static_cast<EventDelegate_id> (&MainWindow::_clkOpenBatteryFile) );
    itemOpenElement.SetText("&Open Element...");
    itemOpenElement.SetEvent( static_cast<EventDelegate_id> (&MainWindow::_clkOpenElement) );
    itemSaveBatteryFile.SetText("&Save Battery File");
    itemSaveBatteryFile.SetEvent( static_cast<EventDelegate_id> (&MainWindow::_clkSaveBatteryFile) );
    itemSaveBatteryFileAs.SetText("Save Battery File &As...");
    itemSaveBatteryFileAs.SetEvent( static_cast<EventDelegate_id> (&MainWindow::_clkSaveBatteryFileAs) );
    itemSaveElementAs.SetText("Save Element &As...");
    itemSaveElementAs.SetEvent( static_cast<EventDelegate_id> (&MainWindow::_clkSaveElementAs) );
    itemCloseBatteryFile.SetText("&Close Battery File");
    itemCloseBatteryFile.SetEvent( static_cast<EventDelegate_id> (&MainWindow::_clkCloseBatteryFile) );
    itemExit.SetText("E&xit");
    itemExit.SetEvent( static_cast<EventDelegate_id> (&MainWindow::_clkExit) );
    // view menu
    itemPokemonListing.SetText("Pokemon &Listing...");
    itemPokemonListing.SetEvent( static_cast<EventDelegate_id> (&MainWindow::_clkPokemonListing) );
    // pokemon menu
    itemNewPokemon.SetText("New Pokemon...");
    itemNewPokemon.SetEvent( static_cast<EventDelegate_id> (&MainWindow::_clkNewPokemon) );
    itemPartySub.SetText("Party Pokemon");
    itemDayCareSub.SetText("Day Care Pokemon");
    itemPCSub.SetText("PC Pokemon");
    // pokemon->party menu
    for (int i = 0;i<6;i++)
    {
        rstringstream ss;
        ss << "Party Pokemon Slot #" << i+1;
        itemPartyPoks[i].SetText(ss.get_device());
        itemPartyPoks[i].SetEvent( static_cast<EventDelegate_id> (&MainWindow::_clkPokemonEntry) );
    }
    // pokemon->day care menu
    for (int i = 0;i<2;i++)
    {
        rstringstream ss;
        ss << "Day Care Pokemon #" << i+1;
        itemDayCarePoks[i].SetText(ss.get_device());
        itemDayCarePoks[i].SetEvent( static_cast<EventDelegate_id> (&MainWindow::_clkPokemonEntry) );
    }
    itemDayCarePoks[2].SetText("(FrLg only) Single Day Care Pokemon");
    itemDayCarePoks[2].SetEvent( static_cast<EventDelegate_id> (&MainWindow::_clkPokemonEntry) );
    // pokemon->pc menu (initialize pc box sub menus)
    for (int i = 0;i<14;i++)
    {
        rstringstream ss;
        ss << "PC Box #" << i+1;
        itemPCBoxSub[i].SetText(ss.get_device());
        // initialize slot items for box
        for (int j = 0;j<30;j++)
        {
            rstringstream ss2;
            ss2 << "Slot #" << j+1;
            itemPCPoks[i][j].SetText(ss2.get_device());
            itemPCPoks[i][j].SetEvent( static_cast<EventDelegate_id> (&MainWindow::_clkPokemonEntry) );
        }
    }
    // window menu
    itemConsole.SetText("&Console Window");
    itemConsole.SetEvent( static_cast<EventDelegate_id> (&MainWindow::_clkConsole) );
    itemHideConsole.SetText("&Hide Console Window");
    itemHideConsole.SetEvent( static_cast<EventDelegate_id> (&MainWindow::_clkHideConsole) );
    itemNewWindow.SetText("&New Window");
    itemNewWindow.SetEvent( static_cast<EventDelegate_id> (&MainWindow::_clkNewWindow) );
    itemCloseActiveWindows.SetText("C&lose Active Windows");
    itemCloseActiveWindows.SetEvent( static_cast<EventDelegate_id> (&MainWindow::_clkCloseActiveWindows) );
    
    menuBar.AddItem(itemFile);
    menuBar.AddItem(itemEdit);
    menuBar.AddItem(itemView);
    menuBar.AddItem(itemPokemon);
    menuBar.AddItem(itemOps);
    menuBar.AddItem(itemWindow);
    menuBar.AddItem(itemHelp);

    // add items to main menus
    //  file
    itemFile.AddItem(itemOpenBatteryFile);
    itemFile.AddItem(itemOpenElement);
    itemFile.AddSeparator();
    itemFile.AddItem(itemSaveBatteryFile); itemSaveBatteryFile.DisableMenuItem();
    itemFile.AddItem(itemSaveBatteryFileAs); itemSaveBatteryFileAs.DisableMenuItem();
    itemFile.AddItem(itemSaveElementAs); itemSaveElementAs.DisableMenuItem();
    itemFile.AddSeparator();
    itemFile.AddItem(itemCloseBatteryFile); itemCloseBatteryFile.DisableMenuItem();
    itemFile.AddSeparator();
    itemFile.AddItem(itemExit);
    //  view
    itemView.AddItem(itemPokemonListing);
    //  pokemon
    itemPokemon.AddItem(itemNewPokemon);
    itemPokemon.AddItem(itemPartySub);
    itemPokemon.AddItem(itemDayCareSub);
    itemPokemon.AddItem(itemPCSub);
    //  pokemon->party
    for (int i = 0;i<6;i++)
        itemPartySub.AddItem(itemPartyPoks[i]);
    //  pokemon->day care
    for (int i = 0;i<3;i++)
        itemDayCareSub.AddItem(itemDayCarePoks[i]);
    // pokemon->pc
    for (int i = 0;i<14;i++)
    {
        itemPCSub.AddItem(itemPCBoxSub[i]);
        for (int j = 0;j<30;j++)
            itemPCBoxSub[i].AddItem(itemPCPoks[i][j]);
    }
    // window
    itemWindow.AddItem(itemConsole);
    itemWindow.AddItem(itemHideConsole); itemHideConsole.DisableMenuItem();
    itemWindow.AddSeparator();
    itemWindow.AddItem(itemNewWindow);
    itemWindow.AddItem(itemCloseActiveWindows);
    
    menuBar.Bind(*this); // rwin32 demands that this be the last statement in menu initialization
}
PokemonEditor_dialog& MainWindow::_getInActiveEntryDialog()
{
    dword i = 0;
    while (i<activeEntryDialogs.length() && activeEntryDialogs[i].WinHandle()!=NULL)
        i++;
    if (i>=activeEntryDialogs.length()) // have to allocate a new one
        activeEntryDialogs.add();
    return activeEntryDialogs[i];
}
void MainWindow::_clkOpenBatteryFile(dword)
{
    common_dialogs::OpenFileDialog openDialog;
    openDialog.AddFilter("sav","Pokemon Battery File");
    openDialog.AddFilter("sa1","Pokemon Battery File");
    openDialog.AddFilter("sa2","Pokemon Battery File");
    openDialog.AddFilter("sa3","Pokemon Battery File");
    openDialog.AddFilter("sa4","Pokemon Battery File");
    LoadBatteryFile( openDialog.RunDialog(*this) );
}
void MainWindow::_clkOpenElement(dword)
{
    YesNoCancel_dialog dialog("You can select one of the three options below:","Option1");
    DialogWindow::DialogResult r = dialog.ShowDlg();
}
void MainWindow::_clkSaveBatteryFile(dword)
{
    if (saveFileName.length()==0)
    {// shouldn't happen but I put it here
        ::QuickMessage("No battery file is loaded.",this);
        return;
    }
    if (!saveFile.SaveTo(saveFileName))
        ::QuickMessage(str(str("Could not save to file location '") + ".' Check to make sure the file path exists and is not read-only. If the problem persists, save the data in a new file and close this one.").c_str(),this);
}
void MainWindow::_clkSaveBatteryFileAs(dword)
{
    bool dummy;
    YesNoCancel_dialog prompt("Choose the format that you wish to save:\r\n","Battery File","Memory File");
    DialogWindow::DialogResult r = prompt.ShowDlg(this);
    common_dialogs::SaveFileDialog saveDialog;
    saveDialog.AddFilter("sav","Pokemon Battery File");
    saveDialog.SetAllFileFilter();
    if (r==DialogWindow::Yes)
    {// save an actual battery file
        static word fileCnt = 1; // make this static so any instance of MainWindow will see its value; keep track of user saves for user
        str fileName;
        rstringstream ss;
        ss << ToString_PokemonGameFromFlag_abbr(saveFile.GetGameFrom(dummy)) << "_battery" << fileCnt;
        saveDialog.SetDefaultFileName(ss.get_device());
        fileName = saveDialog.RunDialog(*this);
        if (fileName.length()>0)
        {
            if (saveFile.SaveTo(fileName))
                ++fileCnt; // only increment on successful saves
            else
                ::QuickMessage(str("Could not save to file location '"+fileName+".' Check to make sure the file path exists and that both the file and path are not read-only.").c_str(),this);
        }
    }
    else if (r==DialogWindow::No)
    {// save a memory file
        static word fileCnt = 1;
        str fileName;
        rstringstream ss;
        ss << ToString_PokemonGameFromFlag_abbr(saveFile.GetGameFrom(dummy)) << "_memory" << fileCnt;
        saveDialog.SetDefaultFileName(ss.get_device());
        fileName = saveDialog.RunDialog(*this);
        if (fileName.length()>0)
        {
            bool b = false;
            if (saveFileState==CurrentState)
                b = saveFile.SaveCurrentMemoryFile(fileName);
            else if (saveFileState==PreviousState)
                b = saveFile.SavePreviousMemoryFile(fileName);
            if (!b)
                ::QuickMessage(str("Could not save to file location '"+fileName+".' Check to make sure the file path exists and that both the file and path are not read-only.").c_str(),this);
            else
                fileCnt++;
        }
    }
}
void MainWindow::_clkSaveElementAs(dword)
{
    // allow the user to choose from among any open pokemon dialogs an element to save

}
void MainWindow::_clkCloseBatteryFile(dword)
{
    if (saveFile.WasAltered())
    {
        YesNoCancel_dialog dialog("The changes to the loaded battery file have not been saved. Do you want to save these changes to the battery file?");
        DialogWindow::DialogResult r = dialog.ShowDlg(this);
        if (r==DialogWindow::Yes)
            _clkSaveBatteryFile(itemSaveBatteryFile.GetID()); // simulate a menu item click for save battery file
        else if (r==DialogWindow::Cancel)
            return;
    }
    saveFile.Clear();
    listingDialog.ResetListing(); // remove the shallow entry listing that the object has cached
    SetText(PokemonEditorTitle);
    // change enable state of menu items
    itemOpenBatteryFile.EnableMenuItem();
    itemSaveBatteryFile.DisableMenuItem();
    itemSaveBatteryFileAs.DisableMenuItem();
    itemCloseBatteryFile.DisableMenuItem();
}
void MainWindow::_clkExit(dword)
{
    Close();
}
void MainWindow::_clkPokemonListing(dword)
{
    if (listingDialog.WinHandle()==NULL)
    {
        listingDialog.CreateDlg(this);
        listingDialog.Show();
    }
    else
        listingDialog.Show(); // it might be hidden
}
void MainWindow::_clkNewPokemon(dword)
{
    NewPokemon_dialog dialog(&saveFile,saveFileState);
    dialog.ShowDlg(this);
}
void MainWindow::_clkPokemonEntry(dword id)
{
    if (saveFileName.length()==0)
    {
        Message_dialog dialog("No loaded battery file data available.");
        dialog.ShowDlg(this);
        return;
    }
    // the menu item ids are allocated by RWin32Lib sequentially,
    // so I can use them to determine offsets for pokemon entries
    // in this case, the constructors we called by a linear constructor for an array of MenuItem objects
    const char* msg = NULL;
    PokemonEntryType t = NullPok;
    dword occurance = 0;
    if (id>=itemPartyPoks[0].GetID() && id<=itemPartyPoks[5].GetID()) // user clicked a party pokemon entry item
    {
        t = PartyPok;
        occurance = id-itemPartyPoks[0].GetID();
    }
    else if (id>=itemDayCarePoks[0].GetID() && id<=itemDayCarePoks[2].GetID()) // user clicked a day care pokemon entry item
    {
        t = DayCarePok;
        occurance = id-itemDayCarePoks[0].GetID();
    }
    else
    {// search through the pc box entry items
        dword pcBox = 0;
        while (pcBox<14 && itemPCPoks[pcBox][0].GetID()>id)
            pcBox++;
        if (pcBox<14)
        {
            t = PCPok;
            occurance = pcBox*30+(id-itemPCPoks[pcBox][0].GetID());
        }
    }
    if (t!=NullPok)
    {
        PokemonEntry theEntry;
        PokemonSaveFile::EntryOperationReturn r = saveFile.GetPokemonEntry(theEntry,t,saveFileState,occurance);
        if (r==PokemonSaveFile::EntryOutstanding)
            msg = "The specified Pokemon entry is outstanding. Another service within PokemonEditor is currently using the entry.";
        else if (r==PokemonSaveFile::EntryUnobtainable)
            msg = "The specified Pokemon entry could not be obtained. This may indicate a corruption in the data at the entry offset.";
        else
        {
            PokemonEditor_dialog& dialog = _getInActiveEntryDialog();
            dialog.InitializeEntry(&saveFile,theEntry);
            dialog.CreateDlg(this);
            dialog.Show();
        }
    }
    else
        msg = "DebugError: no corresponding menu item found!";
    if (msg!=NULL)
    {
        Message_dialog dialog(msg);
        dialog.ShowDlg(this);
    }
}
void MainWindow::_clkConsole(dword)
{
    if (!PokemonEditorConsole::IsConsoleInit()) 
    {
        PokemonEditorConsole::InitConsole();
        itemHideConsole.EnableMenuItem();
        PokemonEditorConsole::SetDataForConsole(&saveFile,this);
    }
    else
        PokemonEditorConsole::FocusConsole();
}
void MainWindow::_clkHideConsole(dword)
{
    PokemonEditorConsole::PackUpConsole();
    itemHideConsole.DisableMenuItem();
}
void MainWindow::_clkNewWindow(dword)
{
}
void MainWindow::_clkCloseActiveWindows(dword)
{
    _closeActiveDialogs();
    // hide the listing dialog if it's created so that it doesn't have to be created again
    if (listingDialog.WinHandle()!=NULL)
        listingDialog.Show(SW_HIDE);
}
bool MainWindow::_closeActiveDialogs()
{ // this closes all active pokemon editor dialogs, plus the listing dialog, which may have some pokemon editor dialogs
    bool b = true;
    for (dword i = 0;i<activeEntryDialogs.length();i++)
    {
        if (activeEntryDialogs[i].WinHandle()!=NULL)
        {
            activeEntryDialogs[i].FlashWindow(FlashCaption,5);
            if (!activeEntryDialogs[i].Close())
                b = false;
        }
    }
    return listingDialog.CloseActiveDialogs() && NewPokemon_dialog::CloseActiveDialogs() && PokemonEditor_dialog::CloseActiveInfoDialogs() && b;
}
bool MainWindow::_minimizeActiveDialogs()
{
    bool b = true;
    for (dword i = 0;i<activeEntryDialogs.length();i++)
    {
        if (activeEntryDialogs[i].WinHandle()!=NULL && !activeEntryDialogs[i].Minimize())
            b = false;
    }
    if (!listingDialog.Minimize())
        b = false;
    if (!listingDialog.MinimizeActiveDialogs())
        b = false;
    if (!NewPokemon_dialog::MinimizeActiveDialogs())
        b = false;
    PokemonEditor_dialog::MinimizeActiveInfoDialogs(); // don't worry about return value, should always be true
    return b;
}
bool MainWindow::_restoreActiveDialogs()
{
    bool b = true;
    for (dword i = 0;i<activeEntryDialogs.length();i++)
    {
        if (activeEntryDialogs[i].WinHandle()!=NULL && !activeEntryDialogs[i].Restore())
            b = false;
    }
    if (!listingDialog.Restore())
        b = false;
    if (!listingDialog.RestoreActiveDialogs())
        b = false;
    if (!NewPokemon_dialog::RestoreActiveDialogs())
        b = false;
    PokemonEditor_dialog::RestoreActiveInfoDialogs(); // don't worry about return values, should always return true
    return b;
}