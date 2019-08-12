//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#include "catch.hpp"
#include "msixtest_int.hpp"

namespace MsixTest {
    namespace XML
    {
        // Copied from product
        class Bstr
        {
            BSTR m_bstr;
        public:
            operator BSTR() && = delete;
            operator BSTR() & { return m_bstr; }
            Bstr() { m_bstr = nullptr; }
            Bstr(std::wstring text)
            {
                m_bstr = ::SysAllocStringLen(text.c_str(), static_cast<UINT>(text.length()));
                REQUIRE(m_bstr != nullptr);
            }
            ~Bstr() { ::SysFreeString(m_bstr); }

            BSTR* AddressOf()
            {
                ::SysFreeString(m_bstr);
                return &m_bstr;
            }

            BSTR& Get() { return m_bstr; }
        };

        void ValidateIMsixMSXMLDocument(const ComPtr<IAppxManifestReader>& manifestReader)
        {
            // Getting this interface should always succeed
            ComPtr<IMsixMSXMLDocument> msixMSXMLDoc = manifestReader.As<IMsixMSXMLDocument>();

            ComPtr<IXMLDOMDocument> xmlDom;
            REQUIRE_SUCCEEDED(msixMSXMLDoc->GetDocument(&xmlDom));

            ComPtr<IXMLDOMElement> xmlDomElement;
            REQUIRE_SUCCEEDED(xmlDom->get_documentElement(&xmlDomElement));

            Bstr name;
            REQUIRE_SUCCEEDED(xmlDomElement->get_nodeName(name.AddressOf()));
            CHECK(name.Get() == std::wstring(L"Package"));
        }
    }
}
