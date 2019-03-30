#include "generalutil.hpp"
#include "MsixResponse.hpp"

HRESULT MsixResponse::Make(MsixResponse ** outInstance)
{
    std::unique_ptr<MsixResponse> instance(new MsixResponse());
    if (instance == nullptr)
    {
        return E_OUTOFMEMORY;
    }

    instance->errorCode = NULL;
    instance->errorText = NULL;
    *outInstance = instance.release();

    return S_OK;
}