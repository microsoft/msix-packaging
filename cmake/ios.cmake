# iOS CMAKE toolchain.
# Pieced together via ROME ios.cmake and LOTS of time searching through CMAKE documentation and various examples of other ios.cmake's
# out in the wild.  Tested with xCode 9 and on MacOS 10.12
EXECUTE_PROCESS(COMMAND xcodebuild -version OUTPUT_VARIABLE XCODE_VERSION ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
STRING(REGEX MATCH "Xcode [0-9\\.]+" XCODE_VERSION "${XCODE_VERSION}")
STRING(REGEX REPLACE "Xcode ([0-9\\.]+)" "\\1" XCODE_VERSION "${XCODE_VERSION}")
MESSAGE(STATUS "Using Xcode version: ${XCODE_VERSION}")

# Default to building for iPhoneOS if not specified otherwise as we cannot determine the platform from the CMAKE_OSX_ARCHITECTURES 
# variable.  Use of CMAKE_OSX_ARCHITECTURES is such that try_compile can correctly determine the value of IOS_PLATFORM from the root 
# project, as CMAKE_OSX_ARCHITECTURES is propagated.
IF (NOT DEFINED IOS_PLATFORM)
    IF (CMAKE_OSX_ARCHITECTURES)
        IF (CMAKE_OSX_ARCHITECTURES MATCHES ".*arm.*")
            SET(IOS_PLATFORM "OS")
        ELSEIF (CMAKE_OSX_ARCHITECTURES MATCHES "i386")
            SET(IOS_PLATFORM "SIMULATOR")
        ELSEIF (CMAKE_OSX_ARCHITECTURES MATCHES "x86_64")
            SET(IOS_PLATFORM "SIMULATOR64")
        ENDIF()
    ENDIF()

    IF (NOT IOS_PLATFORM)
        SET(IOS_PLATFORM "OS")
    ENDIF()
ENDIF()
SET(IOS_PLATFORM ${IOS_PLATFORM} CACHE STRING "Type of iOS platform for which to build.")

# Determine the platform name and architectures for use in xcodebuild commands from the specified IOS_PLATFORM name.
IF (IOS_PLATFORM STREQUAL "OS")
    SET(XCODE_IOS_PLATFORM iphoneos)
    #SET(IOS_ARCH armv7 armv7s arm64)
ELSEIF (IOS_PLATFORM STREQUAL "SIMULATOR")
    SET(XCODE_IOS_PLATFORM iphonesimulator)
    SET(IOS_ARCH i386)
ELSEIF(IOS_PLATFORM STREQUAL "SIMULATOR64")
    SET(XCODE_IOS_PLATFORM iphonesimulator)
    SET(IOS_ARCH x86_64)
ELSE()
    MESSAGE(FATAL_ERROR "Invalid IOS_PLATFORM: ${IOS_PLATFORM}")
ENDIF()
MESSAGE(STATUS "Configuring iOS build for platform: ${IOS_PLATFORM}, architecture(s): ${IOS_ARCH}")

# If user did not specify the SDK root to use, then query xcodebuild for it.
EXECUTE_PROCESS(COMMAND xcodebuild -version -sdk ${XCODE_IOS_PLATFORM} Path OUTPUT_VARIABLE CMAKE_OSX_SYSROOT ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
IF (NOT EXISTS ${CMAKE_OSX_SYSROOT})
    MESSAGE(FATAL_ERROR "Invalid CMAKE_OSX_SYSROOT: ${CMAKE_OSX_SYSROOT} does not exist.")
ENDIF()
MESSAGE (STATUS "Using SDK: ${CMAKE_OSX_SYSROOT} for platform: ${IOS_PLATFORM}")

# Get the SDK version information.
EXECUTE_PROCESS(COMMAND xcodebuild -sdk ${CMAKE_OSX_SYSROOT} -version SDKVersion OUTPUT_VARIABLE IOS_SDK_VERSION ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

# Find the Developer root for the specific iOS platform being compiled for from CMAKE_OSX_SYSROOT.  Should be ../../ from SDK
# specified in CMAKE_OSX_SYSROOT.  There does not appear to be a direct way to obtain this information from xcrun or xcodebuild.
IF (NOT CMAKE_IOS_DEVELOPER_ROOT)
    GET_FILENAME_COMPONENT(IOS_PLATFORM_SDK_DIR ${CMAKE_OSX_SYSROOT} PATH)
    GET_FILENAME_COMPONENT(CMAKE_IOS_DEVELOPER_ROOT ${IOS_PLATFORM_SDK_DIR} PATH)
ENDIF()

IF (NOT EXISTS ${CMAKE_IOS_DEVELOPER_ROOT})
    MESSAGE(FATAL_ERROR "Invalid CMAKE_IOS_DEVELOPER_ROOT: ${CMAKE_IOS_DEVELOPER_ROOT} does not exist.")
ENDIF()

# Find the C & C++ compilers for the specified SDK.
IF (NOT CMAKE_C_COMPILER)
    EXECUTE_PROCESS(COMMAND xcrun -sdk ${CMAKE_OSX_SYSROOT} -find clang OUTPUT_VARIABLE CMAKE_C_COMPILER ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
    MESSAGE(STATUS "Using C compiler: ${CMAKE_C_COMPILER}")
ENDIF()

IF (NOT CMAKE_CXX_COMPILER)
    EXECUTE_PROCESS(COMMAND xcrun -sdk ${CMAKE_OSX_SYSROOT} -find clang++ OUTPUT_VARIABLE CMAKE_CXX_COMPILER ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
    MESSAGE(STATUS "Using CXX compiler: ${CMAKE_CXX_COMPILER}")
ENDIF()

# Find the correct version of libtool.
EXECUTE_PROCESS(COMMAND xcrun -sdk ${CMAKE_OSX_SYSROOT} -find libtool OUTPUT_VARIABLE IOS_LIBTOOL ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
MESSAGE(STATUS "Using libtool: ${IOS_LIBTOOL}")

# REQUIRED: Configure libtool to be used instead of ar + ranlib to build static libraries.
SET(CMAKE_C_CREATE_STATIC_LIBRARY   "${IOS_LIBTOOL} -static -o <TARGET> <LINK_FLAGS> <OBJECTS> ")
SET(CMAKE_CXX_CREATE_STATIC_LIBRARY "${IOS_LIBTOOL} -static -o <TARGET> <LINK_FLAGS> <OBJECTS> ")

# Get the host OSX version.
EXECUTE_PROCESS(COMMAND uname -r OUTPUT_VARIABLE CMAKE_HOST_SYSTEM_VERSION ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

# Standard settings.
SET(CMAKE_SYSTEM_NAME       Darwin)
SET(CMAKE_SYSTEM_VERSION    ${IOS_SDK_VERSION})
SET(UNIX                    TRUE)
SET(APPLE                   TRUE)
SET(IOS                     TRUE)

# Required: force unset of OS X-specific deployment target (otherwise autopopulated)
SET(CMAKE_OSX_DEPLOYMENT_TARGET "" CACHE STRING "Must be empty for iOS builds." FORCE)

# Set up cross compilation flags
SET(CMAKE_C_COMPILER_FORCED                     TRUE)
SET(CMAKE_C_COMPILER_WORKS                      TRUE)
SET(CMAKE_C_OSX_COMPATIBILITY_VERSION_FLAG      "-compatibility_version ")
SET(CMAKE_C_OSX_CURRENT_VERSION_FLAG            "-current_version ")
SET(CMAKE_CXX_COMPILER_FORCED                   TRUE)
SET(CMAKE_CXX_COMPILER_WORKS                    TRUE)
SET(CMAKE_CXX_OSX_COMPATIBILITY_VERSION_FLAG    "${CMAKE_C_OSX_COMPATIBILITY_VERSION_FLAG}")
SET(CMAKE_CXX_OSX_CURRENT_VERSION_FLAG          "${CMAKE_C_OSX_CURRENT_VERSION_FLAG}")
SET(CMAKE_DL_LIBS                               "")
SET(CMAKE_FIND_FRAMEWORK FIRST                  ) # Default to searching for frameworks first.
SET(CMAKE_FIND_LIBRARY_SUFFIXES                 ".dylib" ".so" ".a")
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM ONLY      )
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY      )
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY      )
SET(CMAKE_MODULE_EXISTS                         1)
SET(CMAKE_OSX_ARCHITECTURES                     ${IOS_ARCH} CACHE STRING "Build architecture for iOS")
set(CMAKE_PLATFORM_HAS_INSTALLNAME              1)
SET(CMAKE_SHARED_LIBRARY_PREFIX                 "lib")
SET(CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS         "-dynamiclib -headerpad_max_install_names")
SET(CMAKE_SHARED_LIBRARY_SUFFIX                 ".dylib")
SET(CMAKE_SHARED_MODULE_CREATE_C_FLAGS          "-bundle -headerpad_max_install_names")
SET(CMAKE_SHARED_MODULE_LOADER_C_FLAG           "-Wl,-bundle_loader,")
SET(CMAKE_SHARED_MODULE_LOADER_CXX_FLAG         "-Wl,-bundle_loader,")
SET(CMAKE_SHARED_MODULE_PREFIX                  "lib")
SET(CMAKE_SHARED_MODULE_SUFFIX                  ".so")

IF (NOT DEFINED CMAKE_INSTALL_NAME_TOOL)
    FIND_PROGRAM(CMAKE_INSTALL_NAME_TOOL install_name_tool)
ENDIF (NOT DEFINED CMAKE_INSTALL_NAME_TOOL)

# Specify minimum version of deployment target. Unless specified, the latest SDK version is used by default.
SET(IOS_DEPLOYMENT_TARGET "${IOS_SDK_VERSION}" CACHE STRING "Minimum iOS version to build for." )
MESSAGE(STATUS "Building for minimum iOS version: ${IOS_DEPLOYMENT_TARGET} (SDK version: ${IOS_SDK_VERSION})")

# Xcode uses flags we can build directly from XCODE_IOS_PLATFORM.
SET(XCODE_IOS_PLATFORM_VERSION_FLAGS "-m${XCODE_IOS_PLATFORM}-version-min=${IOS_DEPLOYMENT_TARGET}")

# required flags for C++ on iOS.
SET(CMAKE_C_FLAGS           "${XCODE_IOS_PLATFORM_VERSION_FLAGS} -fobjc-abi-version=2 -fobjc-arc ${CMAKE_C_FLAGS}")
SET(CMAKE_C_LINK_FLAGS      "${XCODE_IOS_PLATFORM_VERSION_FLAGS} -Wl,-search_paths_first ${CMAKE_C_LINK_FLAGS}")
SET(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O3 -fomit-frame-pointer -ffast-math ${CMAKE_CXX_FLAGS_RELEASE}")
SET(CMAKE_CXX_LINK_FLAGS    "${XCODE_IOS_PLATFORM_VERSION_FLAGS} -Wl,-search_paths_first ${CMAKE_CXX_LINK_FLAGS}")

# In order to ensure that the updated compiler flags are used in try_compile() tests, we have to forcibly set them in the 
# CMake cache, not merely set them in the local scope.
LIST(APPEND VARS_TO_FORCE_IN_CACHE
    CMAKE_C_FLAGS
    CMAKE_C_LINK_FLAGS
    CMAKE_CXX_FLAGS
    CMAKE_CXX_LINK_FLAGS
    CMAKE_CXX_RELEASE)

FOREACH(VAR_TO_FORCE ${VARS_TO_FORCE_IN_CACHE})
    SET(${VAR_TO_FORCE} "${${VAR_TO_FORCE}}" CACHE STRING "" FORCE)
ENDFOREACH()

# Set the find root to the iOS developer roots and to user defined paths.
SET(CMAKE_FIND_ROOT_PATH ${CMAKE_IOS_DEVELOPER_ROOT} ${CMAKE_OSX_SYSROOT} ${CMAKE_PREFIX_PATH} CACHE string  "iOS find search path root" FORCE)

# Set up the default search directories for frameworks.
SET(CMAKE_SYSTEM_FRAMEWORK_PATH
    ${CMAKE_OSX_SYSROOT}/System/Library/Frameworks
    ${CMAKE_OSX_SYSROOT}/System/Library/PrivateFrameworks
    ${CMAKE_OSX_SYSROOT}/Developer/Library/Frameworks)