# This is a check to fail arm64 build if the Xcode version is not adequate.

# Get xcode version and build.
execute_process(COMMAND xcodebuild -version OUTPUT_VARIABLE XCODE_OUTPUT ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REGEX MATCH "Xcode [0-9\\.]+" XCODE_VERSION "${XCODE_OUTPUT}")
string(REGEX REPLACE "Xcode ([0-9\\.]+)" "\\1" XCODE_VERSION "${XCODE_VERSION}")
string(REGEX MATCH "Build version [a-zA-Z0-9]+" XCODE_BUILD "${XCODE_OUTPUT}")
string(REGEX REPLACE "Build version ([a-zA-Z0-9]+)" "\\1" XCODE_BUILD "${XCODE_BUILD}")
message(STATUS "Using Xcode version: ${XCODE_VERSION}")
message(STATUS "Using Xcode build: ${XCODE_BUILD}")

if (CMAKE_OSX_ARCHITECTURES MATCHES "arm64")
    # arm64 is only supported for version xcode 12 and higher
    if (XCODE_VERSION LESS 12.0)
        # If you see this and you have Xcode-beta 12 do:
        # sudo xcode-select -switch /Applications/Xcode-beta.app/Contents/Developer
        message(FATAL_ERROR "arm64 is only supported on Xcode 12 12A8161k at this time. Found version ${XCODE_VERSION}. To enable arm64 builds please download Xcode beta 12 build 12A8161k and run `sudo xcode-select -switch /Applications/Xcode-beta.app/Contents/Developer`")
    endif()
endif()
