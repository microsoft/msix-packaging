#include <windows.h>

#include "Package.hpp"
#include "FilePaths.hpp"
#include "GeneralUtil.hpp"
#include "Constants.hpp"
#include <TraceLoggingProvider.h>
#include "MsixTraceLoggingProvider.hpp"
#include <fstream>
#include <experimental/filesystem> // C++-standard header file name
#include <filesystem> // Microsoft-specific implementation header file name
#include "document.h"
#include "stringbuffer.h"
#include "writer.h"
#include "istreamwrapper.h"
#include "ostreamwrapper.h"

using namespace MsixCoreLib;
using namespace rapidjson;

//
// Gets the stream of a file.
//
// Parameters:
//   package - The package reader for the app package.
//   name - Name of the file.
//   stream - The stream for the file.
//

HRESULT GetStreamFromFile(IAppxPackageReader* package, LPCWCHAR name, IStream** stream)
{
    *stream = nullptr;

    ComPtr<IAppxFilesEnumerator> files;
    RETURN_IF_FAILED(package->GetPayloadFiles(&files));

    BOOL hasCurrent = FALSE;
    RETURN_IF_FAILED(files->GetHasCurrent(&hasCurrent));
    while (hasCurrent)
    {
        ComPtr<IAppxFile> file;
        RETURN_IF_FAILED(files->GetCurrent(&file));
        Text<WCHAR> fileName;
        file->GetName(&fileName);
        if (wcscmp(fileName.Get(), name) == 0)
        {
            RETURN_IF_FAILED(file->GetStream(stream));
            return S_OK;
        }
        RETURN_IF_FAILED(files->MoveNext(&hasCurrent));
    }
    return S_OK;
}

std::wstring MsixCoreLib::PackageBase::GetResolvedExecutableFilePath()
{
    return FilePathMappings::GetInstance().GetExecutablePath(m_relativeExecutableFilePath, m_packageFullName.c_str());
}

std::wstring PackageBase::GetVersion()
{
    return ConvertVersionToString(m_version);
}

HRESULT PackageBase::GetElementTextFromQuery(IMsixElement* element, PCWSTR query, std::wstring & text)
{
    BOOL hc = FALSE;
    ComPtr<IMsixElementEnumerator> elementEnum;
    RETURN_IF_FAILED(element->GetElements(query, &elementEnum));
    RETURN_IF_FAILED(elementEnum->GetHasCurrent(&hc));
    if (!hc)
    {
        TraceLoggingWrite(g_MsixTraceLoggingProvider,
            "Query unexpectedly returned no results",
            TraceLoggingValue(query, "Query"),
            TraceLoggingLevel(WINEVENT_LEVEL_ERROR));
        return E_NOT_SET;
    }

    ComPtr<IMsixElement> queriedElement;
    RETURN_IF_FAILED(elementEnum->GetCurrent(&queriedElement));

    Text<wchar_t> elementText;
    RETURN_IF_FAILED(queriedElement->GetText(&elementText));
    text = elementText.Get();

    return S_OK;
}

HRESULT PackageBase::ParseManifest(IMsixElement* element)
{
    RETURN_IF_FAILED(GetElementTextFromQuery(element, L"/*[local-name()='Package']/*[local-name()='Properties']/*[local-name()='DisplayName']", m_displayName));
    RETURN_IF_FAILED(GetElementTextFromQuery(element, L"/*[local-name()='Package']/*[local-name()='Properties']/*[local-name()='PublisherDisplayName']", m_publisherName));
    RETURN_IF_FAILED(GetElementTextFromQuery(element, L"/*[local-name()='Package']/*[local-name()='Properties']/*[local-name()='Logo']", m_relativeLogoPath));

    BOOL hc = FALSE;
    ComPtr<IMsixElementEnumerator> applicationElementEnum;
    RETURN_IF_FAILED(element->GetElements(
        L"/*[local-name()='Package']/*[local-name()='Applications']/*[local-name()='Application']",
        &applicationElementEnum));
    RETURN_IF_FAILED(applicationElementEnum->GetHasCurrent(&hc));

    if (!hc)
    {
        TraceLoggingWrite(g_MsixTraceLoggingProvider,
            "No Application Found",
            TraceLoggingLevel(WINEVENT_LEVEL_ERROR));
        return E_NOT_SET;
    }

    ComPtr<IMsixElement> applicationElement;
    RETURN_IF_FAILED(applicationElementEnum->GetCurrent(&applicationElement));

    Text<wchar_t> executablePath;
    Text<wchar_t> applicationId;
    RETURN_IF_FAILED(applicationElement->GetAttributeValue(L"Executable", &executablePath));
    RETURN_IF_FAILED(applicationElement->GetAttributeValue(L"Id", &applicationId));
    m_relativeExecutableFilePath = executablePath.Get();

    m_applicationId = applicationId.Get();

    return S_OK;
}

