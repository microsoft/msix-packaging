//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once
#include "MSIXWindows.hpp"
#include "ComHelper.hpp"

#include <vector>

// internal interface
// {1f850db4-32b8-4db6-8bf4-5a897eb611f1}
#ifndef WIN32
interface IMsixFactory : public IUnknown
#else
#include "unknwn.h"
#include "objidl.h"
class IMsixFactory : public IUnknown
#endif
{
public:
    virtual HRESULT MarshalOutString(std::string& internal, LPWSTR* result) = 0;
    virtual HRESULT MarshalOutBytes(std::vector<std::uint8_t>& data, UINT32* size, BYTE** buffer) = 0;
    virtual MSIX_VALIDATION_OPTION GetValidationOptions() = 0;
    virtual MSIX::ComPtr<IStream> GetResource(const std::string& resource) = 0;
    virtual HRESULT MarshalOutWstring(std::wstring& internal, LPWSTR* result) = 0;
    virtual HRESULT MarshalOutStringUtf8(std::string& internal, LPSTR* result) = 0;
};
MSIX_INTERFACE(IMsixFactory, 0x1f850db4,0x32b8,0x4db6,0x8b,0xf4,0x5a,0x89,0x7e,0xb6,0x11,0xf1);
