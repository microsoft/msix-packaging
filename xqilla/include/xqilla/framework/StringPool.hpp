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

#ifndef __STRINGPOOL_HPP
#define __STRINGPOOL_HPP

#include <xqilla/framework/XQillaExport.hpp>
#include <xercesc/framework/MemoryManager.hpp>
#include <memory>
#include <cstring>
//Added so xqilla will compile under CC on Solaris
#include <string>
// Added so xqilla will compile on SunOS 10 using STLPort
#include <memory.h>

class XQILLA_API StringPool
{
public:
  StringPool(XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager *mm);
  ~StringPool();

  /** Returns a copy of the given string */
  const XMLCh* getPooledString(const XMLCh *src);
  /** Returns a copy of the given string, with given length */
  const XMLCh* getPooledString(const XMLCh *src, unsigned int length);
  /** Returns a copy of the transcoding of the given string */
  const XMLCh* getPooledString(const char *src);

  unsigned int getCount() const { return _count; }
  unsigned int getHits() const { return _hits; }
  unsigned int getMisses() const { return _misses; }
  unsigned int getTooBig() const { return _toobig; }
  void dumpStatistics() const;

private:
  StringPool(const StringPool&);
  StringPool &operator=(const StringPool&);

  static unsigned int hash(const XMLCh *v, unsigned int length);
  const XMLCh *replicate(const XMLCh *v, unsigned int length) const;
  void resize();

  class Bucket
  {
  public:
    Bucket(const XMLCh *v, unsigned int l, unsigned int h, Bucket *n)
      : value(v), length(l), hashValue(h), next(n) {}

    const XMLCh *value;
    unsigned int length;
    unsigned int hashValue;
    Bucket *next;
  };

  XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager *_mm;
  Bucket **_bucketList;
  unsigned int _modulus;
  unsigned int _count;
  unsigned int _hits;
  unsigned int _misses;
  unsigned int _toobig;
};

inline unsigned int StringPool::hash(const XMLCh *v, unsigned int length)
{
  unsigned int hashVal = 0;
  while(length) {
    hashVal += (hashVal * 37) + (hashVal >> 24) + (unsigned int)(*v);
    ++v;
    --length;
  }
  return hashVal;
}

inline const XMLCh *StringPool::replicate(const XMLCh *v, unsigned int length) const
{
  unsigned int size = length * sizeof(XMLCh);
  XMLCh *ret = (XMLCh*)_mm->allocate(size + sizeof(XMLCh));
  memcpy(ret, v, size);
  ret[length] = 0;
  return ret;
}

#endif
