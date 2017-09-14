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

#ifndef _RESULT_HPP
#define _RESULT_HPP

#include <string>
#include <xercesc/util/XercesDefs.hpp>

#include <xqilla/framework/XQillaExport.hpp>
#include <xqilla/items/Item.hpp>
#include <xqilla/runtime/ResultImpl.hpp>
#include <xqilla/runtime/EmptyResult.hpp>

class Sequence;
class SequenceType;
class DynamicContext;
class StaticType;

/** A scoped pointer wrapper for the lazily evaluated query result */
class XQILLA_API Result
{
public:
  Result(const Item::Ptr &item);
  Result(const Sequence &seq);
  Result(ResultImpl *impl);
  Result(const Result &o);
  Result &operator=(const Result &o);
  ~Result();

  /// Returns the underlying ResultImpl object
  ResultImpl *operator->();

  /// Returns the underlying ResultImpl object
  const ResultImpl *operator->() const;

  /// Returns the underlying ResultImpl object
  ResultImpl *get();

  /// Returns the underlying ResultImpl object
  const ResultImpl *get() const;

  /// Returns true if the underlying pointer is null
  bool isNull() const;

  EmptyResult* getEmpty() const;
  
private:
  ResultImpl *_impl;

  static EmptyResult _empty;
};

inline bool Result::isNull() const
{
  return _impl == 0;
}

inline ResultImpl *Result::get()
{
  if(_impl) return _impl;
  return getEmpty();
}

inline const ResultImpl *Result::get() const
{
  if(_impl) return _impl;
  return getEmpty();
}

inline ResultImpl *Result::operator->()
{
  return get();
}

inline const ResultImpl *Result::operator->() const
{
  return get();
}

#endif
