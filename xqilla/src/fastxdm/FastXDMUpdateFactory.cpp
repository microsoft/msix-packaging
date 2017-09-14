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

#include "FastXDMUpdateFactory.hpp"
#include <xqilla/schema/DocumentCacheImpl.hpp>

#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/context/MessageListener.hpp>
#include <xqilla/exceptions/ASTException.hpp>
#include <xqilla/update/PendingUpdateList.hpp>
#include <xqilla/dom-api/impl/XQillaNSResolverImpl.hpp>

#include <xercesc/framework/LocalFileFormatTarget.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/framework/XMLBuffer.hpp>

#if defined(XERCES_HAS_CPP_NAMESPACE)
XERCES_CPP_NAMESPACE_USE
#endif

static const XMLCh file_scheme[] = { chLatin_f, chLatin_i, chLatin_l, chLatin_e, 0 };
static const XMLCh utf8_str[] = { chLatin_u, chLatin_t, chLatin_f, chDash, chDigit_8, 0 };

void FastXDMUpdateFactory::applyPut(const PendingUpdate &update, DynamicContext *context)
{
  assert(false);
}

void FastXDMUpdateFactory::applyInsertInto(const PendingUpdate &update, DynamicContext *context)
{
  assert(false);
}

void FastXDMUpdateFactory::applyInsertAttributes(const PendingUpdate &update, DynamicContext *context)
{
  assert(false);
}

void FastXDMUpdateFactory::applyReplaceValue(const PendingUpdate &update, DynamicContext *context)
{
  assert(false);
}

void FastXDMUpdateFactory::applyRename(const PendingUpdate &update, DynamicContext *context)
{
  assert(false);
}

void FastXDMUpdateFactory::applyDelete(const PendingUpdate &update, DynamicContext *context)
{
  assert(false);
}

void FastXDMUpdateFactory::applyInsertBefore(const PendingUpdate &update, DynamicContext *context)
{
  assert(false);
}

void FastXDMUpdateFactory::applyInsertAfter(const PendingUpdate &update, DynamicContext *context)
{
  assert(false);
}

void FastXDMUpdateFactory::applyInsertAsFirst(const PendingUpdate &update, DynamicContext *context)
{
  assert(false);
}

void FastXDMUpdateFactory::applyInsertAsLast(const PendingUpdate &update, DynamicContext *context)
{
  assert(false);
}

void FastXDMUpdateFactory::applyReplaceNode(const PendingUpdate &update, DynamicContext *context)
{
  assert(false);
}

void FastXDMUpdateFactory::applyReplaceAttribute(const PendingUpdate &update, DynamicContext *context)
{
  assert(false);
}

void FastXDMUpdateFactory::applyReplaceElementContent(const PendingUpdate &update, DynamicContext *context)
{
  assert(false);
}

void FastXDMUpdateFactory::completeDeletions(DynamicContext *context)
{
  assert(false);
}

void FastXDMUpdateFactory::completeRevalidation(DynamicContext *context)
{
  assert(false);
}

void FastXDMUpdateFactory::completeUpdate(DynamicContext *context)
{
  assert(false);
}

