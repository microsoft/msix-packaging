#pragma once
#include <Windows.h>
#include <vector>
#include <memory>
namespace MsixCoreLib {

    class IPackage {
    public:
        virtual unsigned long long GetVersionNumber() = 0;
        virtual std::wstring GetVersion() = 0;
        virtual std::wstring GetPublisherDisplayName() = 0;
        virtual std::wstring GetPackageFullName() = 0;
        virtual std::wstring GetPackageFamilyName() = 0;
        virtual std::wstring GetDisplayName() = 0;
        virtual std::wstring GetId() = 0;
        virtual std::unique_ptr<IStream> GetLogo() = 0;
        virtual std::vector<std::wstring> GetCapabilities() = 0;
        virtual ~IPackage() {}

    protected:
        IPackage() {}
    };

    class IInstalledPackage : public IPackage
    {
    public:
        virtual std::wstring GetInstalledLocation() = 0;
        virtual std::wstring GetResolvedExecutableFilePath() = 0;
        virtual ~IInstalledPackage() {}
    protected:
        IInstalledPackage() {}
    };
}