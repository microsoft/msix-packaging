#include "GeneralUtil.hpp"
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
#include "ComInterface.hpp"
#include "ComServer.hpp"
#include "FileTypeAssociation.hpp"
#include "ProcessPotentialUpdate.hpp"
#include "InstallComplete.hpp"
#include "ErrorHandler.hpp"

#include "Constants.hpp"

// MSIXWindows.hpp define NOMINMAX because we want to use std::min/std::max from <algorithm>
// GdiPlus.h requires a definiton for min and max. Use std namespace *BEFORE* including it.
using namespace std;
#include <GdiPlus.h>
using namespace Win7MsixInstallerLib;
struct HandlerInfo
{
    CreateHandler create;
    PCWSTR nextHandler;
    PCWSTR errorHandler;
};

std::map<PCWSTR, HandlerInfo> AddHandlers =
{
    //HandlerName                         Function to create                      NextHandler                          ErrorHandlerInfo
    {PopulatePackageInfo::HandlerName,    {PopulatePackageInfo::CreateHandler,    ProcessPotentialUpdate::HandlerName, ErrorHandler::HandlerName}},
    {ProcessPotentialUpdate::HandlerName, {ProcessPotentialUpdate::CreateHandler, Extractor::HandlerName,              ErrorHandler::HandlerName}},
    {Extractor::HandlerName,              {Extractor::CreateHandler,              StartMenuLink::HandlerName,          ErrorHandler::HandlerName}},
    {StartMenuLink::HandlerName,          {StartMenuLink::CreateHandler,          AddRemovePrograms::HandlerName,      ErrorHandler::HandlerName}},
    {AddRemovePrograms::HandlerName,      {AddRemovePrograms::CreateHandler,      Protocol::HandlerName,               ErrorHandler::HandlerName}},
    {Protocol::HandlerName,               {Protocol::CreateHandler,               ComInterface::HandlerName,           ErrorHandler::HandlerName}},
    {ComInterface::HandlerName,           {ComInterface::CreateHandler,           ComServer::HandlerName,              ErrorHandler::HandlerName}},
    {ComServer::HandlerName,              {ComServer::CreateHandler,              FileTypeAssociation::HandlerName,    ErrorHandler::HandlerName}},
    {FileTypeAssociation::HandlerName,    {FileTypeAssociation::CreateHandler,    InstallComplete::HandlerName,        ErrorHandler::HandlerName}},
    {InstallComplete::HandlerName,        {InstallComplete::CreateHandler,        nullptr,                             ErrorHandler::HandlerName}},
    {ErrorHandler::HandlerName,           {ErrorHandler::CreateHandler,           nullptr,                             nullptr}},
};

std::map<PCWSTR, HandlerInfo> RemoveHandlers =
{
    //HandlerName                       Function to create                   NextHandler
    {PopulatePackageInfo::HandlerName,  {PopulatePackageInfo::CreateHandler, StartMenuLink::HandlerName}},
    {StartMenuLink::HandlerName,        {StartMenuLink::CreateHandler,       AddRemovePrograms::HandlerName}},
    {AddRemovePrograms::HandlerName,    {AddRemovePrograms::CreateHandler,   Protocol::HandlerName}},
    {Protocol::HandlerName,             {Protocol::CreateHandler,            ComInterface::HandlerName}},
    {ComInterface::HandlerName,         {ComInterface::CreateHandler,        ComServer::HandlerName}},
    {ComServer::HandlerName,            {ComServer::CreateHandler,           FileTypeAssociation::HandlerName}},
    {FileTypeAssociation::HandlerName,  {FileTypeAssociation::CreateHandler, Extractor::HandlerName}},
    {Extractor::HandlerName,            {Extractor::CreateHandler,           nullptr}},
};

HRESULT MsixRequest::Make(OperationType operationType, const std::wstring & packageFilePath, std::wstring packageFullName, MSIX_VALIDATION_OPTION validationOption, MsixRequest ** outInstance)
{
    AutoPtr<MsixRequest> instance(new MsixRequest());
    if (instance == nullptr)
    {
        return E_OUTOFMEMORY;
    }
    instance->m_operationType = operationType;
    instance->m_packageFilePath = packageFilePath;
    instance->m_packageFullName = packageFullName;
    instance->m_validationOptions = validationOption;
    RETURN_IF_FAILED(FilePathMappings::GetInstance().GetInitializationResult());

    //Set MsixResponse
    instance->m_msixResponse = std::make_shared<MsixResponse>();

    *outInstance = instance.Detach();

    return S_OK;
}

HRESULT MsixRequest::ProcessRequest()
{
    m_msixResponse->Update(InstallationStep::InstallationStepStarted, 0);

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
    default:
        return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
    }

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
        auto hrExecute = currentHandler.create(this, &handler);
        if (FAILED(hrExecute))
        {
            if (handler->IsMandatoryForAddRequest())
            {
                m_msixResponse->SetErrorStatus(hrExecute, L"Can't create the handler " + std::wstring(currentHandlerName));
                return hrExecute;
            }
        }

        hrExecute = handler->ExecuteForAddRequest();
        if (FAILED(hrExecute))
        {
            m_msixResponse->SetErrorStatus(hrExecute, L"Can't execute the handler " + std::wstring(currentHandlerName));
            currentHandlerName = currentHandler.errorHandler;
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
    PCWSTR currentHandlerName = PopulatePackageInfo::HandlerName;
    while (currentHandlerName != nullptr)
    {
        TraceLoggingWrite(g_MsixTraceLoggingProvider,
            "Executing handler",
            TraceLoggingValue(currentHandlerName, "HandlerName"));

        HandlerInfo currentHandler = RemoveHandlers[currentHandlerName];
        AutoPtr<IPackageHandler> handler;
        HRESULT hrExecute = currentHandler.create(this, &handler);
        if (FAILED(hrExecute))
        {
            if (handler->IsMandatoryForAddRequest())
            {
                m_msixResponse->SetErrorStatus(hrExecute, L"Can't create the handler " + std::wstring(currentHandlerName));
                m_msixResponse->Update(InstallationStep::InstallationStepError, 0);
                return hrExecute;
            }
        }

        hrExecute = handler->ExecuteForRemoveRequest();
        if (FAILED(hrExecute))
        {
            TraceLoggingWrite(g_MsixTraceLoggingProvider,
                "Handler failed -- removal is best effort so error is non-fatal",
                TraceLoggingLevel(WINEVENT_LEVEL_WARNING),
                TraceLoggingValue(currentHandlerName, "HandlerName"),
                TraceLoggingValue(hrExecute, "HR"));
            if (handler->IsMandatoryForRemoveRequest())
            {
                m_msixResponse->SetErrorStatus(hrExecute, L"Can't execute the handler " + std::wstring(currentHandlerName));
                return hrExecute;
            }
        }

        currentHandlerName = currentHandler.nextHandler;
    }

    return S_OK;
}

void MsixRequest::SetPackageInfo(std::shared_ptr<PackageBase> packageInfo)
{
    m_packageInfo = packageInfo;
}


std::wstring MsixRequest::GetPackageDirectoryPath()
{
    if (m_packageInfo == nullptr)
        return nullptr;

    return FilePathMappings::GetInstance().GetMsix7Directory() + m_packageInfo->GetPackageFullName();
}