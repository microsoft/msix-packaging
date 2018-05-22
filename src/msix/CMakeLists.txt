# MSIX\src\msix
# Copyright (C) 2017 Microsoft.  All rights reserved.
# See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake) # main (top) cmake dir

set(LIBRARY_NAME msix)

project(${LIBRARY_NAME})

# Define PALs
IF (XML_PARSER MATCHES xerces)
    MESSAGE (STATUS "XML_PARSER defined.  Using XERCES-C XML parser." )
    SET (XmlParser PAL/XML/xerces-c/XmlObject.cpp)
    add_definitions(-DUSING_XERCES=1)
ENDIF()

IF (XML_PARSER MATCHES msxml6)
    MESSAGE (STATUS "XML_PARSER defined.  Using MSXML6 XML parser." )
    SET (XmlParser PAL/XML/msxml6/XmlObject.cpp)
    add_definitions(-DUSING_MSXML=1)
ENDIF()

IF(WIN32)
    SET (DirectoryObject PAL/FileSystem/Win32/DirectoryObject.cpp)
    SET (SHA256 PAL/SHA256/Win32/SHA256.cpp)
    SET (Signature PAL/Signature/Win32/SignatureValidator.cpp)
    SET (Applicability PAL/Applicability/Win32/Applicability.cpp)
ELSE()
    # Visibility variables for non-win32 platforms
    SET(MSIX_EXPORTS)
    LIST(APPEND MSIX_EXPORTS
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
        "IID_IAppxManifestTargetDeviceFamiliesEnumerator"
        "IID_IAppxManifestTargetDeviceFamily"
        "IID_IAppxManifestApplicationsEnumerator"
        "IID_IAppxManifestApplication"
        "IID_IAppxManifestQualifiedResourcesEnumerator"
        "IID_IAppxManifestQualifiedResource"
        "IID_IAppxBundleFactory"
        "IID_IAppxBundleReader"
        "IID_IAppxBundleManifestReader"
        "IID_IAppxBundleManifestPackageInfoEnumerator"
        "IID_IAppxBundleManifestPackageInfo"
    )
    IF((IOS) OR (MACOS))
        # on Apple platforms you can explicitly define which symbols are exported
        SET(CMAKE_VISIBILITY_INLINES_HIDDEN     1)
        SET(CMAKE_C_VISIBILITY_PRESET           hidden)
        SET(CMAKE_CXX_VISIBILITY_PRESET         hidden)
        # iOS and Mac exports start with '_' .The first '_' is be in the exports.def.cmakein
        STRING(REGEX REPLACE ";" "\n_" MSIX_EXPORTS "${MSIX_EXPORTS}")
        CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/exports.def.cmakein ${CMAKE_CURRENT_BINARY_DIR}/exports.def CRLF)
        SET(DEFINE_EXPORTS  "-exported_symbols_list ${CMAKE_CURRENT_BINARY_DIR}/exports.def")    
        # used to get the languages of the device
        FIND_LIBRARY(COREFOUNDATION_LIBRARY CoreFoundation)
        IF(NOT COREFOUNDATION_LIBRARY)
            MESSAGE(FATAL_ERROR "CoreFoundation not found")
        ENDIF()
        SET (Applicability PAL/Applicability/Apple/Applicability.cpp)
        if(IOS)
            add_definitions(-DIOS)
        else()
            add_definitions(-DMACOS)
        endif()
    ELSE()
        IF(LINUX)
            add_definitions(-DLINUX)
            FIND_PACKAGE(ICU COMPONENTS uc)
            SET (Applicability PAL/Applicability/Linux/Applicability.cpp)
        ENDIF()
        IF(AOSP)
            # add here any exports specific for android
            add_definitions(-DAOSP)
            LIST(APPEND MSIX_EXPORTS
                "JNI_OnLoad"
            )
            SET (Applicability PAL/Applicability/AOSP/Applicability.cpp)
        ENDIF()
        # on Linux and linux-derived platforms, you use a version script to achieve similar ends.
        SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -fvisibility=hidden")
        # Make it look readable. The last ; is in the symbol.map.cmakein file
        STRING(REGEX REPLACE ";" ";\n\t" MSIX_EXPORTS "${MSIX_EXPORTS}")
        CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/symbol.map.cmakein ${CMAKE_CURRENT_BINARY_DIR}/symbol.map CRLF)
        SET(DEFINE_EXPORTS "-Wl,--version-script=${CMAKE_CURRENT_BINARY_DIR}/symbol.map")
    ENDIF()
    MESSAGE(STATUS "Using export flag: ${DEFINE_EXPORTS}")
    SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${DEFINE_EXPORTS}")

    IF(OpenSSL_FOUND)
        MESSAGE (STATUS "Using OpenSSL ${OpenSLL_VERSION}")
        INCLUDE_DIRECTORIES(
            ${include_directories}
            ${OpenSLL_INCLUDE_PATH}
        )
        SET (SHA256    PAL/SHA256/OpenSSL/SHA256.cpp)
        SET (Signature PAL/Signature/OpenSSL/SignatureValidator.cpp)
    ELSE()
        # ... and were done here...  :/
        MESSAGE (STATUS "OpenSSL NOT FOUND!")
        RETURN()
    ENDIF()

    SET (DirectoryObject PAL/FileSystem/POSIX/DirectoryObject.cpp)
