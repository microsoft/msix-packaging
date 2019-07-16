# Copyright (C) 2019 Microsoft.  All rights reserved.
# See LICENSE file in the project root for full license information.
# Validates CMake options for the MSIX SDK.

message(STATUS "Validating MSIX SDK options")

# Enforce that target platform is specified.
if((NOT WIN32) AND (NOT MACOS) AND (NOT IOS) AND (NOT AOSP) AND (NOT LINUX))
    message(FATAL_ERROR "You must specify one of: -D[WIN32|MACOS|IOS|AOSP|LINUX]=on")
endif()

if(USE_STATIC_MSVC)
    if(NOT WIN32)
        message(FATAL_ERROR "-DUSE_STATIC_MSVC=on can only be used for Windows")
    endif()
    # By default these flags have /MD set. Modified it to use /MT instead.
    foreach(buildType RELEASE MINSIZEREL RELWITHDEBINFO)
        set(cxxFlag "CMAKE_CXX_FLAGS_${buildType}")
        string(REPLACE "/MD" "/MT" ${cxxFlag} "${${cxxFlag}}")
    endforeach()
    set(cxxFlagDebug "CMAKE_CXX_FLAGS_DEBUG")
    string(REPLACE "/MDd" "/MTd" ${cxxFlagDebug} "${${cxxFlagDebug}}")
endif()

# Set xml parser if not set
if(NOT XML_PARSER)
    if(WIN32)
        set(XML_PARSER msxml6 CACHE STRING "XML Parser not defined. Using msxml6" FORCE)
    elseif(AOSP)
        set(XML_PARSER javaxml CACHE STRING "XML Parser not defined. Using javaxml" FORCE)
    elseif(MAC)
        if(MSIX_PACK)
            set(XML_PARSER xerces CACHE STRING "XML Parser not defined. Using xerces" FORCE)
        else()
            set(XML_PARSER applexml CACHE STRING "XML Parser not defined. Using applexml" FORCE)
        endif()
    elseif(IOS)
        set(XML_PARSER applexml CACHE STRING "XML Parser not defined. Using applexml" FORCE)
    else()
        set(XML_PARSER xerces CACHE STRING "XML Parser not defined. Using xerces" FORCE)
    endif()
endif()

# Set crypto library if not set
if(NOT CRYPTO_LIB)
    if(WIN32)
        set(CRYPTO_LIB crypt32 CACHE STRING "Crypto Lib not defined. Using crypt32" FORCE)
    else()
        set(CRYPTO_LIB openssl CACHE STRING "Crypto Lib not defined. Using openssl" FORCE)
    endif()
endif()

# Validates PACK options are correct
if(MSIX_PACK)
    if (AOSP OR IOS)
        message(FATAL_ERROR "Packaging is not supported for mobile devices.")
    elseif(MAC)
        if(NOT USE_MSIX_SDK_ZLIB)
            message(FATAL_ERROR "Using libCompression APIs and packaging features is not supported.")
        endif()
        if(NOT (XML_PARSER MATCHES xerces))
            message(WARNING "Xerces is the only supported parser for MacOS pack.")
        endif()
    endif()
    if(NOT USE_VALIDATION_PARSER)
        message(STATUS "validation parsing enabled")
        set(USE_VALIDATION_PARSER OFF)
    endif()
endif()
