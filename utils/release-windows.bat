@echo off
setlocal

echo.
echo Deleting previous build directory...
if exist build (
    rmdir /s /q build
)

echo.
echo Configuring project with CMake...
cmake -B build . -GNinja -DCMAKE_BUILD_TYPE=Release
if %errorlevel% neq 0 (
    echo CMake configuration failed.
    goto :error
)

echo.
echo Building plugins...
cmake --build build --config Release
if %errorlevel% neq 0 (
    echo Plugin build failed.
    goto :error
)

echo.
echo Building installer...
makensis utils\setup.nsi
if %errorlevel% neq 0 (
    echo Installer build failed.
    goto :error
)

echo.
echo Creating artifacts zip...
python utils\artifacts.py
if %errorlevel% neq 0 (
    echo Artifact creation failed.
    goto :error
)

echo.
echo ========================================
echo      Release build successful!
echo ========================================
goto :eof

:error
echo.
echo ========================================
echo      Release build FAILED.
echo ========================================
pause
exit /b 1

:eof
endlocal
