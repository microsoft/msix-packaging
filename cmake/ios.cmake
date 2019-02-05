# iOS CMAKE toolchain.
# Pieced together via ROME ios.cmake and LOTS of time searching through CMAKE documentation and various examples of other ios.cmake's
# out in the wild.  Tested with xCode 9 and on MacOS 10.12
execute_process(COMMAND xcodebuild -version OUTPUT_VARIABLE XCODE_VERSION ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REGEX MATCH "Xcode [0-9\\.]+" XCODE_VERSION "${XCODE_VERSION}")
string(REGEX REPLACE "Xcode ([0-9\\.]+)" "\\1" XCODE_VERSION "${XCODE_VERSION}")
message(STATUS "Using Xcode version: ${XCODE_VERSION}")

# Default to building for iPhoneOS if not specified otherwise as we cannot determine the platform from the CMAKE_OSX_ARCHITECTURES 
# variable.  Use of CMAKE_OSX_ARCHITECTURES is such that try_compile can correctly determine the value of IOS_PLATFORM from the root 
# project, as CMAKE_OSX_ARCHITECTURES is propagated.
if (NOT DEFINED IOS_PLATFORM)
    if (CMAKE_OSX_ARCHITECTURES)
        if (CMAKE_OSX_ARCHITECTURES MATCHES ".*arm.*")
            set(IOS_PLATFORM "OS")
            set(IOS_ARCH ${CMAKE_OSX_ARCHITECTURES})
        elseif (CMAKE_OSX_ARCHITECTURES MATCHES "i386")
            set(IOS_PLATFORM "SIMULATOR")
        elseif (CMAKE_OSX_ARCHITECTURES MATCHES "x86_64")
            set(IOS_PLATFORM "SIMULATOR64")
        endif()
    endif()

    if (NOT IOS_PLATFORM)
        set(IOS_PLATFORM "OS")
    endif()
endif()
set(IOS_PLATFORM ${IOS_PLATFORM} CACHE string "Type of iOS platform for which to build.")

# Determine the platform name and architectures for use in xcodebuild commands from the specified IOS_PLATFORM name.
if (IOS_PLATFORM STREQUAL "OS")
    set(XCODE_IOS_PLATFORM iphoneos)
    #set(IOS_ARCH armv7 armv7s arm64)
elseif (IOS_PLATFORM STREQUAL "SIMULATOR")
    set(XCODE_IOS_PLATFORM iphonesimulator)
    set(IOS_ARCH i386)
elseif(IOS_PLATFORM STREQUAL "SIMULATOR64")
    set(XCODE_IOS_PLATFORM iphonesimulator)
    set(IOS_ARCH x86_64)
else()
    message(FATAL_ERROR "Invalid IOS_PLATFORM: ${IOS_PLATFORM}")
endif()
message(STATUS "Configuring iOS build for platform: ${IOS_PLATFORM}, architecture(s): ${IOS_ARCH}")

