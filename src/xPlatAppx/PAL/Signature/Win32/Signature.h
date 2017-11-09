#pragma once
#include <windows.h>
#include "AppxPackaging.hpp"
#include "StreamBase.hpp"
#include <Wincrypt.h>
#include <vector>
#include <algorithm>
#include <limits>
#include <type_traits>


namespace xPlat
{
    // Object identifier for the Windows Store certificate. We look for this
    // identifier in the cert EKUs to determine if the cert originates from
    // Windows Store.
    const std::string WindowsStoreOid = "1.3.6.1.4.1.311.76.3.1";

    // APPX-specific header placed in the P7X file, before the actual signature
    const DWORD P7X_FILE_ID = 0x58434b50;
    
    class AppxSignatureOrigin
    {
    public:
        static bool ValidateSignature(IStream *stream);

        static bool IsStoreOrigin(byte* signatureBuffer, ULONG cbSignatureBuffer);

        static DWORD GenerateSHA256Hash(_In_ byte *buffer, _In_ ULONG cbBuffer, std::vector<byte>& hash);

        static bool IsSelfSigned(byte *signatureBuffer, ULONG cbSignatureBuffer);

        static HRESULT IsMicrosoftTrustedChain(_In_ PCCERT_CHAIN_CONTEXT certChainContext);

        static HRESULT IsAuthenticodeTrustedChain(_In_ PCCERT_CHAIN_CONTEXT certChainContext);

    private:
        static bool GetEnhancedKeyUsage(
            PCCERT_CONTEXT pCertContext,
            std::vector<std::string>& values);

        static bool DoesSignatureCertContainStoreEKU(
            _In_ byte* signatureBuffer,
            _In_ ULONG cbSignatureBuffer);

        static PCCERT_CHAIN_CONTEXT GetCertChainContext(
            _In_ byte* signatureBuffer,
            _In_ ULONG cbSignatureBuffer);



        
    };

}