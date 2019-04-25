#include <windows.h>

#include <shlobj_core.h>
#include <CommCtrl.h>

#include "FilePaths.hpp"
#include "ComServer.hpp"
#include "GeneralUtil.hpp"
#include "Constants.hpp"
#include <TraceLoggingProvider.h>
#include "MsixTraceLoggingProvider.hpp"
using namespace MsixCoreLib;

const PCWSTR ComServer::HandlerName = L"ComServer";

HRESULT ComServer::ExecuteForAddRequest()
{
    for (auto exeServer = m_exeServers.begin(); exeServer != m_exeServers.end(); ++exeServer)
    {
        RETURN_IF_FAILED(ProcessExeServerForAdd(*exeServer));
    }

    for (auto progId = m_progIds.begin(); progId != m_progIds.end(); ++progId)
    {
        RETURN_IF_FAILED(ProcessProgIdForAdd(*progId));
    }

    return S_OK;
}

HRESULT ComServer::ExecuteForRemoveRequest()
{
    for (auto exeServer = m_exeServers.begin(); exeServer != m_exeServers.end(); ++exeServer)
    {
        RETURN_IF_FAILED(ProcessExeServerForRemove(*exeServer));
    }

    for (auto progId = m_progIds.begin(); progId != m_progIds.end(); ++progId)
    {
        RETURN_IF_FAILED(ProcessProgIdForRemove(*progId));
    }

    return S_OK;
}

