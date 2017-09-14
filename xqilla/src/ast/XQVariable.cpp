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
#include <xqilla/ast/XQVariable.hpp>
#include <xqilla/utils/XPath2NSUtils.hpp>
#include <xqilla/runtime/Sequence.hpp>
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/parser/QName.hpp>
#include <xqilla/context/VariableStore.hpp>
#include <xqilla/context/VariableTypeStore.hpp>
#include <xqilla/context/VarHashEntry.hpp>
#include <xqilla/exceptions/StaticErrorException.hpp>
#include <xqilla/exceptions/DynamicErrorException.hpp>
#include <xqilla/ast/StaticAnalysis.hpp>
#include <xercesc/framework/XMLBuffer.hpp>

XERCES_CPP_NAMESPACE_USE;

XQVariable::XQVariable(const XMLCh *qualifiedName, XPath2MemoryManager* memMgr)
  : ASTNodeImpl(VARIABLE, memMgr),
    _uri(0),
    _global(0)
{
  QualifiedName qname(qualifiedName, getMemoryManager());
  _prefix = qname.getPrefix();
  _name = qname.getName();
}

XQVariable::XQVariable(const XMLCh *uri, const XMLCh *name, XPath2MemoryManager* memMgr)
  : ASTNodeImpl(VARIABLE, memMgr),
    _prefix(0),
    _uri(uri),
    _name(name),
    _global(0)
{
}

XQVariable::XQVariable(const XMLCh *prefix, const XMLCh *uri, const XMLCh *name, XQGlobalVariable *global, XPath2MemoryManager* memMgr)
  : ASTNodeImpl(VARIABLE, memMgr),
    _prefix(prefix),
    _uri(uri),
    _name(name),
    _global(global)
{
}

XQVariable::~XQVariable()
{
}

Result XQVariable::createResult(DynamicContext* context, int flags) const
{
  return context->getVariableStore()->getVar(_uri, _name);
}

ASTNode* XQVariable::staticResolution(StaticContext *context)
{
  // An unprefixed variable reference is in no namespace.
  if(_prefix && *_prefix)
    _uri = context->getUriBoundToPrefix(_prefix, this);
  return this;
}

ASTNode *XQVariable::staticTypingImpl(StaticContext *context)
{
  if(context == 0) return this;

  _src.clear();

  const StaticAnalysis *var_src = context->getVariableTypeStore()->getVar(_uri, _name, &_global);
  if(var_src == NULL || (var_src->getProperties() & StaticAnalysis::UNDEFINEDVAR)!=0) {
    XMLBuffer errMsg;
    errMsg.append(X("A variable called {"));
    errMsg.append(_uri);
    errMsg.append(X("}"));
    errMsg.append(_name);
    errMsg.append(X(" does not exist [err:XPST0008]"));
    XQThrow(StaticErrorException, X("XQVariable::staticResolution"), errMsg.getRawBuffer());
  }
  _src.setProperties(var_src->getProperties() & ~(StaticAnalysis::SUBTREE|StaticAnalysis::SAMEDOC));
  _src.getStaticType() = var_src->getStaticType();
  _src.variableUsed(_uri, _name);

  return this;
}

const XMLCh *XQVariable::getPrefix() const {
  return _prefix;
}

const XMLCh *XQVariable::getURI() const {
  return _uri;
}

const XMLCh *XQVariable::getName() const {
  return _name;
}
