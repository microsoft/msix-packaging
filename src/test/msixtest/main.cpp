//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
// This file is used for Windows, macOS and Linux
// Provides a main to create the msixtest executable.
#include "msixtest.hpp"

#include <string>

int main( int argc, char* argv[] )
{
    // test data is relative to the executable find out were it is.
    std::string exePath = std::string(argv[0]);
    #ifdef WIN32
    auto find = exePath.find_last_of('\\');
    #else
    auto find = exePath.find_last_of('/');
    #endif
    if (find != std::string::npos)
    {
        exePath = exePath.substr(0, find);
    }
    return msixtest_main(argc, argv, exePath.c_str());
}
