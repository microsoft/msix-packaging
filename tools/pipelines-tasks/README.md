# MSIX ADO Tasks

This includes the source for Azure Pipelines tasks for:
* Building and packaging MSIX apps.
* Signing MSIX packages.
* Creating VHDX disks for use with MSIX app attach.
* Creating App Installer files.
* App Attach VHDX artifact to AVD.

## Building

You need to have Node.js v10 installed (higher versions cause problems with the tests).
When building for the first time, install the development tools (e.g. TypeScript) and dependencies (e.g. node modules) needed with:
```
.\build.ps1 InstallDevelopmentTools
.\build.ps1 InstallDependencies
```

To compile the project:
```
.\build.ps1 Build
```
You can also use `tsc`, but the script will also update the localization files if needed.

To build the common helpers and update the dependencies in other projects:
```
.\build.ps1 BuildCommonHelpers
```

To publish the extension to a private publisher:
```
.\PublishForTesting.ps1 -publisherName <your publisher name> -extensionId <extension name>
```
You will need to have a [Personal Access Token](https://docs.microsoft.com/en-us/azure/devops/organizations/accounts/use-personal-access-tokens-to-authenticate).
This script is intended to publish to a private publisher to test changes, that is why
you need to set the publisher and use a different name for the extension.
It will auto-increase the patch version of the tasks and the extension.

To run the tests, from the project root (`/tools/pipelines-tasks/`) do:
```
mocha
```

## Updating dependencies

* MakeAppx.exe (under `.\common\lib`), SignTool.exe (under `.\MsixSigning\lib`) and related files were taken from the [MSIX Toolkit](https://github.com/microsoft/MSIX-Toolkit/tree/master/Redist.x86)
* msixmgr.exe (under `.\MsixAppAttach\lib`) was taken from the [MSIX Core 1.1. release](https://github.com/microsoft/msix-packaging/releases)
* The MSBuildHelpers at `.\MsixPackaging\MSBuildHelpers` were taken from [azure-pipelines-tasks](https://github.com/microsoft/azure-pipelines-tasks/tree/master/Tasks/Common/MSBuildHelpers). There were only a minor changes to type annotations to make it compile with our configuration.
* vswhere.exe under `.\MsixPackaging\MSBuildHelpers` was taken from [vswhere release 1.0.62](https://github.com/Microsoft/vswhere/releases/download/1.0.62/vswhere.exe)
* AppAttachFramework(AppAttachKernel.nupkg) (under `.\common\lib`) was taken from internal project that helps to build, package and app attach MSIX packages.