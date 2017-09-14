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
#include <xqilla/ast/XQDOMConstructor.hpp>
#include <xqilla/ast/XQAtomize.hpp>
#include <xqilla/ast/XQTreatAs.hpp>
#include <xqilla/ast/XQLiteral.hpp>
#include <xqilla/ast/XQNamespaceBinding.hpp>
#include <xqilla/schema/SequenceType.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/context/ItemFactory.hpp>
#include <xqilla/context/ContextHelpers.hpp>
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/utils/XPath2NSUtils.hpp>
#include <xqilla/exceptions/ASTException.hpp>
#include <xqilla/exceptions/XPath2TypeMatchException.hpp>
#include <xqilla/exceptions/StaticErrorException.hpp>
#include <xqilla/events/ContentSequenceFilter.hpp>
#include <xqilla/runtime/ClosureResult.hpp>

#include <xercesc/util/XMLChar.hpp>
#include <xercesc/validators/schema/SchemaSymbols.hpp>

#if defined(XERCES_HAS_CPP_NAMESPACE)
XERCES_CPP_NAMESPACE_USE
#endif

XQDOMConstructor::XQDOMConstructor(XPath2MemoryManager* mm)
  : ASTNodeImpl(DOM_CONSTRUCTOR, mm),
    queryPathTree_(0)
{
}

Result XQDOMConstructor::createResult(DynamicContext* context, int flags) const
{
  return EventGenerator::createResult(this, queryPathTree_, context);
}

bool XQDOMConstructor::getStringValue(const VectorOfASTNodes* m_children, XMLBuffer &value, DynamicContext *context)
{
  bool bSomethingFound=false;
  for(VectorOfASTNodes::const_iterator itCont = m_children->begin();
      itCont != m_children->end (); ++itCont) {
    if(getStringValue(*itCont, value, context))
      bSomethingFound = true;
  }
  return bSomethingFound;
}

