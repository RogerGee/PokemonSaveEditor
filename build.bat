:: build.bat - Builds PokemonSaveEditor using the VC++ compiler cl.exe
:: run "build both" to build both PokemonSaveEditor and its
:: dependency, RLibraryOld
@ECHO OFF

if '%1'=='both' (
	CD RLibraryOld
	CALL build.bat
	CD ..
)

IF NOT EXIST .\Out MD .\Out

CD .\Out

RC ..\PokemonEditorResources.rc
MOVE ..\PokemonEditorResources.res .
CL ..\*.cpp RLibraryOld.lib user32.lib gdi32.lib shell32.lib comctl32.lib comdlg32.lib PokemonEditorResources.res RWin32LibResources.res /EHsc /FePokemonSaveEditor.exe /I..\RLibraryOld /link /LIBPATH:..\RLibraryOld\Out /SUBSYSTEM:WINDOWS

CD ..

ECHO Built PokemonSaveEditor
