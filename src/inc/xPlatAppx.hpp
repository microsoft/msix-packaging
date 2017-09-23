#pragma once

#ifdef PLATFORM_APPLE
    #define XPLATAPPX_API

#else /*PLATFORM_MAC*/
    #ifdef XPLATAPPX_API
        #define XPLATAPPX_API __declspec(dllexport)
    #else /*XPLATAPPX_API*/
        #define XPLATAPPX_API __declspec(dllimport)
    #endif /*XPLATAPPX_API*/
#endif /*PLATFORM_MAC*/

// TODO: add #defs to cover platform-specific differences?
XPLATAPPX_API unsigned int UnpackAppx (char* source, char* destination);
XPLATAPPX_API unsigned int PackAppx   (char* source, char* destination);
