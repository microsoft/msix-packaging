/*
 * Copyright (c) 2001-2008
 *     DecisionSoft Limited. All rights reserved.
 * Copyright (c) 2004-2008
 *     Oracle. All rights reserved.
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
 *
 * $Id$
 */

#include <sstream>

#include <xqilla/framework/XQillaExport.hpp>
#include <xqilla/functions/XQUserFunction.hpp>
#include <xqilla/exceptions/FunctionException.hpp>
#include <xqilla/exceptions/XPath2TypeMatchException.hpp>
#include <xqilla/exceptions/StaticErrorException.hpp>
#include <xqilla/ast/XQSequence.hpp>
#include <xqilla/ast/XQVariable.hpp>
#include <xqilla/context/VariableStore.hpp>
#include <xqilla/context/VariableTypeStore.hpp>
#include <xqilla/utils/XPath2NSUtils.hpp>
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/context/ContextHelpers.hpp>
#include <xqilla/ast/XQTreatAs.hpp>
#include <xqilla/update/PendingUpdateList.hpp>
#include <xqilla/exceptions/StaticErrorException.hpp>
#include <xqilla/runtime/ClosureResult.hpp>
#include <xqilla/optimizer/ASTVisitor.hpp>
#include <xqilla/optimizer/StaticTyper.hpp>

#include "../lexer/XQLexer.hpp"

#include <xercesc/util/XMLString.hpp>
#include <xercesc/framework/XMLBuffer.hpp>
#include <xercesc/validators/schema/SchemaSymbols.hpp>
#include <xercesc/validators/datatype/DatatypeValidator.hpp>

XERCES_CPP_NAMESPACE_USE;
using namespace std;

 /* http://www.w3.org/2005/xquery-local-functions */
const XMLCh XQUserFunction::XMLChXQueryLocalFunctionsURI[] =
{
    chLatin_h,       chLatin_t,       chLatin_t, 
    chLatin_p,       chColon,         chForwardSlash, 
    chForwardSlash,  chLatin_w,       chLatin_w, 
    chLatin_w,       chPeriod,        chLatin_w,
    chDigit_3,       chPeriod,        chLatin_o, 
    chLatin_r,       chLatin_g,       chForwardSlash, 
    chDigit_2,       chDigit_0,       chDigit_0, 
    chDigit_5,       chForwardSlash,  chLatin_x,
    chLatin_q,       chLatin_u,       chLatin_e,
    chLatin_r,       chLatin_y,       chDash, 
    chLatin_l,       chLatin_o,       chLatin_c,
    chLatin_a,       chLatin_l,       chDash, 
    chLatin_f,       chLatin_u,       chLatin_n,
    chLatin_c,       chLatin_t,       chLatin_i,
    chLatin_o,       chLatin_n,       chLatin_s,
    chNull
};

XQUserFunction::XQUserFunction(const XMLCh *qname, FunctionSignature *signature,
                               ASTNode *body, bool isGlobal, XPath2MemoryManager *mm)
  : FuncFactory(signature->argSpecs == 0 ? 0 : signature->argSpecs->size(), mm),
    body_(body),
    exFunc_(NULL),
    qname_(qname),
    pattern_(0),
    templateInstance_(0),
    modes_(0),
    signature_(signature),
    isGlobal_(isGlobal),
    isTemplate_(false),
    memMgr_(mm),
    src_(mm),
    staticTyped_(BEFORE),
    recursive_(false),
    delayed_(false),
    moduleDocCache_(NULL)
{
}

XQUserFunction::XQUserFunction(const XMLCh *qname, VectorOfASTNodes *pattern, FunctionSignature *signature,
                               ASTNode *body, XPath2MemoryManager *mm)
  : FuncFactory(signature->argSpecs == 0 ? 0 : signature->argSpecs->size(), mm),
    body_(body),
    exFunc_(NULL),
    qname_(qname),
    pattern_(pattern),
    templateInstance_(0),
    modes_(0),
    signature_(signature),
    isGlobal_(true),
    isTemplate_(true),
    memMgr_(mm),
    src_(mm),
    staticTyped_(BEFORE),
    recursive_(false),
    delayed_(false),
    moduleDocCache_(NULL)
{
}

