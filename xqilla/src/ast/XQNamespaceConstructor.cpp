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
#include <xqilla/ast/XQNamespaceConstructor.hpp>
#include <xqilla/ast/StaticAnalysis.hpp>
#include <xqilla/ast/XQLiteral.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/context/ItemFactory.hpp>
#include <xqilla/exceptions/ASTException.hpp>
#include <xqilla/exceptions/NamespaceLookupException.hpp>
#include <xqilla/exceptions/StaticErrorException.hpp>
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/utils/XPath2NSUtils.hpp>
#include <xqilla/items/Node.hpp>
#include <xqilla/ast/XQAtomize.hpp>
#include <xqilla/events/EventHandler.hpp>
#include <xqilla/exceptions/StaticErrorException.hpp>

#include <xercesc/validators/schema/SchemaSymbols.hpp>
#include <xercesc/framework/XMLBuffer.hpp>
#include <xercesc/util/XMLChar.hpp>

#if defined(XERCES_HAS_CPP_NAMESPACE)
XERCES_CPP_NAMESPACE_USE
#endif

XQNamespaceConstructor::XQNamespaceConstructor(ASTNode* name, VectorOfASTNodes* children, XPath2MemoryManager* mm)
  : XQDOMConstructor(mm),
    m_name(name),
    m_children(children)
{
}

EventGenerator::Ptr XQNamespaceConstructor::generateEvents(EventHandler *events, DynamicContext *context,
                                                      bool preserveNS, bool preserveType) const
{
  const XMLCh *nodeName = m_name->createResult(context)->next(context)->asString(context);
  if(*nodeName && !XMLChar1_0::isValidNCName(nodeName, XMLString::stringLen(nodeName)))
    XQThrow(ASTException,X("XQNamespaceConstructor::generateEvents"),
            X("The name for the namespace node must be either a zero-length string or a valid xs:NCName [err:XTDE0920]"));

  if(XPath2Utils::equals(nodeName, XMLUni::fgXMLNSString))
    XQThrow(ASTException,X("XQNamespaceConstructor::generateEvents"),
            X("The name for the namespace node must not be \"xmlns\" [err:XTDE0920]"));

  XMLBuffer value;
  getStringValue(m_children, value, context);

  if(value.getLen() == 0)
    XQThrow(ASTException,X("XQNamespaceConstructor::generateEvents"),
            X("The value for the namespace node must not be empty [err:XTDE0930]"));

  if(XPath2Utils::equals(nodeName, XMLUni::fgXMLString) &&
     !XPath2Utils::equals(value.getRawBuffer(), XMLUni::fgXMLURIName))
    XQThrow(ASTException,X("XQNamespaceConstructor::generateEvents"),
            X("The name for the namespace node must not be \"xml\"  when the value is not \"http://www.w3.org/XML/1998/namespace\" [err:XTDE0925]"));

  if(XPath2Utils::equals(value.getRawBuffer(), XMLUni::fgXMLURIName) &&
     !XPath2Utils::equals(nodeName, XMLUni::fgXMLString))
    XQThrow(ASTException,X("XQNamespaceConstructor::generateEvents"),
            X("The value for the namespace node must not be \"http://www.w3.org/XML/1998/namespace\"  when the name is not \"xml\" [err:XTDE0925]"));

  events->namespaceEvent(nodeName, value.getRawBuffer());
  return 0;
}

ASTNode* XQNamespaceConstructor::staticResolution(StaticContext *context)
{
  XPath2MemoryManager *mm = context->getMemoryManager();

  // and run static resolution
  m_name = new (mm) XQAtomize(m_name, mm);
  m_name->setLocationInfo(this);
  m_name = m_name->staticResolution(context);

  unsigned int i;
  for(i = 0;i < m_children->size(); ++i) {
    // atomize content and run static resolution 
    (*m_children)[i] = new (mm) XQAtomize((*m_children)[i], mm);
    (*m_children)[i]->setLocationInfo(this);

    (*m_children)[i] = (*m_children)[i]->staticResolution(context);
  }

  return this;
}

ASTNode *XQNamespaceConstructor::staticTypingImpl(StaticContext *context)
{
  _src.clear();

  _src.add(m_name->getStaticAnalysis());

  if(m_name->getStaticAnalysis().isUpdating()) {
    XQThrow(StaticErrorException,X("XQNamespaceConstructor::staticTyping"),
            X("It is a static error for the name expression of an namespace constructor "
              "to be an updating expression [err:XUST0001]"));
  }

  unsigned int i;
  for(i = 0; i < m_children->size(); ++i) {
    _src.add((*m_children)[i]->getStaticAnalysis());

    if((*m_children)[i]->getStaticAnalysis().isUpdating()) {
      XQThrow(StaticErrorException,X("XQNamespaceConstructor::staticTyping"),
              X("It is a static error for the a value expression of an namespace constructor "
                "to be an updating expression [err:XUST0001]"));
    }
  }

  _src.getStaticType() = StaticType::NAMESPACE_TYPE;
  _src.creative(true);
  _src.setProperties(StaticAnalysis::DOCORDER | StaticAnalysis::GROUPED |
                     StaticAnalysis::PEER | StaticAnalysis::SUBTREE | StaticAnalysis::SAMEDOC |
                     StaticAnalysis::ONENODE);
  return this;
}

const XMLCh* XQNamespaceConstructor::getNodeType() const
{
  return Node::namespace_string;
}

ASTNode *XQNamespaceConstructor::getName() const
{
  return m_name;
}

const VectorOfASTNodes *XQNamespaceConstructor::getChildren() const
{
  return m_children;
}

void XQNamespaceConstructor::setName(ASTNode *name)
{
  m_name = name;
}
