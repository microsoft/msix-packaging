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

#ifndef _VARIABLETYPESTORE_HPP
#define _VARIABLETYPESTORE_HPP

#include <xqilla/framework/XQillaExport.hpp>
#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/util/XMemory.hpp>

class SequenceType;
class XPath2MemoryManager;
class StaticAnalysis;
class XQGlobalVariable;

/** This is the wrapper class for the variable store, which implements the 
    lookup and scoping of simple variables. */
class XQILLA_API VariableTypeStore : public XERCES_CPP_NAMESPACE_QUALIFIER XMemory
{
public:
  /** default destructor */
  virtual ~VariableTypeStore() {};

  /** Clears all variable values and added scopes from the store */
  virtual void clear() = 0;

  /** Adds a new local scope to the store. */
  virtual void addLocalScope() = 0;
  /** Adds a new logical block scope to the store. */
  virtual void addLogicalBlockScope() = 0;
  /** Removes the top level scope from the store. To be called at the end of methods to
      implement scoping. */
  virtual void removeScope() = 0;

  /** Declares and/or sets a variable in the global scope. */
  virtual void declareGlobalVar(const XMLCh* namespaceURI, const XMLCh* name,
                                const StaticAnalysis &src, XQGlobalVariable *global) = 0;

  /** Gets a variable from the global scope */
  virtual const StaticAnalysis* getGlobalVar(const XMLCh* namespaceURI, const XMLCh* name,
                                             XQGlobalVariable **global = 0) const = 0;

  /** Declare a var in the top level scope (A full set of
      these namespaceURI/name pair methods should be made) */
  virtual void declareVar(const XMLCh* namespaceURI, const XMLCh* name,
                          const StaticAnalysis &src) = 0;

  /** Looks up the value of a variable in the current scope, using ident as an
      qname. Returns a boolean (true if successful), and the SequenceType value
      of the variable*/
  virtual const StaticAnalysis *getVar(const XMLCh* namespaceURI, const XMLCh* name,
                                       XQGlobalVariable **global = 0) const = 0;
};

#endif
