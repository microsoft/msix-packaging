This library contains the source for the Utils like AppxPackaging, AppxPackagingInterop,
CommandLine, IO, Logger, ProcessRunner, and StandardInput. This library will be
published as a Nuget package and consumed by sources in other projects.

## Prerequisites

* MSBuild
* .NET Standard 2.0
* .NET 5.0 (for test project)
This should be available in a standard installation of Visual Studio.

## Build

Open and build `msix-packaging/tools/utils/Utils.sln` in Visual Studio; or:
```
msbuild.exe msix-packaging/tools/utils/Utils.sln
```

## Testing

MSTest is used as the testing framework and can be run through Visual Studio's Test Explorer.

To run the tests from the command line, go to `msix-packaging/tools/utils/` and call `vstest.console.exe` on `UtilsTests.dll` produced by the build. E.g.
```
cd msix-packaging/tools/utils/
vstest.console.exe UtilsTests\bin\Debug\net6.0\UtilsTests.dll
```

MakeAppx.exe and related binaries under `msix-packaging/tools/utils/TestData/MakeAppx` are taken from the redistributable binaries available in the [MSIX Toolkit](https://github.com/microsoft/msix-toolkit).

## Data/Telemetry

This library includes instrumentation to collect usage and diagnostic (error) data and send it to Microsoft to help improve the products using it.

If you build the library yourself the instrumentation will not be enabled and no data will be sent to Microsoft.

The library respects machine wide privacy settings and users can opt-out on their device, as documented in the Microsoft Windows privacy statement [here](https://support.microsoft.com/en-us/help/4468236/diagnostics-feedback-and-privacy-in-windows-10-microsoft-privacy).

In short to opt-out, go to `Start`, then select `Settings` > `Privacy` > `Diagnostics & feedback`, and select `Basic`.

See the [privacy statement](PRIVACY.md) for more details.