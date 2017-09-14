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

#ifndef __BASEMEMORYMANAGER_HPP
#define __BASEMEMORYMANAGER_HPP

#include <xqilla/framework/XQillaExport.hpp>
#include <xqilla/framework/XPath2MemoryManager.hpp>

#include <xercesc/util/RefHashTableOf.hpp>

// Define ALLOCATE_IN_CHUNKS to 1 to allocate
// CHUNK_SIZE blocks of memory at a time, and
// carve the requested memory from them. The
// DB XML benchmark suggests this behaves badly
// with larger data sets. - jpcs
#define ALLOCATE_IN_CHUNKS 0

#define DEBUG_MEMORY_ALLOCD 0x88884444
#define DEBUG_MEMORY_FREED  0x44442222

XERCES_CPP_NAMESPACE_BEGIN
class DOMNode;
XERCES_CPP_NAMESPACE_END

class VariableStore;
class Collation;
class CollationHelper;
class XQillaNSResolver;
class StringPool;

class XQILLA_API BaseMemoryManager : public XPath2MemoryManager
{
public:
  virtual ~BaseMemoryManager();

  // from MemoryManager
#if _XERCES_VERSION >= 30000
  virtual void* allocate(XMLSize_t numElements);
#else
  virtual void* allocate(size_t numElements); 
#endif
 
  virtual void deallocate(void* p);
  virtual XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager *getExceptionMemoryManager();
  
  /** Returns a copy of the given string */
  virtual const XMLCh* getPooledString(const XMLCh *src);
  virtual const XMLCh* getPooledString(const XMLCh *src, unsigned int length);
  /** Returns a copy of the transcoding of the given string */
  virtual const XMLCh* getPooledString(const char *src);

  /** Use with extreme caution! */
  virtual void reset();

  virtual void dumpStatistics() const;
  virtual size_t getAllocatedObjectCount() const { return objectsAllocated_; }
  virtual size_t getTotalAllocatedMemory() const { return totalMemoryAllocated_; }
  virtual const StringPool *getStringPool() const {
    return fStringPool;
  }
  
  /** create a collation */
  virtual Collation* createCollation(CollationHelper* helper);

  /** create a resolver */
  virtual XQillaNSResolver* createNSResolver(XERCES_CPP_NAMESPACE_QUALIFIER DOMNode *resolverNode);
  
  /** create a store for variables */
  virtual VariableTypeStore* createVariableTypeStore();

  /** create a ATDecimalOrDerived for the given integer */
  virtual ATDecimalOrDerived* createInteger(int value);

protected:
  virtual void initialise();
  virtual void releaseAll();

  virtual void *internal_allocate(size_t size) = 0;
  virtual void internal_deallocate(void *p) = 0;

  struct XQILLA_API MemList {
    MemList *prev;
    MemList *next;
    size_t size;
#if ALLOCATE_IN_CHUNKS
    size_t remaining;
    unsigned int allocCount;
#endif
#if DEBUG_MEMORY
    unsigned long magic;
#endif
  };

#if ALLOCATE_IN_CHUNKS
  struct XQILLA_API MemAlloc {
    MemList *list;
  };
#endif

  MemList *fCurrentBlock;

  size_t objectsAllocated_;
  size_t totalMemoryAllocated_;

  StringPool *fStringPool;

#if _XERCES_VERSION >= 30000
  XERCES_CPP_NAMESPACE_QUALIFIER RefHashTableOf<ATDecimalOrDerived, XERCES_CPP_NAMESPACE_QUALIFIER PtrHasher>* fIntegerPool;
#else
  XERCES_CPP_NAMESPACE_QUALIFIER RefHashTableOf<ATDecimalOrDerived>* fIntegerPool;
#endif
};

#endif

