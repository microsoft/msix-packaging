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
#include <xqilla/framework/BaseMemoryManager.hpp>

#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/RuntimeException.hpp>
#include <xercesc/util/XMLUni.hpp>
#if _XERCES_VERSION < 30000
#include <xercesc/util/HashPtr.hpp>
#endif

#if defined(XERCES_HAS_CPP_NAMESPACE)
XERCES_CPP_NAMESPACE_USE
#endif

#include <xqilla/framework/StringPool.hpp>
#include <xqilla/exceptions/XQillaException.hpp>
#include <xqilla/utils/XStr.hpp>
#include <xqilla/context/impl/CollationImpl.hpp>
#include <xqilla/dom-api/impl/XQillaNSResolverImpl.hpp>
#include "../context/impl/VarTypeStoreImpl.hpp"
#include <xqilla/items/impl/ATDecimalOrDerivedImpl.hpp>

#include <iostream>

static const unsigned int CHUNK_SIZE = 32 * 1024;
#if (defined(__HP_aCC) && defined(__ia64)) || defined(__sparcv9)
#define ALLOC_ALIGN 16
#elif defined(__sparcv8)
#define ALLOC_ALIGN 8
#else
#define ALLOC_ALIGN 4
#endif

#define PAD ((sizeof(MemList) + ALLOC_ALIGN - 1)&~(ALLOC_ALIGN-1))

BaseMemoryManager::~BaseMemoryManager() 
{
}

void BaseMemoryManager::reset()
{
  releaseAll();
  initialise();
}

void BaseMemoryManager::initialise()
{
  fCurrentBlock = 0;
  objectsAllocated_ = 0;
  totalMemoryAllocated_ = 0;
  fStringPool = new (this) StringPool(this);
  fIntegerPool = 0;
}

void BaseMemoryManager::releaseAll()
{
  if (fIntegerPool)
    fIntegerPool->cleanup();
  // Run backwards through the linked list, deleting the blocks of memory
  while(fCurrentBlock) {
    MemList *prev = fCurrentBlock->prev;
    internal_deallocate((char *)fCurrentBlock);
    fCurrentBlock = prev;
  }
}

MemoryManager *BaseMemoryManager::getExceptionMemoryManager()
{
  return XMLPlatformUtils::fgMemoryManager;
}

#if _XERCES_VERSION >= 30000
void *BaseMemoryManager::allocate(XMLSize_t amount)
#else
void *BaseMemoryManager::allocate(size_t amount)
#endif	
{
#if ALLOCATE_IN_CHUNKS
#error "alignment constraints are not implemented for this case; fix that or remove this error if you think it is unnecessary for your platform"	
  size_t memSize = amount + sizeof(MemAlloc);

  if(fCurrentBlock == 0 || fCurrentBlock->remaining < memSize) {
    size_t chunkSize = sizeof(MemList);
	if(memSize > CHUNK_SIZE) {
      chunkSize += memSize;
    }
	else {
      chunkSize += CHUNK_SIZE;
	}

    char *newBlock = 0;
    try {
      newBlock = (char*)internal_allocate(chunkSize);
    }
    catch (...) {}
    if(!newBlock) {
      throw XQillaException(XQillaException::RUNTIME_ERR, X("BaseMemoryManager::allocate(): Out of memory"));
    }

    ++objectsAllocated_;
    totalMemoryAllocated_ += chunkSize;

    MemList *newMemList = (MemList*)newBlock;
    newMemList->prev = fCurrentBlock;
    newMemList->next = 0;
    newMemList->size = chunkSize;
    newMemList->remaining = chunkSize - sizeof(MemList);
    newMemList->allocCount = 0;

    if(fCurrentBlock) {
      fCurrentBlock->next = newMemList;
    }
    fCurrentBlock = newMemList;
  }

  fCurrentBlock->remaining -= memSize;
  ++fCurrentBlock->allocCount;

  // Carve memory from the end of the current chunk
  char *mem = ((char*)fCurrentBlock) + sizeof(MemList)
    + fCurrentBlock->remaining;

  MemAlloc *alloc = (MemAlloc*)mem;
  alloc->list = fCurrentBlock;

  return (void*)(mem + sizeof(MemAlloc));
#else
  char *newBlock = 0;
  try {
    newBlock = (char*)internal_allocate(amount + PAD);
  }
  catch (...) {}
  if(!newBlock) {
    throw XQillaException(XQillaException::RUNTIME_ERR, X("BaseMemoryManager::allocate(): Out of memory"));
  }

  ++objectsAllocated_;
  totalMemoryAllocated_ += amount;

  MemList *newMemList = (MemList *)newBlock;
  newMemList->prev = fCurrentBlock;
  newMemList->next = 0;
  newMemList->size = amount;
#if DEBUG_MEMORY
  newMemList->magic = DEBUG_MEMORY_ALLOCD;
#endif

  if(fCurrentBlock) {
#if DEBUG_MEMORY
	  if (fCurrentBlock->magic != DEBUG_MEMORY_ALLOCD)
		  throw XQillaException(XQillaException::RUNTIME_ERR, X("BaseMemoryManager::allocate(): Corrupt block list"));		  
#endif
    fCurrentBlock->next = newMemList;
  }
  fCurrentBlock = newMemList;

  return (void *)(newBlock + PAD);
#endif
}

