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
#include <xqilla/framework/XPath2MemoryManagerImpl.hpp>

XPath2MemoryManagerImpl::XPath2MemoryManagerImpl()
{
  initialise();
}

XPath2MemoryManagerImpl::~XPath2MemoryManagerImpl() 
{
  releaseAll();
}

#ifdef WIN_USE_HEAP
#if _XERCES_VERSION >= 30000
void *XPath2MemoryManagerImpl::allocate(XMLSize_t amount)
{
  ++objectsAllocated_;
  totalMemoryAllocated_ += amount;
  return HeapAlloc(fHeap,HEAP_NO_SERIALIZE|HEAP_ZERO_MEMORY,amount);
}
#else
void *XPath2MemoryManagerImpl::allocate(size_t amount)
{
  ++objectsAllocated_;
  totalMemoryAllocated_ += amount;
  return HeapAlloc(fHeap,HEAP_NO_SERIALIZE|HEAP_ZERO_MEMORY,amount);
}
#endif

void XPath2MemoryManagerImpl::deallocate(void* p)
{
  --objectsAllocated_;
  totalMemoryAllocated_ -= HeapSize(fHeap,HEAP_NO_SERIALIZE,p);
  HeapFree(fHeap,HEAP_NO_SERIALIZE,p);
}

void XPath2MemoryManagerImpl::initialise()
{
  fHeap=HeapCreate(HEAP_NO_SERIALIZE,128*1024,0);
  BaseMemoryManager::initialise();
}

void XPath2MemoryManagerImpl::releaseAll()
{
  BaseMemoryManager::releaseAll();
  HeapDestroy(fHeap);
}

#else

void *XPath2MemoryManagerImpl::internal_allocate(size_t size)
{
  return new char[size];
}

void XPath2MemoryManagerImpl::internal_deallocate(void *p)
{
  delete [] ((char *)p);
}

#endif

