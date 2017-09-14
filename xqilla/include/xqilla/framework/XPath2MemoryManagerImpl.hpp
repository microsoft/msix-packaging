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

#ifndef __XPATH2MEMORYMANAGERIMPL_HPP
#define __XPATH2MEMORYMANAGERIMPL_HPP

#include <xqilla/framework/XQillaExport.hpp>
#include <xqilla/framework/BaseMemoryManager.hpp>

#if 0
// Turn OFF use of windows heap by default.
// There is a fragmentation problem resulting
// in less available memory
#if defined(WIN32) && !defined(__CYGWIN__)
#define WIN_USE_HEAP
#endif
#endif

class XQILLA_API XPath2MemoryManagerImpl : public BaseMemoryManager
{
public:
  XPath2MemoryManagerImpl();
  virtual ~XPath2MemoryManagerImpl();

#if defined(WIN_USE_HEAP)
#if _XERCES_VERSION >= 30000
  virtual void *allocate(XMLSize_t numElements);
#else
  virtual void *allocate(size_t numElements);
#endif
  virtual void deallocate(void* p);
#endif

protected:
#if defined(WIN_USE_HEAP)
  virtual void initialise();
  virtual void releaseAll();

  virtual void *internal_allocate(size_t size) { return 0; }
  virtual void internal_deallocate(void *p) {}
#else
  virtual void *internal_allocate(size_t size);
  virtual void internal_deallocate(void *p);
#endif

private:
#if defined(WIN_USE_HEAP)
  HANDLE fHeap;
#endif
};

#endif //__XPATH2MEMORYMANAGERIMPL_HPP

