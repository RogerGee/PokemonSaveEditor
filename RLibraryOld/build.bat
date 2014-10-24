@echo off

if not exist .\Out md Out

cd Out

cl /c ..\*.cpp /EHsc

rc ..\RWin32LibResources.rc
move ..\RWin32LibResources.res .

lib /OUT:RLibraryOld.lib *.obj

cd ..

echo Built RLibraryOld
