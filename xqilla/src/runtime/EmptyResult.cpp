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
#include <xqilla/runtime/EmptyResult.hpp>
#include <xqilla/runtime/Sequence.hpp>
#include <xqilla/runtime/ResultBufferImpl.hpp>
#include <xqilla/context/DynamicContext.hpp>

EmptyResult::EmptyResult(const LocationInfo *o)
  : ResultImpl(o)
{
}

Item::Ptr EmptyResult::next(DynamicContext *context)
{
  return 0;
}

Sequence EmptyResult::toSequence(DynamicContext *context)
{
  return Sequence(context->getMemoryManager());
}

void EmptyResult::toResultBuffer(unsigned int readCount, ResultBuffer &buffer)
{
}

void EmptyResult::skip(unsigned count, DynamicContext *context)
{
}
