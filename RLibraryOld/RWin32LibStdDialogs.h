//RWin32LibStdDialogs.h
#ifndef RWIN32LIBSTDDIALOGS_H
#define RWIN32LIBSTDDIALOGS_H

namespace rtypes
{
    namespace rwin32
    {
        namespace common_dialogs
        {
            /*
                Using File System Common Dialogs
                    -extensions:
                        specify only the extension, not a '.' or "*."
            */
            class filesysDialogBase
            {
            public:
                virtual str RunDialog(const Window& OwnerWindow) = 0;
                void SetAllFileFilter() { _includeAllFiles = true; }
                void SetSelectMultipleFiles() { _selectMultipleFiles = true; }
                void SetDefaultFileName(const str& FileName) { _defFile = FileName; }
                void SetDefaultExtension(const str& Ext) { _defExt = Ext; }
                void AddFilter(const str& Extension,const str& Description);
            protected:
                filesysDialogBase();
                bool _includeAllFiles, _selectMultipleFiles;
                container<str> _extensions, _descriptions;
                str _defExt, _defFile, _filter;
                str _getFile(bool open /* if open, open file dialog, else save file dialog */,
                    const Window& owner);
            private:
                void _setFilter();
            };

            class OpenFileDialog : public filesysDialogBase
            {
            public:
                virtual str RunDialog(const Window& OwnerWindow) { return _getFile(true,OwnerWindow); }
            };

            class SaveFileDialog : public filesysDialogBase
            {
            public:
                virtual str RunDialog(const Window& OwnerWindow) { return _getFile(false,OwnerWindow); }
            };
        }
    }
}

#endif