ENDIF()

IF(USE_VALIDATION_PARSER)
    MESSAGE (STATUS "PAL: VALIDATING parser" )
    add_definitions(-DVALIDATING=1)
ELSE()
    MESSAGE (STATUS "PAL: non-validating parser" )
    add_definitions(-DVALIDATING=0)
ENDIF()

MESSAGE (STATUS "PAL: XML             = ${XmlParser}")
MESSAGE (STATUS "PAL: DirectoryObject = ${DirectoryObject}")
MESSAGE (STATUS "PAL: SHA256          = ${SHA256}")
MESSAGE (STATUS "PAL: Signature       = ${Signature}")
MESSAGE (STATUS "PAL: Applicability   = ${Applicability}")

include(msix_resources)

SET(LIB_PUBLIC_HEADERS
    ../inc/AppxPackaging.hpp
    ../inc/MSIXWindows.hpp
)

SET(LIB_PRIVATE_HEADERS
    ../inc/Applicability.hpp
    ../inc/AppxBlockMapObject.hpp
    ../inc/AppxFactory.hpp
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
)

SET(LIB_SOURCES
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
    ${Applicability}
)

# Copy out public headers
CONFIGURE_FILE(../inc/MSIXWindows.hpp   ${CMAKE_CURRENT_BINARY_DIR}/MSIXWindows.hpp  )
CONFIGURE_FILE(../inc/AppxPackaging.hpp ${CMAKE_CURRENT_BINARY_DIR}/AppxPackaging.hpp)

# Define the library
ADD_LIBRARY(${LIBRARY_NAME} SHARED ${LIB_SOURCES} ${LIB_PUBLIC_HEADERS} ${LIB_PRIVATE_HEADERS})

# specify that this library is to be built with C++14
SET_PROPERTY(TARGET ${LIBRARY_NAME} PROPERTY CXX_STANDARD 14)

# Set the build version. It will be used in the name of the lib, with corresponding
# symlinks created. SOVERSION could also be specified for api version.
SET_TARGET_PROPERTIES(${LIBRARY_NAME} PROPERTIES
    VERSION ${VERSION}  # ${VERSION} was defined in the main CMakeLists.
    FRAMEWORK FALSE
    PUBLIC_HEADER "${LIB_HEADERS}" # specify the public headers
)

INCLUDE_DIRECTORIES(
    ${include_directories}
    ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/zlib
    ${CMAKE_PROJECT_ROOT}/lib/zlib
)
IF(USE_SHARED_ZLIB)
    MESSAGE(STATUS "MSIX takes a static dependency on zlib")
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} PRIVATE zlib)
else()
    MESSAGE(STATUS "MSIX takes a dynamic dependency on zlib")
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} PRIVATE zlibstatic)
endif()

IF (XML_PARSER MATCHES xerces)
    MESSAGE(STATUS "MSIX takes a static dependency on xerces")
    INCLUDE_DIRECTORIES(
        ${include_directories}
        ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/xerces/src
        ${CMAKE_PROJECT_ROOT}/lib/xerces/src
    )
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} PRIVATE xerces-c)
ENDIF()

IF(AOSP)
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} PRIVATE -latomic)
    FIND_PACKAGE(Java REQUIRED)
    IF(NOT DEFINED ENV{ANDROID_HOME})
        MESSAGE(FATAL_ERROR "ANDROID_HOME enviroment variable not found.")
    ENDIF()
    INCLUDE(UseJava)
    ADD_JAR(${PROJECT_NAME}-jni
        SOURCES
            PAL/java/com/microsoft/msix/JniHelper.java
        INCLUDE_JARS
            $ENV{ANDROID_HOME}/platforms/android-${CMAKE_SYSTEM_VERSION}/android.jar
        OUTPUT_DIR
            ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
        )
ENDIF()

IF((IOS) OR (MACOS))
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} PRIVATE ${COREFOUNDATION_LIBRARY})
ENDIF()

IF(LINUX)
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} PRIVATE ${ICU_LIBRARIES})
ENDIF()

IF(OpenSSL_FOUND)
    # include the libraries needed to use OpenSSL
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} PRIVATE crypto)
ENDIF()

if(WIN32)
    SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
        COMPILE_FLAGS "/source-charset:utf-8"
        VS_WINRT_COMPONENT TRUE)
    # Using /DELAYLOAD as linker option will use a helper funtion provided by 
    # Visual C++ that will load the dll at runtime by calling LoadLibrary and GetProcAddress.
    # This dlls are NOT supposed to be loaded ever on Win7. See Exceptions.cpp 
    # on how we handle failure of delayloading these.
    # https://docs.microsoft.com/en-us/cpp/build/reference/error-handling-and-notification
    SET(DELAYFLAGS)
    LIST(APPEND DELAYFLAGS
            "/DELAYLOAD:api-ms-win-core-winrt-string-l1-1-0.dll"
            "/DELAYLOAD:api-ms-win-core-winrt-l1-1-0.dll")
    STRING(REPLACE ";" " " DELAYFLAGS "${DELAYFLAGS}")
    SET_PROPERTY(TARGET ${PROJECT_NAME} APPEND_STRING PROPERTY LINK_FLAGS "${DELAYFLAGS}")
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} PRIVATE bcrypt crypt32 wintrust runtimeobject.lib delayimp.lib)
endif()