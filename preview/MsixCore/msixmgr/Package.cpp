#include <windows.h>

#include "Package.hpp"
#include "FilePaths.hpp"
#include "GeneralUtil.hpp"
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

HRESULT PackageBase::ParseManifest(IMsixElement* element)
{
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

    ComPtr<IMsixElementEnumerator> visualElementsEnum;
    RETURN_IF_FAILED(applicationElement->GetElements(L"*[local-name()='VisualElements']", &visualElementsEnum));
    RETURN_IF_FAILED(visualElementsEnum->GetHasCurrent(&hc));
    if (!hc)
    {
        TraceLoggingWrite(g_MsixTraceLoggingProvider,
            "No DisplayName Found",
            TraceLoggingLevel(WINEVENT_LEVEL_ERROR));
        return E_NOT_SET;
    }

    ComPtr<IMsixElement> visualElementsElement;
    RETURN_IF_FAILED(visualElementsEnum->GetCurrent(&visualElementsElement));

    Text<wchar_t> displayName;
    RETURN_IF_FAILED(visualElementsElement->GetAttributeValue(L"DisplayName", &displayName));
    m_displayName = displayName.Get();

    Text<WCHAR> logo;
    RETURN_IF_FAILED(visualElementsElement->GetAttributeValue(L"Square150x150Logo", &logo));
    m_relativeLogoPath = logo.Get();
    return S_OK;

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

    // By default the PSF information is in config.json, but could be in a different json file.
    for (auto& p : std::experimental::filesystem::directory_iterator(GetPackageDirectoryPath()))
    {
        if (std::experimental::filesystem::is_regular_file(p.path()) && CaseInsensitiveEquals(p.path().extension(), L".json"))
        {
            // parse the file and see if it has info we need.
            std::wstring jsonFile = p.path();
            std::ifstream ifs(jsonFile);
            IStreamWrapper isw(ifs);

            Document doc;
            doc.ParseStream(isw);

            if (doc.HasParseError())
            {
                //return error
            }

            const Value& apps = doc["applications"];

            for (Value::ConstValueIterator itr = apps.Begin(); itr != apps.End(); ++itr)
            {
                std::string id = (*itr)["id"].GetString();
                std::wstring jsonApplicationId;
                jsonApplicationId.assign(id.begin(), id.end());

                //ApplicationId in json should match applicationId from the xml manifest. Right now, we only process first application from the xml manifest anyway
                if (CaseInsensitiveEquals(jsonApplicationId, m_applicationId))
                {
                    std::string executable = (*itr)["executable"].GetString();
                    std::wstring psfExecutable;
                    psfExecutable.assign(executable.begin(), executable.end());

                    std::string arguments = (*itr)["arguments"].GetString();
                    std::wstring psfArguements;
                    psfArguements.assign(arguments.begin(), arguments.end());

                    std::string workingDirectory = (*itr)["workingDirectory"].GetString();
                    std::wstring psfWorkingDirectory;
                    psfWorkingDirectory.assign(workingDirectory.begin(), workingDirectory.end());

                    m_executionInfo.commandLineArguments = psfArguements;

                    // resolve the given paths from json into full paths.
                    m_executionInfo.resolvedExecutableFilePath = FilePathMappings::GetInstance().GetExecutablePath(psfExecutable, m_packageFullName.c_str());
                    m_executionInfo.workingDirectory = FilePathMappings::GetInstance().GetExecutablePath(psfWorkingDirectory, m_packageFullName.c_str());

                    const Value& startScript = (*itr)["startScript"];
                    std::string scriptPath = startScript["scriptPath"].GetString();
                    std::wstring psfScriptPath;
                    psfScriptPath.assign(scriptPath.begin(), scriptPath.end());
                    m_scriptSettings.scriptPath = psfScriptPath;

                    bool showWindow = startScript["showWindow"].GetBool();
                    m_scriptSettings.showWindow = showWindow;

                    TraceLoggingWrite(g_MsixTraceLoggingProvider,
                        "PSF redirection",
                        TraceLoggingWideString(m_executionInfo.resolvedExecutableFilePath.c_str(), "Resolved PSF executable"),
                        TraceLoggingWideString(m_executionInfo.workingDirectory.c_str(), "WorkingDirectory"),
                        TraceLoggingWideString(m_executionInfo.commandLineArguments.c_str(), "Arguments"),
                        TraceLoggingWideString(m_scriptSettings.scriptPath.c_str(), "StartScript - ScriptPath"));
                }
            }
        }
    }

    return S_OK;
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

    m_publisherName = m_publisher.substr(m_publisher.find_first_of(L"=") + 1,
        m_publisher.find_first_of(L",") - m_publisher.find_first_of(L"=") - 1);

    RETURN_IF_FAILED(manifestId->GetVersion(&m_version));

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