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

#ifndef _FAXPPDOCUMENTCACHEIMPL_HPP
#define _FAXPPDOCUMENTCACHEIMPL_HPP

#include "../config/xqilla_config.h"

#ifdef HAVE_FAXPP

extern "C" {
#include <faxpp/parser.h>
}

#include <xqilla/schema/DocumentCacheImpl.hpp>
#include <xqilla/schema/SchemaValidatorFilter.hpp>

XERCES_CPP_NAMESPACE_BEGIN
class BinInputStream;
XERCES_CPP_NAMESPACE_END

struct FaxppParserWrapper
{
  FaxppParserWrapper(XERCES_CPP_NAMESPACE_QUALIFIER XMLEntityResolver *e);
  ~FaxppParserWrapper();

  void reset();
  FAXPP_Error parseInputSource(const XERCES_CPP_NAMESPACE_QUALIFIER InputSource &srcToUse, XPath2MemoryManager *m);

  std::vector<XERCES_CPP_NAMESPACE_QUALIFIER BinInputStream*> inputStreams;
  XPath2MemoryManager *mm;
  XERCES_CPP_NAMESPACE_QUALIFIER XMLEntityResolver *entityResolver;
  FAXPP_Parser *parser;
};

class FaxppDocumentCacheImpl : public DocumentCacheImpl
{
public:
  FaxppDocumentCacheImpl(XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager* memMgr, XERCES_CPP_NAMESPACE_QUALIFIER XMLGrammarPool* xmlgr = 0);
  ~FaxppDocumentCacheImpl();

  virtual void setXMLEntityResolver(XERCES_CPP_NAMESPACE_QUALIFIER XMLEntityResolver* const handler);

  virtual void parseDocument(XERCES_CPP_NAMESPACE_QUALIFIER InputSource &srcToUse, EventHandler *handler, DynamicContext *context);

  virtual DocumentCache *createDerivedCache(XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager *memMgr) const;

protected:
  FaxppParserWrapper wrapper_;
  SchemaValidatorFilter *validator_;
};

#endif

#endif

