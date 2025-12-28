@echo off
setlocal

echo ==================================================
echo Cleaning old build folder...
echo ==================================================
if exist build (
    rmdir /S /Q build
)

mkdir build
cd build

echo.
echo ==================================================
echo Configuring CMake (Visual Studio 17 2022, x64)...
echo ==================================================
cmake -S .. -B . -G "Visual Studio 17 2022" -A x64 ^
    -DmetadataSdkDir="C:/Users/a/Documents/metadata_sdk"

if errorlevel 1 (
    echo ERROR: CMake configuration failed.
    pause
    exit /b 1
)

echo.
echo ==================================================
echo Building (Release config)...
echo ==================================================
cmake --build . --config Release

if errorlevel 1 (
    echo ERROR: Build failed.
    pause
    exit /b 1
)

echo.
echo ==================================================
echo Build completed successfully!
echo ==================================================
pause