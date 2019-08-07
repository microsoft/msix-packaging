//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
//

#include "AppxManifestObject.hpp"
#include "AppxManifestValidation.hpp"
#include "UnicodeConversion.hpp"
#include "Encoding.hpp"
#include "Crypto.hpp"
#include "Enumerators.hpp"
#include "IXml.hpp"

#include <regex>
#include <array>
#include <string>

namespace MSIX {

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
        ThrowErrorIf(Error::AppxManifestSemanticError, (m_name.empty() || m_version.empty() || m_publisher.empty()), "Invalid Identity element");
        m_publisherId = ComputePublisherId(m_publisher);
        ValidatePackageString(m_name, XmlAttributeName::Name);
        // If ResourceId == "~" this is the identity of a bundle.
        if (!m_resourceId.empty() && m_resourceId != "~")
        {
            ValidatePackageString(m_resourceId, XmlAttributeName::ResourceId);
        }
        if (m_architecture.empty())
        {   // Default value
            m_architecture = "neutral";
        }
    }

    // IAppxManifestPackageId
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
        return ComparePublisher(wstring_to_utf8(other).c_str(), isSame);
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

    void AppxManifestPackageId::ValidatePackageString(const std::string& packageString, XmlAttributeName identityPart)
    {
        ThrowErrorIf(Error::AppxManifestSemanticError, !AppxManifestValidation::IsIdentifierValid(packageString),
            (std::string("Invalid Package Identifier ") + GetAttributeNameStringUtf8(identityPart) + ": " + packageString).c_str());
    }

    // IAppxManifestPackageIdUtf8
    HRESULT STDMETHODCALLTYPE AppxManifestPackageId::GetName(LPSTR* name) noexcept try
    {
        return m_factory->MarshalOutStringUtf8(m_name, name);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE AppxManifestPackageId::GetPublisher(LPSTR* publisher) noexcept try
    {
        return m_factory->MarshalOutStringUtf8(m_publisher, publisher);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE AppxManifestPackageId::GetResourceId(LPSTR* resourceId) noexcept try
    {
        return m_factory->MarshalOutStringUtf8(m_resourceId, resourceId);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE AppxManifestPackageId::ComparePublisher(LPCSTR other, BOOL *isSame) noexcept try
    {
        ThrowErrorIf(Error::InvalidParameter, (isSame == nullptr), "bad pointer" );
        *isSame = FALSE;
        if (0 == m_publisher.compare(other))
        {
            *isSame = TRUE;
        }
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE AppxManifestPackageId::GetPackageFullName(LPSTR* packageFullName) noexcept try
    {
        auto fullName = GetPackageFullName();
        return m_factory->MarshalOutStringUtf8(fullName, packageFullName);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE AppxManifestPackageId::GetPackageFamilyName(LPSTR* packageFamilyName) noexcept try
    {
        auto familyName = GetPackageFamilyName();
        return m_factory->MarshalOutStringUtf8(familyName, packageFamilyName);
    } CATCH_RETURN();
}
