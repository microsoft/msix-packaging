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
#include "PrepareDevirtualizedRegistry.hpp"
#include "WriteDevirtualizedRegistry.hpp"
#include "FirewallRules.hpp"

#include "Constants.hpp"

using namespace std;
using namespace MsixCoreLib;

enum ErrorHandlingMode
{
    IgnoreAndProcessNextHandler = 0,    // ignore the error and proceed with nextHandler as if the current handler were successful
    ExecuteErrorHandler = 1,            // go to errorHandler instead of nextHandler
    ReturnError = 2,                    // return immediately, do not process any further handlers
};

struct AddHandlerInfo
{
    CreateHandler create;
    PCWSTR nextHandler;
    ErrorHandlingMode errorMode;
    PCWSTR errorHandler;
};

struct RemoveHandlerInfo
{
    CreateHandler create;
    PCWSTR nextHandler;
    ErrorHandlingMode errorMode;
};

std::map<PCWSTR, AddHandlerInfo> AddHandlers =
{
    //HandlerName                               Function to create                            NextHandler (on success)                   ErrorHandlingMode    ErrorHandler (when ExecuteErrorHandler)
    {PopulatePackageInfo::HandlerName,          {PopulatePackageInfo::CreateHandler,          ValidateTargetDeviceFamily::HandlerName,   ReturnError,         nullptr}},
    {ValidateTargetDeviceFamily::HandlerName,   {ValidateTargetDeviceFamily::CreateHandler,   ProcessPotentialUpdate::HandlerName,       ReturnError,         nullptr}},
    {ProcessPotentialUpdate::HandlerName,       {ProcessPotentialUpdate::CreateHandler,       Extractor::HandlerName,                    ReturnError,         nullptr}},
    {Extractor::HandlerName,                    {Extractor::CreateHandler,                    PrepareDevirtualizedRegistry::HandlerName, ExecuteErrorHandler, ErrorHandler::HandlerName}},
    {PrepareDevirtualizedRegistry::HandlerName, {PrepareDevirtualizedRegistry::CreateHandler, WriteDevirtualizedRegistry::HandlerName,   ExecuteErrorHandler, ErrorHandler::HandlerName}},
    {WriteDevirtualizedRegistry::HandlerName,   {WriteDevirtualizedRegistry::CreateHandler,   StartMenuLink::HandlerName,                ExecuteErrorHandler, ErrorHandler::HandlerName}},
    {StartMenuLink::HandlerName,                {StartMenuLink::CreateHandler,                AddRemovePrograms::HandlerName,            ExecuteErrorHandler, ErrorHandler::HandlerName}},
    {AddRemovePrograms::HandlerName,            {AddRemovePrograms::CreateHandler,            Protocol::HandlerName,                     ExecuteErrorHandler, ErrorHandler::HandlerName}},
    {Protocol::HandlerName,                     {Protocol::CreateHandler,                     ComInterface::HandlerName,                 ExecuteErrorHandler, ErrorHandler::HandlerName}},
    {ComInterface::HandlerName,                 {ComInterface::CreateHandler,                 ComServer::HandlerName,                    ExecuteErrorHandler, ErrorHandler::HandlerName}},
    {ComServer::HandlerName,                    {ComServer::CreateHandler,                    StartupTask::HandlerName,                  ExecuteErrorHandler, ErrorHandler::HandlerName}},
    {StartupTask::HandlerName,                  {StartupTask::CreateHandler,                  FileTypeAssociation::HandlerName,          ExecuteErrorHandler, ErrorHandler::HandlerName}},
    {FileTypeAssociation::HandlerName,          {FileTypeAssociation::CreateHandler,          FirewallRules::HandlerName,                ExecuteErrorHandler, ErrorHandler::HandlerName}},
    {FirewallRules::HandlerName,                {FirewallRules::CreateHandler,                InstallComplete::HandlerName,              ExecuteErrorHandler, ErrorHandler::HandlerName}},
    {InstallComplete::HandlerName,              {InstallComplete::CreateHandler,              nullptr,                                   ExecuteErrorHandler, ErrorHandler::HandlerName}},
    {ErrorHandler::HandlerName,                 {ErrorHandler::CreateHandler,                 nullptr,                                   ReturnError,         nullptr}},
};

