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

#ifndef XQGLOBALVARIABLE_HPP
#define XQGLOBALVARIABLE_HPP

#include <xqilla/ast/ASTNodeImpl.hpp>

class SequenceType;

class XQILLA_API XQGlobalVariable : public LocationInfo
{
public:
  XQGlobalVariable(const XMLCh* varQName, SequenceType* seqType, ASTNode* value, XPath2MemoryManager *mm, bool isParam = false);

  void execute(DynamicContext* context) const;
  void resolveName(StaticContext *context);
  void staticResolution(StaticContext *context);
  void staticTypingOnce(StaticContext *context, StaticTyper *styper);
  void resetStaticTypingOnce();
  void staticTyping(StaticContext *context, StaticTyper *styper);

  const XMLCh* getVariableName() const;
  bool isExternal() const;
  bool isParam() const { return isParam_; }

  bool isRequired() const { return required_; }
  void setRequired(bool r) { required_ = r; }

  const XMLCh *getVariableURI() const;
  void setVariableURI(const XMLCh *uri) { m_szURI = uri; }
  const XMLCh *getVariableLocalName() const;
  void setVariableLocalName(const XMLCh *name) { m_szLocalName = name; }
  const SequenceType *getSequenceType() const;
  void setSequenceType(SequenceType *type) { m_Type = type; }
  const ASTNode *getVariableExpr() const;
  void setVariableExpr(ASTNode* value);

  const StaticAnalysis &getStaticAnalysis() const { return _src; }

protected:
  bool isParam_;
  bool required_;
  bool xpath1Compat_;
  const XMLCh* m_szQName;
  const XMLCh* m_szURI;
  const XMLCh* m_szLocalName;
  SequenceType* m_Type;
  ASTNode* m_Value;
  StaticAnalysis _src;
  enum { BEFORE, DURING, AFTER } staticTyped_;
};

#endif
