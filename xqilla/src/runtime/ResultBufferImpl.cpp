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
#include <xqilla/runtime/ResultBufferImpl.hpp>
#include <xqilla/runtime/ResultBuffer.hpp>

const unsigned int ResultBufferImpl::UNLIMITED_COUNT = (unsigned int)-1;

ResultBufferImpl::ResultBufferImpl(const Result &result, unsigned int readCount)
  : _result(result),
    _readCount(0),
    _maxReadCount(readCount)
{
  // Do nothing
}

ResultBufferImpl::ResultBufferImpl(const Item::Ptr &item, unsigned int readCount)
  : _result(0),
    _readCount(0),
    _maxReadCount(readCount)
{
  _items.push_back(item);
}

ResultBufferImpl::~ResultBufferImpl()
{
  // Do nothing
}

void ResultBufferImpl::increaseMaxReadCount(unsigned int readCount)
{
  if(_maxReadCount == UNLIMITED_COUNT || readCount == UNLIMITED_COUNT)
    _maxReadCount = UNLIMITED_COUNT;
  else _maxReadCount += readCount;
}

Item::Ptr ResultBufferImpl::item(unsigned int index, DynamicContext *context)
{
  while(!_result.isNull() && index >= _items.size()) {
    const Item::Ptr item = _result->next(context);
    if(item.isNull()) {
      _result = 0;
    }
    else {
      _items.push_back(item);
    }
  }

  if(index >= _items.size()) {
    return 0;
  }
  else {
    return _items[index];
  }
}

class BufferedResult : public ResultImpl
{
public:
  BufferedResult(ResultBufferImpl *impl, unsigned start)
    : ResultImpl(0), _impl(impl), _pos(start) {}

  virtual Item::Ptr nextOrTail(Result &tail, DynamicContext *context)
  {
    if(_impl->getRefCount() == 1 && _pos >= _impl->_items.size()) {
      // Ditch the buffer if we're the only result for it
      _impl->_result->skip(_pos - _impl->_items.size(), context);
      tail = _impl->_result;
      return 0;
    }
    Item::Ptr item = _impl->item(_pos++, context);
    if(item.isNull()) {
      tail = 0;
    }
    return item;
  }

  virtual void toResultBuffer(unsigned int readCount, ResultBuffer &buffer)
  {
    _impl->increaseMaxReadCount(readCount);
    buffer = ResultBuffer(_impl.get(), _pos);
  }

  virtual void skip(unsigned count, DynamicContext *context)
  {
    _pos += count;
  }

private:
  ResultBufferImpl::Ptr _impl;
  unsigned int _pos;
};

Result ResultBufferImpl::createResult(unsigned start)
{
  return new BufferedResult(this, start);
}
