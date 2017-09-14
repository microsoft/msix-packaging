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

#ifndef _REFERENCECOUNTED_HPP
#define _REFERENCECOUNTED_HPP

#include <xqilla/framework/XQillaExport.hpp>
#include <xercesc/framework/MemoryManager.hpp>

// for null RefCountPointer instances
#define NULLRCP ((void *)0)

/** Super class for reference counted classes */
class XQILLA_API ReferenceCounted
{
public:
  ReferenceCounted()
    : _ref_count(0) {}
  virtual ~ReferenceCounted() {}

  /// Increment the reference count
  void incrementRefCount() const
  {
    ++const_cast<unsigned int&>(_ref_count);
  }

  /// Decrement the reference count, deleting if it becomes zero
  virtual void decrementRefCount() const
  {
    if(--const_cast<unsigned int&>(_ref_count) == 0) {
      delete this;
    }
  }

  unsigned int getRefCount() const
  {
    return _ref_count;
  }

protected:
  unsigned int _ref_count; // mutable
};

/** Super class of all the reference counted wrappers for Items */
template<class T> class RefCountPointer
{
public:
  RefCountPointer(T *p = 0) : _p(p)
  {
    if(_p != 0) _p->incrementRefCount();
  }

  template<class T2> RefCountPointer(const RefCountPointer<T2> &o) : _p((T*)(T2*)o)
  {
    if(_p != 0) _p->incrementRefCount();
  }

  RefCountPointer(const RefCountPointer<T> &o) : _p(o._p)
  {
    if(_p != 0) _p->incrementRefCount();
  }

  RefCountPointer &operator=(const RefCountPointer<T> &o)
  {
    if(_p != o._p) {
      if(_p != 0) _p->decrementRefCount();
      _p = o._p;
      if(_p != 0) _p->incrementRefCount();
    }
    return *this;
  }

  ~RefCountPointer()
  {
    if(_p != 0) _p->decrementRefCount();
  }

  T *operator->() const
  {
    return _p;
  }

  operator T*() const
  {
    return _p;
  }

  T *get() const
  {
    return _p;
  }

  bool isNull() const
  {
    return (_p == 0);
  }

  bool notNull() const
  {
    return (_p != 0);
  }

protected:
  T *_p;
};

template<class T1, class T2>
inline bool operator==(const RefCountPointer<T1> &a, const RefCountPointer<T2> &b)
{
  return (void*)(T1*)a == (void*)(T2*)b;
}

template<class T1, class T2>
inline bool operator!=(const RefCountPointer<T1> &a, const RefCountPointer<T2> &b)
{
  return (void*)(T1*)a != (void*)(T2*)b;
}

template<class T>
inline bool operator==(const RefCountPointer<T> &a, void *b)
{
  return (T*)a == (T*)b;
}

template<class T>
inline bool operator!=(const RefCountPointer<T> &a, void *b)
{
  return (T*)a != (T*)b;
}

#endif
