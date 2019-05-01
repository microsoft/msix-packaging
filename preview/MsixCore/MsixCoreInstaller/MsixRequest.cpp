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
#include "MsixTraceLoggingProvider.hpp"

// handlers
#include "Extractor.hpp"
#include "StartMenuLink.hpp"
#include "AddRemovePrograms.hpp"
#include "PopulatePackageInfo.hpp"
#include "Protocol.hpp"
#include "ComInterface.hpp"
#include "ComServer.hpp"
#include "StartupTask.hpp"
#include "FileTypeAssociation.hpp"
#include "ProcessPotentialUpdate.hpp"
#include "InstallComplete.hpp"
#include "ErrorHandler.hpp"
#include "ValidateTargetDeviceFamily.hpp"

#include "Constants.hpp"

// MSIXWindows.hpp define NOMINMAX because we want to use std::min/std::max from <algorithm>
// GdiPlus.h requires a definiton for min and max. Use std namespace *BEFORE* including it.
using namespace std;
#include <GdiPlus.h>
using namespace MsixCoreLib;

enum ErrorHandling
{
    IgnoreError = 0,
    ExecuteErrorHandler = 1,
    ExecuteNextHandler = 2, // if returned from function, this explicitly tells us to go to next?
    ReturnError = 3,        // for fatal errors, do not go to errorHandler, return immediately with error
    SkipRestOfHandlers = 4, //early return when detect do-nothing?
};

struct AddHandlerInfo
{
    CreateHandler create;
    PCWSTR nextHandler;
    ErrorHandling errorHandling;
    PCWSTR errorHandler;
};

struct RemoveHandlerInfo
{
    CreateHandler create;
    PCWSTR nextHandler;
    ErrorHandling errorHandling;
};

std::map<PCWSTR, AddHandlerInfo> AddHandlers =
{
    //HandlerName                             Function to create                          NextHandler (on success)                 ErrorHandling        ErrorHandler (when ExecuteErrorHandler)
    {PopulatePackageInfo::HandlerName,        {PopulatePackageInfo::CreateHandler,        ValidateTargetDeviceFamily::HandlerName, ReturnError,         nullptr}},
    {ValidateTargetDeviceFamily::HandlerName, {ValidateTargetDeviceFamily::CreateHandler, ProcessPotentialUpdate::HandlerName,     ExecuteErrorHandler, ErrorHandler::HandlerName}},
    {ProcessPotentialUpdate::HandlerName,     {ProcessPotentialUpdate::CreateHandler,     Extractor::HandlerName,                  ExecuteErrorHandler, ErrorHandler::HandlerName}},
    {Extractor::HandlerName,                  {Extractor::CreateHandler,                  StartMenuLink::HandlerName,              ExecuteErrorHandler, ErrorHandler::HandlerName}},
    {StartMenuLink::HandlerName,              {StartMenuLink::CreateHandler,              AddRemovePrograms::HandlerName,          ExecuteErrorHandler, ErrorHandler::HandlerName}},
    {AddRemovePrograms::HandlerName,          {AddRemovePrograms::CreateHandler,          Protocol::HandlerName,                   ExecuteErrorHandler, ErrorHandler::HandlerName}},
    {Protocol::HandlerName,                   {Protocol::CreateHandler,                   ComInterface::HandlerName,               ExecuteErrorHandler, ErrorHandler::HandlerName}},
    {ComInterface::HandlerName,               {ComInterface::CreateHandler,               ComServer::HandlerName,                  ExecuteErrorHandler, ErrorHandler::HandlerName}},
    {ComServer::HandlerName,                  {ComServer::CreateHandler,                  StartupTask::HandlerName,                ExecuteErrorHandler, ErrorHandler::HandlerName}},
    {StartupTask::HandlerName,                {StartupTask::CreateHandler,                FileTypeAssociation::HandlerName,        ExecuteErrorHandler, ErrorHandler::HandlerName}},
    {FileTypeAssociation::HandlerName,        {FileTypeAssociation::CreateHandler,        InstallComplete::HandlerName,            ExecuteErrorHandler, ErrorHandler::HandlerName}},
    {InstallComplete::HandlerName,            {InstallComplete::CreateHandler,            nullptr,                                 ExecuteErrorHandler, ErrorHandler::HandlerName}},
    {ErrorHandler::HandlerName,               {ErrorHandler::CreateHandler,               nullptr,                                 IgnoreError,         nullptr}},
};

std::map<PCWSTR, RemoveHandlerInfo> RemoveHandlers =
{
    //HandlerName                       Function to create                   NextHandler                        ErrorHandling
    {PopulatePackageInfo::HandlerName,  {PopulatePackageInfo::CreateHandler, StartMenuLink::HandlerName,        ReturnError}},
    {StartMenuLink::HandlerName,        {StartMenuLink::CreateHandler,       AddRemovePrograms::HandlerName,    IgnoreError}},
    {AddRemovePrograms::HandlerName,    {AddRemovePrograms::CreateHandler,   Protocol::HandlerName,             IgnoreError}},
    {Protocol::HandlerName,             {Protocol::CreateHandler,            ComInterface::HandlerName,         IgnoreError}},
    {ComInterface::HandlerName,         {ComInterface::CreateHandler,        ComServer::HandlerName,            IgnoreError}},
    {ComServer::HandlerName,            {ComServer::CreateHandler,           StartupTask::HandlerName,          IgnoreError}},
    {StartupTask::HandlerName,          {StartupTask::CreateHandler,         FileTypeAssociation::HandlerName,  IgnoreError}},
    {FileTypeAssociation::HandlerName,  {FileTypeAssociation::CreateHandler, Extractor::HandlerName,            IgnoreError}},
    {Extractor::HandlerName,            {Extractor::CreateHandler,           nullptr,                           IgnoreError}},
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
        HRESULT hr = ProcessAddRequest();
        if (FAILED(hr))
        {
            m_msixResponse->SetErrorStatus(hr, L"Failed to process add request");
        }
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

        AddHandlerInfo currentHandler = AddHandlers[currentHandlerName];
        AutoPtr<IPackageHandler> handler;
        RETURN_IF_FAILED(currentHandler.create(this, &handler));

        HRESULT hrExecute = S_OK;
        RETURN_IF_FAILED(handler->ExecuteForAddRequest(hrExecute));
        if (FAILED(hrExecute))
        {
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

        RemoveHandlerInfo currentHandler = RemoveHandlers[currentHandlerName];
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

    return FilePathMappings::GetInstance().GetMsixCoreDirectory() + m_packageInfo->GetPackageFullName();
}