//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
//  Helper macros for flow control. Throws Error::NotSupported when a feature
//  is not enabled.
#include "Exceptions.hpp"

#ifndef THROW_IF_BUNDLE_NOT_ENABLED
#ifdef BUNDLE_SUPPORT
#define THROW_IF_BUNDLE_NOT_ENABLED
#else
#define THROW_IF_BUNDLE_NOT_ENABLED { MSIX::RaiseException<MSIX::Exception>(__LINE__, __FILE__, "Bundle support not enabled", MSIX::Error::SupportExcludedByBuild); }
#endif // BUNDLE_SUPPORT
#endif // THROW_IF_BUNDLE_NOT_ENABLED

#ifndef THROW_IF_PACK_NOT_ENABLED
#ifdef MSIX_PACK
#define THROW_IF_PACK_NOT_ENABLED
#else
#define THROW_IF_PACK_NOT_ENABLED { MSIX::RaiseException<MSIX::Exception>(__LINE__, __FILE__, "Packaging support not enabled", MSIX::Error::SupportExcludedByBuild ); }
#endif // MSIX_PACK
#endif // THROW_IF_PACK_NOT_ENABLED
