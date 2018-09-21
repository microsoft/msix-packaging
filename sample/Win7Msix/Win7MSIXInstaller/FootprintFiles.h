#ifndef FOOTPRINTFILES_H
#define FOOTPRINTFILES_H

#include "AppxPackaging.hpp"

// Describes the FootprintFilesType structure
template<typename Type>
struct FootprintFilesType
{
	Type fileType;
	const char* description;
	bool isRequired;
};

// Types of footprint files in an app package
const int FootprintFilesCount = 4;
__declspec(selectany) FootprintFilesType<APPX_FOOTPRINT_FILE_TYPE> g_footprintFilesType[FootprintFilesCount] =
{
	{ APPX_FOOTPRINT_FILE_TYPE_MANIFEST, "manifest", true },
	{ APPX_FOOTPRINT_FILE_TYPE_BLOCKMAP, "block map", true },
	{ APPX_FOOTPRINT_FILE_TYPE_SIGNATURE, "digital signature", true },
	{ APPX_FOOTPRINT_FILE_TYPE_CODEINTEGRITY, "CI catalog", false }, // this is ONLY required if there exists 1+ PEs
};

#endif