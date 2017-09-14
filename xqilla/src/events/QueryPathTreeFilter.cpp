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
#include <assert.h>

#include <xqilla/events/QueryPathTreeFilter.hpp>
#include <xqilla/optimizer/QueryPathNode.hpp>
#include <xqilla/utils/XPath2Utils.hpp>

XERCES_CPP_NAMESPACE_USE;
using namespace std;

QueryPathTreeFilter::QueryPathTreeFilter(const QueryPathNode *qpn, EventHandler *next)
  : EventFilter(next)
{
  stack_.push_back(new StackEntry());
  stack_.back()->matched = true;

  if(qpn)
    stack_.back()->addNode(qpn);
}

QueryPathTreeFilter::QueryPathTreeFilter(const QPNVector &qpns, EventHandler *next)
  : EventFilter(next)
{
  stack_.push_back(new StackEntry());
  stack_.back()->matched = true;

  QPNVector::const_iterator i = qpns.begin();
  for(; i != qpns.end(); ++i) {
    stack_.back()->addNode(*i);
  }
}

QueryPathTreeFilter::~QueryPathTreeFilter()
{
  for(FilterStack::iterator i = stack_.begin(); i != stack_.end(); ++i)
    delete *i;
}

void QueryPathTreeFilter::startDocumentEvent(const XMLCh *documentURI, const XMLCh *encoding)
{
  StackEntry *entry = stack_.back();

  StackEntry *newEntry = new StackEntry();
  stack_.push_back(newEntry);
  QPNVector::iterator i = entry->children.begin();
  for(; i != entry->children.end(); ++i) {
    switch((*i)->getType()) {
    case QueryPathNode::ROOT:
      newEntry->matched = true;
      newEntry->addChildren(*i);
      break;
    case QueryPathNode::DESCENDANT_ATTR:
      newEntry->addNode(*i);
      break;
    case QueryPathNode::DESCENDANT:
      newEntry->addNode(*i);
      // Fall through
    case QueryPathNode::CHILD:
      if((*i)->isWildcardNodeType()) {
        newEntry->matched = true;
        newEntry->addChildren(*i);
      }
      break;
    case QueryPathNode::ATTRIBUTE:
    default: break;
    }
  }

  // Always output the document node
  newEntry->matched = true;
  next_->startDocumentEvent(documentURI, encoding);
}

void QueryPathTreeFilter::endDocumentEvent()
{
  StackEntry *entry = stack_.back();

  if(entry->matched)
    next_->endDocumentEvent();

  delete entry;
  stack_.pop_back();
}

void QueryPathTreeFilter::checkAncestors(FilterStack::reverse_iterator s)
{
  // The actual parent, if we weren't projecting
  StackEntry *parent = *s;

  // Look for the node this element would be a child of if it were output now
  // (The last ancestor that was matched)
  while(s != stack_.rend() && !(*s)->matched) ++s;

  assert(s != stack_.rend());

  // Is it our real parent?
  while(*s != parent) {
    // Search to see if this pseudo-parent is expecting actual children (not descendants)
    // that match any of the nodes we're looking for right now
    bool found = false;
    QPNVector::iterator i = (*s)->children.begin();
    for(; !found && i != (*s)->children.end(); ++i) {

      if((*i)->getType() == QueryPathNode::CHILD) {

        QPNVector::iterator j = parent->children.begin();
        for(; !found && j != parent->children.end(); ++j) {
          switch((*j)->getType()) {
          case QueryPathNode::DESCENDANT_ATTR:
            // Could result in the materialization of any element
            found = true;
            break;
          case QueryPathNode::DESCENDANT:
          case QueryPathNode::CHILD:
            if(((*i)->isWildcardName() || (*j)->isWildcardName() ||
                 XPath2Utils::equals((*i)->getName(), (*j)->getName())) &&
              ((*i)->isWildcardURI() || (*j)->isWildcardURI() ||
                XPath2Utils::equals((*i)->getURI(), (*j)->getURI()))) {
              found = true;
            }
            break;
          case QueryPathNode::ROOT:
          case QueryPathNode::ATTRIBUTE:
          default: break;
          }
        }

      }
    }

    if(!found) break;

    // Our pseudo-parent is expecting children that could match nodes in this part
    // of it's sub-tree - better output the next intermediate ancestor to disambiguate
    --s;

    (*s)->matched = true;
    next_->startElementEvent(emptyToNull((*s)->prefix.getRawBuffer()), emptyToNull((*s)->uri.getRawBuffer()),
                             (*s)->localname.getRawBuffer());
  }
}

