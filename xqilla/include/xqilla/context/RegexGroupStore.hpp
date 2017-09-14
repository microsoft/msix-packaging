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

#ifndef _REGEXGROUPSTORE_HPP
#define _REGEXGROUPSTORE_HPP

#include <xqilla/framework/XQillaExport.hpp>

#include <xercesc/util/XercesDefs.hpp>

class Result;

/** The pure virtual base class for accessing regular expression group values at runtime. */
class XQILLA_API RegexGroupStore
{
public:
  /** default destructor */
  virtual ~RegexGroupStore() {};

  /** Looks up the value of a variable by namespace URI and localname pair. */
  virtual const XMLCh *getGroup(int index) const = 0;
};

#endif
