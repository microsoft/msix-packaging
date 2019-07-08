//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once

#include <type_traits>
#include <vector>

#include <openssl/objects.h>

namespace MSIX
{
    // Support for our custom OIDs

    enum class CustomOpenSSLObjectName
    {
        spcIndirectDataContext,
        spcSipInfoObjID,
        spcSpOpusInfo,
        spcStatementType,
        individualCodeSigning,
    };

    struct CustomOpenSSLObject
    {
        CustomOpenSSLObject(CustomOpenSSLObjectName name_, int nid_) :
            name(name_), nid(nid_) {}

        CustomOpenSSLObject(const CustomOpenSSLObject&) = default;
        CustomOpenSSLObject& operator=(const CustomOpenSSLObject&) = default;

        CustomOpenSSLObject(CustomOpenSSLObject&&) = default;
        CustomOpenSSLObject& operator=(CustomOpenSSLObject&&) = default;

        CustomOpenSSLObjectName GetName() const { return name; }

        int GetNID() const { return nid; }

        ASN1_OBJECT* GetObj() const { return OBJ_nid2obj(nid); }

    private:
        CustomOpenSSLObjectName name;
        int nid = NID_undef;
    };

    // This helper class can only support a single use at a time because OBJ_cleanup will destroy
    // any other simultaneous use.  A shared_ptr singleton model would be best if needed.
    struct CustomOpenSSLObjects
    {
        CustomOpenSSLObjects();
        ~CustomOpenSSLObjects();

        CustomOpenSSLObjects(const CustomOpenSSLObjects&) = delete;
        CustomOpenSSLObjects& operator=(const CustomOpenSSLObjects&) = delete;

        CustomOpenSSLObjects(CustomOpenSSLObjects&&) = delete;
        CustomOpenSSLObjects& operator=(CustomOpenSSLObjects&&) = delete;

        const CustomOpenSSLObject& Get(CustomOpenSSLObjectName name) const;

    private:
        // Not enough to bother with more complex search
        std::vector<CustomOpenSSLObject> objects;
    };

    // Custom ASN1 writing.
    // All of these types are ephemeral; used merely to tag the data when serializing.
    namespace ASN1
    {
        // Empty type to allow for template filtering
        struct Item {};

        // Base type for items that hold other items
        struct Container : public Item
        {
            using BytesType = std::vector<uint8_t>;

            Container() = default;
            Container(BytesType&& contents) : m_bytes(std::move(contents)) {}

            BytesType& GetBytes() { return m_bytes; }

        protected:
            BytesType m_bytes;
        };

        struct Sequence : public Container
        {
            Sequence() = default;
            Sequence(Container::BytesType&& contents) : Container(std::move(contents)) {}
            void AppendTo(Container::BytesType& bytes) const;
        };

        struct ObjectIdentifier : public Item
        {
            ObjectIdentifier(ASN1_OBJECT* obj) : m_object(obj) {}
            void AppendTo(Container::BytesType& bytes) const;

        private:
            ASN1_OBJECT* m_object;
        };

        struct Integer : public Item
        {
            Integer(std::uint32_t val) : m_val(val) {}
            void AppendTo(Container::BytesType& bytes) const;

        private:
            uint64_t m_val;
        };

        struct OctetString : public Item
        {
            OctetString(Container::BytesType& bytes) : m_bytes(bytes) {}
            void AppendTo(Container::BytesType& bytes) const;

        private:
            Container::BytesType& m_bytes;
        };

        struct Null : public Item
        {
            void AppendTo(Container::BytesType& bytes) const;
        };

        // Any item can write itself to a vector
        template <typename ItemType>
        inline std::vector<uint8_t>& operator<<(std::vector<uint8_t>& output, const ItemType& item)
        {
            static_assert(std::is_convertible<ItemType*, Item*>::value, "Output type must be an Item");
            item.AppendTo(output);
            return output;
        }

        // Only items can be written to containers
        template <typename ContainerType, typename ItemType>
        inline ContainerType& operator<<(ContainerType& container, const ItemType& item)
        {
            static_assert(std::is_convertible<ContainerType*, Container*>::value, "Receiving type must be a Container");
            static_assert(std::is_convertible<ItemType*, Item*>::value, "Output type must be an Item");
            container.GetBytes() << item;
            return container;
        }
    } // namespace ASN1
} // namespace MSIX
