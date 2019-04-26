# MSIX\src\msix
# Copyright (C) 2017 Microsoft.  All rights reserved.
# See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 3.8.0 FATAL_ERROR)

project(msix)

if(SKIP_BUNDLES)
    message(STATUS "Bundle support disabled.")
else()
    add_definitions(-DBUNDLE_SUPPORT=1)
endif()

# Define PALs
if(XML_PARSER MATCHES xerces)
    message(STATUS "XML_PARSER defined.  Using XERCES-C XML parser." )
    set(XmlParser PAL/XML/xerces-c/XmlObject.cpp)
    add_definitions(-DUSING_XERCES=1)
endif()

if(XML_PARSER MATCHES javaxml)
    message(STATUS "XML_PARSER defined.  Using javaxml parser." )
    set(XmlParser PAL/XML/AOSP/XmlObject.cpp)
    add_definitions(-DUSING_JAVAXML=1)
endif()

if(XML_PARSER MATCHES applexml)
    message(STATUS "XML_PARSER defined.  Using apple xml parser." )
    set(XmlParser)
    list(APPEND XmlParser
        "PAL/XML/APPLE/XmlObject.cpp"
        "PAL/XML/APPLE/NSXmlParserDelegateWrapper.mm"
        "PAL/XML/APPLE/NSXmlParserWrapper.mm"
        "PAL/XML/APPLE/XmlDocumentReader.cpp"
    )
    add_definitions(-DUSING_APPLE_XML=1)
endif()

if(XML_PARSER MATCHES msxml6)
    message(STATUS "XML_PARSER defined.  Using MSXML6 XML parser." )
    set(XmlParser PAL/XML/msxml6/XmlObject.cpp)
    add_definitions(-DUSING_MSXML=1)
endif()

if(CRYPTO_LIB MATCHES crypt32)
    message(STATUS "CRYPTO_LIB defined.  Using crypt32 library." )
    set(SHA256 PAL/SHA256/Win32/SHA256.cpp)
    set(Signature PAL/Signature/Win32/SignatureValidator.cpp)
    add_definitions(-DUSING_CRYPT32=1)
endif()

if(CRYPTO_LIB MATCHES openssl)
    message(STATUS "CRYPTO_LIB defined.  Using OpenSSL library." )
    if(OpenSSL_FOUND)
        message(STATUS "Using OpenSSL ${OpenSLL_VERSION}")
        set(SHA256    PAL/SHA256/OpenSSL/SHA256.cpp)
        set(Signature PAL/Signature/OpenSSL/SignatureValidator.cpp)
    else()
        # ... and were done here...  :/
        message(STATUS "OpenSSL NOT FOUND!")
        return()
    endif()
endif()

if(WIN32)
    set(DirectoryObject PAL/FileSystem/Win32/DirectoryObject.cpp)
    set(Applicability PAL/Applicability/Win32/Applicability.cpp)
