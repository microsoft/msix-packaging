//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once

#include "AppxPackaging.hpp"
#include "XmlWriter.hpp"
#include "ComHelper.hpp"

#include <map>
#include <string>

namespace MSIX {

    class ContentTypeWriter final
    {
    public:
        ContentTypeWriter();

        // Used for editing an existing content type file, but only in the very specific case of signing.
        // Creates a copy and sets the cursor to the end of the existing elements stream.
        ContentTypeWriter(IStream* stream);

        void AddContentType(const std::string& name, const std::string& contentType, bool forceOverride = false);
        void Close();
        ComPtr<IStream> GetStream() { return m_xmlWriter.GetStream(); }

    protected:
        void AddDefault(const std::string& ext, const std::string& contentType);
        void AddOverride(const std::string& file, const std::string& contentType);

        static std::string GetPartNameSearchString(const std::string& fileName);

        std::map<std::string, std::string> m_defaultExtensions;
        XmlWriter m_xmlWriter;

        // For the signing scenario, we need to know if the signature files are already present.
        bool m_hasSignatureOverride = false;
        bool m_hasCIOverride = false;
    };
}