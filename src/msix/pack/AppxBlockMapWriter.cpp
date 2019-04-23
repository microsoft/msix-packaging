//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 

#include "XmlWriter.hpp"
#include "AppxBlockMapWriter.hpp"

namespace MSIX {

    /*
      <BlockMap HashMethod="http://www.w3.org/2001/04/xmlenc#sha256" xmlns="http://schemas.microsoft.com/appx/2010/blockmap">
        <File Size="18944" Name="App1.exe" LfhSize="38">
          <Block Size="2948" Hash="ORIk+3QF9mSpuOq51oT3Xqn0Gy0vcGbnBRn5lBg5irM="/>
        </File>
        <File Size="1430" Name="Assets\LockScreenLogo.scale-200.png" LfhSize="65">
          <Block Hash="pBoFOz/DsMEJcgzNQ3oZclrpFj6nWZAiKhK1lrnHynY="/>
      </File>
      ...
    */
    static const char* blockMapElement = "BlockMap";
    static const char* hashMethodAttribute = "HashMethod";
    static const char* hashMethodAttributeValue = "http://www.w3.org/2001/04/xmlenc#sha256";
    static const char* blockMapNamespace = "http://schemas.microsoft.com/appx/2010/blockmap";
    static const char* fileElement = "File";
    static const char* sizeAttribute = "Size";
    static const char* nameAttribute = "Name";
    static const char* lfhSizeAttribute = "LfhSize";
    static const char* blockElement = "Block";
    static const char* hashAttribute = "Hash";

    // <BlockMap HashMethod="http://www.w3.org/2001/04/xmlenc#sha256" xmlns="http://schemas.microsoft.com/appx/2010/blockmap">
    BlockMapWriter::BlockMapWriter()
    {
        m_xmlWriter = std::make_unique<XmlWriter>(blockMapElement);
        // For now, we always use SHA256.
        m_xmlWriter->AddAttribute(hashMethodAttribute, hashMethodAttributeValue);
        m_xmlWriter->AddAttribute(xmlnsAttribute, blockMapNamespace);
    }

    // <File Size="18944" Name="App1.exe" LfhSize="38">
    void BlockMapWriter::AddFileElement(const std::string& name, std::uint64_t uncompressedSize, std::uint32_t lfh)
    {
        // TODO: might need sway \ to //. For the blockmap we always use the windows separator.
        m_xmlWriter->StartElement(fileElement);
        m_xmlWriter->AddAttribute(sizeAttribute, std::to_string(uncompressedSize));
        m_xmlWriter->AddAttribute(nameAttribute, name);
        m_xmlWriter->AddAttribute(lfhSizeAttribute, std::to_string(lfh));
    }

    // <Block Size="2948" Hash="ORIk+3QF9mSpuOq51oT3Xqn0Gy0vcGbnBRn5lBg5irM="/>
    void BlockMapWriter::AddBlockElement(const std::string& hash, std::size_t size)
    {
        m_xmlWriter->StartElement(blockElement);
        if(size != 0)
        {
            m_xmlWriter->AddAttribute(sizeAttribute, std::to_string(size));
        }
        m_xmlWriter->AddAttribute(hashAttribute, hash);
        m_xmlWriter->CloseElement();
    }

    void BlockMapWriter::CloseFileElement()
    {
        m_xmlWriter->CloseElement();
    }

    void BlockMapWriter::Close()
    {
        m_xmlWriter->CloseElement();
        ThrowErrorIf(Error::Unexpected, m_xmlWriter->GetState() != XmlWriter::Finish, "The blockmap didn't close correctly");
    }
}