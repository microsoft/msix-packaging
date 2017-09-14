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

#include <xqilla/framework/XQillaExport.hpp>
#include <xqilla/ast/XQElementConstructor.hpp>
#include <xqilla/ast/StaticAnalysis.hpp>
#include <xqilla/ast/XQSequence.hpp>
#include <xqilla/ast/XQLiteral.hpp>
#include <xqilla/ast/XQNamespaceBinding.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/context/ItemFactory.hpp>
#include <xqilla/context/ContextHelpers.hpp>
#include <xqilla/dom-api/impl/XQillaNSResolverImpl.hpp>
#include <xqilla/exceptions/ASTException.hpp>
#include <xqilla/exceptions/NamespaceLookupException.hpp>
#include <xqilla/exceptions/StaticErrorException.hpp>
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/utils/XPath2NSUtils.hpp>
#include <xqilla/items/Node.hpp>
#include <xqilla/ast/XQAtomize.hpp>
#include <xqilla/parser/QName.hpp>
#include <xqilla/events/EventHandler.hpp>
#include <xqilla/schema/DocumentCache.hpp>
#include <xqilla/utils/XMLChCompare.hpp>
#include <xqilla/exceptions/StaticErrorException.hpp>

#include "../events/NoInheritFilter.hpp"

#include <xercesc/validators/schema/SchemaSymbols.hpp>
#include <xercesc/framework/XMLBuffer.hpp>
#include <xercesc/util/XMLChar.hpp>
#include <assert.h>

#if defined(XERCES_HAS_CPP_NAMESPACE)
XERCES_CPP_NAMESPACE_USE
#endif

using namespace std;

XQElementConstructor::XQElementConstructor(ASTNode* name, VectorOfASTNodes* attrList, VectorOfASTNodes* children,
                                           XPath2MemoryManager* mm)
  : XQDOMConstructor(mm),
    namespaceExpr(0),
    m_name(name),
    m_attrList(attrList),
    m_children(children),
    m_namespaces(NULL)
{
}

XQElementConstructor::XQElementConstructor(ASTNode* name, VectorOfASTNodes* attrList, VectorOfASTNodes* children,
                                           RefHashTableOf< XMLCh > *namespaces, XPath2MemoryManager *mm)
  : XQDOMConstructor(mm),
    namespaceExpr(0),
    m_name(name),
    m_attrList(attrList),
    m_children(children),
    m_namespaces(namespaces)
{
}

EventGenerator::Ptr XQElementConstructor::generateEvents(EventHandler *events, DynamicContext *context,
                                                    bool preserveNS, bool preserveType) const
{
  NoInheritFilter niFilter(events, context->getMemoryManager());
  if(!context->getInheritNamespaces()) events = &niFilter;

  // Resolve the name
  AnyAtomicType::Ptr itemName = m_name->createResult(context)->next(context);
  const ATQNameOrDerived *pQName = (const ATQNameOrDerived*)itemName.get();
  const XMLCh *prefix = emptyToNull(pQName->getPrefix());
  const XMLCh *uri = emptyToNull(pQName->getURI());
  const XMLCh *localname = pQName->getName();

  events->startElementEvent(prefix, uri, localname);

  ElemConstructFilter elemFilter(events, this, context->getMemoryManager());

  if(m_namespaces != 0) {
    RefHashTableOfEnumerator<XMLCh> nsEnumVal(m_namespaces, false, context->getMemoryManager());
    RefHashTableOfEnumerator<XMLCh> nsEnumKey(m_namespaces, false, context->getMemoryManager());
    while(nsEnumVal.hasMoreElements()) {
      elemFilter.namespaceEvent(emptyToNull((XMLCh*)nsEnumKey.nextElementKey()), emptyToNull(&nsEnumVal.nextElement()));
    }
  }
  if(m_attrList != 0) {
    for(VectorOfASTNodes::const_iterator itAttr = m_attrList->begin(); itAttr != m_attrList->end (); ++itAttr) {
      (*itAttr)->generateAndTailCall(&elemFilter, context, preserveNS, preserveType);
    }
  }

  for(VectorOfASTNodes::const_iterator itCont = m_children->begin(); itCont != m_children->end (); ++itCont) {
    (*itCont)->generateAndTailCall(&elemFilter, context, preserveNS, preserveType);
  }

  const XMLCh *typeURI = SchemaSymbols::fgURI_SCHEMAFORSCHEMA;
  const XMLCh *typeName = DocumentCache::g_szUntyped;
  if(context->getConstructionMode() == StaticContext::CONSTRUCTION_MODE_PRESERVE) {
    typeURI = SchemaSymbols::fgURI_SCHEMAFORSCHEMA;
    typeName = SchemaSymbols::fgATTVAL_ANYTYPE;
  }

  events->endElementEvent(prefix, uri, localname, typeURI, typeName);
  return 0;
}

