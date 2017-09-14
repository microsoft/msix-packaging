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
#include "FastXDMNodeImpl.hpp"

#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/context/ContextHelpers.hpp>
#include <xqilla/runtime/Sequence.hpp>
#include <xqilla/runtime/Result.hpp>
#include <xqilla/exceptions/ItemException.hpp>
#include <xqilla/items/ATQNameOrDerived.hpp>
#include <xqilla/items/ATUntypedAtomic.hpp>
#include <xqilla/schema/DocumentCache.hpp>
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/utils/XMLChCompare.hpp>
#include <xqilla/dom-api/XQillaNSResolver.hpp>
#include <xqilla/axis/NodeTest.hpp>
#include <xqilla/events/EventSerializer.hpp>
#include <xqilla/events/NSFixupFilter.hpp>
#include <xqilla/context/ItemFactory.hpp>

#include <xercesc/framework/MemBufFormatTarget.hpp>
#include <xercesc/validators/datatype/ListDatatypeValidator.hpp>
#include <xercesc/validators/datatype/UnionDatatypeValidator.hpp>
#include <xercesc/validators/schema/ComplexTypeInfo.hpp>
#include <xercesc/validators/schema/SchemaElementDecl.hpp>
#include <xercesc/util/XMLUri.hpp>
#include <xercesc/util/XMLURL.hpp>

#include <set>

#if defined(XERCES_HAS_CPP_NAMESPACE)
XERCES_CPP_NAMESPACE_USE
#endif

const XMLCh FastXDMNodeImpl::fastxdm_string[] = { 'f', 'a', 's', 't', 'x', 'd', 'm', 0 };

FastXDMNodeImpl::FastXDMNodeImpl(const FastXDMDocument::Ptr &document, const FastXDMDocument::Node *node)
  : document_(document),
    node_(node)
{  
}

FastXDMNodeImpl::~FastXDMNodeImpl()
{
}

void *FastXDMNodeImpl::getInterface(const XMLCh *name) const
{
  if(name == fastxdm_string)
    return (void *)this;
  return 0;
}

const XMLCh *FastXDMNodeImpl::asString(const DynamicContext* context) const
{
  XPath2MemoryManager *mm = context->getMemoryManager();

  MemBufFormatTarget target(1023, mm);
  EventSerializer writer(&target, mm);
  NSFixupFilter nsfilter(&writer, mm);
  FastXDMDocument::toEvents(node_, &nsfilter);
  nsfilter.endEvent();
  return XMLString::replicate((XMLCh*)target.getRawBuffer(), mm);
}

void FastXDMNodeImpl::generateEvents(EventHandler *events, const DynamicContext *context,
                                     bool preserveNS, bool preserveType) const
{
  FastXDMDocument::toEvents(node_, events, preserveNS, preserveType);
}

bool FastXDMNodeImpl::hasInstanceOfType(const XMLCh* typeURI, const XMLCh* typeName, const DynamicContext* context) const
{
    const XMLCh* uri, *name;
    getTypeUriAndName(uri, name);
    return context->isTypeOrDerivedFromType(uri, name, typeURI, typeName);
}


Sequence FastXDMNodeImpl::dmBaseURI(const DynamicContext* context) const
{
  static XMLCh base_str[] = { 'b', 'a', 's', 'e', 0 };

  switch(node_->nodeKind) {
  case FastXDMDocument::DOCUMENT: {
    const XMLCh *baseURI = context->getBaseURI();

    const XMLCh* docURI = node_->data.document.documentURI;
    if(docURI != 0 && *docURI != 0)
      baseURI = docURI;

    if(baseURI == 0 || *baseURI == 0)
      return Sequence(context->getMemoryManager());
    return Sequence(context->getItemFactory()->createAnyURI(baseURI, context), context->getMemoryManager());
  }
  case FastXDMDocument::ELEMENT: {
    const XMLCh *baseURI = context->getBaseURI();

    Node::Ptr parent = dmParent(context);
    if(parent.notNull()) {
      Sequence pb = parent->dmBaseURI(context);
      if(!pb.isEmpty())
        baseURI = pb.first()->asString(context);
    }

    if(node_->data.element.attributes.ptr != 0) {
      NodeTest xmlBase;
      xmlBase.setNodeName(base_str);
      xmlBase.setNodeUri(XMLUni::fgXMLURIName);
      xmlBase.setTypeWildcard();

      Item::Ptr item = getAxisResult(XQStep::ATTRIBUTE, &xmlBase, const_cast<DynamicContext*>(context), 0)->
        next(const_cast<DynamicContext*>(context));
      if(item.notNull()) {
        const XMLCh *uri = ((Node*)item.get())->dmStringValue(context);

        if(uri && *uri) {
          if(baseURI && *baseURI) {
            try {
              XMLUri temp(baseURI, context->getMemoryManager());
              XMLUri temp2(&temp, uri, context->getMemoryManager());
              baseURI = context->getMemoryManager()->getPooledString(temp2.getUriText());
            }
            catch(const MalformedURLException &ex) {
              baseURI = uri;
            }
          }
          else baseURI = uri;
        }
      }
    }

    if(baseURI == NULL || *baseURI == 0)
      return Sequence(context->getMemoryManager());
    return Sequence(context->getItemFactory()->createAnyURI(baseURI, context), context->getMemoryManager());
  }
  case FastXDMDocument::TEXT:
  case FastXDMDocument::COMMENT:
  case FastXDMDocument::PROCESSING_INSTRUCTION: {
    Node::Ptr parent = dmParent(context);
    if(parent.notNull()) {
      return parent->dmBaseURI(context);
    }
    return Sequence(context->getMemoryManager());
  }
  case FastXDMDocument::MARKER:
    break;
  }
    
  XQThrow2(ItemException, X("FastXDMNodeImpl::dmNodeKind"), X("Unknown node type."));
}

const XMLCh* FastXDMNodeImpl::dmNodeKind(void) const
{
  switch(node_->nodeKind) {
  case FastXDMDocument::DOCUMENT:
    return document_string;
  case FastXDMDocument::ELEMENT:
    return element_string;
  case FastXDMDocument::TEXT:
    return text_string;
  case FastXDMDocument::COMMENT:
    return comment_string;
  case FastXDMDocument::PROCESSING_INSTRUCTION:
    return processing_instruction_string;
  case FastXDMDocument::MARKER:
    break;
  }
    
  XQThrow2(ItemException, X("FastXDMNodeImpl::dmNodeKind"), X("Unknown node type."));
}


ATQNameOrDerived::Ptr FastXDMNodeImpl::dmNodeName(const DynamicContext* context) const
{
  switch(node_->nodeKind) {
  case FastXDMDocument::ELEMENT:
    return context->getItemFactory()->createQName(node_->data.element.uri, node_->data.element.prefix, node_->data.element.localname, context);
  case FastXDMDocument::PROCESSING_INSTRUCTION:
    return context->getItemFactory()->createQName(XMLUni::fgZeroLenString, XMLUni::fgZeroLenString, node_->data.other.target, context);
  default:
    break;
  }
  return 0;
}

static inline void addStringValueToBuffer(const FastXDMDocument::Node *node, unsigned int endLevel, XMLBuffer& buffer)
{
  for(; node->level > endLevel; ++node) {
    if(node->nodeKind == FastXDMDocument::TEXT)
      buffer.append(node->data.other.value);
  }
}

const XMLCh* FastXDMNodeImpl::dmStringValue(const DynamicContext* context) const
{
  switch(node_->nodeKind)
  {
  case FastXDMDocument::DOCUMENT:
  case FastXDMDocument::ELEMENT: {
    XMLBuffer str(1023, context->getMemoryManager());
    addStringValueToBuffer(node_ + 1, node_->level, str);
    return context->getMemoryManager()->getPooledString(str.getRawBuffer());
  }
  case FastXDMDocument::TEXT:
  case FastXDMDocument::COMMENT:
  case FastXDMDocument::PROCESSING_INSTRUCTION:
    return node_->data.other.value;
  case FastXDMDocument::MARKER:
    break;
  }
  return XMLUni::fgZeroLenString;
}

