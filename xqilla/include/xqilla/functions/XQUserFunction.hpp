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

#ifndef XQUSERFUNCTION_HPP
#define XQUSERFUNCTION_HPP

#include <xqilla/framework/XQillaExport.hpp>
#include <xqilla/functions/FuncFactory.hpp>
#include <xqilla/functions/ExternalFunction.hpp>
#include <xqilla/ast/XQFunction.hpp>
#include <xqilla/ast/StaticAnalysis.hpp>

class XQUserFunction;
class XQQuery;
class FunctionSignature;
class DocumentCache;
class VarStoreImpl;

class XQILLA_API XQUserFunctionInstance : public XQFunction, public ExternalFunction::Arguments
{
public:
  XQUserFunctionInstance(const XQUserFunction *funcDef, const VectorOfASTNodes& args, XPath2MemoryManager *mm);
  XQUserFunctionInstance(const XQUserFunction *funcDef, const VectorOfASTNodes& args, bool addReturnCheck, XPath2MemoryManager *mm);

  virtual Result getArgument(size_t index, DynamicContext *context) const;

  Result createResult(DynamicContext* context, int flags=0) const;
  virtual EventGenerator::Ptr generateEvents(EventHandler *events, DynamicContext *context,
                                bool preserveNS, bool preserveType) const;
  ASTNode* staticResolution(StaticContext* context);
  virtual ASTNode *staticTypingImpl(StaticContext *context);
  virtual PendingUpdateList createUpdateList(DynamicContext *context) const;

  void evaluateArguments(VarStoreImpl &scope, DynamicContext *context) const;

  const XQUserFunction *getFunctionDefinition() const
  {
    return funcDef_;
  }
  void setFunctionDefinition(const XQUserFunction *f) { funcDef_ = f; }

  bool getAddReturnCheck() const { return addReturnCheck_; }

protected:
  bool addReturnCheck_;
  const XQUserFunction *funcDef_;
};

class XQILLA_API XQUserFunction : public FuncFactory, public LocationInfo
{
public:
  class XQILLA_API Mode : public LocationInfo
  {
  public:
    enum State {
      QNAME,
      DEFAULT,
      ALL,
      CURRENT
    };

    Mode(const XMLCh *qname) : state_(QNAME), qname_(qname), uri_(0), name_(0) {}
    Mode(const XMLCh *uri, const XMLCh *name) : state_(QNAME), qname_(0), uri_(uri), name_(name) {}
    Mode(State state) : state_(state), qname_(0), uri_(0), name_(0) {}
    Mode(const Mode *o) : state_(o->state_), qname_(o->qname_), uri_(o->uri_), name_(o->name_) {}

    State getState() const { return state_; }

    const XMLCh *getURI() const { return uri_; }
    void setURI(const XMLCh *uri) { uri_ = uri; }
    const XMLCh *getName() const { return name_; }
    void setName(const XMLCh *name) { name_ = name; }
    const XMLCh *getQName() const { return qname_; }

    bool equals(const Mode *o) const;

    void staticResolution(StaticContext* context);

  private:
    State state_;
    const XMLCh *qname_, *uri_, *name_;
  };

  typedef std::vector<Mode*,XQillaAllocator<Mode*> > ModeList;

  // Constructor for an XQuery function
  XQUserFunction(const XMLCh *qname, FunctionSignature *signature, ASTNode *body, bool isGlobal, XPath2MemoryManager *mm);
  // Constructor for an XQuery template
  XQUserFunction(const XMLCh *qname, VectorOfASTNodes *pattern, FunctionSignature *signature, ASTNode *body, XPath2MemoryManager *mm);

  // from FuncFactory
  virtual ASTNode *createInstance(const VectorOfASTNodes &args, XPath2MemoryManager* expr) const;
  virtual const XMLCh *getQName() const { return qname_; }
  void setName(const XMLCh *name) { name_ = name; }
  void setURI(const XMLCh *uri) { uri_ = uri; }

  VectorOfASTNodes *getPattern() const { return pattern_; }
  void setPattern(VectorOfASTNodes *pattern) { pattern_ = pattern; }

  ModeList *getModeList() const { return modes_; }
  void setModeList(ModeList *modes) { modes_ = modes; }

  FunctionSignature *getSignature() const { return signature_; }

  ASTNode *getTemplateInstance() const { return templateInstance_; }
  void setTemplateInstance(ASTNode *ast) { templateInstance_ = ast; }

  bool isGlobal() const { return isGlobal_; }
  bool isTemplate() const { return isTemplate_; }

  bool isRecursive() const { return recursive_; }

  bool isDelayed() const { return delayed_; }
  void setDelayed(bool v) { delayed_ = v; }

  void resolveName(StaticContext *context);
  /// Resolve URIs, give the function a default static type
  void staticResolutionStage1(StaticContext* context);
  /// Resolve the function body, work out a more static return type
  void staticResolutionStage2(StaticContext* context);
  void staticTypingOnce(StaticContext *context, StaticTyper *styper);
  void resetStaticTypingOnce();
  void staticTyping(StaticContext *context, StaticTyper *styper);

  void setFunctionBody(ASTNode* value) { body_ = value; }
  void setModuleDocumentCache(DocumentCache* docCache) { moduleDocCache_ = docCache; }

  const ASTNode *getFunctionBody() const { return body_; }
  const ExternalFunction *getExternalFunction() const { return exFunc_; }
  DocumentCache *getModuleDocumentCache() const { return moduleDocCache_; }
  const StaticAnalysis &getBodyStaticAnalysis() const { return src_; }

  XPath2MemoryManager *getMemoryManager() const { return memMgr_; }

  static void staticTypeFunctionCalls(ASTNode *item, StaticContext *context, StaticTyper *styper);

  static const XMLCh XMLChXQueryLocalFunctionsURI[];

protected:
  // Constructor for copying
  XQUserFunction(const XQUserFunction *o, XPath2MemoryManager *mm);
  // Implementation for releasing
  void releaseImpl();

  ASTNode *body_;
  const ExternalFunction *exFunc_;
  const XMLCh *qname_;

  VectorOfASTNodes *pattern_;
  ASTNode *templateInstance_;
  ModeList *modes_;

  FunctionSignature *signature_;

  bool isGlobal_;
  bool isTemplate_;

  XPath2MemoryManager *memMgr_;
  StaticAnalysis src_;
  enum StaticTypingStatus { BEFORE, DURING, AFTER } staticTyped_;
  bool recursive_;
  bool delayed_;
  DocumentCache *moduleDocCache_;

  friend class XQUserFunctionInstance;
  friend class ASTCopier;
  friend class ASTReleaser;
};

class XQILLA_API DelayedFuncFactory : public FuncFactory
{
public:
  DelayedFuncFactory(const XMLCh *uri, const XMLCh *name, size_t numArgs,
	  const XMLCh *body, int line, int column, XQQuery *query);
  DelayedFuncFactory(const XMLCh *uri, const XMLCh *name, size_t numArgs,
	  const char *body, int line, int column, XQQuery *query);
  virtual ASTNode *createInstance(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr) const;

  bool isParsed() const { return function_ != 0; }

private:
  const XMLCh *body_;
  const char *body8_;
  int line_, column_;
  XQQuery *query_;
  XQUserFunction *function_;
};

#endif
