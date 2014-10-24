#include "RWin32LibMaster.h"
using namespace rtypes;
using namespace rtypes::rwin32;
using namespace rtypes::rwin32::common_dialogs;

filesysDialogBase::filesysDialogBase()
{
    _includeAllFiles = false;
    _selectMultipleFiles = false;
}
void filesysDialogBase::AddFilter(const str& ext,const str& descript)
{
    _extensions.push_back(ext);
    _descriptions.push_back(descript);
}
void filesysDialogBase::_setFilter()
{// provide a filter string that presents file types in the default format
    const str* descriptions = &_descriptions[0];
    const str* extensions = &_extensions[0];
    dword length = (_descriptions.size()==_extensions.size() ? _descriptions.size() : 0);
    _filter.clear();
    for (dword i = 0;i<length;i++)
    {
        // add the description, followed by the extension
        _filter += descriptions[i];
        _filter += " (*.";
        _filter += extensions[i];
        _filter.push_back(')');
        _filter.push_back('\0');
        // add the extension filter
        _filter += "*.";
        _filter += extensions[i];
        _filter.push_back('\0');
    }
    if (_includeAllFiles)
    {// add an 'All Files' filter
        _filter += "All Files (*.*)";
        _filter.push_back('\0');
        _filter += "*.*";
        _filter.push_back('\0');
    }
}
str filesysDialogBase::_getFile(bool open,const Window& owner)
{
    str fileBuf(MAX_PATH+1);
    OPENFILENAME ofn;
    // copy the default file name into the file buffer
    dword i;
    for (i = 0;i<_defFile.size() && i<MAX_PATH;i++)
        fileBuf[i] = _defFile[i];
    fileBuf[i] = '\0'; // A quirk in the API makes this buffer need to be null terminated.
    // ensure that the filter member is prepared
    _setFilter();
    // create the OPENFILENAME structure
    ZeroMemory(&ofn,sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = owner.WinHandle();
    ofn.lpstrFilter = _filter.c_str();
    ofn.lpstrDefExt = _defExt.c_str();
    ofn.lpstrFile = &fileBuf[0];
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = (open ? OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY : OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT);
    if (open)
    {
        if (!GetOpenFileName(&ofn))
            return "";
    }
    else
    {
        if (!GetSaveFileName(&ofn))
            return "";
    }
    // find the size of the buffer so that we can accurately report it
    i = 0; // reuse i from earlier
    while (i<MAX_PATH /* just in case */ && fileBuf[i])
        i++;
    fileBuf.truncate(i);
    return fileBuf;
}