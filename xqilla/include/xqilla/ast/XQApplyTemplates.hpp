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

#ifndef XQAPPLYTEMPLATES_HPP
#define XQAPPLYTEMPLATES_HPP

#include <xqilla/ast/XQCallTemplate.hpp>
#include <xqilla/functions/XQUserFunction.hpp>
#include <xqilla/simple-api/XQQuery.hpp>

class VariableStore;

class XQILLA_API XQApplyTemplates : public ASTNodeImpl
{
public:
  XQApplyTemplates(ASTNode *expr, TemplateArguments *args, XQUserFunction::Mode *mode, XPath2MemoryManager *mm);
  XQApplyTemplates(ASTNode *expr, TemplateArguments *args, XQUserFunction::Mode *mode,
                   const UserFunctions &templates, XPath2MemoryManager *mm);

  virtual ASTNode *staticResolution(StaticContext *context);
  virtual ASTNode *staticTypingImpl(StaticContext *context);
  virtual Result createResult(DynamicContext *context, int flags=0) const;

  ASTNode *getExpression() const { return expr_; }
  void setExpression(ASTNode *expr) { expr_ = expr; }
  TemplateArguments *getArguments() const { return args_; }
  void setArguments(TemplateArguments *args) { args_ = args; }
  XQUserFunction::Mode *getMode() const { return mode_; }
  void setMode(XQUserFunction::Mode *mode) { mode_ = mode; }
  const UserFunctions &getTemplates() const { return templates_; }

  void evaluateArguments(VarStoreImpl &scope, DynamicContext *context) const;

  static Result executeTemplate(const XQUserFunction *tplt, const TemplateArguments *args, const VariableStore *scope,
                                DynamicContext *context, const LocationInfo *location);


private:
  ASTNode *expr_;
  TemplateArguments *args_;
  XQUserFunction::Mode *mode_;
  UserFunctions templates_;
};

#endif
