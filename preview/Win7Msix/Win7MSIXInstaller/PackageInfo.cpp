#include <windows.h>

#include "PackageInfo.hpp"
#include "GeneralUtil.hpp"
#include <TraceLoggingProvider.h>

HRESULT PackageInfo::SetExecutableAndAppIdFromManifestElement(IMsixElement* element)
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
	m_executableFilePath = executablePath.Get();
	m_applicationId = applicationId.Get();

	return S_OK;
}

HRESULT PackageInfo::SetDisplayNameFromManifestElement(IMsixElement* element)
{
	ComPtr<IMsixElementEnumerator> visualElementsEnum;
	RETURN_IF_FAILED(element->GetElements(
		L"/*[local-name()='Package']/*[local-name()='Applications']/*[local-name()='Application']/*[local-name()='VisualElements']",
		&visualElementsEnum));
	BOOL hc = FALSE;
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

	return S_OK;
}

HRESULT PackageInfo::MakeFromManifestReader(IAppxManifestReader * manifestReader, std::wstring msix7DirectoryPath, PackageInfo ** packageInfo)
{
	std::unique_ptr<PackageInfo> instance(new PackageInfo());
	if (instance == nullptr)
	{
		return E_OUTOFMEMORY;
	}

	RETURN_IF_FAILED(instance->SetManifestReader(manifestReader, msix7DirectoryPath));

	*packageInfo = instance.release();

	return S_OK;
}

HRESULT PackageInfo::MakeFromPackageReader(IAppxPackageReader * packageReader, std::wstring msix7DirectoryPath, PackageInfo ** packageInfo)
{
	std::unique_ptr<PackageInfo> instance(new PackageInfo());
	if (instance == nullptr)
	{
		return E_OUTOFMEMORY;
	}

	instance->m_packageReader = packageReader;

	ComPtr<IAppxManifestReader> manifestReader;
	RETURN_IF_FAILED(packageReader->GetManifest(&manifestReader));
	RETURN_IF_FAILED(instance->SetManifestReader(manifestReader.Get(), msix7DirectoryPath));

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

	*packageInfo = instance.release();

	return S_OK;
}

HRESULT PackageInfo::SetManifestReader(IAppxManifestReader * manifestReader, std::wstring msix7DirectoryPath)
{
	m_manifestReader = manifestReader;

	// Also fill other fields that come from the manifest reader
	ComPtr<IAppxManifestPackageId> manifestId;
	RETURN_IF_FAILED(manifestReader->GetPackageId(&manifestId));
	RETURN_IF_FAILED(manifestId->GetPublisher(&m_publisher));
	RETURN_IF_FAILED(manifestId->GetVersion(&m_version));

	Text<WCHAR> packageFullName;
	RETURN_IF_FAILED(manifestId->GetPackageFullName(&packageFullName));
	m_packageFullName = packageFullName.Get();
	m_packageDirectoryPath = msix7DirectoryPath + packageFullName.Get();

	ComPtr<IMsixDocumentElement> domElement;
	RETURN_IF_FAILED(manifestReader->QueryInterface(UuidOfImpl<IMsixDocumentElement>::iid, reinterpret_cast<void**>(&domElement)));

	ComPtr<IMsixElement> element;
	RETURN_IF_FAILED(domElement->GetDocumentElement(&element));

	RETURN_IF_FAILED(SetExecutableAndAppIdFromManifestElement(element.Get()));

	RETURN_IF_FAILED(SetDisplayNameFromManifestElement(element.Get()));

	Text<WCHAR> packageFamilyName;
	RETURN_IF_FAILED(manifestId->GetPackageFamilyName(&packageFamilyName));
	if (!m_applicationId.empty() && packageFamilyName.Get() != NULL)
	{
		m_appUserModelId = std::wstring(packageFamilyName.Get()) + L"!" + m_applicationId;
	}
	return S_OK;
}