HRESULT PackageBase::ParseManifestCapabilities(IMsixElement* element)
{
    ComPtr<IMsixElementEnumerator> capabilitiesEnum;
    RETURN_IF_FAILED(element->GetElements(
        L"/*[local-name()='Package']/*[local-name()='Capabilities']/*[local-name()='Capability']",
        &capabilitiesEnum));

    BOOL hc = FALSE;
    RETURN_IF_FAILED(capabilitiesEnum->GetHasCurrent(&hc));

    while (hc)
    {
        ComPtr<IMsixElement> capabilityElement;
        RETURN_IF_FAILED(capabilitiesEnum->GetCurrent(&capabilityElement));

        Text<wchar_t> capabilityName;
        RETURN_IF_FAILED(capabilityElement->GetAttributeValue(L"Name", &capabilityName));
        m_capabilities.push_back(capabilityName.Get());

        RETURN_IF_FAILED(capabilitiesEnum->MoveNext(&hc));
    }
    return S_OK;
}

bool GetMemberFromJsonNode(const Value* node, PCSTR memberName, std::wstring & memberValue)
{
    if (!node->HasMember(memberName))
    {
        return false;
    }
    std::string memberValueString = (*node)[memberName].GetString();
    
    memberValue.assign(memberValueString.begin(), memberValueString.end());
    return true;
}