HRESULT ComServer::ProcessExeServerForAdd(ExeServer& exeServer)
{
    RegistryKey clsidKey;
    RETURN_IF_FAILED(m_classesKey.CreateSubKey(clsidKeyName.c_str(), KEY_WRITE, &clsidKey));

    for (auto exeServerClass = exeServer.classes.begin(); exeServerClass != exeServer.classes.end(); ++exeServerClass)
    {
        RegistryKey classIdKey;
        RETURN_IF_FAILED(clsidKey.CreateSubKey(exeServerClass->id.c_str(), KEY_WRITE, &classIdKey));
        RETURN_IF_FAILED(classIdKey.SetStringValue(L"", exeServerClass->displayName));

        std::wstring executableFullPath = FilePathMappings::GetInstance().GetExecutablePath(exeServer.executable, m_msixRequest->GetPackageInfo()->GetPackageFullName().c_str());
        RegistryKey localServerKey;
        RETURN_IF_FAILED(classIdKey.CreateSubKey(localServerKeyName.c_str(), KEY_WRITE, &localServerKey));
        RETURN_IF_FAILED(localServerKey.SetStringValue(L"", executableFullPath));

        RegistryKey progIdKey;
        RETURN_IF_FAILED(classIdKey.CreateSubKey(progIdKeyName.c_str(), KEY_WRITE, &progIdKey));
        RETURN_IF_FAILED(progIdKey.SetStringValue(L"", exeServerClass->progId));

        RegistryKey versionIndependentProgIdKey;
        RETURN_IF_FAILED(classIdKey.CreateSubKey(versionIndependentProgIdKeyName.c_str(), KEY_WRITE, &versionIndependentProgIdKey));
        RETURN_IF_FAILED(versionIndependentProgIdKey.SetStringValue(L"", exeServerClass->versionIndependentProgId));

        RegistryKey auxUserTypeKey;
        RETURN_IF_FAILED(classIdKey.CreateSubKey(auxUserTypeKeyName.c_str(), KEY_WRITE, &auxUserTypeKey));
        if (!exeServerClass->shortDisplayName.empty())
        {
            RegistryKey shortDisplayNameKey;
            RETURN_IF_FAILED(auxUserTypeKey.CreateSubKey(shortDisplayNameKeyName.c_str(), KEY_WRITE, &shortDisplayNameKey));
            RETURN_IF_FAILED(shortDisplayNameKey.SetStringValue(L"", exeServerClass->shortDisplayName));
        }

        if (!exeServerClass->enableOleDefaultHandler.empty())
        {
            RegistryKey inprocHandlerKey;
            RETURN_IF_FAILED(classIdKey.CreateSubKey(inprocHandlerKeyName.c_str(), KEY_WRITE, &inprocHandlerKey));
            RETURN_IF_FAILED(inprocHandlerKey.SetStringValue(L"", defaultInprocHandler));
        }
        
        if (!exeServerClass->insertableObject.empty())
        {
            RegistryKey insertableObjectKey;
            RETURN_IF_FAILED(classIdKey.CreateSubKey(insertableObjectKeyName.c_str(), KEY_WRITE, &insertableObjectKey));
        }

        if (!exeServerClass->autoConvertTo.empty())
        {
            RETURN_IF_FAILED(classIdKey.SetStringValue(autoConvertToValueName.c_str(), exeServerClass->autoConvertTo));
        }

        if (!exeServerClass->implementedCategories.empty())
        {
            RegistryKey implementedCategoriesKey;
            RETURN_IF_FAILED(classIdKey.CreateSubKey(implementedCategoriesKeyName.c_str(), KEY_WRITE, &implementedCategoriesKey));

            for (auto category = exeServerClass->implementedCategories.begin(); category != exeServerClass->implementedCategories.end(); ++category)
            {
                RegistryKey categoryKey;
                RETURN_IF_FAILED(implementedCategoriesKey.CreateSubKey(category->c_str(), KEY_WRITE, &categoryKey));
            }
        }

        if (!exeServerClass->conversionReadableFormat.empty())
        {
            RegistryKey readableKey;
            RETURN_IF_FAILED(classIdKey.CreateSubKey(readableKeyName.c_str(), KEY_WRITE, &readableKey));
            RETURN_IF_FAILED(readableKey.SetStringValue(L"", exeServerClass->conversionReadableFormat));
        }

        if (!exeServerClass->conversionReadWritableFormat.empty())
        {
            RegistryKey readWritableKey;
            RETURN_IF_FAILED(classIdKey.CreateSubKey(readWritableKeyName.c_str(), KEY_WRITE, &readWritableKey));
            RETURN_IF_FAILED(readWritableKey.SetStringValue(L"", exeServerClass->conversionReadWritableFormat));
        }

        if (!exeServerClass->dataFormats.empty() && !exeServerClass->defaultFileDataFormat.empty())
        {
            RegistryKey dataFormatsKey;
            RETURN_IF_FAILED(classIdKey.CreateSubKey(dataFormatsKeyName.c_str(), KEY_WRITE, &dataFormatsKey));

            RegistryKey defaultFileKey;
            RETURN_IF_FAILED(dataFormatsKey.CreateSubKey(defaultFileKeyName.c_str(), KEY_WRITE, &defaultFileKey));
            RETURN_IF_FAILED(defaultFileKey.SetStringValue(L"", exeServerClass->defaultFileDataFormat));

            RegistryKey getSetKey;
            RETURN_IF_FAILED(dataFormatsKey.CreateSubKey(getSetKeyName.c_str(), KEY_WRITE, &getSetKey));

            int i = 0;
            for (auto dataFormat = exeServerClass->dataFormats.begin(); dataFormat != exeServerClass->dataFormats.end(); ++i, ++dataFormat)
            {
                RegistryKey formatKey;
                RETURN_IF_FAILED(getSetKey.CreateSubKey(std::to_wstring(i).c_str(), KEY_WRITE, &formatKey));
                RETURN_IF_FAILED(formatKey.SetStringValue(L"", *dataFormat));
            }
        }

        if (!exeServerClass->miscStatusOleMiscFlag.empty())
        {
            RegistryKey miscStatusKey;
            RETURN_IF_FAILED(classIdKey.CreateSubKey(miscStatusKeyName.c_str(), KEY_WRITE, &miscStatusKey));
            RETURN_IF_FAILED(miscStatusKey.SetStringValue(L"", exeServerClass->miscStatusOleMiscFlag));

            for (auto aspect = exeServerClass->miscStatusAspects.begin(); aspect != exeServerClass->miscStatusAspects.end(); ++aspect)
            {
                RegistryKey aspectKey;
                RETURN_IF_FAILED(miscStatusKey.CreateSubKey(aspect->type.c_str(), KEY_WRITE, &aspectKey));
                RETURN_IF_FAILED(aspectKey.SetStringValue(L"", aspect->oleMiscFlag));
            }
        }

        if (!exeServerClass->verbs.empty())
        {
            RegistryKey verbKey;
            RETURN_IF_FAILED(classIdKey.CreateSubKey(verbKeyName.c_str(), KEY_WRITE, &verbKey));

            for (auto verb = exeServerClass->verbs.begin(); verb != exeServerClass->verbs.end(); ++verb)
            {
                RegistryKey specificVerbKey;
                RETURN_IF_FAILED(verbKey.CreateSubKey(verb->id.c_str(), KEY_WRITE, &specificVerbKey));
                RETURN_IF_FAILED(specificVerbKey.SetStringValue(L"", verb->verb));
            }
        }

        if (!exeServerClass->defaultIcon.empty())
        {
            RegistryKey defaultIconKey;
            RETURN_IF_FAILED(classIdKey.CreateSubKey(defaultIconKeyName.c_str(), KEY_WRITE, &defaultIconKey));
            RETURN_IF_FAILED(defaultIconKey.SetStringValue(L"", exeServerClass->defaultIcon));
        }

        if (!exeServerClass->toolboxBitmap.empty())
        {
            RegistryKey toolboxBitmapKey;
            RETURN_IF_FAILED(classIdKey.CreateSubKey(toolboxBitmapKeyName.c_str(), KEY_WRITE, &toolboxBitmapKey));
            RETURN_IF_FAILED(toolboxBitmapKey.SetStringValue(L"", exeServerClass->toolboxBitmap));
        }
    }

    return S_OK;
}

