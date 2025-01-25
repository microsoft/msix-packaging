/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * $Id$
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/internal/MemoryManagerImpl.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>

#include <cstdint>
#include <new>

XERCES_CPP_NAMESPACE_BEGIN

MemoryManager* MemoryManagerImpl::getExceptionMemoryManager()
{
    return this;
}

void* MemoryManagerImpl::allocate(XMLSize_t size)
{
    void* memptr = new(std::nothrow) uint8_t[size];
    if(memptr==NULL && size!=0)
        throw OutOfMemoryException();
    return memptr;
}

void MemoryManagerImpl::deallocate(void* p)
{
    delete[] static_cast<uint8_t*>(p);
}

XERCES_CPP_NAMESPACE_END