bool XQDOMConstructor::getStringValue(const ASTNode *child, XMLBuffer &value, DynamicContext *context)
{
  bool bSomethingFound=false;
  Result childList = child->createResult(context);
  Item::Ptr item;
  bool addSpace = false;
  while((item = childList->next(context)) != NULLRCP) {
    if(addSpace) value.append(' ');
    else addSpace = true;
    value.append(item->asString(context));
    bSomethingFound=true;
  }
  return bSomethingFound;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

XQContentSequence::XQContentSequence(ASTNode *expr, XPath2MemoryManager* mm)
  : ASTNodeImpl(CONTENT_SEQUENCE, mm),
    expr_(expr)
{
}

ASTNode* XQContentSequence::staticResolution(StaticContext *context)
{
  expr_ = expr_->staticResolution(context);
  return this;
}

ASTNode *XQContentSequence::staticTypingImpl(StaticContext *context)
{
  _src.clear();

  _src.copy(expr_->getStaticAnalysis());

  if(!expr_->getStaticAnalysis().getStaticType().containsType(StaticType::DOCUMENT_TYPE|StaticType::ANY_ATOMIC_TYPE)) {
    ASTNode *pChild = expr_;

    // Not needed if the wrapped expression is a DOM_CONSTRUCTOR
    if(pChild->getType() == ASTNode::DOM_CONSTRUCTOR ||
       (pChild->getType() == ASTNode::NAMESPACE_BINDING &&
        ((XQNamespaceBinding*)pChild)->getExpression()->getType() == ASTNode::DOM_CONSTRUCTOR)) {
      return expr_;
    }
  }

  _src.getStaticType().substitute(StaticType::ANY_ATOMIC_TYPE, StaticType::TEXT_TYPE);
  _src.getStaticType().substitute(StaticType::DOCUMENT_TYPE, StaticType(StaticType::ELEMENT_TYPE | StaticType::TEXT_TYPE | StaticType::PI_TYPE |
                                                                        StaticType::COMMENT_TYPE, 0, StaticType::UNLIMITED));

  _src.creative(true);
  return this;
}

Result XQContentSequence::createResult(DynamicContext* context, int flags) const
{
  return EventGenerator::createResult(this, 0, context);
}

EventGenerator::Ptr XQContentSequence::generateEvents(EventHandler *events, DynamicContext *context,
                                                 bool preserveNS, bool preserveType) const
{
  preserveNS = context->getPreserveNamespaces();
  preserveType = context->getConstructionMode() == StaticContext::CONSTRUCTION_MODE_PRESERVE;

  if(!expr_->getStaticAnalysis().getStaticType().containsType(StaticType::DOCUMENT_TYPE|StaticType::ANY_ATOMIC_TYPE)) {
    return new ClosureEventGenerator(expr_, context, preserveNS, preserveType);
  }

  ContentSequenceFilter filter(events);
  expr_->generateAndTailCall(&filter, context, preserveNS, preserveType);
  return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

XQDirectName::XQDirectName(const XMLCh *qname, bool useDefaultNamespace, XPath2MemoryManager* mm)
  : ASTNodeImpl(DIRECT_NAME, mm),
    qname_(qname),
    useDefaultNamespace_(useDefaultNamespace)
{
}

ASTNode *XQDirectName::staticResolution(StaticContext *context)
{
  XPath2MemoryManager *mm = context->getMemoryManager();

  const XMLCh* prefix = XPath2NSUtils::getPrefix(qname_, context->getMemoryManager());
  const XMLCh* uri = 0;
  if(prefix == 0 || *prefix == 0) {
    // If qname has no prefix
    if(useDefaultNamespace_)
      uri = context->getDefaultElementAndTypeNS();
  }
  else {
    // If qname has a prefix
    uri = context->getUriBoundToPrefix(prefix, this);
  }

  return (new (mm) XQQNameLiteral(SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                                  SchemaSymbols::fgDT_QNAME, uri, prefix,
                                  XPath2NSUtils::getLocalName(qname_), mm))->staticResolution(context);
}

ASTNode *XQDirectName::staticTypingImpl(StaticContext *context)
{
  // Never happens
  return this;
}

Result XQDirectName::createResult(DynamicContext* context, int flags) const
{
  // Never happens
  return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

XQNameExpression::XQNameExpression(ASTNode *expr, XPath2MemoryManager* mm)
  : ASTNodeImpl(NAME_EXPRESSION, mm),
    expr_(expr)
{
}

ASTNode* XQNameExpression::staticResolution(StaticContext *context)
{
  XPath2MemoryManager *mm = context->getMemoryManager();

  expr_ = new (mm) XQAtomize(expr_, mm);
  expr_->setLocationInfo(this);
  SequenceType *seqType = new (mm) SequenceType(new (mm) SequenceType::ItemType(SequenceType::ItemType::TEST_ANYTHING),
                                                SequenceType::EXACTLY_ONE);
  seqType->setLocationInfo(this);
  expr_ = new (mm) XQTreatAs(expr_, seqType, mm);
  expr_->setLocationInfo(this);
  expr_ = expr_->staticResolution(context);

  return this;
}

ASTNode *XQNameExpression::staticTypingImpl(StaticContext *context)
{
  _src.clear();

  _src.getStaticType() = StaticType::QNAME_TYPE;

  _src.add(expr_->getStaticAnalysis());

  if(expr_->getStaticAnalysis().getStaticType().isType(StaticType::QNAME_TYPE)) {
    return expr_;
  }

  if(!expr_->getStaticAnalysis().getStaticType().
     containsType(StaticType::QNAME_TYPE|StaticType::STRING_TYPE|StaticType::UNTYPED_ATOMIC_TYPE)) {
    XMLBuffer buf;
    buf.set(X("The name expression must be a single xs:QName, xs:string or xs:untypedAtomic"));
    buf.append(X(" - the expression has a static type of "));
    expr_->getStaticAnalysis().getStaticType().typeToBuf(buf);
    buf.append(X(" [err:XPTY0004]"));
    XQThrow(XPath2TypeMatchException, X("XQNameExpression::staticTyping"), buf.getRawBuffer());
  }

  return this;
}

Result XQNameExpression::createResult(DynamicContext* context, int flags) const
{
  AnyAtomicType::Ptr itemName = getExpression()->createResult(context)->next(context);

  switch(itemName->getPrimitiveTypeIndex()) {
  case AnyAtomicType::QNAME:
    return (Item::Ptr)itemName;
  case AnyAtomicType::STRING:
  case AnyAtomicType::UNTYPED_ATOMIC:
    try {
      return (Item::Ptr)context->getItemFactory()->createDerivedFromAtomicType(AnyAtomicType::QNAME, itemName->asString(context), context);
    }
    catch(XQException &ex) {
      XQThrow(ASTException,X("XQNameExpression::NameExpressionResult::createResult"),
              X("The name expression cannot be converted to a xs:QName [err:XQDY0074]"));
    }
  default:
    break;
  }

  XMLBuffer buf;
  buf.set(X("The name expression must be a single xs:QName, xs:string or xs:untypedAtomic"));
  buf.append(X(" - found item of type "));
  itemName->typeToBuffer(context, buf);
  buf.append(X(" [err:XPTY0004]"));
  XQThrow(XPath2TypeMatchException, X("XQNameExpression::NameExpressionResult::createResult"), buf.getRawBuffer());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

XQSimpleContent::XQSimpleContent(VectorOfASTNodes *children, XPath2MemoryManager* mm)
  : ASTNodeImpl(SIMPLE_CONTENT, mm),
    children_(children)
{
}

ASTNode* XQSimpleContent::staticResolution(StaticContext *context)
{
  XPath2MemoryManager *mm = context->getMemoryManager();

  unsigned int i;
  for(i = 0;i < children_->size(); ++i) {
    // atomize content and run static resolution 
    (*children_)[i] = new (mm) XQAtomize((*children_)[i], mm);
    (*children_)[i]->setLocationInfo(this);

    (*children_)[i] = (*children_)[i]->staticResolution(context);
  }

  return this;
}

ASTNode *XQSimpleContent::staticTypingImpl(StaticContext *context)
{
  _src.clear();

  bool constant = true;
  unsigned int i;
  for(i = 0; i < children_->size(); ++i) {
    _src.add((*children_)[i]->getStaticAnalysis());

    if((*children_)[i]->getStaticAnalysis().isUpdating()) {
      XQThrow(StaticErrorException,X("XQSimpleContent::staticTyping"),
              X("It is a static error for the a value expression of an attribute constructor "
                "to be an updating expression [err:XUST0001]"));
    }

    if(!(*children_)[i]->isConstant()) constant = false;
  }

  _src.getStaticType() = StaticType::STRING_TYPE;

  return this;
}

Result XQSimpleContent::createResult(DynamicContext* context, int flags) const
{
  // TBD separator - jpcs
  XMLBuffer value;
  XQDOMConstructor::getStringValue(getChildren(), value, context);
  return (Item::Ptr)context->getItemFactory()->createString(value.getRawBuffer(), context);
}