ASTNode* XQElementConstructor::staticResolution(StaticContext *context)
{
  XPath2MemoryManager *mm = context->getMemoryManager();

  // Add a new scope for the namespace definitions
  XQillaNSResolverImpl *newNSScope = new (mm) XQillaNSResolverImpl(mm, context->getNSResolver());
  AutoNsScopeReset jan(context, newNSScope);
  bool namespacesFound = false;
  unsigned int i;

  if(m_attrList != 0) {
    AutoDelete<DynamicContext> dContext(context->createDynamicContext());
    dContext->setMemoryManager(context->getMemoryManager());
    // process the namespace attributes (they are all at the beginning of the list)
    for (VectorOfASTNodes::iterator it=m_attrList->begin();it!=m_attrList->end();) 
    {
      ASTNode* astNode=*it;
      assert(astNode->getType()==ASTNode::DOM_CONSTRUCTOR && 
             ((XQDOMConstructor*)astNode)->getNodeType()==Node::attribute_string);
      XQDOMConstructor* attrConstructor=(XQDOMConstructor*)astNode;
      const ASTNode* dItem=attrConstructor->getName();
      assert(dItem!=NULL);

      if(dItem->getType() != ASTNode::DIRECT_NAME)
        // no more namespace declaration
        break;

      const XMLCh *qname = ((XQDirectName*)dItem)->getQName();

      const XMLCh* XMLNSPrefix=NULL;
      if(XPath2Utils::equals(qname,XMLUni::fgXMLNSString)) {
        XMLNSPrefix=XMLUni::fgZeroLenString;
      }
      else if(XPath2Utils::equals(XPath2NSUtils::getPrefix(qname, context->getMemoryManager()), XMLUni::fgXMLNSString)) {
        XMLNSPrefix=XPath2NSUtils::getLocalName(qname);
      }
      else
        // no more namespace declaration
        break;

      // we are a namespace attribute: check that we have a constant value
      namespacesFound = true;
      const VectorOfASTNodes *children=attrConstructor->getChildren();
      const XMLCh* uri=NULL;
      if(children->size()==0) { // supporting Namespace 1.1 means unsetting the binding...
        uri=XMLUni::fgZeroLenString;
      }
      else if(children->size()>1 || ((*children)[0]->getType()!=ASTNode::LITERAL &&
                                     (*children)[0]->getType()!=ASTNode::SEQUENCE)) {
        XQThrow(StaticErrorException,X("DOM Constructor"),X("The value of a namespace declaration attribute must "
                                                            "be a literal string [err:XQST0022]"));
      }
      else {
        Item::Ptr nsUri = (*children)[0]->createResult(dContext)->next(dContext);
        if(nsUri == NULLRCP)
          XQThrow(StaticErrorException,X("DOM Constructor"),X("The value of a namespace declaration attribute must "
                                                              "be a literal string [err:XQST0022]"));
        uri=nsUri->asString(dContext);
      }
      if(XMLNSPrefix != XMLUni::fgZeroLenString) {
        XQillaNSResolverImpl::forbiddenBindingCheck(XMLNSPrefix, uri, this);
        newNSScope->addNamespaceBinding(XMLNSPrefix, uri);
      }
      else {
        context->setDefaultElementAndTypeNS(uri);
        newNSScope->addNamespaceBinding(XMLUni::fgZeroLenString, uri);
      }
      if(m_namespaces==NULL)
        m_namespaces = new (mm) RefHashTableOf< XMLCh >(5, false, mm);
      
      if(m_namespaces->containsKey(XMLNSPrefix))
        XQThrow(StaticErrorException,X("DOM Constructor"),X("Two namespace declaration attributes specified by a direct ele"
                                                            "ment constructor do not have distinct names. [err:XQST0071]"));
      m_namespaces->put((void*)mm->getPooledString(XMLNSPrefix), (XMLCh*)mm->getPooledString(uri));
      it=m_attrList->erase(it);
    }

    // now run static resolution on the real attributes
    for (i=0;i<m_attrList->size();i++) {
      (*m_attrList)[i] = (*m_attrList)[i]->staticResolution(context);
    }
    // now that we have added the local namespace declaration, check for duplicate attribute names
    std::set<const XMLCh*, XMLChSort> attrNames;
    for (i=0;i<m_attrList->size();i++) 
    {
      ASTNode* astNode=(*m_attrList)[i];
      assert(astNode->getType()==ASTNode::DOM_CONSTRUCTOR && 
             ((XQDOMConstructor*)astNode)->getNodeType()==Node::attribute_string);
      XQDOMConstructor* attrConstructor=(XQDOMConstructor*)astNode;
      const ASTNode* dItem=attrConstructor->getName();
      if(dItem->getType() == ASTNode::NAME_EXPRESSION) {
        dItem = ((XQNameExpression*)dItem)->getExpression();
      }
      if(dItem->getType() == ASTNode::TREAT_AS) {
        dItem = ((XQAtomize*)dItem)->getExpression();
      }
      if(dItem->getType() == ASTNode::ATOMIZE) {
        dItem = ((XQAtomize*)dItem)->getExpression();
      }
      assert(dItem->getType()==ASTNode::QNAME_LITERAL); 
      Item::Ptr item = dItem->createResult(dContext)->next(dContext);
      QualifiedName attrName(item->asString(dContext));
      XMLBuffer buff(200, dContext->getMemoryManager());
      if(attrName.getPrefix()!=0 && *attrName.getPrefix()!=0)
      {
        buff.append(chOpenCurly);
        buff.append(dContext->getUriBoundToPrefix(attrName.getPrefix(), this));
        buff.append(chCloseCurly);
      }
      buff.append(attrName.getName());
      if(attrNames.find(buff.getRawBuffer())!=attrNames.end())
        XQThrow(StaticErrorException,X("DOM Constructor"),X("Two attributes specified by a direct element constructor do not have distinct expanded QNames. [err:XQST0040]"));
      attrNames.insert(XMLString::replicate(buff.getRawBuffer(), dContext->getMemoryManager()));
    }
  }

  // and run static resolution
  m_name = new (mm) XQNameExpression(m_name, mm);
  m_name->setLocationInfo(this);
  m_name = m_name->staticResolution(context);

  for(i = 0; i < m_children->size(); ++i) {
    (*m_children)[i] = new (mm) XQContentSequence((*m_children)[i], mm);
    (*m_children)[i]->setLocationInfo(this);

    (*m_children)[i] = (*m_children)[i]->staticResolution(context);
  }

  if(namespacesFound) {
    // Add an XQNamespaceBinding object to sort out our namespaces
    XQNamespaceBinding *result = new (mm) XQNamespaceBinding(newNSScope, this, mm);
    result->setLocationInfo(this);
    return result;
  }
  return this;
}

