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

#ifndef _FASTXDMUPDATEFACTORY_HPP
#define _FASTXDMUPDATEFACTORY_HPP

#include <xqilla/framework/XQillaExport.hpp>
#include <xqilla/context/UpdateFactory.hpp>

class XQILLA_API FastXDMUpdateFactory : public UpdateFactory
{
public:
  virtual void applyPut(const PendingUpdate &update, DynamicContext *context);
  virtual void applyInsertInto(const PendingUpdate &update, DynamicContext *context);
  virtual void applyInsertAttributes(const PendingUpdate &update, DynamicContext *context);
  virtual void applyReplaceValue(const PendingUpdate &update, DynamicContext *context);
  virtual void applyRename(const PendingUpdate &update, DynamicContext *context);
  virtual void applyDelete(const PendingUpdate &update, DynamicContext *context);
  virtual void applyInsertBefore(const PendingUpdate &update, DynamicContext *context);
  virtual void applyInsertAfter(const PendingUpdate &update, DynamicContext *context);
  virtual void applyInsertAsFirst(const PendingUpdate &update, DynamicContext *context);
  virtual void applyInsertAsLast(const PendingUpdate &update, DynamicContext *context);
  virtual void applyReplaceNode(const PendingUpdate &update, DynamicContext *context);
  virtual void applyReplaceAttribute(const PendingUpdate &update, DynamicContext *context);
  virtual void applyReplaceElementContent(const PendingUpdate &update, DynamicContext *context);

  virtual void completeRevalidation(DynamicContext *context);
  virtual void completeDeletions(DynamicContext *context);
  virtual void completeUpdate(DynamicContext *context);
};

#endif
