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

#include <xqilla/events/NSFixupFilter.hpp>
#include <xqilla/framework/XPath2MemoryManager.hpp>
#include <xqilla/utils/XPath2Utils.hpp>

#include <xercesc/framework/XMLBuffer.hpp>

XERCES_CPP_NAMESPACE_USE;

NSFixupFilter::NSFixupFilter(EventHandler *next, XPath2MemoryManager *mm)
  : EventFilter(next),
    mm_(mm),
    level_(0),
    elements_(0),
    namespaces_(0)
{
}

NSFixupFilter::~NSFixupFilter()
{
  while(elements_ != 0) {
    ElemEntry *tmp = elements_;
    elements_ = elements_->prev;
    mm_->deallocate(tmp);
  }
  while(namespaces_ != 0) {
    NSEntry *tmp = namespaces_;
    namespaces_ = namespaces_->prev;
    mm_->deallocate(tmp);
  }
}

void NSFixupFilter::startDocumentEvent(const XMLCh *documentURI, const XMLCh *encoding)
{
  next_->startDocumentEvent(documentURI, encoding);
  ++level_;
}

void NSFixupFilter::endDocumentEvent()
{
  --level_;
  next_->endDocumentEvent();
}

void NSFixupFilter::startElementEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname)
{
  ++level_;
  bool def = definePrefix(prefix, uri);

  // add the prefix to our linked list
  ElemEntry *entry = (ElemEntry*)mm_->allocate(sizeof(ElemEntry));
  entry->set(mm_->getPooledString(prefix), elements_);
  elements_ = entry;

  next_->startElementEvent(prefix, uri, localname);

  if(def) next_->namespaceEvent(prefix, uri);
}

void NSFixupFilter::endElementEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname,
                                    const XMLCh *typeURI, const XMLCh *typeName)
{
  ElemEntry *etmp = elements_;

  next_->endElementEvent(etmp->prefix, uri, localname, typeURI, typeName);

  --level_;

  elements_ = elements_->prev;
  mm_->deallocate(etmp);

  while(namespaces_ != 0 && namespaces_->level > level_) {
    NSEntry *tmp = namespaces_;
    namespaces_ = namespaces_->prev;
    mm_->deallocate(tmp);
  }
}

void NSFixupFilter::attributeEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname, const XMLCh *value,
                                   const XMLCh *typeURI, const XMLCh *typeName)
{
  if(level_ != 0 && definePrefix(prefix, uri, /*attr*/true)) {
    next_->namespaceEvent(prefix, uri);
  }

  next_->attributeEvent(prefix, uri, localname, value, typeURI, typeName);
}

void NSFixupFilter::namespaceEvent(const XMLCh *prefix, const XMLCh *uri)
{
  if(level_ == 0 || definePrefix(prefix, uri, /*attr*/false, /*redefine*/false)) {
    next_->namespaceEvent(prefix, uri);
  }
}

static void numToBuf(unsigned int n, XMLBuffer &buf)
{
  if(n >= 10) numToBuf(n / 10, buf);
  buf.append('0' + (n % 10));
}

NSFixupFilter::NSEntry *NSFixupFilter::findPrefix(const XMLCh *prefix)
{
  NSEntry *cur = namespaces_;
  while(cur != 0) {
    if(XPath2Utils::equals(prefix, cur->prefix)) {
      break;
    }
    cur = cur->prev;
  }
  return cur;
}  

/// Returns the prefix to use
bool NSFixupFilter::definePrefix(const XMLCh *&prefix, const XMLCh *uri, bool attr, bool redefine)
{
  // Attributes with no namespace do not need a prefix defined
  if(uri == 0 && attr) return false;

  // The "xml" prefix is always defined
  if(XPath2Utils::equals(XMLUni::fgXMLString, prefix)) return false;

  bool define = true;
  bool newPrefix = false;

  // Make up a prefix for an attribute in a namespace without one
  if(prefix == 0 && attr) {
    newPrefix = true;
  }
  else {
    // Check if the prefix is already defined
    NSEntry *cur = findPrefix(prefix);

    if(cur != 0) {
      if(XPath2Utils::equals(uri, cur->uri)) {
        // The prefix is already defined
        define = false;
      }
      else if(cur->level == level_) {
        if(uri == 0) {
          // TBD this is an error - jpcs
          define = false;
        }
        else if(!redefine) {
          define = false;
        }
        else {
          newPrefix = true;
        }
      }
    }
    else if(uri == 0) {
      // The default namespace is implicitly set to no namespace
      define = false;
    }
  }

  if(newPrefix) {
    // See if there's an existing binding
    NSEntry *cur = namespaces_;
    while(cur != 0) {
      if(XPath2Utils::equals(uri, cur->uri) &&
         (!attr || cur->prefix != 0)) {
        break;
      }
      cur = cur->prev;
    }

    if(cur != 0) {
      prefix = cur->prefix;
      define = false;
    }
    else {
      // Choose a new prefix
      static XMLCh ns_prefix[] = { 'n', 's', 0 };
      if(prefix == 0) prefix = ns_prefix;

      XMLBuffer buf(20, mm_);
      unsigned int num = 0;
      do {
        ++num;
        buf.set(prefix);
        buf.append('_');
        numToBuf(num, buf);
      } while(findPrefix(buf.getRawBuffer()) != 0);
      prefix = mm_->getPooledString(buf.getRawBuffer());
    }
  }

  // add the mapping to our linked list
  NSEntry *entry = (NSEntry*)mm_->allocate(sizeof(NSEntry));
  entry->set(mm_->getPooledString(prefix),
             mm_->getPooledString(uri),
             level_, namespaces_);
  namespaces_ = entry;

  return define;
}

