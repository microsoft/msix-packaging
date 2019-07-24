// Copyright (C) Microsoft. All rights reserved.

#include <Windows.h>
#include <WinCrypt.h>
#include <BcryptLibrary.hpp>
#include <assert.h>

/// @note We intentionally load BCRYPT once on first use and never explicitly unload it;
///       it's intentionally kept loaded until process termination unloads it for us.
///
/// @note We initialize the module handle to nullptr @ compile-time and only Load() modifies it,
///       and then only on first need.
///
/// @note In the exceedingly rare event 2 threads concurrently call Load(), we'd call
///       LoadLibrary() twice but producing the same result (the 2nd call finds the DLL already
///       in the LoadedModuleTable and returns the same handle after bumping its reference
///       count). Thus we don't need any special synchronization to manage this, as worst case
///       we'll wind up overwriting the global with the same value on the 2nd load.
volatile PVOID MsixCoreLib::BcryptLibrary::bcryptModule = nullptr;
FARPROC MsixCoreLib::BcryptLibrary::functions[MsixCoreLib::BcryptLibrary::FunctionCount];
PCSTR MsixCoreLib::BcryptLibrary::functionNames[MsixCoreLib::BcryptLibrary::FunctionCount] = {
    "BCryptOpenAlgorithmProvider",
    "BCryptCloseAlgorithmProvider",
    "BCryptGetProperty",
    "BCryptCreateHash",
    "BCryptHashData",
    "BCryptFinishHash",
    "BCryptDestroyHash"
};

///TODO:5204957 MsixCoreLib::BcryptLibrary::Load() (initialization) leaks
_Check_return_ HRESULT MsixCoreLib::BcryptLibrary::Load()
{
    if (ReadPointerAcquire(&bcryptModule) == nullptr)
    {
        HMODULE h = LoadLibraryExW(L"Bcrypt.dll", nullptr, 0);
        if (h == nullptr)
        {
            _Analysis_assume_(GetLastError() != ERROR_SUCCESS);
            return HRESULT_FROM_WIN32(GetLastError());
        }

        C_ASSERT(ARRAYSIZE(functions) == ARRAYSIZE(functionNames));
        FARPROC exports[ARRAYSIZE(functions)];
        for (size_t i = 0; i < ARRAYSIZE(functions); ++i)
        {
            exports[i] = GetProcAddress(h, functionNames[i]);
            if (exports[i] == nullptr)
            {
                DWORD lastError = GetLastError();
                _Analysis_assume_(lastError != ERROR_SUCCESS);
                assert(FreeLibrary(h));
                return HRESULT_FROM_WIN32(lastError);
            }
        }

        CopyMemory(functions, exports, sizeof(functions));
        WritePointerRelease(&bcryptModule, h);
    }
    return S_OK;
}
