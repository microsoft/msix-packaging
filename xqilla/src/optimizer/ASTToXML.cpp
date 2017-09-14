/*
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

// #define SHOW_QUERY_PATH_TREES
// #define SHOW_HIDDEN_AST

#include "../config/xqilla_config.h"
#include <iostream>
#include <sstream>

#include <xqilla/optimizer/ASTToXML.hpp>
#include <xqilla/utils/XStr.hpp>
#include <xqilla/events/EventSerializer.hpp>
#include <xqilla/context/ContextHelpers.hpp>
#include <xqilla/schema/SequenceType.hpp>
#include <xqilla/axis/NodeTest.hpp>
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/context/Collation.hpp>

#include <xqilla/functions/FunctionDoc.hpp>
#include <xqilla/functions/FunctionDocument.hpp>
#include <xqilla/functions/FunctionCollection.hpp>
#include <xqilla/functions/FunctionParseXML.hpp>
#include <xqilla/functions/FunctionParseJSON.hpp>
#include <xqilla/functions/FunctionSignature.hpp>

#include <xqilla/operators/OrderComparison.hpp>
#include <xqilla/operators/Equals.hpp>
#include <xqilla/operators/NotEquals.hpp>
#include <xqilla/operators/LessThan.hpp>
#include <xqilla/operators/LessThanEqual.hpp>
#include <xqilla/operators/GreaterThan.hpp>
#include <xqilla/operators/GreaterThanEqual.hpp>

#include <xqilla/optimizer/QueryPathNode.hpp>

#include <xercesc/framework/MemBufFormatTarget.hpp>

XERCES_CPP_NAMESPACE_USE;

using namespace std;

static const XMLCh s_name[] = { 'n', 'a', 'm', 'e', 0 };
static const XMLCh s_type[] = { 't', 'y', 'p', 'e', 0 };
static const XMLCh s_value[] = { 'v', 'a', 'l', 'u', 'e', 0 };
static const XMLCh s_true[] = { 't', 'r', 'u', 'e', 0 };
static const XMLCh s_false[] = { 'f', 'a', 'l', 's', 'e', 0 };
static const XMLCh s_uri[] = { 'u', 'r', 'i', 0 };
static const XMLCh s_prefix[] = { 'p', 'r', 'e', 'f', 'i', 'x', 0 };
static const XMLCh s_localname[] = { 'l', 'o', 'c', 'a', 'l', 'n', 'a', 'm', 'e', 0 };
static const XMLCh s_units[] = { 'u', 'n', 'i', 't', 's', 0 };
static const XMLCh s_distance[] = { 'd', 'i', 's', 't', 'a', 'n', 'c', 'e', 0 };
static const XMLCh s_unknown[] = { 'u', 'n', 'k', 'n', 'o', 'w', 'n', 0 };
static const XMLCh s_Binding[] = { 'B', 'i', 'n', 'd', 'i', 'n', 'g', 0 };

ASTToXML::ASTToXML()
  : events_(0),
    context_(0),
    indent_(0),
    hasChildren_(false)
{
}

string ASTToXML::print(const XQQuery *query, const DynamicContext *context)
{
  MemBufFormatTarget target;
  EventSerializer writer((char*)"UTF-8", (char*)"1.0", &target);
  run(query, &writer, context);
  return string((char*)target.getRawBuffer(), target.getLen());
}

string ASTToXML::print(const XQUserFunction *item, const DynamicContext *context)
{
  MemBufFormatTarget target;
  EventSerializer writer((char*)"UTF-8", (char*)"1.0", &target);
  run(item, &writer, context);
  return string((char*)target.getRawBuffer(), target.getLen());
}


string ASTToXML::print(const ASTNode *item, const DynamicContext *context)
{
  MemBufFormatTarget target;
  EventSerializer writer((char*)"UTF-8", (char*)"1.0", &target);
  run(item, &writer, context);
  return string((char*)target.getRawBuffer(), target.getLen());
}

string ASTToXML::print(const TupleNode *item, const DynamicContext *context)
{
  MemBufFormatTarget target;
  EventSerializer writer((char*)"UTF-8", (char*)"1.0", &target);
  run(item, &writer, context);
  return string((char*)target.getRawBuffer(), target.getLen());
}

void ASTToXML::run(const XQQuery *query, EventHandler *events, const DynamicContext *context)
{
  events_ = events;
  context_ = context;
  indent_ = 0;
  hasChildren_ = true;
  optimize((XQQuery*)query);
}

void ASTToXML::run(const ASTNode *item, EventHandler *events, const DynamicContext *context)
{
  events_ = events;
  context_ = context;
  indent_ = 0;
  hasChildren_ = true;
  optimize((ASTNode*)item);
}

void ASTToXML::run(const TupleNode *item, EventHandler *events, const DynamicContext *context)
{
  events_ = events;
  context_ = context;
  indent_ = 0;
  hasChildren_ = true;
  optimizeTupleNode((TupleNode*)item);
}

void ASTToXML::run(const XQUserFunction *item, EventHandler *events, const DynamicContext *context)
{
  events_ = events;
  context_ = context;
  indent_ = 0;
  hasChildren_ = true;
  optimizeFunctionDef((XQUserFunction*)item);
}

void ASTToXML::optimize(XQQuery *query)
{
  static const XMLCh s_Module[] = { 'M', 'o', 'd', 'u', 'l', 'e', 0 };
  static const XMLCh s_XQuery[] = { 'X', 'Q', 'u', 'e', 'r', 'y', 0 };
  static const XMLCh s_targetNamespace[] = { 't', 'a', 'r', 'g', 'e', 't', 'N', 'a', 'm', 'e', 's', 'p', 'a', 'c', 'e', 0 };
//   static const XMLCh s_ImportedModule[] = { 'I', 'm', 'p', 'o', 'r', 't', 'e', 'd', 'M', 'o', 'd', 'u', 'l', 'e', 0 };

  if(!hasChildren_) newline();

  indent();
  events_->startElementEvent(0, 0, query->getIsLibraryModule() ? s_Module : s_XQuery);
  if(query->getModuleTargetNamespace()) {
    events_->attributeEvent(0, 0, s_targetNamespace, query->getModuleTargetNamespace(), 0, 0);
  }

  {
    AutoReset<unsigned int> resetIndent(indent_);
    ++indent_;
    hasChildren_ = false;

//     const ImportedModules &modules = query->getImportedModules();
//     for(it1 = modules.begin(); it1 != modules.end(); ++it1) {
//       indent();
//       events_->startElementEvent(0, 0, s_ImportedModule : s_XQuery);
//       events_->attributeEvent(0, 0, s_targetNamespace, (*it1)->getModuleTargetNamespace(), 0, 0);
//       events_->endElementEvent(0, 0, s_ImportedModule, 0, 0);
//       newline();
//     }

    ASTVisitor::optimize(query);
  }

  if(hasChildren_) indent();
  events_->endElementEvent(0, 0, query->getIsLibraryModule() ? s_Module : s_XQuery, 0, 0);
  newline();

  hasChildren_ = true;
}

XQGlobalVariable *ASTToXML::optimizeGlobalVar(XQGlobalVariable *item)
{
  static const XMLCh s_GlobalParam[] = { 'G', 'l', 'o', 'b', 'a', 'l', 'P', 'a', 'r', 'a', 'm', 0 };
  static const XMLCh s_GlobalVar[] = { 'G', 'l', 'o', 'b', 'a', 'l', 'V', 'a', 'r', 0 };
  static const XMLCh s_external[] = { 'e', 'x', 't', 'e', 'r', 'n', 'a', 'l', 0 };

  if(!hasChildren_) newline();

  indent();
  events_->startElementEvent(0, 0, item->isParam() ? s_GlobalParam : s_GlobalVar);
  XMLBuffer buf;
  qname(item->getVariableName(), 0, item->getVariableURI(), item->getVariableLocalName(), buf);
  events_->attributeEvent(0, 0, s_name, buf.getRawBuffer(), 0, 0);
  if(item->isExternal())
    events_->attributeEvent(0, 0, s_external, s_true, 0, 0);

  {
    AutoReset<unsigned int> resetIndent(indent_);
    ++indent_;
    hasChildren_ = false;

    if(item->getSequenceType())
      optimizeSequenceType(item->getSequenceType());

    ASTVisitor::optimizeGlobalVar(item);
  }

  if(hasChildren_) indent();
  events_->endElementEvent(0, 0, item->isParam() ? s_GlobalParam : s_GlobalVar, 0, 0);
  newline();

  hasChildren_ = true;
  return item;
}

XQUserFunction *ASTToXML::optimizeFunctionDef(XQUserFunction *item)
{
  static const XMLCh s_TemplateDefinition[] = { 'T', 'e', 'm', 'p', 'l', 'a', 't', 'e', 'D', 'e', 'f', 'i', 'n', 'i', 't', 'i', 'o', 'n', 0 };
  static const XMLCh s_FunctionDefinition[] = { 'F', 'u', 'n', 'c', 't', 'i', 'o', 'n', 'D', 'e', 'f', 'i', 'n', 'i', 't', 'i', 'o', 'n', 0 };
  static const XMLCh s_Pattern[] = { 'P', 'a', 't', 't', 'e', 'r', 'n', 0 };

  if(!hasChildren_) newline();

  indent();
  events_->startElementEvent(0, 0, item->isTemplate() ? s_TemplateDefinition : s_FunctionDefinition);
  if(item->getQName() != 0 || item->getName() != 0) {
    XMLBuffer buf;
    qname(item->getQName(), 0, item->getURI(), item->getName(), buf);
    events_->attributeEvent(0, 0, s_name, buf.getRawBuffer(), 0, 0);
  }
  optimizeFunctionSignature(item->getSignature());

  {
    AutoReset<unsigned int> resetIndent(indent_);
    ++indent_;
    hasChildren_ = false;

    if(item->getPattern()) {
      if(!hasChildren_) newline();

      indent();
      events_->startElementEvent(0, 0, s_Pattern);
      {
        AutoReset<unsigned int> resetIndent(indent_);
        ++indent_;
        hasChildren_ = false;

        VectorOfASTNodes::iterator patIt = item->getPattern()->begin();
        for(; patIt != item->getPattern()->end(); ++patIt) {
          optimize(*patIt);
        }
      }
      if(hasChildren_) indent();
      events_->endElementEvent(0, 0, s_Pattern, 0, 0);
      newline();

      hasChildren_ = true;
    }

    if(item->getFunctionBody())
      optimize((ASTNode*)item->getFunctionBody());
  }

  if(hasChildren_) indent();
  events_->endElementEvent(0, 0, item->isTemplate() ? s_TemplateDefinition : s_FunctionDefinition, 0, 0);
  newline();

  hasChildren_ = true;
  return item;
}

void ASTToXML::getElementName(ASTNode *item, XMLBuffer &buf)
{
  if(!item) {
    buf.append(X("Null"));
    return;
  }

  switch(item->getType()) {
  case ASTNode::LITERAL:
    buf.append(X("Literal"));
    break;
  case ASTNode::QNAME_LITERAL:
    buf.append(X("QNameLiteral"));
    break;
  case ASTNode::NUMERIC_LITERAL:
    buf.append(X("NumericLiteral"));
    break;
  case ASTNode::SEQUENCE:
    buf.append(X("Sequence"));
    break;
  case ASTNode::FUNCTION:
    buf.append(X("Function"));
    break;
  case ASTNode::NAVIGATION:
    buf.append(X("Nav"));
    break;
  case ASTNode::VARIABLE:
    buf.append(X("Variable"));
    break;
  case ASTNode::STEP:
    buf.append(X("Step"));
    break;
  case ASTNode::IF:
    buf.append(X("If"));
    break;
  case ASTNode::CASTABLE_AS:
    buf.append(X("CastableAs"));
    break;
  case ASTNode::CAST_AS:
    buf.append(X("CastAs"));
    break;
  case ASTNode::TREAT_AS:
    buf.append(X("TreatAs"));
    break;
  case ASTNode::FUNCTION_COERCION:
    buf.append(X("FunctionCoercion"));
    break;
  case ASTNode::OPERATOR:
    buf.append(((XQOperator*)item)->getOperatorName());
    break;
  case ASTNode::CONTEXT_ITEM:
    buf.append(X("ContextItem"));
    break;
  case ASTNode::RETURN:
    buf.append(X("Return"));
    break;
  case ASTNode::QUANTIFIED:
    buf.append(((XQQuantified*)item)->getQuantifierType() == XQQuantified::SOME ? X("Some") : X("Every"));
    break;
  case ASTNode::TYPESWITCH:
    buf.append(X("Typeswitch"));
    break;
  case ASTNode::VALIDATE:
    buf.append(X("Validate"));
    break;
  case ASTNode::FUNCTION_CALL:
    buf.append(X("FunctionCall"));
    break;
  case ASTNode::DOM_CONSTRUCTOR:
    buf.append(X("NodeConstructor"));
    break;
  case ASTNode::SIMPLE_CONTENT:
    buf.append(X("SimpleContent"));
    break;
  case ASTNode::ORDERING_CHANGE:
    if(((XQOrderingChange*)item)->getOrderingValue() == StaticContext::ORDERING_ORDERED)
      buf.append(X("Ordered"));
    else buf.append(X("Unordered"));
    break;
  case ASTNode::ATOMIZE:
    buf.append(X("Atomize"));
    break;
  case ASTNode::EBV:
    buf.append(X("EffectiveBooleanValue"));
    break;
  case ASTNode::MAP:
    buf.append(X("Map"));
    break;
  case ASTNode::PROMOTE_UNTYPED:
    buf.append(X("PromoteUntyped"));
    break;
  case ASTNode::PROMOTE_NUMERIC:
    buf.append(X("PromoteNumeric"));
    break;
  case ASTNode::PROMOTE_ANY_URI:
    buf.append(X("PromoteAnyURI"));
    break;
  case ASTNode::DOCUMENT_ORDER:
    buf.append((((XQDocumentOrder*)item)->getUnordered()) ? X("UniqueNodes") : X("DocumentOrder"));
    break;
  case ASTNode::PREDICATE:
    buf.append(X("Predicate"));
    break;
  case ASTNode::USER_FUNCTION:
    buf.append(((XQUserFunctionInstance*)item)->getFunctionDefinition()->isTemplate() ? X("Template") : X("UserFunction"));
    break;
  case ASTNode::NAME_EXPRESSION:
    buf.append(X("NameExpression"));
    break;
  case ASTNode::CONTENT_SEQUENCE:
    buf.append(X("ContentSequence"));
    break;
  case ASTNode::DIRECT_NAME:
    buf.append(X("DirectName"));
    break;
  case ASTNode::UDELETE:
    buf.append(X("UDelete"));
    break;
  case ASTNode::URENAME:
    buf.append(X("URename"));
    break;
  case ASTNode::UREPLACE:
    buf.append(X("UReplace"));
    break;
  case ASTNode::UREPLACE_VALUE_OF:
    buf.append(X("UReplaceValueOf"));
    break;
  case ASTNode::UINSERT_AS_FIRST:
    buf.append(X("UInsertAsFirst"));
    break;
  case ASTNode::UINSERT_AS_LAST:
    buf.append(X("UInsertAsLast"));
    break;
  case ASTNode::UINSERT_INTO:
    buf.append(X("UInsertInto"));
    break;
  case ASTNode::UINSERT_AFTER:
    buf.append(X("UInsertAfter"));
    break;
  case ASTNode::UINSERT_BEFORE:
    buf.append(X("UInsertBefore"));
    break;
  case ASTNode::UTRANSFORM:
    buf.append(X("UTransform"));
    break;
  case ASTNode::UAPPLY_UPDATES:
    buf.append(X("UApplyUpdates"));
    break;
  case ASTNode::FTCONTAINS:
    buf.append(X("FTContains"));
    break;
  case ASTNode::NAMESPACE_BINDING:
    buf.append(X("NamespaceBinding"));
    break;
  case ASTNode::FUNCTION_CONVERSION:
    buf.append(X("FunctionConversion"));
    break;
  case ASTNode::ANALYZE_STRING:
    buf.append(X("AnalyzeString"));
    break;
  case ASTNode::COPY_OF:
    buf.append(X("CopyOf"));
    break;
  case ASTNode::COPY:
    buf.append(X("Copy"));
    break;
  case ASTNode::DEBUG_HOOK:
#ifdef SHOW_HIDDEN_AST
    buf.append(X("ASTDebugHook"));
#endif
    break;
  case ASTNode::CALL_TEMPLATE:
    buf.append(X("CallTemplate"));
    break;
  case ASTNode::APPLY_TEMPLATES:
    buf.append(X("ApplyTemplates"));
    break;
  case ASTNode::INLINE_FUNCTION:
    buf.append(X("InlineFunction"));
    break;
  case ASTNode::FUNCTION_REF:
    buf.append(X("FunctionRef"));
    break;
  case ASTNode::FUNCTION_DEREF:
    buf.append(X("FunctionDeref"));
    break;
  case ASTNode::PARTIAL_APPLY:
    buf.append(X("PartialApply"));
    break;
  default:
    buf.append(X("Unknown"));
    break;
  }
}

ASTNode *ASTToXML::optimize(ASTNode *item)
{
  XMLBuffer elementName;
  getElementName(item, elementName);
  if(elementName.getLen() == 0)
    return ASTVisitor::optimize(item);

  if(!hasChildren_) {
    // This is the first child
    newline();
  }

  indent();
  events_->startElementEvent(0, 0, elementName.getRawBuffer());

  if(item) {
    AutoReset<unsigned int> resetIndent(indent_);
    ++indent_;
    hasChildren_ = false;
    ASTVisitor::optimize(item);
  }

  if(hasChildren_) {
    // It's not an empty element
    indent();
  }

  events_->endElementEvent(0, 0, elementName.getRawBuffer(), 0, 0);
  newline();

  hasChildren_ = true;
  return item;
}

ASTNode *ASTToXML::optimizeFunction(XQFunction *item)
{
  const XMLCh *funUri = item->getFunctionURI();
  const XMLCh *funName = item->getFunctionName();

  XMLBuffer buf;
  qname(0, 0, funUri, funName, buf);
  events_->attributeEvent(0, 0, s_name, buf.getRawBuffer(), 0, 0);

//   QueryPathNode *queryPathTree = 0;
// #ifdef SHOW_QUERY_PATH_TREES
//   if(funUri == XQFunction::XMLChFunctionURI) {
//     if(funName == FunctionDoc::name) {
//       queryPathTree = ((FunctionDoc*)item)->getQueryPathTree();
//     }
//     else if(funName == FunctionDocument::name) {
//       queryPathTree = ((FunctionDocument*)item)->getQueryPathTree();
//     }
//     else if(funName == FunctionCollection::name) {
//       queryPathTree = ((FunctionCollection*)item)->getQueryPathTree();
//     }
//   }
//   else if(funUri == XQillaFunction::XMLChFunctionURI) {
//     if(funName == FunctionParseXML::name) {
//       queryPathTree = ((FunctionParseXML*)item)->getQueryPathTree();
//     }
//     else if(funName == FunctionParseJSON::name) {
//       queryPathTree = ((FunctionParseJSON*)item)->getQueryPathTree();
//     }
//     else if(funName == FunctionExplain::name) {
//       queryPathTree = ((FunctionExplain*)item)->getQueryPathTree();
//     }
//   }
// #endif

  ASTVisitor::optimizeFunction(item);
//   if(queryPathTree)
//     queryPathTree->toString(indent + INDENT);

  return item;
}

ASTNode *ASTToXML::optimizeOperator(XQOperator *item)
{
  static const XMLCh s_comparison[] = { 'c', 'o', 'm', 'p', 'a', 'r', 'i', 's', 'o', 'n', 0 };
  static const XMLCh s_before[] = { 'b', 'e', 'f', 'o', 'r', 'e', 0 };

  if(item->getOperatorName() == GeneralComp::name) {
    events_->attributeEvent(0, 0, s_comparison, getComparisonOperationName(((GeneralComp *)item)->getOperation()), 0, 0);
  }
  else if(item->getOperatorName() == OrderComparison::name) {
    events_->attributeEvent(0, 0, s_before, ((OrderComparison*)item)->getTestBefore() ? s_true : s_false, 0, 0);
  }

  return ASTVisitor::optimizeOperator(item);
}

ASTNode *ASTToXML::optimizeLiteral(XQLiteral *item)
{
  events_->attributeEvent(0, 0, s_value, item->getValue(), 0, 0);
  XMLBuffer buf;
  qname(0, 0, item->getTypeURI(), item->getTypeName(), buf);
  events_->attributeEvent(0, 0, s_type, buf.getRawBuffer(), 0, 0);

  return ASTVisitor::optimizeLiteral(item);
}

ASTNode *ASTToXML::optimizeQNameLiteral(XQQNameLiteral *item)
{
  events_->attributeEvent(0, 0, s_uri, item->getURI(), 0, 0);
  events_->attributeEvent(0, 0, s_prefix, item->getPrefix(), 0, 0);
  events_->attributeEvent(0, 0, s_localname, item->getLocalname(), 0, 0);

  XMLBuffer buf;
  qname(0, 0, item->getTypeURI(), item->getTypeName(), buf);
  events_->attributeEvent(0, 0, s_type, buf.getRawBuffer(), 0, 0);

  return ASTVisitor::optimizeQNameLiteral(item);
}

ASTNode *ASTToXML::optimizeNumericLiteral(XQNumericLiteral *item)
{
  char obuf[1024];
  m_apm_to_string_mt(obuf, item->getRawValue().m_apm_datalength, const_cast<M_APM>(&item->getRawValue()));

  events_->attributeEvent(0, 0, s_value, X(obuf), 0, 0);
  XMLBuffer buf;
  qname(0, 0, item->getTypeURI(), item->getTypeName(), buf);
  events_->attributeEvent(0, 0, s_type, buf.getRawBuffer(), 0, 0);

  return ASTVisitor::optimizeNumericLiteral(item);
}

void ASTToXML::optimizeNodeTest(const NodeTest *step)
{
  static const XMLCh s_asterisk[] = { '*', 0 };
  static const XMLCh s_nodeType[] = { 'n', 'o', 'd', 'e', 'T', 'y', 'p', 'e', 0 };

  SequenceType::ItemType *type = step->getItemType();
  if(type) {
    XMLBuffer buf;
    type->toBuffer(buf);
    events_->attributeEvent(0, 0, s_type, buf.getRawBuffer(), 0, 0);
  }
  else {
    if(step->getNamespaceWildcard()) {
      events_->attributeEvent(0, 0, s_uri, s_asterisk, 0, 0);
    }
    else {
      if(step->getNodePrefix() != 0) {
        events_->attributeEvent(0, 0, s_prefix, step->getNodePrefix(), 0, 0);
      }
      if(step->getNodeUri() != 0) {
        events_->attributeEvent(0, 0, s_uri, step->getNodeUri(), 0, 0);
      }
    }
    if(step->getNameWildcard()) {
      events_->attributeEvent(0, 0, s_localname, s_asterisk, 0, 0);
    }
    else if(step->getNodeName() != 0) {
      events_->attributeEvent(0, 0, s_localname, step->getNodeName(), 0, 0);
    }
    
    if(step->getTypeWildcard()) {
      events_->attributeEvent(0, 0, s_nodeType, s_asterisk, 0, 0);
    }
    else if(step->isNodeTypeSet()) {
      events_->attributeEvent(0, 0, s_nodeType, step->getNodeType(), 0, 0);
    }
  }
}

ASTNode *ASTToXML::optimizeStep(XQStep *item)
{
  static const XMLCh s_axis[] = { 'a', 'x', 'i', 's', 0 };

  events_->attributeEvent(0, 0, s_axis, getAxisName(item->getAxis()), 0, 0);
  optimizeNodeTest(item->getNodeTest());
  return ASTVisitor::optimizeStep(item);
}

ASTNode *ASTToXML::optimizeVariable(XQVariable *item)
{
  XMLBuffer buf;
  qname(0, item->getPrefix(), item->getURI(), item->getName(), buf);
  events_->attributeEvent(0, 0, s_name, buf.getRawBuffer(), 0, 0);
  return ASTVisitor::optimizeVariable(item);
}

ASTNode *ASTToXML::optimizeCastableAs(XQCastableAs *item)
{
  ASTVisitor::optimizeCastableAs(item);
  optimizeSequenceType(item->getSequenceType());
  return item;
}

ASTNode *ASTToXML::optimizeCastAs(XQCastAs *item)
{
  ASTVisitor::optimizeCastAs(item);
  optimizeSequenceType(item->getSequenceType());
  return item;
}

ASTNode *ASTToXML::optimizeTreatAs(XQTreatAs *item)
{
  ASTVisitor::optimizeTreatAs(item);
  optimizeSequenceType(item->getSequenceType());
  return item;
}

ASTNode *ASTToXML::optimizeFunctionCoercion(XQFunctionCoercion *item)
{
  optimize(item->getExpression());
#ifdef SHOW_HIDDEN_AST
  if(item->getFuncConvert())
    optimize(item->getFuncConvert());
#endif
  optimizeSequenceType(item->getSequenceType());
  return item;
}

ASTNode *ASTToXML::optimizeFunctionConversion(XQFunctionConversion *item)
{
  ASTVisitor::optimizeFunctionConversion(item);
  optimizeSequenceType(item->getSequenceType());
  return item;
}

ASTNode *ASTToXML::optimizeUserFunction(XQUserFunctionInstance *item)
{
  XMLBuffer buf;
  qname(0, 0, item->getFunctionURI(), item->getFunctionName(), buf);
  events_->attributeEvent(0, 0, s_name, buf.getRawBuffer(), 0, 0);

  if(item->getFunctionDefinition()->getSignature()->argSpecs) {
    VectorOfASTNodes &args = const_cast<VectorOfASTNodes &>(item->getArguments());
    ArgumentSpecs::const_iterator binding = item->getFunctionDefinition()->getSignature()->argSpecs->begin();
    for(VectorOfASTNodes::const_iterator arg = args.begin(); arg != args.end() && binding != item->getFunctionDefinition()->getSignature()->argSpecs->end(); ++arg, ++binding) {
      if(!hasChildren_) newline();

      indent();
      events_->startElementEvent(0, 0, s_Binding);
      qname((*binding)->getQName(), 0, (*binding)->getURI(), (*binding)->getName(), buf);
      events_->attributeEvent(0, 0, s_name, buf.getRawBuffer(), 0, 0);

      {
        AutoReset<unsigned int> resetIndent(indent_);
        ++indent_;
        hasChildren_ = false;
        optimize(*arg);
      }

      if(hasChildren_) indent();

      events_->endElementEvent(0, 0, s_Binding, 0, 0);
      newline();

      hasChildren_ = true;
    }
  }

  return item;
}

ASTNode *ASTToXML::optimizeTypeswitch(XQTypeswitch *item)
{
  const XQTypeswitch::Cases *cases = item->getCases();
  for(XQTypeswitch::Cases::const_iterator i = cases->begin(); i != cases->end(); ++i) {
    optimizeCase(*i);
  }
  optimizeCase(item->getDefaultCase());
  return item;
}

void ASTToXML::optimizeCase(const XQTypeswitch::Case *cse)
{
  static const XMLCh s_Case[] = { 'C', 'a', 's', 'e', 0 };
  static const XMLCh s_Default[] = { 'D', 'e', 'f', 'a', 'u', 'l', 't', 0 };

  if(!hasChildren_) newline();
  indent();
  events_->startElementEvent(0, 0, cse->getSequenceType() ? s_Case : s_Default);

  XMLBuffer buf;
  qname(cse->getQName(), 0, cse->getURI(), cse->getName(), buf);
  events_->attributeEvent(0, 0, s_name, buf.getRawBuffer(), 0, 0);

  {
    AutoReset<unsigned int> resetIndent(indent_);
    ++indent_;
    hasChildren_ = false;

    if(cse->getSequenceType())
      optimizeSequenceType(cse->getSequenceType());
    optimize(cse->getExpression());
  }

  if(hasChildren_) indent();
  events_->endElementEvent(0, 0, cse->getSequenceType() ? s_Case : s_Default, 0, 0);
  newline();

  hasChildren_ = true;
}

ASTNode *ASTToXML::optimizeValidate(XQValidate *item)
{
  static const XMLCh s_mode[] = { 'm', 'o', 'd', 'e', 0 };
  static const XMLCh s_strict[] = { 's', 't', 'r', 'i', 'c', 't', 0 };
  static const XMLCh s_lax[] = { 'l', 'a', 'x', 0 };

  events_->attributeEvent(0, 0, s_mode, item->getMode() == DocumentCache::VALIDATION_STRICT ? s_strict : s_lax, 0, 0);
  return ASTVisitor::optimizeValidate(item);
}

ASTNode *ASTToXML::optimizeFunctionCall(XQFunctionCall *item)
{
  XMLBuffer buf;
  qname(0, item->getPrefix(), item->getURI(), item->getName(), buf);
  events_->attributeEvent(0, 0, s_name, buf.getRawBuffer(), 0, 0);
  return ASTVisitor::optimizeFunctionCall(item);
}

ASTNode *ASTToXML::optimizeDOMConstructor(XQDOMConstructor *item)
{
  events_->attributeEvent(0, 0, s_type, item->getNodeType(), 0, 0);
  ASTVisitor::optimizeDOMConstructor(item);
// #ifdef SHOW_QUERY_PATH_TREES
//   if(item->getQueryPathTree())
//     s << item->getQueryPathTree()->toString(indent + INDENT);
// #endif
  return item;
}

void ASTToXML::getElementName(FTSelection *item, XMLBuffer &buf)
{
  switch(item->getType()) {
  case FTSelection::OR:
    buf.append(X("FTOr"));
    break;
  case FTSelection::AND:
    buf.append(X("FTAnd"));
    break;
  case FTSelection::MILD_NOT:
    buf.append(X("FTMildnot"));
    break;
  case FTSelection::UNARY_NOT:
    buf.append(X("FTUnaryNot"));
    break;
  case FTSelection::WORDS:
    buf.append(X("FTWords"));
    break;
  case FTSelection::WORD:
    buf.append(X("FTWord"));
    break;
  case FTSelection::ORDER:
    buf.append(X("FTOrder"));
    break;
  case FTSelection::DISTANCE:
    buf.append(X("FTDistance"));
    break;
  case FTSelection::DISTANCE_LITERAL:
    buf.append(X("FTDistanceLiteral"));
    break;
  case FTSelection::SCOPE:
    buf.append(X("FTScope"));
    break;
  case FTSelection::CONTENT:
    buf.append(X("FTContent"));
    break;
  case FTSelection::WINDOW:
    buf.append(X("FTWindow"));
    break;
  case FTSelection::WINDOW_LITERAL:
    buf.append(X("FTWindowLiteral"));
    break;
  default:
    buf.append(X("FTUnknown"));
    break;
  }
}

FTSelection *ASTToXML::optimizeFTSelection(FTSelection *item)
{
  XMLBuffer elementName;
  getElementName(item, elementName);
  if(elementName.getLen() == 0)
    return ASTVisitor::optimizeFTSelection(item);

  if(!hasChildren_) {
    // This is the first child
    newline();
  }

  indent();
  events_->startElementEvent(0, 0, elementName.getRawBuffer());

  {
    AutoReset<unsigned int> resetIndent(indent_);
    ++indent_;
    hasChildren_ = false;
    ASTVisitor::optimizeFTSelection(item);
  }

  if(hasChildren_) {
    // It's not an empty element
    indent();
  }

  events_->endElementEvent(0, 0, elementName.getRawBuffer(), 0, 0);
  newline();

  hasChildren_ = true;
  return item;
}

FTSelection *ASTToXML::optimizeFTWords(FTWords *selection)
{
  static const XMLCh s_option[] = { 'o', 'p', 't', 'i', 'o', 'n', 0 };
  static const XMLCh s_any[] = { 'a', 'n', 'y', 0 };
  static const XMLCh s_any_word[] = { 'a', 'n', 'y', ' ', 'w', 'o', 'r', 'd', 0 };
  static const XMLCh s_all[] = { 'a', 'l', 'l', 0 };
  static const XMLCh s_all_words[] = { 'a', 'l', 'l', ' ', 'w', 'o', 'r', 'd', 's', 0 };
  static const XMLCh s_phrase[] = { 'p', 'h', 'r', 'a', 's', 'e', 0 };

  switch(selection->getOption()) {
  case FTWords::ANY: events_->attributeEvent(0, 0, s_option, s_any, 0, 0); break;
  case FTWords::ANY_WORD: events_->attributeEvent(0, 0, s_option, s_any_word, 0, 0); break;
  case FTWords::ALL: events_->attributeEvent(0, 0, s_option, s_all, 0, 0); break;
  case FTWords::ALL_WORDS: events_->attributeEvent(0, 0, s_option, s_all_words, 0, 0); break;
  case FTWords::PHRASE: events_->attributeEvent(0, 0, s_option, s_phrase, 0, 0); break;
  }
  return ASTVisitor::optimizeFTWords(selection);
}

FTSelection *ASTToXML::optimizeFTWord(FTWord *selection)
{
  events_->attributeEvent(0, 0, s_value, selection->getQueryString(), 0, 0);
  return ASTVisitor::optimizeFTWord(selection);
}

FTSelection *ASTToXML::optimizeFTDistance(FTDistance *selection)
{
  events_->attributeEvent(0, 0, s_type, getFTRangeTypeName(selection->getRange().type), 0, 0);
  events_->attributeEvent(0, 0, s_units, getFTUnitName(selection->getUnit()), 0, 0);
  return ASTVisitor::optimizeFTDistance(selection);
}

FTSelection *ASTToXML::optimizeFTDistanceLiteral(FTDistanceLiteral *selection)
{
  static const XMLCh s_distance2[] = { 'd', 'i', 's', 't', 'a', 'n', 'c', 'e', '2', 0 };

  events_->attributeEvent(0, 0, s_type, getFTRangeTypeName(selection->getType()), 0, 0);
  events_->attributeEvent(0, 0, s_units, getFTUnitName(selection->getUnit()), 0, 0);

  XMLBuffer buf;
  XPath2Utils::numToBuf(selection->getDistance(), buf);
  events_->attributeEvent(0, 0, s_distance, buf.getRawBuffer(), 0, 0);

  if(selection->getType() == FTRange::FROM_TO) {
    buf.reset();
    XPath2Utils::numToBuf(selection->getDistance2(), buf);
    events_->attributeEvent(0, 0, s_distance2, buf.getRawBuffer(), 0, 0);
  }

  return ASTVisitor::optimizeFTDistanceLiteral(selection);
}

FTSelection *ASTToXML::optimizeFTScope(FTScope *selection)
{
  static const XMLCh s_same[] = { 's', 'a', 'm', 'e', 0 };
  static const XMLCh s_different[] = { 'd', 'i', 'f', 'f', 'e', 'r', 'e', 'n', 't', 0 };

  switch(selection->getType()) {
  case FTScope::SAME: events_->attributeEvent(0, 0, s_type, s_same, 0, 0); break;
  case FTScope::DIFFERENT: events_->attributeEvent(0, 0, s_type, s_different, 0, 0); break;
  }
  events_->attributeEvent(0, 0, s_units, getFTUnitName(selection->getUnit()), 0, 0);

  return ASTVisitor::optimizeFTScope(selection);
}

FTSelection *ASTToXML::optimizeFTContent(FTContent *selection)
{
  static const XMLCh s_at_start[] = { 'a', 't', ' ', 's', 't', 'a', 'r', 't', 0 };
  static const XMLCh s_at_end[] = { 'a', 't', ' ', 'e', 'n', 'd', 0 };
  static const XMLCh s_entire_content[] = { 'e', 'n', 't', 'i', 'r', 'e', ' ', 'c', 'o', 'n', 't', 'e', 'n', 't', 0 };

  switch(selection->getType()) {
  case FTContent::AT_START: events_->attributeEvent(0, 0, s_type, s_at_start, 0, 0); break;
  case FTContent::AT_END: events_->attributeEvent(0, 0, s_type, s_at_end, 0, 0); break;
  case FTContent::ENTIRE_CONTENT: events_->attributeEvent(0, 0, s_type, s_entire_content, 0, 0); break;
  }

  return ASTVisitor::optimizeFTContent(selection);
}

FTSelection *ASTToXML::optimizeFTWindow(FTWindow *selection)
{
  events_->attributeEvent(0, 0, s_units, getFTUnitName(selection->getUnit()), 0, 0);
  return ASTVisitor::optimizeFTWindow(selection);
}

FTSelection *ASTToXML::optimizeFTWindowLiteral(FTWindowLiteral *selection)
{
  events_->attributeEvent(0, 0, s_units, getFTUnitName(selection->getUnit()), 0, 0);

  XMLBuffer buf;
  XPath2Utils::numToBuf(selection->getDistance(), buf);
  events_->attributeEvent(0, 0, s_distance, buf.getRawBuffer(), 0, 0);
  return ASTVisitor::optimizeFTWindowLiteral(selection);
}

const XMLCh *ASTToXML::getFTRangeTypeName(FTRange::Type type)
{
  static const XMLCh s_exactly[] = { 'e', 'x', 'a', 'c', 't', 'l', 'y', 0 };
  static const XMLCh s_at_least[] = { 'a', 't', ' ', 'l', 'e', 'a', 's', 't', 0 };
  static const XMLCh s_at_most[] = { 'a', 't', ' ', 'm', 'o', 's', 't', 0 };
  static const XMLCh s_from_to[] = { 'f', 'r', 'o', 'm', ' ', 't', 'o', 0 };

  switch(type) {
  case FTRange::EXACTLY: return s_exactly;
  case FTRange::AT_LEAST: return s_at_least;
  case FTRange::AT_MOST: return s_at_most;
  case FTRange::FROM_TO: return s_from_to;
  }
  return s_unknown;
}

const XMLCh *ASTToXML::getFTUnitName(FTOption::FTUnit unit)
{
  static const XMLCh s_words[] = { 'w', 'o', 'r', 'd', 's', 0 };
  static const XMLCh s_sentences[] = { 's', 'e', 'n', 't', 'e', 'n', 'c', 'e', 's', 0 };
  static const XMLCh s_paragraphs[] = { 'p', 'a', 'r', 'a', 'g', 'r', 'a', 'p', 'h', 's', 0 };

  switch(unit) {
  case FTOption::WORDS: return s_words;
  case FTOption::SENTENCES: return s_sentences;
  case FTOption::PARAGRAPHS: return s_paragraphs;
  }
  return s_unknown;
}

ASTNode *ASTToXML::optimizeMap(XQMap *item)
{
  if(item->getName() != 0) {
    XMLBuffer buf;
    qname(0, 0, item->getURI(), item->getName(), buf);
    events_->attributeEvent(0, 0, s_name, buf.getRawBuffer(), 0, 0);
  }
  return ASTVisitor::optimizeMap(item);
}

ASTNode *ASTToXML::optimizePromoteUntyped(XQPromoteUntyped *item)
{
  XMLBuffer buf;
  qname(0, 0, item->getTypeURI(), item->getTypeName(), buf);
  events_->attributeEvent(0, 0, s_type, buf.getRawBuffer(), 0, 0);
  return ASTVisitor::optimizePromoteUntyped(item);
}

ASTNode *ASTToXML::optimizePromoteNumeric(XQPromoteNumeric *item)
{
  XMLBuffer buf;
  qname(0, 0, item->getTypeURI(), item->getTypeName(), buf);
  events_->attributeEvent(0, 0, s_type, buf.getRawBuffer(), 0, 0);
  return ASTVisitor::optimizePromoteNumeric(item);
}

ASTNode *ASTToXML::optimizePromoteAnyURI(XQPromoteAnyURI *item)
{
  XMLBuffer buf;
  qname(0, 0, item->getTypeURI(), item->getTypeName(), buf);
  events_->attributeEvent(0, 0, s_type, buf.getRawBuffer(), 0, 0);
  return ASTVisitor::optimizePromoteAnyURI(item);
}

ASTNode *ASTToXML::optimizeDirectName(XQDirectName *item)
{
  XMLBuffer buf;
  qname(item->getQName(), 0, 0, 0, buf);
  events_->attributeEvent(0, 0, s_name, buf.getRawBuffer(), 0, 0);
  return ASTVisitor::optimizeDirectName(item);
}

ASTNode *ASTToXML::optimizeUTransform(UTransform *item)
{
  static const XMLCh s_Copy[] = { 'C', 'o', 'p', 'y', 0 };

  XMLBuffer buf;
  const VectorOfCopyBinding *bindings = item->getBindings();
  for(VectorOfCopyBinding::const_iterator i = bindings->begin(); i != bindings->end(); ++i) {
    if(!hasChildren_) newline();

    indent();
    events_->startElementEvent(0, 0, s_Copy);
    qname((*i)->qname_, 0, (*i)->uri_, (*i)->name_, buf);
    events_->attributeEvent(0, 0, s_name, buf.getRawBuffer(), 0, 0);

    {
      AutoReset<unsigned int> resetIndent(indent_);
      ++indent_;
      hasChildren_ = false;
      optimize((*i)->expr_);
    }

    if(hasChildren_) indent();

    events_->endElementEvent(0, 0, s_Copy, 0, 0);
    newline();

    hasChildren_ = true;
  }

  optimize(item->getModifyExpr());
  optimize(item->getReturnExpr());
  return item;
}

ASTNode *ASTToXML::optimizeCopyOf(XQCopyOf *item)
{
  static const XMLCh s_copy_namespaces[] = { 'c', 'o', 'p', 'y', '-', 'n', 'a', 'm', 'e', 's', 'p', 'a', 'c', 'e', 's', 0 };

  events_->attributeEvent(0, 0, s_copy_namespaces, item->getCopyNamespaces() ? s_true : s_false, 0, 0);
  return ASTVisitor::optimizeCopyOf(item);
}

ASTNode *ASTToXML::optimizeASTDebugHook(ASTDebugHook *item)
{
#ifdef SHOW_HIDDEN_AST
  optimizeLocation(item);
  return ASTVisitor::optimizeASTDebugHook(item);
#else
  return optimize(item->getExpression());
#endif
}

void ASTToXML::optimizeTemplateArgument(XQTemplateArgument *item)
{
  if(!hasChildren_) newline();

  indent();
  events_->startElementEvent(0, 0, s_Binding);

  XMLBuffer buf;
  qname(item->qname, 0, item->uri, item->name, buf);
  events_->attributeEvent(0, 0, s_name, buf.getRawBuffer(), 0, 0);

  {
    AutoReset<unsigned int> resetIndent(indent_);
    ++indent_;
    hasChildren_ = false;
    optimize(item->value);
  }

  if(hasChildren_) indent();

  events_->endElementEvent(0, 0, s_Binding, 0, 0);
  newline();

  hasChildren_ = true;
}

ASTNode *ASTToXML::optimizeCallTemplate(XQCallTemplate *item)
{
  if(item->getQName() || item->getName()) {
    XMLBuffer buf;
    qname(item->getQName(), 0, item->getURI(), item->getName(), buf);
    events_->attributeEvent(0, 0, s_name, buf.getRawBuffer(), 0, 0);
  }

  if(item->getASTName()) {
    optimize(item->getASTName());
  }

  if(item->getArguments()) {
    TemplateArguments::iterator argIt;
    for(argIt = item->getArguments()->begin(); argIt != item->getArguments()->end(); ++argIt) {
      optimizeTemplateArgument(*argIt);
    }
  }

  return item;
}

ASTNode *ASTToXML::optimizeApplyTemplates(XQApplyTemplates *item)
{
  optimize(item->getExpression());
  
  if(item->getArguments()) {
    TemplateArguments::iterator argIt;
    for(argIt = item->getArguments()->begin(); argIt != item->getArguments()->end(); ++argIt) {
      optimizeTemplateArgument(*argIt);
    }
  }

  return item;
}

ASTNode *ASTToXML::optimizeInlineFunction(XQInlineFunction *item)
{
  if(item->getName()) {
    XMLBuffer buf;
    qname(0, item->getPrefix(), item->getURI(), item->getName(), buf);
    events_->attributeEvent(0, 0, s_name, buf.getRawBuffer(), 0, 0);
  }
  optimizeFunctionSignature(item->getSignature());

  if(item->getUserFunction())
    optimizeFunctionDef(item->getUserFunction());
#ifndef SHOW_HIDDEN_AST
  else
#endif
    item->setInstance(optimize(item->getInstance()));

  return item;
}

ASTNode *ASTToXML::optimizePartialApply(XQPartialApply *item)
{
  static const XMLCh s_ArgumentPlaceholder[] = { 'A', 'r', 'g', 'u', 'm', 'e', 'n', 't', 'P', 'l', 'a', 'c', 'e', 'h', 'o', 'l', 'd', 'e', 'r', 0 };

  optimize(item->getExpression());

  VectorOfASTNodes *args = item->getArguments();
  if(args) {
    for(VectorOfASTNodes::const_iterator i = args->begin(); i != args->end(); ++i) {
      if(*i == 0) {
        if(!hasChildren_) newline();
        indent();
        events_->startElementEvent(0, 0, s_ArgumentPlaceholder);
        events_->endElementEvent(0, 0, s_ArgumentPlaceholder, 0, 0);
        newline();
        hasChildren_ = true;
      }
      else optimize(*i);
    }
  }
  return item;
}

ASTNode *ASTToXML::optimizeFunctionRef(XQFunctionRef *item)
{
  static const XMLCh s_arity[] = { 'a', 'r', 'i', 't', 'y', 0 };

  XMLBuffer buf;
  qname(item->getQName(), 0, 0, 0, buf);
  events_->attributeEvent(0, 0, s_name, buf.getRawBuffer(), 0, 0);
  buf.reset();
  XPath2Utils::numToBuf(item->getNumArgs(), buf);
  events_->attributeEvent(0, 0, s_arity, buf.getRawBuffer(), 0, 0);
  return ASTVisitor::optimizeFunctionRef(item);
}

void ASTToXML::getElementName(TupleNode *item, XMLBuffer &buf)
{
  switch(item->getType()) {
  case TupleNode::CONTEXT_TUPLE:
    buf.append(X("ContextTuple"));
    break;
  case TupleNode::FOR:
    buf.append(X("ForTuple"));
    break;
  case TupleNode::LET:
    buf.append(X("LetTuple"));
    break;
  case TupleNode::WHERE:
    buf.append(X("WhereTuple"));
    break;
  case TupleNode::COUNT:
    buf.append(X("CountTuple"));
    break;
  case TupleNode::ORDER_BY:
    buf.append(X("OrderByTuple"));
    break;
  case TupleNode::DEBUG_HOOK:
#ifdef SHOW_HIDDEN_AST
    buf.append(X("TupleDebugHook"));
#endif
    break;
  default:
    buf.append(X("Unknown"));
    break;
  }
}

TupleNode *ASTToXML::optimizeTupleNode(TupleNode *item)
{
  XMLBuffer elementName;
  getElementName(item, elementName);
  if(elementName.getLen() == 0)
    return ASTVisitor::optimizeTupleNode(item);

  if(!hasChildren_) {
    // This is the first child
    newline();
  }

  indent();
  events_->startElementEvent(0, 0, elementName.getRawBuffer());

  {
    AutoReset<unsigned int> resetIndent(indent_);
    ++indent_;
    hasChildren_ = false;
    ASTVisitor::optimizeTupleNode(item);
  }

  if(hasChildren_) {
    // It's not an empty element
    indent();
  }

  events_->endElementEvent(0, 0, elementName.getRawBuffer(), 0, 0);
  newline();

  hasChildren_ = true;
  return item;
}

TupleNode *ASTToXML::optimizeForTuple(ForTuple *item)
{
  static const XMLCh s_position[] = { 'p', 'o', 's', 'i', 't', 'i', 'o', 'n', 0 };

  XMLBuffer buf;
  qname(item->getVarQName(), 0, item->getVarURI(), item->getVarName(), buf);
  events_->attributeEvent(0, 0, s_name, buf.getRawBuffer(), 0, 0);
  qname(item->getPosQName(), 0, item->getPosURI(), item->getPosName(), buf);
  events_->attributeEvent(0, 0, s_position, buf.getRawBuffer(), 0, 0);
  return ASTVisitor::optimizeForTuple(item);
}

TupleNode *ASTToXML::optimizeLetTuple(LetTuple *item)
{
  XMLBuffer buf;
  qname(item->getVarQName(), 0, item->getVarURI(), item->getVarName(), buf);
  events_->attributeEvent(0, 0, s_name, buf.getRawBuffer(), 0, 0);
  return ASTVisitor::optimizeLetTuple(item);
}

TupleNode *ASTToXML::optimizeCountTuple(CountTuple *item)
{
  XMLBuffer buf;
  qname(item->getVarQName(), 0, item->getVarURI(), item->getVarName(), buf);
  events_->attributeEvent(0, 0, s_name, buf.getRawBuffer(), 0, 0);
  return ASTVisitor::optimizeCountTuple(item);
}

TupleNode *ASTToXML::optimizeOrderByTuple(OrderByTuple *item)
{
  static const XMLCh s_direction[] = { 'd', 'i', 'r', 'e', 'c', 't', 'i', 'o', 'n', 0 };
  static const XMLCh s_descending[] = { 'd', 'e', 's', 'c', 'e', 'n', 'd', 'i', 'n', 'g', 0 };
  static const XMLCh s_ascending[] = { 'a', 's', 'c', 'e', 'n', 'd', 'i', 'n', 'g', 0 };
  static const XMLCh s_empty[] = { 'e', 'm', 'p', 't', 'y', 0 };
  static const XMLCh s_least[] = { 'l', 'e', 'a', 's', 't', 0 };
  static const XMLCh s_greatest[] = { 'g', 'r', 'e', 'a', 't', 'e', 's', 't', 0 };
  static const XMLCh s_stable[] = { 's', 't', 'a', 'b', 'l', 'e', 0 };
  static const XMLCh s_collation[] = { 'c', 'o', 'l', 'l', 'a', 't', 'i', 'o', 'n', 0 };

  events_->attributeEvent(0, 0, s_direction, (item->getModifiers() & OrderByTuple::DESCENDING) ? s_descending : s_ascending, 0, 0);
  events_->attributeEvent(0, 0, s_empty, (item->getModifiers() & OrderByTuple::EMPTY_LEAST) ? s_least : s_greatest, 0, 0);
  events_->attributeEvent(0, 0, s_stable, (item->getModifiers() & OrderByTuple::UNSTABLE) ? s_false : s_true, 0, 0);
  events_->attributeEvent(0, 0, s_collation, item->getCollation()->getCollationName(), 0, 0);
  return ASTVisitor::optimizeOrderByTuple(item);
}

void ASTToXML::optimizeLocation(const LocationInfo *location)
{
  static const XMLCh s_location[] = { 'l', 'o', 'c', 'a', 't', 'i', 'o', 'n', 0 };

  XMLBuffer buf;
  buf.set(location->getFile());
  buf.append(':');
  XPath2Utils::numToBuf(location->getLine(), buf);
  buf.append(':');
  XPath2Utils::numToBuf(location->getColumn(), buf);
  events_->attributeEvent(0, 0, s_location, buf.getRawBuffer(), 0, 0);
}

TupleNode *ASTToXML::optimizeTupleDebugHook(TupleDebugHook *item)
{
#ifdef SHOW_HIDDEN_AST
  optimizeLocation(item);
  return ASTVisitor::optimizeTupleDebugHook(item);
#else
  return optimizeTupleNode(item->getParent());
#endif
}

void ASTToXML::optimizeSequenceType(const SequenceType *type)
{
  static const XMLCh s_SequenceType[] = { 'S', 'e', 'q', 'u', 'e', 'n', 'c', 'e', 'T', 'y', 'p', 'e', 0 };

  if(!hasChildren_) newline();

  indent();
  events_->startElementEvent(0, 0, s_SequenceType);

  XMLBuffer buf;
  type->toBuffer(buf);
  events_->attributeEvent(0, 0, s_type, buf.getRawBuffer(), 0, 0);

  events_->endElementEvent(0, 0, s_SequenceType, 0, 0);
  newline();

  hasChildren_ = true;
}

void ASTToXML::optimizeFunctionSignature(const FunctionSignature *signature)
{
  static const XMLCh s_signature[] = { 's', 'i', 'g', 'n', 'a', 't', 'u', 'r', 'e', 0 };

  if(signature) {
    XMLBuffer buf;
    signature->toBuffer(buf, /*typeSyntax*/false);
    events_->attributeEvent(0, 0, s_signature, buf.getRawBuffer(), 0, 0);
  }
}

