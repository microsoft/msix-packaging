#include <windows.h>

#include <shlobj_core.h>
#include <CommCtrl.h>
#include <iostream>
#include <algorithm>

#include "RegistryKey.hpp"
#include "AutoPlay.hpp"
#include "GeneralUtil.hpp"
#include <TraceLoggingProvider.h>
#include "MsixTraceLoggingProvider.hpp"
#include "Constants.hpp"
#include "CryptoProvider.hpp"
#include "Base32Encoding.hpp"
#include "WideString.hpp"
#include <StrSafe.h>

using namespace MsixCoreLib;

const PCWSTR AutoPlay::HandlerName = L"AutoPlay";

HRESULT AutoPlay::ExecuteForAddRequest()
{
    for (auto autoPlay = m_autoPlay.begin(); autoPlay != m_autoPlay.end(); ++autoPlay)
    {
        RETURN_IF_FAILED(ProcessAutoPlayForAdd(*autoPlay));
    }
    return S_OK;
}

HRESULT AutoPlay::ExecuteForRemoveRequest()
{
    return S_OK;
}

HRESULT AutoPlay::ParseManifest()
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
        AutoPlayObject autoPlay;

        ComPtr<IMsixElement> extensionElement;
        RETURN_IF_FAILED(extensionEnum->GetCurrent(&extensionElement));
        Text<wchar_t> extensionCategory;
        RETURN_IF_FAILED(extensionElement->GetAttributeValue(categoryAttribute.c_str(), &extensionCategory));

        if (wcscmp(extensionCategory.Get(), autoPlayContentCategoryNameInManifest.c_str()) == 0)
        {
            autoPlay.autoPlayType = UWPContent;

            BOOL hc_launchAction = FALSE;
            ComPtr<IMsixElementEnumerator> launchActionEnum;
            RETURN_IF_FAILED(extensionElement->GetElements(launchActionQuery.c_str(), &launchActionEnum));
            RETURN_IF_FAILED(launchActionEnum->GetHasCurrent(&hc_launchAction));

            while (hc_launchAction)
            {
                // for each launch action tag
                ComPtr<IMsixElement> launchActionElement;
                RETURN_IF_FAILED(launchActionEnum->GetCurrent(&launchActionElement));

                //verb
                Text<wchar_t> id;
                RETURN_IF_FAILED(launchActionElement->GetAttributeValue(idAttributeName.c_str(), &id));

                //check for isvalidid

                //action
                Text<wchar_t> action;
                RETURN_IF_FAILED(launchActionElement->GetAttributeValue(actionAttributeName.c_str(), &action));

                //content event
                Text<wchar_t> handleEvent;
                RETURN_IF_FAILED(launchActionElement->GetAttributeValue(contentEventAttributeName.c_str(), &handleEvent));

                // The "Provider" is the Application DisplayName
                autoPlay.provider = m_msixRequest->GetPackageInfo()->GetDisplayName();

                // Get the App's app user model id
                autoPlay.appUserModelId = m_msixRequest->GetPackageInfo()->GetId();

                //get the logo
                //autoPlay.defaultIcon =

                //generate prog id
                //StringBuffer progId;
                GenerateProgId(autoPlayContentCategoryNameInManifest.c_str(), id.Get());

                //generate handler name
                std::wstring generatedHandlerName;
                GenerateHandlerName(L"Content", id.Get(), generatedHandlerName);

            }

        }
        else if (wcscmp(extensionCategory.Get(), autoPlayDeviceCategoryNameInManifest.c_str()) == 0)
        {
            autoPlay.autoPlayType = UWPDevice;

            //ParseStandardElement

            // device event

            // Generate ProgID

            //GenerateHandlerName


        }

        else if (wcscmp(extensionCategory.Get(), desktopAppXExtensionCategory.c_str()) == 0)
        {
            // check if content or device element
        }

        m_autoPlay.push_back(autoPlay);
        RETURN_IF_FAILED(extensionEnum->MoveNext(&hasCurrent));
    }

    return S_OK;
}

