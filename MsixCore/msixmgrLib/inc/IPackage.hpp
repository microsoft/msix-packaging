#pragma once
#include <Windows.h>
#include <vector>
#include <memory>
#include "AppxPackaging.hpp"

namespace MsixCoreLib {

    class IPackage {
    public:
        virtual unsigned long long GetVersionNumber() = 0;
        virtual std::wstring GetVersion() = 0;
        virtual APPX_PACKAGE_ARCHITECTURE GetArchitecture() = 0;
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

    struct ExecutionInfo
    {
        std::wstring resolvedExecutableFilePath;
        std::wstring commandLineArguments;
        std::wstring workingDirectory;
    };

    struct ScriptSettings
    {
        std::wstring scriptPath;
        bool runOnce{ true };
        bool showWindow{ false };
        bool waitForScriptToFinish{ true };
    };

    class IInstalledPackage : public IPackage
    {
    public:
        virtual std::wstring GetInstalledLocation() = 0;
        virtual ExecutionInfo* GetExecutionInfo() = 0;
        virtual ScriptSettings* GetScriptSettings() = 0;
        virtual ~IInstalledPackage() {}
    protected:
        IInstalledPackage() {}
    };
}