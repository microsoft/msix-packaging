#pragma once
#include "GeneralUtil.hpp"
#include "IPackageHandler.hpp"
#include "RegistryKey.hpp"
#include <vector>

namespace MsixCoreLib
{
    struct Task
    {
        std::wstring name;
        std::wstring executable;
    };

    class StartupTask : IPackageHandler
    {
    public:

        /// Using task scheduler APIs, creates a scheduled task to run the executable listed in the windows.startupTask extension when the user logs on.
        /// This diverges from the Windows 10 handling of startupTask, as there is built-in OS support to run the tasks on logon without using task scheduler.
        /// Windows 7 and 8 and some versions of windows 10 do not have OS support to understand startupTasks in the same way, so we're using task scheduler instead.
        HRESULT ExecuteForAddRequest();

        /// Removes the scheduled tasks added
        HRESULT ExecuteForRemoveRequest();

        static const PCWSTR HandlerName;
        static HRESULT CreateHandler(_In_ MsixRequest* msixRequest, _Out_ IPackageHandler** instance);
        ~StartupTask() {}
    private:

        MsixRequest* m_msixRequest = nullptr;

        std::vector<Task> m_tasks;

        /// Parses the manifest to find the executable to run as part of startup task
        HRESULT ParseManifest();

        StartupTask() {}
        StartupTask(_In_ MsixRequest* msixRequest) : m_msixRequest(msixRequest) {}

        static const PCWSTR TaskDefinitionXmlPrefix;
        static const PCWSTR TaskDefinitionXmlPostfix;
    };
}
