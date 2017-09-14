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

#include <set>
#include <map>

#include <xqilla/context/UpdateFactory.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/context/MessageListener.hpp>
#include <xqilla/update/PendingUpdateList.hpp>
#include <xqilla/exceptions/DynamicErrorException.hpp>
#include <xqilla/utils/XPath2Utils.hpp>

#include <xercesc/framework/XMLBuffer.hpp>

XERCES_CPP_NAMESPACE_USE;
using namespace std;

class pucompare {
public:
  pucompare(const DynamicContext *context)
    : context_(context) {}

  bool operator()(const PendingUpdate *first, const PendingUpdate *second) const
  {
    return first->getTarget()->uniqueLessThan(second->getTarget(), context_);
  }

private:
  const DynamicContext *context_;
};

typedef set<const PendingUpdate*, pucompare> PendingUpdateSet;

////////////////////////////////////////////////////////////////////////////////////////////////////

class nodecompare {
public:
  nodecompare(const DynamicContext *context)
    : context_(context) {}

  bool operator()(const Node::Ptr &first, const Node::Ptr &second) const
  {
    return first->uniqueLessThan(second, context_);
  }

private:
  const DynamicContext *context_;
};

class AttrName {
public:
  AttrName(const ATQNameOrDerived::Ptr &name, const LocationInfo *info = 0)
    : name_(name), info_(info) {}

  const XMLCh *getURI() const
  {
    return name_->getURI();
  }

  const XMLCh *getPrefix() const
  {
    return name_->getPrefix();
  }

  const XMLCh *getName() const
  {
    return name_->getName();
  }

  bool operator<(const AttrName &o) const
  {
    return name_->compare(o.name_, 0) < 0;
  }

  ATQNameOrDerived::Ptr name_;
  const LocationInfo *info_;
};

typedef set<AttrName> AttrNameSet;

class AttrNamespace
{
public:
  AttrNamespace(const XMLCh *prefix, const XMLCh *uri = 0, const LocationInfo *info = 0)
    : prefix_(prefix), uri_(uri), info_(info) {}

  bool operator<(const AttrNamespace &o) const
  {
    return XPath2Utils::compare(prefix_, o.prefix_) < 0;
  }

  const XMLCh *prefix_;
  const XMLCh *uri_;
  const LocationInfo *info_;
};

typedef set<AttrNamespace> AttrNamespaceSet;

class AttrMapValue {
public:

  void addAttr(const ATQNameOrDerived::Ptr &name, DynamicContext *context, const LocationInfo *loc)
  {
    MessageListener *mlistener = context->getMessageListener();

    // Add the attribute name, and check if it already existed
    pair<AttrNameSet::iterator, bool> result = names.insert(AttrName(name, loc));
    if(!result.second) {
      if(result.first->info_ && mlistener) {
        mlistener->warning(X("In the context of this expression"), result.first->info_);
      }
      XMLBuffer buf;
      buf.append(X("Attribute {"));
      buf.append(name->getURI());
      buf.append(X("}"));
      buf.append(name->getName());
      buf.append(X(" already exists [err:XUDY0021]"));
      XQThrow3(DynamicErrorException, X("AttrMapValue::addAttr"), buf.getRawBuffer(), loc);
    }
  }

  void removeAttr(const ATQNameOrDerived::Ptr &name)
  {
    names.erase(name);
  }

  void addNamespace(const ATQNameOrDerived::Ptr &name, DynamicContext *context, const LocationInfo *loc)
  {
    MessageListener *mlistener = context->getMessageListener();

    // Add the namespace binding, and check if it is already mapped to a different URI
    pair<AttrNamespaceSet::iterator, bool> result =
      namespaces.insert(AttrNamespace(name->getPrefix(), name->getURI(), loc));
    if(!result.second && !XPath2Utils::equals(result.first->uri_, name->getURI())) {
      if(result.first->info_ && mlistener) {
        mlistener->warning(X("In the context of this expression"), result.first->info_);
      }
      XMLBuffer buf;
      buf.append(X("Implied namespace binding for the attribute (\""));
      buf.append(name->getPrefix());
      buf.append(X("\" -> \""));
      buf.append(name->getURI());
      buf.append(X("\") conflicts with those already existing on the target element [err:XUDY0024]"));
      XQThrow3(DynamicErrorException, X("AttrMapValue::addNamespace"), buf.getRawBuffer(), loc);
    }
  }

