#pragma once
#include <Windows.h>
namespace Win7MsixInstallerLib {

    class IPackage {
    public:
        virtual unsigned long long GetVersionNumber() = 0;
        virtual std::wstring GetVersion() = 0;
        virtual std::wstring GetPublisherDisplayName() = 0;
        virtual std::wstring GetPackageFullName() = 0;
        virtual std::wstring GetPackageFamilyName() = 0;
        virtual std::wstring GetDisplayName() = 0;
        virtual std::wstring GetId() = 0;
        virtual IStream* GetLogo() = 0;
    };

    class IInstalledPackageInfo : public IPackage
    {
    public:
        virtual std::wstring GetFullExecutableFilePath() = 0;
        virtual std::wstring GetInstalledLocation() = 0;
    };
}