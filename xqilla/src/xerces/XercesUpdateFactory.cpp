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

#include "XercesUpdateFactory.hpp"
#include "XercesNodeImpl.hpp"
#include "XercesSequenceBuilder.hpp"

#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/context/MessageListener.hpp>
#include <xqilla/exceptions/ASTException.hpp>
#include <xqilla/ast/XQValidate.hpp>
#include <xqilla/update/PendingUpdateList.hpp>
#include <xqilla/schema/DocumentCache.hpp>
#include <xqilla/dom-api/impl/XQillaNSResolverImpl.hpp>
#include <xqilla/items/ATUntypedAtomic.hpp>
#include <xqilla/schema/SchemaValidatorFilter.hpp>

#include <xercesc/dom/DOM.hpp>
#include <xercesc/validators/schema/SchemaSymbols.hpp>
#include <xercesc/framework/LocalFileFormatTarget.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/framework/XMLBuffer.hpp>

XERCES_CPP_NAMESPACE_USE;

void XercesUpdateFactory::applyPut(const PendingUpdate &update, DynamicContext *context)
{
  PutItem item(update.getValue().first()->asString(context), update.getTarget(), &update, context);

  std::pair<PutSet::iterator, bool> res = putSet_.insert(item);
  if(!res.second) {
    if(context->getMessageListener() != 0) {
      context->getMessageListener()->warning(X("In the context of this expression"), res.first->location);
    }

    XMLBuffer buf;
    buf.append(X("fn:put() called with the URI \""));
    buf.append(item.uri);
    buf.append(X("\" twice. [err:XUDY0031]"));

    XQThrow3(ASTException, X("XercesUpdateFactory::applyPut"), buf.getRawBuffer(), &update);
  }
}

#include <iostream>
#include <xqilla/utils/UTF8Str.hpp>

void printTypes(const char *label, const DOMNode *node, int indent = 0)
{
  if(indent == 0) std::cerr << "\n";

  if(node->getNodeType() == DOMNode::ELEMENT_NODE) {
    const XMLCh *typeURI, *typeName;
    XercesNodeImpl::typeUriAndName(node, typeURI, typeName);
    std::cerr << label << ":" << std::string(indent * 2, ' ')
              << "name: {" << UTF8(node->getNamespaceURI()) << "}" << UTF8(Axis::getLocalName(node))
              << ", type: {" << UTF8(typeURI) << "}" << UTF8(typeName) << "\n";

    DOMNode *child = node->getFirstChild();
    while(child) {
      printTypes(label, child, indent + 1);
      child = child->getNextSibling();
    }
  }
}

static DOMNode *importNodeFix(DOMDocument *doc, DOMNode *node, bool deep)
{
  DOMNode *newNode = doc->importNode(node, deep);
  // Xerces-C has a bug that doesn't copy the prefix correctly - jpcs
  if((node->getNodeType() == DOMNode::ELEMENT_NODE ||
      node->getNodeType() == DOMNode::ATTRIBUTE_NODE) &&
     node->getNamespaceURI() != 0 && *node->getNamespaceURI() != 0 &&
     node->getPrefix() != 0 && *node->getPrefix() != 0)
    newNode->setPrefix(node->getPrefix());
  return newNode;
}

void XercesUpdateFactory::applyInsertInto(const PendingUpdate &update, DynamicContext *context)
{
  const XercesNodeImpl *nodeImpl = (const XercesNodeImpl*)update.getTarget()->getInterface(Item::gXQilla);
  DOMNode *domnode = const_cast<DOMNode*>(nodeImpl->getDOMNode());
  DOMDocument *doc = const_cast<DOMDocument*>(XPath2Utils::getOwnerDoc(domnode));

  bool untyped = nodeImpl->dmNodeKind() == Node::element_string &&
    XPath2Utils::equals(nodeImpl->getTypeName(), DocumentCache::g_szUntyped) &&
    XPath2Utils::equals(nodeImpl->getTypeURI(), SchemaSymbols::fgURI_SCHEMAFORSCHEMA);

  bool containsElementOrText = false;

  Result children = update.getValue();
  Item::Ptr item;
  while((item = children->next(context)).notNull()) {
    const XercesNodeImpl *childImpl = (const XercesNodeImpl*)item->getInterface(Item::gXQilla);
    DOMNode *newChild = importNodeFix(doc, const_cast<DOMNode*>(childImpl->getDOMNode()), /*deep*/true);

    if(childImpl->dmNodeKind() == Node::element_string ||
       childImpl->dmNodeKind() == Node::text_string) {
      containsElementOrText = true;
    }

    // If the type-name property of $target is xs:untyped, then upd:setToUntyped() is invoked on each
    // element or attribute node in $content.
    if(!untyped) setTypes(newChild, childImpl->getDOMNode());

    // For each node in $content, the parent property is set to parent($target).
    // The children property of $target is modified to add the nodes in $content, preserving their order.
    domnode->appendChild(newChild);
  }

  // If at least one of the nodes in $content is an element or text node, upd:removeType($target) is invoked.
  if(containsElementOrText) {
    removeType(domnode);
  }

  addToPutSet(update.getTarget(), &update, context);
}