HRESULT ComServer::ProcessProgIdForAdd(ProgId& progId)
{
    RegistryKey progIdKey;
    RETURN_IF_FAILED(m_classesKey.CreateSubKey(progId.id.c_str(), KEY_WRITE, &progIdKey));

    if (!progId.clsid.empty())
    {
        RegistryKey clsidKey;
        RETURN_IF_FAILED(progIdKey.CreateSubKey(clsidKeyName.c_str(), KEY_WRITE, &clsidKey));
        RETURN_IF_FAILED(clsidKey.SetStringValue(L"", progId.clsid));

        for (auto exeServer = m_exeServers.begin(); exeServer != m_exeServers.end(); ++exeServer)
        {
            for (auto exeServerClass = exeServer->classes.begin(); exeServerClass != exeServer->classes.end(); ++exeServerClass)
            {
                if (exeServerClass->id.compare(progId.clsid) == 0 && !exeServerClass->insertableObject.empty())
                {
                    RegistryKey insertableObjectKey;
                    RETURN_IF_FAILED(progIdKey.CreateSubKey(insertableObjectKeyName.c_str(), KEY_WRITE, &insertableObjectKey));
                }
            }
        }
    }
    else
    {
        RegistryKey curVerKey;
        RETURN_IF_FAILED(progIdKey.CreateSubKey(curVerKeyName.c_str(), KEY_WRITE, &curVerKey));
        RETURN_IF_FAILED(curVerKey.SetStringValue(L"", progId.currentVersion));
    }
    return S_OK;
}

HRESULT ComServer::ProcessExeServerForRemove(ExeServer& exeServer)
{
    RegistryKey clsidKey;
    RETURN_IF_FAILED(m_classesKey.OpenSubKey(clsidKeyName.c_str(), KEY_WRITE, &clsidKey));

    for (auto exeServerClass = exeServer.classes.begin(); exeServerClass != exeServer.classes.end(); ++exeServerClass)
    {
        const HRESULT hrDeleteKey = clsidKey.DeleteTree(exeServerClass->id.c_str());
        if (FAILED(hrDeleteKey))
        {
            TraceLoggingWrite(g_MsixTraceLoggingProvider,
                "Unable to delete clsid for com exeServer",
                TraceLoggingLevel(WINEVENT_LEVEL_WARNING),
                TraceLoggingValue(hrDeleteKey, "HR"),
                TraceLoggingValue(exeServerClass->id.c_str(), "CLSID"));
        }
    }
    return S_OK;
}