XQUserFunction::XQUserFunction(const XQUserFunction *o, XPath2MemoryManager *mm)
  : FuncFactory(o->uri_, o->name_, o->minArgs_, o->maxArgs_, mm),
    body_(o->body_),
    exFunc_(o->exFunc_),
    qname_(o->qname_),
    pattern_(0),
    templateInstance_(o->templateInstance_),
    modes_(0),
    signature_(new (mm) FunctionSignature(o->signature_, mm)),
    isGlobal_(o->isGlobal_),
    isTemplate_(o->isTemplate_),
    memMgr_(mm),
    src_(mm),
    staticTyped_(o->staticTyped_),
    recursive_(o->recursive_),
    delayed_(o->delayed_),
    moduleDocCache_(o->moduleDocCache_)
{
  if(o->pattern_) {
    pattern_ = new (mm) VectorOfASTNodes(XQillaAllocator<ASTNode*>(mm));
    *pattern_ = *o->pattern_;
  }
  if(o->modes_) {
    modes_ = new (mm) ModeList(XQillaAllocator<Mode*>(mm));

    XQUserFunction::ModeList::const_iterator modeIt = o->getModeList()->begin();
    for(; modeIt != o->getModeList()->end(); ++modeIt) {
      modes_->push_back(new (mm) Mode(*modeIt));
    }
  }
  setLocationInfo(o);
  src_.copy(o->src_);
}

void XQUserFunction::releaseImpl()
{
  if(pattern_) {
    pattern_->~VectorOfASTNodes();
    memMgr_->deallocate(pattern_);
  }

  if(modes_) {
    XQUserFunction::ModeList::iterator modeIt = modes_->begin();
    for(; modeIt != modes_->end(); ++modeIt) {
      memMgr_->deallocate(*modeIt);
    }
    
#if defined(_MSC_VER) && (_MSC_VER < 1300)
    modes_->~vector<Mode*,XQillaAllocator<Mode*> >();
#else
    modes_->~ModeList();
#endif
    memMgr_->deallocate(modes_);
  }

  signature_->release();

  src_.clear();
  memMgr_->deallocate(this);
}

ASTNode* XQUserFunction::createInstance(const VectorOfASTNodes &args, XPath2MemoryManager *mm) const
{
  return new (mm) XQUserFunctionInstance(this, args, mm);
}

void XQUserFunction::Mode::staticResolution(StaticContext* context)
{
  if(qname_ != 0) {
    uri_ = context->getUriBoundToPrefix(XPath2NSUtils::getPrefix(qname_, context->getMemoryManager()), this);
    name_ = XPath2NSUtils::getLocalName(qname_);
  }
}

bool XQUserFunction::Mode::equals(const Mode *o) const
{
  if(o == 0) return state_ == DEFAULT;
  if(state_ == ALL || o->state_ == CURRENT) return true;

  return state_ == o->state_ && XPath2Utils::equals(uri_, o->uri_) &&
    XPath2Utils::equals(name_, o->name_);
}

void XQUserFunction::resolveName(StaticContext *context)
{
  XPath2MemoryManager *mm = context->getMemoryManager();

  if(qname_ != 0 && name_ == 0) {
    const XMLCh *prefix = XPath2NSUtils::getPrefix(qname_, mm);
    name_ = XPath2NSUtils::getLocalName(qname_);

    if(prefix == 0 || *prefix == 0) {
      uri_ = context->getDefaultFuncNS();
    }
    else {
      uri_ = context->getUriBoundToPrefix(prefix, this);
    }
  }

  if(name_ != 0) {
    setURINameHash(uri_, name_);
  }
}

