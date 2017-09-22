#pragma once

#ifdef XPLATAPPX_API
#define XPLATAPPX_API __declspec(dllexport)
#else  
#define XPLATAPPX_API __declspec(dllimport)
#endif 

// TODO: add #defs to cover platform-specific differences?
XPLATAPPX_API unsigned int UnpackAppx (char* source, char* destination);
XPLATAPPX_API unsigned int PackAppx   (char* source, char* destination);