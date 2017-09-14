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
#include <xqilla/ast/XQCommentConstructor.hpp>
#include <xqilla/ast/StaticAnalysis.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/context/ItemFactory.hpp>
#include <xqilla/exceptions/ASTException.hpp>
#include <xqilla/items/Node.hpp>
#include <xqilla/ast/XQAtomize.hpp>
#include <xqilla/events/EventHandler.hpp>
#include <xqilla/exceptions/StaticErrorException.hpp>

#include <xercesc/util/XMLChar.hpp>
#include <xercesc/framework/XMLBuffer.hpp>

#if defined(XERCES_HAS_CPP_NAMESPACE)
XERCES_CPP_NAMESPACE_USE
#endif

XQCommentConstructor::XQCommentConstructor(ASTNode *value, XPath2MemoryManager* mm, bool xslt)
  : XQDOMConstructor(mm),
    m_value(value),
    xslt_(xslt)
{
}

EventGenerator::Ptr XQCommentConstructor::generateEvents(EventHandler *events, DynamicContext *context,
                                                    bool preserveNS, bool preserveType) const
{
  XMLBuffer value;
  getStringValue(m_value, value, context);

  // Check for two dashes in a row, or a dash at the end
  if(xslt_) {
    XMLBuffer buf(value.getLen());
    bool foundDash = false;
    const XMLCh *ptr = value.getRawBuffer();
    const XMLCh *end = ptr + value.getLen();
    while(ptr != end) {
      if(*ptr == chDash) {
        if(foundDash) {
          buf.append(' ');
        }
        foundDash = true;
      }
      else foundDash = false;

      buf.append(*ptr);
      ++ptr;
    }

    if(foundDash) {
      buf.append(' ');
    }

    value.set(buf.getRawBuffer());
  }
  else {
    bool foundDash = false;
    const XMLCh *ptr = value.getRawBuffer();
    const XMLCh *end = ptr + value.getLen();
    while(ptr != end) {
      if(*ptr == chDash) {
        if(foundDash) break;
        foundDash = true;
      }
      else foundDash = false;
      ++ptr;
    }
    if(foundDash)
      XQThrow(ASTException,X("DOM Constructor"),X("It is a dynamic error if the result of the content expression of "
                                                  "a computed comment constructor contains two adjacent hyphens or "
                                                  "ends with a hyphen. [err:XQDY0072]"));
  }

  events->commentEvent(value.getRawBuffer());
  return 0;
}

ASTNode* XQCommentConstructor::staticResolution(StaticContext *context)
{
  XPath2MemoryManager *mm = context->getMemoryManager();

  m_value = new (mm) XQAtomize(m_value, mm);
  m_value->setLocationInfo(this);

  m_value = m_value->staticResolution(context);

  return this;
}

ASTNode *XQCommentConstructor::staticTypingImpl(StaticContext *context)
{
  _src.clear();

  _src.add(m_value->getStaticAnalysis());

  if(m_value->getStaticAnalysis().isUpdating()) {
    XQThrow(StaticErrorException,X("XQCommentConstructor::staticTyping"),
            X("It is a static error for the value expression of a comment node constructor "
              "to be an updating expression [err:XUST0001]"));
  }

  _src.getStaticType() = StaticType::COMMENT_TYPE;
  _src.creative(true);
  _src.setProperties(StaticAnalysis::DOCORDER | StaticAnalysis::GROUPED |
                     StaticAnalysis::PEER | StaticAnalysis::SUBTREE | StaticAnalysis::SAMEDOC |
                     StaticAnalysis::ONENODE);
  return this;
}

const XMLCh* XQCommentConstructor::getNodeType() const
{
  return Node::comment_string;
}

ASTNode *XQCommentConstructor::getValue() const
{
  return m_value;
}

void XQCommentConstructor::setValue(ASTNode *value)
{
  m_value = value;
}

