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
#include <xqilla/ast/XQPIConstructor.hpp>
#include <xqilla/ast/StaticAnalysis.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/context/ItemFactory.hpp>
#include <xqilla/exceptions/ASTException.hpp>
#include <xqilla/items/Node.hpp>
#include <xqilla/ast/XQAtomize.hpp>
#include <xqilla/events/EventHandler.hpp>
#include <xqilla/exceptions/StaticErrorException.hpp>

#include <xercesc/framework/XMLBuffer.hpp>
#include <xercesc/util/XMLChar.hpp>

#if defined(XERCES_HAS_CPP_NAMESPACE)
XERCES_CPP_NAMESPACE_USE
#endif

XQPIConstructor::XQPIConstructor(ASTNode* name, ASTNode* value, XPath2MemoryManager* mm, bool xslt)
  : XQDOMConstructor(mm),
    m_name(name),
    m_value(value),
    xslt_(xslt)
{
}

static bool checkString(const XMLCh *str)
{
  // Check for the substring "?>"
  bool foundQ = false;
  const XMLCh *ptr = str;
  while(*ptr != 0) {
    if(*ptr == chCloseAngle && foundQ)
      return false;
    else if(*ptr == chQuestion) foundQ = true;
    else foundQ = false;
    ++ptr;
  }
  return true;
}

EventGenerator::Ptr XQPIConstructor::generateEvents(EventHandler *events, DynamicContext *context,
                                               bool preserveNS, bool preserveType) const
{
  Result resName = m_name->createResult(context);
  AnyAtomicType::Ptr itemName = resName->next(context);
  if(itemName.isNull() || resName->next(context).notNull())
    XQThrow(ASTException,X("DOM Constructor"),X("The target for the processing instruction must be a single "
                                                "xs:NCName, xs:string or xs:untypedAtomic item [err:XPTY0004]"));

  const XMLCh* nodeName = NULL;
  // the specs specify that a xs:NCName could be returned, but we create a xs:string in that case
  if(itemName->getPrimitiveTypeIndex() == AnyAtomicType::STRING ||
     itemName->getPrimitiveTypeIndex() == AnyAtomicType::UNTYPED_ATOMIC) {
    nodeName = itemName->asString(context);
    if(!XMLChar1_0::isValidNCName(nodeName, XMLString::stringLen(nodeName)))
      XQThrow(ASTException,X("DOM Constructor"),
              X("The target for the processing instruction must be a valid xs:NCName [err:XQDY0041]"));
  }
  else
    XQThrow(ASTException,X("DOM Constructor"),X("The target for the processing instruction must be a single "
                                                "xs:NCName, xs:string or xs:untypedAtomic item [err:XPTY0004]"));

  if(XMLString::compareIString(nodeName, XMLUni::fgXMLString) == 0)
    XQThrow(ASTException,X("DOM Constructor"),
            X("The target for the processing instruction must not be 'XML' [err:XQDY0064]"));

  if(!checkString(nodeName)) {
    XQThrow(ASTException,X("DOM Constructor"),
            X("The target for the processing instruction must not contain the string '?>' [err:XQDY0026]"));
  }

  if(nodeName == NULL || *nodeName == 0)
    XQThrow(ASTException,X("DOM Constructor"),X("The name for the processing instruction is empty"));

  XMLBuffer value;
  getStringValue(m_value, value, context);

  if(!checkString(value.getRawBuffer())) {
    XQThrow(ASTException,X("DOM Constructor"),
            X("The content for the processing instruction must not contain the string '?>' [err:XQDY0026]"));
  }

  const XMLCh* piContent = value.getRawBuffer();
  // remove leading whitespace
  while(XMLChar1_0::isWhitespace(*piContent)) piContent++;

  events->piEvent(nodeName, piContent);
  return 0;
}

ASTNode* XQPIConstructor::staticResolution(StaticContext *context)
{
  XPath2MemoryManager *mm = context->getMemoryManager();

  m_name = new (mm) XQAtomize(m_name, mm);
  m_name->setLocationInfo(this);
  m_name = m_name->staticResolution(context);

  m_value = new (mm) XQAtomize(m_value, mm);
  m_value->setLocationInfo(this);
  m_value = m_value->staticResolution(context);

  return this;
}

ASTNode *XQPIConstructor::staticTypingImpl(StaticContext *context)
{
  _src.clear();

  _src.add(m_name->getStaticAnalysis());

  if(m_name->getStaticAnalysis().isUpdating()) {
    XQThrow(StaticErrorException,X("XQPIConstructor::staticTyping"),
            X("It is a static error for the name expression of a processing instruction node constructor "
              "to be an updating expression [err:XUST0001]"));
  }

  _src.add(m_value->getStaticAnalysis());

  if(m_value->getStaticAnalysis().isUpdating()) {
    XQThrow(StaticErrorException,X("XQPIConstructor::staticTyping"),
            X("It is a static error for the value expression of a processing instruction node constructor "
              "to be an updating expression [err:XUST0001]"));
  }

  _src.getStaticType() = StaticType::PI_TYPE;
  _src.creative(true);
  _src.setProperties(StaticAnalysis::DOCORDER | StaticAnalysis::GROUPED |
                     StaticAnalysis::PEER | StaticAnalysis::SUBTREE | StaticAnalysis::SAMEDOC |
                     StaticAnalysis::ONENODE);
  return this;
}

const XMLCh* XQPIConstructor::getNodeType() const
{
  return Node::processing_instruction_string;
}

ASTNode *XQPIConstructor::getName() const
{
  return m_name;
}

ASTNode *XQPIConstructor::getValue() const
{
  return m_value;
}

void XQPIConstructor::setName(ASTNode *name)
{
  m_name = name;
}

void XQPIConstructor::setValue(ASTNode *value)
{
  m_value = value;
}

