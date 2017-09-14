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

#include <xqilla/operators/OrderComparison.hpp>
#include <xqilla/items/Node.hpp>
#include <xqilla/items/ATBooleanOrDerived.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/items/DatatypeFactory.hpp>
#include <xqilla/context/ItemFactory.hpp>
#include <xqilla/ast/XQTreatAs.hpp>
#include <xqilla/schema/SequenceType.hpp>
#include <xqilla/exceptions/StaticErrorException.hpp>

/*static*/ const XMLCh OrderComparison::name[]={ 'O', 'r', 'd', 'e', 'r', 'C', 'o', 'm', 'p', 'a', 'r', 'i', 's', 'o', 'n', 0 };

OrderComparison::OrderComparison(const VectorOfASTNodes &args, bool testBefore, XPath2MemoryManager* memMgr)
  : XQOperator(name, args, memMgr)
{
	_testBefore = testBefore;
}

bool OrderComparison::getTestBefore() const
{
    return _testBefore;
}

ASTNode* OrderComparison::staticResolution(StaticContext *context)
{
  XPath2MemoryManager *mm = context->getMemoryManager();

  for(VectorOfASTNodes::iterator i = _args.begin(); i != _args.end(); ++i) {
    SequenceType *seqType = new (mm) SequenceType(new (mm) SequenceType::ItemType(SequenceType::ItemType::TEST_NODE),
                                                  SequenceType::QUESTION_MARK);
    seqType->setLocationInfo(this);

    *i = new (mm) XQTreatAs(*i, seqType, mm);
    (*i)->setLocationInfo(this);

    *i = (*i)->staticResolution(context);
  }

  return this;
}

ASTNode *OrderComparison::staticTypingImpl(StaticContext *context)
{
  _src.clear();

  _src.getStaticType() = StaticType(StaticType::BOOLEAN_TYPE, 0, 1);

  for(VectorOfASTNodes::iterator i = _args.begin(); i != _args.end(); ++i) {
    _src.add((*i)->getStaticAnalysis());

    if((*i)->getStaticAnalysis().isUpdating()) {
      XQThrow(StaticErrorException,X("OrderComparison::staticTyping"),
              X("It is a static error for an operand of an operator "
                "to be an updating expression [err:XUST0001]"));
    }
  }

  return this;
}

BoolResult OrderComparison::boolResult(DynamicContext* context) const
{
  Item::Ptr arg1 = getArgument(0)->createResult(context)->next(context);
  if(arg1.isNull()) return BoolResult::Null;
  Item::Ptr arg2 = getArgument(1)->createResult(context)->next(context);
  if(arg2.isNull()) return BoolResult::Null;
  
	if(getTestBefore())
    return ((Node*)arg1.get())->lessThan((Node*)arg2.get(), context);
  return ((Node*)arg2.get())->lessThan((Node*)arg1.get(), context);
}

Result OrderComparison::createResult(DynamicContext* context, int flags) const
{
  return (Item::Ptr)context->getItemFactory()->createBoolean(boolResult(context), context);
}
