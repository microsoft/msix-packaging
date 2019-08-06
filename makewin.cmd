@echo off
SETLOCAL

if "%~1" == "" goto USAGE
if /I "%~1" == "--help" goto USAGE
if /I "%~1" == "-h" goto USAGE
if /I "%~1" == "/?" goto USAGE

:: Set up MSVC environment
:: Kudos to https://gist.github.com/AndrewPardoe/689a3b969670787d5dba538bb0a48a1e

pushd "%~dp0"
pushd %ProgramFiles(x86)%\"Microsoft Visual Studio"\Installer
for /f "usebackq tokens=*" %%i in (`vswhere -latest -prerelease -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
  set VCINSTALLDIR=%%i\VC
)
popd

if exist %VCINSTALLDIR%\Auxiliary\Build\vcvarsall.bat (
    if /I "%~1" == "x86" (
        call "%VCINSTALLDIR%\Auxiliary\Build\vcvarsall.bat" %1
    ) else (
        if /I "%~1" == "x64" (
            call "%VCINSTALLDIR%\Auxiliary\Build\vcvarsall.bat" %1
        ) else goto USAGE
    )
)
popd

set build="MinSizeRel"
set validationParser="-DUSE_VALIDATION_PARSER=off"
set zlib="-DUSE_SHARED_ZLIB=off"
set parser="-DXML_PARSER=msxml6"
set crypto="-DCRYPTO_LIB=crypt32"
set msvc="-DUSE_STATIC_MSVC=off"
set bundle="-DSKIP_BUNDLES=off"
set pack="-DMSIX_PACK=off"

:parseArgs
if /I "%~2" == "--debug" (
    set build="Debug"
)
if /I "%~2" == "-d" (
    set build="Debug"
)
if /I "%~2" == "--symbols" (
    set build="RelWithDebInfo"
)
if /I "%~2" == "-sym" (
    set build="RelWithDebInfo"
)
if /I "%~2" == "--parser-xerces" (
    set parser="-DXML_PARSER=xerces"
)
if /I "%~2" == "-px" (
    set parser="-DXML_PARSER=xerces"
)
if /I "%~2" == "--validation-parser" (
    set validationParser="-DUSE_VALIDATION_PARSER=on"
)
if /I "%~2" == "-vp" (
    set validationParser="-DUSE_VALIDATION_PARSER=on"
)
if /I "%~2" == "--shared-zlib" (
    set zlib="-DUSE_SHARED_ZLIB=on"
)
if /I "%~2" == "-sz" (
    set zlib="-DUSE_SHARED_ZLIB=on"
)
if /I "%~2" == "--crypto-openssl" (
    set crypto="-DCRYPTO_LIB=openssl"
)
if /I "%~2" == "-co" (
    set crypto="-DCRYPTO_LIB=openssl"
)
if /I "%~2" == "-mt" (
    set msvc="-DUSE_STATIC_MSVC=on"
)
if /I "%~2" == "--skip-bundles" (
    set bundle="-DSKIP_BUNDLES=on"
)
if /I "%~2" == "-sb" (
    set bundle="-DSKIP_BUNDLES=on"
)
if /I "%~2" == "--pack" (
    set pack="-DMSIX_PACK=on"
    set validationParser="-DUSE_VALIDATION_PARSER=on"
)
shift /2
if not "%~2"=="" goto parseArgs

if not exist .vs md .vs
cd .vs
if exist CMakeFiles rd /s /q CMakeFiles
if exist CMakeCache.txt del CMakeCache.txt

echo cmake -DWIN32=on -DCMAKE_BUILD_TYPE=%build% %validationParser% %zlib% %parser% %crypto% %msvc% %bundle% %pack% -G"NMake Makefiles" ..
cmake -DWIN32=on -DCMAKE_BUILD_TYPE=%build% %validationParser% %zlib% %parser% %crypto% %msvc% %bundle% %pack% -G"NMake Makefiles" ..
nmake /NOLOGO

goto Exit
:USAGE
echo Usage
echo:
echo    makewin.cmd ^<x86^|x64^> [options]
echo:
echo Helper to build the MSIX SDK for Windows. Assumes the user has a version of Visual Studio installed.
echo:
echo Options
echo    --debug, -d              = Build chk binary.
echo    --symbols, -sym          = Produce symbols for release binaries.
echo    --parser-xerces, -px     = Use Xerces-C parser. Default MSXML6.
echo    --validation-parser, -vp = Enable XML schema validation.
echo    --shared-zlib, -sz       = Don't statically link zlib.
echo    --crypto-openssl, -co    = Use OpenSSL crypto [currently for testing]. Default Crypt32.
echo    -mt                      = Use compiler flag /MT to use static version of the run-time library.
echo    --skip-bundles, -sb      = Turn off bundle support.
echo    --pack                   = Include packaging features. Sets validation parser on.
echo    --help, -h, /?           = Print this usage information and exit.
:Exit
EXIT /B 0
