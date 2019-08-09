# MsixCore
Copyright (c) 2018 Microsoft Corp. All rights reserved.

# Description 
MsixCore is a preview for the Windows 7 handler for MSIX. The codebase comprises of a Visual Studio solution file that builds msixmgr Application. 

The current preview includes a user interface that will initiate the install of the MSIX package. It will unpack the files in the VFS folder to the local computer, register the application in Add/Remove Programs, places a link to the executable in the Start Menu. The application can then be uninstalled by removing it from Add\Remove Programs. 

The main folder of interest is the MsixCoreApps directory created in Program Files. MsixCoreApps contain the extracted packages that have been installed specifically with the directory name being packageFullName.

(NOTE: Uninstalling the application from Add/Remove Programs in the MsixCore preview requires the msixmgr application to be installed using the default settings of the MSI project detailed below.)

# Prerequisites
In order to create the MSI project in Visual Studio, the MSI Installer Projects for Visual Studio must be installed - 
(https://marketplace.visualstudio.com/items?itemName=VisualStudioProductTeam.MicrosoftVisualStudio2017InstallerProjects)

# Build
Clone the msix-packaging repository to a local workspace and build it x64 (see https://github.com/Microsoft/msix-packaging for prerequisites) using the -mt flag, which is necessary to avoid vclibs dependency; this should output the msix.dll, which is consumed by MsixCore project.

```
Makewin.cmd x64 -mt
```
Open the msix-packaging/preview/MsixCore/msixmgr.sln file in Visual Studio 2017. Build the msixmgr project in release/x64 to create the msixmgr.exe

## Using a MSI Setup Project
Once the msixmgr project has been built, the msixmgrSetup project can be built.
The msixmgrSetup Project creates a .msi package to deploy the msix.dll and msixmgr.exe onto a Windows 7 SP1 or higher machine. The MSI Setup Project will register the specific file type association for the .msix and .appx extensions such that the installer is initiated directly from double-clicking a MSIX or APPX package.

## Using the Command Line
The executables can also be manually deployed to a Windows 7 SP1 or higher machine without using the MSI setup project. Place the msix.dll and msixmgr.exe in the same location. 
Sample packages can be found at msix-packaging/preview/MsixCore/tests; these can be copied over to a Windows 7 SP1 or higher machine and installed, although they may require adding the APPX_TEST_ROOT.cer (in the same tests folder) to the trusted store in order to install. Some of these packages are large and are stored using git lfs (Large File Storage).

```
certutil -addstore root APPX_TEST_ROOT.cer
```

Installation - Using command prompt or PowerShell, navigate to the directory that contains the executables and run the following command to install notepadplus.msix. The -quietUX parameter can also be added at the end of the command so that users don't see the installer UI.
```
msixmgr.exe -AddPackage C:\SomeDirectory\notepadplus.msix
```
Querying for Installed MSIX Packages - 
```
msixmgr.exe -FindAllPackages
```
Querying for a specific MSIX Package -
```
msixmgr.exe -FindPackage notepadplus_0.0.0.1_x64__8wekyb3d8bbwe
```
Uninstallation - The -quietUX parameter can also be used here.
```
msixmgr.exe -RemovePackage notepadplus_0.0.0.1_x64__8wekyb3d8bbwe
```
Unpacking -
```
msixmgr.exe -Unpack notepadplus.msix
```

## Build Status
[![Build status](https://microsoft.visualstudio.com/xPlatAppx/_apis/build/status/CIGitHub-for-MsixCoreInstaller)](https://github.com/Microsoft/msix-packaging/releases/tag/MsixCoreInstaller-preview)
If succeeded, the built MSI can be downloaded from the release tab.
