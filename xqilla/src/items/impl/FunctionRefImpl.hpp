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

#ifndef _XQILLAFUNCTIONREFIMPL_HPP
#define _XQILLAFUNCTIONREFIMPL_HPP

#include <xqilla/items/FunctionRef.hpp>
#include <xqilla/context/impl/VarStoreImpl.hpp>

class ASTNode;
class FunctionRefResult;
class FuncFactory;

class XQILLA_API FunctionRefImpl : public FunctionRef
{
public:
  typedef RefCountPointer<const FunctionRefImpl> Ptr;

  /** Initialises closure from the StaticAnalysis of variables that are needed */
  FunctionRefImpl(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname,
                  const FunctionSignature *signature, const ASTNode *instance,
                  const StaticAnalysis &sa, DynamicContext *context);
  ~FunctionRefImpl();

  virtual ATQNameOrDerived::Ptr getName(const DynamicContext *context) const;
  virtual size_t getNumArgs() const;
  virtual const FunctionSignature *getSignature() const { return signature_; }
  virtual Result execute(const VectorOfResults &args, DynamicContext *context, const LocationInfo *location) const;
  virtual FunctionRef::Ptr partialApply(const Result &arg, unsigned int argNum, DynamicContext *context, const LocationInfo *location) const;

  virtual const XMLCh *asString(const DynamicContext *context) const;

  virtual void generateEvents(EventHandler *events, const DynamicContext *context,
                              bool preserveNS, bool preserveType) const;

  virtual void typeToBuffer(DynamicContext *context, XERCES_CPP_NAMESPACE_QUALIFIER XMLBuffer &buffer) const;

  virtual void *getInterface(const XMLCh *name) const;

  static ASTNode *createInstance(const FuncFactory *factory, const FunctionSignature *signature, XPath2MemoryManager *mm,
                                 const LocationInfo *location);
  static ASTNode *createInstance(const XMLCh *uri, const XMLCh *name, unsigned int numArgs, StaticContext *context,
                                 const LocationInfo *location, FunctionSignature *&signature);

protected:
  /** Partially evaluates the given FunctionRefImpl with the given argument */
  FunctionRefImpl(const FunctionRefImpl *other, const Result &argument, unsigned int argNum, DynamicContext *context);

  const XMLCh *prefix_, *uri_, *name_;
  const FunctionSignature *signature_;
  bool signatureOwned_;
  const ASTNode *instance_;
  VarStoreImpl varStore_;

  friend class FunctionRefScope;
};
#endif