else()
    # Visibility variables for non-win32 platforms
    set(MSIX_EXPORTS)
    list(APPEND MSIX_EXPORTS
        "CoCreateAppxFactory"
        "CoCreateAppxFactoryWithHeap"
        "CreateStreamOnFile"
        "CreateStreamOnFileUTF16"
        "GetLogTextUTF8"
        "UnpackPackage"
        "UnpackPackageFromStream"
        "UnpackBundle"
        "UnpackBundleFromStream"
        "CoCreateAppxBundleFactory"
        "CoCreateAppxBundleFactoryWithHeap"
    )
    if((IOS) OR (MACOS))
        # on Apple platforms you can explicitly define which symbols are exported
        set(CMAKE_VISIBILITY_INLINES_HIDDEN     1)
        set(CMAKE_C_VISIBILITY_PRESET           hidden)
        set(CMAKE_CXX_VISIBILITY_PRESET         hidden)
        if (ASAN)
            set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fsanitize=address")
        endif()

        # iOS and Mac exports start with '_' .The first '_' is be in the exports.def.cmakein
        string(REGEX REPLACE ";" "\n_" MSIX_EXPORTS "${MSIX_EXPORTS}")
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/exports.def.cmakein ${CMAKE_CURRENT_BINARY_DIR}/exports.def CRLF)
        set(DEFINE_EXPORTS  "-exported_symbols_list ${CMAKE_CURRENT_BINARY_DIR}/exports.def")
        # used to get the languages of the device
        find_library(COREFOUNDATION_LIBRARY CoreFoundation)
        if(NOT COREFOUNDATION_LIBRARY)
            message(FATAL_ERROR "CoreFoundation library not found")
        endif()
        find_library(FOUNDATION_LIBRARY Foundation)
        if(NOT FOUNDATION_LIBRARY)
            message(FATAL_ERROR "Foundation library not found")
        endif()
        set(Applicability PAL/Applicability/Apple/Applicability.cpp)
        if(IOS)
            add_definitions(-DIOS)
        else()
            add_definitions(-DMACOS)
        endif()
    else()
        if(LINUX)
            add_definitions(-DLINUX)
            find_package(ICU COMPONENTS uc)
            set(Applicability PAL/Applicability/Linux/Applicability.cpp)
        endif()
        if(AOSP)
            # add here any exports specific for android
            add_definitions(-DAOSP)
            list(APPEND MSIX_EXPORTS
                "JNI_OnLoad"
            )
            set(InteropHpp    PAL/Interop/AOSP/JniHelper.hpp)
            set(InteropCpp    PAL/Interop/AOSP/JniHelper.cpp)
            set(Applicability PAL/Applicability/AOSP/Applicability.cpp)
        endif()
        # on Linux and linux-derived platforms, you use a version script to achieve similar ends.
        set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -fvisibility=hidden")
        # Make it look readable. The last ; is in the symbol.map.cmakein file
        string(REGEX REPLACE ";" ";\n\t" MSIX_EXPORTS "${MSIX_EXPORTS}")
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/symbol.map.cmakein ${CMAKE_CURRENT_BINARY_DIR}/symbol.map CRLF)
        set(DEFINE_EXPORTS "-Wl,--version-script=${CMAKE_CURRENT_BINARY_DIR}/symbol.map")
    endif()
    message(STATUS "Using export flag: ${DEFINE_EXPORTS}")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${DEFINE_EXPORTS}")

    set(DirectoryObject PAL/FileSystem/POSIX/DirectoryObject.cpp)
endif()

if(USE_VALIDATION_PARSER)
    message(STATUS "PAL: VALIDATING parser" )
    add_definitions(-DVALIDATING=1)
else()
    message(STATUS "PAL: non-validating parser" )
    add_definitions(-DVALIDATING=0)
endif()

# Compression option
if(((IOS) OR (MACOS)) AND (NOT USE_MSIX_SDK_ZLIB))
    set(CompressionObject PAL/DataCompression/Apple/CompressionObject.cpp)
else()
    set(CompressionObject PAL/DataCompression/Zlib/CompressionObject.cpp)
endif()

message(STATUS "PAL: XML             = ${XmlParser}")
message(STATUS "PAL: DirectoryObject = ${DirectoryObject}")
message(STATUS "PAL: SHA256          = ${SHA256}")
message(STATUS "PAL: Signature       = ${Signature}")
message(STATUS "PAL: Applicability   = ${Applicability}")
message(STATUS "PAL: Compression     = ${CompressionObject}")

include(msix_resources)

set(LIB_PUBLIC_HEADERS
    ../inc/AppxPackaging.hpp
    ../inc/MSIXWindows.hpp
    ../inc/MsixErrors.hpp
)

# Bundle specific files
set(BundleSources)
if(NOT SKIP_BUNDLES)
    list(APPEND BundleSources
        AppxBundleManifest.cpp
        ApplicabilityCommon.cpp
        ${Applicability}
    )
