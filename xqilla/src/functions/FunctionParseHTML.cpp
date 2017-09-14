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

#include "../config/xqilla_config.h"

#include <xqilla/functions/FunctionParseHTML.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/exceptions/FunctionException.hpp>
#include <xqilla/exceptions/XMLParseException.hpp>
#include <xqilla/ast/StaticAnalysis.hpp>
#include <xqilla/events/QueryPathTreeFilter.hpp>
#include <xqilla/events/SequenceBuilder.hpp>
#include <xqilla/utils/UTF8Str.hpp>

#ifdef HAVE_LIBTIDY
#include <tidy.h>
#include <buffio.h>
#endif

#include <xercesc/util/XMLString.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>

XERCES_CPP_NAMESPACE_USE;

const XMLCh FunctionParseHTML::name[] = {
  chLatin_p, chLatin_a, chLatin_r, chLatin_s, chLatin_e, chDash, chLatin_h, chLatin_t, chLatin_m, chLatin_l, chNull 
};
const unsigned int FunctionParseHTML::minArgs = 1;
const unsigned int FunctionParseHTML::maxArgs = 1;

/**
 * xqilla:parse-html($html as xs:string?) as document?
 */
FunctionParseHTML::FunctionParseHTML(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
  : XQillaFunction(name, "($html as xs:string?) as document-node()?", args, memMgr),
    queryPathTree_(0)
{
}

ASTNode *FunctionParseHTML::staticTypingImpl(StaticContext *context)
{
  _src.clearExceptType();

  _src.setProperties(StaticAnalysis::DOCORDER | StaticAnalysis::GROUPED |
                     StaticAnalysis::PEER | StaticAnalysis::SUBTREE | StaticAnalysis::ONENODE);
  _src.creative(true);

  calculateSRCForArguments(context);
  return this;
}

Sequence FunctionParseHTML::createSequence(DynamicContext* context, int flags) const
{
  Item::Ptr item = getParamNumber(1, context)->next(context);

  if(item.isNull()) return Sequence(context->getMemoryManager());

  AutoDelete<SequenceBuilder> builder(context->createSequenceBuilder());
  QueryPathTreeFilter qptf(queryPathTree_, builder.get());
  EventHandler *handler = queryPathTree_ ? (EventHandler*)&qptf : (EventHandler*)builder.get();

  parseHTML(item->asString(context), handler, context, this);
  
  handler->endEvent();
  return builder->getSequence();
}

#ifdef HAVE_LIBTIDY

// void dumpNode(TidyDoc tdoc, TidyNode tnod, int indent, EventHandler *handler)
// {
//   switch(tidyNodeGetType(tnod)) {
//   case TidyNode_Root:
//     handler->startDocumentEvent(0, 0);

//     printf( "\%*.*sNode: Root\n", indent, indent, " ");

//     TidyNode child;
//     for(child = tidyGetChild(tnod); child; child = tidyGetNext(child)) {
//       dumpNode(tdoc, child, indent + 4, handler);
//     }

//     handler->endDocumentEvent();
//     return;
//     break;
//   case TidyNode_Comment: {
//     TidyBuffer buf;
//     tidyBufInit(&buf);
//     tidyNodeGetText(tdoc, tnod, &buf);

//     printf( "\%*.*sNode: Comment = \"%s\"\n", indent, indent, " ", UTF8((XMLCh*)buf.bp) );
//     tidyBufFree(&buf);
//     return;
//     break;
//   }
//   case TidyNode_ProcIns: {
//     TidyBuffer buf;
//     tidyBufInit(&buf);
//     tidyNodeGetText(tdoc, tnod, &buf);

//     printf( "\%*.*sNode: ProcIns = \"%s\"\n", indent, indent, " ", UTF8((XMLCh*)buf.bp) );
//     tidyBufFree(&buf);
//     return;
//     break;
//   }
//   case TidyNode_Text: {
//     TidyBuffer buf;
//     tidyBufInit(&buf);
//     tidyNodeGetText(tdoc, tnod, &buf);

//     printf( "\%*.*sNode: Text = \"%s\"\n", indent, indent, " ", UTF8((XMLCh*)buf.bp) );
//     tidyBufFree(&buf);
//     return;
//     break;
//   }
//   case TidyNode_CDATA: {
//     TidyBuffer buf;
//     tidyBufInit(&buf);
//     tidyNodeGetText(tdoc, tnod, &buf);

//     printf( "\%*.*sNode: CDATA = \"%s\"\n", indent, indent, " ", UTF8((XMLCh*)buf.bp) );
//     tidyBufFree(&buf);
//     return;
//     break;
//   }

//   case TidyNode_Start:
//   case TidyNode_End:
//   case TidyNode_StartEnd: {
// //     handler->startElementEvent(0, 0, );

//     printf( "\%*.*sNode: \%s\n", indent, indent, " ", tidyNodeGetName(tnod));

//     TidyNode child;
//     for(child = tidyGetChild(tnod); child; child = tidyGetNext(child)) {
//       dumpNode(tdoc, child, indent + 4, handler);
//     }

// //     handler->endDocumentEvent();
//     return;
//     break;
//   }
//   case TidyNode_DocType:
//   case TidyNode_Section:
//   case TidyNode_Asp:
//   case TidyNode_Jste:
//   case TidyNode_Php:
//   case TidyNode_XmlDecl:
//     // Ignore
//     return;
//     break;
//   }
// }

void FunctionParseHTML::parseHTML(const XMLCh *html, EventHandler *handler, DynamicContext *context, const LocationInfo *location)
{
  unsigned int len = XMLString::stringLen(html) * sizeof(XMLCh);

  TidyDoc tdoc = tidyCreate();
  int rc = tidySetCharEncoding(tdoc, "utf16le"); // TBD Check this value - jpcs

  TidyBuffer buf;
  tidyBufInit(&buf);
  tidyBufAttach(&buf, (byte*)html, len);

  TidyBuffer errbuf;
  tidyBufInit(&errbuf);
  TidyBuffer output;
  tidyBufInit(&output);

  if(rc >= 0)
    rc = tidyOptSetBool(tdoc, TidyXmlOut, yes) ? 0 : -1;
  if(rc >= 0)
    rc = tidyOptSetBool(tdoc, TidyMark, no) ? 0 : -1;
  if(rc >= 0)
    rc = tidyOptSetBool(tdoc, TidyNumEntities, yes) ? 0 : -1;
  if(rc >= 0)
    rc = tidySetErrorBuffer(tdoc, &errbuf);
  if(rc >= 0)
    rc = tidyParseBuffer(tdoc, &buf);
  if(rc >= 0)
    rc = tidyCleanAndRepair(tdoc);
//   if(rc >= 0)
//     rc = tidyRunDiagnostics(tdoc);

//   dumpNode(tdoc, tidyGetRoot(tdoc), 0, handler);
  
  if(rc > 1)
    rc = tidyOptSetBool(tdoc, TidyForceOutput, yes) ? rc : -1;
  if(rc >= 0)
    rc = tidySaveBuffer(tdoc, &output);
  const XMLCh *xml = (XMLCh*)output.bp;

  MemBufInputSource src((XMLByte*)xml, XMLString::stringLen(xml) * sizeof(XMLCh), name);
  src.setEncoding(XMLUni::fgUTF16EncodingString);

  try {
    ((DocumentCache*)context->getDocumentCache())->parseDocument(src, handler, context);
  }
  catch(XMLParseException &e) {
    XQThrow3(FunctionException, X("FunctionParseHTML::parseHTML"), e.getError(), location);
  }

  tidyBufFree(&errbuf);
  tidyBufFree(&output);
  tidyRelease(tdoc);
}

#else

void FunctionParseHTML::parseHTML(const XMLCh *html, EventHandler *handler, DynamicContext *context, const LocationInfo *location)
{
  XQThrow3(FunctionException, X("FunctionParseHTML::parseHTML"),
           X("XQilla was not compiled with HTML Tidy support [err:XQILLA]"), location);
}

#endif