void XQUserFunction::staticResolutionStage1(StaticContext *context)
{
  XPath2MemoryManager *mm = context->getMemoryManager();

  resolveName(context);

  if(name_ != 0 && !isTemplate_ && !delayed_) {
    if(XPath2Utils::equals(uri_, XMLUni::fgXMLURIName) ||
       XPath2Utils::equals(uri_, SchemaSymbols::fgURI_SCHEMAFORSCHEMA) ||
       XPath2Utils::equals(uri_, SchemaSymbols::fgURI_XSI) ||
       XPath2Utils::equals(uri_, XQFunction::XMLChFunctionURI) ||
       XPath2Utils::equals(uri_, SchemaSymbols::fgURI_SCHEMAFORSCHEMA)) {
      XQThrow(FunctionException, X("XQUserFunction::staticResolutionStage1"),
              X("A user defined function must not be in the namespaces xml, xsd, xsi, fn or xdt [err:XQST0045]"));
    }
    else if(uri_ == 0 || *uri_ == 0)
      XQThrow(FunctionException, X("XQUserFunction::staticResolutionStage1"),
              X("A user defined function must be defined in a namespace [err:XQST0060]"));
  }

  // Check for the implementation of an external function
  if(body_ == NULL) {
    exFunc_ = context->lookUpExternalFunction(uri_, name_, minArgs_);

    if(exFunc_ == NULL) {
      XMLBuffer buf;
      buf.set(X("External function '{"));
      buf.append(uri_);
      buf.append(X("}"));
      buf.append(name_);
      buf.append(X("' with "));
      XMLCh szNumBuff[20];
      XMLString::binToText((unsigned int)minArgs_, szNumBuff, 19, 10);
      buf.append(szNumBuff);
      buf.append(X(" argument(s) has not been bound to an implementation"));
      XQThrow(FunctionException, X("XQUserFunction::staticResolutionStage1"), buf.getRawBuffer());
    }
  }

  signature_->staticResolution(context);

  // Resolve the mode names
  if(modes_) {
    if(modes_->empty()) {
      XQThrow(StaticErrorException, X("XQUserFunction::staticResolution"),
              X("At least one mode must be specified for a template [err:XTSE0550]"));
    }

    ModeList::iterator it, it2;
    for(it = modes_->begin(); it != modes_->end(); ++it) {
      (*it)->staticResolution(context);

      // Check for "#all" with other values
      if((*it)->getState() == Mode::ALL && modes_->size() != 1) {
        XQThrow3(StaticErrorException, X("XQUserFunction::staticResolution"),
                 X("The mode #all must not be specified in addition to other modes [err:XTSE0550]"), *it);
      }

      // Check for duplicate modes
      it2 = it;
      for(++it2; it2 != modes_->end(); ++it2) {
        if((*it)->getState() == (*it2)->getState() &&
           XPath2Utils::equals((*it)->getName(), (*it2)->getName()) &&
           XPath2Utils::equals((*it)->getURI(), (*it2)->getURI())) {
          XMLBuffer buf;
          buf.append(X("The mode {"));
          buf.append((*it)->getURI());
          buf.append(X("}"));
          buf.append((*it)->getName());
          buf.append(X(" has been specified more than once [err:XTSE0550]"));
          XQThrow3(StaticErrorException, X("XQUserFunction::staticResolution"), buf.getRawBuffer(), *it2);
        }
      }
    }
  }

  // Set up a default StaticType and StaticAnalysis
  if(signature_->returnType) {
    if(body_ != NULL) {
      body_ = signature_->returnType->convertFunctionArg(body_, context, /*numericfunction*/false, signature_->returnType);
    }

    bool isPrimitive;
    signature_->returnType->getStaticType(src_.getStaticType(), context, isPrimitive, signature_->returnType);
  }
  else {
    // Default type is item()*
    src_.getStaticType() = StaticType(StaticType::ITEM_TYPE, 0, StaticType::UNLIMITED);
  }

  if(signature_->updating == FunctionSignature::OP_TRUE) {
    src_.updating(true);
  }

  // TBD What about the other parts of the StaticAnalysis - jpcs
  src_.forceNoFolding(true);

  if(pattern_ != 0 && !pattern_->empty()) {
    // Set the pattern's initial static type to NODE_TYPE
    VectorOfASTNodes::iterator patIt = pattern_->begin();
    for(; patIt != pattern_->end(); ++patIt) {
      const_cast<StaticAnalysis&>((*patIt)->getStaticAnalysis()).getStaticType() = StaticType(StaticType::NODE_TYPE, 0, StaticType::UNLIMITED);
    }
  }

  if(isTemplate_) {
    // Build an instance of the template for us to call
    VectorOfASTNodes newArgs = VectorOfASTNodes(XQillaAllocator<ASTNode*>(mm));

    if(signature_->argSpecs != 0) {
      ArgumentSpecs::const_iterator argIt;
      for(argIt = signature_->argSpecs->begin(); argIt != signature_->argSpecs->end(); ++argIt) {
        XQVariable *argVar = new (mm) XQVariable((*argIt)->getURI(), (*argIt)->getName(), mm);
        argVar->setLocationInfo(*argIt);
        newArgs.push_back(argVar);
      }
    }

    templateInstance_ = createInstance(newArgs, mm);
    templateInstance_->setLocationInfo(this);
  }
}

