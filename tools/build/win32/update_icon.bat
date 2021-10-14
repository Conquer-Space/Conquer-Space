@echo off

set DATA_PATH=%~dp0..\..\..\binaries\data\

rc >nul 2>&1 && (
    rc /fo %DATA_PATH%app.o %DATA_PATH%app.rc
) || (
    echo This has to be run with the Visual Studio command prompt.
    pause
)