static Sequence getListTypeTypedValue(const XMLCh *stringVal, DatatypeValidator *dtv, const DynamicContext* context)
{
  AutoDelete<BaseRefVectorOf<XMLCh> > tokenVector(XMLString::tokenizeString(stringVal));
  Sequence s(tokenVector->size(), context->getMemoryManager());

  //the actual type we want
  DatatypeValidator* theItemTypeDTV = ((ListDatatypeValidator*)dtv)->getItemTypeDTV();
  if(theItemTypeDTV->getType() == DatatypeValidator::Union) {
    RefVectorOf<DatatypeValidator>* membersDV = ((UnionDatatypeValidator*)theItemTypeDTV)->getMemberTypeValidators();
    unsigned int size = (unsigned int) membersDV->size();
    // find the first datatype in the union that validates the piece
    for ( unsigned int j = 0; j < tokenVector->size(); j++ ) {
      const XMLCh* szPiece=tokenVector->elementAt(j);
      bool bFound=false;
      for (unsigned int i=0; i<size; i++) {
        DatatypeValidator* pDV=membersDV->elementAt(i);
        try {
          pDV->validate(szPiece, NULL, context->getMemoryManager());
          const XMLCh* itemTypeDTVName = pDV->getTypeLocalName();
          const XMLCh* itemTypeDTVUri = pDV->getTypeUri();
          // TBD avoid double lookup of DatatypeValidator - jpcs
          s.addItem(context->getItemFactory()->createDerivedFromAtomicType(itemTypeDTVUri, itemTypeDTVName, szPiece, context));
          bFound=true;
          break;
        }
        catch (XMLException&) {
          //absorbed
        }
      }
      if(!bFound)
        XQThrow2(ItemException, X("FastXDMNodeImpl::getListTypeTypedValue"),
                 X("Value in list doesn't validate with any of the componenets of the union type"));
    }
  } 
  else {
    const XMLCh* itemTypeDTVName = theItemTypeDTV->getTypeLocalName();
    const XMLCh* itemTypeDTVUri = theItemTypeDTV->getTypeUri();

    for ( unsigned int j = 0; j < tokenVector->size(); j++ )
      s.addItem(context->getItemFactory()->createDerivedFromAtomicType(itemTypeDTVUri, itemTypeDTVName, tokenVector->elementAt(j), context));
  }
  return s;
}

// Minimal implementation for resolving xs:QName and xs:NOTATION values from the
// namespace bindings in an element
class FastXDMNSResolverImpl : public XQillaNSResolver
{
public:
  FastXDMNSResolverImpl(const Node::Ptr &node, DynamicContext *context)
    : node_(node), context_(context) {}

  virtual const XMLCh* lookupNamespaceURI(const XMLCh* prefix) const
  {
    NodeTest test;
    test.setNodeName(prefix);
    test.setNameWildcard();
    test.setTypeWildcard();

    Item::Ptr found = node_->getAxisResult(XQStep::NAMESPACE, &test, context_, 0)->next(context_);
    if(found.notNull()) {
      return ((Node*)found.get())->dmStringValue(context_);
    }
    return 0;
  }

  virtual const XMLCh* lookupPrefix(const XMLCh* uri) const { return 0; }
  
  virtual void addNamespaceBinding(const XMLCh* prefix, const XMLCh* uri) {}

  virtual void release() {}

protected:
  Node::Ptr node_;
  DynamicContext *context_;
};

Sequence FastXDMNodeImpl::dmTypedValue(DynamicContext* context) const
{
  switch(node_->nodeKind) {
  case FastXDMDocument::DOCUMENT: {
    XMLBuffer str(1023, context->getMemoryManager());
    addStringValueToBuffer(node_ + 1, node_->level, str);
    return Sequence(context->getItemFactory()->createUntypedAtomic(str.getRawBuffer(), context), context->getMemoryManager());
  }
  case FastXDMDocument::ELEMENT: {
    // Data Model, § 7.2.3 and 7.2.4
    // If the nilled property is true, its typed-value is ().
    if(((const ATBooleanOrDerived*)dmNilled(context).get())->isTrue())
      return Sequence(context->getMemoryManager());

    const XMLCh *typeUri, *typeName;
    getTypeUriAndName(typeUri, typeName);

    XMLBuffer str(1023, context->getMemoryManager());
    addStringValueToBuffer(node_ + 1, node_->level, str);

    // If the element is of type xdt:untyped or xs:anyType, its typed-value is its dm:string-value as an xdt:untypedAtomic.
    if((XPath2Utils::equals(typeName, DocumentCache::g_szUntyped) || XPath2Utils::equals(typeName, SchemaSymbols::fgATTVAL_ANYTYPE)) &&
       XPath2Utils::equals(typeUri, SchemaSymbols::fgURI_SCHEMAFORSCHEMA)) {
      return Sequence(context->getItemFactory()->createUntypedAtomic(str.getRawBuffer(), context), context->getMemoryManager());
    }

    FastXDMNSResolverImpl newNSScope(this, context);
    AutoNsScopeReset jan(context, &newNSScope);

    // If the element has a simple type or a complex type with simple content: it's typed value is compute 
    // as described in 3.3.1.2 Atomic Value Type Names. The result is a sequence of zero or more atomic values 
    // derived from the string-value of the node and its type in a way that is consistent with XML Schema validation.
    DatatypeValidator *dtv = context->getDocumentCache()->getDatatypeValidator(typeUri, typeName);
    if(dtv) {
      if(dtv->getType() == DatatypeValidator::List)
        return getListTypeTypedValue(str.getRawBuffer(), dtv, context);
      else {
        // TBD avoid double lookup of DatatypeValidator - jpcs
        return Sequence(context->getItemFactory()->createDerivedFromAtomicType(typeUri, typeName, str.getRawBuffer(), context),
                        context->getMemoryManager());
      }
    }

    ComplexTypeInfo *cti = context->getDocumentCache()->getComplexTypeInfo(typeUri, typeName);
    assert(cti);

    switch(cti->getContentType()) {
    case SchemaElementDecl::Simple: {
      DatatypeValidator *dtv = cti->getDatatypeValidator();
      assert(dtv != 0);

      if(dtv->getType() == DatatypeValidator::List)
        return getListTypeTypedValue(str.getRawBuffer(), dtv, context);
      else {
        // TBD avoid double lookup of DatatypeValidator - jpcs
        return Sequence(context->getItemFactory()->createDerivedFromAtomicType(dtv->getTypeUri(), dtv->getTypeLocalName(), str.getRawBuffer(), context),
                        context->getMemoryManager());
      }
    }
    case SchemaElementDecl::Empty:
      // If the element is empty: its typed-value is the empty sequence.
      return Sequence(context->getMemoryManager());
    case SchemaElementDecl::Mixed_Simple:
    case SchemaElementDecl::Mixed_Complex:
      // If the element has a complex type with mixed content, its typed-value is its dm:string-value as an xdt:untypedAtomic.
      return Sequence(context->getItemFactory()->createUntypedAtomic(str.getRawBuffer(), context), context->getMemoryManager());
    case SchemaElementDecl::Children:
      break;
    }
    // Otherwise, the element must be a complex type with element-only content. 
    // The typed-value of such an element is undefined. Attempting to access this property with the dm:typed-value 
    // accessor always raises an error.
    XQThrow2(ItemException, X("NodeImpl::dmTypedValue"),
             X("Attempt to get typed value from a complex type with non-mixed complex content [err:FOTY0012]"));
  }
  case FastXDMDocument::TEXT:
    return Sequence(context->getItemFactory()->createUntypedAtomic(node_->data.other.value, context), context->getMemoryManager());
  case FastXDMDocument::COMMENT:
  case FastXDMDocument::PROCESSING_INSTRUCTION:
    return Sequence(context->getItemFactory()->createString(node_->data.other.value, context), context->getMemoryManager());
  case FastXDMDocument::MARKER:
    break;
  }
  return Sequence(context->getMemoryManager());
}

Sequence FastXDMNodeImpl::dmDocumentURI(const DynamicContext* context) const
{
  if(node_->nodeKind != FastXDMDocument::DOCUMENT)
    return Sequence(context->getMemoryManager());
    
  const XMLCh* docURI = node_->data.document.documentURI;
  if(docURI == NULL || *docURI == 0)
    return Sequence(context->getMemoryManager());

  return Sequence(context->getItemFactory()->createAnyURI(docURI, context), context->getMemoryManager());
}

