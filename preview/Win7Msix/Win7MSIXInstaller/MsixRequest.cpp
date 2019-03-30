#include "generalutil.hpp"
#include "MsixRequest.hpp"

#include <string>
#include <map>
#include <algorithm>
#include <utility>
#include <iomanip>
#include <iostream>
#include <functional>
#include <thread>

#include "FootprintFiles.hpp"
#include "FilePaths.hpp"
#include "InstallUI.hpp"
#include <cstdio>
#include <experimental/filesystem> // C++-standard header file name
#include <filesystem> // Microsoft-specific implementation header file name
#include <TraceLoggingProvider.h>

// handlers
#include "Extractor.hpp"
#include "StartMenuLink.hpp"
#include "AddRemovePrograms.hpp"
#include "PopulatePackageInfo.hpp"
#include "Protocol.hpp"
#include "FileTypeAssociation.hpp"
#include "InstallComplete.hpp"


// MSIXWindows.hpp define NOMINMAX because we want to use std::min/std::max from <algorithm>
// GdiPlus.h requires a definiton for min and max. Use std namespace *BEFORE* including it.
using namespace std;
#include <GdiPlus.h>

struct HandlerInfo
{
    CreateHandler create;
    PCWSTR nextHandler;
};

std::map<PCWSTR, HandlerInfo> AddHandlers =
{
    //HandlerName                       Function to create                   NextHandler
    {PopulatePackageInfo::HandlerName,  {PopulatePackageInfo::CreateHandler, CreateAndShowUI::HandlerName }},
    {CreateAndShowUI::HandlerName,      {CreateAndShowUI::CreateHandler,     Extractor::HandlerName }},
    {Extractor::HandlerName,            {Extractor::CreateHandler,           StartMenuLink::HandlerName }},
    {StartMenuLink::HandlerName,        {StartMenuLink::CreateHandler,       AddRemovePrograms::HandlerName}},
    {AddRemovePrograms::HandlerName,    {AddRemovePrograms::CreateHandler,   Protocol::HandlerName}},
    {Protocol::HandlerName,             {Protocol::CreateHandler,            FileTypeAssociation::HandlerName}},
    {FileTypeAssociation::HandlerName,  {FileTypeAssociation::CreateHandler, InstallComplete::HandlerName }},
    {InstallComplete::HandlerName,      {InstallComplete::CreateHandler,     nullptr}},
};

std::map<PCWSTR, HandlerInfo> RemoveHandlers =
{
    //HandlerName                       Function to create                   NextHandler
    {CreateAndShowUI::HandlerName,      {CreateAndShowUI::CreateHandler,     StartMenuLink::HandlerName}},
    {StartMenuLink::HandlerName,        {StartMenuLink::CreateHandler,       AddRemovePrograms::HandlerName}},
    {AddRemovePrograms::HandlerName,    {AddRemovePrograms::CreateHandler,   Protocol::HandlerName}},
    {Protocol::HandlerName,             {Protocol::CreateHandler,            FileTypeAssociation::HandlerName}},
    {FileTypeAssociation::HandlerName,  {FileTypeAssociation::CreateHandler, Extractor::HandlerName}},
    {Extractor::HandlerName,            {Extractor::CreateHandler,           nullptr}},
};

HRESULT MsixRequest::Make(OperationType operationType, Flags flags, std::wstring packageFilePath, std::wstring packageFullName, MSIX_VALIDATION_OPTION validationOption, MsixRequest ** outInstance)
{
    std::unique_ptr<MsixRequest> instance(new MsixRequest());
    if (instance == nullptr)
    {
        return E_OUTOFMEMORY;
    }

    instance->m_operationType = operationType;
    instance->m_flags = flags;
    instance->m_packageFilePath = packageFilePath;
    instance->m_packageFullName = packageFullName;
    instance->m_validationOptions = validationOption;
    RETURN_IF_FAILED(instance->InitializeFilePathMappings());

    //Set response object
    AutoPtr<MsixResponse> responseObject;
    RETURN_IF_FAILED(MsixResponse::Make(&responseObject));
    instance->m_msixResponse = responseObject;

    *outInstance = instance.release();

    return S_OK;
}

HRESULT MsixRequest::InitializeFilePathMappings()
{
    return m_filePathMappings.Initialize();
}

