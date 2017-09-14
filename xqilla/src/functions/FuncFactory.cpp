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
#include <xqilla/functions/FuncFactory.hpp>

XERCES_CPP_NAMESPACE_USE;

FuncFactory::FuncFactory(const XMLCh *uri, const XMLCh *name, size_t minArgs, size_t maxArgs,
                         MemoryManager *mm)
  : uri_(uri),
    name_(name),
    minArgs_(minArgs),
    maxArgs_(maxArgs),
    uriname_(1023, mm)
{
  setURINameHash(uri, name);
}

FuncFactory::FuncFactory(size_t numArgs, MemoryManager *mm)
  : uri_(0),
    name_(0),
    minArgs_(numArgs),
    maxArgs_(numArgs),
    uriname_(1023, mm)
{
}

void FuncFactory::setURINameHash(const XMLCh *uri, const XMLCh *name)
{
  uri_ = uri;
  name_ = name;

  uriname_.set(name);
  uriname_.append(':');
  uriname_.append(uri);
}