ATQNameOrDerived::Ptr FastXDMNodeImpl::dmTypeName(const DynamicContext* context) const
{
  switch(node_->nodeKind) {
  case FastXDMDocument::DOCUMENT:
  case FastXDMDocument::COMMENT:
  case FastXDMDocument::PROCESSING_INSTRUCTION:
  case FastXDMDocument::MARKER:
    break;
  case FastXDMDocument::ELEMENT:
  case FastXDMDocument::TEXT:
    const XMLCh* typeUri, *typeName;
    getTypeUriAndName(typeUri, typeName);
    return context->getItemFactory()->createQName(typeUri, XMLUni::fgZeroLenString, typeName, context);
  }

  return 0;
}

ATBooleanOrDerived::Ptr FastXDMNodeImpl::dmNilled(const DynamicContext* context) const
{
  if(node_->nodeKind != FastXDMDocument::ELEMENT)
    return 0;

  return context->getItemFactory()->createBoolean(false, context);
}

bool FastXDMNodeImpl::lessThan(const Node::Ptr &other, const DynamicContext *context) const
{
  const FastXDMNodeImpl *otherImpl = (const FastXDMNodeImpl*)other->getInterface(fastxdm_string);
  if(otherImpl != 0) {
    // Order first by the address of the FastXDMDocument object
    if(document_.get() != otherImpl->getDocument().get())
      return document_.get() < otherImpl->getDocument().get();

    // Then order by the node index
    return node_ < otherImpl->getNode();
  }

  const FastXDMAttributeNodeImpl *attrImpl = (const FastXDMAttributeNodeImpl*)other->
    getInterface(FastXDMAttributeNodeImpl::fastxdm_attr_string);
  if(attrImpl != 0) {
    // Order first by the address of the FastXDMDocument object
    if(document_.get() != attrImpl->getDocument().get())
      return document_.get() < attrImpl->getDocument().get();

    // Then order by the node index
    if(node_ == attrImpl->getAttribute()->owner.ptr) return true;
    return node_ < attrImpl->getAttribute()->owner.ptr;
  }

  const FastXDMNamespaceNodeImpl *nsImpl = (const FastXDMNamespaceNodeImpl*)other->getInterface(FastXDMNamespaceNodeImpl::fastxdm_ns_string);
  if(nsImpl != 0) {
    if(nsImpl->getOwner().isNull()) return false;
    return lessThan(nsImpl->getOwner(), context);
  }

  // It's not a Fast XDM implementation Node, so it can't
  // be from the same tree as us - jpcs

  // TBD find a better way to order these - jpcs

  // Order them according to the address of their roots
  return this->root(context).get() < other->root(context).get();
}

bool FastXDMNodeImpl::equals(const Node::Ptr &other) const
{
  const FastXDMNodeImpl *otherImpl = (const FastXDMNodeImpl*)other->getInterface(fastxdm_string);
  if(otherImpl == 0) return false;

  if(document_.get() != otherImpl->getDocument().get())
    return false;

  return node_ == otherImpl->getNode();
}

bool FastXDMNodeImpl::uniqueLessThan(const Node::Ptr &other, const DynamicContext *context) const
{
  return lessThan(other, context);
}

ATBooleanOrDerived::Ptr FastXDMNodeImpl::dmIsId(const DynamicContext* context) const
{
  switch(node_->nodeKind) {
  case FastXDMDocument::ELEMENT:
    if(hasInstanceOfType(SchemaSymbols::fgURI_SCHEMAFORSCHEMA, XMLUni::fgIDString, context))
      return context->getItemFactory()->createBoolean(true, context);
  case FastXDMDocument::TEXT:
  case FastXDMDocument::DOCUMENT:
  case FastXDMDocument::COMMENT:
  case FastXDMDocument::PROCESSING_INSTRUCTION:
  case FastXDMDocument::MARKER:
    break;
  }
  return context->getItemFactory()->createBoolean(false, context);
}

ATBooleanOrDerived::Ptr FastXDMNodeImpl::dmIsIdRefs(const DynamicContext* context) const
{
  switch(node_->nodeKind) {
  case FastXDMDocument::ELEMENT:
    if(hasInstanceOfType(SchemaSymbols::fgURI_SCHEMAFORSCHEMA, XMLUni::fgIDRefString, context) ||
       hasInstanceOfType(SchemaSymbols::fgURI_SCHEMAFORSCHEMA, XMLUni::fgIDRefsString, context))
      return context->getItemFactory()->createBoolean(true, context);
  case FastXDMDocument::TEXT:
  case FastXDMDocument::DOCUMENT:
  case FastXDMDocument::COMMENT:
  case FastXDMDocument::PROCESSING_INSTRUCTION:
  case FastXDMDocument::MARKER:
    break;
  }
  return context->getItemFactory()->createBoolean(false, context);
}

void FastXDMNodeImpl::getTypeUriAndName(const XMLCh*& uri, const XMLCh*& name) const
{
  switch(node_->nodeKind) {
  case FastXDMDocument::ELEMENT:
    uri = node_->data.element.typeURI;
    name = node_->data.element.typeName;
    return;
  case FastXDMDocument::TEXT:
    uri = SchemaSymbols::fgURI_SCHEMAFORSCHEMA;
    name = ATUntypedAtomic::fgDT_UNTYPEDATOMIC;
    return;
  case FastXDMDocument::DOCUMENT:
  case FastXDMDocument::COMMENT:
  case FastXDMDocument::PROCESSING_INSTRUCTION:
  case FastXDMDocument::MARKER:
    break;
  }
  XQThrow2(ItemException, X("FastXDMNodeImpl::getTypeUriAndName"), X("Tried to get type information on Node other than element, attribute or text"));
}

const XMLCh* FastXDMNodeImpl::getTypeName() const
{
  const XMLCh* uri, *name;
  getTypeUriAndName(uri,name);
  return name;
}

const XMLCh* FastXDMNodeImpl::getTypeURI() const
{
  const XMLCh* uri, *name;
  getTypeUriAndName(uri,name);
  return uri;
}

static inline Item::Ptr testNode(const FastXDMDocument::Ptr &document, const FastXDMDocument::Node *node, const NodeTest *nodeTest, DynamicContext *context)
{
  if(nodeTest == 0) return new FastXDMNodeImpl(document, node);

  SequenceType::ItemType *itemType = nodeTest->getItemType();
  if(itemType != 0) {
    Node::Ptr result = new FastXDMNodeImpl(document, node);
    if(itemType->matches(result, context)) {
      return result;
    }
  }

  switch(node->nodeKind) {
  case FastXDMDocument::DOCUMENT: {
    if(!nodeTest->getTypeWildcard() && nodeTest->getNodeType() != Node::document_string) return 0;
    if(!nodeTest->getNameWildcard() || !nodeTest->getNamespaceWildcard()) return 0;
    break;
  }
  case FastXDMDocument::ELEMENT: {
    if(nodeTest->getNodeType() != Node::element_string && !nodeTest->getTypeWildcard()) return 0;
    if(!XPath2Utils::equals(node->data.element.localname, nodeTest->getNodeName()) && !nodeTest->getNameWildcard()) return 0;
    if(!XPath2Utils::equals(node->data.element.uri, nodeTest->getNodeUri()) && !nodeTest->getNamespaceWildcard()) return 0;
    break;
  }
  case FastXDMDocument::TEXT: {
    if(nodeTest->getNodeType() != Node::text_string) {
      if(!nodeTest->getTypeWildcard() || nodeTest->getHasChildren()) return 0;
    }
    if(!nodeTest->getNameWildcard() || !nodeTest->getNamespaceWildcard()) return 0;
    break;
  }
  case FastXDMDocument::PROCESSING_INSTRUCTION: {
    if(nodeTest->getTypeWildcard()) { if(nodeTest->getHasChildren()) return 0; }
    else if(nodeTest->getNodeType() != Node::processing_instruction_string) return 0;
    if(!nodeTest->getNameWildcard() && !XPath2Utils::equals(node->data.other.target, nodeTest->getNodeName())) return 0;
    if(!nodeTest->getNamespaceWildcard()) return 0;
    break;
  }
  case FastXDMDocument::COMMENT: {
    if(nodeTest->getTypeWildcard()) { if(nodeTest->getHasChildren()) return 0; }
    else if(nodeTest->getNodeType() != Node::comment_string) return 0;
    if(!nodeTest->getNameWildcard() || !nodeTest->getNamespaceWildcard()) return 0;
    break;
  }
  case FastXDMDocument::MARKER:
    return 0;
  }
  return new FastXDMNodeImpl(document, node);
}

