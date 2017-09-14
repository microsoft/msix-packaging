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
#include <xqilla/operators/UnaryMinus.hpp>
#include <xqilla/operators/Multiply.hpp>
#include <xqilla/ast/XQLiteral.hpp>
#include <xqilla/items/ATDecimalOrDerived.hpp>
#include <xqilla/exceptions/XPath2ErrorException.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/items/DatatypeFactory.hpp>

/*static*/ const XMLCh UnaryMinus::name[]={ XERCES_CPP_NAMESPACE_QUALIFIER chLatin_U, XERCES_CPP_NAMESPACE_QUALIFIER chLatin_M, XERCES_CPP_NAMESPACE_QUALIFIER chLatin_i, XERCES_CPP_NAMESPACE_QUALIFIER chLatin_n, XERCES_CPP_NAMESPACE_QUALIFIER chLatin_u, XERCES_CPP_NAMESPACE_QUALIFIER chLatin_s, XERCES_CPP_NAMESPACE_QUALIFIER chNull };

UnaryMinus::UnaryMinus(bool positive, const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
  : ArithmeticOperator(name, args, memMgr),
    positive_(positive)
{
  assert(_args.size() == 1);
}

ASTNode* UnaryMinus::staticResolution(StaticContext *context)
{
  return ArithmeticOperator::staticResolution(context);
}

ASTNode *UnaryMinus::staticTypingImpl(StaticContext *context)
{
  _src.clear();

  ASTNode *result = ArithmeticOperator::staticTypingImpl(context);

  if(result == this && positive_) {
    // constant fold unary plus after type checking
    result = *_args.begin();
  }

  return result;
}

void UnaryMinus::calculateStaticType()
{
  const StaticType &arg0 = _args[0]->getStaticAnalysis().getStaticType();
  // untypedAtomic will be promoted to xs:double
  if(arg0.containsType(StaticType::NUMERIC_TYPE)) {
    _src.getStaticType() = arg0 & StaticType::NUMERIC_TYPE;
  }
  if(arg0.containsType(StaticType::UNTYPED_ATOMIC_TYPE)) {
    _src.getStaticType() |= StaticType::DOUBLE_TYPE;
  }
}

Item::Ptr UnaryMinus::execute(const AnyAtomicType::Ptr &atom1, const AnyAtomicType::Ptr &atom2,
                              DynamicContext *context) const
{
  assert(atom2.isNull());

  if(atom1.isNull()) return 0;

  // only works on Numeric types
  if(atom1->isNumericValue()) {
    if(positive_)
      return atom1;
    else
      return ((const Numeric*)atom1.get())->invert(context);
  } else {
    XQThrow(XPath2ErrorException,X("UnaryMinus::createSequence"),
            X("An attempt to apply unary minus a non numeric type has occurred [err:XPTY0004]"));
  }
}
