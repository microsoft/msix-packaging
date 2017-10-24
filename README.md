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

SUPPORT
-------
    TODO: write stuffs here

HOW TO CONTRIBUTE TO xPlatAppx
------------------------------
    TODO: write stuffs here

