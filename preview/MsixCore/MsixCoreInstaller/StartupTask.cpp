#include <windows.h>

#include <shlobj_core.h>
#include <CommCtrl.h>

#include "FilePaths.hpp"
#include "StartupTask.hpp"
#include "GeneralUtil.hpp"
#include "Constants.hpp"
#include <TraceLoggingProvider.h>
#include "MsixTraceLoggingProvider.hpp"
#include <taskschd.h>
using namespace MsixCoreLib;

const PCWSTR StartupTask::HandlerName = L"StartupTask";

const PCWSTR StartupTask::TaskDefinitionXmlPrefix =
    L"<Task xmlns=\"http://schemas.microsoft.com/windows/2004/02/mit/task\">\
        <Principals>\
            <Principal id=\"Author\">\
                <GroupId>S-1-1-0</GroupId>\
                <RunLevel>LeastPrivilege</RunLevel>\
            </Principal>\
        </Principals>\
        <Triggers>\
            <LogonTrigger>\
                <Enabled>true</Enabled>\
            </LogonTrigger>\
        </Triggers>\
        <Settings>\
            <MultipleInstancesPolicy>IgnoreNew</MultipleInstancesPolicy>\
            <DisallowStartIfOnBatteries>true</DisallowStartIfOnBatteries>\
            <StopIfGoingOnBatteries>true</StopIfGoingOnBatteries>\
            <AllowHardTerminate>true</AllowHardTerminate>\
            <StartWhenAvailable>false</StartWhenAvailable>\
            <RunOnlyIfNetworkAvailable>false</RunOnlyIfNetworkAvailable>\
            <AllowStartOnDemand>true</AllowStartOnDemand>\
            <Enabled>true</Enabled>\
            <Hidden>false</Hidden>\
            <RunOnlyIfIdle>false</RunOnlyIfIdle>\
            <WakeToRun>false</WakeToRun>\
            <Priority>7</Priority>\
            <UseUnifiedSchedulingEngine>true</UseUnifiedSchedulingEngine>\
        </Settings>\
        <Actions Context=\"Author\">\
            <Exec>\
                <Command>\"";
const PCWSTR StartupTask::TaskDefinitionXmlPostfix =
                L"\"</Command>\
            </Exec>\
        </Actions>\
    </Task>";

const PCWSTR windowsTaskFolderName = L"\\Microsoft\\Windows";
const PCWSTR taskFolderName = L"MsixCore";

std::wstring StartupTask::CreateTaskXml(std::wstring& executable)
{
    return std::wstring(TaskDefinitionXmlPrefix) +
        executable +
        TaskDefinitionXmlPostfix;
}

HRESULT StartupTask::ExecuteForAddRequest()
{
    RETURN_IF_FAILED(CreateScheduledTasks());
    return S_OK;
}

HRESULT StartupTask::CreateScheduledTasks()
{
    if (m_tasks.empty())
    {
        // Startup tasks are not required, if there are none, nothing to do.
        return S_OK;
    }

    AutoCoInitialize coInit;
    RETURN_IF_FAILED(coInit.Initialize(COINIT_MULTITHREADED));
    {
        // New scope so that ComPtrs are released prior to calling CoUninitialize

        VARIANT variantNull;
        ZeroMemory(&variantNull, sizeof(VARIANT));
        variantNull.vt = VT_NULL;

        ComPtr<ITaskService> taskService;
        RETURN_IF_FAILED(CoCreateInstance(CLSID_TaskScheduler,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_ITaskService,
            reinterpret_cast<void**>(&taskService)));
        RETURN_IF_FAILED(taskService->Connect(variantNull /*serverName*/, variantNull /*user*/, variantNull /*domain*/, variantNull /*password*/));

        Bstr rootFolderPathBstr(windowsTaskFolderName);
        ComPtr<ITaskFolder> rootFolder;
        RETURN_IF_FAILED(taskService->GetFolder(rootFolderPathBstr, &rootFolder));

        Bstr taskFolderBstr(taskFolderName);
        ComPtr<ITaskFolder> msixCoreFolder;
        HRESULT hrCreateFolder = rootFolder->CreateFolder(taskFolderBstr, variantNull /*sddl*/, &msixCoreFolder);
        if (hrCreateFolder == HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS))
        {
            RETURN_IF_FAILED(rootFolder->GetFolder(taskFolderBstr, &msixCoreFolder));
        }
        else
        {
            RETURN_IF_FAILED(hrCreateFolder);
        }

        ComPtr<ITaskDefinition> taskDefinition;
        RETURN_IF_FAILED(taskService->NewTask(0, &taskDefinition));

        for (auto& task : m_tasks)
        {
            std::wstring taskDefinitionXml = CreateTaskXml(task.executable);
            Bstr taskDefinitionXmlBstr(taskDefinitionXml);
            RETURN_IF_FAILED(taskDefinition->put_XmlText(taskDefinitionXmlBstr));

            Bstr taskNameBstr(task.name);
            ComPtr<IRegisteredTask> registeredTask;
            RETURN_IF_FAILED(msixCoreFolder->RegisterTaskDefinition(taskNameBstr, taskDefinition.Get(), TASK_CREATE_OR_UPDATE,
                variantNull /*userId*/, variantNull /*password*/, TASK_LOGON_GROUP, variantNull /*sddl*/, &registeredTask));
        }
    }
    return S_OK;
}

