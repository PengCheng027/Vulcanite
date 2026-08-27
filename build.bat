@echo off
setlocal enabledelayedexpansion

REM ============================================================
REM  Vulcanite - CMake build script
REM  Usage:
REM    build.bat               configure + build Debug (default)
REM    build.bat Release       configure + build Release
REM    build.bat reconfigure   force re-configure
REM    build.bat clean         remove build directory
REM ============================================================

set ROOT=%~dp0
REM 去掉末尾反斜杠,避免 "路径\" 中反斜杠转义引号
if "%ROOT:~-1%"=="\" set ROOT=%ROOT:~0,-1%
set BUILD_DIR=%ROOT%\build
set CONFIG=Debug
set RECONFIGURE=0
set CLEAN=0

for %%a in (%*) do (
    if /i "%%a"=="Release"      set CONFIG=Release
    if /i "%%a"=="Debug"        set CONFIG=Debug
    if /i "%%a"=="reconfigure"  set RECONFIGURE=1
    if /i "%%a"=="clean"        set CLEAN=1
)

if "%CLEAN%"=="1" (
    echo [Vulcanite] Removing build dir: %BUILD_DIR%
    if exist "%BUILD_DIR%" rmdir /s /q "%BUILD_DIR%"
    echo [Vulcanite] Clean done.
    exit /b 0
)

if not exist "%ROOT%\Vendor\spdlog\CMakeLists.txt" (
    echo [Vulcanite] Submodule spdlog missing, updating...
    git -C "%ROOT%" submodule update --init --recursive
)
if not exist "%ROOT%\Vendor\glfw\CMakeLists.txt" (
    echo [Vulcanite] Submodule glfw missing, updating...
    git -C "%ROOT%" submodule update --init --recursive
)

if not exist "%BUILD_DIR%\CMakeCache.txt" set RECONFIGURE=1
if "%RECONFIGURE%"=="1" (
    echo [Vulcanite] Configuring CMake for Visual Studio 17 2022, x64...
    cmake -S "%ROOT%" -B "%BUILD_DIR%" -G "Visual Studio 17 2022" -A x64
    if errorlevel 1 (
        echo [Vulcanite] Configure failed!
        exit /b 1
    )
)

echo [Vulcanite] Building %CONFIG%...
cmake --build "%BUILD_DIR%" --config %CONFIG%

if errorlevel 1 (
    echo [Vulcanite] Build failed!
    exit /b 1
)

echo.
echo [Vulcanite] Build OK: %CONFIG%
echo [Vulcanite] Executable: %ROOT%\bin\%CONFIG%\Vulcanite.exe
endlocal