void XQUserFunction::staticResolutionStage2(StaticContext *context)
{
  // Prevent static typing being run on this function by a
  // loop of delayed functions
  AutoReset<StaticTypingStatus> reset(staticTyped_);
  staticTyped_ = AFTER;

  if(pattern_ != 0 && !pattern_->empty()) {
    VectorOfASTNodes::iterator patIt = pattern_->begin();
    for(; patIt != pattern_->end(); ++patIt) {
      (*patIt) = (*patIt)->staticResolution(context);
    }
  }
  if(templateInstance_) {
    templateInstance_ = templateInstance_->staticResolution(context);
  }
  if(body_ != NULL) {
    body_ = body_->staticResolution(context);
  }
}

class UDFStaticTyper : private ASTVisitor
{
public:
  UDFStaticTyper() : context_(0), styper_(0) {}

  void run(ASTNode *item, StaticContext *context, StaticTyper *styper)
  {
    context_ = context;
    styper_ = styper;
    optimize(item);
  }

private:
  virtual ASTNode *optimizeUserFunction(XQUserFunctionInstance *item)
  {
    XQQuery *module = context_->getModule()->
      findModuleForFunction(item->getFunctionURI(), item->getFunctionName(), item->getArguments().size());
    if(module == context_->getModule()) {
      // See if we can work out a better return type for the user defined function.
      // This call will just return if it's already been static typed
      const_cast<XQUserFunction*>(item->getFunctionDefinition())->staticTypingOnce(context_, styper_);
    }
    return ASTVisitor::optimizeUserFunction(item);
  }

  virtual ASTNode *optimizeApplyTemplates(XQApplyTemplates *item)
  {
    // The XQApplyTemplates could call any template with a pattern -
    // so try to static type all of them before us

    const UserFunctions &templates = context_->getTemplateRules();

    UserFunctions::const_iterator inIt;
    for(inIt = templates.begin(); inIt != templates.end(); ++inIt) {
      if((*inIt)->getPattern() != 0)
        (*inIt)->staticTypingOnce(context_, styper_);
    }

    return ASTVisitor::optimizeApplyTemplates(item);
  }

  virtual ASTNode *optimizeCallTemplate(XQCallTemplate *item)
  {
    // The XQCallTemplate could call any template with a name -
    // so try to static type all of them before us

    const UserFunctions &templates = context_->getTemplateRules();

    UserFunctions::const_iterator inIt;
    for(inIt = templates.begin(); inIt != templates.end(); ++inIt) {
      if((*inIt)->getName() != 0)
        (*inIt)->staticTypingOnce(context_, styper_);
    }

    return ASTVisitor::optimizeCallTemplate(item);
  }

  virtual ASTNode *optimizeInlineFunction(XQInlineFunction *item)
  {
    return item;
  }

  virtual ASTNode *optimizeFunctionRef(XQFunctionRef *item)
  {
    return item;
  }

  StaticContext *context_;
  StaticTyper *styper_;
};

