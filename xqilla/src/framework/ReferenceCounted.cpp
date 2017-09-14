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
#include <xqilla/framework/ReferenceCounted.hpp>
#include <xqilla/framework/XPath2MemoryManager.hpp>

// ReferenceCounted::ReferenceCounted(XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager *mm)
//   : _ref_count(0),
//     _ref_mm(mm)
// {
//   // Do nothing
// }

// ReferenceCounted::~ReferenceCounted()
// {
//   // Do nothing
// }

// void ReferenceCounted::incrementRefCount() const
// {
//   ++const_cast<unsigned int&>(_ref_count);
// }

// void ReferenceCounted::decrementRefCount() const
// {
//   if(--const_cast<unsigned int&>(_ref_count) == 0) {
//     if(_ref_mm == 0) {
//       delete this;
//     }
//     else {
//       this->~ReferenceCounted();
//       operator delete((void*)this, _ref_mm);
//     }
//   }
// }
