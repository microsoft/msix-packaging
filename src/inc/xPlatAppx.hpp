#pragma once

#if defined WIN32
    #undef XPLATAPPX_API
    #define XPLATAPPX_API extern "C" __declspec(dllexport) 
    #define XPLATAPPX_CONVENTION __cdecl
#else
    #define XPLATAPPX_API
    #define XPLATAPPX_CONVENTION
#endif

#include "AppxPackaging.hpp"

enum xPlatValidationOptions : unsigned long
{
    xPlatValidationOptionFull                       = 0x00000000,
    xPlatValidationOptionSkipSignature              = 0x00000001,
    xPlatValidationOptionSkipSignatureOrigin        = 0x00000002,
    xPlatValidationOptionSkipAppxManifestValidation = 0x00000004,
};

enum xPlatPackUnpackOptions : unsigned long
{
    xPlatPackUnpackOptionsNone                      = 0x00000000,
    xPlatPackUnpackOptionsCreatePackageSubfolder    = 0x00000001
};

// TODO: add #defs to cover platform-specific differences?
XPLATAPPX_API unsigned int XPLATAPPX_CONVENTION UnpackAppx (
    xPlatPackUnpackOptions packUnpackOptions, 
    xPlatValidationOptions validationOptions,
    char* source,
    char* destination
);

XPLATAPPX_API unsigned int XPLATAPPX_CONVENTION PackAppx   (
    xPlatPackUnpackOptions packUnpackOptions, 
    xPlatValidationOptions validationOptions, 
    char* source, 
    char* certFile, 
    char* destination);
