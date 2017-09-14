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

#ifndef _VARIABLESTORETEMPLATE_HPP
#define _VARIABLESTORETEMPLATE_HPP

#include <xqilla/framework/XQillaExport.hpp>

#include <vector>
#include <xercesc/util/StringPool.hpp>

#include <assert.h>
#include <xqilla/context/impl/VarHashEntryImpl.hpp>
#include <xqilla/context/VarHashEntry.hpp>
#include <xqilla/context/Scope.hpp>
#include <xqilla/utils/XStr.hpp>

template<class TYPE> class VarHashEntry;
class XPath2MemoryManager;

/** This is the wrapper class for the variable store, which implements the 
    lookup and scoping of simple variables. */
template<class TYPE>
class VariableStoreTemplate
{
public:
  typedef Scope<TYPE> MyScope;

  VariableStoreTemplate(XPath2MemoryManager *memMgr);
  /** default destructor */
  ~VariableStoreTemplate();

  void clear();

  void addScope(typename MyScope::Type type);
  void addScope(MyScope *scope);
  /** Removes the top level scope from the store. To be called at the end of methods to
      implement scoping. */
  void removeScope();
  /** Removes the top level scope from the store, and returns it. The scope must have
      release called on it, otherwise it will leak memory. */
  MyScope *popScope();

  void setCurrentScope(MyScope *scope);

  /** Declares and/or sets a variable in the global scope. */
  void setGlobalVar(const XMLCh* namespaceURI, const XMLCh* name, const TYPE &value);

  /** Deletes a variable from the global scope. */
  void delGlobalVar(const XMLCh* namespaceURI, const XMLCh* name);

  /** Gets a variable from the global scope */
  VarHashEntry<TYPE>* getGlobalVar(const XMLCh* namespaceURI, const XMLCh* name) const;

  /** Declare a var in the top level scope (A full set of
      these namespaceURI/name pair methods should be made) */
  void declareVar(const XMLCh* namespaceURI, const XMLCh* name, const TYPE &value);

  /** Declares and/or sets a variable in the top level scope. */
  void setVar(const XMLCh* namespaceURI, const XMLCh* name, const TYPE &value);

  /** Looks up the value of a variable in the current scope, using ident as an
      qname. Returns the VarHashEntry for the variable, or null if it doesn't
      exist*/
  VarHashEntry<TYPE>* getVar(const XMLCh* namespaceURI, const XMLCh* name) const;

  /** Deletes a variable from the current scope. */
  void delVar(const XMLCh* namespaceURI, const XMLCh* name);

  const MyScope *getCurrentScope() const { return _current; }
  MyScope *getCurrentScope() { return _current; }
  const MyScope *getGlobalScope() const { return _global; }
  const XMLCh *lookUpNamespace(unsigned int nsID) const { return _uriPool.getValueForId(nsID); }

private:
  /** find routine which returns the scope in the hash table which contains the 
      identifiers, or the current/global one if it can't be found. */
  MyScope* find(unsigned int nsID, const XMLCh* name) const;

  XERCES_CPP_NAMESPACE_QUALIFIER XMLStringPool _uriPool;
  XPath2MemoryManager* _memMgr;
  MyScope *_global, *_current;
};

template<class TYPE>
VariableStoreTemplate<TYPE>::VariableStoreTemplate(XPath2MemoryManager* memMgr)
  : _uriPool(17, memMgr)
{
  _memMgr=memMgr;
  _global=new (_memMgr) MyScope(_memMgr, MyScope::GLOBAL_SCOPE);
  _current=NULL;
}

template<class TYPE>
VariableStoreTemplate<TYPE>::~VariableStoreTemplate()
{
  clear();
  delete _global;
}

template<class TYPE>
void VariableStoreTemplate<TYPE>::clear()
{
  // Remove all the scopes
  while(_current) {
    removeScope();
  }
  // Clear the global scope
  _global->clear();
}

template<class TYPE>
void VariableStoreTemplate<TYPE>::addScope(typename MyScope::Type type)
{
  addScope(new (_memMgr) MyScope(_memMgr, type));
}

template<class TYPE>
void VariableStoreTemplate<TYPE>::addScope(MyScope *scope)
{
  scope->setNext(_current);
  setCurrentScope(scope);
}

template<class TYPE>
void VariableStoreTemplate<TYPE>::setCurrentScope(MyScope *scope)
{
  _current = scope;
}

