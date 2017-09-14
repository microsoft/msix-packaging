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

#ifndef XQINLINEFUNCTION_HPP
#define XQINLINEFUNCTION_HPP

#include <xqilla/ast/ASTNodeImpl.hpp>

class XQUserFunction;
class FunctionSignature;

class XQILLA_API XQInlineFunction : public ASTNodeImpl
{
public:
  XQInlineFunction(XQUserFunction *func, XPath2MemoryManager *mm);
  XQInlineFunction(XQUserFunction *func, const XMLCh *prefix, const XMLCh *uri, const XMLCh *name,
                   unsigned int numArgs, FunctionSignature *signature, ASTNode *instance,
                   XPath2MemoryManager *mm);

  virtual ASTNode *staticResolution(StaticContext *context);
  virtual ASTNode *staticTypingImpl(StaticContext *context);
  virtual Result createResult(DynamicContext* context, int flags=0) const;

  const XMLCh *getPrefix() const { return prefix_; }
  const XMLCh *getURI() const { return uri_; }
  const XMLCh *getName() const { return name_; }
  unsigned int getNumArgs() const { return numArgs_; }

  XQUserFunction *getUserFunction() const { return func_; }
  void setUserFunction(XQUserFunction *func) { func_ = func; }

  FunctionSignature *getSignature() const { return signature_; }
  void setSignature(FunctionSignature *s) { signature_ = s; }

  ASTNode *getInstance() const { return instance_; }
  void setInstance(ASTNode *i) { instance_ = i; }

private:
  XQUserFunction *func_;
  const XMLCh *prefix_, *uri_, *name_;
  unsigned int numArgs_;
  FunctionSignature *signature_;
  ASTNode *instance_;
};

#endif
