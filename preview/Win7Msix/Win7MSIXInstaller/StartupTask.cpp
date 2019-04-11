#include <windows.h>

#include <shlobj_core.h>
#include <CommCtrl.h>

#include "FilePaths.hpp"
#include "StartupTask.hpp"
#include "GeneralUtil.hpp"
#include "Constants.hpp"
#include <TraceLoggingProvider.h>
#include <taskschd.h>

const PCWSTR StartupTask::HandlerName = L"StartupTask";

/// TODO stop hardcode userID for my machine
const PCWSTR StartupTask::TaskDefinitionXmlPrefix =
L"<Task xmlns=\"http://schemas.microsoft.com/windows/2004/02/mit/task\">\
        <Principals>\
            <Principal id=\"Author\">\
                <UserId>wcheng-win7\\wcheng</UserId>\
                <LogonType>InteractiveToken</LogonType>\
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

const PCWSTR taskFolder = L"\\Microsoft\\Windows\\MsixCore";

HRESULT StartupTask::ExecuteForAddRequest()
{
    /// TODO: reminder to generalize user ID


    AutoCoInitialize coInit;
    RETURN_IF_FAILED(coInit.Initialize(COINIT_MULTITHREADED));
    {
        // New scope so that ComPtrs are released prior to calling CoUninitialize
        Bstr rootFolderPathBstr(taskFolder);

        Bstr taskNameBstr(m_msixRequest->GetPackageInfo()->GetPackageFullName());

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

        ComPtr<ITaskFolder> taskFolder;
        RETURN_IF_FAILED(taskService->GetFolder(rootFolderPathBstr, &taskFolder));

        ComPtr<ITaskDefinition> taskDefinition;
        RETURN_IF_FAILED(taskService->NewTask(0, &taskDefinition));

        std::wstring taskDefinitionXml = TaskDefinitionXmlPrefix + m_executable + TaskDefinitionXmlPostfix;
        Bstr taskDefinitionXmlBstr(taskDefinitionXml);
        RETURN_IF_FAILED(taskDefinition->put_XmlText(taskDefinitionXmlBstr));

        ComPtr<IRegisteredTask> registeredTask;
        RETURN_IF_FAILED(taskFolder->RegisterTaskDefinition(taskNameBstr, taskDefinition.Get(), TASK_CREATE_OR_UPDATE,
            variantNull /*userId*/, variantNull /*password*/, TASK_LOGON_SERVICE_ACCOUNT, variantNull /*sddl*/, &registeredTask));
    }
    return S_OK;
}

HRESULT StartupTask::ExecuteForRemoveRequest()
{

    return S_OK;
}

HRESULT StartupTask::ParseManifest()
{
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
            m_executable = m_msixRequest->GetFilePathMappings()->GetExecutablePath(executable.Get(), m_msixRequest->GetPackageInfo()->GetPackageFullName().c_str());
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
