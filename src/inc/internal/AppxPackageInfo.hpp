//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once

#include "ComHelper.hpp"
#include "MSIXFactory.hpp"
#include "Exceptions.hpp"
#include "IXml.hpp"
#include "Encoding.hpp"
#include "UnicodeConversion.hpp"
#include "Crypto.hpp"

#include <string>
#include <memory>

// {76b7d3e1-768a-45cb-9626-ba6452bed2de}
#ifndef WIN32
interface IAppxManifestPackageIdInternal : public IUnknown
#else
#include "unknwn.h"
#include "objidl.h"
class IAppxManifestPackageIdInternal : public IUnknown
#endif
{
public:
    virtual const std::string& GetPublisherId() = 0;
    virtual const std::string& GetPublisher() = 0;
    virtual const std::string GetPackageFullName() = 0;
    virtual const std::string& GetName() = 0;
    virtual const std::string& GetArchitecture() = 0;
    virtual const std::string& GetResourceId() = 0;
    virtual const std::string GetPackageFamilyName() = 0;
};
MSIX_INTERFACE(IAppxManifestPackageIdInternal, 0x76b7d3e1,0x768a,0x45cb,0x96,0x26,0xba,0x64,0x52,0xbe,0xd2,0xde);

namespace MSIX {

    // The maximum possible value for sub version is 65536 or std::uint16_t
    static std::uint16_t ConvertSubVersion(std::string& subVersion)
    {
        ThrowErrorIf(Error::Unexpected, subVersion.empty(), "Error decoding the version of the package id");
        // Permit parsing of "-0" as a subVersion
        if (subVersion[0] == '-')
        {
            if(subVersion.size() == 2 && subVersion[1] == '0')
            {
                return 0;
            }
            ThrowError(Error::Unexpected);
        }
        std::uint32_t value = 0;
        for (size_t i = 0; i < subVersion.size(); i++)
        {
            auto digit = subVersion[i] - '0';
            ThrowErrorIf(Error::Unexpected, digit > 9, "Bad format.");
            ThrowErrorIf(Error::Unexpected, value > 0xFFFFFFFF / 10, "Bad format.");
            value *= 10;
            ThrowErrorIf(Error::Unexpected, value > 0xFFFFFFFF - digit, "Bad format.");
            value += digit;
        }
        ThrowErrorIf(Error::Unexpected, value > 0xFFFF, "Arithmetic overflow.");
        return static_cast<std::uint16_t>(value);
    }

    static std::uint64_t DecodeVersionNumber(std::string& version)
    {
        std::uint64_t result = 0;
        size_t position = 0;
        size_t found = 0;
        int segmentNumber = 0;
        constexpr auto delimiter = '.';

        while (found != std::string::npos && segmentNumber < 4)
        {
            segmentNumber++;
            found = version.find(delimiter, position);
            auto segment = version.substr(position, found - position);
            auto sub = ConvertSubVersion(segment);
            result |= (static_cast<std::uint64_t>(sub) << (16 * (4 - segmentNumber)));
            position = found + 1;
        }

        // If we didn't reach the end of the string, format was bad
        ThrowErrorIf(Error::InvalidParameter, found != std::string::npos, "Bad format" );

        return result;
    }

    static std::string ComputePublisherId(const std::string& publisher)
    {
        auto wpublisher = utf8_to_u16string(publisher);
        std::vector<std::uint8_t> buffer(wpublisher.size() * sizeof(char16_t));
        memcpy(buffer.data(), &wpublisher[0], wpublisher.size() * sizeof(char16_t));

        std::vector<std::uint8_t> hash;
        ThrowErrorIfNot(Error::Unexpected, SHA256::ComputeHash(buffer.data(), static_cast<uint32_t>(buffer.size()), hash), "Failed computing publisherId");
        return Encoding::Base32Encoding(hash);
    }

    class AppxManifestPackageId final : public ComClass<AppxManifestPackageId, IAppxManifestPackageId, IAppxManifestPackageIdInternal, IAppxManifestPackageIdUtf8>
    {
    public:
        AppxManifestPackageId(
            IMsixFactory* factory,
            const std::string& name,
            const std::string& version,
            const std::string& resourceId,
            const std::string& architecture,
            const std::string& publisher);

        // AppxManifestPackageId
        HRESULT STDMETHODCALLTYPE GetName(LPWSTR* name) noexcept override;
        HRESULT STDMETHODCALLTYPE GetArchitecture(APPX_PACKAGE_ARCHITECTURE* architecture) noexcept override;
        HRESULT STDMETHODCALLTYPE GetPublisher(LPWSTR* publisher) noexcept override;
        HRESULT STDMETHODCALLTYPE GetVersion(UINT64* packageVersion) noexcept override;
        HRESULT STDMETHODCALLTYPE GetResourceId(LPWSTR* resourceId) noexcept override;
        HRESULT STDMETHODCALLTYPE ComparePublisher(LPCWSTR other, BOOL *isSame) noexcept override;
        HRESULT STDMETHODCALLTYPE GetPackageFullName(LPWSTR* packageFullName) noexcept override;
        HRESULT STDMETHODCALLTYPE GetPackageFamilyName(LPWSTR* packageFamilyName) noexcept override;

        // IAppxManifestPackageIdInternal
        const std::string& GetPublisherId() override { return m_publisherId; }
        const std::string& GetPublisher() override { return m_publisher; }
        const std::string& GetName() override { return m_name; }
        const std::string& GetResourceId() override { return m_resourceId; }
        const std::string& GetArchitecture() override { return m_architecture; }
        const std::string GetPackageFullName() override
        {
            return m_name + "_" + m_version + "_" + m_architecture + "_" + m_resourceId + "_" + m_publisherId;
        }
        const std::string GetPackageFamilyName() override
        {
            return m_name + "_" + m_publisherId;
        }

        // IAppxManifestPackageIdUtf8
        HRESULT STDMETHODCALLTYPE GetName(LPSTR* name) noexcept override;
        HRESULT STDMETHODCALLTYPE GetPublisher(LPSTR* publisher) noexcept override;
        HRESULT STDMETHODCALLTYPE GetResourceId(LPSTR* resourceId) noexcept override;
        HRESULT STDMETHODCALLTYPE ComparePublisher(LPCSTR other, BOOL *isSame) noexcept override;
        HRESULT STDMETHODCALLTYPE GetPackageFullName(LPSTR* packageFullName) noexcept override;
        HRESULT STDMETHODCALLTYPE GetPackageFamilyName(LPSTR* packageFamilyName) noexcept override;

    private:
        void ValidatePackageString(const std::string& packageString, XmlAttributeName identityPart);

    protected:
        IMsixFactory* m_factory;
        std::string   m_name;
        std::string   m_version;
        std::string   m_resourceId;
        std::string   m_architecture;
        std::string   m_publisherId;
        std::string   m_publisher;
    };
}
