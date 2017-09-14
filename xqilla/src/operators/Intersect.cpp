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
#include <xqilla/operators/Intersect.hpp>
#include <xqilla/exceptions/XPath2ErrorException.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/items/Node.hpp>
#include <xqilla/ast/XQDocumentOrder.hpp>
#include <xqilla/ast/XQTreatAs.hpp>
#include <xqilla/schema/SequenceType.hpp>
#include <xqilla/exceptions/StaticErrorException.hpp>
#include <xqilla/runtime/Sequence.hpp>

XERCES_CPP_NAMESPACE_USE;

const XMLCh Intersect::name[]={ chLatin_i, chLatin_n, chLatin_t, chLatin_e, chLatin_r, chLatin_s, chLatin_e, chLatin_c, chLatin_t, chNull };

Intersect::Intersect(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
  : XQOperator(name, args, memMgr),
    sortAdded_(false)
{
}

ASTNode* Intersect::staticResolution(StaticContext *context)
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

ASTNode *Intersect::staticTypingImpl(StaticContext *context)
{
  _src.clear();

  _src.copy(_args[0]->getStaticAnalysis());

  if(_args[0]->getStaticAnalysis().isUpdating()) {
    XQThrow(StaticErrorException,X("Intersect::staticTyping"),
            X("It is a static error for an operand of an operator "
              "to be an updating expression [err:XUST0001]"));
  }

  _src.add(_args[1]->getStaticAnalysis());

  _src.getStaticType().typeNodeIntersect(_args[1]->getStaticAnalysis().getStaticType());
  _src.getStaticType().multiply(0, 1);

  if(_args[1]->getStaticAnalysis().isUpdating()) {
    XQThrow(StaticErrorException,X("Intersect::staticTyping"),
            X("It is a static error for an operand of an operator "
              "to be an updating expression [err:XUST0001]"));
  }

  return this;
}

Result Intersect::createResult(DynamicContext* context, int flags) const
{
  // TBD Improve this implementation - jpcs
  Sequence param1 = getArgument(0)->createResult(context)->toSequence(context);
  Sequence param2 = getArgument(1)->createResult(context)->toSequence(context);

  XPath2MemoryManager* memMgr = context->getMemoryManager();
  Sequence result(param1.getLength(),memMgr);

  Sequence::const_iterator p1It = param1.begin();
  Sequence::const_iterator p2It;
  Sequence::const_iterator end1 = param1.end();
  Sequence::const_iterator end2 = param2.end();

  for(;p1It != end1; ++p1It) {
    for(p2It = param2.begin();p2It != end2; ++p2It) {
      if(((Node*)p1It->get())->equals((Node*)p2It->get())) {
        result.addItem(*p1It);
      }
    }
  }

  return result;
}
