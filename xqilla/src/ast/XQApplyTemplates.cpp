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

#include <xqilla/ast/XQApplyTemplates.hpp>
#include <xqilla/ast/XQTreatAs.hpp>
#include <xqilla/exceptions/StaticErrorException.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/context/ContextHelpers.hpp>
#include <xqilla/events/SequenceBuilder.hpp>
#include <xqilla/utils/XPath2NSUtils.hpp>
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/runtime/ClosureResult.hpp>
#include <xqilla/schema/SequenceType.hpp>
#include <xqilla/functions/FunctionSignature.hpp>

#include <xercesc/framework/XMLBuffer.hpp>

using namespace std;
XERCES_CPP_NAMESPACE_USE;

XQApplyTemplates::XQApplyTemplates(ASTNode *expr, TemplateArguments *args, XQUserFunction::Mode *mode, XPath2MemoryManager *mm)
  : ASTNodeImpl(APPLY_TEMPLATES, mm),
    expr_(expr),
    args_(args),
    mode_(mode),
    templates_(XQillaAllocator<XQUserFunction*>(mm))
{
}

XQApplyTemplates::XQApplyTemplates(ASTNode *expr, TemplateArguments *args, XQUserFunction::Mode *mode,
                                   const UserFunctions &templates, XPath2MemoryManager *mm)
  : ASTNodeImpl(APPLY_TEMPLATES, mm),
    expr_(expr),
    args_(args),
    mode_(mode),
    templates_(XQillaAllocator<XQUserFunction*>(mm))
{
  templates_ = templates;
}

static const XMLCh err_XTTE0520[] = { 'e', 'r', 'r', ':', 'X', 'T', 'T', 'E', '0', '5', '2', '0', 0 };

ASTNode* XQApplyTemplates::staticResolution(StaticContext *context) 
{
  XPath2MemoryManager *mm = context->getMemoryManager();

  SequenceType *seqType = new (mm) SequenceType(new (mm) SequenceType::ItemType(SequenceType::ItemType::TEST_NODE),
                                                SequenceType::STAR);
  seqType->setLocationInfo(this);

  expr_ = new (mm) XQTreatAs(expr_, seqType, mm, err_XTTE0520);
  expr_->setLocationInfo(this);

  expr_ = expr_->staticResolution(context);

  // Resolve the call template argument names and check for duplicates
  TemplateArguments::iterator it;
  if(args_ != 0) {
    for(it = args_->begin(); it != args_->end(); ++it) {
      if((*it)->name == 0) {
        (*it)->uri = context->getUriBoundToPrefix(XPath2NSUtils::getPrefix((*it)->qname, mm), this);
        (*it)->name = XPath2NSUtils::getLocalName((*it)->qname);
      }
      (*it)->value = (*it)->value->staticResolution(context);

      for(TemplateArguments::iterator it2 = args_->begin(); it2 != it; ++it2) {
        if(XPath2Utils::equals((*it)->name, (*it2)->name) &&
           XPath2Utils::equals((*it)->uri, (*it2)->uri)) {
          XMLBuffer buf;
          buf.set(X("Two template parameters have the name {"));
          buf.append((*it)->uri);
          buf.append(X("}"));
          buf.append((*it)->name);
          buf.append(X(" [err:XTSE0670]"));

          XQThrow3(StaticErrorException, X("XQApplyTemplates::staticResolution"), buf.getRawBuffer(), *it);
        }
      }
    }
  }

  if(mode_ != 0) {
    mode_->staticResolution(context);
  }

  return this;
}

