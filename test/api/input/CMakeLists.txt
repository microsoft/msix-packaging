# Copyright (C) 2019 Microsoft.  All rights reserved.
# See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 3.8.0 FATAL_ERROR)

if(XML_PARSER MATCHES msxml6)
    set(APITEST_1_XPATH "/*[local-name()='Package']/*[local-name()='Applications']/*[local-name()='Application']")
else()
    set(APITEST_1_XPATH "/Package/Applications/Application")
endif()

if(WIN32)
    set(APITEST_1_PACKAGE "..\\test\\appx\\TestAppxPackage_Win32.appx")
    set(APITEST_1_BUNDLE "..\\test\\appx\\bundles\\StoreSigned_Desktop_x86_x64_MoviesTV.appxbundle")
else()
    if (IOS OR AOSP)
        set(APITEST_1_PACKAGE "TestAppxPackage_Win32.appx")
        set(APITEST_1_BUNDLE "bundles/StoreSigned_Desktop_x86_x64_MoviesTV.appxbundle")
    else()
        set(APITEST_1_PACKAGE "../test/appx/TestAppxPackage_Win32.appx")
        set(APITEST_1_BUNDLE "../test/appx/bundles/StoreSigned_Desktop_x86_x64_MoviesTV.appxbundle")
    endif()
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/apitest_test_1.cmakein ${CMAKE_CURRENT_BINARY_DIR}/apitest_test_1.txt CRLF)
