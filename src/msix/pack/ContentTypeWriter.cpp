//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 

#include "XmlWriter.hpp"
#include "ContentTypeWriter.hpp"
#include "Encoding.hpp"

#include <map>
#include <algorithm>

namespace MSIX {

    /*
    <Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
      <Default ContentType="image/png" Extension="png"/>
      <Default ContentType="application/x-msdownload" Extension="dll"/>
      <Default ContentType="application/vnd.ms-appx.manifest+xml" Extension="xml"/>
      <Override ContentType="application/vnd.ms-appx.blockmap+xml" PartName="/AppxBlockMap.xml"/>
      <Override ContentType="application/vnd.ms-appx.signature" PartName="/AppxSignature.p7x"/>
      <Override ContentType="application/vnd.ms-pkiseccat" PartName="/AppxMetadata/CodeIntegrity.cat"/>
    </Types>
    */

    static const char* typesElement = "Types";
    static const char* typesNamespace = "http://schemas.openxmlformats.org/package/2006/content-types";
    static const char* defaultElement = "Default";
    static const char* contentTypeAttribute = "ContentType";
    static const char* extensionAttribute = "Extension";
    static const char* overrideElement = "Override";
    static const char* partNameAttribute = "PartName";

    // <Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
    ContentTypeWriter::ContentTypeWriter() : m_xmlWriter(XmlWriter(typesElement, true))
    {
        m_xmlWriter.AddAttribute(xmlnsAttribute, typesNamespace);
    }

    // File extension to MIME value map that are added as default elements
    // If the extension is already in the map and its content type is different or
    // if the file doesn't have an extensions AddOverride is called.
    void ContentTypeWriter::AddContentType(const std::string& name, const std::string& contentType, bool forceOverride)
    {
        auto percentageEncodedName = Encoding::EncodeFileName(name);

        auto lastDir = percentageEncodedName;
        auto lastSlash = lastDir.find_last_of("/");
        if (lastSlash != std::string::npos)
        {
            lastDir = lastDir.substr(lastSlash + 1);
        }

        if (forceOverride)
        {
            AddOverride(percentageEncodedName, contentType);
            return;
        }

        auto findLastPeriod = lastDir.find_last_of(".");
        if (findLastPeriod != std::string::npos)
        {
            // See if already exist
            std::string ext = lastDir.substr(findLastPeriod + 1);
            std::string normalizedExt = ext;
            std::transform(normalizedExt.begin(), normalizedExt.end(), normalizedExt.begin(), ::tolower);
            auto find = m_defaultExtensions.find(normalizedExt);
            if (find != m_defaultExtensions.end())
            {
                if (find->second != contentType)
                {
                    // The extension is in the table but with a different content type
                    AddOverride(percentageEncodedName, contentType);
                }
            }
            else
            {
                auto result = m_defaultExtensions.emplace(normalizedExt, contentType);
                AddDefault(ext, contentType);
            }
        }
        else
        {
            AddOverride(percentageEncodedName, contentType);
        }
    }

    void ContentTypeWriter::Close()
    {
        m_xmlWriter.CloseElement();
        ThrowErrorIf(Error::Unexpected, m_xmlWriter.GetState() != XmlWriter::Finish, "Content Type xml didn't close correctly");
    }

    // <Default ContentType="application/vnd.ms-appx.manifest+xml" Extension="xml"/>
    void ContentTypeWriter::AddDefault(const std::string& ext, const std::string& contentType)
    {
        m_xmlWriter.StartElement(defaultElement);
        m_xmlWriter.AddAttribute(contentTypeAttribute, contentType);
        m_xmlWriter.AddAttribute(extensionAttribute, ext);
        m_xmlWriter.CloseElement();
    }

    // <Override ContentType="application/vnd.ms-appx.signature" PartName="/AppxSignature.p7x"/>
    void ContentTypeWriter::AddOverride(const std::string& file, const std::string& contentType)
    {
        std::string partName = "/" + file;
        m_xmlWriter.StartElement(overrideElement);
        m_xmlWriter.AddAttribute(contentTypeAttribute, contentType);
        m_xmlWriter.AddAttribute(partNameAttribute, partName);
        m_xmlWriter.CloseElement();
    }
}