HRESULT MsixCoreLib::PackageBase::ProcessPSFIfNecessary()
{
    m_executionInfo.resolvedExecutableFilePath = FilePathMappings::GetInstance().GetExecutablePath(m_relativeExecutableFilePath, m_packageFullName.c_str());

    if (!CaseInsensitiveIsSubString(m_relativeExecutableFilePath, L"PSFLauncher"))
    {
        // Package doesn't have PSF - nothing to do
        return S_OK;
    }

    TraceLoggingWrite(g_MsixTraceLoggingProvider,
        "Processing PSF redirection",
        TraceLoggingWideString(m_relativeExecutableFilePath.c_str(), "PSF Executable"),
        TraceLoggingWideString(GetPackageDirectoryPath().c_str(), "path"));

    // PSF config information should be in a file called config.json, so look for that file
    std::wstring jsonConfigFile = GetPackageDirectoryPath() + psfConfigFile;
    bool configFileExists = false;
    RETURN_IF_FAILED(FileExists(jsonConfigFile, configFileExists));

    if (!configFileExists)
    {
        TraceLoggingWrite(g_MsixTraceLoggingProvider,
            "Config.json is not found in the directory",
            TraceLoggingWideString(jsonConfigFile.c_str(), "Config.json file path"),
            TraceLoggingLevel(WINEVENT_LEVEL_ERROR));

        return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    }
    else
    {
        // parse the file and see if it has info we need.
        std::ifstream ifs(jsonConfigFile);
        IStreamWrapper isw(ifs);

        Document doc;
        doc.ParseStream(isw);

        if (doc.HasParseError())
        {
            TraceLoggingWrite(g_MsixTraceLoggingProvider,
                "Config.json has a document parsing error",
                TraceLoggingWideString(jsonConfigFile.c_str(), "Config.json file path"),
                TraceLoggingLevel(WINEVENT_LEVEL_ERROR));

            return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
        }
        if (doc.HasMember("applications"))
        {
            const Value& apps = doc["applications"];

            for (Value::ConstValueIterator itr = apps.Begin(); itr != apps.End(); ++itr)
            {
                std::wstring jsonApplicationId;
                bool idExists = GetMemberFromJsonNode(itr, "id", jsonApplicationId);
                if (!idExists)
                {
                    continue;
                }

                TraceLoggingWrite(g_MsixTraceLoggingProvider,
                    "Application id found in config.json file",
                    TraceLoggingWideString(jsonApplicationId.c_str(), "jsonApplicationId"));

                //ApplicationId in json should match applicationId from the xml manifest. Right now, we only process first application from the xml manifest anyway
                if (!CaseInsensitiveEquals(jsonApplicationId, m_applicationId))
                {
                    continue;
                }

                // Fail if endScript is present in current implementation as we do not support those cases yet
                if ((*itr).HasMember("endScript"))
                {
                    TraceLoggingWrite(g_MsixTraceLoggingProvider,
                        "presence of endScript in config.json is not supported currently",
                        TraceLoggingLevel(WINEVENT_LEVEL_ERROR));

                    return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
                }

                std::wstring psfExecutable;
                bool executableExists = GetMemberFromJsonNode(itr, "executable", psfExecutable);
                if (!executableExists)
                {
                    TraceLoggingWrite(g_MsixTraceLoggingProvider,
                        "No executable specified for application",
                        TraceLoggingLevel(WINEVENT_LEVEL_ERROR));

                    return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
                }

                std::wstring psfArguments;
                bool argumentExists = GetMemberFromJsonNode(itr, "arguments", psfArguments);
                
                std::wstring psfWorkingDirectory;
                bool workingDirectoryExists = GetMemberFromJsonNode(itr, "workingDirectory", psfWorkingDirectory);
                
                m_executionInfo.commandLineArguments = psfArguments;

                // resolve the given paths from json into full paths.
                m_executionInfo.resolvedExecutableFilePath = FilePathMappings::GetInstance().GetExecutablePath(psfExecutable, m_packageFullName.c_str());
                m_executionInfo.workingDirectory = FilePathMappings::GetInstance().GetExecutablePath(psfWorkingDirectory, m_packageFullName.c_str());

                if ((*itr).HasMember("startScript"))
                {
                    const Value& startScript = (*itr)["startScript"];

                    std::wstring psfScriptPath;
                    bool scriptPathExists = GetMemberFromJsonNode(&startScript, "scriptPath", psfScriptPath);
                    if (!scriptPathExists)
                    {
                        TraceLoggingWrite(g_MsixTraceLoggingProvider,
                            "No scriptPath specified for PSF script",
                            TraceLoggingLevel(WINEVENT_LEVEL_ERROR));

                        return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
                    }

                    m_scriptSettings.scriptPath = psfScriptPath;

                    if (startScript.HasMember("runOnce"))
                    {
                        bool runOnce = startScript["runOnce"].GetBool();
                        m_scriptSettings.runOnce = runOnce;

                        if (runOnce == false)
                        {
                            TraceLoggingWrite(g_MsixTraceLoggingProvider,
                                "runOnce = false is not supported currently",
                                TraceLoggingBool(runOnce, "runOnce"),
                                TraceLoggingLevel(WINEVENT_LEVEL_ERROR));

                            return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
                        }
                    }

                    if (startScript.HasMember("showWindow"))
                    {
                        bool showWindow = startScript["showWindow"].GetBool();
                        m_scriptSettings.showWindow = showWindow;
                    }

                    if (startScript.HasMember("waitForScriptToFinish"))
                    {
                        bool waitForScriptToFinish = startScript["waitForScriptToFinish"].GetBool();
                        m_scriptSettings.waitForScriptToFinish = waitForScriptToFinish;
                    }
                }

                TraceLoggingWrite(g_MsixTraceLoggingProvider,
                    "PSF redirection",
                    TraceLoggingWideString(m_executionInfo.resolvedExecutableFilePath.c_str(), "Resolved PSF executable"),
                    TraceLoggingWideString(m_executionInfo.workingDirectory.c_str(), "WorkingDirectory"),
                    TraceLoggingWideString(m_executionInfo.commandLineArguments.c_str(), "Arguments"),
                    TraceLoggingWideString(m_scriptSettings.scriptPath.c_str(), "StartScript - ScriptPath"));

                return S_OK;
            }
        }
    }    

    TraceLoggingWrite(g_MsixTraceLoggingProvider,
        "Application id config.json file does not match application id from appxmanifest.xml");

    return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
}