HRESULT ComServer::ProcessProgIdForRemove(ProgId& progId)
{
    const HRESULT hrDeleteKey = m_classesKey.DeleteTree(progId.id.c_str());
    if (FAILED(hrDeleteKey))
    {
        TraceLoggingWrite(g_MsixTraceLoggingProvider,
            "Unable to delete progId for comServer",
            TraceLoggingLevel(WINEVENT_LEVEL_WARNING),
            TraceLoggingValue(hrDeleteKey, "HR"),
            TraceLoggingValue(progId.id.c_str(), "CLSID"));
    }

    return S_OK;
}

HRESULT ComServer::ParseManifest()
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

        if (wcscmp(extensionCategory.Get(), ComServerCategoryNameInManifest.c_str()) == 0)
        {
            BOOL hc_exeServer = FALSE;
            ComPtr<IMsixElementEnumerator> exeServerEnum;
            RETURN_IF_FAILED(extensionElement->GetElements(exeServerQuery.c_str(), &exeServerEnum));
            RETURN_IF_FAILED(exeServerEnum->GetHasCurrent(&hc_exeServer));

            while (hc_exeServer)
            {
                ComPtr<IMsixElement> exeServerElement;
                RETURN_IF_FAILED(exeServerEnum->GetCurrent(&exeServerElement));

                RETURN_IF_FAILED(ParseExeServerElement(exeServerElement.Get()));

                RETURN_IF_FAILED(exeServerEnum->MoveNext(&hc_exeServer));
            }

            BOOL hc_progId = FALSE;
            ComPtr<IMsixElementEnumerator> progIdEnum;
            RETURN_IF_FAILED(extensionElement->GetElements(progIdQuery.c_str(), &progIdEnum));
            RETURN_IF_FAILED(progIdEnum->GetHasCurrent(&hc_progId));

            while (hc_progId)
            {
                ComPtr<IMsixElement> progIdElement;
                RETURN_IF_FAILED(progIdEnum->GetCurrent(&progIdElement));

                RETURN_IF_FAILED(ParseProgIdElement(progIdElement.Get()));

                RETURN_IF_FAILED(progIdEnum->MoveNext(&hc_progId));
            }
        }
        RETURN_IF_FAILED(extensionEnum->MoveNext(&hasCurrent));
    }

    return S_OK;
}

HRESULT ComServer::ParseExeServerElement(IMsixElement* exeServerElement)
{
    ExeServer exeServer;

    RETURN_IF_FAILED(GetAttributeValueFromElement(exeServerElement, executableAttribute, exeServer.executable));
    RETURN_IF_FAILED(GetAttributeValueFromElement(exeServerElement, argumentsAttribute, exeServer.arguments));
    RETURN_IF_FAILED(GetAttributeValueFromElement(exeServerElement, displayNameAttribute, exeServer.displayName));
    RETURN_IF_FAILED(GetAttributeValueFromElement(exeServerElement, launchAndActivationPermissionAttribute, exeServer.launchAndActivationPermission));
    
    BOOL hasCurrent = FALSE;
    ComPtr<IMsixElementEnumerator> classesEnum;
    RETURN_IF_FAILED(exeServerElement->GetElements(exeServerClassQuery.c_str(), &classesEnum));
    RETURN_IF_FAILED(classesEnum->GetHasCurrent(&hasCurrent));
    while (hasCurrent)
    {
        ComPtr<IMsixElement> classElement;
        RETURN_IF_FAILED(classesEnum->GetCurrent(&classElement));

        RETURN_IF_FAILED(ParseExeServerClassElement(exeServer, classElement.Get()));
        RETURN_IF_FAILED(classesEnum->MoveNext(&hasCurrent));
    }

    m_exeServers.push_back(exeServer);

    return S_OK;
}

HRESULT ComServer::ParseProgIdElement(IMsixElement* progIdElement)
{
    ProgId progId;

    RETURN_IF_FAILED(GetAttributeValueFromElement(progIdElement, idAttribute, progId.id));
    std::wstring id;
    RETURN_IF_FAILED(GetAttributeValueFromElement(progIdElement, clsidAttribute, id));
    if (!id.empty())
    {
        progId.clsid = GuidFromManifestId(id);
    }
    RETURN_IF_FAILED(GetAttributeValueFromElement(progIdElement, currentVersionAttribute, progId.currentVersion));

    m_progIds.push_back(progId);

    return S_OK;
}