void QueryPathTreeFilter::startElementEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname)
{
  StackEntry *entry = stack_.back();

  StackEntry *newEntry = new StackEntry();
  stack_.push_back(newEntry);

  QPNVector::iterator i = entry->children.begin();
  for(; i != entry->children.end(); ++i) {
    switch((*i)->getType()) {
    case QueryPathNode::DESCENDANT_ATTR:
      newEntry->addNode(*i);
      break;
    case QueryPathNode::DESCENDANT:
      newEntry->addNode(*i);
      // Fall through
    case QueryPathNode::CHILD:
      if(((*i)->isWildcardName() || XPath2Utils::equals((*i)->getName(), localname)) &&
        ((*i)->isWildcardURI() || XPath2Utils::equals((*i)->getURI(), uri))) {
        newEntry->matched = true;
        newEntry->addChildren(*i);
      }
      break;
    case QueryPathNode::ROOT:
    case QueryPathNode::ATTRIBUTE:
    default: break;
    }
  }

  if(newEntry->matched) {
    FilterStack::reverse_iterator s = stack_.rbegin();
    ++s;
    assert(*s == entry);
    checkAncestors(s);

    next_->startElementEvent(prefix, uri, localname);
  } else {
    newEntry->prefix.set(prefix);
    newEntry->uri.set(uri);
    newEntry->localname.set(localname);
  }
}

void QueryPathTreeFilter::endElementEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname,
                                     const XMLCh *typeURI, const XMLCh *typeName)
{
  StackEntry *entry = stack_.back();

  if(entry->matched) {
    next_->endElementEvent(prefix, uri, localname, typeURI, typeName);
  }

  stack_.pop_back();
  delete entry;
}

void QueryPathTreeFilter::piEvent(const XMLCh *target, const XMLCh *value)
{
  if(stack_.back()->nonElementChildren) {
    checkAncestors(stack_.rbegin());
    next_->piEvent(target, value);
  }
}

void QueryPathTreeFilter::textEvent(const XMLCh *value)
{
  if(stack_.back()->nonElementChildren) {
    checkAncestors(stack_.rbegin());
    next_->textEvent(value);
  }
}

void QueryPathTreeFilter::textEvent(const XMLCh *chars, unsigned int length)
{
  if(stack_.back()->nonElementChildren) {
    checkAncestors(stack_.rbegin());
    next_->textEvent(chars, length);
  }
}

void QueryPathTreeFilter::commentEvent(const XMLCh *value)
{
  if(stack_.back()->nonElementChildren) {
    checkAncestors(stack_.rbegin());
    next_->commentEvent(value);
  }
}

void QueryPathTreeFilter::attributeEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname, const XMLCh *value,
                                    const XMLCh *typeURI, const XMLCh *typeName)
{
  StackEntry *entry = stack_.back();

  if(entry->attrChildren) {
    bool found = false;

    QPNVector::iterator i = entry->children.begin();
    for(; i != entry->children.end() && !found; ++i) {
      switch((*i)->getType()) {
      case QueryPathNode::DESCENDANT_ATTR:
      case QueryPathNode::ATTRIBUTE:
        if(((*i)->isWildcardName() || XPath2Utils::equals((*i)->getName(), localname)) &&
           ((*i)->isWildcardURI() || XPath2Utils::equals((*i)->getURI(), uri))) {
          found = true;
        }
        break;
      default: break;
      }
    }

    if(found) {
      if(!entry->matched) {
        // Output the element for this attribute
        entry->matched = true;
        next_->startElementEvent(emptyToNull(entry->prefix.getRawBuffer()), emptyToNull(entry->uri.getRawBuffer()),
                                 entry->localname.getRawBuffer());
      }

      next_->attributeEvent(prefix, uri, localname, value, typeURI, typeName);
    }
  }
}

void QueryPathTreeFilter::namespaceEvent(const XMLCh *prefix, const XMLCh *uri)
{
  StackEntry *entry = stack_.back();
  if(!entry->matched) {
    // Output the element for this namespace node
    entry->matched = true;
    next_->startElementEvent(emptyToNull(entry->prefix.getRawBuffer()), emptyToNull(entry->uri.getRawBuffer()),
                             entry->localname.getRawBuffer());
  }

  // We always output namespace nodes
  next_->namespaceEvent(prefix, uri);
}

void QueryPathTreeFilter::StackEntry::addNode(const QueryPathNode *qpn)
{
  switch(qpn->getType()) {
  case QueryPathNode::DESCENDANT:
  case QueryPathNode::CHILD:
    if(qpn->isWildcardNodeType())
      nonElementChildren = true;
    break;
  case QueryPathNode::DESCENDANT_ATTR:
  case QueryPathNode::ATTRIBUTE:
    attrChildren = true;
    break;
  default: break;
  }

  children.push_back(qpn);
}

void QueryPathTreeFilter::StackEntry::addChildren(const QueryPathNode *qpn)
{
  const QueryPathNode *child = qpn->getFirstChild();
  while(child != 0) {
    addNode(child);
    child = child->getNextSibling();
  }
}

