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

#ifndef _RESULTIMPL_HPP
#define _RESULTIMPL_HPP

#include <xqilla/framework/XQillaExport.hpp>
#include <xqilla/items/Item.hpp>
#include <xqilla/ast/LocationInfo.hpp>

class DynamicContext;
class Sequence;
class Result;
class ResultBuffer;

/** A lazily evaluated query result */
class XQILLA_API ResultImpl : public LocationInfo
{
public:
  virtual ~ResultImpl() {}

  Result *getResultPointer() { return resultPointer_; }
  void setResultPointer(Result *p) { resultPointer_ = p; }

  /// Get the next item from the iterator. Returns null if the is no next value.
  virtual Item::Ptr next(DynamicContext *context);

  /// Gets the next item, or returns a tail call Result which will give the next item.
  virtual Item::Ptr nextOrTail(Result &tail, DynamicContext *context);

  /// Cast operator to a Sequence
  virtual Sequence toSequence(DynamicContext *context);

  /// Returns a result buffer of the result
  virtual void toResultBuffer(unsigned int readCount, ResultBuffer &buffer);

  /// Skip ahead
  virtual void skip(unsigned count, DynamicContext *context);

protected:
  ResultImpl(const LocationInfo *o);

  Result *resultPointer_;

private:
  ResultImpl(const ResultImpl &) {};
  ResultImpl &operator=(const ResultImpl &) { return *this; };
};

#endif
