#pragma once

#if defined PLATFORM_APPLE
    #define XPLATAPPX_API
#elif defined PLATFORM_ANDROID
    #define XPLATAPPX_API
#elif defined PLATFORM_LINUX
    #define XPLATAPPX_API
#elif defined XPLATAPPX_API
    #undef XPLATAPPX_API
    #define XPLATAPPX_API __declspec(dllexport)
#else
    #define XPLATAPPX_API __declspec(dllexport)
#endif

#include "AppxPackaging.hpp"

enum xPlatValidationOptions : unsigned long
{
    xPlatValidationOptionFull                       = 0x00000000,
    xPlatValidationOptionSkipSignatureOrigin        = 0x00000001,
    xPlatValidationOptionSkipAppxManifestValidation = 0x00000002,
};

enum xPlatPackUnpackOptions : unsigned long
{
    xPlatPackUnpackOptionsNone                      = 0x00000000,
    xPlatPackUnpackOptionsCreatePackageSubfolder    = 0x00000001
};

// TODO: add #defs to cover platform-specific differences?
XPLATAPPX_API unsigned int UnpackAppx (
    xPlatPackUnpackOptions packUnpackOptions, 
    xPlatValidationOptions validationOptions,
    char* source,
    char* destination
);

XPLATAPPX_API unsigned int PackAppx   (
    xPlatPackUnpackOptions packUnpackOptions, 
    xPlatValidationOptions validationOptions, 
    char* source, 
    char* certFile, 
    char* destination);