void XQUserFunction::staticTypeFunctionCalls(ASTNode *item, StaticContext *context, StaticTyper *styper)
{
  // TBD DB XML version of UDFStaticTyper? - jpcs
  UDFStaticTyper().run(item, context, styper);
}

void XQUserFunction::staticTypingOnce(StaticContext *context, StaticTyper *styper)
{
  // Avoid inifinite recursion for recursive functions
  // TBD Need to declare everything as being used - jpcs
  if(staticTyped_ != BEFORE) {
    if(staticTyped_ == DURING)
      recursive_ = true;

    XQGlobalVariable *global = 0;
    StaticTyper::PrologItem *breadcrumb = styper->getTrail();
    for(; breadcrumb; breadcrumb = breadcrumb->prev) {
      if(breadcrumb->global) global = breadcrumb->global;
      if(breadcrumb->function == this) break;
    }

    if(global && breadcrumb) {
      XMLBuffer buf;
      buf.append(X("The initializing expression for variable {"));
      buf.append(global->getVariableURI());
      buf.append(X("}"));
      buf.append(global->getVariableLocalName());
      buf.append(X(" depends on itself [err:XQST0054]"));
      XQThrow3(StaticErrorException, X("XQUserFunction::staticTypingOnce"), buf.getRawBuffer(), global);
    }

    return;
  }
  staticTyped_ = DURING;

  StaticTyper::PrologItem breadcrumb(this, styper->getTrail());
  AutoReset<StaticTyper::PrologItem*> autorReset2(styper->getTrail());
  styper->getTrail() = &breadcrumb;;

  GlobalVariables globalsUsed(XQillaAllocator<XQGlobalVariable*>(context->getMemoryManager()));
  {
    AutoReset<GlobalVariables*> autoReset(styper->getGlobalsUsed());
    styper->getGlobalsUsed() = &globalsUsed;
    staticTyping(context, styper);
  }

  if(!globalsUsed.empty()) {
    // Static type the global variables we depend on
    GlobalVariables::iterator it = globalsUsed.begin();
    for(; it != globalsUsed.end(); ++it) {
      (*it)->staticTypingOnce(context, styper);
    }

    // Re-static type this function definition
    staticTyping(context, styper);
  }

  staticTyped_ = AFTER;
}

void XQUserFunction::resetStaticTypingOnce()
{
  staticTyped_ = BEFORE;
}