void XercesUpdateFactory::applyInsertAttributes(const PendingUpdate &update, DynamicContext *context)
{
  const XercesNodeImpl *nodeImpl = (const XercesNodeImpl*)update.getTarget()->getInterface(Item::gXQilla);
  DOMElement *element = (DOMElement*)nodeImpl->getDOMNode();
  DOMDocument *doc = const_cast<DOMDocument*>(XPath2Utils::getOwnerDoc(element));

  bool untyped = nodeImpl->dmNodeKind() == Node::element_string &&
    XPath2Utils::equals(nodeImpl->getTypeName(), DocumentCache::g_szUntyped) &&
    XPath2Utils::equals(nodeImpl->getTypeURI(), SchemaSymbols::fgURI_SCHEMAFORSCHEMA);

  Result children = update.getValue();
  Item::Ptr item;
  while((item = children->next(context)).notNull()) {
    const XercesNodeImpl *childImpl = (const XercesNodeImpl*)item->getInterface(Item::gXQilla);
    DOMNode *newChild = importNodeFix(doc, const_cast<DOMNode*>(childImpl->getDOMNode()), /*deep*/true);

    // 1. Error checks:
    //    a. If the QNames of any two attribute nodes in $content have implied namespace bindings that conflict with each other,
    //       a dynamic error is raised [err:XUDY0024].
    //    b. If the QName of any attribute node in $content has an implied namespace binding that conflicts with a namespace
    //       binding in the "namespaces" property of $target, a dynamic error is raised [err:XUDY0024].
    // Checks performed by UpdateFactory

    // If the type-name property of $target is xs:untyped, then upd:setToUntyped($A) is invoked.
    if(!untyped) setTypes(newChild, childImpl->getDOMNode());

    // The parent property of $A is set to $target.
    // attributes: Modified to include the nodes in $content.
    element->setAttributeNode((DOMAttr*)newChild);

  }

  // upd:removeType($target) is invoked.
  removeType(element);

  addToPutSet(update.getTarget(), &update, context);
}

void XercesUpdateFactory::applyReplaceValue(const PendingUpdate &update, DynamicContext *context)
{
  const XercesNodeImpl *nodeImpl = (const XercesNodeImpl*)update.getTarget()->getInterface(Item::gXQilla);
  DOMNode *domnode = const_cast<DOMNode*>(nodeImpl->getDOMNode());

  // 2. If $target is a text, comment, or processing instruction node: content of $target is set to $string-value.
  domnode->setNodeValue(update.getValue().first()->asString(context));

  if(domnode->getNodeType() == DOMNode::ATTRIBUTE_NODE) {
    // 1. If $target is an attribute node:
    //       a. string-value of $target is set to $string-value. (done above)
    //       b. upd:removeType($target) is invoked.
    removeType(domnode);
  }
  else if(domnode->getNodeType() == DOMNode::TEXT_NODE ||
          domnode->getNodeType() == DOMNode::CDATA_SECTION_NODE) {
    // 3. If $target is a text node, upd:removeType(parent($target)) is invoked.
    if(domnode->getParentNode() != 0)
      removeType(domnode->getParentNode());
  }

  addToPutSet(update.getTarget(), &update, context);
}

