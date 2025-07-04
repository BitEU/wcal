@echo off
echo Building Windows Calendar App...

REM Check if MSVC compiler is available
where cl >nul 2>nul
if errorlevel 1 (
    echo Error: MSVC compiler not found. Please run this from a Visual Studio Developer Command Prompt.
    echo Or install Visual Studio Build Tools and run vcvars64.bat first.
    pause
    exit /b 1
)

REM Clean previous build
echo Cleaning previous build...
del *.obj calcurse.exe 2>nul

REM Compile source files
echo Compiling source files...
cl /c /W3 /O2 /TC /nologo main.c
if errorlevel 1 goto :error

cl /c /W3 /O2 /TC /nologo ui.c
if errorlevel 1 goto :error

cl /c /W3 /O2 /TC /nologo calendar.c
if errorlevel 1 goto :error

cl /c /W3 /O2 /TC /nologo appointments.c
if errorlevel 1 goto :error

cl /c /W3 /O2 /TC /nologo todo.c
if errorlevel 1 goto :error

cl /c /W3 /O2 /TC /nologo storage.c
if errorlevel 1 goto :error

cl /c /W3 /O2 /TC /nologo input.c
if errorlevel 1 goto :error

REM Link executable
echo Linking executable...
cl main.obj ui.obj calendar.obj appointments.obj todo.obj storage.obj input.obj /Fe:calcurse.exe /link kernel32.lib user32.lib
if errorlevel 1 goto :error

echo.
echo Build successful! Run calcurse.exe to start the calendar app.
echo.
pause
exit /b 0

:error
echo.
echo Build failed!
pause
exit /b 1