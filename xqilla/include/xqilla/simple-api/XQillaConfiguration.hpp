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

#ifndef XQILLACONFIGURATION_H
#define XQILLACONFIGURATION_H

#include <xqilla/framework/XQillaExport.hpp>
#include <xqilla/simple-api/XQilla.hpp>

#include <xercesc/framework/MemoryManager.hpp>
#include <xercesc/util/XMemory.hpp>

class DocumentCache;
class SequenceBuilder;
class ItemFactory;
class UpdateFactory;
class URIResolver;
class ASTToXML;
class StaticContext;
class DynamicContext;

class XQILLA_API XQillaConfiguration : public XERCES_CPP_NAMESPACE_QUALIFIER XMemory
{
public:
  virtual ~XQillaConfiguration() {}

  virtual DocumentCache *createDocumentCache(XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager *memMgr) = 0;

  virtual SequenceBuilder *createSequenceBuilder(const DynamicContext *context) = 0;

  virtual ItemFactory *createItemFactory(DocumentCache *cache,
                                         XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager *memMgr) = 0;

  virtual UpdateFactory *createUpdateFactory(XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager *memMgr) = 0;

  virtual URIResolver *createDefaultURIResolver(XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager *memMgr) = 0;

  virtual ASTToXML *createASTToXML() = 0;

  virtual void populateStaticContext(StaticContext *context) {}

  virtual void populateDynamicContext(DynamicContext *context) {}

  virtual void testInterrupt() {}
};

#endif