HRESULT StartupTask::ExecuteForRemoveRequest()
{
    RETURN_IF_FAILED(DeleteScheduledTasks())
    return S_OK;
}

HRESULT StartupTask::DeleteScheduledTasks()
{
    if (m_tasks.empty())
    {
        return S_OK;
    }

    AutoCoInitialize coInit;
    RETURN_IF_FAILED(coInit.Initialize(COINIT_MULTITHREADED));
    {
        // New scope so that ComPtrs are released prior to calling CoUninitialize

        VARIANT variantNull;
        ZeroMemory(&variantNull, sizeof(VARIANT));
        variantNull.vt = VT_NULL;

        ComPtr<ITaskService> taskService;
        RETURN_IF_FAILED(CoCreateInstance(CLSID_TaskScheduler,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_ITaskService,
            reinterpret_cast<void**>(&taskService)));
        RETURN_IF_FAILED(taskService->Connect(variantNull, variantNull, variantNull, variantNull));

        Bstr rootFolderPathBstr(windowsTaskFolderName);
        ComPtr<ITaskFolder> rootFolder;
        RETURN_IF_FAILED(taskService->GetFolder(rootFolderPathBstr, &rootFolder));

        Bstr taskFolderBstr(taskFolderName);
        ComPtr<ITaskFolder> msixCoreFolder;
        RETURN_IF_FAILED(rootFolder->GetFolder(taskFolderBstr, &msixCoreFolder));

        for (auto& task : m_tasks)
        {
            Bstr taskNameBstr(task.name);
            ComPtr<IRegisteredTask> registeredTask;
            RETURN_IF_FAILED(msixCoreFolder->DeleteTask(taskNameBstr, 0 /*flags*/));
        }
    }
    return S_OK;
}

HRESULT StartupTask::ParseManifest()
{
    std::wstring currentPackageFamilyName = GetFamilyNameFromFullName(m_msixRequest->GetPackageInfo()->GetPackageFullName());

    ComPtr<IMsixDocumentElement> domElement;
    RETURN_IF_FAILED(m_msixRequest->GetPackageInfo()->GetManifestReader()->QueryInterface(UuidOfImpl<IMsixDocumentElement>::iid, reinterpret_cast<void**>(&domElement)));

    ComPtr<IMsixElement> element;
    RETURN_IF_FAILED(domElement->GetDocumentElement(&element));

    ComPtr<IMsixElementEnumerator> extensionEnum;
    RETURN_IF_FAILED(element->GetElements(extensionQuery.c_str(), &extensionEnum));
    BOOL hasCurrent = FALSE;
    RETURN_IF_FAILED(extensionEnum->GetHasCurrent(&hasCurrent));
    while (hasCurrent)
    {
        ComPtr<IMsixElement> extensionElement;
        RETURN_IF_FAILED(extensionEnum->GetCurrent(&extensionElement));
        Text<wchar_t> extensionCategory;
        RETURN_IF_FAILED(extensionElement->GetAttributeValue(categoryAttribute.c_str(), &extensionCategory));

        if (wcscmp(extensionCategory.Get(), startupTaskCategoryNameInManifest.c_str()) == 0)
        {
            Text<wchar_t> executable;
            RETURN_IF_FAILED(extensionElement->GetAttributeValue(executableAttribute.c_str(), &executable));

            ComPtr<IMsixElementEnumerator> startupTaskEnum;
            RETURN_IF_FAILED(extensionElement->GetElements(startupTaskQuery.c_str(), &startupTaskEnum));
            BOOL taskHasCurrent = FALSE;
            RETURN_IF_FAILED(startupTaskEnum->GetHasCurrent(&taskHasCurrent));
            if (taskHasCurrent)
            {
                ComPtr<IMsixElement> startupTaskElement;
                RETURN_IF_FAILED(startupTaskEnum->GetCurrent(&startupTaskElement));

                Text<wchar_t> taskId;
                RETURN_IF_FAILED(startupTaskElement->GetAttributeValue(taskIdAttribute.c_str(), &taskId));

                Task task;
                task.executable = FilePathMappings::GetInstance().GetExecutablePath(executable.Get(), m_msixRequest->GetPackageInfo()->GetPackageFullName().c_str());
                task.name = currentPackageFamilyName + L" " + taskId.Get();

                m_tasks.push_back(task);
            }
        }
        RETURN_IF_FAILED(extensionEnum->MoveNext(&hasCurrent));
    }

    return S_OK;
}

HRESULT StartupTask::CreateHandler(MsixRequest * msixRequest, IPackageHandler ** instance)
{
    std::unique_ptr<StartupTask> localInstance(new StartupTask(msixRequest));
    if (localInstance == nullptr)
    {
        return E_OUTOFMEMORY;
    }

    RETURN_IF_FAILED(localInstance->ParseManifest());

    *instance = localInstance.release();

    return S_OK;
}
