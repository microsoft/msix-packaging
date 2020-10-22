//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once

#include "XmlWriter.hpp"
#include "ComHelper.hpp"

#include <vector>

namespace MSIX {

    class BundleManifestWriter final
    {
    public:
        BundleManifestWriter();

        HRESULT StartBundleManifest(std::string bundleName, std::string bundlePublisher, UINT64 bundleVersion);
        HRESULT AddPackage(/*PackageInfo* packageInfo*/);
        HRESULT AddOptionalBundle(/*OptionalBundleInfo* bundleInfo*/);
        HRESULT EndBundleManifest();
        HRESULT InitializeWriter();

        /*void AddFile(const std::string& name, std::uint64_t uncompressedSize, std::uint32_t lfh);
        void AddBlock(const std::vector<std::uint8_t>& block, ULONG size, bool isCompressed);
        void CloseFile();
        void Close();*/
        ComPtr<IStream> GetStream() { return m_xmlWriter.GetStream(); }

    protected:
        XmlWriter m_xmlWriter;
    };
}