#ifndef FOOTPRINTFILES_H
#define FOOTPRINTFILES_H

#include "AppxPackaging.hpp"
namespace MsixCoreLib
{
// Describes the FootprintFilesType structure
template<typename Type>
struct FootprintFilesType
{
	Type fileType;
	const char* description;
	bool isRequired;
};

// Types of footprint files in an app package
__declspec(selectany) FootprintFilesType<APPX_FOOTPRINT_FILE_TYPE> g_footprintFilesType[] =
{
	{ APPX_FOOTPRINT_FILE_TYPE_MANIFEST, "manifest", true },
	{ APPX_FOOTPRINT_FILE_TYPE_BLOCKMAP, "block map", true },
	{ APPX_FOOTPRINT_FILE_TYPE_SIGNATURE, "digital signature", true },
	{ APPX_FOOTPRINT_FILE_TYPE_CODEINTEGRITY, "CI catalog", false }, // this is ONLY required if there exists 1+ PEs
};
const int FootprintFilesCount = ARRAYSIZE(g_footprintFilesType);
#endif
}