ASTNode *XQElementConstructor::staticTypingImpl(StaticContext *context)
{
  _src.clear();

  unsigned int i;
  if(m_attrList != 0) {
    for(i = 0; i < m_attrList->size(); ++i) {
      _src.add((*m_attrList)[i]->getStaticAnalysis()); 
    }
  }

  _src.add(m_name->getStaticAnalysis());

  if(m_name->getStaticAnalysis().isUpdating()) {
    XQThrow(StaticErrorException,X("XQElementConstructor::staticTyping"),
            X("It is a static error for the name expression of an element constructor "
              "to be an updating expression [err:XUST0001]"));
  }

  for(i = 0; i < m_children->size(); ++i) {
    _src.add((*m_children)[i]->getStaticAnalysis());

    if((*m_children)[i]->getStaticAnalysis().isUpdating()) {
      XQThrow(StaticErrorException,X("XQElementConstructor::staticTyping"),
              X("It is a static error for a content expression of an element constructor "
                "to be an updating expression [err:XUST0001]"));
    }
  }

  _src.getStaticType() = StaticType::ELEMENT_TYPE;
  _src.creative(true);
  _src.setProperties(StaticAnalysis::DOCORDER | StaticAnalysis::GROUPED |
                     StaticAnalysis::PEER | StaticAnalysis::SUBTREE | StaticAnalysis::SAMEDOC |
                     StaticAnalysis::ONENODE);
  return this;
}

