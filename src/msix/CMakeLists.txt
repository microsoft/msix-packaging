# MSIX\src\msix
# Copyright (C) 2017 Microsoft.  All rights reserved.
# See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 3.8.0 FATAL_ERROR)

project(msix)

# Handle exports and flags we need to set
list(APPEND MSIX_UNPACK_EXPORTS
    "UnpackPackage"
    "UnpackPackageFromStream"
    "UnpackPackageFromPackageReader"
    "UnpackBundle"
    "UnpackBundleFromStream"
    "UnpackBundleFromBundleReader"
)

if(MSIX_PACK)
    list(APPEND MSIX_PACK_EXPORTS
        "PackPackage"
        "PackBundle"
    )
endif()

list(APPEND MSIX_EXPORTS
    "CoCreateAppxFactory"
    "CoCreateAppxFactoryWithHeap"
    "CoCreateAppxFactoryWithOptions"
    "CoCreateAppxFactoryWithHeapAndOptions"
    "CreateStreamOnFile"
    "CreateStreamOnFileUTF16"
    "MsixGetLogTextUTF8"
    "CoCreateAppxBundleFactory"
    "CoCreateAppxBundleFactoryWithHeap"
    ${MSIX_UNPACK_EXPORTS}
    ${MSIX_PACK_EXPORTS}
)

if(WIN32)
    string(REGEX REPLACE ";" "\n    " MSIX_EXPORTS "${MSIX_EXPORTS}")
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/windowsexports.def.cmakein ${CMAKE_CURRENT_BINARY_DIR}/windowsexports.def CRLF)
else()
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
        if(IOS)
            add_definitions(-DIOS)
        else()
            add_definitions(-DMACOS)
        endif()
        find_library(COREFOUNDATION_LIBRARY CoreFoundation REQUIRED)
        find_library(FOUNDATION_LIBRARY Foundation REQUIRED)
    else()
        if(LINUX)
            add_definitions(-DLINUX)
        endif()
        if(AOSP)
            # add here any exports specific for android
            add_definitions(-DAOSP)
            list(APPEND MSIX_EXPORTS
                "JNI_OnLoad"
            )
        endif()
        # Hide visibility and discard unused functions
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden -ffunction-sections -fdata-sections")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -ffunction-sections -fdata-sections")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--gc-sections")
        # On Linux and linux-derived platforms, you use a version script to achieve similar ends.
        # Make it look readable. The last ; is in the symbol.map.cmakein file
        string(REGEX REPLACE ";" ";\n\t" MSIX_EXPORTS "${MSIX_EXPORTS}")
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/symbol.map.cmakein ${CMAKE_CURRENT_BINARY_DIR}/symbol.map CRLF)
        set(DEFINE_EXPORTS "-Wl,--version-script=${CMAKE_CURRENT_BINARY_DIR}/symbol.map")
    endif()
    message(STATUS "Using export flag: ${DEFINE_EXPORTS}")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${DEFINE_EXPORTS}")
endif()

include(msix_resources) # Handles all the certificates and schemas we are going to use.

set(LIB_PUBLIC_HEADERS
    ../inc/public/AppxPackaging.hpp
    ../inc/public/MSIXWindows.hpp
    ../inc/public/MsixErrors.hpp
)

set(MsixSrc) # list with all the files we are going to use

# Common for pack and unpack
list(APPEND MsixSrc
    common/AppxFactory.cpp
    common/MSIXResource.cpp
    common/Log.cpp
    common/UnicodeConversion.cpp
    common/Encoding.cpp
    common/Exceptions.cpp
    common/AppxPackageInfo.cpp
    common/AppxManifestObject.cpp
    common/ZipObject.cpp
    common/FileNameValidation.cpp
    common/AppxManifestValidation.cpp
    common/IXml.cpp
    common/TimeHelpers.cpp
)

# Unpack. Always add
list(APPEND MsixSrc
    unpack/AppxBlockMapObject.cpp
    unpack/AppxPackageObject.cpp
    unpack/AppxSignature.cpp
    unpack/InflateStream.cpp
    unpack/ZipObjectReader.cpp
)

# Pack
if(MSIX_PACK)
    add_definitions(-DMSIX_PACK=1)
    list(APPEND MsixSrc
        pack/AppxPackageWriter.cpp
        pack/XmlWriter.cpp
        pack/AppxBlockMapWriter.cpp
        pack/ContentTypeWriter.cpp
        pack/ContentType.cpp
        pack/DeflateStream.cpp
        pack/ZipObjectWriter.cpp
        pack/BundleManifestWriter.cpp
        pack/BundleWriterHelper.cpp
        pack/AppxBundleWriter.cpp
        pack/VersionHelpers.cpp
        pack/MappingFileParser.cpp
        pack/BundleValidationHelper.cpp
    )
endif()

# Bundle specific files
if(NOT SKIP_BUNDLES)
    add_definitions(-DBUNDLE_SUPPORT=1)
    list(APPEND MsixSrc
        unpack/AppxBundleManifest.cpp
        unpack/ApplicabilityCommon.cpp
    )
    # Applicability.
    if (WIN32)
        list(APPEND MsixSrc PAL/Applicability/Win32/Applicability.cpp)
    elseif(LINUX)
        find_package(ICU REQUIRED COMPONENTS uc)
        list(APPEND MsixSrc PAL/Applicability/Linux/Applicability.cpp)
    elseif(AOSP)
        list(APPEND MsixSrc PAL/Applicability/AOSP/Applicability.cpp)
    else() # iOS and MACOS
        list(APPEND MsixSrc PAL/Applicability/Apple/Applicability.cpp)
    endif()
endif()