static inline Item::Ptr testAttribute(const FastXDMDocument::Ptr &document, const FastXDMDocument::Attribute *attr, const NodeTest *nodeTest, DynamicContext *context)
{
  if(nodeTest != 0) {
      SequenceType::ItemType *itemType = nodeTest->getItemType();
      if(itemType != 0) {
        Node::Ptr result = new FastXDMAttributeNodeImpl(document, attr);
        if(itemType->matches(result, context)) {
          return result;
        }
      }

      if(nodeTest->getNodeType() != Node::attribute_string) {
        if(!nodeTest->getTypeWildcard() || nodeTest->getHasChildren()) return 0;
      }
      if(!XPath2Utils::equals(attr->localname, nodeTest->getNodeName()) && !nodeTest->getNameWildcard()) return 0;
      if(!XPath2Utils::equals(attr->uri, nodeTest->getNodeUri()) && !nodeTest->getNamespaceWildcard()) return 0;
  }
  return new FastXDMAttributeNodeImpl(document, attr);
}

static inline bool testNamespace(const FastXDMNamespaceNodeImpl::Ptr &ns, const NodeTest *nodeTest, DynamicContext *context)
{
  if(nodeTest != 0) {
    return nodeTest->filterNode(ns, context);
  }
  return true;
}

class FastXDMAxis : public ResultImpl
{
public:
  FastXDMAxis(const LocationInfo *info, const FastXDMDocument::Ptr &document, const FastXDMDocument::Node *node, const NodeTest *nodeTest)
    : ResultImpl(info),
      document_(document),
      node_(node),
      nodeTest_(nodeTest)
  {
  }

  Item::Ptr next(DynamicContext *context)
  {
    const FastXDMDocument::Node *node = 0;
    while((node = nextNode()) != 0) {
      context->testInterrupt();

      Item::Ptr result = testNode(document_, node, nodeTest_, context);
      if(result.notNull()) return result;
    }

    return 0;
  }

  virtual const FastXDMDocument::Node *nextNode() = 0;

  virtual std::string asString(DynamicContext *context, int indent) const { return ""; }

protected:
  FastXDMDocument::Ptr document_;
  const FastXDMDocument::Node *node_;
  const NodeTest *nodeTest_;
};

class FastXDMChildAxis : public FastXDMAxis
{
public:
  FastXDMChildAxis(const LocationInfo *info, const FastXDMDocument::Ptr &document, const FastXDMDocument::Node *node, const NodeTest *nodeTest)
    : FastXDMAxis(info, document, node, nodeTest),
      toDo_(true)
  {
  }

  const FastXDMDocument::Node *nextNode()
  {
    if(toDo_) {
      toDo_ = false;
      if((node_ + 1)->level <= node_->level)
        node_ = 0;
      else ++node_;
    }
    else if(node_ != 0)
      node_ = node_->nextSibling.ptr;

    return node_;
  }

protected:
  bool toDo_;
};

class FastXDMDescendantAxis : public FastXDMAxis
{
public:
  FastXDMDescendantAxis(const LocationInfo *info, const FastXDMDocument::Ptr &document, const FastXDMDocument::Node *node, const NodeTest *nodeTest)
    : FastXDMAxis(info, document, node, nodeTest),
      endLevel_(node->level)
  {
  }

  const FastXDMDocument::Node *nextNode()
  {
    if(node_ == 0) return 0;

    ++node_;
    if(node_->level <= endLevel_) {
      node_ = 0;
      return 0;
    }

    return node_;
  }

protected:
  unsigned int endLevel_;
};

class FastXDMDescendantOrSelfAxis : public FastXDMAxis
{
public:
  FastXDMDescendantOrSelfAxis(const LocationInfo *info, const FastXDMDocument::Ptr &document, const FastXDMDocument::Node *node, const NodeTest *nodeTest)
    : FastXDMAxis(info, document, node, nodeTest),
      endLevel_(node->level)
  {
  }

  const FastXDMDocument::Node *nextNode()
  {
    if(node_ == 0) return 0;

    const FastXDMDocument::Node *ret = node_;

    ++node_;
    if(node_->level <= endLevel_) {
      node_ = 0;
    }

    return ret;
  }

protected:
  unsigned int endLevel_;
};

class FastXDMAncestorAxis : public FastXDMAxis
{
public:
  FastXDMAncestorAxis(const LocationInfo *info, const FastXDMDocument::Ptr &document, const FastXDMDocument::Node *node, const NodeTest *nodeTest)
    : FastXDMAxis(info, document, node, nodeTest)
  {
  }

  const FastXDMDocument::Node *nextNode()
  {
    if(node_ == 0) return 0;
    node_ = FastXDMDocument::getParent(node_);
    return node_;
  }
};

class FastXDMAncestorOrSelfAxis : public FastXDMAxis
{
public:
  FastXDMAncestorOrSelfAxis(const LocationInfo *info, const FastXDMDocument::Ptr &document, const FastXDMDocument::Node *node, const NodeTest *nodeTest)
    : FastXDMAxis(info, document, node, nodeTest),
      toDo_(true)
  {
  }

  const FastXDMDocument::Node *nextNode()
  {
    if(toDo_) {
      toDo_ = false;
    }
    else if(node_ != 0) {
      node_ = FastXDMDocument::getParent(node_);
    }
    return node_;
  }

protected:
  bool toDo_;
};

class FastXDMParentAxis : public FastXDMAxis
{
public:
  FastXDMParentAxis(const LocationInfo *info, const FastXDMDocument::Ptr &document, const FastXDMDocument::Node *node, const NodeTest *nodeTest)
    : FastXDMAxis(info, document, node, nodeTest)
  {
  }

  const FastXDMDocument::Node *nextNode()
  {
    if(node_ == 0) return 0;
    const FastXDMDocument::Node *parent = FastXDMDocument::getParent(node_);
    node_ = 0;
    return parent;
  }
};

class FastXDMFollowingSiblingAxis : public FastXDMAxis
{
public:
  FastXDMFollowingSiblingAxis(const LocationInfo *info, const FastXDMDocument::Ptr &document, const FastXDMDocument::Node *node, const NodeTest *nodeTest)
    : FastXDMAxis(info, document, node, nodeTest)
  {
  }

  const FastXDMDocument::Node *nextNode()
  {
    if(node_ == 0) return 0;
    node_ = node_->nextSibling.ptr;
    return node_;
  }
};

class FastXDMPrecedingSiblingAxis : public FastXDMAxis
{
public:
  FastXDMPrecedingSiblingAxis(const LocationInfo *info, const FastXDMDocument::Ptr &document, const FastXDMDocument::Node *node, const NodeTest *nodeTest)
    : FastXDMAxis(info, document, node, nodeTest),
      level_(node->level)
  {
  }

  const FastXDMDocument::Node *nextNode()
  {
    if(node_ == 0 || node_->level == 0) {
      node_ = 0;
      return 0;
    }

    --node_;
    while(node_->level > level_)
      --node_;

    if(node_->level < level_) {
      node_ = 0;
    }
    return node_;
  }

protected:
  unsigned int level_;
};

class FastXDMFollowingAxis : public FastXDMAxis
{
public:
  FastXDMFollowingAxis(const LocationInfo *info, const FastXDMDocument::Ptr &document, const FastXDMDocument::Node *node, const NodeTest *nodeTest)
    : FastXDMAxis(info, document, node, nodeTest),
      toDo_(true)
  {
  }

  FastXDMFollowingAxis(const LocationInfo *info, const FastXDMDocument::Ptr &document, const FastXDMDocument::Attribute *attr, const NodeTest *nodeTest)
    : FastXDMAxis(info, document, attr->owner.ptr, nodeTest),
      toDo_(false)
  {
  }

  FastXDMFollowingAxis(const LocationInfo *info, const FastXDMDocument::Ptr &document, const FastXDMNamespaceNodeImpl *ns, const NodeTest *nodeTest)
    : FastXDMAxis(info, document, ns->getOwner()->getNode(), nodeTest),
      toDo_(false)
  {
  }

