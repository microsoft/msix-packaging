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
    SET (DirectoryObject PAL/FileSystem/Win32/DirectoryObject.CPP)
    SET (SHA256 PAL/SHA256/Win32/SHA256.CPP)
    SET (Signature PAL/Signature/Win32/SignatureValidator.cpp)
	SET (MSIX_API=1)    
ELSE()
    # Visibility variables for non-win32 platforms
    IF((IOS) OR (MACOS))
        # on Apple platforms you can explicitly define which symbols are exported
        SET(CMAKE_VISIBILITY_INLINES_HIDDEN     1)
        SET(CMAKE_C_VISIBILITY_PRESET           hidden)    
        SET(CMAKE_CXX_VISIBILITY_PRESET         hidden)            
        SET(DEFINE_EXPORTS                      "-exported_symbols_list ${CMAKE_PROJECT_ROOT}/src/msix/exports.def")    
    ELSE()
        # on Linux and linux-derived platforms, you use a version script to achieve similar ends.
        SET(CMAKE_C_FLAGS                       "${CMAKE_C_FLAGS} -fvisibility=hidden")
        SET(DEFINE_EXPORTS                      "-Wl,--version-script=${CMAKE_PROJECT_ROOT}/src/msix/symbol.map")
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

MESSAGE (STATUS "PAL: DirectoryObject = ${DirectoryObject}")
MESSAGE (STATUS "PAL: SHA256          = ${SHA256}")
MESSAGE (STATUS "PAL: Signature       = ${Signature}")
MESSAGE (STATUS "PAL: XML             = ${XmlParser}")

include(msix_resources)

SET(LIB_PUBLIC_HEADERS
    ../inc/AppxPackaging.hpp
    ../inc/MSIXWindows.hpp
)

SET(LIB_PRIVATE_HEADERS
    ../inc/AppxBlockMapObject.hpp
    ../inc/AppxFactory.hpp
    ../inc/AppxPackageObject.hpp
    ../inc/AppxSignature.hpp
    ../inc/ComHelper.hpp
    ../inc/DirectoryObject.hpp
    ../inc/Exceptions.hpp
    ../inc/FileStream.hpp
    ../inc/InflateStream.hpp
    ../inc/Log.hpp
    ../inc/ObjectBase.hpp
    ../inc/RangeStream.hpp
    ../inc/StorageObject.hpp
    ../inc/StreamBase.hpp
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
    AppxPackageObject.cpp
    AppxPackaging_i.cpp
    AppxSignature.cpp
    InflateStream.cpp
    Log.cpp
    UnicodeConversion.cpp
    msix.cpp
    ZipObject.cpp
    ${DirectoryObject}
    ${SHA256}
    ${Signature}
    ${XmlParser}
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

MESSAGE(STATUS "MSIX takes a static dependency on zlib")
INCLUDE_DIRECTORIES(
	${include_directories}
	${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/zlib
	${CMAKE_PROJECT_ROOT}/lib/zlib
)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} PRIVATE zlibstatic)

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
ENDIF()

IF(OpenSSL_FOUND)
    # include the libraries needed to use OpenSSL
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} PRIVATE crypto)
ENDIF()

if(WIN32)
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} PRIVATE bcrypt crypt32 wintrust)
endif()