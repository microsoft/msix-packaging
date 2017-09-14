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
#include <xqilla/ast/XQTextConstructor.hpp>
#include <xqilla/ast/StaticAnalysis.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/context/ItemFactory.hpp>
#include <xqilla/exceptions/ASTException.hpp>
#include <xqilla/items/Node.hpp>
#include <xqilla/ast/XQAtomize.hpp>
#include <xqilla/events/EventHandler.hpp>
#include <xqilla/exceptions/StaticErrorException.hpp>

#include <xercesc/framework/XMLBuffer.hpp>

#if defined(XERCES_HAS_CPP_NAMESPACE)
XERCES_CPP_NAMESPACE_USE
#endif

XQTextConstructor::XQTextConstructor(ASTNode *value, XPath2MemoryManager* mm)
  : XQDOMConstructor(mm),
    m_value(value)
{
}

EventGenerator::Ptr XQTextConstructor::generateEvents(EventHandler *events, DynamicContext *context,
                                                 bool preserveNS, bool preserveType) const
{
  XMLBuffer value;
  if(getStringValue(m_value, value, context))
    events->textEvent(value.getRawBuffer());
  return 0;
}

ASTNode* XQTextConstructor::staticResolution(StaticContext *context)
{
  XPath2MemoryManager *mm = context->getMemoryManager();

  m_value = new (mm) XQAtomize(m_value, mm);
  m_value->setLocationInfo(this);

  m_value = m_value->staticResolution(context);

  return this;
}

ASTNode *XQTextConstructor::staticTypingImpl(StaticContext *context)
{
  _src.clear();

  _src.add(m_value->getStaticAnalysis());

  if(m_value->getStaticAnalysis().isUpdating()) {
    XQThrow(StaticErrorException,X("XQTextConstructor::staticTyping"),
            X("It is a static error for the value expression of a text node constructor "
              "to be an updating expression [err:XUST0001]"));
  }

  _src.getStaticType() = StaticType(StaticType::TEXT_TYPE, 0, 1);
  _src.creative(true);
  _src.setProperties(StaticAnalysis::DOCORDER | StaticAnalysis::GROUPED |
                     StaticAnalysis::PEER | StaticAnalysis::SUBTREE | StaticAnalysis::SAMEDOC |
                     StaticAnalysis::ONENODE);
  return this;
}

const XMLCh* XQTextConstructor::getNodeType() const
{
  return Node::text_string;
}

ASTNode *XQTextConstructor::getValue() const
{
  return m_value;
}

void XQTextConstructor::setValue(ASTNode *value)
{
  m_value = value;
}

