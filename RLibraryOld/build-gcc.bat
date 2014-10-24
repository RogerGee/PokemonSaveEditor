@echo off

ECHO This compiler is not supported at this time.
GOTO END

if not exist .\Out md Out

cd Out

g++ -c ..\*.cpp

windres --input-format=rc --output-format=res ..\RWin32LibResources.rc -oRWin32LibResources.o

ar rcs RWin32Old.a *.o

cd ..

echo Built RLibraryOld

:END
