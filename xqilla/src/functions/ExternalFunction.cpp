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
#include <xqilla/functions/ExternalFunction.hpp>
#include <xqilla/framework/XPath2MemoryManager.hpp>
#include <xqilla/runtime/Result.hpp>
#include <xqilla/update/PendingUpdateList.hpp>

XERCES_CPP_NAMESPACE_USE;

ExternalFunction::ExternalFunction(const XMLCh *uri, const XMLCh *name, unsigned int numberOfArgs, XPath2MemoryManager *mm)
  : uri_(mm->getPooledString(uri)),
    name_(mm->getPooledString(name)),
    uriName_(0),
    numArgs_(numberOfArgs)
{
  XMLBuffer buf;
  buf.set(name);
  buf.append(uri);
  uriName_ = mm->getPooledString(buf.getRawBuffer());
}

Result ExternalFunction::execute(const Arguments *args, DynamicContext *context) const
{
  return 0;
}

PendingUpdateList ExternalFunction::executeUpdate(const Arguments *args, DynamicContext *context) const
{
  return PendingUpdateList();
}

