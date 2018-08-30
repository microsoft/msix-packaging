//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
//

#include "AppxManifestObject.hpp"
#include "UnicodeConversion.hpp"
#include "Encoding.hpp"
#include "SHA256.hpp"
#include "Enumerators.hpp"

#include <regex>
#include <array>
#include <string>

namespace MSIX {

    // Used for validation of AppxPackageId name and resourceId
    static const std::size_t ProhibitedStringsSize = 24;
    static const std::array<const char*, ProhibitedStringsSize> ProhibitedStrings = {
        u8".",    u8"..",   u8"con",  u8"prn",  u8"aux",  u8"nul",  u8"com1", u8"com2",
        u8"com3", u8"com4", u8"com5", u8"com6", u8"com7", u8"com8", u8"com9", u8"lpt1",
        u8"lpt2", u8"lpt3", u8"lpt4", u8"lpt5", u8"lpt6", u8"lpt7", u8"lpt8", u8"lpt9",
    };

    static const std::size_t ProhibitedStringsBeginWithSize = 23;
    static const std::array<const char*, ProhibitedStringsBeginWithSize> ProhibitedStringsBeginWith = {
        u8"con.",  u8"prn.",  u8"aux.",  u8"nul.",  u8"com1.", u8"com2.", u8"com3.", u8"com4.",
        u8"com5.", u8"com6.", u8"com7.", u8"com8.", u8"com9.", u8"lpt1.", u8"lpt2.", u8"lpt3.",
        u8"lpt4.", u8"lpt5.", u8"lpt6.", u8"lpt7.", u8"lpt8.", u8"lpt9.", u8"xn--",
    };

    AppxManifestPackageId::AppxManifestPackageId(
        IMsixFactory* factory,
        const std::string& name,
        const std::string& version,
        const std::string& resourceId,
        const std::string& architecture,
        const std::string& publisher) :
         m_factory(factory), m_name(name), m_version(version), m_resourceId(resourceId), m_architecture(architecture), m_publisher(publisher)
    {
        // Only name, publisherId and version are required.
        ThrowErrorIf(Error::AppxManifestSemanticError, (m_name.empty() || m_version.empty() || m_version.empty()), "Invalid Identity element");
        m_publisherId = ComputePublisherId(m_publisher);
        std::regex nameRegex("[a-zA-Z0-9\\.\\-]+"); // valid characters for name
        ValidatePackageString(m_name);
        // If ResourceId == "~" this is the identity of a bundle.
        if (!m_resourceId.empty() && m_resourceId != "~")
        {
            ValidatePackageString(m_resourceId);
        }
        if (m_architecture.empty())
        {   // Default value
            m_architecture = "neutral";
        }
    }

