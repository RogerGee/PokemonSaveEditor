:: build.bat - Builds PokemonSaveEditor using the MinGW GCC compiler port (g++.exe)
:: run "build both" to build both PokemonSaveEditor and its
:: dependency, RLibraryOld
@ECHO OFF

ECHO This compiler is not supported at this time.
GOTO END

if '%1'=='both' (
	CD RLibraryOld
	CALL build-gcc.bat
	CD ..
)

IF NOT EXIST .\Out MD .\Out

CD .\Out

g++ ..\*.cpp ..\RLibraryOld\Out\RLibraryOld.a ..\RLibraryOld\Out\RWin32LibResources.o -oPokemonSaveEditor.exe -I ..\RLibraryOld -Wl,-subsystem,windows

CD ..

ECHO Built PokemonSaveEditor

:END