void XercesUpdateFactory::applyRename(const PendingUpdate &update, DynamicContext *context)
{
  const XercesNodeImpl *nodeImpl = (const XercesNodeImpl*)update.getTarget()->getInterface(Item::gXQilla);
  DOMNode *domnode = const_cast<DOMNode*>(nodeImpl->getDOMNode());

  ATQNameOrDerived *qname = (ATQNameOrDerived*)update.getValue().first().get();

  if(domnode->getNodeType() == DOMNode::PROCESSING_INSTRUCTION_NODE) {
    DOMProcessingInstruction *newPI = domnode->getOwnerDocument()->
      createProcessingInstruction(qname->getName(), domnode->getNodeValue());
    domnode->getParentNode()->replaceChild(newPI, domnode);
    domnode = newPI;
  }
  else {
    // If $newName has an implied namespace binding that conflicts with an existing namespace binding
    // in the namespaces property of $target, a dynamic error is raised [err:XUDY0024].

    // If $target has a parent, and $newName has an implied namespace binding that conflicts with a
    // namespace binding in the namespaces property of parent($target), a dynamic error is raised [err:XUDY0024].

    domnode->getOwnerDocument()->renameNode(domnode, qname->getURI(), qname->getName());
    if(qname->getURI() != 0 && *qname->getURI() != 0)
      domnode->setPrefix(qname->getPrefix());

    removeType(domnode);
  }

  // Deliberately create a new XercesNodeImpl, since the PI is actually
  // replaced, not just renamed, meaning it is no longer attached to the tree
  addToPutSet(nodeImpl, &update, context);
}

void XercesUpdateFactory::applyDelete(const PendingUpdate &update, DynamicContext *context)
{
  const XercesNodeImpl *nodeImpl = (const XercesNodeImpl*)update.getTarget()->getInterface(Item::gXQilla);
  DOMNode *domnode = const_cast<DOMNode*>(nodeImpl->getDOMNode());

  forDeletion_.insert(domnode);

  addToPutSet(update.getTarget(), &update, context);
}

void XercesUpdateFactory::applyInsertBefore(const PendingUpdate &update, DynamicContext *context)
{
  const XercesNodeImpl *nodeImpl = (const XercesNodeImpl*)update.getTarget()->getInterface(Item::gXQilla);
  DOMNode *domnode = const_cast<DOMNode*>(nodeImpl->getDOMNode());
  Node::Ptr parentNode = nodeImpl->dmParent(context);
  DOMNode *parent = domnode->getParentNode();
  DOMDocument *doc = const_cast<DOMDocument*>(XPath2Utils::getOwnerDoc(domnode));

  bool untyped = parentNode->dmNodeKind() == Node::element_string &&
    XPath2Utils::equals(parentNode->getTypeName(), DocumentCache::g_szUntyped) &&
    XPath2Utils::equals(parentNode->getTypeURI(), SchemaSymbols::fgURI_SCHEMAFORSCHEMA);

  bool containsElementOrText = false;

  Result children = update.getValue();
  Item::Ptr item;
  while((item = children->next(context)).notNull()) {
    const XercesNodeImpl *childImpl = (const XercesNodeImpl*)item->getInterface(Item::gXQilla);
    DOMNode *newChild = importNodeFix(doc, const_cast<DOMNode*>(childImpl->getDOMNode()), /*deep*/true);

    if(childImpl->dmNodeKind() == Node::element_string ||
       childImpl->dmNodeKind() == Node::text_string) {
      containsElementOrText = true;
    }

    // If the type-name property of parent($target) is xs:untyped, then upd:setToUntyped() is invoked on each
    // element or attribute node in $content.
    if(!untyped) setTypes(newChild, childImpl->getDOMNode());

    // For each node in $content, the parent property is set to parent($target).
    // The children property of parent($target) is modified to add the nodes in $content just before $target,
    // preserving their order.
    parent->insertBefore(newChild, domnode);
  }

  // If at least one of the nodes in $content is an element or text node, upd:removeType(parent($target)) is invoked.
  if(containsElementOrText) {
    removeType(parent);
  }

  addToPutSet(update.getTarget(), &update, context);
}

void XercesUpdateFactory::applyInsertAfter(const PendingUpdate &update, DynamicContext *context)
{
  const XercesNodeImpl *nodeImpl = (const XercesNodeImpl*)update.getTarget()->getInterface(Item::gXQilla);
  DOMNode *domnode = const_cast<DOMNode*>(nodeImpl->getDOMNode());
  DOMNode *before = domnode->getNextSibling();
  Node::Ptr parentNode = nodeImpl->dmParent(context);
  DOMNode *parent = domnode->getParentNode();
  DOMDocument *doc = const_cast<DOMDocument*>(XPath2Utils::getOwnerDoc(domnode));

  bool untyped = parentNode->dmNodeKind() == Node::element_string &&
    XPath2Utils::equals(parentNode->getTypeName(), DocumentCache::g_szUntyped) &&
    XPath2Utils::equals(parentNode->getTypeURI(), SchemaSymbols::fgURI_SCHEMAFORSCHEMA);

  bool containsElementOrText = false;

  Result children = update.getValue();
  Item::Ptr item;
  while((item = children->next(context)).notNull()) {
    const XercesNodeImpl *childImpl = (const XercesNodeImpl*)item->getInterface(Item::gXQilla);
    DOMNode *newChild = importNodeFix(doc, const_cast<DOMNode*>(childImpl->getDOMNode()), /*deep*/true);

    if(childImpl->dmNodeKind() == Node::element_string ||
       childImpl->dmNodeKind() == Node::text_string) {
      containsElementOrText = true;
    }

    // If the type-name property of parent($target) is xs:untyped, then upd:setToUntyped() is invoked on each
    // element or attribute node in $content.
    if(!untyped) setTypes(newChild, childImpl->getDOMNode());

    // For each node in $content, the parent property is set to parent($target).
    // The children property of parent($target) is modified to add the nodes in $content just before $target,
    // preserving their order.
    parent->insertBefore(newChild, before);
  }

  // If at least one of the nodes in $content is an element or text node, upd:removeType(parent($target)) is invoked.
  if(containsElementOrText) {
    removeType(parent);
  }

  addToPutSet(update.getTarget(), &update, context);
}

