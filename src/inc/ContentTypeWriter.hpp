//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once

#include "AppxPackaging.hpp"
#include "XmlWriter.hpp"
#include "ComHelper.hpp"

#include <set>

namespace MSIX {

    class ContentTypeWriter final
    {
    public:
        typedef enum
        {
            BlockMap = 1,
            AppxManifest = 2,
            AppxSignature = 3,
        }
        PayloadFile;

        ContentTypeWriter();
        ~ContentTypeWriter() {}

        APPX_COMPRESSION_OPTION AddDefault(const std::string& name);
        void AddPayloadFile(ContentTypeWriter::PayloadFile payloadFile);
        void Close();
        ComPtr<IStream> GetStream() { return m_xmlWriter->GetStream(); }

    protected:
        std::set<std::string> m_extensions;
        std::unique_ptr<XmlWriter> m_xmlWriter;
    };
}