# If user did not specify the SDK root to use, then query xcodebuild for it.
execute_process(COMMAND xcodebuild -version -sdk ${XCODE_IOS_PLATFORM} Path OUTPUT_VARIABLE CMAKE_OSX_SYSROOT ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
if (NOT EXISTS ${CMAKE_OSX_SYSROOT})
    message(FATAL_ERROR "Invalid CMAKE_OSX_SYSROOT: ${CMAKE_OSX_SYSROOT} does not exist.")
endif()
message (STATUS "Using SDK: ${CMAKE_OSX_SYSROOT} for platform: ${IOS_PLATFORM}")

# Get the SDK version information.
execute_process(COMMAND xcodebuild -sdk ${CMAKE_OSX_SYSROOT} -version SDKVersion OUTPUT_VARIABLE IOS_SDK_VERSION ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

# Find the Developer root for the specific iOS platform being compiled for from CMAKE_OSX_SYSROOT.  Should be ../../ from SDK
# specified in CMAKE_OSX_SYSROOT.  There does not appear to be a direct way to obtain this information from xcrun or xcodebuild.
if (NOT CMAKE_IOS_DEVELOPER_ROOT)
    GET_FILENAME_COMPONENT(IOS_PLATFORM_SDK_DIR ${CMAKE_OSX_SYSROOT} PATH)
    GET_FILENAME_COMPONENT(CMAKE_IOS_DEVELOPER_ROOT ${IOS_PLATFORM_SDK_DIR} PATH)
endif()

if (NOT EXISTS ${CMAKE_IOS_DEVELOPER_ROOT})
    message(FATAL_ERROR "Invalid CMAKE_IOS_DEVELOPER_ROOT: ${CMAKE_IOS_DEVELOPER_ROOT} does not exist.")
endif()

# Find the C & C++ compilers for the specified SDK.
if (NOT CMAKE_C_COMPILER)
    execute_process(COMMAND xcrun -sdk ${CMAKE_OSX_SYSROOT} -find clang OUTPUT_VARIABLE CMAKE_C_COMPILER ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
    message(STATUS "Using C compiler: ${CMAKE_C_COMPILER}")
endif()

if (NOT CMAKE_CXX_COMPILER)
    execute_process(COMMAND xcrun -sdk ${CMAKE_OSX_SYSROOT} -find clang++ OUTPUT_VARIABLE CMAKE_CXX_COMPILER ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
    message(STATUS "Using CXX compiler: ${CMAKE_CXX_COMPILER}")
endif()

# Find the correct version of libtool.
execute_process(COMMAND xcrun -sdk ${CMAKE_OSX_SYSROOT} -find libtool OUTPUT_VARIABLE IOS_LIBTOOL ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
message(STATUS "Using libtool: ${IOS_LIBTOOL}")

# REQUIRED: Configure libtool to be used instead of ar + ranlib to build static libraries.
set(CMAKE_C_CREATE_STATIC_LIBRARY   "${IOS_LIBTOOL} -static -o <TARGET> <LINK_FLAGS> <OBJECTS> ")
set(CMAKE_CXX_CREATE_STATIC_LIBRARY "${IOS_LIBTOOL} -static -o <TARGET> <LINK_FLAGS> <OBJECTS> ")

# Get the host OSX version.
execute_process(COMMAND uname -r OUTPUT_VARIABLE CMAKE_HOST_SYSTEM_VERSION ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

# Standard settings.
set(CMAKE_SYSTEM_NAME       Darwin)
set(CMAKE_SYSTEM_VERSION    ${IOS_SDK_VERSION})
set(UNIX                    TRUE)
set(APPLE                   TRUE)
set(IOS                     TRUE)

# Required: force unset of OS X-specific deployment target (otherwise autopopulated)
set(CMAKE_OSX_DEPLOYMENT_TARGET "" CACHE string "Must be empty for iOS builds." FORCE)

# Set up cross compilation flags
set(CMAKE_C_COMPILER_FORCED                     TRUE)
set(CMAKE_C_COMPILER_WORKS                      TRUE)
set(CMAKE_C_OSX_COMPATIBILITY_VERSION_FLAG      "-compatibility_version ")
set(CMAKE_C_OSX_CURRENT_VERSION_FLAG            "-current_version ")
set(CMAKE_CXX_COMPILER_FORCED                   TRUE)
set(CMAKE_CXX_COMPILER_WORKS                    TRUE)
set(CMAKE_CXX_OSX_COMPATIBILITY_VERSION_FLAG    "${CMAKE_C_OSX_COMPATIBILITY_VERSION_FLAG}")
set(CMAKE_CXX_OSX_CURRENT_VERSION_FLAG          "${CMAKE_C_OSX_CURRENT_VERSION_FLAG}")
set(CMAKE_DL_LIBS                               "")
set(CMAKE_FIND_FRAMEWORK FIRST                  ) # Default to searching for frameworks first.
set(CMAKE_FIND_LIBRARY_SUFFIXES                 ".dylib" ".so" ".a")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM BOTH      ) # Both to be able to find Git
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY      )
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY      )
set(CMAKE_MODULE_EXISTS                         1)
set(CMAKE_OSX_ARCHITECTURES                     ${IOS_ARCH} CACHE string "Build architecture for iOS")
set(CMAKE_PLATFORM_HAS_INSTALLNAME              1)
set(CMAKE_SHARED_LIBRARY_PREFIX                 "lib")
set(CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS         "-dynamiclib -headerpad_max_install_names")
set(CMAKE_SHARED_LIBRARY_SUFFIX                 ".dylib")
set(CMAKE_SHARED_MODULE_CREATE_C_FLAGS          "-bundle -headerpad_max_install_names")
set(CMAKE_SHARED_MODULE_LOADER_C_FLAG           "-Wl,-bundle_loader,")
set(CMAKE_SHARED_MODULE_LOADER_CXX_FLAG         "-Wl,-bundle_loader,")
set(CMAKE_SHARED_MODULE_PREFIX                  "lib")
set(CMAKE_SHARED_MODULE_SUFFIX                  ".so")

if (NOT DEFINED CMAKE_INSTALL_NAME_TOOL)
    find_program(CMAKE_INSTALL_NAME_TOOL install_name_tool)
endif (NOT DEFINED CMAKE_INSTALL_NAME_TOOL)

# Specify minimum version of deployment target. Unless specified, the latest SDK version is used by default.
set(IOS_DEPLOYMENT_TARGET "${IOS_SDK_VERSION}" CACHE string "Minimum iOS version to build for." )
message(STATUS "Building for minimum iOS version: ${IOS_DEPLOYMENT_TARGET} (SDK version: ${IOS_SDK_VERSION})")

# Xcode uses flags we can build directly from XCODE_IOS_PLATFORM.
set(XCODE_IOS_PLATFORM_VERSION_FLAGS "-m${XCODE_IOS_PLATFORM}-version-min=${IOS_DEPLOYMENT_TARGET}")

# required flags for C++ on iOS.
set(CMAKE_C_FLAGS           "${XCODE_IOS_PLATFORM_VERSION_FLAGS} -fobjc-abi-version=2 -fobjc-arc ${CMAKE_C_FLAGS}")
set(CMAKE_C_LINK_FLAGS      "${XCODE_IOS_PLATFORM_VERSION_FLAGS} -Wl,-search_paths_first ${CMAKE_C_LINK_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O3 -fomit-frame-pointer -ffast-math ${CMAKE_CXX_FLAGS_RELEASE}")
set(CMAKE_CXX_LINK_FLAGS    "${XCODE_IOS_PLATFORM_VERSION_FLAGS} -Wl,-search_paths_first ${CMAKE_CXX_LINK_FLAGS}")

# In order to ensure that the updated compiler flags are used in try_compile() tests, we have to forcibly set them in the 
# CMake cache, not merely set them in the local scope.
list(APPEND VARS_TO_FORCE_IN_CACHE
    CMAKE_C_FLAGS
    CMAKE_C_LINK_FLAGS
    CMAKE_CXX_FLAGS
    CMAKE_CXX_LINK_FLAGS
    CMAKE_CXX_RELEASE)

foreach(VAR_TO_FORCE ${VARS_TO_FORCE_IN_CACHE})
    set(${VAR_TO_FORCE} "${${VAR_TO_FORCE}}" CACHE string "" FORCE)
endforeach()

# Set the find root to the iOS developer roots and to user defined paths.
set(CMAKE_FIND_ROOT_PATH ${CMAKE_IOS_DEVELOPER_ROOT} ${CMAKE_OSX_SYSROOT} ${CMAKE_PREFIX_PATH} CACHE string  "iOS find search path root" FORCE)

# Set up the default search directories for frameworks.
set(CMAKE_SYSTEM_FRAMEWORK_PATH
    ${CMAKE_OSX_SYSROOT}/System/Library/Frameworks
    ${CMAKE_OSX_SYSROOT}/System/Library/PrivateFrameworks
    ${CMAKE_OSX_SYSROOT}/Developer/Library/Frameworks)