  const FastXDMDocument::Node *nextNode()
  {
    if(node_ == 0) return 0;

    if(toDo_) {
      toDo_ = false;

      // Find the next node in document order after node_,
      // that isn't one of it's descendants
      const FastXDMDocument::Node *result = node_->nextSibling.ptr;
      if(result == 0) {
        result = node_ + 1;
        while(result->level > node_->level)
          ++result;
      }

      node_ = result;
    }
    else {
      ++node_;
    }

    if(node_->nodeKind == FastXDMDocument::MARKER) {
      node_ = 0;
      return 0;
    }

    return node_;
  }

protected:
  bool toDo_;
};

class FastXDMPrecedingAxis : public FastXDMAxis
{
public:
  FastXDMPrecedingAxis(const LocationInfo *info, const FastXDMDocument::Ptr &document, const FastXDMDocument::Node *node, const NodeTest *nodeTest)
    : FastXDMAxis(info, document, node, nodeTest),
      level_(node->level - 1)
  {
  }

  const FastXDMDocument::Node *nextNode()
  {
    if(node_ == 0) return 0;

    while(node_->level != 0) {
      --node_;

      // Skip the ancestors of the context node
      if(node_->level == level_) {
        level_ = node_->level - 1;
        continue;
      }

      return node_;
    }

    node_ = 0;
    return 0;
  }

protected:
  unsigned int level_;
};

class FastXDMSelfAxis : public FastXDMAxis
{
public:
  FastXDMSelfAxis(const LocationInfo *info, const FastXDMDocument::Ptr &document, const FastXDMDocument::Node *node, const NodeTest *nodeTest)
    : FastXDMAxis(info, document, node, nodeTest)
  {
  }

  const FastXDMDocument::Node *nextNode()
  {
    const FastXDMDocument::Node *result = node_;
    node_ = 0;
    return result;
  }
};

class FastXDMAttributeAxis : public ResultImpl
{
public:
  FastXDMAttributeAxis(const LocationInfo *info, const FastXDMDocument::Ptr &document, const FastXDMDocument::Node *node, const NodeTest *nodeTest)
    : ResultImpl(info),
      document_(document),
      node_(node),
      attr_(node->data.element.attributes.ptr),
      nodeTest_(nodeTest)
  {
  }

  Item::Ptr next(DynamicContext *context)
  {
    const FastXDMDocument::Attribute *attr = attr_;
    while(attr->owner.ptr == node_) {
      context->testInterrupt();

      ++attr_;

      Item::Ptr result = testAttribute(document_, attr, nodeTest_, context);
      if(result.notNull()) return result;

      attr = attr_;
    }

    return 0;
  }

  virtual std::string asString(DynamicContext *context, int indent) const { return ""; }

protected:
  FastXDMDocument::Ptr document_;
  const FastXDMDocument::Node *node_;
  const FastXDMDocument::Attribute *attr_;
  const NodeTest *nodeTest_;
};

class FastXDMNamespaceAxis : public ResultImpl
{
public:
  FastXDMNamespaceAxis(const LocationInfo *info, const FastXDMNodeImpl::Ptr &node, const NodeTest *nodeTest)
    : ResultImpl(info),
      owner_(node),
      node_(node->getNode()),
      attr_(0),
      ns_(0),
      order_(0),
      nodeTest_(nodeTest),
      state_(CHECK_ELEMENT)
  {
  }

  Item::Ptr next(DynamicContext *context)
  {
    FastXDMNamespaceNodeImpl::Ptr result;
    while(result.isNull() || !testNamespace(result, nodeTest_, context)) {
      result = 0;

      switch(state_) {
      case CHECK_ELEMENT:
        if(done_.insert(node_->data.element.prefix).second && node_->data.element.uri != 0) {
          result = new FastXDMNamespaceNodeImpl(owner_, node_->data.element.prefix,
                                                node_->data.element.uri, order_++);
        }
        state_ = CHECK_ATTR;
        attr_ = node_->data.element.attributes.ptr;
        break;
      case CHECK_ATTR:
        if(attr_ != 0 && attr_->owner.ptr == node_) {
          if(attr_->uri != 0 && done_.insert(attr_->prefix).second) {
            result = new FastXDMNamespaceNodeImpl(owner_, attr_->prefix, attr_->uri, order_++);
          }
          ++attr_;
        }
        else {
          state_ = CHECK_NS;
          ns_ = node_->data.element.namespaces.ptr;
        }
        break;
      case CHECK_NS:
        if(ns_ != 0 && ns_->owner.ptr == node_) {
          if(done_.insert(ns_->prefix).second && ns_->uri != 0) {
            result = new FastXDMNamespaceNodeImpl(owner_, ns_->prefix, ns_->uri, order_++);
          }
          ++ns_;
        }
        else {
          state_ = CHECK_ELEMENT;
          node_ = FastXDMDocument::getParent(node_);
          if(node_ == 0 || node_->nodeKind != FastXDMDocument::ELEMENT) {
            state_ = DO_XML;
          }
        }
        break;
      case DO_XML:
        if(done_.insert(XMLUni::fgXMLString).second) {
          result = new FastXDMNamespaceNodeImpl(owner_, XMLUni::fgXMLString, XMLUni::fgXMLURIName, order_++);
        }
        state_ = DONE;
        owner_ = 0;
        break;
      case DONE:
        return 0;
      }
    }

    return result;
  }

  virtual std::string asString(DynamicContext *context, int indent) const { return ""; }

protected:
  FastXDMNodeImpl::Ptr owner_;
  const FastXDMDocument::Node *node_;
  const FastXDMDocument::Attribute *attr_;
  const FastXDMDocument::Namespace *ns_;
  unsigned int order_;
  const NodeTest *nodeTest_;

  enum {
    CHECK_ELEMENT,
    CHECK_ATTR,
    CHECK_NS,
    DO_XML,
    DONE
  } state_;

  typedef std::set<const XMLCh*, XMLChSort> DoneSet;
  DoneSet done_;
};

Node::Ptr FastXDMNodeImpl::root(const DynamicContext* context) const
{
  return new FastXDMNodeImpl(document_, document_->getNode(0));
}

Node::Ptr FastXDMNodeImpl::dmParent(const DynamicContext* context) const
{
  if(node_->level == 0) return 0;

  const FastXDMDocument::Node *parent = node_ - 1;
  while(parent->level >= node_->level)
    --parent;

  return new FastXDMNodeImpl(document_, parent);
}

Result FastXDMNodeImpl::dmAttributes(const DynamicContext* context, const LocationInfo *info) const
{
  if(node_->nodeKind == FastXDMDocument::ELEMENT && node_->data.element.attributes.ptr != 0) {
    return new FastXDMAttributeAxis(info, document_, node_, 0);
  }
  return 0;
}

Result FastXDMNodeImpl::dmNamespaceNodes(const DynamicContext* context, const LocationInfo *info) const
{
  if(node_->nodeKind == FastXDMDocument::ELEMENT) {
    return new FastXDMNamespaceAxis(info, this, 0);
  }
  return 0;
}

Result FastXDMNodeImpl::dmChildren(const DynamicContext *context, const LocationInfo *info) const
{
  if(node_->nodeKind == FastXDMDocument::ELEMENT || node_->nodeKind == FastXDMDocument::DOCUMENT) {
    return new FastXDMChildAxis(info, document_, node_, 0);
  }
  return 0;
}

