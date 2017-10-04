#pragma once

#if defined PLATFORM_APPLE
    #define XPLATAPPX_API
#elif defined PLATFORM_ANDROID
    #define XPLATAPPX_API
#elif defined PLATFORM_LINUX
    #define XPLATAPPX_API
#elif defined XPLATAPPX_API
    #define XPLATAPPX_API __declspec(dllexport)
#else
    #define XPLATAPPX_API __declspec(dllexport)
#endif

// TODO: add #defs to cover platform-specific differences?
XPLATAPPX_API unsigned int UnpackAppx (char* source, char* destination);
XPLATAPPX_API unsigned int PackAppx   (char* source, char* destination);