HRESULT Package::MakeFromPackageReader(IAppxPackageReader * packageReader, std::shared_ptr<Package> * packageInfo)
{
    std::shared_ptr<Package> instance = std::make_shared<Package>();
    if (instance == nullptr)
    {
        return E_OUTOFMEMORY;
    }

    instance->m_packageReader = packageReader;

    ComPtr<IAppxManifestReader> manifestReader;
    RETURN_IF_FAILED(packageReader->GetManifest(&manifestReader));
    RETURN_IF_FAILED(instance->SetManifestReader(manifestReader.Get()));

    // Get the number of payload files
    DWORD numberOfPayloadFiles = 0;
    ComPtr<IAppxFilesEnumerator> fileEnum;
    RETURN_IF_FAILED(packageReader->GetPayloadFiles(&fileEnum));

    BOOL hc = FALSE;
    RETURN_IF_FAILED(fileEnum->GetHasCurrent(&hc));
    while (hc)
    {
        numberOfPayloadFiles++;
        RETURN_IF_FAILED(fileEnum->MoveNext(&hc));
    }
    instance->m_numberOfPayloadFiles = numberOfPayloadFiles;

    *packageInfo = instance;

    return S_OK;
}

HRESULT PackageBase::SetManifestReader(IAppxManifestReader * manifestReader)
{
    m_manifestReader = manifestReader;

    // Also fill other fields that come from the manifest reader
    ComPtr<IAppxManifestPackageId> manifestId;
    RETURN_IF_FAILED(manifestReader->GetPackageId(&manifestId));

    Text<WCHAR> publisher;
    RETURN_IF_FAILED(manifestId->GetPublisher(&publisher));
    m_publisher = publisher.Get();

    RETURN_IF_FAILED(manifestId->GetVersion(&m_version));

    RETURN_IF_FAILED(manifestId->GetArchitecture(&m_architecture));

    Text<WCHAR> packageFullName;
    RETURN_IF_FAILED(manifestId->GetPackageFullName(&packageFullName));
    m_packageFullName = packageFullName.Get();
    m_packageFamilyName = GetFamilyNameFromFullName(m_packageFullName);

    ComPtr<IMsixDocumentElement> domElement;
    RETURN_IF_FAILED(manifestReader->QueryInterface(UuidOfImpl<IMsixDocumentElement>::iid, reinterpret_cast<void**>(&domElement)));

    ComPtr<IMsixElement> element;
    RETURN_IF_FAILED(domElement->GetDocumentElement(&element));

    RETURN_IF_FAILED(ParseManifest(element.Get()));

    RETURN_IF_FAILED(ParseManifestCapabilities(element.Get()));

    Text<WCHAR> packageFamilyName;
    RETURN_IF_FAILED(manifestId->GetPackageFamilyName(&packageFamilyName));
    if (!m_applicationId.empty() && packageFamilyName.Get() != NULL)
    {
        m_appUserModelId = std::wstring(packageFamilyName.Get()) + L"!" + m_applicationId;
    }
    return S_OK;
}

std::wstring MsixCoreLib::Package::GetResolvedExecutableFilePath()
{
    return FilePathMappings::GetInstance().GetExecutablePath(m_relativeExecutableFilePath, m_packageFullName.c_str());
}

std::unique_ptr<IStream> Package::GetLogo()
{
    if (m_packageReader.Get() == nullptr)
    {
        return nullptr;
    }

    IStream * logoStream;
    if (GetStreamFromFile(m_packageReader.Get(), m_relativeLogoPath.data(), &logoStream) == S_OK)
    {
        return std::unique_ptr<IStream>(logoStream);
    }
    return nullptr;
}

std::unique_ptr<IStream> InstalledPackage::GetLogo()
{
    auto iconPath = m_packageDirectoryPath + m_relativeLogoPath;
    IStream* stream;
    if (SUCCEEDED(CreateStreamOnFileUTF16(iconPath.c_str(), true, &stream)))
    {
        return std::unique_ptr<IStream>(stream);
    }
    return nullptr;
}


HRESULT InstalledPackage::MakeFromManifestReader(const std::wstring & directoryPath, IAppxManifestReader * manifestReader, std::shared_ptr<InstalledPackage> * packageInfo)
{
    std::shared_ptr<InstalledPackage> instance = std::make_shared<InstalledPackage>();
    if (instance == nullptr)
    {
        return E_OUTOFMEMORY;
    }

    RETURN_IF_FAILED(instance->SetManifestReader(manifestReader));
    instance->m_packageDirectoryPath = directoryPath + L"\\";

    RETURN_IF_FAILED(instance->ProcessPSFIfNecessary());

    *packageInfo = instance;

    return S_OK;
}