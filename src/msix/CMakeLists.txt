# MSIX\src\msix
# Copyright (C) 2017 Microsoft.  All rights reserved.
# See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake) # main (top) cmake dir

set(LIBRARY_NAME msix)

project(${LIBRARY_NAME})

# Define PALs
if(XML_PARSER MATCHES xerces)
    message(STATUS "XML_PARSER defined.  Using XERCES-C XML parser." )
    set(XmlParser PAL/XML/xerces-c/XmlObject.cpp)
    add_definitions(-DUSING_XERCES=1)
endif()

if(XML_PARSER MATCHES msxml6)
    message(STATUS "XML_PARSER defined.  Using MSXML6 XML parser." )
    set(XmlParser PAL/XML/msxml6/XmlObject.cpp)
    add_definitions(-DUSING_MSXML=1)
endif()

if(SKIP_BUNDLES)
    message(STATUS "Bundle support disabled.")
else()
    add_definitions(-DBUNDLE_SUPPORT=1)
endif()

if(WIN32)
    set(DirectoryObject PAL/FileSystem/Win32/DirectoryObject.cpp)
    set(SHA256 PAL/SHA256/Win32/SHA256.cpp)
    set(Signature PAL/Signature/Win32/SignatureValidator.cpp)
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
        "IID_IUnknown"
        "IID_IStream"
        "IID_IAppxFactory"
        "IID_IAppxPackageReader"
        "IID_IAppxFile"
        "IID_IAppxFilesEnumerator"
        "IID_IAppxBlockMapReader"
        "IID_IAppxBlockMapFile"
        "IID_IAppxBlockMapFilesEnumerator"
        "IID_IAppxBlockMapBlock"
        "IID_IAppxBlockMapBlocksEnumerator"
        "IID_IAppxManifestReader"
        "IID_IAppxManifestReader2"
        "IID_IAppxManifestReader3"
        "IID_IAppxManifestPackageId"
        "IID_IAppxManifestProperties"
        "IID_IAppxManifestTargetDeviceFamiliesEnumerator"
        "IID_IAppxManifestTargetDeviceFamily"
        "IID_IAppxManifestPackageDependenciesEnumerator"
        "IID_IAppxManifestPackageDependency"
        "IID_IAppxManifestResourcesEnumerator"
        "IID_IAppxManifestApplicationsEnumerator"
        "IID_IAppxManifestApplication"
        "IID_IAppxManifestQualifiedResourcesEnumerator"
        "IID_IAppxManifestQualifiedResource"
        "IID_IAppxBundleFactory"
        "IID_IAppxBundleReader"
        "IID_IAppxBundleManifestReader"
        "IID_IAppxBundleManifestPackageInfoEnumerator"
        "IID_IAppxBundleManifestPackageInfo"
        "IID_IMsixDocumentElement"
        "IID_IMsixElement"
        "IID_IMsixElementEnumerator"
        "IID_IMsixFactoryOverrides"
        "IID_IMsixStreamFactory"
    )
    if((IOS) OR (MACOS))
        # on Apple platforms you can explicitly define which symbols are exported
        set(CMAKE_VISIBILITY_INLINES_HIDDEN     1)
        set(CMAKE_C_VISIBILITY_PRESET           hidden)
        set(CMAKE_CXX_VISIBILITY_PRESET         hidden)
        # iOS and Mac exports start with '_' .The first '_' is be in the exports.def.cmakein
        string(REGEX REPLACE ";" "\n_" MSIX_EXPORTS "${MSIX_EXPORTS}")
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/exports.def.cmakein ${CMAKE_CURRENT_BINARY_DIR}/exports.def CRLF)
        set(DEFINE_EXPORTS  "-exported_symbols_list ${CMAKE_CURRENT_BINARY_DIR}/exports.def")
        # used to get the languages of the device
        find_library(COREFOUNDATION_LIBRARY CoreFoundation)
        if(NOT COREFOUNDATION_LIBRARY)
            message(FATAL_ERROR "CoreFoundation not found")
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

    if(OpenSSL_FOUND)
        message(STATUS "Using OpenSSL ${OpenSLL_VERSION}")
        include_directories(
            ${include_directories}
            ${OpenSLL_INCLUDE_PATH}
        )
        set(SHA256    PAL/SHA256/OpenSSL/SHA256.cpp)
        set(Signature PAL/Signature/OpenSSL/SignatureValidator.cpp)
    else()
        # ... and were done here...  :/
        message(STATUS "OpenSSL NOT FOUND!")
        return()
    endif()
    set(DirectoryObject PAL/FileSystem/POSIX/DirectoryObject.cpp)
