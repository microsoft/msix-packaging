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

#include <xqilla/ast/XQNamespaceBinding.hpp>
#include <xqilla/ast/StaticAnalysis.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/context/ContextHelpers.hpp>
#include <xqilla/update/PendingUpdateList.hpp>

#if defined(XERCES_HAS_CPP_NAMESPACE)
XERCES_CPP_NAMESPACE_USE
#endif

using namespace std;

XQNamespaceBinding::XQNamespaceBinding(DOMXPathNSResolver *namespaces, ASTNode *expr, XPath2MemoryManager *mm)
  : ASTNodeImpl(NAMESPACE_BINDING, mm),
    expr_(expr),
    namespaces_(namespaces)
{
}

ASTNode *XQNamespaceBinding::staticResolution(StaticContext *context)
{
  AutoNsScopeReset jan(context, namespaces_);

  const XMLCh *defaultElementNS = context->getMemoryManager()->
    getPooledString(namespaces_->lookupNamespaceURI(XMLUni::fgZeroLenString));
  context->setDefaultElementAndTypeNS(defaultElementNS);

  expr_ = expr_->staticResolution(context);

  return this;
}

ASTNode *XQNamespaceBinding::staticTypingImpl(StaticContext *context)
{
  _src.clear();
  _src.copy(expr_->getStaticAnalysis());
  return this;
}

class NamespaceBindingResult : public ResultImpl
{
public:
  NamespaceBindingResult(const XQNamespaceBinding *ast, DynamicContext *context)
    : ResultImpl(ast),
      ast_(ast),
      result_(0)
  {
    AutoNsScopeReset jan(context, ast_->getNamespaces());

    const XMLCh *defaultElementNS = context->getMemoryManager()->
      getPooledString(ast_->getNamespaces()->lookupNamespaceURI(XMLUni::fgZeroLenString));
    context->setDefaultElementAndTypeNS(defaultElementNS);

    result_ = ast->getExpression()->createResult(context);
  }

  NamespaceBindingResult(const XQNamespaceBinding *ast, const Result &contextItems, DynamicContext *context)
    : ResultImpl(ast),
      ast_(ast),
      result_(0)
  {
    AutoNsScopeReset jan(context, ast_->getNamespaces());

    const XMLCh *defaultElementNS = context->getMemoryManager()->
      getPooledString(ast_->getNamespaces()->lookupNamespaceURI(XMLUni::fgZeroLenString));
    context->setDefaultElementAndTypeNS(defaultElementNS);

    result_ = ast->getExpression()->iterateResult(contextItems, context);
  }

  virtual Item::Ptr next(DynamicContext *context)
  {
    AutoNsScopeReset jan(context, ast_->getNamespaces());

    const XMLCh *defaultElementNS = context->getMemoryManager()->
      getPooledString(ast_->getNamespaces()->lookupNamespaceURI(XMLUni::fgZeroLenString));
    context->setDefaultElementAndTypeNS(defaultElementNS);

    return result_->next(context);
  }

private:
  const XQNamespaceBinding *ast_;
  Result result_;
};

Result XQNamespaceBinding::createResult(DynamicContext *context, int flags) const
{
  return new NamespaceBindingResult(this, context);
}

Result XQNamespaceBinding::iterateResult(const Result &contextItems, DynamicContext *context) const
{
  return new NamespaceBindingResult(this, contextItems, context);
}

EventGenerator::Ptr XQNamespaceBinding::generateEvents(EventHandler *events, DynamicContext *context,
                                                       bool preserveNS, bool preserveType) const
{
  AutoNsScopeReset jan(context, namespaces_);

  const XMLCh *defaultElementNS = context->getMemoryManager()->
    getPooledString(namespaces_->lookupNamespaceURI(XMLUni::fgZeroLenString));
  context->setDefaultElementAndTypeNS(defaultElementNS);

  return expr_->generateEvents(events, context, preserveNS, preserveType);
}

PendingUpdateList XQNamespaceBinding::createUpdateList(DynamicContext *context) const
{
  AutoNsScopeReset jan(context, namespaces_);

  const XMLCh *defaultElementNS = context->getMemoryManager()->
    getPooledString(namespaces_->lookupNamespaceURI(XMLUni::fgZeroLenString));
  context->setDefaultElementAndTypeNS(defaultElementNS);

  return expr_->createUpdateList(context);
}