HRESULT MsixRequest::ProcessRequest()
{
    switch (m_operationType)
    {
        case OperationType::Add:
        {
            RETURN_IF_FAILED(ProcessAddRequest());
            break;
        }
        case OperationType::Remove:
        {
            RETURN_IF_FAILED(ProcessRemoveRequest());
            break;
        }
        case OperationType::FindAllPackages:
        {
            RETURN_IF_FAILED(FindAllPackages());
            break;
        }
        case OperationType::FindPackage:
        {
            RETURN_IF_FAILED(DisplayPackageInfo());
            break;
        }
        default:
            return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
    }

    return S_OK;
}

HRESULT MsixRequest::DisplayPackageInfo()
{
    AutoPtr<IPackageHandler> handler;
    RETURN_IF_FAILED(PopulatePackageInfo::CreateHandler(this, &handler));
    HRESULT packageFoundResult = handler->ExecuteForRemoveRequest();

    if (packageFoundResult == S_OK)
    {
        std::wcout << std::endl;
        std::wcout << L"PackageFullName: " << m_packageInfo->GetPackageFullName().c_str() << std::endl;
        std::wcout << L"DisplayName: " << m_packageInfo->GetDisplayName().c_str() << std::endl;
        std::wcout << L"DirectoryPath: " << m_packageInfo->GetPackageDirectoryPath().c_str() << std::endl;
        std::wcout << std::endl;
    }
    else
    {
        std::wcout << std::endl;
        std::wcout << L"Package not found " << std::endl;
    }

    return S_OK;
}

HRESULT MsixRequest::FindAllPackages()
{
    int numPackages = 0;
    for (auto& p : std::experimental::filesystem::directory_iterator(m_filePathMappings.GetMsix7Directory()))
    {
        std::cout << p.path().filename() << std::endl;
        numPackages++;
    }

    std::cout << numPackages << " Package(s) found" << std::endl;
    
    return S_OK;
}

HRESULT MsixRequest::ProcessAddRequest()
{
    PCWSTR currentHandlerName = PopulatePackageInfo::HandlerName;

    while (currentHandlerName != nullptr)
    {
        TraceLoggingWrite(g_MsixTraceLoggingProvider,
            "Executing handler",
            TraceLoggingValue(currentHandlerName, "HandlerName"));

        HandlerInfo currentHandler = AddHandlers[currentHandlerName];
        AutoPtr<IPackageHandler> handler;
        RETURN_IF_FAILED(currentHandler.create(this, &handler));
        HRESULT hr = handler->ExecuteForAddRequest();
        if (FAILED(hr))
        {
            m_msixResponse->SetErrorCode(hr);
            //call cancel and return
            return hr;
        }
        else
        {
            currentHandlerName = currentHandler.nextHandler;
        }
    }

    return S_OK;
}

HRESULT MsixRequest::ProcessRemoveRequest()
{
    // Run PopulatePackageInfo separately - if it fails (for instance, if the package is not found) it IS fatal.
    AutoPtr<IPackageHandler> handler;
    RETURN_IF_FAILED(PopulatePackageInfo::CreateHandler(this, &handler));
    RETURN_IF_FAILED(handler->ExecuteForRemoveRequest());

    PCWSTR currentHandlerName = CreateAndShowUI::HandlerName;

    while (currentHandlerName != nullptr)
    {
        TraceLoggingWrite(g_MsixTraceLoggingProvider,
            "Executing handler",
            TraceLoggingValue(currentHandlerName, "HandlerName"));

        HandlerInfo currentHandler = RemoveHandlers[currentHandlerName];
        AutoPtr<IPackageHandler> handler;
        RETURN_IF_FAILED(currentHandler.create(this, &handler));
        HRESULT hrExecute = handler->ExecuteForRemoveRequest();
        if (FAILED(hrExecute))
        {
            TraceLoggingWrite(g_MsixTraceLoggingProvider,
                "Handler failed -- removal is best effort so error is non-fatal",
                TraceLoggingLevel(WINEVENT_LEVEL_WARNING),
                TraceLoggingValue(currentHandlerName, "HandlerName"),
                TraceLoggingValue(hrExecute, "HR"));
        }

        currentHandlerName = currentHandler.nextHandler;
    }

    return S_OK;
}

void MsixRequest::SetUI(UI * ui)
{
    m_UI = ui;
}

void MsixRequest::SetPackageInfo(PackageInfo* packageInfo) 
{
    m_packageInfo = packageInfo;
}

/*void MsixRequest::SetMsixResponse(MsixResponse* msixResponse)
{
    m_msixResponse = msixResponse;
}*/