HRESULT AutoPlay::GenerateProgId(std::wstring categoryName, std::wstring subCategory)
{
    std::wstring packageMoniker = m_msixRequest->GetPackageInfo()->GetPackageFamilyName();
    std::wstring applicationId = m_msixRequest->GetPackageInfo()->GetApplicationId();

    if (packageMoniker.empty() || applicationId.empty() || categoryName.empty())
    {
        return E_INVALIDARG;
    }

    // Constants
    std::wstring AppXPrefix = L"AppX";
    static const size_t MaxProgIDLength = 39;

    // The maximum number of characters we can have as base32 encoded is 32.
    // We arrive at this due to the interface presented by the GetChars function;
    // it is byte only. Ideally, the MaxBase32EncodedStringLength would be
    // 35 [39 total for the progID, minus 4 for the prefix]. 35 characters means
    // a maximum of 22 bytes in the digest [ceil(35*5/8)]. However, 22 bytes of digest
    // actually encodes 36 characters [ceil(22/8*5)]. So we have to reduce the
    // character count of the encoded string. At 33 characters, this translates into
    // a 21 byte buffer. A 21 byte buffer translates into 34 characters. Although
    // this meets the requirements, it is confusing since a 33 character limit
    // results in a 34 character long encoding. In comparison, a 32 character long
    // encoding divides evenly and always results in a 20 byte digest.

    static const size_t MaxBase32EncodedStringLength = 32;

    // The maximum number of bytes the digest can be is 20. We arrive at this by:
    // - The maximum count of characters [without prefix] in the encoding (32):
    // - multiplied by 5 (since each character in base 32 encoding is based off of 5 bits)
    // - divided by 8 (to find how many bytes are required)
    // - The  initial plus 7 is to enable integer math (equivalent of writing ceil)

    static const ULONG MaxByteCountOfDigest = (MaxBase32EncodedStringLength * 5 + 7) / 8;

    HRESULT hr = S_OK;

    // Build the progIdSeed by appending the incoming strings
    // The package moniker and the application ID are case sensitive
    //std::wstring tempProgIDBuffer;

    std::wstring tempProgIDBuilder;
    tempProgIDBuilder.append(packageMoniker);
    std::transform(tempProgIDBuilder.begin(), tempProgIDBuilder.end(), tempProgIDBuilder.begin(), ::tolower);
    tempProgIDBuilder.append(applicationId);

    // The category name and the subcategory are not case sensitive
    // so we should lower case them

    std::wstring tempLowerBuffer;
    tempLowerBuffer.assign(categoryName);
    std::transform(tempLowerBuffer.begin(), tempLowerBuffer.end(), tempLowerBuffer.begin(), ::tolower);
    tempProgIDBuilder.append(tempLowerBuffer);

    if (!subCategory.empty())
    {
        tempLowerBuffer.assign(subCategory);
        std::transform(tempLowerBuffer.begin(), tempLowerBuffer.end(), tempLowerBuffer.begin(), ::tolower);
        tempProgIDBuilder.append(tempLowerBuffer);
    }

    // Create the crypto provider and start the digest / hash
    AutoPtr<CryptoProvider> cryptoProvider;
    RETURN_IF_FAILED(CryptoProvider::Create(&cryptoProvider));
    RETURN_IF_FAILED(cryptoProvider->StartDigest());
    COMMON_BYTES data = { 0 };
    data.length = (ULONG)tempProgIDBuilder.size() * sizeof(WCHAR);
    data.bytes = (LPBYTE)tempProgIDBuilder.c_str();

    RETURN_IF_FAILED(cryptoProvider->DigestData(&data));

    // Grab the crypto digest
    COMMON_BYTES digest = { 0 };
    RETURN_IF_FAILED(cryptoProvider->GetDigest(&digest));

    // Ensure the string buffer has enough capacity
    StringBuffer base32EncodedDigest;
    RETURN_IF_FAILED(base32EncodedDigest.SetCapacity(
        MaxBase32EncodedStringLength));

    // Base 32 encode the bytes of the digest and put them into the string buffer
    ULONG base32EncodedDigestCharCount = 0;
    RETURN_IF_FAILED(Base32Encoding::GetChars(
        digest.bytes,
        GetMinValue(digest.length, MaxByteCountOfDigest),
        MaxBase32EncodedStringLength,
        base32EncodedDigest.GetChars(),
        &base32EncodedDigestCharCount));

    // Set the length of the string buffer to the appropriate value
    RETURN_IF_FAILED(base32EncodedDigest.SetLength(base32EncodedDigestCharCount));

    // ProgID name is formed by appending the encoded digest to the "AppX" prefix string
    //RETURN_IF_FAILED(tempProgIDBuilder.Delete(0, tempProgIDBuilder.GetLength()));
    tempProgIDBuilder.clear();

    tempProgIDBuilder.append(AppXPrefix);
    tempProgIDBuilder.append(base32EncodedDigest.GetString()->chars);

    assert(tempProgIDBuilder.GetLength() <= MaxProgIDLength);

    // Sometimes the app's prog id will change on app update (WWA -> XAML for instance), but we don't want to forget the
    // file associations for that app. Convert the progId if we know that this is one that needs to be preserved.
    //Common::StringBuffer preservedProgId;
    //IfFailedReturn(PreserveSpecificProgIds(tempProgIDBuffer, preservedProgId));

    // Set the return value
    //RETURN_IF_FAILED(generatedProgId.InitializeFromString(tempProgIDBuilder.GetChars()));

    return S_OK;
}

