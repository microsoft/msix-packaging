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

#include <xqilla/operators/Except.hpp>
#include <xqilla/exceptions/XPath2ErrorException.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/items/Node.hpp>
#include <xqilla/ast/XQDocumentOrder.hpp>
#include <xqilla/ast/XQTreatAs.hpp>
#include <xqilla/schema/SequenceType.hpp>
#include <xqilla/exceptions/StaticErrorException.hpp>

/*static*/ const XMLCh Except::name[]={ XERCES_CPP_NAMESPACE_QUALIFIER chLatin_E, XERCES_CPP_NAMESPACE_QUALIFIER chLatin_x, XERCES_CPP_NAMESPACE_QUALIFIER chLatin_c, XERCES_CPP_NAMESPACE_QUALIFIER chLatin_e, XERCES_CPP_NAMESPACE_QUALIFIER chLatin_p, XERCES_CPP_NAMESPACE_QUALIFIER chLatin_t, XERCES_CPP_NAMESPACE_QUALIFIER chNull };

Except::Except(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
  : XQOperator(name, args, memMgr),
    sortAdded_(false)
{
}

ASTNode* Except::staticResolution(StaticContext *context)
{
  XPath2MemoryManager *mm = context->getMemoryManager();

  if(!sortAdded_) {
    sortAdded_ = true;
    // Wrap ourselves in a document order sort
    ASTNode *result = new (mm) XQDocumentOrder(this, mm);
    result->setLocationInfo(this);
    return result->staticResolution(context);
  }

  for(VectorOfASTNodes::iterator i = _args.begin(); i != _args.end(); ++i) {
    SequenceType *seqType = new (mm) SequenceType(new (mm) SequenceType::ItemType(SequenceType::ItemType::TEST_NODE),
                                                  SequenceType::STAR);
    seqType->setLocationInfo(this);

    *i = new (mm) XQTreatAs(*i, seqType, mm);
    (*i)->setLocationInfo(this);

    *i = (*i)->staticResolution(context);
  }

  return this;
}

ASTNode *Except::staticTypingImpl(StaticContext *context)
{
  _src.clear();

  _src.copy(_args[0]->getStaticAnalysis());

  if(_args[0]->getStaticAnalysis().isUpdating()) {
    XQThrow(StaticErrorException,X("Except::staticTyping"),
            X("It is a static error for an operand of an operator "
              "to be an updating expression [err:XUST0001]"));
  }

  _src.add(_args[1]->getStaticAnalysis());

  unsigned int min = 0;
  if(_src.getStaticType().getMin() > _args[1]->getStaticAnalysis().getStaticType().getMax())
    min = _src.getStaticType().getMin() - _args[1]->getStaticAnalysis().getStaticType().getMax();

  _src.getStaticType().setCardinality(min, _src.getStaticType().getMax());

  if(_args[1]->getStaticAnalysis().isUpdating()) {
    XQThrow(StaticErrorException,X("Except::staticTyping"),
            X("It is a static error for an operand of an operator "
              "to be an updating expression [err:XUST0001]"));
  }

  return this;
}

class ExceptResult : public ResultImpl
{
public:
  ExceptResult(const Except *op, DynamicContext *context)
    : ResultImpl(op),
      _result(op->getArgument(0)->createResult(context)),
      _excpt(op->getArgument(1)->createResult(context))
  {
  }

  Item::Ptr next(DynamicContext *context)
  {
    Item::Ptr item = _result->next(context);
    while(item.notNull()) {    
      bool found = false;
      Result except_result(_excpt.createResult());
      Item::Ptr except_item;
      while((except_item = except_result->next(context)).notNull()) {
        if(((Node*)item.get())->equals((Node*)except_item.get())) {
          found = true;
          break;
        }
      }

      if(!found) break;

      item = _result->next(context);
    }

    if(item.isNull()) {
      _result = 0;
      _excpt = 0;
    }

    return item;
  }

private:
  Result _result;
  ResultBuffer _excpt;
};

Result Except::createResult(DynamicContext* context, int flags) const
{
  return new ExceptResult(this, context);
}

