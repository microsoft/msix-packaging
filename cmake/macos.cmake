# This is a workaround while Xcode 12 is still in beta.

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
        message(FATAL_ERROR "arm64 is not supported for Xcode version ${XCODE_VERSION}")
    endif()

    # WARNING, this will be false once Xcode 12 is release. Currently there are two
    # Xcode-beta 12, build 12A8161k and build 12A6163b. The later does NOT support arm64
    if (NOT XCODE_BUILD MATCHES "12A8161k")
        message(FATAL_ERROR "arm64 is not supported for Xcode build ${XCODE_BUILD}")
    endif()

endif()
