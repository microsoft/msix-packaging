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

        void AddContentType(const std::string& name, const std::string& contentType, bool forceOverride = false);
        void Close();
        ComPtr<IStream> GetStream() { return m_xmlWriter.GetStream(); }

    protected:
        void AddDefault(const std::string& ext, const std::string& contentType);
        void AddOverride(const std::string& file, const std::string& contentType);

        // File extension to MIME value map that are added as default elements
        // If the extension is already in the map and its content type is different,
        // AddOverride is called.
        std::map<std::string, std::string> m_defaultExtensions;
        XmlWriter m_xmlWriter;
    };
}