void XercesUpdateFactory::applyInsertAsFirst(const PendingUpdate &update, DynamicContext *context)
{
  const XercesNodeImpl *nodeImpl = (const XercesNodeImpl*)update.getTarget()->getInterface(Item::gXQilla);
  DOMNode *domnode = const_cast<DOMNode*>(nodeImpl->getDOMNode());
  DOMNode *firstChild = domnode->getFirstChild();
  DOMDocument *doc = const_cast<DOMDocument*>(XPath2Utils::getOwnerDoc(domnode));

  bool untyped = nodeImpl->dmNodeKind() == Node::element_string &&
    XPath2Utils::equals(nodeImpl->getTypeName(), DocumentCache::g_szUntyped) &&
    XPath2Utils::equals(nodeImpl->getTypeURI(), SchemaSymbols::fgURI_SCHEMAFORSCHEMA);

  bool containsElementOrText = false;

  Result children = update.getValue();
  Item::Ptr item;
  while((item = children->next(context)).notNull()) {
    const XercesNodeImpl *childImpl = (const XercesNodeImpl*)item->getInterface(Item::gXQilla);
    DOMNode *newChild = importNodeFix(doc, const_cast<DOMNode*>(childImpl->getDOMNode()), /*deep*/true);

    if(childImpl->dmNodeKind() == Node::element_string ||
       childImpl->dmNodeKind() == Node::text_string) {
      containsElementOrText = true;
    }

    // If the type-name property of $target is xs:untyped, then upd:setToUntyped() is invoked on each
    // element or attribute node in $content.
    if(!untyped) setTypes(newChild, childImpl->getDOMNode());

    // For each node in $content, the parent property is set to parent($target).
    // The children property of $target is modified to add the nodes in $content just before $target,
    // preserving their order.
    domnode->insertBefore(newChild, firstChild);
  }

  // If at least one of the nodes in $content is an element or text node, upd:removeType($target) is invoked.
  if(containsElementOrText) {
    removeType(domnode);
  }

  addToPutSet(update.getTarget(), &update, context);
}

void XercesUpdateFactory::applyInsertAsLast(const PendingUpdate &update, DynamicContext *context)
{
  const XercesNodeImpl *nodeImpl = (const XercesNodeImpl*)update.getTarget()->getInterface(Item::gXQilla);
  DOMNode *domnode = const_cast<DOMNode*>(nodeImpl->getDOMNode());
  DOMDocument *doc = const_cast<DOMDocument*>(XPath2Utils::getOwnerDoc(domnode));

  bool untyped = nodeImpl->dmNodeKind() == Node::element_string &&
    XPath2Utils::equals(nodeImpl->getTypeName(), DocumentCache::g_szUntyped) &&
    XPath2Utils::equals(nodeImpl->getTypeURI(), SchemaSymbols::fgURI_SCHEMAFORSCHEMA);

  bool containsElementOrText = false;

  Result children = update.getValue();
  Item::Ptr item;
  while((item = children->next(context)).notNull()) {
    const XercesNodeImpl *childImpl = (const XercesNodeImpl*)item->getInterface(Item::gXQilla);
    DOMNode *newChild = importNodeFix(doc, const_cast<DOMNode*>(childImpl->getDOMNode()), /*deep*/true);

    if(childImpl->dmNodeKind() == Node::element_string ||
       childImpl->dmNodeKind() == Node::text_string) {
      containsElementOrText = true;
    }

    // If the type-name property of $target is xs:untyped, then upd:setToUntyped() is invoked on each
    // element or attribute node in $content.
    if(!untyped) setTypes(newChild, childImpl->getDOMNode());

    // For each node in $content, the parent property is set to parent($target).
    // The children property of $target is modified to add the nodes in $content just before $target,
    // preserving their order.
    domnode->appendChild(newChild);
  }

  // If at least one of the nodes in $content is an element or text node, upd:removeType($target) is invoked.
  if(containsElementOrText) {
    removeType(domnode);
  }

  addToPutSet(update.getTarget(), &update, context);
}

