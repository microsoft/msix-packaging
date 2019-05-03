#include "UserSid.hpp"

HRESULT ConvertSidToString(
    __in const PSID sid,
    __out StringBuffer* sidString)
{
    IfNullReturnError(sid, E_INVALIDARG);
    IfNullReturnError(sidString, E_INVALIDARG);

    NTSTATUS status;
    UNICODE_STRING unicodeStringSid;
    IfNtFailedReturnHr(RtlConvertSidToUnicodeString(&unicodeStringSid, sid, TRUE));

    NT_ASSERT(unicodeStringSid.Length != 0);

    HRESULT hr;
    hr = sidString->SetValue(unicodeStringSid.Buffer, unicodeStringSid.Length / sizeof(WCHAR));

    RtlFreeUnicodeString(&unicodeStringSid);

    return hr;
}


HRESULT SidHelper::GetUserSidFromToken(
    __in HANDLE userToken,
    __deref_out PSID* sid)
{
    RETURN_HR_IF_NULL(E_INVALIDARG, userToken);
    RETURN_HR_IF_NULL(E_INVALIDARG, sid);

    DWORD tokenSize = 0;
    RETURN_HR_IF(E_UNEXPECTED, ::GetTokenInformation(userToken, TokenUser, NULL, tokenSize, &tokenSize));
    RETURN_LAST_ERROR_IF(::GetLastError() != ERROR_INSUFFICIENT_BUFFER);

    wistd::unique_ptr<BYTE[]> tokenBuffer(new APPXCOMMON_NEW_TAG BYTE[tokenSize]);
    RETURN_IF_NULL_ALLOC(tokenBuffer);
    RETURN_IF_WIN32_BOOL_FALSE(::GetTokenInformation(userToken, TokenUser, tokenBuffer.get(), tokenSize, &tokenSize));
    auto pTokenUser = reinterpret_cast<PTOKEN_USER>(tokenBuffer.get());

    DWORD sidSize = GetLengthSid(pTokenUser->User.Sid);
    wistd::unique_ptr<BYTE[]> newSid(new APPXCOMMON_NEW_TAG BYTE[sidSize]);
    RETURN_IF_NULL_ALLOC(newSid);
    RETURN_IF_WIN32_BOOL_FALSE(::CopySid(sidSize, newSid.get(), pTokenUser->User.Sid));

    *sid = newSid.release();
    return S_OK;
}

HRESULT SidHelper::GetCurrentUserSid(
    __deref_out PSID* sid)
{
    RETURN_HR(GetUserSidFromToken(GetCurrentThreadEffectiveToken(), sid));
}