const XMLCh* XQElementConstructor::getNodeType() const
{
  return Node::element_string;
}

ASTNode *XQElementConstructor::getName() const
{
  return m_name;
}

const VectorOfASTNodes *XQElementConstructor::getAttributes() const
{
  return m_attrList;
}

const VectorOfASTNodes *XQElementConstructor::getChildren() const
{
  return m_children;
}

void XQElementConstructor::setName(ASTNode *name)
{
  m_name = name;
}

//////////////////////////////////////////////////////////////////////

ElemConstructFilter::ElemConstructFilter(EventHandler *next, const LocationInfo *location, XPath2MemoryManager *mm)
    : EventFilter(next),
      mm_(mm),
      location_(location),
      level_(0),
      seenContent_(false)
{
}

void ElemConstructFilter::startElementEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname)
{
  seenContent_ = true;
  ++level_;
  next_->startElementEvent(prefix, uri, localname);
}

void ElemConstructFilter::endElementEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname,
                                          const XMLCh *typeURI, const XMLCh *typeName)
{
  next_->endElementEvent(prefix, uri, localname, typeURI, typeName);
  --level_;
}

void ElemConstructFilter::piEvent(const XMLCh *target, const XMLCh *value)
{
  seenContent_ = true;
  next_->piEvent(target, value);
}

void ElemConstructFilter::textEvent(const XMLCh *value)
{
  seenContent_ = true;
  next_->textEvent(value);
}

void ElemConstructFilter::textEvent(const XMLCh *chars, unsigned int length)
{
  seenContent_ = true;
  next_->textEvent(chars, length);
}

void ElemConstructFilter::commentEvent(const XMLCh *value)
{
  seenContent_ = true;
  next_->commentEvent(value);
}

void ElemConstructFilter::attributeEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname, const XMLCh *value,
                                         const XMLCh *typeURI, const XMLCh *typeName)
{
  if(level_ == 0) {
    if(seenContent_)
      XQThrow3(ASTException,X("ElemConstructFilter::attributeEvent"),
               X("Attribute nodes must be created before the other content of an element [err:XQTY0024]"), location_);

    if(!attrs_.insert(AttrRecord(uri, localname, mm_)).second)
      XQThrow3(ASTException,X("ElemConstructFilter::attributeEvent"),
               X("An element has two attributes with the same expanded name [err:XQDY0025]"), location_);
  }

  next_->attributeEvent(prefix, uri, localname, value, typeURI, typeName);
}

void ElemConstructFilter::namespaceEvent(const XMLCh *prefix, const XMLCh *uri)
{
  if(level_ == 0) {
    if(seenContent_)
      XQThrow3(ASTException,X("ElemConstructFilter::namespaceEvent"),
               X("Namespace nodes must be created before the other content of an element [err:XQTY0024]"), location_);
    if(!attrs_.insert(AttrRecord(prefix, 0, mm_)).second)
      XQThrow3(ASTException,X("ElemConstructFilter::namespaceEvent"),
               X("An element has two namespaces for the same prefix [err:XQDY0025]"), location_);
  }

  next_->namespaceEvent(prefix, uri);
}

ElemConstructFilter::AttrRecord::AttrRecord(const XMLCh *u, const XMLCh *n, XPath2MemoryManager *mm)
  : uri(mm->getPooledString(u)),
    name(mm->getPooledString(n))
{
}

bool ElemConstructFilter::AttrRecord::operator<(const AttrRecord &o) const
{
  int cmp = XMLString::compareString(name, o.name);
  if(cmp < 0) return true;
  if(cmp > 0) return false;
  return XMLString::compareString(uri, o.uri) < 0;
}
