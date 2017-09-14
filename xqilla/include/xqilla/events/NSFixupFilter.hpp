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

#ifndef _NSFIXUPFILTER_HPP
#define _NSFIXUPFILTER_HPP

#include <xqilla/events/EventHandler.hpp>

class XPath2MemoryManager;

class XQILLA_API NSFixupFilter : public EventFilter
{
public:
  NSFixupFilter(EventHandler *next, XPath2MemoryManager *mm);
  ~NSFixupFilter();

  virtual void startDocumentEvent(const XMLCh *documentURI, const XMLCh *encoding);
  virtual void endDocumentEvent();
  virtual void startElementEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname);
  virtual void endElementEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname,
                               const XMLCh *typeURI, const XMLCh *typeName);
  virtual void attributeEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname, const XMLCh *value,
                              const XMLCh *typeURI, const XMLCh *typeName);
  virtual void namespaceEvent(const XMLCh *prefix, const XMLCh *uri);

private:
  struct ElemEntry {
    void set(const XMLCh *p, ElemEntry *pr)
    { prefix = p; prev = pr; }

    const XMLCh *prefix;
    ElemEntry *prev;
  };

  struct NSEntry {
    void set(const XMLCh *p, const XMLCh *u, unsigned int l, NSEntry *pr)
    { prefix = p; uri = u; level = l; prev = pr; }

    const XMLCh *prefix;
    const XMLCh *uri;
    unsigned int level;

    NSEntry *prev;
  };

  NSEntry *findPrefix(const XMLCh *prefix);
  /// Returns the prefix to use
  bool definePrefix(const XMLCh *&prefix, const XMLCh *uri, bool attr = false, bool redefine = true);

  XPath2MemoryManager *mm_;
  unsigned int level_;
  ElemEntry *elements_;
  NSEntry *namespaces_;
};

#endif
