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
#include <sstream>

#include <xqilla/operators/Union.hpp>

#include <xqilla/runtime/Sequence.hpp>
#include <xqilla/items/Node.hpp>
#include <xqilla/exceptions/XPath2ErrorException.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/ast/XQDocumentOrder.hpp>
#include <xqilla/ast/XQTreatAs.hpp>
#include <xqilla/schema/SequenceType.hpp>
#include <xqilla/exceptions/StaticErrorException.hpp>
#include <xqilla/runtime/ClosureResult.hpp>

/*static*/ const XMLCh Union::name[]={ XERCES_CPP_NAMESPACE_QUALIFIER chLatin_U, XERCES_CPP_NAMESPACE_QUALIFIER chLatin_n, XERCES_CPP_NAMESPACE_QUALIFIER chLatin_i, XERCES_CPP_NAMESPACE_QUALIFIER chLatin_o, XERCES_CPP_NAMESPACE_QUALIFIER chLatin_n, XERCES_CPP_NAMESPACE_QUALIFIER chNull };

Union::Union(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
  : XQOperator(name, args, memMgr),
    sortAdded_(false)
{
}

ASTNode* Union::staticResolution(StaticContext *context)
{
  XPath2MemoryManager *mm = context->getMemoryManager();

  if(!sortAdded_) {
    sortAdded_ = true;
    // Wrap ourselves in a document order sort
    ASTNode *result = new (mm) XQDocumentOrder(this, mm);
    return result->staticResolution(context);
  }

  for(VectorOfASTNodes::iterator i = _args.begin(); i != _args.end(); ++i) {
    SequenceType *seqType = new (mm) SequenceType(new (mm) SequenceType::ItemType(SequenceType::ItemType::TEST_NODE),
                                                  SequenceType::STAR);
    seqType->setLocationInfo(this);

    *i = new (mm) XQTreatAs(*i, seqType, mm);
    (*i)->setLocationInfo(*i);

    *i = (*i)->staticResolution(context);
  }

  return this;
}

ASTNode *Union::staticTypingImpl(StaticContext *context)
{
  _src.clear();

  _src.add(_args[0]->getStaticAnalysis());
  _src.getStaticType() = _args[0]->getStaticAnalysis().getStaticType();

  if(_args[0]->getStaticAnalysis().isUpdating()) {
    XQThrow(StaticErrorException,X("Union::staticTyping"),
            X("It is a static error for an operand of an operator "
              "to be an updating expression [err:XUST0001]"));
  }

  _src.add(_args[1]->getStaticAnalysis());
  _src.getStaticType().typeConcat(_args[1]->getStaticAnalysis().getStaticType());

  unsigned int min = _args[0]->getStaticAnalysis().getStaticType().getMin();
  if(min > _args[1]->getStaticAnalysis().getStaticType().getMin())
    min = _args[1]->getStaticAnalysis().getStaticType().getMin();

  _src.getStaticType().setCardinality(min, _src.getStaticType().getMax());

  if(_args[1]->getStaticAnalysis().isUpdating()) {
    XQThrow(StaticErrorException,X("Union::staticTyping"),
            X("It is a static error for an operand of an operator "
              "to be an updating expression [err:XUST0001]"));
  }

  return this;
}

Result Union::createResult(DynamicContext* context, int flags) const
{
  return ClosureResult::create(getStaticAnalysis(), context, new UnionResult(this, flags));
}

Union::UnionResult::UnionResult(const Union *op, int flags)
  : ResultImpl(op),
    _op(op),
    _flags(flags),
    _index(0),
    _result(0)
{
}

Item::Ptr Union::UnionResult::next(DynamicContext *context)
{
  Item::Ptr item = 0;
  while(true) {
    item = _result->next(context);

    if(item == NULLRCP) {
      if(_index > 1) {
        _index = 0;
        return 0;
      }
      else {
        _result = _op->getArgument(_index++)->createResult(context, _flags);
      }
    }
    else {
      return item;
    }
  }

  return 0;
}

