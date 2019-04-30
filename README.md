# MSIX SDK 
   Copyright (c) 2019 Microsoft Corp.  All rights reserved.

## Description
   The MSIX SDK project is an effort to enable developers on a variety of platforms to unpack 
   packages for the purposes of distribution from either the Microsoft Store, or their own content distribution networks.  
    
   The MSIX Packaging APIs that a client app would use to interact with .msix/.appx packages are a subset of those
   documented [here](https://msdn.microsoft.com/en-us/library/windows/desktop/hh446766(v=vs.85).aspx).

## Overview
The MSIX SDK project includes cross platform API support for unpacking of .msix/.appx packages

|                                      |                                 |
|--------------------------------------|---------------------------------|
| **msix**      | A shared library (DLL on Win32, dylib on macOS, SO on Linux and Android) that exports a subset of the functionality contained within appxpackaging.dll on Windows. See [here](https://msdn.microsoft.com/en-us/library/windows/desktop/hh446766(v=vs.85).aspx) for additional details.<br />On all platforms instead of CoCreating IAppxFactory, a C-style export: CoCreateAppxFactory is provided. Similarly, the CoCreateAppxBundleFactory export is equivalent as CoCreating IAppxBundleFactory.<br /><br /> The 'UnpackPackage' and 'UnpackBundle' exports that provide a simplified unpackage implementation. See the [samples directory](sample) for usage of the SDK.|
| **msixtool**  | A command line wrapper over the UnpackPackage and UnpackBundle implementations.  This tool exists primarily as a means of validating the implementation of the MSIX SDK internal routines and is compiled for Win32, macOS, and Linux platforms.  msixtool used be named makemsix. For compatibility reasons, a copy of msixtool is generated after is built and named makemsix. This copy will be removed in a future release.|

Guidance on how to package your app contents and construct your app manifest such that it can take advantage of the cross platform support of this SDK is [here](tdf-guidance.md).

## Release Notes
Release notes on the latest features and performance improvements made to the SDK are listed [here](https://docs.microsoft.com/en-us/windows/msix/msix-sdk/release-notes/sdk-release-notes-1.6)

## Setup Instructions
1. Clone the repository:
        ```git clone [URL]```

## Issues
If you are using Visual Studio 2017 and you run into errors about not being able to find the v140 toolset:
1. Install the Microsoft Build Tools (https://chocolatey.org/packages/microsoft-build-tools)
2. Start -> visual studio installer -> Visual Studio Build Tools 2017 -> Modify the 2014 toolset -> individual components 
3. Make sure that VC++ 2015.3 v140 toolset for desktop is selected and then unselect VC++ 2017 141 toolset
4. Close, then re-open the solution.

## Dependencies
Depending on the platform for which the MSIX shared library (MSIX.DLL | libmsix.dylib | libmsix.so) is compiled, one or 
more of the following dependencies may be statically linked into the binary:

* [ZLib Tag v1.2.11 Commit cacf7f1d4e3d44d871b605da3b647f07d718623f](https://github.com/madler/zlib/releases/tag/v1.2.11)
* [Xerces-C Tag Xerces-C_3_2_0 Commit dffc3028df8ea44985c92f2df28115860e39e344](https://github.com/apache/xerces-c/releases/tag/Xerces-C_3_2_0)
* [OpenSSL Tag OpenSSL_1_0_2q Commit 5707219a6aae8052cb98aa361d115be01b8fd894](https://github.com/openssl/openssl/releases/tag/OpenSSL_1_0_2q)
* [Android NDK](https://developer.android.com/ndk)

For convinience, Zlib, Xerces-C and OpenSSL are git-subtrees that are mapped in under the lib folder of this project.  Edits on top
of these subtrees for build related optimizations are tracked within this repository. OpenSSL is only used on non-Windows platforms

The Android NDK is only required for targeting the Android platform.

## Prerequisites
Make sure that you have CMAKE installed on your machine 

   * https://cmake.org/download/

One or more of the following prerequisites may also be required on your machine:

##### Ninja-build:
https://github.com/ninja-build/ninja/releases

##### Android NDK:
https://developer.android.com/ndk/downloads/index.html

##### Clang/LLVM:
http://releases.llvm.org/download.html
    
##### VS 2017 clients:
Open Visual Studio 2017
File->Open Folder->navigate to project root and select "CMakeLists.txt"

See [cmake-support-vs](https://blogs.msdn.microsoft.com/vcblog/2016/10/05/cmake-support-in-visual-studio/) for details regarding how to configure your environment.

##### Xcode clients: 

open terminal, from project root:
mkdir build && cd build && cmake -DMACOS=on -G"Xcode" ..
open xcode
File->Open->navigate to project root/build and select "Project.xcodeproj"

See [cmake-Xcode-integration](https://www.johnlamp.net/cmake-tutorial-2-ide-integration.html#section-Xcode) for additional details

## Build
### On Windows using Visual Studio nmake:
```
   makewin.cmd <x86|x64> -mt

   This will start MSVC environment calling vcvarsall.bat <arch>, clean the output directory, call cmake and nmake. The latest Visual Studio version is obtained by calling vswhere.exe 
```

### On Mac using make:
```
   ./makemac
   ./makeios
```

### On Linux using make:
```
   ./makelinux
   ./makeaosp
``` 

### How to compile for Android on Windows:

- Unpack the latest Android NDK to c:\android-ndk
- Unpack Ninja-build to c:\ninja
- Add c:\ninja to the path environment variable
- Create a folder under the root of the enlistment called "android", cd into that folder, then run the following command to create ninja build files:
```
    cmake -DCMAKE_ANDROID_NDK=c:/android-ndk ^
        -DCMAKE_ANDROID_NDK_TOOLCHAIN_VERSION=clang ^
        -DCMAKE_SYSTEM_NAME=Android ^
        -DCMAKE_SYSTEM_VERSION=19 ^
        -DCMAKE_ANDROID_ARCH_ABI=x86 ^
        -DCMAKE_ANDROID_STL_TYPE=c++_shared ^
        -DCMAKE_BUILD_TYPE=Release ^
        -DAOSP=on ^
        -G"Ninja" ..
```
To compile, run the following command from the android folder:
```
    ninja
```

## Build Status
The following native platforms are in development now:

### Windows
||master|
|---|---|
**Debug x32**|[![Build Status](https://dev.azure.com/ms/msix-packaging/_apis/build/status/msix-packaging%20Windows%20CI?branchName=master&configuration=debug_32)](https://dev.azure.com/ms/msix-packaging/_build/latest?definitionId=64&branchName=master)|
**Debug x64**|[![Build Status](https://dev.azure.com/ms/msix-packaging/_apis/build/status/msix-packaging%20Windows%20CI?branchName=master&configuration=debug_64)](https://dev.azure.com/ms/msix-packaging/_build/latest?definitionId=64&branchName=master)|
**Release x32**|[![Build Status](https://dev.azure.com/ms/msix-packaging/_apis/build/status/msix-packaging%20Windows%20CI?branchName=master&configuration=release_32)](https://dev.azure.com/ms/msix-packaging/_build/latest?definitionId=64&branchName=master)|
**Release x64**|[![Build Status](https://dev.azure.com/ms/msix-packaging/_apis/build/status/msix-packaging%20Windows%20CI?branchName=master&configuration=release_64)](https://dev.azure.com/ms/msix-packaging/_build/latest?definitionId=64&branchName=master)|
**Release x32 Validation Parser**|[![Build Status](https://dev.azure.com/ms/msix-packaging/_apis/build/status/msix-packaging%20Windows%20CI?branchName=master&configuration=release_32_validation_parser)](https://dev.azure.com/ms/msix-packaging/_build/latest?definitionId=64&branchName=master)|
**Release x64 Validation Parser**|[![Build Status](https://dev.azure.com/ms/msix-packaging/_apis/build/status/msix-packaging%20Windows%20CI?branchName=master&configuration=release_64_validation_parser)](https://dev.azure.com/ms/msix-packaging/_build/latest?definitionId=64&branchName=master)|
**Release x32 Xerces**|[![Build Status](https://dev.azure.com/ms/msix-packaging/_apis/build/status/msix-packaging%20Windows%20CI?branchName=master&configuration=release_32_xerces)](https://dev.azure.com/ms/msix-packaging/_build/latest?definitionId=64&branchName=master)|
**Release x64 Xerces**|[![Build Status](https://dev.azure.com/ms/msix-packaging/_apis/build/status/msix-packaging%20Windows%20CI?branchName=master&configuration=release_64_xerces)](https://dev.azure.com/ms/msix-packaging/_build/latest?definitionId=64&branchName=master)|

Built in the Azure Pipelines Hosted VS2017 pool. See specifications [here](https://github.com/Microsoft/azure-pipelines-image-generation/blob/master/images/win/Vs2017-Server2016-Readme.md)

### macOS
||master|
|---|---|
**Debug**|[![Build Status](https://dev.azure.com/ms/msix-packaging/_apis/build/status/msix-packaging%20macOS%20CI?branchName=master&configuration=debug)](https://dev.azure.com/ms/msix-packaging/_build/latest?definitionId=69&branchName=master)|
**Release**|[![Build Status](https://dev.azure.com/ms/msix-packaging/_apis/build/status/msix-packaging%20macOS%20CI?branchName=master&configuration=release)](https://dev.azure.com/ms/msix-packaging/_build/latest?definitionId=69&branchName=master)|

Built in the Azure Pipelines macOS pool. See specification [here](https://github.com/Microsoft/azure-pipelines-image-generation/blob/master/images/macos/macos-Readme.md)

### iOS
||master|
|---|---|
**Debug emulator**|[![Build Status](https://dev.azure.com/ms/msix-packaging/_apis/build/status/msix-packaging%20iOS%20CI?branchName=master&configuration=debug_x86)](https://dev.azure.com/ms/msix-packaging/_build/latest?definitionId=74&branchName=master)|
**Release emulator**|[![Build Status](https://dev.azure.com/ms/msix-packaging/_apis/build/status/msix-packaging%20iOS%20CI?branchName=master&configuration=release_x86)](https://dev.azure.com/ms/msix-packaging/_build/latest?definitionId=74&branchName=master)|
**Release arm64**|[![Build Status](https://dev.azure.com/ms/msix-packaging/_apis/build/status/msix-packaging%20iOS%20CI?branchName=master&configuration=release_arm64)](https://dev.azure.com/ms/msix-packaging/_build/latest?definitionId=74&branchName=master)

Built in the Azure Pipelines macOS pool. See specification [here](https://github.com/Microsoft/azure-pipelines-image-generation/blob/master/images/macos/macos-Readme.md)

### Android
||master|
|---|---|
**Debug emulator**|[![Build Status](https://dev.azure.com/ms/msix-packaging/_apis/build/status/msix-packaging%20aosp%20CI?branchName=master&configuration=debug_emulator)](https://dev.azure.com/ms/msix-packaging/_build/latest?definitionId=76&branchName=master)|
**Release emulator**|[![Build Status](https://dev.azure.com/ms/msix-packaging/_apis/build/status/msix-packaging%20aosp%20CI?branchName=master&configuration=release_emulator)](https://dev.azure.com/ms/msix-packaging/_build/latest?definitionId=76&branchName=master)|
**Release arm**|[![Build Status](https://dev.azure.com/ms/msix-packaging/_apis/build/status/msix-packaging%20aosp%20CI?branchName=master&configuration=release_arm)](https://dev.azure.com/ms/msix-packaging/_build/latest?definitionId=76&branchName=master)|

Built in the Azure Pipelines macOS pool. See specification [here](https://github.com/Microsoft/azure-pipelines-image-generation/blob/master/images/macos/macos-Readme.md)

### Linux
||master|
|---|---|
**Debug**|[![Build Status](https://dev.azure.com/ms/msix-packaging/_apis/build/status/msix-packaging%20Linux%20CI?branchName=master&configuration=debug)](https://dev.azure.com/ms/msix-packaging/_build/latest?definitionId=72&branchName=master)|
**Release**|[![Build Status](https://dev.azure.com/ms/msix-packaging/_apis/build/status/msix-packaging%20Linux%20CI?branchName=master&configuration=release)](https://dev.azure.com/ms/msix-packaging/_build/latest?definitionId=72&branchName=master)|

Built in the Azure Pipelines Hosted Ubuntu 1604. See specification [here](https://github.com/Microsoft/azure-pipelines-image-generation/blob/master/images/linux/Ubuntu1604-README.md)

## Windows 7 support
The MSIX SDK is fully supported and tested on Windows 7. However, an Application Manifest **_MUST_**  be included to any executable that is expected to run on Windows 7 and uses msix.dll. Specifically, the Application Manifest **_MUST_**  include the supportedOS flags for Windows 7. The manifest is not included on msix.dll because the compat manifest doesn't matter on DLLs.
See the [manifest](manifest.cmakein) that is used for msixtool and samples of this project as example. The Windows 7 machine might also require the [Microsoft Visual C++ Redistributable](https://www.visualstudio.com/downloads/) binaries installed to run properly.

## Android support
The MSIX SDK minimum supported for Android is API Level 19.

We also produce msix-jni.jar which acts as a helper to get the languages from the Android device. Because of it, we expect either a -DANDROID_SDK and -DANDROID_SDK_VERSION on the cmake command and, if not present, we default to $ANDROID_HOME and 24 respectively.
The default level for the SDK level is 24 because we use the [Configuration class](https://developer.android.com/reference/android/content/res/Configuration) and, depending on the version of the device, we either use the locale attribute (deprecated as of API level 24) or getLocales.
We recommend using the [makeaosp](makeaosp) script to build for Android on non-Windows devices.

## Testing
Unit tests should be run on builds that have the "Release" or "RelWithDebug" CMAKE switch. 

First build the project, then:

  On Windows:
  From within powershell, navigate to test\Win32, and run ".\Win32.ps1"

  On Mac & Linux:
  From within bash, navigate to test/MacOS-Linux, and run "./MacOS-Linux-Etc.sh [Apple|Linux]"

Testing on mobile platforms:

  On iOS :
  First build the project for iOS, then launch xCode and load test/mobile/iOSBVT.xcworkspace, compile the test app,
  and then launch the iPhone simulator. You can also run "./testios.sh" from test/MacOS-Linux. 

  On Android:
  From within bash, navigate to test/MacOS-Linux, and run "./testaosponmac.sh". The test assumes there's an Android emulator named Nexus_5X_API_19_x86 and the build output is on a .vs directory at the root of the project.

## Releasing
If you are the current maintainer of this project:

  1. Pull latest payload to release in master
  2. Confirm that all platforms/architectures/flavors build and all BVTs pass
  3. From a windows cmd prompt: release_master.cmd
  4. Confirm that new branch called "release_v1.xxx" where "xxx" is the next incremental version is created
  
## Contributing
This project welcomes contributions and suggestions. Most contributions require you to
agree to a Contributor License Agreement (CLA) declaring that you have the right to,
and actually do, grant us the rights to use your contribution. For details, visit
https://cla.microsoft.com.

When you submit a pull request, a CLA-bot will automatically determine whether you need
to provide a CLA and decorate the PR appropriately (e.g., label, comment). Simply follow the
instructions provided by the bot. You will only need to do this once across all repositories 
using our CLA.

If you have any questions or comments, you can send them [our team](mailto:MSIXPackagingOSSCustomerQs@service.microsoft.com) directly! 

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/).
For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/)
or contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional 
questions or comments.

## Report a Computer Security Vulnerability
If you are a security researcher and believe you have found a security vulnerability that meets
the [definition of a security vulnerability](https://technet.microsoft.com/library/cc751383.aspx) that is not resolved by the [10 Immutable Laws of Security](https://technet.microsoft.com/library/cc722487.aspx),
please send e-mail to us at secure@microsoft.com. To help us to better understand the nature and
scope of the possible issue, please include as much of the below information as possible.

  * Type of issue (buffer overflow, SQL injection, cross-site scripting, etc.)
  * Product and version that contains the bug, or URL if for an online service
  * Service packs, security updates, or other updates for the product you have installed
  * Any special configuration required to reproduce the issue
  * Step-by-step instructions to reproduce the issue on a fresh install
  * Proof-of-concept or exploit code
  * Impact of the issue, including how an attacker could exploit the issue

Microsoft follows [Coordinated Vulnerability Disclosure](https://technet.microsoft.com/security/dn467923.aspx) (CVD) and, to protect the ecosystem, we 
request that those reporting to us do the same.  To encrypt your message to our PGP key, please
download it from the [Microsoft Security Response Center PGP Key](https://aka.ms/msrcpgp). You should receive a response
within 24 hours. If for some reason you do not, please follow up with us to ensure we received
your original message. For further information, please visit the Microsoft Security Response 
Policy and Practices page and read the [Acknowledgment Policy for Microsoft Security Bulletins](https://www.microsoft.com/technet/security/bulletin/policy.mspx).

For additional details, see [Report a Computer Security Vulnerability](https://technet.microsoft.com/en-us/security/ff852094.aspx) on Technet
