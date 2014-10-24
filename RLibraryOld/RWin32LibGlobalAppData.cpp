#include "RWin32LibMaster.h"
using namespace rtypes;
using namespace rtypes::rwin32;

namespace
{
    const char GenericMainWindowClass[] = "RWin32Lib_GenericWindowClass";
    
    Application* AppInst = 0;
}

HINSTANCE global_app_data::GetAppInstanceModule()
{
    if (AppInst==0)
        return NULL;
    return AppInst->_hInst;
}
const char* global_app_data::GetGenericMainWindowClass()
{
    return GenericMainWindowClass;
}
HWND global_app_data::GetMainWindow()
{
    if (AppInst==0)
        return NULL; // no main app window at this time
    if (AppInst->GetMainAppWindow()==0)
        return NULL;
    return AppInst->GetMainAppWindow()->WinHandle();
}
Application* global_app_data::GetRegisteredApplication()
{
    return AppInst;
}

bool Application::RegisterApplication(HINSTANCE hInstance,
                SimpleResource ResAppIcon,
                SimpleResource ResAppIconSmall,
                SimpleResource ResAppCursor)
{
    if (AppInst)
        // can't register because another application is registered
        return false;
    // load any modules needed by RWin32Lib
    //      !!use RichEdit v1.0 for backwards compatibility!!
    if ( !LoadLibrary(TEXT("Riched32.dll")) )
        return false;

    // initialize common control elements/visual style elements
    INITCOMMONCONTROLSEX initCmmCtlData;
    initCmmCtlData.dwSize = sizeof( INITCOMMONCONTROLSEX );
    initCmmCtlData.dwICC = NULL;
    // apply control flags to register specific window classes
    
    if (!InitCommonControlsEx(&initCmmCtlData))
        // failure indicates the control classes cannot be registered,
        // which may indicate non-linkage to comctl32.dll
        return false;

    // set app instance
    _hInst = hInstance;
    _resAppIcon = ResAppIcon;
    _resAppIconSmall = ResAppIconSmall;
    _resAppCursor = ResAppCursor;
    AppInst = this;

    // define and register generic main window class
    WNDCLASSEX wcex_generic;
    wcex_generic.cbClsExtra = 0;
    wcex_generic.cbSize = sizeof(WNDCLASSEX);
    wcex_generic.cbWndExtra = 0;
    wcex_generic.hbrBackground = (HBRUSH) (COLOR_WINDOW+1);
    if (_resAppCursor.resourceID == NULL)
        // use the default application cursor
        wcex_generic.hCursor = LoadCursor(NULL,IDI_APPLICATION);
    else
        wcex_generic.hCursor = ( _resAppCursor.useAppModule 
            ? LoadCursor(_hInst,MAKEINTRESOURCE(_resAppCursor.resourceID))
            : LoadCursor(NULL,MAKEINTRESOURCE(_resAppCursor.resourceID)) );
    if (_resAppIcon.resourceID == NULL)
        // use a default system icon
        wcex_generic.hIcon = LoadIcon(NULL,IDI_APPLICATION);
    else
        wcex_generic.hIcon = ( _resAppIcon.useAppModule
            ? LoadIcon(_hInst,MAKEINTRESOURCE(_resAppIcon.resourceID))
            : LoadIcon(NULL,MAKEINTRESOURCE(_resAppIcon.resourceID)) );
    if (_resAppIconSmall.resourceID == NULL)
        //use a default system icon
        wcex_generic.hIconSm = LoadIcon(NULL,IDI_APPLICATION);
    else
        wcex_generic.hIconSm = ( _resAppIconSmall.useAppModule
            ? LoadIcon(_hInst,MAKEINTRESOURCE(_resAppIconSmall.resourceID))
            : LoadIcon(NULL,MAKEINTRESOURCE(_resAppIconSmall.resourceID)) );
    wcex_generic.hInstance = _hInst;
    wcex_generic.lpfnWndProc = Window::GetMasterWndProc();
    wcex_generic.lpszClassName = GenericMainWindowClass;
    wcex_generic.lpszMenuName = NULL;
    wcex_generic.style = CS_HREDRAW | CS_VREDRAW;

    //  register the class
    if (!RegisterClassEx(&wcex_generic))
        return false;
    _init = true;
    return true;
}