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

#include <xqilla/ast/XQIf.hpp>
#include <xqilla/ast/XQEffectiveBooleanValue.hpp>
#include <xqilla/runtime/Sequence.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/ast/StaticAnalysis.hpp>
#include <xqilla/context/ContextHelpers.hpp>
#include <xqilla/update/PendingUpdateList.hpp>
#include <xqilla/exceptions/StaticErrorException.hpp>
#include <xqilla/runtime/ClosureResult.hpp>

XQIf::XQIf(ASTNode* test, ASTNode* whenTrue, ASTNode* whenFalse, XPath2MemoryManager* memMgr)
  : ASTNodeImpl(IF, memMgr),
  _test(test),
  _whenTrue(whenTrue),
  _whenFalse(whenFalse)
{
}

EventGenerator::Ptr XQIf::generateEvents(EventHandler *events, DynamicContext *context,
                                    bool preserveNS, bool preserveType) const
{
  if(_test->boolResult(context))
    return new ClosureEventGenerator(_whenTrue, context, preserveNS, preserveType);
  else return new ClosureEventGenerator(_whenFalse, context, preserveNS, preserveType);
}

ASTNode* XQIf::staticResolution(StaticContext *context)
{
  XPath2MemoryManager *mm = context->getMemoryManager();

  _test = new (mm) XQEffectiveBooleanValue(_test, mm);
  _test->setLocationInfo(this);
  _test = _test->staticResolution(context);

  _whenTrue = _whenTrue->staticResolution(context);
  _whenFalse = _whenFalse->staticResolution(context);

  return this;
}

ASTNode *XQIf::staticTypingImpl(StaticContext *context)
{
  _src.clear();

  if(_test->getStaticAnalysis().isUpdating()) {
    XQThrow(StaticErrorException,X("XQIf::staticTyping"),
            X("It is a static error for the conditional expression of an if expression "
              "to be an updating expression [err:XUST0001]"));
  }

  _src.add(_test->getStaticAnalysis());

  _src.getStaticType() = _whenTrue->getStaticAnalysis().getStaticType();
  _src.setProperties(_whenTrue->getStaticAnalysis().getProperties());
  _src.add(_whenTrue->getStaticAnalysis());

  if(_src.isUpdating()) {
    if(!_whenFalse->getStaticAnalysis().isUpdating() &&
       !_whenFalse->getStaticAnalysis().isPossiblyUpdating())
      XQThrow(StaticErrorException, X("XQIf::staticTyping"),
              X("Mixed updating and non-updating operands [err:XUST0001]"));
  }
  else {
    if(_whenFalse->getStaticAnalysis().isUpdating() &&
       !_whenTrue->getStaticAnalysis().isPossiblyUpdating())
      XQThrow(StaticErrorException, X("XQIf::staticTyping"),
              X("Mixed updating and non-updating operands [err:XUST0001]"));
  }

  _src.getStaticType() |= _whenFalse->getStaticAnalysis().getStaticType();
  _src.setProperties(_src.getProperties() & _whenFalse->getStaticAnalysis().getProperties());
  _src.add(_whenFalse->getStaticAnalysis());

  return this;
}

ASTNode *XQIf::getTest() const {
  return _test;
}

ASTNode *XQIf::getWhenTrue() const {
  return _whenTrue;
}

ASTNode *XQIf::getWhenFalse() const {
  return _whenFalse;
}

void XQIf::setTest(ASTNode *item)
{
  _test = item;
}

void XQIf::setWhenTrue(ASTNode *item)
{
  _whenTrue = item;
}

void XQIf::setWhenFalse(ASTNode *item)
{
  _whenFalse = item;
}

PendingUpdateList XQIf::createUpdateList(DynamicContext *context) const
{
  if(_test->boolResult(context))
    return _whenTrue->createUpdateList(context);
  else return _whenFalse->createUpdateList(context);
}

Result XQIf::createResult(DynamicContext* context, int flags) const
{
  if(_test->boolResult(context))
    return getWhenTrue()->createResult(context);
  return getWhenFalse()->createResult(context);
}

