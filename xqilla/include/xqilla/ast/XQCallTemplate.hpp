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

#ifndef XQCALLTEMPLATE_HPP
#define XQCALLTEMPLATE_HPP

#include <xqilla/ast/ASTNodeImpl.hpp>
#include <xqilla/simple-api/XQQuery.hpp>

class SequenceType;

class XQILLA_API XQTemplateArgument : public LocationInfo
{
public:
  XQTemplateArgument(const XMLCh *qname, ASTNode *value, XPath2MemoryManager *memMgr);

  const XMLCh *qname, *uri, *name;
  ASTNode *value;
  SequenceType *seqType;
  StaticAnalysis varSrc;
};

typedef std::vector<XQTemplateArgument*, XQillaAllocator<XQTemplateArgument*> > TemplateArguments;

class XQILLA_API XQCallTemplate : public ASTNodeImpl
{
public:
  XQCallTemplate(const XMLCh *qname, TemplateArguments *args, XPath2MemoryManager *mm);
  XQCallTemplate(ASTNode *qname, TemplateArguments *args, XPath2MemoryManager *mm);
  XQCallTemplate(const XMLCh *qname, const XMLCh *uri, const XMLCh *name, ASTNode *astName, TemplateArguments *args,
                 const UserFunctions &templates, XPath2MemoryManager *mm);

  virtual ASTNode *staticResolution(StaticContext *context);
  virtual ASTNode *staticTypingImpl(StaticContext *context);
  virtual Result createResult(DynamicContext* context, int flags=0) const;

  const XMLCh *getQName() const { return qname_; }
  void setQName(const XMLCh *qname) { qname_ = qname; }
  const XMLCh *getURI() const { return uri_; }
  void setURI(const XMLCh *uri) { uri_ = uri; }
  const XMLCh *getName() const { return name_; }
  void setName(const XMLCh *name) { name_ = name; }
  ASTNode *getASTName() const { return astName_; }
  void setASTName(ASTNode *name) { astName_ = name; }
  TemplateArguments *getArguments() const { return args_; }
  void setArguments(TemplateArguments *args) { args_ = args; }
  const UserFunctions &getTemplates() const { return templates_; }

private:
  const XMLCh *qname_, *uri_, *name_;
  ASTNode *astName_;
  TemplateArguments *args_;
  UserFunctions templates_;
};

#endif
