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

#ifndef _UPDATEFACTORY_HPP
#define _UPDATEFACTORY_HPP

#include <xqilla/framework/XQillaExport.hpp>
#include <xqilla/schema/DocumentCache.hpp>

#include <xercesc/util/XMemory.hpp>

class PendingUpdateList;
class PendingUpdate;
class DynamicContext;

class XQILLA_API UpdateFactory : public XERCES_CPP_NAMESPACE_QUALIFIER XMemory
{
public:
  virtual ~UpdateFactory() {}

  virtual void applyUpdates(const PendingUpdateList &pul, DynamicContext *context, DocumentCache::ValidationMode valMode);
  virtual void checkUpdates(const PendingUpdateList &pul, DynamicContext *context, DocumentCache::ValidationMode valMode) {}

  virtual void applyPut(const PendingUpdate &update, DynamicContext *context) = 0;
  virtual void applyInsertInto(const PendingUpdate &update, DynamicContext *context) = 0;
  virtual void applyInsertAttributes(const PendingUpdate &update, DynamicContext *context) = 0;
  virtual void applyReplaceValue(const PendingUpdate &update, DynamicContext *context) = 0;
  virtual void applyRename(const PendingUpdate &update, DynamicContext *context) = 0;
  virtual void applyDelete(const PendingUpdate &update, DynamicContext *context) = 0;
  virtual void applyInsertBefore(const PendingUpdate &update, DynamicContext *context) = 0;
  virtual void applyInsertAfter(const PendingUpdate &update, DynamicContext *context) = 0;
  virtual void applyInsertAsFirst(const PendingUpdate &update, DynamicContext *context) = 0;
  virtual void applyInsertAsLast(const PendingUpdate &update, DynamicContext *context) = 0;
  virtual void applyReplaceNode(const PendingUpdate &update, DynamicContext *context) = 0;
  virtual void applyReplaceAttribute(const PendingUpdate &update, DynamicContext *context) = 0;
  virtual void applyReplaceElementContent(const PendingUpdate &update, DynamicContext *context) = 0;

  virtual void completeUpdate(DynamicContext *context) = 0;

protected:
  DocumentCache::ValidationMode valMode_;  
};

#endif