std::map<PCWSTR, RemoveHandlerInfo> RemoveHandlers =
{
    //HandlerName                               Function to create                            NextHandler                                 ErrorHandling
    {PopulatePackageInfo::HandlerName,          {PopulatePackageInfo::CreateHandler,          StartMenuLink::HandlerName,                 ReturnError}},
    {StartMenuLink::HandlerName,                {StartMenuLink::CreateHandler,                AddRemovePrograms::HandlerName,             IgnoreAndProcessNextHandler}},
    {AddRemovePrograms::HandlerName,            {AddRemovePrograms::CreateHandler,            PrepareDevirtualizedRegistry::HandlerName,  IgnoreAndProcessNextHandler}},
    {PrepareDevirtualizedRegistry::HandlerName, {PrepareDevirtualizedRegistry::CreateHandler, Protocol::HandlerName,                      IgnoreAndProcessNextHandler}},
    {Protocol::HandlerName,                     {Protocol::CreateHandler,                     ComInterface::HandlerName,                  IgnoreAndProcessNextHandler}},
    {ComInterface::HandlerName,                 {ComInterface::CreateHandler,                 ComServer::HandlerName,                     IgnoreAndProcessNextHandler}},
    {ComServer::HandlerName,                    {ComServer::CreateHandler,                    StartupTask::HandlerName,                   IgnoreAndProcessNextHandler}},
    {StartupTask::HandlerName,                  {StartupTask::CreateHandler,                  FileTypeAssociation::HandlerName,           IgnoreAndProcessNextHandler}},
    {FileTypeAssociation::HandlerName,          {FileTypeAssociation::CreateHandler,          FirewallRules::HandlerName,                 IgnoreAndProcessNextHandler}},
    {FirewallRules::HandlerName,                {FirewallRules::CreateHandler,                WriteDevirtualizedRegistry::HandlerName,    IgnoreAndProcessNextHandler}},
    {WriteDevirtualizedRegistry::HandlerName,   {WriteDevirtualizedRegistry::CreateHandler,   Extractor::HandlerName,                     IgnoreAndProcessNextHandler}},
    {Extractor::HandlerName,                    {Extractor::CreateHandler,                    nullptr,                                    IgnoreAndProcessNextHandler}},
};

HRESULT MsixRequest::Make(OperationType operationType, IStream * packageStream, std::wstring packageFullName, MSIX_VALIDATION_OPTION validationOption, MsixRequest ** outInstance)
{
    AutoPtr<MsixRequest> instance(new MsixRequest());
    if (instance == nullptr)
    {
        return E_OUTOFMEMORY;
    }
    instance->m_operationType = operationType;
    instance->m_packageStream = packageStream;
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

        AddHandlerInfo currentHandler = AddHandlers[currentHandlerName];
        AutoPtr<IPackageHandler> handler;
        HRESULT hr = currentHandler.create(this, &handler);
        if (SUCCEEDED(hr))
        {
            hr = handler->ExecuteForAddRequest();
        }
        if (FAILED(hr) && currentHandler.errorMode != IgnoreAndProcessNextHandler)
        {
            if (currentHandler.errorMode == ReturnError)
            {
                m_msixResponse->SetErrorStatus(hr, L"Failed to process add request");
                return hr;
            }
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
        HRESULT hr = currentHandler.create(this, &handler);
        if (SUCCEEDED(hr))
        {
            hr = handler->ExecuteForRemoveRequest();
        }

        if (FAILED(hr))
        {
            TraceLoggingWrite(g_MsixTraceLoggingProvider,
                "Handler failed -- if errorMode is IgnoreAndProcessNextHandler(0), this is non-fatal",
                TraceLoggingLevel(WINEVENT_LEVEL_WARNING),
                TraceLoggingValue(currentHandlerName, "HandlerName"),
                TraceLoggingValue(hr, "HR"),
                TraceLoggingUInt32(currentHandler.errorMode, "ErrorMode"));
            if (currentHandler.errorMode == ReturnError)
            {
                m_msixResponse->SetErrorStatus(hr, L"Failed to process add request");
                return hr;
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

void MsixRequest::SetRegistryDevirtualizer(std::shared_ptr<RegistryDevirtualizer> registryDevirualizer)
{
    m_registryDevirtualizer = registryDevirualizer;
}