@echo off
setlocal EnableDelayedExpansion

rem Usage: setup-opendds.bat <install-dir>
rem Called from all three OpenDDS project PreBuildEvents.
rem
rem Prerequisites handled automatically:
rem  * Perl   — checked on PATH, then C:\Strawberry, then local portable;
rem             if none found, downloads portable Strawberry Perl (~170 MB).
rem  * cmake  — checked on PATH, then VS 2022 default install location.
rem  * git    — must be on PATH (Git for Windows).
rem
rem First-time build: clones OpenDDS, downloads ACE/TAO, compiles (~30-90 min).
rem Subsequent builds: exits immediately (opendds_idl.exe already present).

set "INSTALL_DIR=%~1"
if "%INSTALL_DIR%"=="" ( echo ERROR: Usage: %~nx0 ^<install-dir^> & exit /b 1 )

rem Normalize early so INSTALL_DIR_ABS is always set regardless of which Perl path is taken.
for %%F in ("%INSTALL_DIR%") do set "INSTALL_DIR_ABS=%%~fF"

if exist "%INSTALL_DIR%\bin\opendds_idl.exe" if exist "%INSTALL_DIR%\bin\tao_idl.exe" exit /b 0

echo.
echo ============================================================
echo  Automated OpenDDS setup
echo  Install target : %INSTALL_DIR%
echo  WARNING: First-time build takes 30-90 minutes.
echo ============================================================
echo.

rem ============================================================
rem  Locate cmake
rem ============================================================
set "CMAKE="
where cmake.exe >nul 2>&1
if not errorlevel 1 (
  set "CMAKE=cmake"
) else (
  for %%E in (Community Enterprise Professional BuildTools) do (
    set "_C=C:\Program Files\Microsoft Visual Studio\2022\%%E\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
    if exist "!_C!" ( set "CMAKE=!_C!" & goto :cmake_found )
  )
  echo ERROR: cmake.exe not found.
  echo        In the Visual Studio Installer, add "C++ CMake tools for Windows"
  echo        ^(Individual Components tab^) and retry.
  exit /b 1
)
:cmake_found

rem ============================================================
rem  Locate git
rem ============================================================
where git.exe >nul 2>&1
if errorlevel 1 (
  echo ERROR: git not found. Install "Git for Windows" from https://git-scm.com/
  exit /b 1
)

rem ============================================================
rem  Locate or auto-install Perl
rem  (ACE/TAO's MPC build system requires it regardless of cmake vs perl configure)
rem ============================================================
set "PERL_EXE="

where perl.exe >nul 2>&1
if not errorlevel 1 ( set "PERL_EXE=perl" & goto :perl_found )

if exist "C:\Strawberry\perl\bin\perl.exe" (
  set "PERL_EXE=C:\Strawberry\perl\bin\perl.exe"
  goto :perl_found
)

rem Normalize INSTALL_DIR path before computing sibling perl dir
for %%F in ("%INSTALL_DIR%") do set "INSTALL_DIR_ABS=%%~fF"
set "PERL_DIR=%INSTALL_DIR_ABS%\..\perl"
for %%F in ("%PERL_DIR%") do set "PERL_DIR=%%~fF"

if exist "%PERL_DIR%\perl\bin\perl.exe" (
  set "PERL_EXE=%PERL_DIR%\perl\bin\perl.exe"
  goto :perl_found
)

rem Auto-download portable Strawberry Perl (no installer, no admin required)
rem Use a process-unique temp path to avoid race conditions when multiple
rem OpenDDS projects trigger this script in parallel during a solution build.
echo Perl not found. Downloading portable Strawberry Perl (~250 MB^)...
set "PERL_URL=https://github.com/StrawberryPerl/Perl-Dist-Strawberry/releases/download/SP_54231_64bit/strawberry-perl-5.42.3.1-64bit-portable.zip"
set "PERL_ZIP=%TEMP%\strawberry-perl-portable-%RANDOM%%RANDOM%.zip"

curl -L -f --progress-bar -o "%PERL_ZIP%" "%PERL_URL%"
if errorlevel 1 (
  if exist "%PERL_ZIP%" del "%PERL_ZIP%" >nul 2>&1
  echo ERROR: Download failed. Install Perl manually from https://strawberryperl.com/
  exit /b 1
)

rem Another parallel process may have already extracted Perl by the time we get here.
if exist "%PERL_DIR%\perl\bin\perl.exe" (
  del "%PERL_ZIP%" >nul 2>&1
  set "PERL_EXE=%PERL_DIR%\perl\bin\perl.exe"
  echo Perl already extracted by parallel process.
  goto :perl_found
)

if not exist "%PERL_DIR%\" mkdir "%PERL_DIR%"
echo Extracting Strawberry Perl to %PERL_DIR%...
powershell -NoProfile -NonInteractive -Command "Expand-Archive -LiteralPath '%PERL_ZIP%' -DestinationPath '%PERL_DIR%' -Force"
if errorlevel 1 (
  del "%PERL_ZIP%" >nul 2>&1
  echo ERROR: Extraction failed.
  exit /b 1
)
del "%PERL_ZIP%" >nul 2>&1

if not exist "%PERL_DIR%\perl\bin\perl.exe" (
  echo ERROR: perl.exe not found after extraction. Unexpected archive layout.
  exit /b 1
)
set "PERL_EXE=%PERL_DIR%\perl\bin\perl.exe"
echo Perl ready at %PERL_EXE%

:perl_found
rem Add perl to PATH so cmake and the MPC-driven ACE/TAO build can find it.
for %%F in ("%PERL_EXE%") do set "PERL_BIN=%%~dpF"
set "PATH=%PERL_BIN%;%PATH%"

rem ============================================================
rem  Clone OpenDDS
rem ============================================================
set "SRC_DIR=%INSTALL_DIR_ABS%-src"
if "!SRC_DIR!"=="" set "SRC_DIR=%INSTALL_DIR%-src"

if not exist "%SRC_DIR%\" (
  echo Cloning OpenDDS. ACE/TAO will be downloaded by cmake during configure.
  git clone --depth 1 https://github.com/OpenDDS/OpenDDS.git "%SRC_DIR%"
  if errorlevel 1 ( echo ERROR: git clone failed. & exit /b 1 )
)

