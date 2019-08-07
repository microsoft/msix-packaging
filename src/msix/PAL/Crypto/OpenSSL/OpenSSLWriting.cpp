//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 

#include "OpenSSLWriting.hpp"
#include "SharedOpenSSL.hpp"

#include <algorithm>
#include <iterator>

namespace MSIX
{
    namespace
    {
        struct CustomObjectDef
        {
            CustomOpenSSLObjectName name;
            const char* oid;
            const char* shortName;
            const char* longName;
        };

#define MSIX_MAKE_CUSTOM_OBJECT_DEF(_name_,_oid_) { CustomOpenSSLObjectName:: ## _name_, _oid_, #_name_, #_name_ }

        CustomObjectDef customObjects[]
        {
            MSIX_MAKE_CUSTOM_OBJECT_DEF(spcIndirectDataContext, "1.3.6.1.4.1.311.2.1.4"),
            MSIX_MAKE_CUSTOM_OBJECT_DEF(spcSipInfoObjID, "1.3.6.1.4.1.311.2.1.30"),
            MSIX_MAKE_CUSTOM_OBJECT_DEF(spcSpOpusInfo, "1.3.6.1.4.1.311.2.1.12"),
            MSIX_MAKE_CUSTOM_OBJECT_DEF(spcStatementType, "1.3.6.1.4.1.311.2.1.11"),
            MSIX_MAKE_CUSTOM_OBJECT_DEF(individualCodeSigning, "1.3.6.1.4.1.311.2.1.21"),
        };
    }

    CustomOpenSSLObjects::CustomOpenSSLObjects()
    {
        for (const auto& obj : customObjects)
        {
            int nid = OBJ_create(obj.oid, obj.shortName, obj.longName);
            if (nid == NID_undef)
            {
                ThrowOpenSSLError("Failed to create custom OpenSSL object");
            }
            objects.emplace_back(obj.name, nid);
        }
    }

    CustomOpenSSLObjects::~CustomOpenSSLObjects()
    {
        OBJ_cleanup();
    }

    const CustomOpenSSLObject& CustomOpenSSLObjects::Get(CustomOpenSSLObjectName name) const
    {
        for (const auto& obj : objects)
        {
            if (obj.GetName() == name)
            {
                return obj;
            }
        }

        UNEXPECTED;
    }
    
    // Custom ASN1 types
namespace ASN1 {

#define LENGTH_LONG_FORM_BIT 0x80

    size_t CountBytesNeededForInteger(uint64_t val)
    {
        for (size_t i = 0; i < sizeof(val); ++i)
        {
            size_t potentialByteCount = sizeof(val) - i;

            uint64_t mask = 0xFF;
            mask = mask << ((potentialByteCount - 1) * 8);

            if (mask & val)
            {
                return potentialByteCount;
            }
        }

        // Will always need 1 byte for 0
        return 1;
    }

    void AppendCountBytesOfInteger(Container::BytesType& bytes, size_t count, uint64_t val)
    {
        for (size_t i = 0; i < count; ++i)
        {
            size_t bitShift = (count - i - 1) * 8;

            uint64_t mask = 0xFF;
            mask = mask << bitShift;

            bytes.push_back(static_cast<Container::BytesType::value_type>((mask & val) >> bitShift));
        }
    }

    void AppendLength(Container::BytesType& bytes, size_t length)
    {
        if (length > 127)
        {
            // long form, first count the bytes needed
            size_t byteCount = CountBytesNeededForInteger(length);
            bytes.push_back(static_cast<Container::BytesType::value_type>(byteCount | LENGTH_LONG_FORM_BIT));

            // Add the bytes
            AppendCountBytesOfInteger(bytes, byteCount, length);
        }
        else
        {
            // short form
            bytes.push_back(static_cast<Container::BytesType::value_type>(length));
        }
    }

    void Sequence::AppendTo(Container::BytesType& bytes) const
    {
        bytes.push_back(V_ASN1_SEQUENCE | V_ASN1_CONSTRUCTED);
        AppendLength(bytes, m_bytes.size());
        bytes.insert(bytes.end(), m_bytes.begin(), m_bytes.end());
    }

    void ObjectIdentifier::AppendTo(Container::BytesType& bytes) const
    {
        int byteCount = i2d_ASN1_OBJECT(m_object, nullptr);
        size_t currentSize = bytes.size();
        bytes.resize(currentSize + static_cast<size_t>(byteCount));

        uint8_t* currentEnd = &bytes[currentSize];
        ThrowOpenSSLErrIfFailed(i2d_ASN1_OBJECT(m_object, reinterpret_cast<unsigned char**>(&currentEnd)));
    }

    void Integer::AppendTo(Container::BytesType& bytes) const
    {
        bytes.push_back(V_ASN1_INTEGER);

        // Count the number of bytes to use
        size_t byteCount = CountBytesNeededForInteger(m_val);
        AppendLength(bytes, byteCount);

        // Add the bytes
        AppendCountBytesOfInteger(bytes, byteCount, m_val);
    }

    void OctetString::AppendTo(Container::BytesType& bytes) const
    {
        bytes.push_back(V_ASN1_OCTET_STRING);
        AppendLength(bytes, m_bytes.size());
        bytes.insert(bytes.end(), m_bytes.begin(), m_bytes.end());
    }

    void Null::AppendTo(Container::BytesType& bytes) const
    {
        bytes.push_back(V_ASN1_NULL);
        AppendLength(bytes, 0);
    }
} // namespace ASN1

} // namespace MSIX