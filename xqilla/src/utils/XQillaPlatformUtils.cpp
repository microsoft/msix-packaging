/*
 * Copyright (c) 2001, 2008,
 *     DecisionSoft Limited. All rights reserved.
 * Copyright (c) 2004, 2015 Oracle and/or its affiliates. All rights reserved.
 *     
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "../config/xqilla_config.h"

#include <xqilla/framework/XPath2MemoryManagerImpl.hpp>
#include <xqilla/utils/XQillaPlatformUtils.hpp>
#include <xqilla/items/impl/ATDecimalOrDerivedImpl.hpp>
#include <xqilla/items/impl/ATDoubleOrDerivedImpl.hpp>
#include <xqilla/items/impl/ATFloatOrDerivedImpl.hpp>
#include <xqilla/items/ATUntypedAtomic.hpp>
#include <xqilla/schema/AnyAtomicTypeDatatypeValidator.hpp>
#include <xqilla/functions/FunctionLookup.hpp>
#include <xqilla/mapm/m_apm.h>

#include "../dom-api/XQillaImplementation.hpp"
#include "DateUtils.hpp"

#include <xercesc/dom/DOMImplementationRegistry.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/validators/datatype/DatatypeValidatorFactory.hpp>

XERCES_CPP_NAMESPACE_USE;

static int gInitFlag = 0;

void XQillaPlatformUtils::initialize(MemoryManager *memMgr) {
  //  Make sure we haven't already been initialized. Note that this is not
  //  thread safe and is not intended for that.
  if(gInitFlag++ == 0) {
    XMLPlatformUtils::Initialize(XMLUni::fgXercescDefaultLocale, 0, 0, memMgr);
    XQillaImplementation::initialize();
    m_apm_library_initialize();
    DateUtils::initialize();
    FunctionLookup::initialize();

    // Add the extra XQuery types to the built-in datatype registry
    DatatypeValidatorFactory dvf;

#if _XERCES_VERSION >= 30000
    // Handled during XMLPlatformUtils::Initialize() in Xerces-C 3.0
#else
    // Trigger the creation of the built-in datatype registry
    dvf.expandRegistryToFullSchemaSet();
#endif

    // Expand the Xerces built-in registry to include xs:anyAtomicType
    // as a base type
    DatatypeValidator* dv = new AnyAtomicTypeDatatypeValidator();
    dv->setTypeName(AnyAtomicType::fgDT_ANYATOMICTYPE, SchemaSymbols::fgURI_SCHEMAFORSCHEMA);
    DatatypeValidatorFactory::getBuiltInRegistry()->
      put((void*)AnyAtomicType::fgDT_ANYATOMICTYPE, dv);

    // Add xs:yearMonthDuration as a built-in derived type
    RefHashTableOf<XERCES_CPP_NAMESPACE_QUALIFIER KVStringPair>* facets =
      new RefHashTableOf<XERCES_CPP_NAMESPACE_QUALIFIER KVStringPair>(1);
    facets->put((void*) SchemaSymbols::fgELT_PATTERN,
                new XERCES_CPP_NAMESPACE_QUALIFIER KVStringPair(SchemaSymbols::fgELT_PATTERN,
                                                                ATDurationOrDerived::pattern_DT_YEARMONTHDURATION));
    
    dvf.createDatatypeValidator(ATDurationOrDerived::fgDT_YEARMONTHDURATION, 
                                dvf.getDatatypeValidator(SchemaSymbols::fgDT_DURATION),
                                facets, 0, false, 0, false);
    
    // Add xs:dayTimeDuration as a built-in derived type
    facets = new RefHashTableOf<XERCES_CPP_NAMESPACE_QUALIFIER KVStringPair>(1);
    facets->put((void*) SchemaSymbols::fgELT_PATTERN,
                new XERCES_CPP_NAMESPACE_QUALIFIER KVStringPair(SchemaSymbols::fgELT_PATTERN,
                                                                ATDurationOrDerived::pattern_DT_DAYTIMEDURATION));
    
    dvf.createDatatypeValidator(ATDurationOrDerived::fgDT_DAYTIMEDURATION, 
                                dvf.getDatatypeValidator(SchemaSymbols::fgDT_DURATION),
                                facets, 0, false, 0, false);

    // Add xs:untypedAtomic as a built-in derived type -- no facets
    dvf.createDatatypeValidator(ATUntypedAtomic::fgDT_UNTYPEDATOMIC, 
                                dvf.getDatatypeValidator(AnyAtomicType::fgDT_ANYATOMICTYPE),
                                0, 0, false, 0, false);
  }
}

void XQillaPlatformUtils::terminate() {
  if(gInitFlag == 0) {
    return;
  }

  if(--gInitFlag == 0) {
    DateUtils::terminate();
    m_apm_library_terminate();
    FunctionLookup::terminate();
    XQillaImplementation::terminate();
    XMLPlatformUtils::Terminate();
  }
}

void XQillaPlatformUtils::enableExtendedPrecision(bool bEnable)
{
    if(bEnable)
    {
        ATDecimalOrDerivedImpl::g_nSignificantDigits=50;
        ATFloatOrDerivedImpl::g_nSignificantDigits=25;
        ATDoubleOrDerivedImpl::g_nSignificantDigits=25;
    }
    else
    {
        ATDecimalOrDerivedImpl::g_nSignificantDigits=18;
        ATFloatOrDerivedImpl::g_nSignificantDigits=7;
        ATDoubleOrDerivedImpl::g_nSignificantDigits=16;
    }
}