void BaseMemoryManager::deallocate(void* p)
{
  if(p) {
#if ALLOCATE_IN_CHUNKS
    MemAlloc *alloc = (MemAlloc*)(((char*)p) - sizeof(MemAlloc));
    MemList *oldMemList = alloc->list;

    --oldMemList->allocCount;
    if(oldMemList->allocCount == 0) {
      // Decouple the old block from the list
      if(oldMemList->prev) {
        oldMemList->prev->next = oldMemList->next;
      }
      if(oldMemList->next) {
        oldMemList->next->prev = oldMemList->prev;
      }
      else {
        // If the old block doesn't have a next, it must currently be fCurrentBlock
        fCurrentBlock = oldMemList->prev;
      }

      --objectsAllocated_;
      totalMemoryAllocated_ -= oldMemList->size;

      internal_deallocate((void*)oldMemList);
    }
#else
    char *oldBlock = ((char *)p) - PAD;
    MemList *oldMemList = (MemList *)oldBlock;
#if DEBUG_MEMORY
    if (oldMemList->magic != DEBUG_MEMORY_ALLOCD) {
	    if (oldMemList->magic == DEBUG_MEMORY_FREED)
		    throw XQillaException(XQillaException::RUNTIME_ERR, X("BaseMemoryManager::deallocate(): Trying to free free'd memory"));
	    else
		    throw XQillaException(XQillaException::RUNTIME_ERR, X("BaseMemoryManager::deallocate(): Trying to free unalloc'd memory"));
    } else
	    oldMemList->magic = DEBUG_MEMORY_FREED;
#endif

    // Decouple the old block from the list
    if(oldMemList->prev) {
#if DEBUG_MEMORY
    if (oldMemList->prev->magic != DEBUG_MEMORY_ALLOCD) {
	    if (oldMemList->prev->magic == DEBUG_MEMORY_FREED)
		    throw XQillaException(XQillaException::RUNTIME_ERR, X("BaseMemoryManager::deallocate(): Free'd memory found on list"));
	    else
		    throw XQillaException(XQillaException::RUNTIME_ERR, X("BaseMemoryManager::deallocate(): stray memory found on list"));
    }
#endif	    
      oldMemList->prev->next = oldMemList->next;
    }
    if(oldMemList->next) {
#if DEBUG_MEMORY
    if (oldMemList->next->magic != DEBUG_MEMORY_ALLOCD) {
	    if (oldMemList->next->magic == DEBUG_MEMORY_FREED)
		    throw XQillaException(XQillaException::RUNTIME_ERR, X("BaseMemoryManager::deallocate(): Free'd memory found on list (next)"));
	    else
		    throw XQillaException(XQillaException::RUNTIME_ERR, X("BaseMemoryManager::deallocate(): stray memory found on list (next)"));
    }
#endif	    
      oldMemList->next->prev = oldMemList->prev;
    }
    else {
      // If the old block doesn't have a next, it must currently be fCurrentBlock
      fCurrentBlock = oldMemList->prev;
    }
#if DEBUG_MEMORY
    if (fCurrentBlock && fCurrentBlock->magic != DEBUG_MEMORY_ALLOCD) {
	    if (fCurrentBlock->magic == DEBUG_MEMORY_FREED)
		    throw XQillaException(XQillaException::RUNTIME_ERR, X("BaseMemoryManager::deallocate(): Free'd memory found on list (fCurrent)"));
	    else
		    throw XQillaException(XQillaException::RUNTIME_ERR, X("BaseMemoryManager::deallocate(): stray memory found on list (fCurrent)"));
    }
#endif
    --objectsAllocated_;
    totalMemoryAllocated_ -= oldMemList->size;

    internal_deallocate(oldBlock);
#endif
  }
}

const XMLCh *BaseMemoryManager::getPooledString(const XMLCh *src)
{
  return fStringPool->getPooledString(src);
}

const XMLCh *BaseMemoryManager::getPooledString(const XMLCh *src, unsigned int length)
{
  return fStringPool->getPooledString(src, length);
}

const XMLCh *BaseMemoryManager::getPooledString(const char *src)
{
  return fStringPool->getPooledString(src);
}

void BaseMemoryManager::dumpStatistics() const
{
  std::cout << "Memory statistics:" << std::endl;

  fStringPool->dumpStatistics();

  std::cout << "\tAllocated objects: " << (unsigned int)objectsAllocated_ << std::endl;
  std::cout << "\tTotal Memory Allocated: " << (unsigned int)totalMemoryAllocated_ << std::endl;
}

/** create a collation */
Collation* BaseMemoryManager::createCollation(CollationHelper* helper) {
  return new (this) CollationImpl(this,helper);
}

/** create a resolver */
XQillaNSResolver* BaseMemoryManager::createNSResolver(DOMNode *resolverNode) {
  return new (this) XQillaNSResolverImpl(this, resolverNode);
}

/** create a store for variables */
VariableTypeStore* BaseMemoryManager::createVariableTypeStore() {
  return new (this) VarTypeStoreImpl(this);
} 

/** create a ATDecimalOrDerived for the given integer */
ATDecimalOrDerived* BaseMemoryManager::createInteger(int value) {
  if (!fIntegerPool)
#if _XERCES_VERSION >= 30000
    fIntegerPool = new (this) RefHashTableOf<ATDecimalOrDerived, PtrHasher>(53, true, this);
#else
    fIntegerPool = new (this) RefHashTableOf<ATDecimalOrDerived>(53,true, new (this) HashPtr(),this);
#endif

  ATDecimalOrDerived* itemValue=fIntegerPool->get((const void*)((size_t)value));
  if(itemValue!=NULL)
      return itemValue;
  itemValue=new ATDecimalOrDerivedImpl(value);
  itemValue->incrementRefCount();
  fIntegerPool->put((void*)((size_t)value), itemValue);
  return itemValue;
}
