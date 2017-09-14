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

#include <xqilla/ast/XQEffectiveBooleanValue.hpp>
#include <xqilla/context/StaticContext.hpp>
#include <xqilla/items/Node.hpp>
#include <xqilla/exceptions/StaticErrorException.hpp>
#include <xqilla/exceptions/XPath2TypeMatchException.hpp>
#include <xqilla/context/ContextHelpers.hpp>
#include <xqilla/ast/XQLiteral.hpp>
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/context/ItemFactory.hpp>

XERCES_CPP_NAMESPACE_USE

XQEffectiveBooleanValue::XQEffectiveBooleanValue(ASTNode* expr, XPath2MemoryManager* memMgr)
  : ASTNodeImpl(EBV, memMgr),
    expr_(expr)
{
}

ASTNode* XQEffectiveBooleanValue::staticResolution(StaticContext *context)
{
  AutoNodeSetOrderingReset orderReset(context);
  expr_ = expr_->staticResolution(context);
  return this;
}

ASTNode *XQEffectiveBooleanValue::staticTypingImpl(StaticContext *context)
{
  _src.clear();

  _src.add(expr_->getStaticAnalysis());
  _src.getStaticType() = StaticType::BOOLEAN_TYPE;

  if(expr_->getStaticAnalysis().isUpdating()) {
    XQThrow(StaticErrorException,X("XQEffectiveBooleanValue::staticTyping"),
            X("It is a static error to take the effective boolean value "
              "of an updating expression [err:XUST0001]"));
  }

  if(expr_->getStaticAnalysis().getStaticType().getMin() >= 2 &&
     !expr_->getStaticAnalysis().getStaticType().containsType(StaticType::NODE_TYPE)) {
    XQThrow(XPath2TypeMatchException, X("XQEffectiveBooleanValue::staticTyping"),
            X("Effective Boolean Value cannot be extracted from this type [err:FORG0006]"));
  }

  return this;
}

static inline bool getEffectiveBooleanValueInternal(const Item::Ptr &first, const Item::Ptr &second, DynamicContext* context, const LocationInfo *info)
{
  // If its operand is a singleton value ...
  if(second.isNull() && first->isAtomicValue()) {
    const AnyAtomicType::Ptr atom=first;
    // ... of type xs:boolean or derived from xs:boolean, fn:boolean returns the value of its operand unchanged.
    if(atom->getPrimitiveTypeIndex() == AnyAtomicType::BOOLEAN)
        return ((ATBooleanOrDerived*)atom.get())->isTrue();

    // ... of type xs:string, xdt:untypedAtomic, or a type derived from one of these, fn:boolean returns false 
    // if the operand value has zero length; otherwise it returns true.
    if((atom->getPrimitiveTypeIndex() == AnyAtomicType::STRING || 
        atom->getPrimitiveTypeIndex() == AnyAtomicType::ANY_URI || 
        atom->getPrimitiveTypeIndex() == AnyAtomicType::UNTYPED_ATOMIC))
      return !XPath2Utils::equals(atom->asString(context), XERCES_CPP_NAMESPACE_QUALIFIER XMLUni::fgZeroLenString);

    // ... of any numeric type or derived from a numeric type, fn:boolean returns false if the operand value is 
    // NaN or is numerically equal to zero; otherwise it returns true.
    if(atom->isNumericValue()) {
      return (!((Numeric*)atom.get())->isZero() && !((Numeric*)atom.get())->isNaN());
    }
  }

  // In all other cases, fn:boolean raises a type error. 
  XQThrow3(XPath2TypeMatchException,X("EffectiveBooleanValueResult::nextOrTail"), X("Effective Boolean Value cannot be extracted from this type [err:FORG0006]"), info);
  return true;
}

bool XQEffectiveBooleanValue::get(const Item::Ptr &first, const Item::Ptr &second, DynamicContext* context, const LocationInfo *info)
{
  // From $ 15.1.4 of the F&O specs:
  // The effective boolean value of an operand is defined as follows:
  //
  // If its operand is an empty sequence, fn:boolean returns false.
  if(first.isNull()) {
    return false;
  }

  // If its operand is a sequence whose first item is a node, fn:boolean returns true.
  if(first->isNode())
    return true;

  return getEffectiveBooleanValueInternal(first, second, context, info);
}

BoolResult XQEffectiveBooleanValue::boolResult(DynamicContext* context) const
{
  bool result;
  Result parent = expr_->createResult(context);
  const Item::Ptr first = parent->next(context);
  if(first.isNull()) {
    result = false;
  }
  else if(first->isNode()) {
    result = true;
  }
  else {
    result = getEffectiveBooleanValueInternal(first, parent->next(context), context, this);
  }

  return result;
}

Result XQEffectiveBooleanValue::createResult(DynamicContext* context, int flags) const
{
  return (Item::Ptr)context->getItemFactory()->createBoolean(boolResult(context), context);
}