ASTNode *XQApplyTemplates::staticTypingImpl(StaticContext *context)
{
  _src.clear();

  // Calculate our static type from the template instances
  if(context)
    templates_ = context->getTemplateRules();

  bool first = true;

  UserFunctions::iterator inIt;
  VectorOfASTNodes::iterator patIt;
  XQUserFunction::ModeList::iterator modeIt;
  bool found;
  for(inIt = templates_.begin(); inIt != templates_.end(); ++inIt) {
    if((*inIt) == 0) continue;

    if((*inIt)->getPattern() == 0) {
      *inIt = 0;
      continue;
    }

    // Eliminate templates based on mode
    found = false;
    for(modeIt = (*inIt)->getModeList()->begin();
        modeIt != (*inIt)->getModeList()->end(); ++modeIt) {
      if((*modeIt)->equals(mode_)) {
        found = true;
        break;
      }
    }

    if(!found) {
      // We can't match this template
      *inIt = 0;
      continue;
    }

    // See if we can eliminate some templates based on the static type
    // of the select expression
    found = false;
    for(patIt = (*inIt)->getPattern()->begin();
        patIt != (*inIt)->getPattern()->end(); ++patIt) {
      if((*patIt)->getStaticAnalysis().getStaticType().containsType(expr_->getStaticAnalysis().getStaticType())) {
        found = true;
        break;
      }
    }

    if(!found) {
      // We can't match this template
      *inIt = 0;
      continue;
    }

    if(first) {
      first = false;
      _src.getStaticType() = (*inIt)->getBodyStaticAnalysis().getStaticType();
      _src.setProperties((*inIt)->getBodyStaticAnalysis().getProperties());
    } else {
      _src.getStaticType() |= (*inIt)->getBodyStaticAnalysis().getStaticType();
      _src.setProperties(_src.getProperties() & (*inIt)->getBodyStaticAnalysis().getProperties());
    }
    _src.add((*inIt)->getBodyStaticAnalysis());
  }

  TemplateArguments::iterator it;

  if(args_ != 0) {
    for(it = args_->begin(); it != args_->end(); ++it) {
      if(!_src.removeVariable((*it)->uri, (*it)->name)) {
        // TBD This parameter will never be used - jpcs
      }
    }
  }

  _src.add(expr_->getStaticAnalysis());

  if(args_ != 0) {
    for(it = args_->begin(); it != args_->end(); ++it) {
      _src.add((*it)->value->getStaticAnalysis());
    }
  }

  return this;
}

Result XQApplyTemplates::executeTemplate(const XQUserFunction *tplt, const TemplateArguments *args, const VariableStore *scope,
                                         DynamicContext *context, const LocationInfo *location)
{
  VarStoreImpl localscope(context->getMemoryManager(), context->getVariableStore());
  if(scope == 0) scope = &localscope;

  // Check to see if we set the correct parameters for the template
  if(tplt->getSignature()->argSpecs != 0) {
    ArgumentSpecs::const_iterator argIt = tplt->getSignature()->argSpecs->begin();
    for(; argIt != tplt->getSignature()->argSpecs->end(); ++argIt) {
      bool found = false;

      if(args != 0) {
        TemplateArguments::const_iterator it;
        for(it = args->begin(); it != args->end(); ++it) {
          if(XPath2Utils::equals((*it)->name, (*argIt)->getName()) &&
             XPath2Utils::equals((*it)->uri, (*argIt)->getURI())) {
            if(scope == &localscope)
              localscope.setVar((*it)->uri, (*it)->name, (*it)->value->createResult(context));
            found = true;
            break;
          }
        }
      }

      if(!found) {
        // [ERR XTSE0690] It is a static error  if a template that is invoked using xsl:call-template
        // declares a template parameter  specifying required="yes" and not specifying tunnel="yes", if
        // no value for this parameter is supplied by the calling instruction.
        XMLBuffer buf;
        buf.set(X("No value is specified for the template parameter {"));
        buf.append((*argIt)->getURI());
        buf.append(X("}"));
        buf.append((*argIt)->getName());
        buf.append(X(" [err:XTSE0690]"));

        XQThrow3(StaticErrorException, X("XQApplyTemplates::staticResolution"), buf.getRawBuffer(), location);
      }

    }
  }

  AutoVariableStoreReset vsReset(context, scope);
  return tplt->getTemplateInstance()->createResult(context);
}

