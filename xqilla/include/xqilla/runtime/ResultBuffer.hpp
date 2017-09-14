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

#ifndef _RESULTBUFFER_HPP
#define _RESULTBUFFER_HPP

#include <xqilla/framework/XQillaExport.hpp>
#include <xqilla/runtime/ResultBufferImpl.hpp>

/** A reference counting wrapper for the result buffer */
class XQILLA_API ResultBuffer
{
public:
  ResultBuffer(const Result &result, unsigned int readCount = ResultBufferImpl::UNLIMITED_COUNT);
  ResultBuffer(const Item::Ptr &item, unsigned int readCount = ResultBufferImpl::UNLIMITED_COUNT);
  ResultBuffer(ResultBufferImpl *impl, unsigned start = 0);

  /// Creates a result that returns the same items as the one used to construct this buffer
  Result createResult();

  bool isNull() const
  {
    return _impl.isNull();
  }

private:
  ResultBufferImpl::Ptr _impl;
  unsigned _start;
};

#endif
