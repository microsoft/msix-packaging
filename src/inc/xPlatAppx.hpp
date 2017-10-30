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

enum xPlatPackUnpackOptions : unsigned long
{
    xPlatPackUnpackOptionsNone                      = 0x00000000,
    xPlatPackUnpackOptionsCreatePackageSubfolder    = 0x00000001
};

// TODO: add #defs to cover platform-specific differences?
XPLATAPPX_API unsigned int XPLATAPPX_CONVENTION UnpackAppx (
    xPlatPackUnpackOptions packUnpackOptions, 
    APPX_VALIDATION_OPTION validationOptions,
    char* source,
    char* destination
);

XPLATAPPX_API unsigned int XPLATAPPX_CONVENTION PackAppx   (
    xPlatPackUnpackOptions packUnpackOptions, 
    APPX_VALIDATION_OPTION validationOptions, 
    char* source, 
    char* certFile, 
    char* destination);

XPLATAPPX_API unsigned int XPLATAPPX_CONVENTION ValidateAppxSignature(char* appx);