HRESULT AutoPlay::GenerateHandlerName(LPWSTR type, const std::wstring handlerNameSeed, std::wstring generatedHandlerName)
{
    // Constants
    static const ULONG HashedByteCount = 32;      // SHA256 generates 256 hashed bits, which is 32 bytes
    static const ULONG Base32EncodedLength = 52;  // SHA256 generates 256 hashed bits, which is 52 characters after base 32 encoding (5 bits per character)

    std::wstring packageFamilyMoniker = m_msixRequest->GetPackageInfo()->GetPackageFamilyName();
    std::wstring applicationId = m_msixRequest->GetPackageInfo()->GetApplicationId();
    //StringBuffer handlerNameBuffer;
    std::wstring handlerNameBuilder;
    HCRYPTPROV hProv = NULL;
    HCRYPTHASH hHash = NULL;
    DWORD hashLength;
    BYTE bytes[HashedByteCount];
    ULONG base32EncodedDigestCharCount;
    StringBuffer base32EncodedDigest;
    size_t typeLength;

    // First, convert Package Moniker and App Id to StringBuffers for convenience - lowercase the values so that the comparison
    // in future versions or other code will be case insensitive
    handlerNameBuilder.append(packageFamilyMoniker);
    handlerNameBuilder.append(applicationId);
    std::transform(handlerNameBuilder.begin(), handlerNameBuilder.end(), handlerNameBuilder.begin(), ::tolower);

    // Next, SHA256 hash the Package Moniker and Application Id
    if (!CryptAcquireContext(&hProv, nullptr, MS_ENH_RSA_AES_PROV, PROV_RSA_AES, CRYPT_VERIFYCONTEXT))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    if (!CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    if (!CryptHashData(hHash, (BYTE *)handlerNameBuilder.c_str(), (DWORD)handlerNameBuilder.size() * sizeof(wchar_t), 0))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    hashLength = HashedByteCount;
    if (!CryptGetHashParam(hHash, HP_HASHVAL, bytes, &hashLength, 0))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    // Ensure the string buffer has enough capacity for the string and a null terminator
    RETURN_IF_FAILED(base32EncodedDigest.SetCapacity(Base32EncodedLength + 1));

    // Base 32 encode the bytes of the digest and put them into the string buffer
    RETURN_IF_FAILED(Base32Encoding::GetChars(
        bytes,
        HashedByteCount,
        Base32EncodedLength,
        base32EncodedDigest.GetChars(),
        &base32EncodedDigestCharCount));

    // Set the length of the string buffer to the appropriate value
    RETURN_IF_FAILED(base32EncodedDigest.SetLength(base32EncodedDigestCharCount));

    // Find the length of the type string
    RETURN_IF_FAILED(StringCchLength(type, STRSAFE_MAX_CCH, &typeLength));

    // Finally, construct the string
    handlerNameBuilder.clear();
    handlerNameBuilder.append(base32EncodedDigest.GetString()->chars);
    handlerNameBuilder.append(L"!", 1);
    handlerNameBuilder.append(type, (ULONG)typeLength);
    handlerNameBuilder.append(L"!", 1);
    handlerNameBuilder.append(handlerNameSeed);

    // Set the return value
    generatedHandlerName.assign(handlerNameBuilder.c_str());
    std::transform(generatedHandlerName.begin(), generatedHandlerName.end(), generatedHandlerName.begin(), ::tolower);

    return S_OK;
}

HRESULT AutoPlay::ProcessAutoPlayForAdd(AutoPlayObject& autoPlayObject)
{
    /*RegistryKey handlerRootKey;
    RETURN_IF_FAILED(handlerRootKey.Open(HKEY_LOCAL_MACHINE, handlerRootRegKeyName.c_str(),  KEY_WRITE));

    RegistryKey handlerKey;
    RETURN_IF_FAILED(handlerRootKey.CreateSubKey(HandlerName, KEY_WRITE, &handlerKey));

    // Keys associated with all types
    RETURN_IF_FAILED(handlerKey.SetStringValue(L"Action", action));

    RETURN_IF_FAILED(handlerKey.SetStringValue(L"Provider", provider));

    //Get the logo
    RETURN_IF_FAILED(handlerKey.SetStringValue(L"DefaultIcon", provider));

    RegistryKey handleEventRootKey;
    RETURN_IF_FAILED(handleEventRootKey.Open(HKEY_LOCAL_MACHINE, eventHandlerRootRegKeyName.c_str(), KEY_WRITE));

    RegistryKey handleEventKey;
    RETURN_IF_FAILED(handleEventRootKey.CreateSubKey(contentEvent, KEY_WRITE, &handleEventKey));

    RETURN_IF_FAILED(handleEventKey.SetStringValue(HandlerName, nullptr));*/

    return S_OK;
}

HRESULT AutoPlay::CreateHandler(MsixRequest * msixRequest, IPackageHandler ** instance)
{
    std::unique_ptr<AutoPlay > localInstance(new AutoPlay(msixRequest));
    if (localInstance == nullptr)
    {
        return E_OUTOFMEMORY;
    }

    RETURN_IF_FAILED(localInstance->ParseManifest());

    *instance = localInstance.release();

    return S_OK;
}