HRESULT ComServer::ParseExeServerClassElement(ExeServer & exeServer, IMsixElement * classElement)
{
    ExeServerClass exeServerClass;

    std::wstring id;
    RETURN_IF_FAILED(GetAttributeValueFromElement(classElement, idAttribute, id));
    exeServerClass.id = GuidFromManifestId(id);
    RETURN_IF_FAILED(GetAttributeValueFromElement(classElement, displayNameAttribute, exeServerClass.displayName));
    RETURN_IF_FAILED(GetAttributeValueFromElement(classElement, progIdAttribute, exeServerClass.progId));
    RETURN_IF_FAILED(GetAttributeValueFromElement(classElement, versionIndependentProgIdAttribute, exeServerClass.versionIndependentProgId));
    RETURN_IF_FAILED(GetAttributeValueFromElement(classElement, insertableObjectAttribute, exeServerClass.insertableObject));
    RETURN_IF_FAILED(GetAttributeValueFromElement(classElement, shortDisplayNameAttribute, exeServerClass.shortDisplayName));
    RETURN_IF_FAILED(GetAttributeValueFromElement(classElement, enableOleDefaultHandlerAttribute, exeServerClass.enableOleDefaultHandler));

    std::wstring autoConvertToId;
    RETURN_IF_FAILED(GetAttributeValueFromElement(classElement, autoConvertToAttribute, autoConvertToId));
    if (!autoConvertToId.empty())
    {
        exeServerClass.autoConvertTo = GuidFromManifestId(autoConvertToId);
    }

    RETURN_IF_FAILED(ParseImplementedCategories(classElement, exeServerClass));
    RETURN_IF_FAILED(ParseConversionFormats(classElement, readableFormatsQuery, exeServerClass.conversionReadableFormat));
    RETURN_IF_FAILED(ParseConversionFormats(classElement, readWritableFormatsQuery, exeServerClass.conversionReadWritableFormat));
    RETURN_IF_FAILED(ParseDataFormats(classElement, exeServerClass));
    RETURN_IF_FAILED(ParseMiscStatus(classElement, exeServerClass));
    RETURN_IF_FAILED(ParseVerbs(classElement, exeServerClass));
    RETURN_IF_FAILED(ParseDefaultIcon(classElement, exeServerClass));
    RETURN_IF_FAILED(ParseToolboxBitmap(classElement, exeServerClass));

    exeServer.classes.push_back(exeServerClass);
    return S_OK;
}

HRESULT ComServer::ParseImplementedCategories(IMsixElement* classElement, ExeServerClass& exeServerClass)
{
    BOOL hasCurrent = FALSE;
    ComPtr<IMsixElementEnumerator> implementedCategoriesEnum;
    RETURN_IF_FAILED(classElement->GetElements(implementedCategoriesQuery.c_str(), &implementedCategoriesEnum));
    RETURN_IF_FAILED(implementedCategoriesEnum->GetHasCurrent(&hasCurrent));
    while (hasCurrent)
    {
        ComPtr<IMsixElement> categoryElement;
        RETURN_IF_FAILED(implementedCategoriesEnum->GetCurrent(&categoryElement));

        std::wstring categoryId;
        RETURN_IF_FAILED(GetAttributeValueFromElement(categoryElement.Get(), idAttribute, categoryId));
        exeServerClass.implementedCategories.push_back(GuidFromManifestId(categoryId));

        RETURN_IF_FAILED(implementedCategoriesEnum->MoveNext(&hasCurrent));
    }

    return S_OK;
}

HRESULT ComServer::ParseConversionFormats(IMsixElement* rootElement, const std::wstring & formatsQuery, std::wstring & formats)
{
    BOOL hasCurrent = FALSE;
    ComPtr<IMsixElementEnumerator> readableFormatsEnum;
    RETURN_IF_FAILED(rootElement->GetElements(formatsQuery.c_str(), &readableFormatsEnum));
    RETURN_IF_FAILED(readableFormatsEnum->GetHasCurrent(&hasCurrent));
    while (hasCurrent)
    {
        ComPtr<IMsixElement> formatElement;
        RETURN_IF_FAILED(readableFormatsEnum->GetCurrent(&formatElement));

        std::wstring formatName;
        RETURN_IF_FAILED(GetAttributeValueFromElement(formatElement.Get(), formatNameAttribute, formatName));

        std::wstring standardFormat;
        RETURN_IF_FAILED(GetAttributeValueFromElement(formatElement.Get(), standardFormatAttribute, standardFormat));

        if (!formats.empty())
        {
            formats += L",";
        }

        if (!formatName.empty())
        {
            formats += formatName;
        }
        else
        {
            formats += standardFormat;
        }

        RETURN_IF_FAILED(readableFormatsEnum->MoveNext(&hasCurrent));
    }

    return S_OK;
}