void XercesUpdateFactory::applyReplaceNode(const PendingUpdate &update, DynamicContext *context)
{
  const XercesNodeImpl *nodeImpl = (const XercesNodeImpl*)update.getTarget()->getInterface(Item::gXQilla);
  DOMNode *domnode = const_cast<DOMNode*>(nodeImpl->getDOMNode());
  Node::Ptr parentNode = nodeImpl->dmParent(context);
  DOMNode *parent = domnode->getParentNode();
  DOMDocument *doc = const_cast<DOMDocument*>(XPath2Utils::getOwnerDoc(domnode));

  bool untyped = parentNode->dmNodeKind() == Node::element_string &&
    XPath2Utils::equals(parentNode->getTypeName(), DocumentCache::g_szUntyped) &&
    XPath2Utils::equals(parentNode->getTypeURI(), SchemaSymbols::fgURI_SCHEMAFORSCHEMA);

  Result children = update.getValue();
  Item::Ptr item;
  while((item = children->next(context)).notNull()) {
    const XercesNodeImpl *childImpl = (const XercesNodeImpl*)item->getInterface(Item::gXQilla);
    DOMNode *newChild = importNodeFix(doc, const_cast<DOMNode*>(childImpl->getDOMNode()), /*deep*/true);

    // 1b. If the type-name property of parent($target) is xs:untyped, then upd:setToUntyped() is invoked
    //     on each element node in $replacement.
    if(!untyped) setTypes(newChild, childImpl->getDOMNode());

    // 1a. For each node in $replacement, the parent property is set to parent($target).
    // 3b. If $target is an element, text, comment, or processing instruction node, the children property
    //     of parent($target) is modified to add the nodes in $replacement just before $target, preserving
    //     their order.
    parent->insertBefore(newChild, domnode);
  }

  // 2a. $target is marked for deletion.
  forDeletion_.insert(domnode);

  // 3c. upd:removeType(parent($target)) is invoked.
  removeType(parent);

  addToPutSet(update.getTarget(), &update, context);
}

void XercesUpdateFactory::applyReplaceAttribute(const PendingUpdate &update, DynamicContext *context)
{
  const XercesNodeImpl *nodeImpl = (const XercesNodeImpl*)update.getTarget()->getInterface(Item::gXQilla);
  DOMAttr *domnode = (DOMAttr*)nodeImpl->getDOMNode();
  Node::Ptr parentNode = nodeImpl->dmParent(context);
  DOMElement *element = domnode->getOwnerElement();
  DOMDocument *doc = element->getOwnerDocument();

  bool untyped = parentNode->dmNodeKind() == Node::element_string &&
    XPath2Utils::equals(parentNode->getTypeName(), DocumentCache::g_szUntyped) &&
    XPath2Utils::equals(parentNode->getTypeURI(), SchemaSymbols::fgURI_SCHEMAFORSCHEMA);

  Result children = update.getValue();
  Item::Ptr item;
  while((item = children->next(context)).notNull()) {
    const XercesNodeImpl *childImpl = (const XercesNodeImpl*)item->getInterface(Item::gXQilla);
    DOMNode *newChild = importNodeFix(doc, const_cast<DOMNode*>(childImpl->getDOMNode()), /*deep*/true);

    // 1. Error checks:
    //    a. If the QNames of any two attribute nodes in $replacement have implied namespace bindings that conflict with
    //       each other, a dynamic error is raised [err:XUDY0024].
    //    b. If the QName of any attribute node in $replacement has an implied namespace binding that conflicts with a
    //       namespace binding in the "namespaces" property of parent($target), a dynamic error is raised [err:XUDY0024].
    // Checks performed by UpdateFactory

    // 2b. If the type-name property of parent($target) is xs:untyped, then upd:setToUntyped() is invoked
    //     on each element node in $replacement.
    if(!untyped) setTypes(newChild, childImpl->getDOMNode());

    // 2a. For each node in $replacement, the parent property is set to parent($target).
    // 4a. If $target is an attribute node, the attributes property of parent($target) is modified by removing $target
    //     and adding the nodes in $replacement (if any).
    // 4b. If $target is an attribute node, the namespaces property of parent($target) is modified to include namespace
    //     bindings for any attribute namespace prefixes in $replacement that did not already have bindings.
    element->setAttributeNode((DOMAttr*)newChild);
  }

  // 3a. $target is marked for deletion.
  forDeletion_.insert(domnode);

  // 4d. upd:removeType(parent($target)) is invoked.
  removeType(element);

  // Use parentNode, since the attr replace could have removed the original attr
  addToPutSet(parentNode, &update, context);
}

