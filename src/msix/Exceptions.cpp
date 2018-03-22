//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#include "Exceptions.hpp"

namespace MSIX {
#ifdef WIN32
__declspec(noinline)
#endif
void 
#ifndef WIN32
__attribute__(( noinline)) 
#endif
RaiseExceptionIfFailed(HRESULT hr, const int line, const char* const file)
{
    if (FAILED(hr)) {
        MSIX::RaiseException<MSIX::Exception> (line, file, nullptr, hr);
    }
}

}