HRESULT ComServer::ParseDataFormats(IMsixElement* classElement, ExeServerClass& exeServerClass)
{
    BOOL hasCurrent = FALSE;
    ComPtr<IMsixElementEnumerator> dataFormatsEnum;
    RETURN_IF_FAILED(classElement->GetElements(dataFormatsQuery.c_str(), &dataFormatsEnum));
    RETURN_IF_FAILED(dataFormatsEnum->GetHasCurrent(&hasCurrent));
    if (hasCurrent)
    {
        ComPtr<IMsixElement> dataFormatsElement;
        RETURN_IF_FAILED(dataFormatsEnum->GetCurrent(&dataFormatsElement));

        std::wstring defaultFormat;
        RETURN_IF_FAILED(GetAttributeValueFromElement(dataFormatsElement.Get(), defaultFormatNameAttribute, defaultFormat));
        if (defaultFormat.empty())
        {
            RETURN_IF_FAILED(GetAttributeValueFromElement(dataFormatsElement.Get(), defaultStandardFormatAttribute, defaultFormat));
        }
        exeServerClass.defaultFileDataFormat = defaultFormat;

        RETURN_IF_FAILED(ParseDataFormat(dataFormatsElement.Get(), exeServerClass))
    }
    return S_OK;
}

/// Converts the DataFormat direction from the Manifest type to the registry key name for the direction, following the DATADIR enum.
HRESULT ConvertDataFormatDirectionStringToRegistryFlag(std::wstring direction, std::wstring& directionRegistryFlag)
{
    if (direction.compare(L"Get") == 0)
    {
        directionRegistryFlag = L"1";
        return S_OK;
    }
    else if (direction.compare(L"Set") == 0)
    {
        directionRegistryFlag = L"2";
        return S_OK;
    }
    else if (direction.compare(L"GetAndSet") == 0)
    {
        directionRegistryFlag = L"3";
        return S_OK;
    }
    else
    {
        return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    }
}

HRESULT ComServer::ParseDataFormat(IMsixElement* dataFormatsElement, ExeServerClass& exeServerClass)
{
    BOOL hasCurrent = FALSE;
    ComPtr<IMsixElementEnumerator> dataFormatEnum;
    RETURN_IF_FAILED(dataFormatsElement->GetElements(dataFormatQuery.c_str(), &dataFormatEnum));
    RETURN_IF_FAILED(dataFormatEnum->GetHasCurrent(&hasCurrent));
    while (hasCurrent)
    {
        ComPtr<IMsixElement> dataFormatElement;
        RETURN_IF_FAILED(dataFormatEnum->GetCurrent(&dataFormatElement));

        std::wstring aspectFlag;
        RETURN_IF_FAILED(GetAttributeValueFromElement(dataFormatElement.Get(), aspectFlagAttribute, aspectFlag));

        std::wstring mediumFlag;
        RETURN_IF_FAILED(GetAttributeValueFromElement(dataFormatElement.Get(), mediumFlagAttribute, mediumFlag));

        std::wstring direction;
        RETURN_IF_FAILED(GetAttributeValueFromElement(dataFormatElement.Get(), directionAttribute, direction));

        std::wstring directionRegistryFlag;
        RETURN_IF_FAILED(ConvertDataFormatDirectionStringToRegistryFlag(direction, directionRegistryFlag));

        std::wstring formatName;
        RETURN_IF_FAILED(GetAttributeValueFromElement(dataFormatElement.Get(), formatNameAttribute, formatName));
        if (formatName.empty())
        {
            RETURN_IF_FAILED(GetAttributeValueFromElement(dataFormatElement.Get(), standardFormatAttribute, formatName));
        }

        std::wstring dataFormat = formatName + L',' + aspectFlag + L',' + mediumFlag + L',' + directionRegistryFlag;
        exeServerClass.dataFormats.push_back(dataFormat);

        RETURN_IF_FAILED(dataFormatEnum->MoveNext(&hasCurrent));
    }

    return S_OK;
}

