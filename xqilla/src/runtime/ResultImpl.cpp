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

#include <xqilla/runtime/ResultImpl.hpp>
#include <xqilla/runtime/SequenceResult.hpp>
#include <xqilla/runtime/ResultBuffer.hpp>
#include <xqilla/runtime/ResultBufferImpl.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/items/Numeric.hpp>
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/exceptions/XPath2TypeMatchException.hpp>

ResultImpl::ResultImpl(const LocationInfo *o)
  : resultPointer_(0)
{
  setLocationInfo(o);
}

Item::Ptr ResultImpl::next(DynamicContext *context)
{
  // Store resultPointer_ locally, as "this" may get deleted
  Result &resultPointer = *resultPointer_;

  Item::Ptr item;
  do {
    item = resultPointer->nextOrTail(resultPointer, context);
  } while(item.isNull() && !resultPointer.isNull());
  
  return item;
}

Item::Ptr ResultImpl::nextOrTail(Result &tail, DynamicContext *context)
{
  Item::Ptr item = next(context);
  if(item.isNull()) {
    tail = 0;
  }
  return item;
}

Sequence ResultImpl::toSequence(DynamicContext *context)
{
  // Control our own scoped pointer
  Result me(this);

  Sequence result(context->getMemoryManager());

  Item::Ptr item = 0;
  while((item = me->next(context)).notNull()) {
    result.addItem(item);
  }

  return result;
}

void ResultImpl::toResultBuffer(unsigned int readCount, ResultBuffer &buffer)
{
  // Control our own scoped pointer
  Result me(this);
  buffer = ResultBuffer(new ResultBufferImpl(me, readCount));
}

void ResultImpl::skip(unsigned count, DynamicContext *context)
{
  // Store resultPointer_ locally, as "this" may get deleted
  Result &resultPointer = *resultPointer_;

  Item::Ptr item;
  while(count > 0) {
    do {
      if(resultPointer.isNull()) return;
      item = resultPointer->nextOrTail(resultPointer, context);
    } while(item.isNull());
    --count;
  }
}
