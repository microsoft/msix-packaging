//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once
#include <memory>
#include <string>

#include "Exceptions.hpp"
#include "StreamBase.hpp"
#include "ComHelper.hpp"

// internal interface
EXTERN_C const IID IID_IVerifierObject;
#ifndef WIN32
// {cb0a105c-3a6c-4e48-9351-377c4dccd890}
interface IVerifierObject : public IUnknown
#else
#include "Unknwn.h"
#include "Objidl.h"
class IVerifierObject : public IUnknown
#endif
// An internal interface for objects that are used to verify structured data represented by an underlying stream.
{
public:
    virtual const std::string& GetPublisher() = 0;
    virtual bool HasStream() = 0;
    virtual MSIX::ComPtr<IStream> GetStream() = 0;
    virtual MSIX::ComPtr<IStream> GetValidationStream(const std::string& part, const MSIX::ComPtr<IStream>& stream) = 0;
    virtual const std::string GetPackageFullName() = 0;
};

SpecializeUuidOfImpl(IVerifierObject);