endif()

# Define the library
add_library(${PROJECT_NAME} SHARED 
    AppxBlockMapObject.cpp
    AppxFactory.cpp
    AppxManifestObject.cpp
    AppxManifestValidation.cpp
    AppxPackageObject.cpp
    AppxPackageInfo.cpp
    AppxSignature.cpp
    Encoding.cpp
    Exceptions.cpp
    InflateStream.cpp
    Log.cpp
    UnicodeConversion.cpp
    msix.cpp
    ZipObject.cpp
    MSIXResource.cpp
    IXML.cpp
    ${DirectoryObject}
    ${SHA256}
    ${Signature}
    ${XmlParser}
    ${CompressionObject}
    ${InteropCpp}
    ${BundleSources}
)

# Copy out public headers to <binary dir>/src/msix
configure_file(../inc/MSIXWindows.hpp   ${CMAKE_CURRENT_BINARY_DIR}/MSIXWindows.hpp  )
configure_file(../inc/AppxPackaging.hpp ${CMAKE_CURRENT_BINARY_DIR}/AppxPackaging.hpp)
configure_file(../inc/MsixErrors.hpp ${CMAKE_CURRENT_BINARY_DIR}/MsixErrors.hpp)

# Linker and includes
# Include MSIX headers
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_PROJECT_ROOT}/src/inc)



if(WIN32)
    string(REPLACE "/GR" " " CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        COMPILE_FLAGS "/source-charset:utf-8 /Gw /GL /GR-"
        )
    # Using /DELAYLOAD as linker option will use a helper funtion provided by
    # Visual C++ that will load the dll at runtime by calling LoadLibrary and GetProcAddress.
    # This dlls are NOT supposed to be loaded ever on Win7. See Exceptions.cpp
    # on how we handle failure of delayloading these.
    # https://docs.microsoft.com/en-us/cpp/build/reference/error-handling-and-notification
    set(DELAYFLAGS)
    list(APPEND DELAYFLAGS
            "/DELAYLOAD:api-ms-win-core-winrt-string-l1-1-0.dll"
            "/DELAYLOAD:api-ms-win-core-winrt-l1-1-0.dll")
    string(REPLACE ";" " " DELAYFLAGS "${DELAYFLAGS}")
    set_property(TARGET ${PROJECT_NAME} APPEND_STRING PROPERTY LINK_FLAGS "${DELAYFLAGS} /LTCG")
    if(USE_STATIC_MSVC)
        if(CMAKE_BUILD_TYPE MATCHES Debug)
            set_property(TARGET ${PROJECT_NAME} APPEND_STRING PROPERTY LINK_FLAGS " /NODEFAULTLIB:MSVCRTD")
        else()
            set_property(TARGET ${PROJECT_NAME} APPEND_STRING PROPERTY LINK_FLAGS " /NODEFAULTLIB:MSVCRT")
        endif()
    endif()
    target_link_libraries(${PROJECT_NAME} PRIVATE bcrypt crypt32 wintrust runtimeobject.lib delayimp.lib)
endif()

# Set the build version. It will be used in the name of the lib, with corresponding
# symlinks created.
set_target_properties(${PROJECT_NAME} PROPERTIES
    FRAMEWORK FALSE
    PUBLIC_HEADER "${LIB_PUBLIC_HEADERS}" # specify the public headers
)

# Compression
if(((IOS) OR (MACOS)) AND (NOT USE_MSIX_SDK_ZLIB))
    # for macos and ios use the inbox libcompression zlib apis instead of zlib, unless zlib is explicitly requested.
    message(STATUS "MSIX takes a dependency on inbox libcompression")
    target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_PROJECT_ROOT}/src/msix/PAL/DataCompression/Apple)
    target_link_libraries(${PROJECT_NAME} PRIVATE libcompression.dylib)
