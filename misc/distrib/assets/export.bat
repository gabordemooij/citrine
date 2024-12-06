

setlocal enabledelayedexpansion

rem Ensure at least one argument is provided
if "%~1"=="" (
    echo No arguments provided.
    exit /b 1
)

rem Initialize an empty variable to hold the processed filenames
set "args="

rem Loop through all command-line arguments
for %%i in (%*) do (
    rem Extract the filename with extension
    set "args=!args! %%~nxi"
)

rem Call the other batch file with the filenames only
ctrnl.exe pak-o-mat.ctr %args%


