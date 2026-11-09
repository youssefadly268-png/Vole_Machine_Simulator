@echo off
setlocal EnableDelayedExpansion
chcp 65001 >nul
cd /d "%~dp0"
title Vole Simulator - Build

rem ================= Settings =================
set QT_VER=6.8.1
set EXE_NAME=GUI.exe
set MIN_FREE_GB=5
set DEPS=%~dp0.deps
set QT_ROOT=%DEPS%\Qt
set LOG=%~dp0build_log.txt
rem ============================================

echo ==================================================
echo        Vole Simulator - Automatic Build
echo ==================================================
echo  Log file: %LOG%
echo.
echo Build started %date% %time% > "%LOG%"

rem ---------- 0. Checks ----------
echo [1/5] Checking system...
where curl >nul 2>nul
if errorlevel 1 (
    echo ERROR: curl not found. Windows 10 or 11 is required.
    goto :error
)

for /f %%G in ('powershell -NoProfile -Command "[math]::Floor((Get-Item '%~dp0').PSDrive.Free/1GB)"') do set FREE_GB=%%G
echo       Free space: %FREE_GB% GB
if %FREE_GB% LSS %MIN_FREE_GB% (
    echo ERROR: At least %MIN_FREE_GB% GB free space is required.
    goto :error
)

curl -s -I https://github.com >nul 2>nul
if errorlevel 1 (
    echo ERROR: No internet connection, or GitHub is blocked.
    goto :error
)
echo       OK
echo.

if not exist "%DEPS%" mkdir "%DEPS%"

rem ---------- 1. aqt ----------
echo [2/5] Preparing downloader...
if not exist "%DEPS%\aqt.exe" (
    call :download "%DEPS%\aqt.exe" "https://github.com/miurahr/aqtinstall/releases/latest/download/aqt.exe"
    if errorlevel 1 goto :error
)
echo       OK
echo.

rem ---------- 2. Qt + tools ----------
echo [3/5] Installing Qt and tools (first run only, may take 10-30 min)...
if not exist "%QT_ROOT%\%QT_VER%\mingw_64\bin\Qt6Core.dll" (
    echo       Downloading Qt %QT_VER% ...
    call :retry "%DEPS%\aqt.exe" install-qt windows desktop %QT_VER% win64_mingw -O "%QT_ROOT%"
    if errorlevel 1 goto :error
)
if not exist "%QT_ROOT%\Tools\mingw1310_64\bin\g++.exe" (
    echo       Downloading MinGW compiler ...
    call :retry "%DEPS%\aqt.exe" install-tool windows desktop tools_mingw1310 qt.tools.win64_mingw1310 -O "%QT_ROOT%"
    if errorlevel 1 goto :error
)
if not exist "%QT_ROOT%\Tools\Ninja\ninja.exe" (
    echo       Downloading Ninja ...
    call :retry "%DEPS%\aqt.exe" install-tool windows desktop tools_ninja -O "%QT_ROOT%"
    if errorlevel 1 goto :error
)
if not exist "%QT_ROOT%\Tools\CMake_64\bin\cmake.exe" (
    echo       Downloading CMake ...
    call :retry "%DEPS%\aqt.exe" install-tool windows desktop tools_cmake -O "%QT_ROOT%"
    if errorlevel 1 goto :error
)
echo       OK
echo.

set PATH=%QT_ROOT%\Tools\mingw1310_64\bin;%QT_ROOT%\Tools\Ninja;%QT_ROOT%\Tools\CMake_64\bin;%QT_ROOT%\%QT_VER%\mingw_64\bin;%PATH%

rem ---------- 3. Build ----------
echo [4/5] Building the project...
cmake -S . -B build -G Ninja -DCMAKE_PREFIX_PATH="%QT_ROOT%/%QT_VER%/mingw_64" -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -DCMAKE_BUILD_TYPE=Release >> "%LOG%" 2>&1
if errorlevel 1 goto :error
cmake --build build >> "%LOG%" 2>&1
if errorlevel 1 goto :error
if not exist "build\%EXE_NAME%" (
    echo ERROR: build\%EXE_NAME% was not created. Check the exe name in CMakeLists.txt.
    goto :error
)
echo       OK
echo.

rem ---------- 4. Deploy ----------
echo [5/5] Copying required DLLs next to the exe...
windeployqt.exe --release --compiler-runtime "build\%EXE_NAME%" >> "%LOG%" 2>&1
if errorlevel 1 goto :error
echo       OK
echo.

echo ==================================================
echo   SUCCESS!  Program: build\%EXE_NAME%
echo ==================================================
start "" "build\%EXE_NAME%"
pause
exit /b 0

rem ---------- helpers ----------
:download
for /l %%i in (1,1,3) do (
    curl -L --fail -o %1 %2 && exit /b 0
    echo       Retry %%i/3 ...
    timeout /t 3 >nul
)
exit /b 1

:retry
for /l %%i in (1,1,3) do (
    %* && exit /b 0
    echo       Retry %%i/3 ...
    timeout /t 5 >nul
)
exit /b 1

:error
echo.
echo ==================================================
echo   FAILED. See build_log.txt for details.
echo ==================================================
pause
exit /b 1