HRESULT ComServer::ParseMiscStatus(IMsixElement* classElement, ExeServerClass& exeServerClass)
{
    BOOL hasCurrent = FALSE;
    ComPtr<IMsixElementEnumerator> miscStatusEnum;
    RETURN_IF_FAILED(classElement->GetElements(miscStatusQuery.c_str(), &miscStatusEnum));
    RETURN_IF_FAILED(miscStatusEnum->GetHasCurrent(&hasCurrent));
    if (hasCurrent)
    {
        ComPtr<IMsixElement> miscStatusElement;
        RETURN_IF_FAILED(miscStatusEnum->GetCurrent(&miscStatusElement));

        RETURN_IF_FAILED(GetAttributeValueFromElement(miscStatusElement.Get(), oleMiscFlagAttribute, exeServerClass.miscStatusOleMiscFlag));
        RETURN_IF_FAILED(ParseAspects(miscStatusElement.Get(), exeServerClass));
    }
    return S_OK;
}

/// Converts the aspect type from manifest string to the registry key name for the aspect type, following the DVASPECT enum.
HRESULT ConvertAspectTypeToRegistryName(std::wstring aspect, std::wstring& aspectRegistryName)
{
    if (aspect.compare(L"Content") == 0)
    {
        aspectRegistryName = L"1";
        return S_OK;
    }
    else if (aspect.compare(L"Thumbnail") == 0)
    {
        aspectRegistryName = L"2";
        return S_OK;
    }
    else if (aspect.compare(L"Icon") == 0)
    {
        aspectRegistryName = L"4";
        return S_OK;
    }
    else if (aspect.compare(L"DocPrint") == 0)
    {
        aspectRegistryName = L"8";
        return S_OK;
    }
    else
    {
        return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    }
}

HRESULT  ComServer::ParseAspects(IMsixElement* miscStatusElement, ExeServerClass& exeServerClass)
{
    BOOL hasCurrent = FALSE;
    ComPtr<IMsixElementEnumerator> aspectEnum;
    RETURN_IF_FAILED(miscStatusElement->GetElements(aspectQuery.c_str(), &aspectEnum));
    RETURN_IF_FAILED(aspectEnum->GetHasCurrent(&hasCurrent));
    while (hasCurrent)
    {
        ComPtr<IMsixElement> aspectElement;
        RETURN_IF_FAILED(aspectEnum->GetCurrent(&aspectElement));

        Aspect aspect;
        RETURN_IF_FAILED(GetAttributeValueFromElement(aspectElement.Get(), oleMiscFlagAttribute, aspect.oleMiscFlag));

        std::wstring aspectType;
        RETURN_IF_FAILED(GetAttributeValueFromElement(aspectElement.Get(), typeAttribute, aspectType));
        RETURN_IF_FAILED(ConvertAspectTypeToRegistryName(aspectType, aspect.type));

        exeServerClass.miscStatusAspects.push_back(aspect);
        RETURN_IF_FAILED(aspectEnum->MoveNext(&hasCurrent));
    }

    return S_OK;
}