if not exist "%SRC_DIR%\CMakeLists.txt" (
  echo ERROR: CMakeLists.txt not found in %SRC_DIR%. OpenDDS 3.20+ is required.
  echo        Try deleting %SRC_DIR% and rebuilding.
  exit /b 1
)

rem ============================================================
rem  CMake configure
rem  Use ALL_BUILD.vcxproj (not CMakeCache.txt) as the success indicator:
rem  CMakeCache.txt can exist after a failed configure.
rem ============================================================
set "BUILD_DIR=%SRC_DIR%\build"
if not exist "%BUILD_DIR%\ALL_BUILD.vcxproj" (
  echo Configuring OpenDDS with CMake...
  "%CMAKE%" -S "%SRC_DIR%" -B "%BUILD_DIR%" -G "Visual Studio 17 2022" -A x64 ^
    -DCMAKE_INSTALL_PREFIX="%INSTALL_DIR%" ^
    -DPERL_EXECUTABLE="%PERL_EXE%" ^
    -DBUILD_SHARED_LIBS=ON ^
    -DOPENDDS_BUILD_TESTS=OFF ^
    -DOPENDDS_BUILD_EXAMPLES=OFF
  if errorlevel 1 ( echo ERROR: cmake configure failed. & exit /b 1 )
)

rem ============================================================
rem  Build + Install (Release then Debug)
rem ============================================================
echo Building OpenDDS Release ^(may take 30-90 minutes^)...
"%CMAKE%" --build "%BUILD_DIR%" --config Release --parallel
if errorlevel 1 ( echo ERROR: Release build failed. & exit /b 1 )
"%CMAKE%" --install "%BUILD_DIR%" --config Release
if errorlevel 1 ( echo ERROR: Release install failed. & exit /b 1 )

echo Building OpenDDS Debug...
"%CMAKE%" --build "%BUILD_DIR%" --config Debug --parallel
if errorlevel 1 ( echo ERROR: Debug build failed. & exit /b 1 )
"%CMAKE%" --install "%BUILD_DIR%" --config Debug
if errorlevel 1 ( echo ERROR: Debug install failed. & exit /b 1 )

rem ============================================================
rem  Copy tao_idl.exe from ACE/TAO build tree to install bin
rem  (OpenDDS cmake --install does not install tao_idl)
rem ============================================================
if not exist "%INSTALL_DIR%\bin\tao_idl.exe" (
  if not exist "%BUILD_DIR%\ace_tao\bin\tao_idl.exe" (
    echo ERROR: tao_idl.exe not found in %BUILD_DIR%\ace_tao\bin\
    echo        This is unexpected after a successful cmake build.
    exit /b 1
  )
  echo Copying tao_idl.exe to install directory...
  copy /Y "%BUILD_DIR%\ace_tao\bin\tao_idl.exe" "%INSTALL_DIR%\bin\tao_idl.exe" >nul
)

echo.
echo OpenDDS setup complete: %INSTALL_DIR%
exit /b 0