const XMLCh *ASTToXML::getAxisName(XQStep::Axis axis)
{
  static const XMLCh s_ancestor[] = { 'a', 'n', 'c', 'e', 's', 't', 'o', 'r', 0 };
  static const XMLCh s_ancestor_or_self[] = { 'a', 'n', 'c', 'e', 's', 't', 'o', 'r', '-', 'o', 'r', '-', 's', 'e', 'l', 'f', 0 };
  static const XMLCh s_attribute[] = { 'a', 't', 't', 'r', 'i', 'b', 'u', 't', 'e', 0 };
  static const XMLCh s_child[] = { 'c', 'h', 'i', 'l', 'd', 0 };
  static const XMLCh s_descendant[] = { 'd', 'e', 's', 'c', 'e', 'n', 'd', 'a', 'n', 't', 0 };
  static const XMLCh s_descendant_or_self[] = { 'd', 'e', 's', 'c', 'e', 'n', 'd', 'a', 'n', 't', '-', 'o', 'r', '-', 's', 'e', 'l', 'f', 0 };
  static const XMLCh s_following[] = { 'f', 'o', 'l', 'l', 'o', 'w', 'i', 'n', 'g', 0 };
  static const XMLCh s_following_sibling[] = { 'f', 'o', 'l', 'l', 'o', 'w', 'i', 'n', 'g', '-', 's', 'i', 'b', 'l', 'i', 'n', 'g', 0 };
  static const XMLCh s_namespace[] = { 'n', 'a', 'm', 'e', 's', 'p', 'a', 'c', 'e', 0 };
  static const XMLCh s_parent[] = { 'p', 'a', 'r', 'e', 'n', 't', 0 };
  static const XMLCh s_preceding[] = { 'p', 'r', 'e', 'c', 'e', 'd', 'i', 'n', 'g', 0 };
  static const XMLCh s_preceding_sibling[] = { 'p', 'r', 'e', 'c', 'e', 'd', 'i', 'n', 'g', '-', 's', 'i', 'b', 'l', 'i', 'n', 'g', 0 };
  static const XMLCh s_self[] = { 's', 'e', 'l', 'f', 0 };

  switch(axis) {
  case XQStep::ANCESTOR:
    return s_ancestor;
  case XQStep::ANCESTOR_OR_SELF:
    return s_ancestor_or_self;
  case XQStep::ATTRIBUTE:
    return s_attribute;
  case XQStep::CHILD:
    return s_child;
  case XQStep::DESCENDANT:
    return s_descendant;
  case XQStep::DESCENDANT_OR_SELF:
    return s_descendant_or_self;
  case XQStep::FOLLOWING:
    return s_following;
  case XQStep::FOLLOWING_SIBLING:
    return s_following_sibling;
  case XQStep::NAMESPACE:
    return s_namespace;
  case XQStep::PARENT:
    return s_parent;
  case XQStep::PRECEDING:
    return s_preceding;
  case XQStep::PRECEDING_SIBLING:
    return s_preceding_sibling;
  case XQStep::SELF:
    return s_self;
  }

  return 0;
}

