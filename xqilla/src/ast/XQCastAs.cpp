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
#include <assert.h>
#include <sstream>

#include <xqilla/ast/XQCastAs.hpp>
#include <xqilla/ast/XQLiteral.hpp>
#include <xqilla/ast/XQSequence.hpp>
#include <xqilla/schema/SequenceType.hpp>
#include <xqilla/parser/QName.hpp>
#include <xqilla/exceptions/TypeErrorException.hpp>
#include <xqilla/exceptions/XPath2TypeCastException.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/items/Item.hpp>
#include <xqilla/items/AnyAtomicType.hpp>
#include <xqilla/ast/XQAtomize.hpp>
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/context/ItemFactory.hpp>

#include <xercesc/validators/schema/SchemaSymbols.hpp>

XERCES_CPP_NAMESPACE_USE;

XQCastAs::XQCastAs(ASTNode* expr, SequenceType* exprType, XPath2MemoryManager* memMgr)
  : ASTNodeImpl(CAST_AS, memMgr),
    _expr(expr),
    _exprType(exprType),
    _isPrimitive(false),
    _typeIndex((AnyAtomicType::AtomicObjectType)-1)
{
}

XQCastAs::XQCastAs(ASTNode* expr, SequenceType* exprType, bool isPrimitive, AnyAtomicType::AtomicObjectType typeIndex, XPath2MemoryManager* memMgr)
  : ASTNodeImpl(CAST_AS, memMgr),
    _expr(expr),
    _exprType(exprType),
    _isPrimitive(isPrimitive),
    _typeIndex(typeIndex)
{
}

ASTNode* XQCastAs::staticResolution(StaticContext *context)
{
  XPath2MemoryManager *mm = context->getMemoryManager();

  _exprType->staticResolution(context);

  const SequenceType::ItemType *itemType = _exprType->getItemType();
  if(itemType != NULL) {
    if(XPath2Utils::equals(itemType->getTypeURI(), SchemaSymbols::fgURI_SCHEMAFORSCHEMA) &&
       (XPath2Utils::equals(itemType->getType()->getName(), XMLUni::fgNotationString) ||
        XPath2Utils::equals(itemType->getType()->getName(), AnyAtomicType::fgDT_ANYATOMICTYPE)))
      XQThrow(TypeErrorException,X("XQCastAs::staticResolution"),
              X("The target type of a cast expression must be an atomic type that is in the in-scope schema types and is not xs:NOTATION or xdt:anyAtomicType [err:XPST0080]"));
  }

  if(_exprType->getItemTestType() != SequenceType::ItemType::TEST_ATOMIC_TYPE)
    XQThrow(TypeErrorException,X("XQCastAs::staticResolution"),X("Cannot cast to a non atomic type"));

  _typeIndex = context->getItemFactory()->
    getPrimitiveTypeIndex(_exprType->getTypeURI(),
                          _exprType->getConstrainingType()->getName(), _isPrimitive);

  // If this is a cast to xs:QName or xs:NOTATION and the argument is a string literal
  // evaluate immediately, since they aren't allowed otherwise
  if((_typeIndex == AnyAtomicType::QNAME || _typeIndex == AnyAtomicType::NOTATION) &&
     _expr->getType() == LITERAL &&
     ((XQLiteral*)_expr)->getPrimitiveType() == AnyAtomicType::STRING) {

    AutoDelete<DynamicContext> dContext(context->createDynamicContext());
    dContext->setMemoryManager(mm);

    AnyAtomicType::Ptr item = (AnyAtomicType*)_expr->createResult(dContext)->next(dContext).get();
    try {
      if(_isPrimitive) {
        item = item->castAsNoCheck(_typeIndex, 0, 0, dContext);
      }
      else {
        item = item->castAsNoCheck(_typeIndex, _exprType->getTypeURI(),
                                   _exprType->getConstrainingType()->getName(), dContext);
      }
    }
    catch(XQException &e) {
      if(e.getXQueryLine() == 0)
        e.setXQueryPosition(this);
      throw;
    }

    return XQLiteral::create(item, dContext, mm, this)->staticResolution(context);
  }

  _expr = new (mm) XQAtomize(_expr, mm);
  _expr->setLocationInfo(this);
  _expr = _expr->staticResolution(context);

  return this;
}