elseif((AOSP) AND (NOT USE_MSIX_SDK_ZLIB))
    # for AOSP, use the libz.so from the android ndk.
    message(STATUS "MSIX takes a dependency on inbox zlib")
    find_package(ZLIB REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE -lz)
else() # WIN32 or USE_MSIX_SDK_ZLIB
    target_include_directories(${PROJECT_NAME} PRIVATE 
            ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/zlib
            ${CMAKE_PROJECT_ROOT}/lib/zlib
            ${CMAKE_PROJECT_ROOT}/src/msix/PAL/DataCompression/Zlib
        )
    if(USE_SHARED_ZLIB)
        message(STATUS "MSIX takes a dynamic dependency on zlib")
        target_link_libraries(${PROJECT_NAME} PRIVATE zlib)
    else()
        message(STATUS "MSIX takes a static dependency on zlib")
        target_link_libraries(${PROJECT_NAME} PRIVATE zlibstatic)
    endif()
endif()

# Parser
if(XML_PARSER MATCHES xerces)
    target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/xerces/src
        ${CMAKE_PROJECT_ROOT}/lib/xerces/src
    )
    target_link_libraries(${PROJECT_NAME} PRIVATE xerces-c)
endif()

if(XML_PARSER MATCHES applexml)
    target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/PAL/XML/Apple)
endif()

if(AOSP)
    target_link_libraries(${PROJECT_NAME} PRIVATE -latomic)
    if((NOT SKIP_BUNDLES) OR (XML_PARSER MATCHES javaxml))
        target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/PAL/Interop/AOSP)
        # JNI
        find_package(Java REQUIRED)
        if(NOT ANDROID_SDK)
            if(NOT DEFINED ENV{ANDROID_HOME})
                message(FATAL_ERROR "ANDROID_HOME enviroment variable not found.")
            endif()
            set(ANDROID_SDK $ENV{ANDROID_HOME})
        endif()
        if(NOT ANDROID_SDK_VERSION)
            SET(ANDROID_SDK_VERSION 24)
        endif()
        if(ANDROID_SDK_VERSION LESS 24)
            message(FATAL_ERROR "MSIX minimum Android SDK is 24")
        endif()
        include(UseJava)
        message(STATUS "Android SDK = ${ANDROID_SDK}")
        set(jar_sources)
        if(NOT SKIP_BUNDLES)
            list(APPEND jar_sources PAL/java/com/microsoft/msix/Language.java)
        endif()
        if(XML_PARSER MATCHES javaxml)
            list(APPEND jar_sources
                PAL/java/com/microsoft/msix/XmlDom.java
                PAL/java/com/microsoft/msix/XmlElement.java
                PAL/java/com/microsoft/msix/JavaXmlErrorHandler.java
                PAL/java/com/microsoft/msix/JavaXmlException.java)
        endif()
        # we only use JNI for bundles or to use java parser, don't even bother otherwise
        if(jar_sources)
            add_jar(${PROJECT_NAME}-jni
                SOURCES
                    ${jar_sources}
                INCLUDE_JARS
                    ${ANDROID_SDK}/platforms/android-${ANDROID_SDK_VERSION}/android.jar
                OUTPUT_DIR
                    ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
                )
        endif()
    endif()
endif()

# Misc Linking
if((IOS) OR (MACOS))
    target_link_libraries(${PROJECT_NAME} PRIVATE ${COREFOUNDATION_LIBRARY} ${FOUNDATION_LIBRARY})
endif()
if(LINUX)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${ICU_LIBRARIES})
endif()

if(OpenSSL_FOUND)
    # include the libraries needed to use OpenSSL
    target_include_directories(${PROJECT_NAME} PRIVATE ${OpenSLL_INCLUDE_PATH})
    if((IOS) OR (MACOS))
        target_link_libraries(${PROJECT_NAME} PRIVATE crypto -Wl,-dead_strip)
    else()
        target_link_libraries(${PROJECT_NAME} PRIVATE crypto -Wl,--gc-sections)
    endif()
endif()