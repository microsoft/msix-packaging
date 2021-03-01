//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 

#include "XmlWriter.hpp"
#include "AppxBlockMapWriter.hpp"
#include "StringHelper.hpp"

#include <vector>

namespace MSIX {

    /*
      <BlockMap HashMethod="http://www.w3.org/2001/04/xmlenc#sha256" xmlns="http://schemas.microsoft.com/appx/2010/blockmap" 
        xmlns:b4="http://schemas.microsoft.com/appx/2021/blockmap" IgnorableNamespaces="b4">
        <File Size="189440" Name="App1.exe" LfhSize="38">
          <Block Size="29408" Hash="ORIk+3QF9mSpuOq51oT3Xqn0Gy0vcGbnBRn5lBg5irM="/>
          <Block Size="49487" Hash="BRIk+3QF9mSpuOq51oT3Xqn0jy0vcGbnBRn5lBg5irM="/>
          <Block Size="1487" Hash="CDIk+3QF9mSpuOq51oT3Xqn0jy0vcGbnBRn5lBg5iui="/>
          <b4:FileHash Hash="77hl7hZclsGViUCfuMMqsxsRNJW+PVnNblygNB2vxgE="/>
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
    static const char* blockMapNamespaceV4 = "http://schemas.microsoft.com/appx/2021/blockmap";
    static const char* v4NamespacePrefix = "b4";
    static const char* xmlnsAttributeV4 = "xmlns:b4";
    static const char* ignorableNsAttribute = "IgnorableNamespaces";

    static const char* fileElement = "File";
    static const char* fileHashElementV4 = "b4:FileHash";
    static const char* sizeAttribute = "Size";
    static const char* nameAttribute = "Name";
    static const char* lfhSizeAttribute = "LfhSize";
    static const char* blockElement = "Block";
    static const char* hashAttribute = "Hash";

    // <BlockMap HashMethod="http://www.w3.org/2001/04/xmlenc#sha256" xmlns="http://schemas.microsoft.com/appx/2010/blockmap" 
    //   xmlns:b4 = "http://schemas.microsoft.com/appx/2021/blockmap" IgnorableNamespaces = "b4">
    BlockMapWriter::BlockMapWriter() : m_xmlWriter(XmlWriter(blockMapElement))
    {
        m_xmlWriter.AddAttribute(xmlnsAttribute, blockMapNamespace);
        m_xmlWriter.AddAttribute(xmlnsAttributeV4, blockMapNamespaceV4);
        m_xmlWriter.AddAttribute(ignorableNsAttribute, v4NamespacePrefix);

        // For now, we always use SHA256.
        m_xmlWriter.AddAttribute(hashMethodAttribute, hashMethodAttributeValue);
    }

    // <File Size="18944" Name="App1.exe" LfhSize="38">
    void BlockMapWriter::AddFile(const std::string& name, std::uint64_t uncompressedSize, std::uint32_t lfh)
    {
        // For the blockmap we always use the windows separator.
        std::string winName = Helper::toBackSlash(name);
        m_xmlWriter.StartElement(fileElement);
        m_xmlWriter.AddAttribute(nameAttribute, winName);
        m_xmlWriter.AddAttribute(sizeAttribute, std::to_string(uncompressedSize));
        m_xmlWriter.AddAttribute(lfhSizeAttribute, std::to_string(lfh));

        if (uncompressedSize > DefaultBlockSize)
        {
            // If the file size is more than a block (64KB), we will add <FileHash> element after all the <Block> elements.
            // Otherwise, file hash is the same as the block hash, as there is only 1 block in the file.
            m_fileHashEngine.Reset();
            m_addFileHash = true;
        }
        else
        {
            m_addFileHash = false;
        }
    }

    // <Block Size="2948" Hash="ORIk+3QF9mSpuOq51oT3Xqn0Gy0vcGbnBRn5lBg5irM="/>
    void BlockMapWriter::AddBlock(const std::vector<std::uint8_t>& block, ULONG size, bool isCompressed)
    {
        // hash block
        std::vector<std::uint8_t> hash;
        ThrowErrorIfNot(MSIX::Error::BlockMapInvalidData,
            MSIX::SHA256::ComputeHash(block.data(), static_cast<uint32_t>(block.size()), hash), 
            "Failed computing hash");

        m_xmlWriter.StartElement(blockElement);
        m_xmlWriter.AddAttribute(hashAttribute, Base64::ComputeBase64(hash));
        // We only add the size attribute for compressed files, we cannot just check for the 
        // size of the block because the last block is going to be smaller than the default.
        if(isCompressed)
        {
            m_xmlWriter.AddAttribute(sizeAttribute, std::to_string(size));
        }
        m_xmlWriter.CloseElement();

        if (m_addFileHash)
        {
            m_fileHashEngine.HashData(block.data(), static_cast<uint32_t>(block.size()));
        }
    }

    void BlockMapWriter::CloseFile()
    {
        if (m_addFileHash)
        {
            std::vector<std::uint8_t> hash;
            m_fileHashEngine.FinalizeAndGetHashValue(hash);

            // <b4:FileHash Hash="4EsIP4hU04SShLPR1KIiRBzuYpLVPcETqMp1HZaKdfc="/>
            m_xmlWriter.StartElement(fileHashElementV4);
            m_xmlWriter.AddAttribute(hashAttribute, Base64::ComputeBase64(hash));
            m_xmlWriter.CloseElement();
        }

        m_xmlWriter.CloseElement();
    }

    void BlockMapWriter::Close()
    {
        m_xmlWriter.CloseElement();
        ThrowErrorIf(Error::Unexpected, m_xmlWriter.GetState() != XmlWriter::Finish, "The blockmap didn't close correctly");
    }
}
