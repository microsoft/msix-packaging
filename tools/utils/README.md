This library contains the source for the Utils like AppxPackaging, AppxPackagingInterop,
CommandLine, IO, Logger, ProcessRunner, and StandardInput. This library will be
published as a Nuget package and consumed by sources in other projects.

## Prerequisites

* MSBuild
* .NET Framework 4.6
* .NET Core 2.0
This should be available in a standard installation of Visual Studio.

## Build

Open and build `msix-packaging/tools/utils/Utils.sln` in Visual Studio; or:
```
msbuild.exe msix-packaging/tools/utils/Utils.sln
```

## Testing

TAEF is used as the testing framework. This is available through the Windows Driver Kit.

To run the tests, go to `msix-packaging/tools/utils/` and call `te.exe` on `UtilsTests.dll` produced by the build. E.g.
```
cd msix-packaging/tools/utils/
te.exe UtilsTests/bin/Debug/UtilsTests.dll
```
