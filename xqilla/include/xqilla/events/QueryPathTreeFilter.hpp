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

#ifndef _QUERYPATHTREEFILTER_HPP
#define _QUERYPATHTREEFILTER_HPP

#include <vector>

#include <xqilla/events/EventHandler.hpp>

#include <xercesc/framework/XMLBuffer.hpp>

class QueryPathNode;
typedef std::vector<const QueryPathNode *> QPNVector;

class XQILLA_API QueryPathTreeFilter : public EventFilter
{
public:
  QueryPathTreeFilter(const QueryPathNode *qpn, EventHandler *next);
  QueryPathTreeFilter(const QPNVector &qpns, EventHandler *next);
  virtual ~QueryPathTreeFilter();

  virtual void startDocumentEvent(const XMLCh *documentURI, const XMLCh *encoding);
  virtual void endDocumentEvent();
  virtual void startElementEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname);
  virtual void endElementEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname,
                               const XMLCh *typeURI, const XMLCh *typeName);
  virtual void piEvent(const XMLCh *target, const XMLCh *value);
  virtual void textEvent(const XMLCh *value);
  virtual void textEvent(const XMLCh *chars, unsigned int length);
  virtual void commentEvent(const XMLCh *value);
  virtual void attributeEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname, const XMLCh *value,
                              const XMLCh *typeURI, const XMLCh *typeName);
  virtual void namespaceEvent(const XMLCh *prefix, const XMLCh *uri);

protected:
  struct StackEntry {
    StackEntry() : matched(false), nonElementChildren(false), attrChildren(false), children() {}

    void addNode(const QueryPathNode *isn);
    void addChildren(const QueryPathNode *isn);

    XERCES_CPP_NAMESPACE_QUALIFIER XMLBuffer prefix;
    XERCES_CPP_NAMESPACE_QUALIFIER XMLBuffer uri;
    XERCES_CPP_NAMESPACE_QUALIFIER XMLBuffer localname;

    bool matched;

    bool nonElementChildren;
    bool attrChildren;
    QPNVector children;
  };

  typedef std::vector<StackEntry*> FilterStack;

  void checkAncestors(FilterStack::reverse_iterator s);

  FilterStack stack_;
};

#endif
