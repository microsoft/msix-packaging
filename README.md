xPlatAppx 

Copyright (c) 2017 Microsoft Corp. 
All rights reserved.

DESCRIPTION
-----------
The xPlatAppx project is an effort to enable developers on a variety of platforms to package and unpackage application packages for the purposes of distribution from either the Microsoft Windows Apps Store, or 2nd/3rd party stores.  To that end, the file format of these packages need to be in a format that is easily digestible to through the Microsoft Windows Apps Store back-end ingestion processes; which means that the file format for these packages will be an .AppX package.

OVERVIEW
--------
The xPlatAppx project includes:
1. Cross platform API support for unpacakge of .appx package 
2. Developer tooling to build .appx package on Windows, MacOS and Linux. 

The scheduling of the highlevel work listed above will be determined with respect to the relative priorities. 

<More detailed info will be added soon. >

BUILD INSTRUCTIONS
------------------

Step 1: Clone the repository:

git clone https://microsoft.visualstudio.com/DefaultCollection/_git/xPlatAppx

Step 2: Initialize git submodules:

git submodule init

git submodule update

HELP
----

If you are using Visual Studio 2017 and you run into errors about not being able to find the v140 toolset, then 1st:

step 1: Install the Microsoft Build Tools (https://chocolatey.org/packages/microsoft-build-tools)

step 2: start -> visual studio installer -> Visual Studio Build Tools 2017, enable the 2014 toolset

step 3: close, then re-open the solution.

INSTALLATION
------------

TODO: write stuffs here

SUPPORT
-------

TODO: write stuffs here

HOW TO CONTRIBUTE TO xPlatAppx
------------------------------

TODO: write stuffs here

