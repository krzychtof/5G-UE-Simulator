@echo off
echo ========================================
echo   UE Simulator 5G/6G - 3GPP Compliant
echo ========================================
echo.

if not exist build mkdir build
cd build

echo [1/3] CMake Configuration...
cmake .. -G "MinGW Makefiles"
if %errorlevel% neq 0 (
    echo.
    echo ERROR: CMake configuration failed!
    echo Check if CMake and compiler are installed.
    pause
    exit /b 1
)

echo.
echo [2/3] Building...
cmake --build .
if %errorlevel% neq 0 (
    echo.
    echo ERROR: Build failed!
    pause
    exit /b 1
)

echo.
echo [3/3] Build completed successfully!
echo.
echo ========================================
echo   Running simulator...
echo ========================================
echo.

ue_simulator.exe

cd ..
pause