endif()

if(USE_VALIDATION_PARSER)
    message(STATUS "PAL: VALIDATING parser" )
    add_definitions(-DVALIDATING=1)
else()
    message(STATUS "PAL: non-validating parser" )
    add_definitions(-DVALIDATING=0)
endif()

message(STATUS "PAL: XML             = ${XmlParser}")
message(STATUS "PAL: DirectoryObject = ${DirectoryObject}")
message(STATUS "PAL: SHA256          = ${SHA256}")
message(STATUS "PAL: Signature       = ${Signature}")
message(STATUS "PAL: Applicability   = ${Applicability}")

include(msix_resources)

set(LIB_PUBLIC_HEADERS
    ../inc/AppxPackaging.hpp
    ../inc/MSIXWindows.hpp
)

if (((IOS) OR (MACOS)) AND (NOT USE_MSIX_SDK_ZLIB))
    set(CompressionObjectCpp    PAL/DataCompression/Apple/CompressionObject.cpp)
else()
    set(CompressionObjectCpp    PAL/DataCompression/Zlib/CompressionObject.cpp)
endif()

set(BundleHeaders)
set(BundleSources)
if(NOT SKIP_BUNDLES)
    list(APPEND BundleHeaders
        ../inc/Applicability.hpp
        ../inc/AppxBundleManifest.hpp
    )
    list(APPEND BundleSources
        AppxBundleManifest.cpp
        ApplicabilityCommon.cpp
        ${Applicability}
    )
endif()

set(LIB_PRIVATE_HEADERS
    ../inc/AppxBlockMapObject.hpp
    ../inc/AppxFactory.hpp
    ../inc/AppxFile.hpp
    ../inc/AppxManifestObject.hpp
    ../inc/AppxPackageInfo.hpp
    ../inc/AppxPackageObject.hpp
    ../inc/AppxSignature.hpp
    ../inc/ComHelper.hpp
    ../inc/DirectoryObject.hpp
    ../inc/Encoding.hpp
    ../inc/Enumerators.hpp
    ../inc/Exceptions.hpp
    ../inc/FileStream.hpp
    ../inc/ICompressionObject.hpp
    ../inc/InflateStream.hpp
    ../inc/Log.hpp
    ../inc/MSIXFactory.hpp
    ../inc/MSIXResource.hpp
    ../inc/ObjectBase.hpp
    ../inc/RangeStream.hpp
    ../inc/StorageObject.hpp
    ../inc/StreamBase.hpp
    ../inc/StreamHelper.hpp
    ../inc/UnicodeConversion.hpp
    ../inc/VectorStream.hpp
    ../inc/VerifierObject.hpp
    ../inc/IXml.hpp
    ../inc/ZipFileStream.hpp
    ../inc/ZipObject.hpp
    ${BundleHeaders}
)

set(LIB_SOURCES
    AppxBlockMapObject.cpp
    AppxFactory.cpp
    AppxManifestObject.cpp
    AppxPackageObject.cpp
    AppxPackageInfo.cpp
    AppxPackaging_i.cpp
    AppxSignature.cpp
    Exceptions.cpp
    InflateStream.cpp
    Log.cpp
    UnicodeConversion.cpp
    msix.cpp
    ZipObject.cpp
    ${DirectoryObject}
    ${SHA256}
    ${Signature}
    ${XmlParser}
    ${CompressionObjectCpp}
    ${BundleSources}
)

