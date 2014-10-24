//RWin32LibGlobalAppData.h
#ifndef RWIN32LIBGLOBALAPPDATA_H
#define RWIN32LIBGLOBALAPPDATA_H

// RWin32Lib Resource IDs
#define RWIN32_GENERIC_DIALOG 33
#define RWIN32_MANIFEST_RESOURCE 34

namespace rtypes
{
    namespace rwin32
    {
        namespace global_app_data
        {
            HINSTANCE GetAppInstanceModule();
            const char* GetGenericMainWindowClass();
            HWND GetMainWindow();
        }
    }
}

#endif