template<class TYPE>
void VariableStoreTemplate<TYPE>::removeScope()
{
  delete popScope();
}

template<class TYPE>
typename VariableStoreTemplate<TYPE>::MyScope *VariableStoreTemplate<TYPE>::popScope()
{
  assert( _current!=NULL );

  MyScope *result = _current;
  _current = _current->getNext();
  return result;
}

template<class TYPE>
void VariableStoreTemplate<TYPE>::setGlobalVar(const XMLCh* namespaceURI,
                                               const XMLCh* name,
                                               const TYPE &value)
{
  if(!_uriPool.exists(namespaceURI))
    namespaceURI=_memMgr->getPooledString(namespaceURI);
  unsigned int nsID=_uriPool.addOrFind(namespaceURI);
  VarHashEntry<TYPE>* result=_global->get(nsID, name);
  if(result)
    result->setValue(value);
  else
    _global->put(nsID, name, new (_memMgr) VarHashEntryImpl<TYPE>(value));
}

template<class TYPE>
void VariableStoreTemplate<TYPE>::setVar(const XMLCh* namespaceURI,
                                         const XMLCh* name,
                                         const TYPE &value)
{
  if(!_uriPool.exists(namespaceURI))
    namespaceURI=_memMgr->getPooledString(namespaceURI);
  unsigned int nsID=_uriPool.addOrFind(namespaceURI);
  MyScope* scope=find(nsID, name);
  VarHashEntry<TYPE>* result = scope->get(nsID, name);

  if(result)
    result->setValue(value);
  else
    scope->put(nsID, name, new (_memMgr) VarHashEntryImpl<TYPE>(value));
}

template<class TYPE>
void VariableStoreTemplate<TYPE>::declareVar(const XMLCh* namespaceURI,
                                             const XMLCh* name,
                                             const TYPE &value)
{
  if(!_uriPool.exists(namespaceURI))
    namespaceURI=_memMgr->getPooledString(namespaceURI);
  unsigned int nsID=_uriPool.addOrFind(namespaceURI);
  _current->put(nsID, name, new (_memMgr) VarHashEntryImpl<TYPE>(value));
}

/** Returns a null VarHashEntry if unsuccessful */
template<class TYPE>
VarHashEntry<TYPE>* VariableStoreTemplate<TYPE>::getVar(const XMLCh* namespaceURI,
                                                        const XMLCh* name) const
{
  unsigned int nsID=_uriPool.getId(namespaceURI);
  // if the namespace is not in the map, the variable is not there neither
  if(nsID!=0)
    return find(nsID, name)->get(nsID, name);
  return NULL;
}

template<class TYPE>
VarHashEntry<TYPE>* VariableStoreTemplate<TYPE>::getGlobalVar(const XMLCh* namespaceURI,
                                                              const XMLCh* name) const
{
  unsigned int nsID=_uriPool.getId(namespaceURI);
  // if the namespace is not in the map, the variable is not there neither
  if(nsID!=0)
    return _global->get(nsID, name);
  return 0;
}

template<class TYPE>
void VariableStoreTemplate<TYPE>::delVar( const XMLCh* namespaceURI, const XMLCh* name)
{
  unsigned int nsID=_uriPool.getId(namespaceURI);
  // if the namespace is not in the map, the variable is not there neither
  if(nsID!=0)
    find(nsID, name)->remove(nsID, name);
}

template<class TYPE>
void VariableStoreTemplate<TYPE>::delGlobalVar( const XMLCh* namespaceURI, const XMLCh* name)
{
  unsigned int nsID=_uriPool.getId(namespaceURI);
  // if the namespace is not in the map, the variable is not there neither
  if(nsID!=0)
    _global->remove(nsID, name);
}

template<class TYPE>
typename VariableStoreTemplate<TYPE>::MyScope* VariableStoreTemplate<TYPE>::find(unsigned int nsID, const XMLCh* name) const
{
  // Check the local scopes
  MyScope* index=_current;
  while(index) {
    if(index->get(nsID, name)!=NULL)
      return index; 
    if(index->getType() == MyScope::LOCAL_SCOPE)
      break;
    index=index->getNext();
  }
  
  // Check the global scope
  if(_global->get(nsID, name)!=NULL)
    return _global;

  // If we can't find it anywhere, then return the top level scope.
  // This lets us set it in the correct place.
  // For gets, we could return any scope.
  return _current==NULL?_global:_current;
}

#endif
