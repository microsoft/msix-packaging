//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#include "catch.hpp"
#include "msixtest_int.hpp"

namespace MsixTest {
    namespace XML
    {
        void ValidateIMsixMSXMLDocument(const ComPtr<IAppxManifestReader>& manifestReader)
        {
            // Getting this interface should always succeed
            ComPtr<IMsixMSXMLDocument> msixMSXMLDoc = manifestReader.As<IMsixMSXMLDocument>();

            IXMLDOMDocument* xmlDom = nullptr;
            REQUIRE_HR(static_cast<HRESULT>(MSIX::Error::NoInterface), msixMSXMLDoc->GetDocument(&xmlDom));
        }
    }
}
