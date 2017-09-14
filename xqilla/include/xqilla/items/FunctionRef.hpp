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

#ifndef _XQILLAFUNCTIONREF_HPP
#define _XQILLAFUNCTIONREF_HPP

#include <vector>

#include <xqilla/items/Item.hpp>
#include <xqilla/items/ATQNameOrDerived.hpp>
#include <xqilla/runtime/Result.hpp>

class DynamicContext;
class LocationInfo;
class FunctionSignature;

typedef std::vector<Result> VectorOfResults;

class XQILLA_API FunctionRef : public Item
{
public:
  typedef RefCountPointer<const FunctionRef> Ptr;

  virtual bool isNode() const { return false; }
  virtual bool isAtomicValue() const { return false; }
  virtual bool isFunction() const { return true; }

  virtual const XMLCh *getTypeURI() const { return 0; }
  virtual const XMLCh *getTypeName() const { return 0; }

  virtual ATQNameOrDerived::Ptr getName(const DynamicContext *context) const = 0;
  virtual size_t getNumArgs() const = 0;
  virtual const FunctionSignature *getSignature() const = 0;
  virtual Result execute(const VectorOfResults &args, DynamicContext *context, const LocationInfo *location) const = 0;
  virtual FunctionRef::Ptr partialApply(const Result &arg, unsigned int argNum, DynamicContext *context, const LocationInfo *location) const = 0;

protected:
  FunctionRef() {}
};
#endif