  AttrNameSet names;
  AttrNamespaceSet namespaces;
};

class AttrMap : public map<Node::Ptr, AttrMapValue, nodecompare>
{
public:
  AttrMap(const DynamicContext *context) : map<Node::Ptr, AttrMapValue, nodecompare>(nodecompare(context)) {}

  AttrMapValue &get(const Node::Ptr &node, DynamicContext *context)
  {
    iterator found = find(node);
    if(found == end()) {
      AttrMapValue &value = insert(value_type(node, AttrMapValue())).first->second;

      // Add the existing attributes
      Result attrs = node->dmAttributes(context, 0);
      Node::Ptr tmp;
      while((tmp = (Node*)attrs->next(context).get()).notNull()) {
        value.names.insert(tmp->dmNodeName(context));
      }

      // Add the existing namespace bindings
      Result namespaces = node->dmNamespaceNodes(context, 0);
      while((tmp = (Node*)namespaces->next(context).get()).notNull()) {
        ATQNameOrDerived::Ptr name = tmp->dmNodeName(context);
        value.namespaces.insert(AttrNamespace(name.isNull() ? 0 : name->getName(), tmp->dmStringValue(context)));
      }

      return value;
    }
    return found->second;
  }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

void UpdateFactory::applyUpdates(const PendingUpdateList &pul, DynamicContext *context, DocumentCache::ValidationMode valMode)
{
  PendingUpdateList::const_iterator i;
  MessageListener *mlistener = context->getMessageListener();

  valMode_ = valMode;

  // 1. Checks the update primitives on $pul for compatibility. Raises a dynamic error if any of the following conditions are detected:
  //    a. Two or more upd:rename primitives on the merged list have the same target node [err:XUDY0015].
  //    b. Two or more upd:replaceNode primitives on the merged list have the same target node [err:XUDY0016].
  //    c. Two or more upd:replaceValue primitives on the merged list have the same target node [err:XUDY0017].
  //    d. Two or more upd:replaceElementContent primitives on $pul have the same target node [err:XUDY0017].
  {
    PendingUpdateSet replaceNodeSet = PendingUpdateSet(pucompare(context));
    PendingUpdateSet replaceValueSet = PendingUpdateSet(pucompare(context));
    PendingUpdateSet renameSet = PendingUpdateSet(pucompare(context));

    for(i = pul.begin(); i != pul.end(); ++i) {
      switch(i->getType()) {
      case PendingUpdate::REPLACE_ELEMENT_CONTENT:
      case PendingUpdate::REPLACE_VALUE: {
        pair<PendingUpdateSet::iterator, bool> res = replaceValueSet.insert(&(*i));
        if(!res.second) {
          if(mlistener != 0) {
            mlistener->warning(X("In the context of this expression"), *res.first);
          }
          XQThrow3(DynamicErrorException, X("UApplyUpdates::createSequence"),
                   X("Incompatible updates - two replace value expressions have the same target node [err:XUDY0017]"), &(*i));
        }
        break;
      }
      case PendingUpdate::REPLACE_ATTRIBUTE:
      case PendingUpdate::REPLACE_NODE: {
        pair<PendingUpdateSet::iterator, bool> res = replaceNodeSet.insert(&(*i));
        if(!res.second) {
          if(mlistener != 0) {
            mlistener->warning(X("In the context of this expression"), *res.first);
          }
          XQThrow3(DynamicErrorException, X("UApplyUpdates::createSequence"),
                   X("Incompatible updates - two replace expressions have the same target node [err:XUDY0016]"), &(*i));
        }
        break;
      }
      case PendingUpdate::RENAME: {
        pair<PendingUpdateSet::iterator, bool> res = renameSet.insert(&(*i));
        if(!res.second) {
          if(mlistener != 0) {
            mlistener->warning(X("In the context of this expression"), *res.first);
          }
          XQThrow3(DynamicErrorException, X("UApplyUpdates::createSequence"),
                   X("Incompatible updates - two rename expressions have the same target node [err:XUDY0015]"), &(*i));
        }
        break;
      }
      case PendingUpdate::PUT:
      case PendingUpdate::INSERT_INTO:
      case PendingUpdate::INSERT_ATTRIBUTES:
      case PendingUpdate::PUDELETE:
      case PendingUpdate::INSERT_BEFORE:
      case PendingUpdate::INSERT_AFTER:
      case PendingUpdate::INSERT_INTO_AS_FIRST:
      case PendingUpdate::INSERT_INTO_AS_LAST:
        break;
      }
    }
  }

  // Perform some checks on any attribute and namespace binding updates. Check for:
  //   1. Attribute name clashes on the original node, and from other updates
  //   2. Implied namespace binding clashes on the original node, and from other updates
  {
    AttrMap attrCheck(context);

    for(i = pul.begin(); i != pul.end(); ++i) {
      switch(i->getType()) {
      case PendingUpdate::INSERT_ATTRIBUTES: {
        AttrMapValue &value = attrCheck.get(i->getTarget(), context);

        Result children = i->getValue();
        Node::Ptr node;
        while((node = (Node*)children->next(context).get()).notNull()) {
          ATQNameOrDerived::Ptr name = node->dmNodeName(context);

          // Check if the attribute already exists
          value.addAttr(name, context, &(*i));

          // Check for a namespace clash
          if(name->getPrefix() != 0 && *(name->getPrefix()) != 0)
            value.addNamespace(name, context, &(*i));
        }
        break;
      }
      case PendingUpdate::RENAME: {
        ATQNameOrDerived *name = (ATQNameOrDerived*)i->getValue().first().get();

        if(i->getTarget()->dmNodeKind() == Node::element_string) {
          AttrMapValue &value = attrCheck.get(i->getTarget(), context);
          value.addNamespace(name, context, &(*i));
        }
        else if(i->getTarget()->dmNodeKind() == Node::attribute_string) {
          Node::Ptr parentNode = i->getTarget()->dmParent(context);
          if(parentNode.notNull()) {
            AttrMapValue &value = attrCheck.get(parentNode, context);

            value.removeAttr(i->getTarget()->dmNodeName(context));

            // Check if the new attribute already exists
            value.addAttr(name, context, &(*i));

            // Check for a namespace clash
            if(name->getPrefix() != 0 && *(name->getPrefix()) != 0)
              value.addNamespace(name, context, &(*i));
          }
        }
        break;
      }
      default: break;
      }
    }
    
    for(i = pul.begin(); i != pul.end(); ++i) {
      switch(i->getType()) {
      case PendingUpdate::REPLACE_ATTRIBUTE: {
        Node::Ptr parentNode = i->getTarget()->dmParent(context);
        AttrMapValue &value = attrCheck.get(parentNode, context);

        value.removeAttr(i->getTarget()->dmNodeName(context));

        Result children = i->getValue();
        Node::Ptr node;
        while((node = (Node*)children->next(context).get()).notNull()) {
          ATQNameOrDerived::Ptr name = node->dmNodeName(context);

          // Check if the attribute already exists
          value.addAttr(name, context, &(*i));

          // Check for a namespace clash
          if(name->getPrefix() != 0 && *(name->getPrefix()) != 0)
            value.addNamespace(name, context, &(*i));
        }
        break;
      }
      default: break;
      }
    }
  }

  // Call checkUpdates to allow implementations to perform their own checks
  checkUpdates(pul, context, valMode);
	  
  // We apply PUT first, to catch the duplicate puts properly
  for(i = pul.begin(); i != pul.end(); ++i) {
    switch(i->getType()) {
    case PendingUpdate::PUT:
      applyPut(*i, context);
      break;
    case PendingUpdate::INSERT_INTO:
    case PendingUpdate::INSERT_ATTRIBUTES:
    case PendingUpdate::REPLACE_VALUE:
    case PendingUpdate::RENAME:
    case PendingUpdate::PUDELETE:
    case PendingUpdate::REPLACE_ELEMENT_CONTENT:
    case PendingUpdate::REPLACE_ATTRIBUTE:
    case PendingUpdate::REPLACE_NODE:
    case PendingUpdate::INSERT_BEFORE:
    case PendingUpdate::INSERT_AFTER:
    case PendingUpdate::INSERT_INTO_AS_FIRST:
    case PendingUpdate::INSERT_INTO_AS_LAST:
      break;
    }
  }

#

  // 2. The semantics of all the update primitives on $pul are made effective, in the following order:
  //    a. First, all upd:insertInto, upd:insertAttributes, upd:replaceValue, and upd:rename primitives are applied.
  for(i = pul.begin(); i != pul.end(); ++i) {
    switch(i->getType()) {
    case PendingUpdate::INSERT_INTO:
      applyInsertInto(*i, context);
      break;
    case PendingUpdate::INSERT_ATTRIBUTES:
      applyInsertAttributes(*i, context);
      break;
    case PendingUpdate::REPLACE_VALUE:
      applyReplaceValue(*i, context);
      break;
    case PendingUpdate::RENAME:
      applyRename(*i, context);
      break;
    case PendingUpdate::PUDELETE:
    case PendingUpdate::PUT:
    case PendingUpdate::REPLACE_ELEMENT_CONTENT:
    case PendingUpdate::REPLACE_ATTRIBUTE:
    case PendingUpdate::REPLACE_NODE:
    case PendingUpdate::INSERT_BEFORE:
    case PendingUpdate::INSERT_AFTER:
    case PendingUpdate::INSERT_INTO_AS_FIRST:
    case PendingUpdate::INSERT_INTO_AS_LAST:
      break;
    }
  }

  //    b. Next, all upd:insertBefore, upd:insertAfter, upd:insertIntoAsFirst, and upd:insertIntoAsLast primitives are applied.
  for(i = pul.begin(); i != pul.end(); ++i) {
    switch(i->getType()) {
    case PendingUpdate::INSERT_BEFORE:
      applyInsertBefore(*i, context);
      break;
    case PendingUpdate::INSERT_AFTER:
      applyInsertAfter(*i, context);
      break;
    case PendingUpdate::INSERT_INTO_AS_FIRST:
      applyInsertAsFirst(*i, context);
      break;
    case PendingUpdate::INSERT_INTO_AS_LAST:
      applyInsertAsLast(*i, context);
      break;

    case PendingUpdate::PUT:
    case PendingUpdate::INSERT_INTO:
    case PendingUpdate::INSERT_ATTRIBUTES:
    case PendingUpdate::REPLACE_VALUE:
    case PendingUpdate::RENAME:
    case PendingUpdate::PUDELETE:
    case PendingUpdate::REPLACE_ELEMENT_CONTENT:
    case PendingUpdate::REPLACE_ATTRIBUTE:
    case PendingUpdate::REPLACE_NODE:
      break;
    }
  }

  //    c. Next, all upd:replaceNode primitives are applied.
  for(i = pul.begin(); i != pul.end(); ++i) {
    switch(i->getType()) {
    case PendingUpdate::REPLACE_NODE:
      applyReplaceNode(*i, context);
      break;
    case PendingUpdate::REPLACE_ATTRIBUTE:
      applyReplaceAttribute(*i, context);
      break;

    case PendingUpdate::INSERT_BEFORE:
    case PendingUpdate::INSERT_AFTER:
    case PendingUpdate::INSERT_INTO_AS_FIRST:
    case PendingUpdate::INSERT_INTO_AS_LAST:
    case PendingUpdate::PUT:
    case PendingUpdate::INSERT_INTO:
    case PendingUpdate::INSERT_ATTRIBUTES:
    case PendingUpdate::REPLACE_VALUE:
    case PendingUpdate::RENAME:
    case PendingUpdate::PUDELETE:
    case PendingUpdate::REPLACE_ELEMENT_CONTENT:
      break;
    }
  }

  //    d. Next, all upd:replaceElementContent primitives are applied.
  for(i = pul.begin(); i != pul.end(); ++i) {
    switch(i->getType()) {
    case PendingUpdate::REPLACE_ELEMENT_CONTENT:
      applyReplaceElementContent(*i, context);
      break;

    case PendingUpdate::REPLACE_NODE:
    case PendingUpdate::REPLACE_ATTRIBUTE:
    case PendingUpdate::INSERT_BEFORE:
    case PendingUpdate::INSERT_AFTER:
    case PendingUpdate::INSERT_INTO_AS_FIRST:
    case PendingUpdate::INSERT_INTO_AS_LAST:
    case PendingUpdate::PUT:
    case PendingUpdate::INSERT_INTO:
    case PendingUpdate::INSERT_ATTRIBUTES:
    case PendingUpdate::REPLACE_VALUE:
    case PendingUpdate::RENAME:
    case PendingUpdate::PUDELETE:
      break;
    }
  }

  //    e. Finally, all upd:delete primitives are applied.
  for(i = pul.begin(); i != pul.end(); ++i) {
    switch(i->getType()) {
    case PendingUpdate::PUDELETE:
      applyDelete(*i, context);
      break;
    case PendingUpdate::INSERT_INTO:
    case PendingUpdate::INSERT_ATTRIBUTES:
    case PendingUpdate::REPLACE_VALUE:
    case PendingUpdate::RENAME:
    case PendingUpdate::PUT:
    case PendingUpdate::REPLACE_ELEMENT_CONTENT:
    case PendingUpdate::REPLACE_ATTRIBUTE:
    case PendingUpdate::REPLACE_NODE:
    case PendingUpdate::INSERT_BEFORE:
    case PendingUpdate::INSERT_AFTER:
    case PendingUpdate::INSERT_INTO_AS_FIRST:
    case PendingUpdate::INSERT_INTO_AS_LAST:
      break;
    }
  }

  // 3. If, as a net result of the above steps, the children property of some node contains adjacent text nodes, these adjacent text
  //    nodes are merged into a single text node. The string-value of the resulting text node is the concatenated string-values of the
  //    adjacent text nodes, with no intervening space added. The node identity of the resulting text node is implementation-dependent.
  //    * Handled in the called methods

  // 4. If, as a net result of the above steps, the children property of some node contains an empty text node, that empty text node is
  //    deleted from the children property.
  //    * Handled in the called methods

  // 5. For each document or element node $top that was marked for revalidation by one of the earlier steps, upd:revalidate($top) is
  //    invoked.

  // 6. If the resulting XDM instance violates any constraint specified in [XQuery/XPath Data Model (XDM)], a dynamic error is raised
  //    [err:XUDY0021].
  //
  //    Note:
  //    For example, a data model constraint violation might occur if multiple attributes with the same parent have the same qualified
  //    name (see Section 6.2.1 OverviewDM.)

  // 7. The upd:applyUpdates operation is atomic with respect to the data model. In other words, if upd:applyUpdates terminates
  //    normally, the resulting XDM instance reflects the result of all update primitives; but if upd:applyUpdates raises an error, the
  //    resulting XDM instance reflects no changes. Atomicity is guaranteed only with respect to operations on XDM instances, and only
  //    with respect to error conditions specified in this document.
  //
  //    Note:
  //    The results of implementation-dependent error conditions such as exceeding resource limits are beyond the scope of this
  //    specification.

  // 8. Propagation of XDM changes to an underlying persistent store is beyond the scope of this specification. For example, the effect
  //    on persistent storage of deleting a node that has no parent is beyond the scope of this specification.
  completeUpdate(context);
}

