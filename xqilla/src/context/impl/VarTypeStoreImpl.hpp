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
  VarStoreImpl - Implementation of the variable store class
*/

#ifndef _VARTYPESTOREIMPL_HPP
#define _VARTYPESTOREIMPL_HPP

#include <xqilla/framework/XQillaExport.hpp>

#include <xqilla/context/VariableTypeStore.hpp>
#include <xqilla/context/impl/VariableStoreTemplate.hpp>
#include <xqilla/context/Scope.hpp>
#include <xqilla/context/VarHashEntry.hpp>
#include <xqilla/framework/StringPool.hpp>

class XPath2MemoryManager;

/** implementation of the variable type store. */
class XQILLA_API VarTypeStoreImpl : public VariableTypeStore
{
public:
  /** constructor. automatically creates a global store for the
      global variables. */
  VarTypeStoreImpl(XPath2MemoryManager* memMgr);
  /// destructor
  ~VarTypeStoreImpl();

  /** Clears all variable values and added scopes from the store */
  virtual void clear();

  /** Adds a new scope to the store. Add a LOCAL_SCOPE scope at the
      beginning of *procedural* methods, and a LOGICAL_BLOCK_SCOPE
      scope at the beginning of logical block methods. */
  virtual void addLocalScope();
  virtual void addLogicalBlockScope();
  /** Removes the top level scope from the store. To be called at
      the end of methods to implement scoping. */
  virtual void removeScope();

  /** Declares and/or sets a variable in the global scope. */
  virtual void declareGlobalVar(const XMLCh* namespaceURI,
                                const XMLCh* name,
                                const StaticAnalysis &src,
                                XQGlobalVariable *global);

  /** Looks up the value of a variable in the current scope.
      Returns a boolean (true if successful), and the SequenceType
      value of the variable*/
  virtual const StaticAnalysis* getGlobalVar(const XMLCh* namespaceURI,
                                             const XMLCh* name,
                                             XQGlobalVariable **global = 0) const;

  ///Declares a var in the top level scope
  virtual void declareVar(const XMLCh* namespaceURI,
                          const XMLCh* name,
                          const StaticAnalysis &src);

  /** Looks up the value of a variable in the current scope.
      Returns a boolean (true if successful), and the SequenceType
      value of the variable*/
  virtual const StaticAnalysis* getVar(const XMLCh* namespaceURI,
                                       const XMLCh* name,
                                       XQGlobalVariable **global = 0) const;

private:
  struct VarType
  {
    VarType(const StaticAnalysis *t, XQGlobalVariable *g)
      : type(t), global(g) {}

    const StaticAnalysis *type;
    XQGlobalVariable *global;
  };

  VariableStoreTemplate<VarType> _store;
};

#endif
