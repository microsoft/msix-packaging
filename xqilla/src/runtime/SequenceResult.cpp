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
#include <xqilla/runtime/SequenceResult.hpp>
#include <xqilla/runtime/Result.hpp>
#include <xqilla/context/DynamicContext.hpp>

SequenceResult::SequenceResult(const LocationInfo *o, const Sequence &seq)
  : ResultImpl(o),
    seq_(seq),
    i_(0)
{
}

Item::Ptr SequenceResult::nextOrTail(Result &tail, DynamicContext *context)
{
  if(i_ >= seq_._itemList.size()) {
    tail = 0;
    return 0;
  }
  return seq_._itemList[i_++];
}

Sequence SequenceResult::toSequence(DynamicContext *context)
{
  return Sequence(seq_, i_);
}

void SequenceResult::skip(unsigned count, DynamicContext *context)
{
  i_ += count;
}

