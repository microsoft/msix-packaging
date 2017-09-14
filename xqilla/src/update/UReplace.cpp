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

#include <xqilla/update/UReplace.hpp>
#include <xqilla/update/PendingUpdateList.hpp>
#include <xqilla/update/UInsertAsLast.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/items/Node.hpp>
#include <xqilla/ast/XQTreatAs.hpp>
#include <xqilla/ast/XQDOMConstructor.hpp>
#include <xqilla/schema/SequenceType.hpp>
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/exceptions/StaticErrorException.hpp>
#include <xqilla/exceptions/XPath2TypeMatchException.hpp>
#include <xqilla/exceptions/DynamicErrorException.hpp>

XERCES_CPP_NAMESPACE_USE;

UReplace::UReplace(ASTNode *target, ASTNode *expr, XPath2MemoryManager* memMgr)
  : ASTNodeImpl(UREPLACE, memMgr),
    target_(target),
    expr_(expr)
{
}

static const XMLCh err_XUTY0008[] = { 'e', 'r', 'r', ':', 'X', 'U', 'T', 'Y', '0', '0', '0', '8', 0 };
static const XMLCh err_XUDY0027[] = { 'e', 'r', 'r', ':', 'X', 'U', 'D', 'Y', '0', '0', '2', '7', 0 };

ASTNode* UReplace::staticResolution(StaticContext *context)
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

  target_ = new (mm) XQTreatAs(target_, targetType2, mm, err_XUTY0008);
  target_->setLocationInfo(this);
  target_ = target_->staticResolution(context);

  expr_ = new (mm) XQContentSequence(expr_, mm);
  expr_->setLocationInfo(this);
  expr_ = expr_->staticResolution(context);

  return this;
}

ASTNode *UReplace::staticTypingImpl(StaticContext *context)
{
  _src.clear();

  _src.add(target_->getStaticAnalysis());

  if(target_->getStaticAnalysis().isUpdating()) {
    XQThrow(StaticErrorException,X("UReplace::staticTyping"),
            X("It is a static error for the target expression of a replace expression "
              "to be an updating expression [err:XUST0001]"));
  }

  _src.add(expr_->getStaticAnalysis());

  if(expr_->getStaticAnalysis().isUpdating()) {
    XQThrow(StaticErrorException,X("UReplace::staticTyping"),
            X("It is a static error for the with expression of a replace expression "
              "to be an updating expression [err:XUST0001]"));
  }

  _src.updating(true);
  return this;
}

Result UReplace::createResult(DynamicContext* context, int flags) const
{
  return 0;
}

PendingUpdateList UReplace::createUpdateList(DynamicContext *context) const
{
  Node::Ptr node = (Node*)target_->createResult(context)->next(context).get();

  if(node->dmNodeKind() == Node::document_string)
    XQThrow(XPath2TypeMatchException,X("UReplace::createUpdateList"),
            X("The target expression of a replace expression does not return a single "
              "node that is not a document node [err:XUTY0008]"));
    
  Node::Ptr parentNode = node->dmParent(context);
  if(parentNode.isNull())
    XQThrow(XPath2TypeMatchException,X("UReplace::createUpdateList"),
            X("The target node of a replace expression does not have a parent [err:XUDY0009]"));

  Result tmpRes = expr_->createResult(context);
  Item::Ptr tmp;

  Sequence value(context->getMemoryManager());

  if(node->dmNodeKind() == Node::element_string ||
     node->dmNodeKind() == Node::text_string ||
     node->dmNodeKind() == Node::comment_string ||
     node->dmNodeKind() == Node::processing_instruction_string) {
    while((tmp = tmpRes->next(context)).notNull()) {
      // 3. If $target is an element, text, comment, or processing instruction node, then $rlist must consist exclusively of zero or
      //    more element, text, comment, or processing instruction nodes [err:XUTY0010].
      if(((Node*)tmp.get())->dmNodeKind() == Node::attribute_string)
        XQThrow(XPath2TypeMatchException,X("UReplace::staticTyping"),
                X("The with expression of a replace expression must not contain attributes when not replacing an attribute [err:XUTY0010]"));
      value.addItem(tmp);
    }
    return PendingUpdate(PendingUpdate::REPLACE_NODE, node, value, this);
  }
  else {
    while((tmp = tmpRes->next(context)).notNull()) {
      // 4. If $target is an attribute node, then:
      //    a. $rlist must consist exclusively of zero or more attribute nodes [err:XUTY0011].
      if(((Node*)tmp.get())->dmNodeKind() != Node::attribute_string)
        XQThrow(XPath2TypeMatchException,X("UReplace::staticTyping"),
                X("The with expression of a replace expression must only contain attributes when replacing an attribute [err:XUTY0011]"));
      //    b. No attribute node in $rlist may have a QName whose implied namespace binding conflicts with a namespace binding in the
      //       "namespaces" property of $parent [err:XUDY0023].
      ATQNameOrDerived::Ptr qname = ((Node*)tmp.get())->dmNodeName(context);
      if(!UInsertAsLast::checkNamespaceBinding(qname, parentNode, context, this)) {
        XMLBuffer buf;
        buf.append(X("Implied namespace binding for the replace expression (\""));
        buf.append(qname->getPrefix());
        buf.append(X("\" -> \""));
        buf.append(qname->getURI());
        buf.append(X("\") conflicts with those already existing on the parent element of the target attribute [err:XUDY0023]"));
        XQThrow(DynamicErrorException, X("URename::createUpdateList"), buf.getRawBuffer());
      }

      value.addItem(tmp);
    }
    return PendingUpdate(PendingUpdate::REPLACE_ATTRIBUTE, node, value, this);
  }
}

