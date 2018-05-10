//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once

#include <string>
#include <vector>
#include <memory>

#include "AppxPackaging.hpp"
#include "AppxPackageInfo.hpp"
#include "MSIXWindows.hpp"
#include "Exceptions.hpp"
#include "ComHelper.hpp"
#include "VerifierObject.hpp"
#include "IXml.hpp"

EXTERN_C const IID IID_IAppxManifestObject;
#ifndef WIN32
// {eff6d561-a236-4058-9f1d-8f93633fba4b}
interface IAppxManifestObject : public IUnknown
#else
#include "Unknwn.h"
#include "Objidl.h"
class IAppxManifestObject : public IUnknown
#endif
{
public:
    virtual const std::string& GetPublisher() = 0;
    virtual const std::string GetPackageFullName() = 0;
    virtual const std::string& GetVersion() = 0;
    virtual const std::string& GetName() = 0;
    virtual const std::string& GetArchitecture() = 0;
    virtual const MSIX_PLATFORMS GetPlatform() = 0;
};

EXTERN_C const IID IID_IBundleInfo;
#ifndef WIN32
// {ff82ffcd-747a-4df9-8879-853ab9dd15a1}
interface IBundleInfo : public IUnknown
#else
#include "Unknwn.h"
#include "Objidl.h"
class IBundleInfo : public IUnknown
#endif
{
public:
    virtual std::vector<std::unique_ptr<MSIX::AppxPackageInBundle>>& GetPackages() = 0;
};

SpecializeUuidOfImpl(IAppxManifestObject);
SpecializeUuidOfImpl(IBundleInfo);

namespace MSIX {
        // Object backed by AppxManifest.xml
    class AppxManifestObject final : public ComClass<AppxManifestObject, IVerifierObject, IAppxManifestObject>
    {
    public:
        AppxManifestObject(IXmlFactory* factory, const ComPtr<IStream>& stream);

        // IVerifierObject
        bool HasStream() override { return !!m_stream; }
        ComPtr<IStream> GetStream() override { return m_stream; }
        ComPtr<IStream> GetValidationStream(const std::string& part, const ComPtr<IStream>&) override { NOTSUPPORTED; }

        // IAppxManifestObject
        const std::string& GetPublisher() override { return GetPackageId()->PublisherId; }
        const std::string GetPackageFullName() override { return m_packageId->GetPackageFullName(); }
        const std::string& GetVersion() override { return GetPackageId()->Version; }
        const std::string& GetName() override { return GetPackageId()->Name; }
        const std::string& GetArchitecture() override { return GetPackageId()->Architecture; }
        const MSIX_PLATFORMS GetPlatform() override { return m_platform; }

        AppxPackageId* GetPackageId() { return m_packageId.get(); }

    protected:
        ComPtr<IStream> m_stream;
        std::unique_ptr<AppxPackageId> m_packageId;
        MSIX_PLATFORMS m_platform = MSIX_PLATFORM_NONE;
    };

    class AppxBundleManifestObject final : public ComClass<AppxBundleManifestObject, IVerifierObject, IBundleInfo, IAppxManifestObject>
    {
    public:
        AppxBundleManifestObject(IXmlFactory* factory, const ComPtr<IStream>& stream);

         // IVerifierObject
        bool HasStream() override { return !!m_stream; }
        ComPtr<IStream> GetStream() override { return m_stream; }
        ComPtr<IStream> GetValidationStream(const std::string& part, const ComPtr<IStream>&) override { NOTSUPPORTED; }

        // IAppxManifestObject
        const std::string& GetPublisher() override { return GetPackageId()->PublisherId; }
        const std::string GetPackageFullName() override { NOTSUPPORTED; }
        const std::string& GetVersion() override { return GetPackageId()->Version; }
        const std::string& GetName() override { return GetPackageId()->Name; }
        const std::string& GetArchitecture() override { NOTSUPPORTED; }
        const MSIX_PLATFORMS GetPlatform() override { NOTSUPPORTED; }

        AppxPackageId* GetPackageId() { return m_packageId.get(); }

        // IBundleInfo
        std::vector<std::unique_ptr<AppxPackageInBundle>>& GetPackages() override { return m_packages; }

    protected:
        ComPtr<IStream> m_stream;
        std::unique_ptr<AppxPackageId> m_packageId;
        std::vector<std::unique_ptr<AppxPackageInBundle>> m_packages;
    };
}
