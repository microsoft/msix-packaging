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

#ifndef _XQVARIABLE_HPP
#define _XQVARIABLE_HPP

#include <xqilla/framework/XQillaExport.hpp>

#include <xqilla/ast/ASTNodeImpl.hpp>
#include <xqilla/runtime/Sequence.hpp>

class XQGlobalVariable;

class XQILLA_API XQVariable : public ASTNodeImpl
{
public:
  XQVariable(const XMLCh *qualifiedName, XPath2MemoryManager* memMgr);
  XQVariable(const XMLCh *uri, const XMLCh *name, XPath2MemoryManager* memMgr);
  XQVariable(const XMLCh *prefix, const XMLCh *uri, const XMLCh *name, XQGlobalVariable *global, XPath2MemoryManager* memMgr);
  ~XQVariable();

  virtual Result createResult(DynamicContext* context, int flags=0) const;
  virtual ASTNode* staticResolution(StaticContext *context);
  virtual ASTNode *staticTypingImpl(StaticContext *context);

  const XMLCh *getPrefix() const;
  const XMLCh *getURI() const;
  const XMLCh *getName() const;
  XQGlobalVariable *getGlobal() const { return _global; }

private:
  const XMLCh* _prefix;
  const XMLCh* _uri;
  const XMLCh* _name;
  XQGlobalVariable *_global;
};

#endif