void XQApplyTemplates::evaluateArguments(VarStoreImpl &scope, DynamicContext *context) const
{
  if(args_ == 0) return;

  TemplateArguments::const_iterator it;
  for(it = args_->begin(); it != args_->end(); ++it) {
    // TBD variable use count - jpcs
    scope.setVar((*it)->uri, (*it)->name, (*it)->value->createResult(context));
  }
}

class ApplyTemplatesResult : public ResultImpl
{
public:
  ApplyTemplatesResult(const XQApplyTemplates *ast, const Result &input, const UserFunctions &templates, DynamicContext *context)
    : ResultImpl(ast),
      ast_(ast),
      templates_(templates),
      input_(input),
      node_(0),
      scope_(context->getMemoryManager(), context->getVariableStore()),
      result_(0)
  {
    ast_->evaluateArguments(scope_, context);
  }

  // Constructor for the default template for elements / document nodes
  ApplyTemplatesResult(ApplyTemplatesResult *parent, const Node::Ptr &node, DynamicContext *context)
    : ResultImpl(parent),
      ast_(parent->ast_),
      templates_(context->getTemplateRules()),
      input_(node->getAxisResult(XQStep::CHILD, 0, context, parent)),
      node_(0),
      scope_(context->getMemoryManager(), &parent->scope_),
      result_(0)
  {
  }

  Item::Ptr next(DynamicContext *context)
  {
    AutoContextInfoReset ciReset(context, node_);
    AutoVariableStoreReset vsReset(context, &scope_);

    Item::Ptr item;
    while((item = result_->next(context)).isNull()) {
      ciReset.resetContextInfo();
      vsReset.reset();

      node_ = input_->next(context);
      if(node_.isNull()) return 0;

      context->setContextItem(node_);
      // TBD What is the correct position / size here - jpcs

      // Find the first template that matches
      XQUserFunction *tplt = 0;
      UserFunctions::const_iterator it = templates_.begin();
      for(; tplt == 0 && it != templates_.end(); ++it) {
        if(*it == 0 || (*it)->getPattern() == 0) continue;

        if(ast_->getMode() && ast_->getMode()->getState() == XQUserFunction::Mode::CURRENT) {
          // TBD filter on the current mode - jpcs
        }

        VectorOfASTNodes::const_iterator patIt = (*it)->getPattern()->begin();
        for(; patIt != (*it)->getPattern()->end(); ++patIt) {
          if(!(*patIt)->createResult(context)->next(context).isNull()) {
            tplt = *it;
            break;
          }
        }
      }

      context->setVariableStore(&scope_);

      if(tplt != 0) {
        result_ = XQApplyTemplates::executeTemplate(tplt, ast_->getArguments(), &scope_, context, this);
      } else {
        result_ = executeDefaultTemplate(context);
      }
    }

    return item;
  }

  Result executeDefaultTemplate(DynamicContext *context)
  {
    const XMLCh *nodeKind = node_->dmNodeKind();
    if(nodeKind == Node::document_string || nodeKind == Node::element_string) {
      return new ApplyTemplatesResult(this, node_, context);
    }
    else if(nodeKind == Node::text_string || nodeKind == Node::attribute_string) {
      AutoDelete<SequenceBuilder> builder(context->createSequenceBuilder());
      builder->textEvent(node_->dmStringValue(context));
      builder->endEvent();
      return builder->getSequence();
    }
    else {

      return 0;
    }
  }

  string asString(DynamicContext *context, int indent) const { return ""; }

private:
  const XQApplyTemplates *ast_;
  UserFunctions templates_;

  Result input_;

  Node::Ptr node_;
  VarStoreImpl scope_;
  Result result_;
};

Result XQApplyTemplates::createResult(DynamicContext *context, int flags) const
{
  return ClosureResult::create(getStaticAnalysis(), context,
    new ApplyTemplatesResult(this, expr_->createResult(context), templates_, context));
}
