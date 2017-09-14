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

#ifndef _MODULERESOLVER_HPP
#define _MODULERESOLVER_HPP

#include <xqilla/framework/XQillaExport.hpp>

class StaticContext;

class XQILLA_API ModuleResolver {
public:
  /** virtual destructor, does nothing */
  virtual ~ModuleResolver() {};      

  /** Fills the string vector with a list of locations tht, once assembled, build the module referenced by the given URI.
     If the URI is unknown, returns false, otherwise returns true. */
  virtual bool resolveModuleLocation(VectorOfStrings* result, const XMLCh* nsUri, const StaticContext* context) = 0;
};
#endif
