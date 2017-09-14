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

#ifndef _XQFUNCTION_HPP
#define _XQFUNCTION_HPP

#include <xqilla/framework/XQillaExport.hpp>
#include <xqilla/ast/ASTNodeImpl.hpp>
#include <xqilla/runtime/Sequence.hpp>
#include <xqilla/items/ATStringOrDerived.hpp>
#include <xqilla/context/ItemFactory.hpp>

#include <xercesc/util/XMLUniDefs.hpp>      // because every implementation will use these to define the function name
#include <vector>

class SequenceType;
class FunctionSignature;

class XQILLA_API XQFunction : public ASTNodeImpl
{
public:
  static const XMLCh XMLChFunctionURI[];

  XQFunction(const XMLCh *name, const char *signature, const VectorOfASTNodes &args, XPath2MemoryManager* memMgr);

  const XMLCh* getFunctionURI()const { return uri_; }
  const XMLCh* getFunctionName()const { return name_; }
  FunctionSignature *getSignature() const { return signature_; }
  void setSignature(FunctionSignature *s) { signature_ = s; }
  const VectorOfASTNodes &getArguments() const { return _args; }
  size_t getNumArgs() const;
  
  virtual ASTNode *staticResolution(StaticContext *context);
  virtual ASTNode *staticTypingImpl(StaticContext *context);

  virtual Result createResult(DynamicContext* context, int flags=0) const;
  virtual Sequence createSequence(DynamicContext* context, int flags=0) const;

  virtual Result getParamNumber(size_t number, DynamicContext* context, int flags=0) const;

  void parseSignature(StaticContext *context);

protected:
  void resolveArguments(StaticContext *context, bool numericFunction = false);
  void calculateSRCForArguments(StaticContext *context, bool checkTimezone = false);

  const XMLCh *name_, *uri_;
  const char *sigString_;
  FunctionSignature *signature_;
  VectorOfASTNodes _args;
};

#endif
