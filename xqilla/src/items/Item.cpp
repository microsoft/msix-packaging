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
#include <xqilla/items/Item.hpp>
#include <xqilla/framework/XQillaExport.hpp>
#include <xqilla/framework/XPath2MemoryManager.hpp>
#include <xercesc/util/XMLUniDefs.hpp>

const XMLCh Item::gXQilla[] =   // Points to "XQilla"
{
  XERCES_CPP_NAMESPACE_QUALIFIER chLatin_X,
  XERCES_CPP_NAMESPACE_QUALIFIER chLatin_Q,
  XERCES_CPP_NAMESPACE_QUALIFIER chLatin_i,
  XERCES_CPP_NAMESPACE_QUALIFIER chLatin_l,
  XERCES_CPP_NAMESPACE_QUALIFIER chLatin_l,
  XERCES_CPP_NAMESPACE_QUALIFIER chLatin_a,
  XERCES_CPP_NAMESPACE_QUALIFIER chNull
};
