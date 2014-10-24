//RWin32LibMaster.h
#ifndef RWIN32LIBMASTER_H
#define RWIN32LIBMASTER_H

//  include API elements
#include <Windows.h>
#include <RichEdit.h>
#include <Commctrl.h>
//  include RLibrary elements
#define _BYTE_TDEF_DEF_ // use the byte typedef from Windows.h
#include "RTypesTypes.h"
#include "RContainer.h"
#include "RArray.h"
#include "RString.h"
#include "RStream.h"
#include "RStringStream.h"
//  include RWin32Lib elements
#include "RWin32LibDefaults.h"
#include "RWin32LibGlobalAppData.h"
#include "RWin32LibTypes.h"
#include "RWin32LibColors.h"
#include "RWin32LibFileIO.h"
#include "RWin32LibThreading.h"
#include "RWin32LibConsole.h"
#include "RWin32LibGraphics.h"
//      (main window functionality)
#include "RWin32LibWindow.h"
//      (system controls)
#include "RWin32LibButton.h"
#include "RWin32LibTextBox.h"
#include "RWin32LibListBox.h"
#include "RWin32LibComboBox.h"
#include "RWin32LibStatic.h"
//      (common controls)

//  other UI elements
#include "RWin32LibStdDialogs.h"
#include "RWin32LibMenu.h"
#include "RWin32LibApplication.h"

#endif