# MSIX ADO Tasks

## Building

You need to have Node.js v10 installed (higher versions cause problems with the tests).  You also need
to have the Node modules typescript, tfx-cli and mocha installed globally.

After cloning the repo for the first time, install all the dependencies needed (e.g. node modules) with:
```
.\build.ps1 InstallDependencies
```

To compile the project:
```
.\build.ps1 Build
```
You can also use only `tsc` from the root, but the script will also update the localization files if needed.

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

To run the tests, from the repo root do:
```
mocha
```

## Updating dependencies

* MakeAppx.exe (under `\common\lib`), SignTool.exe (under `\MsixSigning\lib`) and related files were taken from the [MSIX Toolkit](https://github.com/microsoft/MSIX-Toolkit/tree/master/Redist.x86)
* msixmgr.exe (under `\MsixAppAttach\lib`) was taken from the [MSIX Core 1.1. release](https://github.com/microsoft/msix-packaging/releases)
* The MSBuildHelpers at `\MsixPackaging\MSBuildHelpers` were taken from [azure-pipelines-tasks](https://github.com/microsoft/azure-pipelines-tasks/tree/master/Tasks/Common/MSBuildHelpers). There were only a minor changes to type annotations to make it compile with our configuration.
* vswhere.exe under `\MsixPackaging\MSBuildHelpers` was taken from [vswhere release 1.0.62](https://github.com/Microsoft/vswhere/releases/download/1.0.62/vswhere.exe)

# Contributing

This project welcomes contributions and suggestions.  Most contributions require you to agree to a
Contributor License Agreement (CLA) declaring that you have the right to, and actually do, grant us
the rights to use your contribution. For details, visit https://cla.opensource.microsoft.com.

When you submit a pull request, a CLA bot will automatically determine whether you need to provide
a CLA and decorate the PR appropriately (e.g., status check, comment). Simply follow the instructions
provided by the bot. You will only need to do this once across all repos using our CLA.

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/).
For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or
contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.
