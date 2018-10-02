//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once

#include <stdint.h>
#include <stddef.h>

namespace MSIX
{
    class NSXmlParserWrapper
    {
        void* wrapped;
    public:
        NSXmlParserWrapper() = default;
        ~NSXmlParserWrapper() = default;
        bool Parse(uint8_t * data, size_t length, void* xmlDocumentReader);
    };
}