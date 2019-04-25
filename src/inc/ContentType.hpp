//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
//  Helper file for determining content type and compression settings for adding files to a package
#pragma once

#include "AppxPackaging.hpp"

#include <map>

namespace MSIX {

    typedef std::pair<std::string, APPX_COMPRESSION_OPTION> ContentTypeData;

    class ContentType
    {
    public:
        static const ContentTypeData GetExtensionContentTypeData(std::string& ext);
        static const std::string GetPayloadFileContentType(APPX_FOOTPRINT_FILE_TYPE footprintFile);
    };
}
