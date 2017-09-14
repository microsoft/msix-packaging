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

/*
  Scope
*/

#ifndef _SCOPE_HPP
#define _SCOPE_HPP

#include <xqilla/framework/XQillaExport.hpp>
#include <xqilla/context/VarHashEntry.hpp>
#include <vector>
#include <xercesc/util/RefHash2KeysTableOf.hpp>
#include <xercesc/util/XMemory.hpp>

template<class TYPE> class VarHashEntry;

/** used inside VariableStore to implement variable scoping */
template<class TYPE>
class Scope : public XERCES_CPP_NAMESPACE_QUALIFIER XMemory
{
public:
  /** enum for classifying type of scope */
  typedef enum {
    GLOBAL_SCOPE,
    LOCAL_SCOPE,
    LOGICAL_BLOCK_SCOPE
  } Type;

  typedef XERCES_CPP_NAMESPACE_QUALIFIER RefHash2KeysTableOf< VarHashEntry<TYPE> > VarHash;

  /** constructor. */
  Scope(XPath2MemoryManager* memMgr, Type type);
  ~Scope();

  void clear();

  Type getType() const;
  VarHashEntry<TYPE>* get(unsigned int nsID, const XMLCh* name);
  void put(unsigned int nsID, const XMLCh* name, VarHashEntry<TYPE>* value);
  void remove(unsigned int nsID, const XMLCh* name);
  std::vector< std::pair<unsigned int, const XMLCh*> > getVars() const;

  Scope* getNext();
  void setNext(Scope* next);
  
private:
  typename Scope<TYPE>::Type _type;
  VarHash _map;
  XPath2MemoryManager* _memMgr;
  Scope<TYPE>* _next;
};

template<class TYPE>
Scope<TYPE>::Scope(XPath2MemoryManager* memMgr, Type type) :
    _map(17, true, memMgr)
{
  _memMgr=memMgr;
  _type = type;
  _next = NULL;
}

template<class TYPE>
void Scope<TYPE>::clear()
{
  _map.removeAll();
}

template<class TYPE>
typename Scope<TYPE>::Type Scope<TYPE>::getType() const
{
  return _type;
}

template<class TYPE>
VarHashEntry<TYPE>* Scope<TYPE>::get(unsigned int nsID, const XMLCh* name)
{
	return _map.get(name,nsID);
}

template<class TYPE>
void Scope<TYPE>::put(unsigned int nsID, const XMLCh* name, VarHashEntry<TYPE>* value)
{
	_map.put((void*)_memMgr->getPooledString(name),nsID,value);
}

template<class TYPE>
void Scope<TYPE>::remove(unsigned int nsID, const XMLCh* name)
{
	_map.removeKey(name,nsID);
}

template<class TYPE>
std::vector< std::pair<unsigned int, const XMLCh*> > Scope<TYPE>::getVars() const
{
    std::vector< std::pair<unsigned int, const XMLCh*> > result;
    XERCES_CPP_NAMESPACE_QUALIFIER RefHash2KeysTableOfEnumerator< VarHashEntry<TYPE> > iterator(const_cast<VarHash*>(&_map));
    while(iterator.hasMoreElements())
	{
		XMLCh* name;
		int nsID;
		iterator.nextElementKey((void*&)name, nsID);
        result.push_back(std::pair<unsigned int, const XMLCh*>(nsID,name));
	}
	return result;
}

template<class TYPE>
Scope<TYPE>::~Scope()
{
  _map.removeAll();
}

template<class TYPE>
Scope<TYPE>* Scope<TYPE>::getNext()
{
    return _next;
}

template<class TYPE>
void Scope<TYPE>::setNext(Scope<TYPE>* next)
{
    _next=next;
}

#endif // _SCOPE_HPP
