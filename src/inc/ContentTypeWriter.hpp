//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once

#include "AppxPackaging.hpp"
#include "XmlWriter.hpp"
#include "ComHelper.hpp"

#include <map>

namespace MSIX {

    class ContentTypeWriter final
    {
    public:
        ContentTypeWriter();
        ~ContentTypeWriter() {}

        void AddContentType(const std::string& name, const std::string& contentType, bool forceOverride = false);
        void Close();
        ComPtr<IStream> GetStream() { return m_xmlWriter->GetStream(); }

    protected:
        void AddDefault(const std::string& ext, const std::string& contentType);
        void AddOverride(const std::string& file, const std::string& contentType);

        std::map<std::string, std::string> m_defaultExtensions; // ext to contentType map
        std::unique_ptr<XmlWriter> m_xmlWriter;
    };
}