    HRESULT STDMETHODCALLTYPE AppxManifestPackageId::GetName(LPWSTR* name) noexcept try
    {
        return m_factory->MarshalOutString(m_name, name);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE AppxManifestPackageId::GetArchitecture(APPX_PACKAGE_ARCHITECTURE* architecture) noexcept try
    {
        if (m_architecture == "x86")
        { 
            *architecture = APPX_PACKAGE_ARCHITECTURE_X86;
        }
        else if (m_architecture == "x64")
        {
            *architecture = APPX_PACKAGE_ARCHITECTURE_X64;
        }
        else if (m_architecture == "arm")
        {
            *architecture = APPX_PACKAGE_ARCHITECTURE_ARM;
        }
        else if (m_architecture == "arm64")
        {
            *architecture = APPX_PACKAGE_ARCHITECTURE_ARM64;
        }
        else if (m_architecture == "neutral")
        {
            *architecture = APPX_PACKAGE_ARCHITECTURE_NEUTRAL;
        }
        else
        {
            return static_cast<HRESULT>(Error::InvalidParameter);
        }
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE AppxManifestPackageId::GetPublisher(LPWSTR* publisher) noexcept try
    {
        return m_factory->MarshalOutString(m_publisher, publisher);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE AppxManifestPackageId::GetVersion(UINT64* packageVersion) noexcept try
    {
        ThrowErrorIf(Error::InvalidParameter, (packageVersion == nullptr), "bad pointer" );
        *packageVersion = DecodeVersionNumber(m_version);
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE AppxManifestPackageId::GetResourceId(LPWSTR* resourceId) noexcept try
    {
        return m_factory->MarshalOutString(m_resourceId, resourceId);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE AppxManifestPackageId::ComparePublisher(LPCWSTR other, BOOL *isSame) noexcept try
    {
        ThrowErrorIf(Error::InvalidParameter, (isSame == nullptr), "bad pointer" );
        *isSame = FALSE;
        if (0 == m_publisher.compare(utf16_to_utf8(other)))
        {
            *isSame = TRUE;
        }
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE AppxManifestPackageId::GetPackageFullName(LPWSTR* packageFullName) noexcept try
    {
        auto fullName = GetPackageFullName();
        return m_factory->MarshalOutString(fullName, packageFullName);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE AppxManifestPackageId::GetPackageFamilyName(LPWSTR* packageFamilyName) noexcept try
    {
        auto familyName = GetPackageFamilyName();
        return m_factory->MarshalOutString(familyName, packageFamilyName);
    } CATCH_RETURN();

    void AppxManifestPackageId::ValidatePackageString(std::string& packageString)
    {
        std::regex e("[a-zA-Z0-9\\.\\-]+"); // valid characters
        ThrowErrorIf(Error::AppxManifestSemanticError, !std::regex_match(packageString, e), "Invalid Package String");
        std::string packageStringLower;
        packageStringLower.resize(packageString.size());
        std::transform(packageString.begin(), packageString.end(), packageStringLower.begin(), ::tolower);
        // Package string can't be the same as any of the strings in ProhibitedStrings
        for(const auto& prohibited : ProhibitedStrings)
        {
            if(strlen(prohibited) == packageStringLower.size())
            {
                ThrowErrorIf(Error::AppxManifestSemanticError, strcmp(prohibited, packageStringLower.c_str()) == 0,
                    "Invalid Package String");
            }
        }
        // Package string can't be begin with the strings in ProhibitedStringsBeginWith
        for(const auto& prohibited : ProhibitedStringsBeginWith)
        {
            if(strlen(prohibited) <= packageStringLower.size())
            {
                ThrowErrorIf(Error::AppxManifestSemanticError, strncmp(prohibited, packageStringLower.c_str(), strlen(prohibited)) == 0,
                    "Invalid Package String");
            }
        }
        // Package string can't contain ".xn--"
        ThrowErrorIf(Error::AppxManifestSemanticError, strstr(".xn--", packageStringLower.c_str()) != nullptr,
                    "Invalid Package String");
        // Package string can't end with "."
        ThrowErrorIf(Error::AppxManifestSemanticError, packageStringLower[packageStringLower.size() - 1]  == '.',
                    "Invalid Package String");
    }

    std::string AppxManifestPackageId::ComputePublisherId(const std::string& publisher)
    {
        auto wpublisher = utf8_to_u16string(publisher);
        std::vector<std::uint8_t> buffer(wpublisher.size() * sizeof(char16_t));
        memcpy(buffer.data(), &wpublisher[0], wpublisher.size() * sizeof(char16_t));

        std::vector<std::uint8_t> hash;
        ThrowErrorIfNot(Error::Unexpected, SHA256::ComputeHash(buffer.data(), buffer.size(), hash),  "Failed computing publisherId");
        return Base32Encoding(hash);
    }

    AppxBundleManifestPackageInfo::AppxBundleManifestPackageInfo(
        IMsixFactory* factory,
        const std::string& name,
        const std::string& bundleName,
        const std::string& version,
        const std::uint64_t size,
        const std::uint64_t offset,
        const std::string& resourceId,
        const std::string& architecture,
        const std::string& publisher,
        std::vector<Bcp47Tag>& languages,
        APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE packageType):
        m_factory(factory), m_fileName(name), m_size(size), m_offset(offset), m_languages(std::move(languages)), m_packageType(packageType)
    {
        std::regex e (".+\\.((appx)|(msix))");
        ThrowErrorIf(Error::AppxManifestSemanticError, !std::regex_match(m_fileName, e), "Invalid FileName attribute in AppxBundleManifest.xml");
        m_packageId = ComPtr<IAppxManifestPackageId>::Make<AppxManifestPackageId>(factory, bundleName, version, resourceId, architecture, publisher);
    }

    // IAppxBundleManifestPackageInfo
    HRESULT STDMETHODCALLTYPE AppxBundleManifestPackageInfo::GetPackageType(APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE *packageType) noexcept try
    {
        ThrowErrorIf(Error::InvalidParameter, (packageType == nullptr), "bad pointer");
        *packageType = m_packageType;
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE AppxBundleManifestPackageInfo::GetPackageId(IAppxManifestPackageId **packageId) noexcept try
    {
        ThrowErrorIf(Error::InvalidParameter, (packageId == nullptr || *packageId != nullptr), "bad pointer");
        auto package = m_packageId;
        *packageId = package.Detach();
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE AppxBundleManifestPackageInfo::GetFileName(LPWSTR *fileName) noexcept try
    {
        return m_factory->MarshalOutString(m_fileName, fileName);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE AppxBundleManifestPackageInfo::GetOffset(UINT64 *offset) noexcept try
    {
        ThrowErrorIf(Error::InvalidParameter, (offset == nullptr), "bad pointer");
        *offset = m_offset;
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE AppxBundleManifestPackageInfo::GetSize(UINT64 *size) noexcept try
    {
        ThrowErrorIf(Error::InvalidParameter, (size == nullptr), "bad pointer");
        *size = m_size;
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE AppxBundleManifestPackageInfo::GetResources(IAppxManifestQualifiedResourcesEnumerator **resources) noexcept try
    {
        ThrowErrorIf(Error::InvalidParameter, (resources == nullptr || *resources != nullptr), "bad pointer.");
        std::vector<ComPtr<IAppxManifestQualifiedResource>> languages;
        for(auto& bcp47 : m_languages)
        {
            auto resource = ComPtr<IAppxManifestQualifiedResource>::Make<AppxBundleQualifiedResource>(m_factory, bcp47.GetFullTag());
            languages.push_back(std::move(resource));
        }
        *resources = ComPtr<IAppxManifestQualifiedResourcesEnumerator>::
            Make<EnumeratorCom<IAppxManifestQualifiedResourcesEnumerator, IAppxManifestQualifiedResource>>(languages).Detach();
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();
}