void XercesUpdateFactory::applyReplaceElementContent(const PendingUpdate &update, DynamicContext *context)
{
  const XercesNodeImpl *nodeImpl = (const XercesNodeImpl*)update.getTarget()->getInterface(Item::gXQilla);
  DOMElement *domnode = (DOMElement*)nodeImpl->getDOMNode();

  // 1. For each node $C that is a child of $target, the parent property of $C is set to empty.
  DOMNode *child = domnode->getFirstChild();
  while(child != 0) {
    forDeletion_.insert(child);
    child = child->getNextSibling();
  }

  const XMLCh *value = update.getValue().first()->asString(context);
  if(value != 0 && *value != 0) {
    // 2. The parent property of $text is set to $target.
    // 3a. children is set to consist exclusively of $text. If $text is an empty sequence, then $target has
    //     no children.
    // 3b. typed-value and string-value are set to the content property of $text. If $text is an empty sequence,
    //     then typed-value is an empty sequence and string-value is an empty string.
    domnode->appendChild(domnode->getOwnerDocument()->createTextNode(value));
  }

  // 3c. upd:removeType($target) is invoked.
  removeType(domnode);

  addToPutSet(update.getTarget(), &update, context);
}

void XercesUpdateFactory::removeType(DOMNode *node)
{
  DOMNode *ancestor = node;

  // 1. If $N is an element node, its properties are changed as follows:
  if(node->getNodeType() == DOMNode::ELEMENT_NODE) {
    // a. If type-name is not equal to xs:untyped, then
    const XMLCh *typeURI, *typeName;
    XercesNodeImpl::typeUriAndName(node, typeURI, typeName);
    if(!XPath2Utils::equals(typeName, DocumentCache::g_szUntyped) ||
       !XPath2Utils::equals(typeURI, SchemaSymbols::fgURI_SCHEMAFORSCHEMA)) {
      // i.  type-name is set to xs:anyType
      XercesSequenceBuilder::setElementTypeInfo((DOMElement *)node, SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                                                SchemaSymbols::fgATTVAL_ANYTYPE);
      // ii. If the parent of N is an element node, then upd:removeType(parent($N)) is invoked.
      DOMNode *parent = node->getParentNode();
      if(parent && parent->getNodeType() == DOMNode::ELEMENT_NODE)
        removeType(parent);
    }

    // b. string-value is set equal to the concatenated contents of the text node descendants, in document order.
    // c. typed-value is set equal to the string-value property, as an instance of xs:untypedAtomic.
    // d. nilled, is-id, and is-idrefs are set to false.

    // Automatically done by changing the type
  }
  // 2. If $N is an attribute node, its properties are changed as follows:
  else if(node->getNodeType() == DOMNode::ATTRIBUTE_NODE) {
    //    a. type-name is set to xs:untypedAtomic.
    XercesSequenceBuilder::setAttributeTypeInfo((DOMAttr *)node, SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                                                ATUntypedAtomic::fgDT_UNTYPEDATOMIC);

    //    b. typed-value is set equal to the string-value property, as an instance of xs:untypedAtomic.
    //    c. is-id and is-idrefs are set to false.

    // Automatically done by changing the type

    //    d. If $N has a parent, upd:removeType(parent($N)) is invoked.
    ancestor = ((DOMAttr*)node)->getOwnerElement();
    if(ancestor) removeType(ancestor);
  }
  else return;

  // 3. The topmost ancestor of $N is marked for revalidation.
  if(ancestor) {
    while(ancestor->getParentNode() != 0)
      ancestor = ancestor->getParentNode();

    forRevalidation_.insert(ancestor);
  }
}

