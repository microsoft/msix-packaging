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
#include <xqilla/functions/FunctionTime.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/exceptions/StaticErrorException.hpp>
#include <xqilla/ast/StaticAnalysis.hpp>
#include <xqilla/context/MessageListener.hpp>
#include <xqilla/update/PendingUpdateList.hpp>

#include <xercesc/util/PlatformUtils.hpp>

XERCES_CPP_NAMESPACE_USE;

const XMLCh FunctionTime::name[] = {
  chLatin_t, chLatin_i, chLatin_m, chLatin_e, chNull
};
const unsigned int FunctionTime::minArgs = 2;
const unsigned int FunctionTime::maxArgs = 2;

/**
 * xqilla:time($value as item()*, $label as xs:string) as item()*
 */
FunctionTime::FunctionTime(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
  : XQillaFunction(name, "($value as item()*, $label as xs:string) as item()*", args, memMgr)
{
}

ASTNode *FunctionTime::staticTypingImpl(StaticContext *context)
{
  _src.clear();

  _src.copy(_args[0]->getStaticAnalysis());
  _src.add(_args[1]->getStaticAnalysis());

  if(_args[1]->getStaticAnalysis().isUpdating()) {
    XQThrow(StaticErrorException,X("XQFunction::staticTyping"),
            X("It is a static error for an argument to a function "
              "to be an updating expression [err:XUST0001]"));
  }

  _src.forceNoFolding(true);
  return this;
}

class Timer
{
public:
  Timer(const XMLCh *label, DynamicContext *context, const LocationInfo *location, bool startNow = true)
    : label_(label),
      context_(context),
      location_(location),
      startTime_(0),
      totalTime_(0)
  {
    if(startNow) start();
  }

  ~Timer()
  {
    end();

    if(context_->getMessageListener() != 0) {
      Sequence value(context_->getItemFactory()->createDayTimeDuration(((double)totalTime_) / 1000, context_),
                     context_->getMemoryManager());
      context_->getMessageListener()->trace(label_, value, location_, context_);
    }
  }

  void start()
  {
    if(startTime_ == 0)
      startTime_ = XMLPlatformUtils::getCurrentMillis();
  }

  void end()
  {
    if(startTime_ != 0)
      totalTime_ += XMLPlatformUtils::getCurrentMillis() - startTime_;
    startTime_ = 0;
  }

private:
  const XMLCh *label_;
  DynamicContext *context_;
  const LocationInfo *location_;
  unsigned long startTime_;
  unsigned long totalTime_;
};

class TimerGuard
{
public:
  TimerGuard(Timer *timer)
    : timer_(timer)
  {
    timer_->start();
  }

  ~TimerGuard()
  {
    timer_->end();
  }

private:
  Timer *timer_;
};

class FunctionTimeResult : public ResultImpl
{
public:
  FunctionTimeResult(const FunctionTime *ast, DynamicContext *context)
    : ResultImpl(ast),
      timer_(new Timer(ast->getArguments()[1]->createResult(context)->
          next(context)->asString(context), context, ast)),
      result_(0)
  {
    TimerGuard tGuard(timer_);
    result_ = ast->getArguments()[0]->createResult(context);
  }

  virtual Item::Ptr next(DynamicContext *context)
  {
    TimerGuard tGuard(timer_);
    return result_->next(context);
  }

  virtual std::string asString(DynamicContext *context, int indent) const { return ""; }

private:
  AutoDelete<Timer> timer_;
  Result result_;
};

Result FunctionTime::createResult(DynamicContext* context, int flags) const
{
  return new FunctionTimeResult(this, context);
}

EventGenerator::Ptr FunctionTime::generateEvents(EventHandler *events, DynamicContext *context,
                                            bool preserveNS, bool preserveType) const
{
  Timer timer(getParamNumber(2, context)->next(context)->asString(context),
              context, this);

  _args[0]->generateAndTailCall(events, context, preserveNS, preserveType);
  return 0;
}

PendingUpdateList FunctionTime::createUpdateList(DynamicContext *context) const
{
  Timer timer(getParamNumber(2, context)->next(context)->asString(context),
              context, this);

  return _args[0]->createUpdateList(context);
}
