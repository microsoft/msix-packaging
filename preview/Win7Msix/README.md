# Win7Msix
Copyright (c) 2018 Microsoft Corp. All rights reserved.

# Description 
Win7Msix is a preview for the Windows 7 handler for MSIX. The codebase comprises of a Visual Studio solution file that builds Win7MSIXInstaller Application. 

The current preview includes a user interface that will initiate the extraction of the MSIX package. It will unpack the files in the VFS folder to the local computer, register the application in Add/Remove Programs, places a link to the executable in the Start Menu and generates an uninstallation script, which is an XML file that contains all the directories created, files unpacked and registry keys written. The application can then be uninstalled by removing it from Add\Remove Programs. 

The main folders of interest are the hidden WindowsApps and Uninstallers created in Program Files. WindowsApps contain the extracted packages that have been installed specifically with the directory name being packageFullName. Uninstallers contains all the uninstall scripts that will be used as a parameter for uninstalling the package.

(NOTE: Uninstalling the application from Add/Remove Programs in the Win7Msix preview requires the Win7MsixInstaller application to be installed using the default settings of the MSI project detailed below.)

# Prerequisites
In order to create the MSI project in Visual Studio, the MSI Installer Projects for Visual Studio must be installed - 
(https://marketplace.visualstudio.com/items?itemName=VisualStudioProductTeam.MicrosoftVisualStudio2017InstallerProjects)

# Build
Clone the msix-packaging repository to a local workspace and build it x64 (see https://github.com/Microsoft/msix-packaging for prerequisites) using the -mt flag, which is necessary to avoid vclibs dependency; this should output the msix.dll, which is consumed by Win7Msix project.

```
Makewin.cmd x64 -mt
```
Open the msix-packaging/preview/Win7Msix/Win7MSIXInstaller.sln file in Visual Studio 2017. Build the Win7MSIXInstaller project in release/x64 to create the Win7MsixInstaller.exe

## Using a MSI Setup Project
Once the Win7MSIXInstaller project has been built, the Win7MSIXInstallerSetup project can be built.
The Win7MSIXInstallerSetup Project creates a .msi package to deploy the msix.dll and Win7MsixInstaller.exe onto a Win 7 machine. The MSI Setup Project will register the specific file type association for the .msix and .appx extensions such that the installer is initiated directly from double-clicking a MSIX or APPX package.

## Using the Command Line
The executables can also be manually deployed to a Windows 7 machine without using the MSI setup project. Place the msix.dll and Win7MsixInstaller.exe in the same location. 
Sample packages can be found at msix-packaging/preview/Win7Msix/tests; these can be copied over to a Windows 7 machine and installed, although they may require adding the APPX_TEST_ROOT.cer (in the same tests folder) to the trusted store in order to install.

```
certutil -addstore root APPX_TEST_ROOT.cer
```

Installation - Using command prompt or PowerShell, navigate to the directory that contains the executables and run the following command to install notepadplus.msix.

```
Win7MSIXInstaller.exe -AddPackage C:\SomeDirectory\notepadplus.msix
```

Uninstallation - 

```
Win7MSIXInstaller.exe -RemovePackage notepadplus_0.0.0.1_x64__8wekyb3d8bbwe
```

## Build Status
[![Build status](https://microsoft.visualstudio.com/xPlatAppx/_apis/build/status/CIGitHub-for-Win7MsixInstaller)](https://github.com/Microsoft/msix-packaging/releases/tag/Win7MsixInstaller-preview)
If succeeded, the built MSI can be downloaded from the release tab.