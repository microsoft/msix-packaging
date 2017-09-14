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

#ifndef COLLATION_HPP
#define COLLATION_HPP

#include <xqilla/framework/XQillaExport.hpp>
#include <xercesc/util/XercesDefs.hpp>

class Sequence;
class DynamicContext;

class XQILLA_API Collation
{
public:
  virtual ~Collation() {};

  virtual const XMLCh* getCollationName() const = 0;
  virtual Sequence sort(Sequence data, const DynamicContext* context) const = 0;
  virtual int compare(const XMLCh* const string1, const XMLCh* const string2) const = 0;

protected:
  Collation() {}
};

#endif
