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

#ifndef _XERCESUPDATEFACTORY_HPP
#define _XERCESUPDATEFACTORY_HPP

#include <set>

#include <xqilla/framework/XQillaExport.hpp>
#include <xqilla/context/UpdateFactory.hpp>
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/ast/LocationInfo.hpp>

XERCES_CPP_NAMESPACE_BEGIN
class DOMNode;
XERCES_CPP_NAMESPACE_END

class XQILLA_API XercesUpdateFactory : public UpdateFactory
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

  virtual void completeUpdate(DynamicContext *context);

protected:
  void completeDeletions(DynamicContext *context);
  void completeRevalidation(DynamicContext *context);
  void removeType(XERCES_CPP_NAMESPACE_QUALIFIER DOMNode *node);
  void setTypes(XERCES_CPP_NAMESPACE_QUALIFIER DOMNode *node, const XERCES_CPP_NAMESPACE_QUALIFIER DOMNode *from);
  void addToPutSet(const Node::Ptr &node, const LocationInfo *location, DynamicContext *context);

  typedef std::set<XERCES_CPP_NAMESPACE_QUALIFIER DOMNode *> DOMNodeSet;

  class PutItem {
  public:
    PutItem(const XMLCh *urich, const Node::Ptr &nd, const LocationInfo *loc, const DynamicContext *c)
      : uri(urich), node(nd), location(loc), context(c) {}

    const XMLCh *uri;
    const Node::Ptr node;
    const LocationInfo *location;
    const DynamicContext *context;

    bool operator<(const PutItem &other) const;
  };
  typedef std::set<PutItem> PutSet;

  DOMNodeSet forDeletion_;
  DOMNodeSet forRevalidation_;
  PutSet putSet_;
};

#endif
