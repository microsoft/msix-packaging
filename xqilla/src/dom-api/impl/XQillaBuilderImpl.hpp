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

#ifndef __XQILLADOMBUILDERIMPL_HPP
#define __XQILLADOMBUILDERIMPL_HPP

#include <xqilla/framework/XQillaExport.hpp>
#include "../../config/xqilla_config.h"

#include <xercesc/util/XercesVersion.hpp>

#if _XERCES_VERSION >= 30000
#include <xercesc/parsers/DOMLSParserImpl.hpp>
#else
#include <xercesc/parsers/DOMBuilderImpl.hpp>
#endif

XERCES_CPP_NAMESPACE_BEGIN
class DOMInputSource;
class XMLGrammarPool;
class MemoryManager;
class XMLValidator;
XERCES_CPP_NAMESPACE_END  

class XQillaBuilderImpl :
#if _XERCES_VERSION >= 30000
	public XERCES_CPP_NAMESPACE_QUALIFIER DOMLSParserImpl
#else
	public XERCES_CPP_NAMESPACE_QUALIFIER DOMBuilderImpl
#endif
{
public:

    /** @name Constructors and Destructor */
    //@{
    /** Construct a DOMBuilderImpl, with an optional validator
      *
      * Constructor with an instance of validator class to use for
      * validation. If you don't provide a validator, a default one will
      * be created for you in the scanner.
      *
      * @param gramPool   Pointer to the grammar pool instance from 
      *                   external application.
      *                   The parser does NOT own it.
      *
      * @param valToAdopt Pointer to the validator instance to use. The
      *                   parser is responsible for freeing the memory.
      */
  XQillaBuilderImpl
  (
   XERCES_CPP_NAMESPACE_QUALIFIER XMLValidator* const   valToAdopt,
   XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager* const  manager,
   XERCES_CPP_NAMESPACE_QUALIFIER XMLGrammarPool* const gramPool,
   bool adoptGramPool
   );

  virtual ~XQillaBuilderImpl();

#if _XERCES_VERSION >= 30000
  XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument* parse(const XERCES_CPP_NAMESPACE_QUALIFIER DOMLSInput* source);
#else
  XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument* parse(const XERCES_CPP_NAMESPACE_QUALIFIER DOMInputSource& source);
#endif

  XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument* parseURI(const XMLCh* const systemId);

  XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument* parseURI(const char* const systemId);


  static const XMLCh gXQilla[];

private:
  void initParser();
  XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument* getDocumentAndAddGrammar();
  XERCES_CPP_NAMESPACE_QUALIFIER XMLGrammarPool *gramPool_;
};


#endif