Result FastXDMNodeImpl::getAxisResult(XQStep::Axis axis, const NodeTest *nodeTest, const DynamicContext *context, const LocationInfo *info) const
{
  switch(axis) {
  case XQStep::ANCESTOR: {
    return new FastXDMAncestorAxis(info, document_, node_, nodeTest);
  }
  case XQStep::ANCESTOR_OR_SELF: {
    return new FastXDMAncestorOrSelfAxis(info, document_, node_, nodeTest);
  }
  case XQStep::ATTRIBUTE: {
    if(node_->nodeKind == FastXDMDocument::ELEMENT && node_->data.element.attributes.ptr != 0) {
      return new FastXDMAttributeAxis(info, document_, node_, nodeTest);
    }
    break;
  }
  case XQStep::CHILD: {
    if(node_->nodeKind == FastXDMDocument::ELEMENT || node_->nodeKind == FastXDMDocument::DOCUMENT) {
      return new FastXDMChildAxis(info, document_, node_, nodeTest);
    }
    break;
  }
  case XQStep::DESCENDANT: {
    if(node_->nodeKind == FastXDMDocument::ELEMENT || node_->nodeKind == FastXDMDocument::DOCUMENT) {
      return new FastXDMDescendantAxis(info, document_, node_, nodeTest);
    }
    break;
  }
  case XQStep::DESCENDANT_OR_SELF: {
    return new FastXDMDescendantOrSelfAxis(info, document_, node_, nodeTest);
  }
  case XQStep::FOLLOWING: {
    return new FastXDMFollowingAxis(info, document_, node_, nodeTest);
  }
  case XQStep::FOLLOWING_SIBLING: {
    return new FastXDMFollowingSiblingAxis(info, document_, node_, nodeTest);
  }
  case XQStep::NAMESPACE: {
    if(node_->nodeKind == FastXDMDocument::ELEMENT) {
      return new FastXDMNamespaceAxis(info, this, nodeTest);
    }
    break;
  }
  case XQStep::PARENT: {
    return new FastXDMParentAxis(info, document_, node_, nodeTest);
  }
  case XQStep::PRECEDING: {
    return new FastXDMPrecedingAxis(info, document_, node_, nodeTest);
  }
  case XQStep::PRECEDING_SIBLING: {
    return new FastXDMPrecedingSiblingAxis(info, document_, node_, nodeTest);
  }
  case XQStep::SELF: {
    return new FastXDMSelfAxis(info, document_, node_, nodeTest);
  }
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

const XMLCh FastXDMAttributeNodeImpl::fastxdm_attr_string[] = { 'f', 'a', 's', 't', 'x', 'd', 'm', '_', 'a', 't', 't', 'r', 0 };

FastXDMAttributeNodeImpl::FastXDMAttributeNodeImpl(const FastXDMDocument::Ptr &document, const FastXDMDocument::Attribute *attr)
  : document_(document),
    attr_(attr)
{  
}

FastXDMAttributeNodeImpl::~FastXDMAttributeNodeImpl()
{
}

void *FastXDMAttributeNodeImpl::getInterface(const XMLCh *name) const
{
  if(name == fastxdm_attr_string)
    return (void *)this;
  return 0;
}

const XMLCh* FastXDMAttributeNodeImpl::asString(const DynamicContext* context) const
{
  XMLBuffer buffer(1023, context->getMemoryManager());
  buffer.append(X("{"));
  buffer.append(attr_->uri);
  buffer.append(X("}"));
  buffer.append(attr_->localname);
  buffer.append(X("=\""));
  buffer.append(attr_->value);
  buffer.append(X("\""));
  return XMLString::replicate(buffer.getRawBuffer(), context->getMemoryManager());
}

void FastXDMAttributeNodeImpl::generateEvents(EventHandler *events, const DynamicContext *context,
                                              bool preserveNS, bool preserveType) const
{
  FastXDMDocument::toEvents(attr_, events, preserveType);
}

bool FastXDMAttributeNodeImpl::hasInstanceOfType(const XMLCh* typeURI, const XMLCh* typeName, const DynamicContext* context) const
{
    const XMLCh* uri, *name;
    getTypeUriAndName(uri, name);
    return context->isTypeOrDerivedFromType(uri, name, typeURI, typeName);
}


Sequence FastXDMAttributeNodeImpl::dmBaseURI(const DynamicContext* context) const
{
  Node::Ptr parent = dmParent(context);
  if(parent.notNull()) {
    return parent->dmBaseURI(context);
  }
  return Sequence(context->getMemoryManager());
}

const XMLCh* FastXDMAttributeNodeImpl::dmNodeKind(void) const
{
  return attribute_string;
}


ATQNameOrDerived::Ptr FastXDMAttributeNodeImpl::dmNodeName(const DynamicContext* context) const
{
  return context->getItemFactory()->createQName(attr_->uri, attr_->prefix, attr_->localname, context);
}

const XMLCh* FastXDMAttributeNodeImpl::dmStringValue(const DynamicContext* context) const
{
  return attr_->value;
}

Sequence FastXDMAttributeNodeImpl::dmTypedValue(DynamicContext* context) const
{
  // Data Model, § 7.3.3 and 7.3.4
  const XMLCh* typeUri, *typeName;
  getTypeUriAndName(typeUri, typeName);

  // If the attribute is of type xdt:untypedAtomic: its typed-value is its dm:string-value as an xdt:untypedAtomic
  if(XPath2Utils::equals(typeName, ATUntypedAtomic::fgDT_UNTYPEDATOMIC) && XPath2Utils::equals(typeUri, SchemaSymbols::fgURI_SCHEMAFORSCHEMA)) {
    return Sequence(context->getItemFactory()->createUntypedAtomic(attr_->value, context), context->getMemoryManager()); 
  } 

  FastXDMNSResolverImpl newNSScope(dmParent(context), context);
  AutoNsScopeReset jan(context, &newNSScope);

  // Otherwise: its typed-value is a sequence of zero or more atomic values derived from the string-value of 
  // the node and its type in a way that is consistent with XML Schema validation. The type of each atomic value 
  // is assigned as described in 3.3.1.2 Atomic Value Type Names
  DatatypeValidator *dtv = context->getDocumentCache()->getDatatypeValidator(typeUri, typeName);
  assert(dtv);

  if(dtv->getType() == DatatypeValidator::List)
    return getListTypeTypedValue(attr_->value, dtv, context);

  // TBD avoid double lookup of DatatypeValidator - jpcs
  return Sequence(context->getItemFactory()->createDerivedFromAtomicType(typeUri, typeName, attr_->value, context),
                  context->getMemoryManager());
}

Sequence FastXDMAttributeNodeImpl::dmDocumentURI(const DynamicContext* context) const
{
  return Sequence(context->getMemoryManager());
}

ATQNameOrDerived::Ptr FastXDMAttributeNodeImpl::dmTypeName(const DynamicContext* context) const
{
  const XMLCh* typeUri, *typeName;
  getTypeUriAndName(typeUri, typeName);
  return context->getItemFactory()->createQName(typeUri, XMLUni::fgZeroLenString, typeName, context);
}

ATBooleanOrDerived::Ptr FastXDMAttributeNodeImpl::dmNilled(const DynamicContext* context) const
{
  return 0;
}

bool FastXDMAttributeNodeImpl::lessThan(const Node::Ptr &other, const DynamicContext *context) const
{
  const FastXDMAttributeNodeImpl *otherImpl = (const FastXDMAttributeNodeImpl*)other->getInterface(fastxdm_attr_string);
  if(otherImpl != 0) {
    // Order first by the address of the FastXDMDocument object
    if(document_.get() != otherImpl->getDocument().get())
      return document_.get() < otherImpl->getDocument().get();

    // Then order by the node index
    return attr_ < otherImpl->getAttribute();
  }

  const FastXDMNodeImpl *nodeImpl = (const FastXDMNodeImpl*)other->getInterface(FastXDMNodeImpl::fastxdm_string);
  if(nodeImpl != 0) {
    // Order first by the address of the FastXDMDocument object
    if(document_.get() != nodeImpl->getDocument().get())
      return document_.get() < nodeImpl->getDocument().get();

    // Then order by the node index
    if(attr_->owner.ptr == nodeImpl->getNode()) return false;
    return attr_->owner.ptr < nodeImpl->getNode();
  }

  const FastXDMNamespaceNodeImpl *nsImpl = (const FastXDMNamespaceNodeImpl*)other->
    getInterface(FastXDMNamespaceNodeImpl::fastxdm_ns_string);
  if(nsImpl != 0) {
    if(nsImpl->getOwner().isNull()) return false;

    // Order first by the address of the FastXDMDocument object
    if(document_.get() != nsImpl->getOwner()->getDocument().get())
      return document_.get() < nsImpl->getOwner()->getDocument().get();

    // Then order by the node index
    if(attr_->owner.ptr == nsImpl->getOwner()->getNode()) return true;
    return attr_->owner.ptr < nsImpl->getOwner()->getNode();
  }

  // It's not a Fast XDM implementation Node, so it can't
  // be from the same tree as us - jpcs

  // TBD find a better way to order these - jpcs

  // Order them according to the address of their roots
  return this->root(context).get() < other->root(context).get();
}

bool FastXDMAttributeNodeImpl::equals(const Node::Ptr &other) const
{
  const FastXDMAttributeNodeImpl *otherImpl = (const FastXDMAttributeNodeImpl*)other->getInterface(fastxdm_attr_string);
  if(otherImpl == 0) return false;

  if(document_.get() != otherImpl->getDocument().get())
    return false;

  return attr_ == otherImpl->getAttribute();
}

bool FastXDMAttributeNodeImpl::uniqueLessThan(const Node::Ptr &other, const DynamicContext *context) const
{
  return lessThan(other, context);
}

ATBooleanOrDerived::Ptr FastXDMAttributeNodeImpl::dmIsId(const DynamicContext* context) const
{
  if(hasInstanceOfType(SchemaSymbols::fgURI_SCHEMAFORSCHEMA, XMLUni::fgIDString, context))
    return context->getItemFactory()->createBoolean(true, context);
  return context->getItemFactory()->createBoolean(false, context);
}

ATBooleanOrDerived::Ptr FastXDMAttributeNodeImpl::dmIsIdRefs(const DynamicContext* context) const
{
  if(hasInstanceOfType(SchemaSymbols::fgURI_SCHEMAFORSCHEMA, XMLUni::fgIDRefString, context) ||
     hasInstanceOfType(SchemaSymbols::fgURI_SCHEMAFORSCHEMA, XMLUni::fgIDRefsString, context))
    return context->getItemFactory()->createBoolean(true, context);
  return context->getItemFactory()->createBoolean(false, context);
}

void FastXDMAttributeNodeImpl::getTypeUriAndName(const XMLCh*& uri, const XMLCh*& name) const
{
  uri = attr_->typeURI;
  name = attr_->typeName;
}

const XMLCh* FastXDMAttributeNodeImpl::getTypeName() const
{
  const XMLCh* uri, *name;
  getTypeUriAndName(uri,name);
  return name;
}

const XMLCh* FastXDMAttributeNodeImpl::getTypeURI() const
{
  const XMLCh* uri, *name;
  getTypeUriAndName(uri,name);
  return uri;
}

class FastXDMAttributeAncestorOrSelfAxis : public ResultImpl
{
public:
  FastXDMAttributeAncestorOrSelfAxis(const LocationInfo *info, const FastXDMDocument::Ptr &document, const FastXDMDocument::Attribute *attr, const NodeTest *nodeTest)
    : ResultImpl(info),
      document_(document),
      attr_(attr),
      node_(attr->owner.ptr),
      nodeTest_(nodeTest)
  {
  }

  Item::Ptr next(DynamicContext *context)
  {
    if(attr_ != 0) {
      Item::Ptr result = testAttribute(document_, attr_, nodeTest_, context);
      attr_ = 0;

      if(result.notNull()) return result;
    }

    while(node_ != 0) {
      Item::Ptr result = testNode(document_, node_, nodeTest_, context);
      node_ = FastXDMDocument::getParent(node_);

      if(result.notNull()) return result;
    }

    return 0;
  }

  virtual std::string asString(DynamicContext *context, int indent) const { return ""; }

protected:
  FastXDMDocument::Ptr document_;
  const FastXDMDocument::Attribute *attr_;
  const FastXDMDocument::Node *node_;
  const NodeTest *nodeTest_;
};

Node::Ptr FastXDMAttributeNodeImpl::root(const DynamicContext* context) const
{
  if(document_->getNumNodes() == 0) return this;
  return new FastXDMNodeImpl(document_, document_->getNode(0));
}

Node::Ptr FastXDMAttributeNodeImpl::dmParent(const DynamicContext* context) const
{
  if(attr_->owner.ptr == 0) return 0;
  return new FastXDMNodeImpl(document_, attr_->owner.ptr);
}

Result FastXDMAttributeNodeImpl::dmAttributes(const DynamicContext* context, const LocationInfo *info) const
{
  return 0;
}

Result FastXDMAttributeNodeImpl::dmNamespaceNodes(const DynamicContext* context, const LocationInfo *info) const
{
  return 0;
}

Result FastXDMAttributeNodeImpl::dmChildren(const DynamicContext *context, const LocationInfo *info) const
{
  return 0;
}

Result FastXDMAttributeNodeImpl::getAxisResult(XQStep::Axis axis, const NodeTest *nodeTest, const DynamicContext *context, const LocationInfo *info) const
{
  switch(axis) {
  case XQStep::ANCESTOR: {
    if(attr_->owner.ptr == 0) return 0;
    return new FastXDMAncestorOrSelfAxis(info, document_, attr_->owner.ptr, nodeTest);
  }
  case XQStep::ANCESTOR_OR_SELF: {
    return new FastXDMAttributeAncestorOrSelfAxis(info, document_, attr_, nodeTest);
  }
  case XQStep::FOLLOWING: {
    return new FastXDMFollowingAxis(info, document_, attr_, nodeTest);
  }
  case XQStep::PARENT: {
    if(attr_->owner.ptr == 0) return 0;
    return new FastXDMSelfAxis(info, document_, attr_->owner.ptr, nodeTest);
  }
  case XQStep::PRECEDING: {
    if(attr_->owner.ptr == 0) return 0;
    return new FastXDMPrecedingAxis(info, document_, attr_->owner.ptr, nodeTest);
  }
  case XQStep::DESCENDANT_OR_SELF:
  case XQStep::SELF: {
    return nodeTest->filterResult((Item::Ptr)this, info);
  }
  case XQStep::ATTRIBUTE:
  case XQStep::CHILD:
  case XQStep::DESCENDANT:
  case XQStep::FOLLOWING_SIBLING:
  case XQStep::NAMESPACE:
  case XQStep::PRECEDING_SIBLING:
    break;
  }

  return 0;
}


////////////////////////////////////////////////////////////////////////////////////////////////////

const XMLCh FastXDMNamespaceNodeImpl::fastxdm_ns_string[] = { 'f', 'a', 's', 't', 'x', 'd', 'm', '_', 'n', 's', 0 };

FastXDMNamespaceNodeImpl::FastXDMNamespaceNodeImpl(const FastXDMNodeImpl::Ptr &owner, const XMLCh *prefix, const XMLCh *uri,
                                                   unsigned int order)
  : owner_(owner),
    prefix_(prefix),
    uri_(uri),
    order_(order)
{  
}

FastXDMNamespaceNodeImpl::~FastXDMNamespaceNodeImpl()
{
}

void *FastXDMNamespaceNodeImpl::getInterface(const XMLCh *name) const
{
  if(name == fastxdm_ns_string)
    return (void *)this;
  return 0;
}

const XMLCh* FastXDMNamespaceNodeImpl::asString(const DynamicContext* context) const
{
  XMLBuffer buffer(1023, context->getMemoryManager());
  buffer.append(X("["));
  buffer.append(prefix_);
  buffer.append(X("=\""));
  buffer.append(uri_);
  buffer.append(X("\"]"));
  return XMLString::replicate(buffer.getRawBuffer(), context->getMemoryManager());
}

void FastXDMNamespaceNodeImpl::generateEvents(EventHandler *events, const DynamicContext *context,
                                              bool preserveNS, bool preserveType) const
{
  events->namespaceEvent(prefix_, uri_);
}

bool FastXDMNamespaceNodeImpl::hasInstanceOfType(const XMLCh* typeURI, const XMLCh* typeName, const DynamicContext* context) const
{
  XQThrow2(ItemException, X("NodeImpl::getTypeUriAndName"), X("Tried to get type informations on Node other than DOMElement, DOMAttribute or DOMText"));
}

Sequence FastXDMNamespaceNodeImpl::dmBaseURI(const DynamicContext* context) const
{
  return Sequence(context->getMemoryManager());
}

const XMLCh* FastXDMNamespaceNodeImpl::dmNodeKind(void) const
{
  return namespace_string;
}


ATQNameOrDerived::Ptr FastXDMNamespaceNodeImpl::dmNodeName(const DynamicContext* context) const
{
  if(prefix_ == 0) return 0;
  return context->getItemFactory()->createQName(XMLUni::fgZeroLenString, XMLUni::fgZeroLenString, prefix_, context);
}

const XMLCh* FastXDMNamespaceNodeImpl::dmStringValue(const DynamicContext* context) const
{
  return uri_;
}

Sequence FastXDMNamespaceNodeImpl::dmTypedValue(DynamicContext* context) const
{
  return Sequence(context->getItemFactory()->createString(uri_, context), context->getMemoryManager());
}

Sequence FastXDMNamespaceNodeImpl::dmDocumentURI(const DynamicContext* context) const
{
  return Sequence(context->getMemoryManager());
}

ATQNameOrDerived::Ptr FastXDMNamespaceNodeImpl::dmTypeName(const DynamicContext* context) const
{
  return 0;
}

ATBooleanOrDerived::Ptr FastXDMNamespaceNodeImpl::dmNilled(const DynamicContext* context) const
{
  return 0;
}

bool FastXDMNamespaceNodeImpl::lessThan(const Node::Ptr &other, const DynamicContext *context) const
{
  const FastXDMNamespaceNodeImpl *otherImpl = (const FastXDMNamespaceNodeImpl*)other->getInterface(fastxdm_ns_string);
  if(otherImpl != 0) {
    // Check if out owner is null
    if(owner_.isNull()) {
      if(otherImpl->getOwner().notNull()) return true;

      return this < otherImpl;
    }
    if(otherImpl->getOwner().isNull()) {
      return false;
    }

    // Order first by the owner object
    if(!owner_->equals(otherImpl->getOwner()))
      return owner_->lessThan(otherImpl->getOwner(), context);

    // Then order by the node index
    return order_ < otherImpl->getOrder();
  }

  const FastXDMNodeImpl *nodeImpl = (const FastXDMNodeImpl*)other->getInterface(FastXDMNodeImpl::fastxdm_string);
  if(nodeImpl != 0) {
    // Check if out owner is null
    if(owner_.isNull()) return true;

    // Order first by the address of the FastXDMDocument object
    if(owner_->getDocument().get() != nodeImpl->getDocument().get())
      return owner_->getDocument().get() < nodeImpl->getDocument().get();

    // Then order by the node index
    if(owner_->getNode() == nodeImpl->getNode()) return false;
    return owner_->getNode() < nodeImpl->getNode();
  }

  const FastXDMAttributeNodeImpl *attrImpl = (const FastXDMAttributeNodeImpl*)other->
    getInterface(FastXDMAttributeNodeImpl::fastxdm_attr_string);
  if(attrImpl != 0) {
    // Check if out owner is null
    if(owner_.isNull()) return true;

    // Order first by the address of the FastXDMDocument object
    if(owner_->getDocument().get() != attrImpl->getDocument().get())
      return owner_->getDocument().get() < attrImpl->getDocument().get();

    // Then order by the node index
    if(owner_->getNode() == attrImpl->getAttribute()->owner.ptr) return false;
    return owner_->getNode() < attrImpl->getAttribute()->owner.ptr;
  }

  // It's not a Fast XDM implementation Node, so it can't
  // be from the same tree as us - jpcs

  // TBD find a better way to order these - jpcs

  // Order them according to the address of their roots
  return this->root(context).get() < other->root(context).get();
}

bool FastXDMNamespaceNodeImpl::equals(const Node::Ptr &other) const
{
  if(owner_.isNull()) return false;

  const FastXDMNamespaceNodeImpl *otherImpl = (const FastXDMNamespaceNodeImpl*)other->getInterface(fastxdm_ns_string);
  if(otherImpl == 0) return false;

  if(otherImpl->getOwner().isNull()) return false;
  if(!owner_->equals(otherImpl->getOwner())) return false;

  return order_ == otherImpl->getOrder();
}

bool FastXDMNamespaceNodeImpl::uniqueLessThan(const Node::Ptr &other, const DynamicContext *context) const
{
  return lessThan(other, context);
}

ATBooleanOrDerived::Ptr FastXDMNamespaceNodeImpl::dmIsId(const DynamicContext* context) const
{
  return 0;
}

ATBooleanOrDerived::Ptr FastXDMNamespaceNodeImpl::dmIsIdRefs(const DynamicContext* context) const
{
  return 0;
}

const XMLCh* FastXDMNamespaceNodeImpl::getTypeName() const
{
  XQThrow2(ItemException, X("NodeImpl::getTypeUriAndName"), X("Tried to get type informations on Node other than DOMElement, DOMAttribute or DOMText"));
}

const XMLCh* FastXDMNamespaceNodeImpl::getTypeURI() const
{
  XQThrow2(ItemException, X("NodeImpl::getTypeUriAndName"), X("Tried to get type informations on Node other than DOMElement, DOMAttribute or DOMText"));
}

class FastXDMNamespaceAncestorOrSelfAxis : public ResultImpl
{
public:
  FastXDMNamespaceAncestorOrSelfAxis(const LocationInfo *info, const FastXDMNamespaceNodeImpl::Ptr &ns, const NodeTest *nodeTest)
    : ResultImpl(info),
      ns_(ns),
      document_(ns->getOwner()->getDocument()),
      node_(ns->getOwner()->getNode()),
      nodeTest_(nodeTest)
  {
  }

  Item::Ptr next(DynamicContext *context)
  {
    if(ns_.notNull()) {
      if(testNamespace(ns_, nodeTest_, context)) {
        Item::Ptr result = ns_;
        ns_ = 0;
        return result;
      }
      ns_ = 0;
    }

    while(node_ != 0) {
      Item::Ptr result = testNode(document_, node_, nodeTest_, context);
      node_ = FastXDMDocument::getParent(node_);

      if(result.notNull()) return result;
    }

    return 0;
  }

  virtual std::string asString(DynamicContext *context, int indent) const { return ""; }

protected:
  FastXDMNamespaceNodeImpl::Ptr ns_;
  FastXDMDocument::Ptr document_;
  const FastXDMDocument::Node *node_;
  const NodeTest *nodeTest_;
};

Node::Ptr FastXDMNamespaceNodeImpl::root(const DynamicContext* context) const
{
  if(owner_.isNull()) return this;
  return owner_->root(context);
}

Node::Ptr FastXDMNamespaceNodeImpl::dmParent(const DynamicContext* context) const
{
  return owner_;
}

Result FastXDMNamespaceNodeImpl::dmAttributes(const DynamicContext* context, const LocationInfo *info) const
{
  return 0;
}

Result FastXDMNamespaceNodeImpl::dmNamespaceNodes(const DynamicContext* context, const LocationInfo *info) const
{
  return 0;
}

Result FastXDMNamespaceNodeImpl::dmChildren(const DynamicContext *context, const LocationInfo *info) const
{
  return 0;
}

Result FastXDMNamespaceNodeImpl::getAxisResult(XQStep::Axis axis, const NodeTest *nodeTest, const DynamicContext *context, const LocationInfo *info) const
{
  switch(axis) {
  case XQStep::ANCESTOR: {
    if(owner_.notNull())
      return new FastXDMAncestorOrSelfAxis(info, owner_->getDocument(), owner_->getNode(), nodeTest);
    break;
  }
  case XQStep::ANCESTOR_OR_SELF: {
    if(owner_.notNull())
      return new FastXDMNamespaceAncestorOrSelfAxis(info, this, nodeTest);
    else 
      return nodeTest->filterResult((Item::Ptr)this, info);
  }
  case XQStep::FOLLOWING: {
    if(owner_.notNull())
      return new FastXDMFollowingAxis(info, owner_->getDocument(), this, nodeTest);
    break;
  }
  case XQStep::PARENT: {
    if(owner_.notNull())
      return nodeTest->filterResult((Item::Ptr)owner_, info);
    break;
  }
  case XQStep::PRECEDING: {
    if(owner_.notNull())
      return new FastXDMPrecedingAxis(info, owner_->getDocument(), owner_->getNode(), nodeTest);
    break;
  }
  case XQStep::DESCENDANT_OR_SELF:
  case XQStep::SELF: {
    return nodeTest->filterResult((Item::Ptr)this, info);
  }
  case XQStep::ATTRIBUTE:
  case XQStep::CHILD:
  case XQStep::DESCENDANT:
  case XQStep::FOLLOWING_SIBLING:
  case XQStep::NAMESPACE:
  case XQStep::PRECEDING_SIBLING:
    break;
  }

  return 0;
}

