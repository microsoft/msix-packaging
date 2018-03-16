# MSIX Packaging SDK 
--------------------
    Copyright (c) 2017 Microsoft Corp.  All rights reserved.

# DESCRIPTION
-------------
    The MSIX Packaging format is an evolution of the Windows 8 era AppX package format as detailed at: 
    https://blogs.msdn.microsoft.com/windowsappdev/2012/12/04/designing-a-simple-and-secure-app-package-appx/.  

    The MSIX Packaging SDK project is an effort to enable developers on a variety of platforms to package and unpackage 
    application packages for the purposes of distribution from either the Microsoft Store, or 2nd/3rd party stores.  
    
    The MSIX Packaging APIs that a client app would use to interact with .msix/.appx packages are a subset of those
    documented here: https://msdn.microsoft.com/en-us/library/windows/desktop/hh446766(v=vs.85).aspx.  See 
    ./sample/ExtractContentsSample/ExtractContentsSample.cpp for additional details.


# OVERVIEW
----------
The MSIX Packaging SDK project includes cross platform API support for unpacking of .msix/.appx packages

    The MSIX Packaging SDK project includes:

        msix            - A shared library (DLL on Win32, dylib on MacOs, SO on Linux and Android) that exports a subset
                          of the functionality contained within appxpackaging.dll on Windows.  See:
                          https://msdn.microsoft.com/en-us/library/windows/desktop/hh446766(v=vs.85).aspx
                          for additional details.

                          On all platforms instead of CoCreating IAppxFactory, a c-style export: CoCreateAppxFactory is 
                          provided.  See sample folder at root of package for cross platform consumption examples.

                          Finally, there is one export 'Unpack' that provides an simplified unpackage implementation.
                          
        makemsix        - A command line wrapper over the Unpack implementation.  This tool exists
                          primarily as a means of validating the implementation of the MSIX Packaging SDK internal
                          routines and is compiled for Win32, MacOS, and Linux platforms.

# SETUP INSTRUCTIONS
--------------------
    Step 1: Clone the repository:
        git clone [URL]
        
    Step 2: Initialize git submodules:
        git submodule init
        git submodule update

# ISSUES
--------
    If you are using Visual Studio 2017 and you run into errors about not being able to find the v140 toolset, then 1st:
        step 1: Install the Microsoft Build Tools (https://chocolatey.org/packages/microsoft-build-tools)
        step 2: start -> visual studio installer -> Visual Studio Build Tools 2017 -> Modify the 2014 toolset -> individual components 
        step 3: make sure that VC++ 2015.3 v140 toolset for desktop is selected and then unselect VC++ 2017 141 toolset
        step 4: close, then re-open the solution.

# PREREQUISITES
---------------
    Make sure that you have CMAKE installed on your machine 

    * https://cmake.org/download/

    One or more of the following prerequisites may also be required on your machine:

    Ninja-build:
    ------------
    https://github.com/ninja-build/ninja/releases

    Android NDK:
    ------------
    https://developer.android.com/ndk/downloads/index.html

    Clang/LLVM:
    -----------
    http://releases.llvm.org/download.html
    
    VS 2017 clients: 
    ----------------
    Open Visual Studio 2017
    File->Open Folder->navigate to project root and select "CMakeLists.txt"

    See: https://blogs.msdn.microsoft.com/vcblog/2016/10/05/cmake-support-in-visual-studio/
    for details regarding how to configure your environment.

    Xcode clients: 
    --------------
    open terminal, from project root:
    mkdir build && cd build && cmake -DMACOS=on -G"Xcode" ..
    open xcode
    File->Open->navigate to project root/build and select "Project.xcodeproj"

    See: https://www.johnlamp.net/cmake-tutorial-2-ide-integration.html#section-Xcode for additional details

# BUILD
-------
    On Windows using Visual Studio 2017 nmake:
    ------------------------------------------
        makewin32.cmd

    On Mac using make: 
    ------------------
        ./makemac
        ./makeios
    
    On Linux using make:
    --------------------
        ./makelinux
        ./makeaosp
    
    How to compile for Android on Windows:
    --------------------------------------
    Unpack the latest Android NDK to c:\android-ndk
    Unpack Ninja-build to c:\ninja
    Add c:\ninja to the path environment variable
    Create a folder under the root of the enlistment called "android", cd into that folder, then run the following command to create ninja build files:

    cmake -DCMAKE_ANDROID_NDK=c:/android-ndk ^
        -DCMAKE_ANDROID_NDK_TOOLCHAIN_VERSION=clang ^
        -DCMAKE_SYSTEM_NAME=Android ^
        -DCMAKE_SYSTEM_VERSION=19 ^
        -DCMAKE_ANDROID_ARCH_ABI=x86 ^
        -DCMAKE_ANDROID_STL_TYPE=c++_shared ^
        -DCMAKE_BUILD_TYPE=Release ^
        -DAOSP=on ^
        -G"Ninja" ..

    To compile, run the following command from the android folder:

    ninja    

# Contributing
--------------
    This project welcomes contributions and suggestions. Most contributions require you to
    agree to a Contributor License Agreement (CLA) declaring that you have the right to,
    and actually do, grant us the rights to use your contribution. For details, visit
    https://cla.microsoft.com.

    When you submit a pull request, a CLA-bot will automatically determine whether you need
    to provide a CLA and decorate the PR appropriately (e.g., label, comment). Simply follow the
    instructions provided by the bot. You will only need to do this once across all repositories using our CLA.

    This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/).
    For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/)
    or contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.