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

#include <xqilla/update/UReplaceValueOf.hpp>
#include <xqilla/update/PendingUpdateList.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/items/Node.hpp>
#include <xqilla/ast/XQTreatAs.hpp>
#include <xqilla/ast/XQAtomize.hpp>
#include <xqilla/ast/XQDOMConstructor.hpp>
#include <xqilla/context/ItemFactory.hpp>
#include <xqilla/schema/SequenceType.hpp>
#include <xqilla/exceptions/StaticErrorException.hpp>
#include <xqilla/exceptions/XPath2TypeMatchException.hpp>
#include <xqilla/exceptions/DynamicErrorException.hpp>

#include <xercesc/framework/XMLBuffer.hpp>

#if defined(XERCES_HAS_CPP_NAMESPACE)
XERCES_CPP_NAMESPACE_USE
#endif

UReplaceValueOf::UReplaceValueOf(ASTNode *target, ASTNode *expr, XPath2MemoryManager* memMgr)
  : ASTNodeImpl(UREPLACE_VALUE_OF, memMgr),
    target_(target),
    expr_(expr)
{
}

static const XMLCh err_XUTY0008[] = { 'e', 'r', 'r', ':', 'X', 'U', 'T', 'Y', '0', '0', '0', '8', 0 };
static const XMLCh err_XUDY0027[] = { 'e', 'r', 'r', ':', 'X', 'U', 'D', 'Y', '0', '0', '2', '7', 0 };

ASTNode* UReplaceValueOf::staticResolution(StaticContext *context)
{
  XPath2MemoryManager *mm = context->getMemoryManager();

  SequenceType *targetType1 = new (mm) SequenceType(new (mm) SequenceType::ItemType(SequenceType::ItemType::TEST_ANYTHING),
                                                    SequenceType::PLUS);
  targetType1->setLocationInfo(this);

  SequenceType *targetType2 = new (mm) SequenceType(new (mm) SequenceType::ItemType(SequenceType::ItemType::TEST_NODE),
                                                    SequenceType::EXACTLY_ONE);
  targetType2->setLocationInfo(this);

  target_ = new (mm) XQTreatAs(target_, targetType1, mm, err_XUDY0027);
  target_->setLocationInfo(this);
  target_ = target_->staticResolution(context);

  target_ = new (mm) XQTreatAs(target_, targetType2, mm, err_XUTY0008);
  target_->setLocationInfo(this);
  target_ = target_->staticResolution(context);

  expr_ = new (mm) XQAtomize(expr_, mm);
  expr_->setLocationInfo(this);
  expr_ = expr_->staticResolution(context);

  return this;
}

ASTNode *UReplaceValueOf::staticTypingImpl(StaticContext *context)
{
  _src.clear();

  _src.add(target_->getStaticAnalysis());

  if(target_->getStaticAnalysis().isUpdating()) {
    XQThrow(StaticErrorException,X("UReplaceValueOf::staticTyping"),
            X("It is a static error for the target expression of a replace expression "
              "to be an updating expression [err:XUST0001]"));
  }

  _src.add(expr_->getStaticAnalysis());

  if(expr_->getStaticAnalysis().isUpdating()) {
    XQThrow(StaticErrorException,X("UReplaceValueOf::staticTyping"),
            X("It is a static error for the with expression of a replace expression "
              "to be an updating expression [err:XUST0001]"));
  }

  _src.updating(true);
  return this;
}

Result UReplaceValueOf::createResult(DynamicContext* context, int flags) const
{
  return 0;
}

PendingUpdateList UReplaceValueOf::createUpdateList(DynamicContext *context) const
{
  Node::Ptr node = (Node*)target_->createResult(context)->next(context).get();

  if(node->dmNodeKind() == Node::document_string)
    XQThrow(XPath2TypeMatchException,X("UReplaceValueOf::createUpdateList"),
            X("The target expression of a replace expression does not return a single "
              "node that is not a document node [err:XUTY0008]"));
    
  XMLBuffer buf;
  XQDOMConstructor::getStringValue(expr_, buf, context);

  // If $target is a comment node, and $string contains two adjacent hyphens or ends with a hyphen, a dynamic error is raised [err:XQDY0072].
  if(node->dmNodeKind() == Node::comment_string) {
    bool foundOne = false;
    for(const XMLCh *str = buf.getRawBuffer(); *str; ++str) {
      if(*str == '-') {
        if(foundOne) {
          XQThrow(DynamicErrorException,X("UReplaceValueOf::createUpdateList"),
                  X("The replace value of expression would result in a comment node whose content contains two adjacent hyphens [err:XQDY0072]"));
        }
        else foundOne = true;
      }
      else {
        foundOne = false;
      }
    }

    if(foundOne) {
      XQThrow(DynamicErrorException,X("UReplaceValueOf::createUpdateList"),
              X("The replace value of expression would result in a comment node whose content ends with a hyphen [err:XQDY0072]"));
    }
  }
  // If $target is a processing instruction node, and $string contains the substring "?>", a dynamic error is raised [err:XQDY0026].
  else if(node->dmNodeKind() == Node::processing_instruction_string) {
    bool foundQuestion = false;
    for(const XMLCh *str = buf.getRawBuffer(); *str; ++str) {
      if(*str == '?') {
        foundQuestion = true;
      }
      else {
        if(foundQuestion && *str == '>') {
          XQThrow(DynamicErrorException,X("UReplaceValueOf::createUpdateList"),
                  X("The replace value of expression would result in a processing instruction node whose content includes the string \"?>\" [err:XQDY0026]"));
        }
        foundQuestion = false;
      }
    }
  }

  Item::Ptr value = context->getItemFactory()->createString(buf.getRawBuffer(), context);

  if(node->dmNodeKind() == Node::element_string) {
    return PendingUpdate(PendingUpdate::REPLACE_ELEMENT_CONTENT, node, value, this);
  }
  else {
    return PendingUpdate(PendingUpdate::REPLACE_VALUE, node, value, this);
  }
}