void XercesUpdateFactory::setTypes(DOMNode *node, const DOMNode *from)
{
  if(node->getNodeType() == DOMNode::ELEMENT_NODE) {
    const XMLCh *turi, *tname;
    XercesNodeImpl::typeUriAndName(from, turi, tname);
    XercesSequenceBuilder::setElementTypeInfo((DOMElement *)node, turi, tname);

    DOMNamedNodeMap *attrs = node->getAttributes();
    DOMNamedNodeMap *attrsfrom = from->getAttributes();
    for(unsigned int i = 0; i < attrs->getLength(); ++i) {
      DOMNode *a = attrs->item(i);
      DOMNode *afrom = attrsfrom->getNamedItemNS(a->getNamespaceURI(), Axis::getLocalName(a));
      if(afrom) setTypes(a, afrom);
    }

    DOMNode *child = node->getFirstChild();
    DOMNode *cfrom = from->getFirstChild();
    while(child) {
      if(child->getNodeType() == DOMNode::ELEMENT_NODE)
        setTypes(child, cfrom);
      child = child->getNextSibling();
      cfrom = cfrom->getNextSibling();
    }
  }
  else if(node->getNodeType() == DOMNode::ATTRIBUTE_NODE) {
    const XMLCh *turi, *tname;
    XercesNodeImpl::typeUriAndName(from, turi, tname);
    XercesSequenceBuilder::setAttributeTypeInfo((DOMAttr *)node, turi, tname);
  }
}

bool XercesUpdateFactory::PutItem::operator<(const PutItem &other) const
{
  if(uri == 0 && other.uri == 0) {
    return node->uniqueLessThan(other.node, context);
  }

  return XPath2Utils::compare(uri, other.uri) < 0;
}

void XercesUpdateFactory::addToPutSet(const Node::Ptr &node, const LocationInfo *location, DynamicContext *context)
{
  Node::Ptr root = node->root(context);

  Sequence docURISeq = root->dmDocumentURI(context);
  const XMLCh *docuri = 0;
  if(!docURISeq.isEmpty()) {
    docuri = docURISeq.first()->asString(context);
  }

  PutItem item(docuri, root, location, context);

  std::pair<PutSet::iterator, bool> res = putSet_.insert(item);
  if(!res.second && !res.first->node->equals(item.node)) {
    if(context->getMessageListener() != 0) {
      context->getMessageListener()->warning(X("In the context of this expression"), res.first->location);
    }

    XMLBuffer buf;
    buf.append(X("Document writing conflict for URI \""));
    buf.append(item.uri);
    buf.append(X("\""));

    XQThrow3(ASTException, X("XercesUpdateFactory::addToPutSet"), buf.getRawBuffer(), location);
  }
}

void XercesUpdateFactory::completeDeletions(DynamicContext *context)
{
  //    e. Finally, for each node marked for deletion by one of the update primitives listed above, let $N be the node that is marked
  //       for deletion, and let $P be its parent node. The following actions are applied:
  //          i. The parent property of $N is set to empty.
  //         ii. If $N is an attribute node, the attributes property of $P is modified to remove $N.
  //        iii. If $N is a non-attribute node, the children property of $P is modified to remove $N.
  //         iv. If $N is an element, attribute, or text node, and $P is an element node, then upd:removeType($P) is invoked.

  for(DOMNodeSet::iterator i = forDeletion_.begin(); i != forDeletion_.end(); ++i) {
    DOMNode *domnode = *i;

    if(domnode->getNodeType() == DOMNode::ATTRIBUTE_NODE) {
      DOMAttr *attr = (DOMAttr*)domnode;
      DOMElement *owner = attr->getOwnerElement();
      if(owner != 0) {
        owner->removeAttributeNode(attr);
        removeType(owner);
      }
    }
    else {
      DOMNode *parent = domnode->getParentNode();
      if(parent != 0) {
        parent->removeChild(domnode);
        if(domnode->getNodeType() == DOMNode::ELEMENT_NODE ||
           domnode->getNodeType() == DOMNode::TEXT_NODE ||
           domnode->getNodeType() == DOMNode::CDATA_SECTION_NODE) {
          removeType(parent);
        }
      }
    }
  }
}

static inline DOMElement *nextElement(DOMNode *node)
{
  while(node && node->getNodeType() != DOMNode::ELEMENT_NODE) {
    node = node->getNextSibling();
  }
  return (DOMElement*)node;
}

class RevalidationEventHandler : public EventHandler
{
public:
  RevalidationEventHandler(DOMNode *node)
    : node_(0),
      child_(node)
  {
  }

  virtual void startDocumentEvent(const XMLCh *documentURI, const XMLCh *encoding)
  {
    assert(child_ && child_->getNodeType() == DOMNode::DOCUMENT_NODE);
    node_ = child_;
    child_ = nextElement(node_->getFirstChild());
  }

  virtual void endDocumentEvent()
  {
    assert(node_ && node_->getNodeType() == DOMNode::DOCUMENT_NODE);
    child_ = node_;
    node_ = 0;
  }

