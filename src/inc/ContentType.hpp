//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
//  Helper file for determining content type and compression settings for adding files to a package
#pragma once

#include "AppxPackaging.hpp"

#include <map>

namespace MSIX {

    class ContentType
    {
    public:
        ContentType(std::string contentType, APPX_COMPRESSION_OPTION compressionOpt) :
            m_contentType(contentType), m_compressionOpt(compressionOpt)
        {}

        const std::string& GetContentType() { return m_contentType; }
        APPX_COMPRESSION_OPTION GetCompressionOpt() { return m_compressionOpt; }

        static const ContentType& GetContentTypeByExtension(std::string& ext);
        static const std::string GetPayloadFileContentType(APPX_FOOTPRINT_FILE_TYPE footprintFile);
    
    private:
        APPX_COMPRESSION_OPTION m_compressionOpt;
        std::string m_contentType;
    };
}