const XMLCh *ASTToXML::getComparisonOperationName(GeneralComp::ComparisonOperation co)
{
  switch(co) {
  case GeneralComp::EQUAL:
    return Equals::name;
  case GeneralComp::NOT_EQUAL:
    return NotEquals::name;
  case GeneralComp::LESS_THAN:
    return LessThan::name;
  case GeneralComp::LESS_THAN_EQUAL:
    return LessThanEqual::name;
  case GeneralComp::GREATER_THAN:
    return GreaterThan::name;
  case GeneralComp::GREATER_THAN_EQUAL:
    return GreaterThanEqual::name;
  }

  return 0;
}

void ASTToXML::indent()
{
  static const XMLCh s_manySpaces[] = {
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 0 };

  while(indentBuffer_.getLen() < (indent_ * 2)) {
    indentBuffer_.append(s_manySpaces);
  }

  if(indent_ != 0)
    events_->textEvent(indentBuffer_.getRawBuffer(), indent_ * 2);
}

void ASTToXML::newline()
{
  static const XMLCh s_newline[] = { '\n', 0 };
  events_->textEvent(s_newline);
}

void ASTToXML::qname(const XMLCh *qname, const XMLCh *prefix, const XMLCh *uri, const XMLCh *name, XMLBuffer &buf)
{
  static const XMLCh s_lcurly[] = { '{', 0 };
  static const XMLCh s_rcurly[] = { '}', 0 };
  static const XMLCh s_colon[] = { ':', 0 };

  if(name) {
    if(prefix && !uri) {
      buf.set(prefix);
      buf.append(s_colon);
    }
    else {
      buf.set(s_lcurly);
      buf.append(uri);
      buf.append(s_rcurly);
    }
    buf.append(name);
  } else {
    buf.set(qname);
  }
}
