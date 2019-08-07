//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#include "Exceptions.hpp"

#ifdef WIN32
#include <delayimp.h>

// Custom failure hook that will be called if delayloading a dll fails. This give us the ability
// to throw a C++ exception if we failed instead of throwing an SEH exception,which is the default behaviour.
// See: https://docs.microsoft.com/en-us/cpp/build/reference/failure-hooks
FARPROC WINAPI MsixDelayLoadFailureHandler(unsigned /*dliNotify*/, PDelayLoadInfo pDelayInfo)
{
    // If you are here, it means that we tried to delay  load a Win8+ dll and we failed. If you're
    // running on Win7, you are probably missing the manifest to your exe (can't be added to the dll).
    // See manifest.cmakein at the root of the project for an example the manifest.
    if (pDelayInfo)
    {
        DWORD dwGLE = pDelayInfo->dwLastError ? pDelayInfo->dwLastError : ERROR_PROC_NOT_FOUND;
        std::ostringstream builder;
        builder << "Call failed delayloading " << pDelayInfo->szDll;
        ThrowErrorAndLog(HRESULT_FROM_WIN32(dwGLE), builder.str().c_str());
    }
    ThrowErrorAndLog(HRESULT_FROM_WIN32(ERROR_PROC_NOT_FOUND), "Failed delayloading");
}
const PfnDliHook __pfnDliFailureHook2 = MsixDelayLoadFailureHandler;
#endif

namespace MSIX {

MSIX_NOINLINE(void) RaiseExceptionIfFailed(HRESULT hr, const int line, const char* const file)
{
    if (FAILED(hr)) {
        MSIX::RaiseException<MSIX::Exception> (line, file, nullptr, hr);
    }
}

}