# Compression option
if(((IOS) OR (MACOS)) AND (NOT USE_MSIX_SDK_ZLIB))
    list(APPEND MsixSrc PAL/DataCompression/Apple/CompressionObject.cpp)
else()
    list(APPEND MsixSrc PAL/DataCompression/Zlib/CompressionObject.cpp)
endif()

# Directory object
if(WIN32)
    list(APPEND MsixSrc PAL/FileSystem/Win32/DirectoryObject.cpp)
else()
    list(APPEND MsixSrc PAL/FileSystem/POSIX/DirectoryObject.cpp)
endif()

# Xml Parser
if(XML_PARSER MATCHES xerces)
    list(APPEND MsixSrc PAL/XML/xerces-c/XmlObject.cpp)
    add_definitions(-DUSING_XERCES=1)
elseif(XML_PARSER MATCHES javaxml)
    list(APPEND MsixSrc PAL/XML/AOSP/XmlObject.cpp)
    add_definitions(-DUSING_JAVAXML=1)
elseif(XML_PARSER MATCHES applexml)
    set(XmlParser)
    list(APPEND MsixSrc
        "PAL/XML/APPLE/XmlObject.cpp"
        "PAL/XML/APPLE/NSXmlParserDelegateWrapper.mm"
        "PAL/XML/APPLE/NSXmlParserWrapper.mm"
        "PAL/XML/APPLE/XmlDocumentReader.cpp"
    )
    add_definitions(-DUSING_APPLE_XML=1)
elseif(XML_PARSER MATCHES msxml6)
    list(APPEND MsixSrc PAL/XML/msxml6/XmlObject.cpp)
    add_definitions(-DUSING_MSXML=1)
endif()

# Crypto
if(CRYPTO_LIB MATCHES crypt32)
    list(APPEND MsixSrc
        PAL/Crypto/Win32/Crypto.cpp
        PAL/Signature/Win32/SignatureValidator.cpp
    )
elseif(CRYPTO_LIB MATCHES openssl)
    if(OpenSSL_FOUND)
        list(APPEND MsixSrc
            PAL/Crypto/OpenSSL/Crypto.cpp
            PAL/Signature/OpenSSL/SignatureValidator.cpp
        )
    else()
        # ... and were done here...  :/
        message(FATAL_ERROR "OpenSSL NOT FOUND!")
    endif()
endif()

# Interop
if(AOSP)
    list(APPEND MsixSrc
        PAL/Interop/AOSP/JniHelper.hpp
        PAL/Interop/AOSP/JniHelper.cpp
    )
endif()

message(STATUS "Source files:")
message(STATUS "\tmsix.cpp")
foreach(FILE ${MsixSrc})
    message(STATUS "\t${FILE}")
endforeach()

# Define the library
add_library(${PROJECT_NAME} SHARED 
    msix.cpp
    ${MsixSrc}
 )

# Adding dependency to the third party libs directory
add_dependencies(${PROJECT_NAME} LIBS)

# Copy out public headers to <binary dir>/src/unpack
configure_file(../inc/public/MSIXWindows.hpp   ${CMAKE_CURRENT_BINARY_DIR}/MSIXWindows.hpp  )
configure_file(../inc/public/AppxPackaging.hpp ${CMAKE_CURRENT_BINARY_DIR}/AppxPackaging.hpp)
configure_file(../inc/public/MsixErrors.hpp    ${CMAKE_CURRENT_BINARY_DIR}/MsixErrors.hpp)

# Linker and includes
# Include MSIX headers
target_include_directories(${PROJECT_NAME} PRIVATE ${MSIX_PROJECT_ROOT}/src/inc/public ${MSIX_PROJECT_ROOT}/src/inc/shared ${MSIX_PROJECT_ROOT}/src/inc/internal)

if(WIN32)
    string(REPLACE "/GR" " " CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    if(CMAKE_BUILD_TYPE MATCHES Debug)
        set_target_properties(${PROJECT_NAME} PROPERTIES
            COMPILE_FLAGS "/source-charset:utf-8 /GR-"
            )
    else()
        set_target_properties(${PROJECT_NAME} PROPERTIES
            COMPILE_FLAGS "/source-charset:utf-8 /Gw /GL /GR-"
            )
    endif()
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
    set_property(TARGET ${PROJECT_NAME} APPEND_STRING PROPERTY LINK_FLAGS " /DEF:${CMAKE_CURRENT_BINARY_DIR}/windowsexports.def")
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
    target_include_directories(${PROJECT_NAME} PRIVATE ${MSIX_PROJECT_ROOT}/src/msix/PAL/DataCompression/Apple)
    target_link_libraries(${PROJECT_NAME} PRIVATE libcompression.dylib)
elseif((AOSP) AND (NOT USE_MSIX_SDK_ZLIB))
    # for AOSP, use the libz.so from the android ndk.
    find_package(ZLIB REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE -lz)
else() # WIN32 or USE_MSIX_SDK_ZLIB
    target_include_directories(${PROJECT_NAME} PRIVATE 
            ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/zlib
            ${MSIX_PROJECT_ROOT}/lib/zlib
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
        ${MSIX_PROJECT_ROOT}/lib/xerces/src
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
    target_include_directories(${PROJECT_NAME} PRIVATE ${OpenSSL_INCLUDE_PATH})
    if((IOS) OR (MACOS))
        target_link_libraries(${PROJECT_NAME} PRIVATE crypto -Wl,-dead_strip)
    elseif(NOT MSVC)
        target_link_libraries(${PROJECT_NAME} PRIVATE crypto -Wl,--gc-sections)
    else()
        target_link_libraries(${PROJECT_NAME} PRIVATE crypto)
    endif()
endif()
