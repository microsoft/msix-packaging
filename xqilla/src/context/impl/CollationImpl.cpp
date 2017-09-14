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

#include "../../config/xqilla_config.h"
#include <xqilla/context/impl/CollationImpl.hpp>
#include <xqilla/items/ATStringOrDerived.hpp>
#include <xqilla/items/Item.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/items/DatatypeFactory.hpp>
#include <xqilla/context/ItemFactory.hpp>

CollationImpl::CollationImpl(XPath2MemoryManager* memMgr, CollationHelper* helper)
{
  _memMgr=memMgr;
    _helper=helper;
}

const XMLCh* CollationImpl::getCollationName() const
{
    return _helper->getCollationName();
}

int CollationImpl::compare(const XMLCh* const string1, const XMLCh* const string2) const
{
    return _helper->compare(string1,string2);
}
    
Sequence CollationImpl::sort(Sequence data, const DynamicContext* context) const
{
  // build a sequence made of strings
  Sequence stringSeq = Sequence(data.getLength(), context->getMemoryManager());
  for(Sequence::iterator it=data.begin(); it!=data.end(); ++it) {
    const XMLCh *str = (*it)->asString(context);
    stringSeq.addItem(context->getItemFactory()->createString(str, context).get());
  }

  stringSeq.sortWithCollation(this, context);

  return stringSeq;
}
