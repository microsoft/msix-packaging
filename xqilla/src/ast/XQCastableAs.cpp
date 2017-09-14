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

#include <xqilla/ast/XQCastableAs.hpp>
#include <xqilla/ast/XQLiteral.hpp>
#include <xqilla/ast/XQSequence.hpp>
#include <xqilla/schema/SequenceType.hpp>
#include <xqilla/parser/QName.hpp>
#include <xqilla/exceptions/TypeErrorException.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/items/Item.hpp>
#include <xqilla/items/AnyAtomicType.hpp>
#include <xqilla/items/ATBooleanOrDerived.hpp>
#include <xqilla/items/DatatypeFactory.hpp>
#include <xqilla/context/ItemFactory.hpp>
#include <xqilla/ast/XQAtomize.hpp>
#include <xqilla/context/ContextHelpers.hpp>
#include <xqilla/utils/XPath2Utils.hpp>

#include <xercesc/validators/schema/SchemaSymbols.hpp>

XERCES_CPP_NAMESPACE_USE;

XQCastableAs::XQCastableAs(ASTNode* expr, SequenceType* exprType, XPath2MemoryManager* memMgr)
  : ASTNodeImpl(CASTABLE_AS, memMgr),
    _expr(expr),
    _exprType(exprType),
    _isPrimitive(false),
    _typeIndex((AnyAtomicType::AtomicObjectType)-1)
{
}

XQCastableAs::XQCastableAs(ASTNode* expr, SequenceType* exprType, bool isPrimitive, AnyAtomicType::AtomicObjectType typeIndex, XPath2MemoryManager* memMgr)
  : ASTNodeImpl(CASTABLE_AS, memMgr),
    _expr(expr),
    _exprType(exprType),
    _isPrimitive(isPrimitive),
    _typeIndex(typeIndex)
{
}

ASTNode* XQCastableAs::staticResolution(StaticContext *context)
{
  XPath2MemoryManager *mm = context->getMemoryManager();

  _exprType->staticResolution(context);

  const SequenceType::ItemType* itemType = _exprType->getItemType();
  if(XPath2Utils::equals(itemType->getTypeURI(), SchemaSymbols::fgURI_SCHEMAFORSCHEMA) &&
     (XPath2Utils::equals(itemType->getType()->getName(), XMLUni::fgNotationString) ||
      XPath2Utils::equals(itemType->getType()->getName(), AnyAtomicType::fgDT_ANYATOMICTYPE)))
    XQThrow(TypeErrorException,X("XQCastableAs::staticResolution"),
            X("The target type of a castable expression must be an atomic type that is in the in-scope schema types "
              "and is not xs:NOTATION or xdt:anyAtomicType [err:XPST0080]"));

  if(_exprType->getItemTestType() != SequenceType::ItemType::TEST_ATOMIC_TYPE)
    XQThrow(TypeErrorException,X("XQCastableAs::staticResolution"),X("Cannot cast to a non atomic type"));

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

    bool result = false;
    try {
      if(_isPrimitive) {
        ((AnyAtomicType*)_expr->createResult(dContext)->next(dContext).get())->
          castAsNoCheck(_typeIndex, 0, 0, dContext);
      }
      else {
        ((AnyAtomicType*)_expr->createResult(dContext)->next(dContext).get())->
          castAsNoCheck(_typeIndex, _exprType->getTypeURI(),
                        _exprType->getConstrainingType()->getName(), dContext);
      }
      result = true;
    }
    catch(XQException &e) {}

    return XQLiteral::create(dContext->getItemFactory()->createBoolean(result, dContext), dContext, mm, this)->staticResolution(context);
  }

  _expr = new (mm) XQAtomize(_expr, mm);
  _expr->setLocationInfo(this);

  {
    AutoNodeSetOrderingReset orderReset(context);
    _expr = _expr->staticResolution(context);
  }

  return this;
}

ASTNode *XQCastableAs::staticTypingImpl(StaticContext *context)
{
  _src.clear();

  _src.getStaticType() = StaticType::BOOLEAN_TYPE;
  _src.add(_expr->getStaticAnalysis());

  return this;
}

ASTNode *XQCastableAs::getExpression() const {
  return _expr;
}

SequenceType *XQCastableAs::getSequenceType() const {
  return _exprType;
}

void XQCastableAs::setExpression(ASTNode *item) {
  _expr = item;
}

BoolResult XQCastableAs::boolResult(DynamicContext* context) const
{
  // The semantics of the cast expression are as follows:
  //    1. Atomization is performed on the input expression.
  Result toBeCasted(getExpression()->createResult(context));

  const Item::Ptr first = toBeCasted->next(context);

  bool result = false;
  if(first == NULLRCP) {
    //    3. If the result of atomization is an empty sequence:
    //       1. If ? is specified after the target type, the result of the cast expression is an empty sequence.
    //       2. If ? is not specified after the target type, a type error is raised.[err:XP0004][err:XP0006]
    result = getSequenceType()->getOccurrenceIndicator() != SequenceType::EXACTLY_ONE;
  }
  else {
    const Item::Ptr second = toBeCasted->next(context);

    //    2. If the result of atomization is a sequence of more than one atomic value, a type error is raised.[err:XP0004][err:XP0006]
    if(second != NULLRCP) {
      result = false;
    }
    else {
      //    4. If the result of atomization is a single atomic value, the result of the cast expression depends on the input type and the target type.
      //       The normative definition of these rules is given in [XQuery 1.0 and XPath 2.0 Functions and Operators].
      if(getIsPrimitive()) {
        result = ((const AnyAtomicType::Ptr)first)->castable(getTypeIndex(), 0, 0, context);
      }
      else {
        result = ((const AnyAtomicType::Ptr)first)->castable(getTypeIndex(),
                                                             getSequenceType()->getTypeURI(),
                                                             getSequenceType()->getConstrainingType()->getName(),
                                                             context);
      }
    }
  }
  return result;
}

Result XQCastableAs::createResult(DynamicContext* context, int flags) const
{
  return (Item::Ptr)context->getItemFactory()->createBoolean(boolResult(context), context);
}
