xPlatAppx 
---------
    Copyright (c) 2017 Microsoft Corp. 
    All rights reserved.

DESCRIPTION
-----------
    The xPlatAppx project is an effort to enable developers on a variety of platforms to package and unpackage 
    application packages for the purposes of distribution from either the Microsoft Windows Apps Store, or 
    2nd/3rd party stores.  To that end, the file format of these packages need to be in a format that is easily 
    digestible to through the Microsoft Windows Apps Store back-end ingestion processes; which means that the 
    file format for these packages will be an .AppX package.


OVERVIEW
--------
The xPlatAppx project includes:
1. Cross platform API support for unpacakge of .appx package 
2. Developer tooling to build .appx package on Windows, MacOS and Linux. 

The scheduling of the highlevel work listed above will be determined with respect to the relative priorities. 

    The xPlatAppx project includes:

        xPlatAppx       - A shared library (DLL on Win32, dylib on MacOs, SO on Linux) that exports a subset
                          of the functionality contained within appxpackaging.dll on Windows.  See:
                          https://msdn.microsoft.com/en-us/library/windows/desktop/hh446766(v=vs.85).aspx
                          for additional details.

                          On platforms that do not support COM (e.g. anything _other_ than Windows) instead of
                          CoCreating IAppxFactory, a c-style export: CoCreateAppxFactory is provided instead.
                          See sample folder at root of package for cross platform consumption examples

                          Finally, there are two additional exports: 'Pack' and 'Unpack' that provide
                          simplified package and unpackage implementations respectively.
                          
        MakeXplatAppx   - A command line wrapper over the Pack and Unpack implementations.  This tool exists
                          primarily as a means of validating the implementation of xPlatAppx internal routines
                          and is compiled for Win32, MacOS, and Linux platforms.

SETUP INSTRUCTIONS
------------------
    Step 1: Clone the repository:
        git clone https://microsoft.visualstudio.com/DefaultCollection/_git/xPlatAppx
    Step 2: Initialize git submodules:
        git submodule init
        git submodule update

ISSUES
------
    If you are using Visual Studio 2017 and you run into errors about not being able to find the v140 toolset, then 1st:
        step 1: Install the Microsoft Build Tools (https://chocolatey.org/packages/microsoft-build-tools)
        step 2: start -> visual studio installer -> Visual Studio Build Tools 2017 -> Modify the 2014 toolset -> individual components 
        step 3: make sure that VC++ 2015.3 v140 toolset for desktop is selected and then unselect VC++ 2017 141 toolset
        step 4: close, then re-open the solution.

INSTALLATION
------------
    TODO: write stuffs here


PREREQUISITES
-------------
    Make sure that you have CMAKE installed on your machine 

    * https://cmake.org/download/

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

    Make Android on Windows:
    ------------------------
    Unpack the latest Android NDK to c:\android-ndk
    Unpack Ninja-build to c:\ninja
    Add c:\ninja to the path environment variable
    Create a folder under the root of the enlistment called "android", cd into that folder, then run the following command to create ninja build files:

    cmake -DCMAKE_ANDROID_NDK=c:/android-ndk ^
        -DCMAKE_ANDROID_NDK_TOOLCHAIN_VERSION=clang ^
        -DCMAKE_SYSTEM_NAME=Android ^
        -DCMAKE_SYSTEM_VERSION=27 ^
        -DCMAKE_ANDROID_ARCH_ABI=x86 ^
        -DCMAKE_ANDROID_STL_TYPE=c++_static ^
        -DCMAKE_BUILD_TYPE=Release ^
        -DAOSP=on ^
        -G"Ninja" ..

    To compile, run the following command from the android folder:

    ninja


    Ninja-build:
    ------------
    https://github.com/ninja-build/ninja/releases

    Android NDK:
    ------------
    https://developer.android.com/ndk/downloads/index.html

    Clang/LLVM:
    -----------
    http://releases.llvm.org/download.html

BUILD
-----
    On Windows using Visual Studio 2017 nmake:
        makewin32.cmd

    On Mac using make: 
        ./makemac
    

SUPPORT
-------
    TODO: write stuffs here

HOW TO CONTRIBUTE TO xPlatAppx
------------------------------
    TODO: write stuffs here