  virtual void startElementEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname)
  {
    assert(child_ && child_->getNodeType() == DOMNode::ELEMENT_NODE);
    node_ = child_;
    child_ = nextElement(node_->getFirstChild());
  }

  virtual void endElementEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname,
                               const XMLCh *typeURI, const XMLCh *typeName)
  {
    assert(node_ && node_->getNodeType() == DOMNode::ELEMENT_NODE);

    // Copy the element's type
    const XMLCh *oldTypeURI, *oldTypeName;
    XercesNodeImpl::typeUriAndName(node_, oldTypeURI, oldTypeName);
    if(!XPath2Utils::equals(oldTypeName, typeName) ||
       !XPath2Utils::equals(oldTypeURI, typeURI)) {
      XercesSequenceBuilder::setElementTypeInfo((DOMElement *)node_, typeURI, typeName);
    }

    child_ = node_;
    node_ = node_->getParentNode();
    child_ = nextElement(child_->getNextSibling());
  }

  virtual void attributeEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname, const XMLCh *value,
                              const XMLCh *typeURI, const XMLCh *typeName)
  {
    assert(node_ && node_->getNodeType() == DOMNode::ELEMENT_NODE);

    DOMNamedNodeMap *attrs = node_->getAttributes();
    DOMAttr *at = (DOMAttr*)attrs->getNamedItemNS(uri, localname);

    // Add the attribute
    if(!at) {
      at = node_->getOwnerDocument()->createAttributeNS(uri, localname);
      if(prefix && *prefix)
        at->setPrefix(prefix);
      attrs->setNamedItemNS(at);
    }

    // Copy the schema normalized value
    at->setNodeValue(value);

    // Copy the attribute's type
    const XMLCh *oldTypeURI, *oldTypeName;
    XercesNodeImpl::typeUriAndName(at, oldTypeURI, oldTypeName);
    if(!XPath2Utils::equals(oldTypeName, typeName) ||
       !XPath2Utils::equals(oldTypeURI, typeURI)) {
      XercesSequenceBuilder::setAttributeTypeInfo(at, typeURI, typeName);
    }
  }

  virtual void piEvent(const XMLCh *target, const XMLCh *value)
  {
  }

  virtual void textEvent(const XMLCh *value)
  {
  }

  virtual void textEvent(const XMLCh *chars, unsigned int length)
  {
  }

  virtual void commentEvent(const XMLCh *value)
  {
  }

  virtual void namespaceEvent(const XMLCh *prefix, const XMLCh *uri)
  {
  }

  virtual void atomicItemEvent(AnyAtomicType::AtomicObjectType type, const XMLCh *value, const XMLCh *typeURI,
                               const XMLCh *typeName)
  {
  }

  virtual void endEvent()
  {
  }

private:
  DOMNode *node_;
  DOMNode *child_;
};

void XercesUpdateFactory::completeRevalidation(DynamicContext *context)
{
  if(valMode_ == DocumentCache::VALIDATION_SKIP) return;

  for(DOMNodeSet::iterator i = forRevalidation_.begin(); i != forRevalidation_.end(); ++i) {
    DOMNode *top = *i;

    // TBD element default/normalized values - jpcs

    // Stream the node through the schema validator, and back to the RevalidationEventHandler,
    // which will write the information back into the node
    RevalidationEventHandler reh(top);
    SchemaValidatorFilter svf(valMode_ == DocumentCache::VALIDATION_STRICT, &reh,
                              context->getDocumentCache()->getGrammarResolver(),
                              context->getMemoryManager(), 0);
    ValidateArgumentCheckFilter argCheck(&svf, valMode_, context, 0);

    XercesNodeImpl::generateEvents(top, &argCheck, true, false);
    argCheck.endEvent();
  }
}

void XercesUpdateFactory::completeUpdate(DynamicContext *context)
{
  completeDeletions(context);
  completeRevalidation(context);

  // Call the URIResolvers to handle the PutSet
  for(PutSet::iterator i = putSet_.begin(); i != putSet_.end(); ++i) {
    try {
      if(!context->putDocument(i->node, i->uri)) {
        XMLBuffer buf;
        buf.append(X("Writing of updated document failed for URI \""));
        buf.append(i->uri);
        buf.append(X("\""));

        XQThrow3(ASTException, X("XercesUpdateFactory::completeUpdate"), buf.getRawBuffer(), i->location);
      }
    }
    catch(XQException& e) {
      if(e.getXQueryLine() == 0) {
        e.setXQueryPosition(i->location);
      }
      throw e;
    }
  }
}