HRESULT ComServer::ParseVerbs(IMsixElement* classElement, ExeServerClass& exeServerClass)
{
    BOOL hasCurrent = FALSE;
    ComPtr<IMsixElementEnumerator> verbEnum;
    RETURN_IF_FAILED(classElement->GetElements(comVerbQuery.c_str(), &verbEnum));
    RETURN_IF_FAILED(verbEnum->GetHasCurrent(&hasCurrent));
    while (hasCurrent)
    {
        ComPtr<IMsixElement> verbElement;
        RETURN_IF_FAILED(verbEnum->GetCurrent(&verbElement));

        Verb verb;
        RETURN_IF_FAILED(GetAttributeValueFromElement(verbElement.Get(), idAttribute, verb.id));

        std::wstring displayName;
        RETURN_IF_FAILED(GetAttributeValueFromElement(verbElement.Get(), displayNameAttribute, displayName));

        std::wstring appendMenuFlag;
        RETURN_IF_FAILED(GetAttributeValueFromElement(verbElement.Get(), appendMenuFlagAttribute, appendMenuFlag));

        std::wstring oleVerbFlag;
        RETURN_IF_FAILED(GetAttributeValueFromElement(verbElement.Get(), oleVerbFlagAttribute, oleVerbFlag));

        verb.verb = displayName + L"," + appendMenuFlag + L"," + oleVerbFlag;

        exeServerClass.verbs.push_back(verb);
        RETURN_IF_FAILED(verbEnum->MoveNext(&hasCurrent));
    }

    return S_OK;
}

HRESULT ComServer::ParseDefaultIcon(IMsixElement * classElement, ExeServerClass & exeServerClass)
{
    BOOL hasCurrent = FALSE;
    ComPtr<IMsixElementEnumerator> defaultIconEnum;
    RETURN_IF_FAILED(classElement->GetElements(defaultIconQuery.c_str(), &defaultIconEnum));
    RETURN_IF_FAILED(defaultIconEnum->GetHasCurrent(&hasCurrent));
    if (hasCurrent)
    {
        ComPtr<IMsixElement> defaultIconElement;
        RETURN_IF_FAILED(defaultIconEnum->GetCurrent(&defaultIconElement));

        std::wstring path;
        RETURN_IF_FAILED(GetAttributeValueFromElement(defaultIconElement.Get(), pathAttribute, path));

        std::wstring resourceIndex;
        RETURN_IF_FAILED(GetAttributeValueFromElement(defaultIconElement.Get(), resourceIndexAttribute, resourceIndex));

        std::wstring resolvedPath = FilePathMappings::GetInstance().GetExecutablePath(path, m_msixRequest->GetPackageInfo()->GetPackageFullName().c_str());

        exeServerClass.defaultIcon = std::wstring(L"\"") + resolvedPath + std::wstring(L"\",") + resourceIndex;
    }

    return S_OK;
}

HRESULT ComServer::ParseToolboxBitmap(IMsixElement * classElement, ExeServerClass & exeServerClass)
{
    BOOL hasCurrent = FALSE;
    ComPtr<IMsixElementEnumerator> toolboxBitmapEnum;
    RETURN_IF_FAILED(classElement->GetElements(toolboxBitmapQuery.c_str(), &toolboxBitmapEnum));
    RETURN_IF_FAILED(toolboxBitmapEnum->GetHasCurrent(&hasCurrent));
    if (hasCurrent)
    {
        ComPtr<IMsixElement> toolboxBitmapElement;
        RETURN_IF_FAILED(toolboxBitmapEnum->GetCurrent(&toolboxBitmapElement));

        std::wstring path;
        RETURN_IF_FAILED(GetAttributeValueFromElement(toolboxBitmapElement.Get(), pathAttribute, path));

        std::wstring resourceIndex;
        RETURN_IF_FAILED(GetAttributeValueFromElement(toolboxBitmapElement.Get(), resourceIndexAttribute, resourceIndex));

        std::wstring resolvedPath = FilePathMappings::GetInstance().GetExecutablePath(path, m_msixRequest->GetPackageInfo()->GetPackageFullName().c_str());

        exeServerClass.toolboxBitmap = std::wstring(L"\"") + resolvedPath + std::wstring(L"\",") + resourceIndex;
    }

    return S_OK;
}

HRESULT ComServer::CreateHandler(MsixRequest * msixRequest, IPackageHandler ** instance)
{
    std::unique_ptr<ComServer> localInstance(new ComServer(msixRequest));
    if (localInstance == nullptr)
    {
        return E_OUTOFMEMORY;
    }

    RETURN_IF_FAILED(localInstance->m_classesKey.Open(HKEY_CLASSES_ROOT, nullptr, KEY_READ | KEY_WRITE | WRITE_DAC));

    RETURN_IF_FAILED(localInstance->ParseManifest());

    *instance = localInstance.release();

    return S_OK;
}