void XQUserFunction::staticTyping(StaticContext *context, StaticTyper *styper)
{
  // Nothing more to do for external functions
  if(body_ == NULL) return;

  if(signature_->updating == FunctionSignature::OP_TRUE && signature_->returnType != NULL) {
    XQThrow(StaticErrorException, X("XQUserFunction::staticTyping"),
            X("It is a static error for an updating function to declare a return type [err:XUST0028]"));
  }

  // Find user defined functions and templates that are referenced in our body,
  // and try to call staticTyping() on them before us.
  if(context) staticTypeFunctionCalls(body_, context, styper);

  bool ciTypeSet = false;
  StaticType ciType = StaticType();
  if(pattern_ != NULL) {
    VectorOfASTNodes::iterator patIt = pattern_->begin();
    for(; patIt != pattern_->end(); ++patIt) {
      (*patIt) = (*patIt)->staticTyping(context, styper);
      if(!ciTypeSet) {
        ciTypeSet = true;
        ciType = (*patIt)->getStaticAnalysis().getStaticType();
      }
      else ciType |= (*patIt)->getStaticAnalysis().getStaticType();
    }
    if(ciTypeSet) {
      ciType.setCardinality(1, 1);
    }
  }
  if(isTemplate_ && name_ != 0) {
    // Named template
    ciTypeSet = true;
    ciType = StaticType::ITEM_TYPE;
  }

  // define the new variables in a new scope and assign them the proper values
  if(context) {
    VariableTypeStore *varStore = context->getVariableTypeStore();

    if(isGlobal_) varStore->addLocalScope();
    else varStore->addLogicalBlockScope();

    // Declare the parameters
    if(signature_->argSpecs) {
      ArgumentSpecs::iterator it;
      for(it = signature_->argSpecs->begin(); it != signature_->argSpecs->end (); ++it) {
        varStore->declareVar((*it)->getURI(), (*it)->getName(), (*it)->getStaticAnalysis());
      }
    }
  }

  {
    // Declare the context item
    AutoContextItemTypeReset contextTypeReset(context, ciType);
    body_ = body_->staticTyping(context, styper);
  }

  if(context)
    context->getVariableTypeStore()->removeScope();

  if(signature_->updating == FunctionSignature::OP_TRUE) {
    if(!body_->getStaticAnalysis().isUpdating() && !body_->getStaticAnalysis().isPossiblyUpdating())
      XQThrow(StaticErrorException, X("XQUserFunction::staticTyping"),
              X("It is a static error for the body expression of a user defined updating function "
                "not to be an updating expression [err:XUST0002]"));
  }
  else {
    if(body_->getStaticAnalysis().isUpdating()) {
      if(isTemplate_) {
        XQThrow(StaticErrorException, X("XQUserFunction::staticTyping"),
                X("It is a static error for the body expression of a template "
                  "to be an updating expression [err:XUST0001]"));
      } else {
        XQThrow(StaticErrorException, X("XQUserFunction::staticTyping"),
                X("It is a static error for the body expression of a user defined function "
                  "to be an updating expression [err:XUST0001]"));
      }
    }
  }

  // Remove the parameter variables from the stored StaticAnalysis
  src_.clear();
  src_.copy(body_->getStaticAnalysis());
  if(signature_->argSpecs) {
    for(ArgumentSpecs::iterator it = signature_->argSpecs->begin(); it != signature_->argSpecs->end (); ++it) {
      if(!src_.removeVariable((*it)->getURI(), (*it)->getName())) {
        // The parameter isn't used, so set it to null, so that we don't bother to evaluate it
        (*it)->setNotUsed();
      }
    }
  }

  // Run staticTyping on the template instances
  if(templateInstance_ != 0 && context) {
    StaticAnalysis templateVarSrc(context->getMemoryManager());
    templateVarSrc.getStaticType() = StaticType::ITEM_TYPE;

    VariableTypeStore *varStore = context->getVariableTypeStore();
    varStore->addLogicalBlockScope();

    if(signature_->argSpecs != 0) {
      ArgumentSpecs::const_iterator argIt;
      for(argIt = signature_->argSpecs->begin(); argIt != signature_->argSpecs->end(); ++argIt) {
        varStore->declareVar((*argIt)->getURI(), (*argIt)->getName(), templateVarSrc);
      }
    }

    // Turn off warnings here, since they are largely irrelevent to the user
    AutoMessageListenerReset reset(context);

    templateInstance_ = templateInstance_->staticTyping(context, styper);

    varStore->removeScope();
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

XQUserFunctionInstance::XQUserFunctionInstance(const XQUserFunction* funcDef, const VectorOfASTNodes& args, XPath2MemoryManager* expr) 
  : XQFunction(funcDef->getName(), 0, args, expr),
    addReturnCheck_(funcDef->body_ == NULL && funcDef->signature_->returnType != NULL),
    funcDef_(funcDef)
{
  _type = ASTNode::USER_FUNCTION;
  uri_ = funcDef->getURI();
  signature_ = funcDef->getSignature();
}

XQUserFunctionInstance::XQUserFunctionInstance(const XQUserFunction *funcDef, const VectorOfASTNodes& args, bool addReturnCheck, XPath2MemoryManager *mm)
  : XQFunction(funcDef->getName(), 0, args, mm),
    addReturnCheck_(addReturnCheck),
    funcDef_(funcDef)
{
  _type = ASTNode::USER_FUNCTION;
  uri_ = funcDef->getURI();
  signature_ = funcDef->getSignature();
}

Result XQUserFunctionInstance::getArgument(size_t index, DynamicContext *context) const
{
  if(index >= funcDef_->getMaxArgs()) return 0;

  return _args[index]->createResult(context);
}

ASTNode* XQUserFunctionInstance::staticResolution(StaticContext* context)
{
  XPath2MemoryManager *mm = context->getMemoryManager();

  // We don't trust external functions, so check their return type
  if(addReturnCheck_) {
    addReturnCheck_ = false;
    XQTreatAs *treatAs = new (mm) XQTreatAs(this, funcDef_->signature_->returnType, mm);
    treatAs->setLocationInfo(funcDef_->signature_->returnType);
    return treatAs->staticResolution(context);
  }

  if(funcDef_->signature_->argSpecs != 0) {
    VectorOfASTNodes::iterator argIt = _args.begin();
    for(ArgumentSpecs::iterator defIt = funcDef_->signature_->argSpecs->begin();
        defIt != funcDef_->signature_->argSpecs->end() && argIt != _args.end(); ++defIt, ++argIt) {
      // The spec doesn't allow us to skip static errors, so we have to check even if
      // the parameter isn't used
      *argIt = (*defIt)->getType()->convertFunctionArg(*argIt, context, /*numericfunction*/false, *argIt);
      *argIt = (*argIt)->staticResolution(context);
    }
  }

  return this;
}

ASTNode *XQUserFunctionInstance::staticTypingImpl(StaticContext *context)
{
  if(funcDef_->body_ != NULL) {
    _src.clear();
    _src.copy(funcDef_->src_);
  } else {
    // Force the type check to happen, by declaring our type as item()*
    _src.clear();
    _src.getStaticType() = StaticType(StaticType::ITEM_TYPE, 0, StaticType::UNLIMITED);
    _src.forceNoFolding(true);
  }

  VectorOfASTNodes::iterator argIt;
  for(argIt = _args.begin(); argIt != _args.end(); ++argIt) {
    // The spec doesn't allow us to skip static errors, so we have to check even if
    // the parameter isn't used
    if((*argIt)->getStaticAnalysis().isUpdating()) {
      if(funcDef_->isTemplate()) {
        XQThrow(StaticErrorException, X("XQUserFunctionInstance::staticTyping"),
                X("It is a static error for the argument expression of a call template expression "
                  "to be an updating expression [err:XUST0001]"));
      } else {
        XQThrow(StaticErrorException, X("XQUserFunctionInstance::staticTyping"),
                X("It is a static error for the argument expression of a function call expression "
                  "to be an updating expression [err:XUST0001]"));
      }
    }

    // TBD Check all static errors in staticResolution, so we can skip static typing - jpcs
    // if((*defIt)->_qname)
    _src.add((*argIt)->getStaticAnalysis());
  }

  return this;
}

void XQUserFunctionInstance::evaluateArguments(VarStoreImpl &scope, DynamicContext *context) const
{
  if(funcDef_->getSignature()->argSpecs != 0) {
    // the variables should be evaluated in the calling context
    // (before the VariableStore::addLocalScope call: after this call, the variables that can be seen are only the local ones)
    VectorOfASTNodes::const_iterator argIt = _args.begin();
    for(ArgumentSpecs::const_iterator defIt = funcDef_->signature_->argSpecs->begin();
        defIt != funcDef_->signature_->argSpecs->end() && argIt != _args.end(); ++defIt, ++argIt) {
      if((*defIt)->isUsed()) {
        scope.setVar((*defIt)->getURI(), (*defIt)->getName(), (*argIt)->createResult(context));
      }
      else {
        // Skip evaluation of the parameter, since it isn't used, and debugging isn't enabled
      }
    }
  }
}

EventGenerator::Ptr XQUserFunctionInstance::generateEvents(EventHandler *events, DynamicContext *context,
                                                           bool preserveNS, bool preserveType) const
{
  if(funcDef_->getFunctionBody() == NULL) {
    return ASTNodeImpl::generateEvents(events, context, preserveNS, preserveType);
  }

  context->testInterrupt();

  VarStoreImpl scope(context->getMemoryManager(), funcDef_->isGlobal() ?
                     context->getGlobalVariableStore() : context->getVariableStore());
  evaluateArguments(scope, context);

  AutoDocumentCacheReset reset(context);
  if(funcDef_->getModuleDocumentCache())
    context->setDocumentCache(funcDef_->getModuleDocumentCache());

  AutoVariableStoreReset reset2(context, &scope);
  AutoRegexGroupStoreReset reset3(context);
  if(!funcDef_->isTemplate()) context->setRegexGroupStore(0);

  return new ClosureEventGenerator(funcDef_->getFunctionBody(), context, preserveNS, preserveType);
}

PendingUpdateList XQUserFunctionInstance::createUpdateList(DynamicContext *context) const
{
  context->testInterrupt();

  if(funcDef_->getFunctionBody() == NULL) {
    return funcDef_->getExternalFunction()->executeUpdate(this, context);
  }

  VarStoreImpl scope(context->getMemoryManager(), funcDef_->isGlobal() ?
                     context->getGlobalVariableStore() : context->getVariableStore());
  evaluateArguments(scope, context);

  AutoDocumentCacheReset reset(context);
  if(funcDef_->getModuleDocumentCache())
    context->setDocumentCache(funcDef_->getModuleDocumentCache());

  AutoVariableStoreReset reset2(context, &scope);
  AutoRegexGroupStoreReset reset3(context, 0);
  if(!funcDef_->isTemplate()) context->setRegexGroupStore(0);
  PendingUpdateList result = funcDef_->getFunctionBody()->createUpdateList(context);

  return result;
}

Result XQUserFunctionInstance::createResult(DynamicContext* context, int flags) const
{
  context->testInterrupt();

  if(funcDef_->body_ != NULL) {
    VarStoreImpl scope(context->getMemoryManager(), funcDef_->isGlobal() ?
                       context->getGlobalVariableStore() : context->getVariableStore());
    evaluateArguments(scope, context);

    AutoRegexGroupStoreReset reset3(context);
    if(!funcDef_->isTemplate()) context->setRegexGroupStore(0);

    AutoDocumentCacheReset reset(context);
    DocumentCache* docCache = funcDef_->getModuleDocumentCache();
    if(docCache) context->setDocumentCache(docCache);

    AutoVariableStoreReset vsReset(context, &scope);
    return ClosureResult::create(funcDef_->getFunctionBody(), context);
  } else {
    return funcDef_->getExternalFunction()->execute(this, context);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

DelayedFuncFactory::DelayedFuncFactory(const XMLCh *uri, const XMLCh *name, size_t numArgs,
                                       const XMLCh *body, int line, int column, XQQuery *query)
  : FuncFactory(uri, name, numArgs, numArgs, query->getStaticContext()->getMemoryManager()),
    body_(body),
    body8_(0),
    line_(line),
    column_(column),
    query_(query),
    function_(0)
{
}

DelayedFuncFactory::DelayedFuncFactory(const XMLCh *uri, const XMLCh *name, size_t numArgs,
                                       const char *body, int line, int column, XQQuery *query)
  : FuncFactory(uri, name, numArgs, numArgs, query->getStaticContext()->getMemoryManager()),
    body_(0),
    body8_(body),
    line_(line),
    column_(column),
    query_(query),
    function_(0)
{
}

ASTNode *DelayedFuncFactory::createInstance(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr) const
{
  if(function_ == 0) {
    DynamicContext *context = (DynamicContext*)query_->getStaticContext();

    if(body_ == 0) {
      const_cast<const XMLCh *&>(body_) = context->getMemoryManager()->getPooledString(body8_);
    }

    XQLexer lexer(memMgr, _LANG_FUNCDECL_, query_->getFile(), line_, column_, body_);
    XQParserArgs args(&lexer, query_);
    XQParser::yyparse(&args);

    const_cast<XQUserFunction*&>(function_) = args._function;
    function_->setDelayed(true);

    query_->addFunction(function_);
    function_->staticResolutionStage1(context);

    AutoDelete<Optimizer> optimizer(XQilla::createOptimizer(context, XQilla::NO_OPTIMIZATION));
    optimizer->startOptimize(function_);
  }
  return function_->createInstance(args, memMgr);
}