ASTNode *XQCastAs::staticTypingImpl(StaticContext *context)
{
  _src.clear();

  const SequenceType::ItemType *itemType = _exprType->getItemType();
  if(itemType != NULL) {
    _src.getStaticType() = StaticType::create(_typeIndex);
    if(_exprType->getOccurrenceIndicator() == SequenceType::QUESTION_MARK) {
      _src.getStaticType().multiply(0, 1);
    }
  }

  _src.add(_expr->getStaticAnalysis());

  if(_typeIndex == AnyAtomicType::QNAME &&
     !_expr->getStaticAnalysis().getStaticType().containsType(StaticType::QNAME_TYPE) &&
     (_exprType->getOccurrenceIndicator() == SequenceType::EXACTLY_ONE ||
      _expr->getStaticAnalysis().getStaticType().containsType(StaticType::ITEM_TYPE))) {
    XQThrow(TypeErrorException,X("XQCastAs::staticTyping"),
            X("Only a subtype of xs:QName can be cast to a subtype of xs:QName [err:XPTY0004]"));
  }
  if(_typeIndex == AnyAtomicType::NOTATION &&
     !_expr->getStaticAnalysis().getStaticType().containsType(StaticType::NOTATION_TYPE) &&
     (_exprType->getOccurrenceIndicator() == SequenceType::EXACTLY_ONE ||
      _expr->getStaticAnalysis().getStaticType().containsType(StaticType::ITEM_TYPE))) {
    XQThrow(TypeErrorException,X("XQCastAs::staticTyping"),
            X("Only a subtype of xs:NOTATION can be cast to a subtype of xs:NOTATION [err:XPTY0004]"));
  }

  return this;
}

ASTNode *XQCastAs::getExpression() const {
  return _expr;
}

SequenceType *XQCastAs::getSequenceType() const {
  return _exprType;
}

void XQCastAs::setExpression(ASTNode *item) {
  _expr = item;
}

AnyAtomicType::Ptr XQCastAs::cast(const AnyAtomicType::Ptr &in, DynamicContext *context) const
{
  try {
    if(_isPrimitive) {
      return in->castAs(_typeIndex, 0, 0, context);
    }
    else {
      return in->castAs(_typeIndex, _exprType->getTypeURI(),
                        _exprType->getConstrainingType()->getName(), context);
    }
  }
  catch(XQException &e) {
    if(e.getXQueryLine() == 0)
      e.setXQueryPosition(this);
    throw;
  }
}

AnyAtomicType::Ptr XQCastAs::cast(const XMLCh *value, DynamicContext *context) const
{
  try {
    if(_isPrimitive) {
      return context->getItemFactory()->
        createDerivedFromAtomicType(_typeIndex, value, context);
    }
    else {
      return context->getItemFactory()->
        createDerivedFromAtomicType(_typeIndex, _exprType->getTypeURI(),
                                    _exprType->getConstrainingType()->getName(), value, context);
    }
  }
  catch(XQException &e) {
    if(e.getXQueryLine() == 0)
      e.setXQueryPosition(this);
    throw;
  }
}

Result XQCastAs::createResult(DynamicContext* context, int flags) const
{
  // The semantics of the cast expression are as follows:
  //    1. Atomization is performed on the input expression.
  Result toBeCasted(getExpression()->createResult(context));

  const Item::Ptr first = toBeCasted->next(context);

  if(first.isNull()) {
    //    3. If the result of atomization is an empty sequence:
    //       1. If ? is specified after the target type, the result of the cast expression is an empty sequence.
    //       2. If ? is not specified after the target type, a type error is raised [err:XPTY0004].
    if(getSequenceType()->getOccurrenceIndicator() == SequenceType::EXACTLY_ONE) {
      XQThrow(TypeErrorException,X("XQCastAs::createResult"),
              X("The input to a non-optional cast as expression is an empty sequence [err:XPTY0004]"));
    }
    else {
      return 0;
    }
  }

  const Item::Ptr second = toBeCasted->next(context);

  //    2. If the result of atomization is a sequence of more than one atomic value, a type error is raised.[err:XPTY0004]
  if(second.notNull()) {
    XQThrow(TypeErrorException,X("XQCastAs::createResult"),
            X("The input to a cast as expression is more than one atomic value [err:XPTY0004]"));
  }

  //    4. If the result of atomization is a single atomic value, the result of the cast expression depends on the input type and the target type.
  //       The normative definition of these rules is given in [XQuery 1.0 and XPath 2.0 Functions and Operators].
  return (Item::Ptr)cast((const AnyAtomicType*)first.get(), context);
}