# Copy out public headers
configure_file(../inc/MSIXWindows.hpp   ${CMAKE_CURRENT_BINARY_DIR}/MSIXWindows.hpp  )
configure_file(../inc/AppxPackaging.hpp ${CMAKE_CURRENT_BINARY_DIR}/AppxPackaging.hpp)

# Define the library
add_library(${LIBRARY_NAME} SHARED ${LIB_SOURCES} ${LIB_PUBLIC_HEADERS} ${LIB_PRIVATE_HEADERS})

if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        COMPILE_FLAGS "/source-charset:utf-8"
        VS_WINRT_COMPONENT TRUE)
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
    set_property(TARGET ${PROJECT_NAME} APPEND_STRING PROPERTY LINK_FLAGS "${DELAYFLAGS}")
    if(USE_STATIC_MSVC)
        if(CMAKE_BUILD_TYPE MATCHES Debug)
            set_property(TARGET ${PROJECT_NAME} APPEND_STRING PROPERTY LINK_FLAGS " /NODEFAULTLIB:MSVCRTD")
        else()
            set_property(TARGET ${PROJECT_NAME} APPEND_STRING PROPERTY LINK_FLAGS " /NODEFAULTLIB:MSVCRT")
        endif()
    endif()
    target_link_libraries(${PROJECT_NAME} PRIVATE bcrypt crypt32 wintrust runtimeobject.lib delayimp.lib)
endif()

# specify that this library is to be built with C++14
set_property(TARGET ${LIBRARY_NAME} PROPERTY CXX_STANDARD 14)

# Set the build version. It will be used in the name of the lib, with corresponding
# symlinks created. SOVERSION could also be specified for api version.
set_target_properties(${LIBRARY_NAME} PROPERTIES
    VERSION ${VERSION}  # ${VERSION} was defined in the main CMakeLists.
    FRAMEWORK FALSE
    PUBLIC_HEADER "${LIB_HEADERS}" # specify the public headers
)
      
if (((IOS) OR (MACOS)) AND (NOT USE_MSIX_SDK_ZLIB))
    # for macos and ios use the inbox libcompression zlib apis instead of zlib, unless zlib is explicitly requested.
    target_link_libraries(${PROJECT_NAME} PRIVATE libcompression.dylib)
    include_directories(${include_directories}  ${CMAKE_PROJECT_ROOT}/src/msix/PAL/DataCompression/Apple)
elseif ((AOSP) AND (NOT USE_MSIX_SDK_ZLIB))
    # for AOSP, use the libz.so from the android ndk.
    find_package(ZLIB REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE -lz)
else()
    include_directories(
            ${include_directories}
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

if(XML_PARSER MATCHES xerces)
    message(STATUS "MSIX takes a static dependency on xerces")
    include_directories(
        ${include_directories}
        ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/xerces/src
        ${CMAKE_PROJECT_ROOT}/lib/xerces/src
    )
    target_link_libraries(${PROJECT_NAME} PRIVATE xerces-c)
endif()

if(AOSP)
    target_link_libraries(${PROJECT_NAME} PRIVATE -latomic)
    if(NOT SKIP_BUNDLES) # This will change when we have JNI functionality that is not bundle related.
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
        add_jar(${PROJECT_NAME}-jni
            SOURCES
                PAL/java/com/microsoft/msix/JniHelper.java
            INCLUDE_JARS
                ${ANDROID_SDK}/platforms/android-${ANDROID_SDK_VERSION}/android.jar
            OUTPUT_DIR
                ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
            )
    endif()
endif()

if((IOS) OR (MACOS))
    target_link_libraries(${PROJECT_NAME} PRIVATE ${COREFOUNDATION_LIBRARY})
endif()

if(LINUX)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${ICU_LIBRARIES})
endif()

if(OpenSSL_FOUND)
    # include the libraries needed to use OpenSSL
    target_link_libraries(${PROJECT_NAME} PRIVATE crypto)
endif()
