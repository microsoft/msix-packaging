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

#include "../../config/xqilla_config.h"
#include "XQillaBuilderImpl.hpp"
#include "XQillaDocumentImpl.hpp"
#include "XQillaXMLGrammarPoolImpl.hpp"

#include <xercesc/framework/XMLGrammarPool.hpp>
#include <xercesc/framework/XMLValidator.hpp>
#include <xercesc/util/XMLUni.hpp>
#include <xercesc/dom/impl/DOMDocumentImpl.hpp>
#include <xercesc/framework/XMLGrammarDescription.hpp>
#include <xercesc/framework/XMLSchemaDescription.hpp>
#include <xercesc/internal/XMLScanner.hpp>
#include <xercesc/internal/XMLScannerResolver.hpp>

#if _XERCES_VERSION >= 30000
#include <xercesc/framework/Wrapper4DOMLSInput.hpp>
#else
#include <xercesc/framework/Wrapper4DOMInputSource.hpp>
#endif

XERCES_CPP_NAMESPACE_USE;

const XMLCh XQillaBuilderImpl::gXQilla[] =   // Points to "XPath2"
{chLatin_X, chLatin_P, chLatin_a, chLatin_t, chLatin_h, chDigit_2, chNull};




XQillaBuilderImpl::XQillaBuilderImpl(XMLValidator* const   valToAdopt,
                                     MemoryManager* const  manager,
                                     XMLGrammarPool* const gramPool,
                                     bool adoptGramPool)
  :
#if _XERCES_VERSION >= 30000
	DOMLSParserImpl(valToAdopt, manager, gramPool),
#else
	DOMBuilderImpl(valToAdopt, manager, gramPool),
#endif
  gramPool_(adoptGramPool ? gramPool : 0)
{
}

XQillaBuilderImpl::~XQillaBuilderImpl()
{
  delete gramPool_;
}

// ---------------------------------------------------------------------------
//  DOMBuilderImpl: Parsing methods
// ---------------------------------------------------------------------------
#if _XERCES_VERSION >= 30000
DOMDocument* XQillaBuilderImpl::parse(const DOMLSInput* source)
{
    initParser();
    
    Wrapper4DOMLSInput isWrapper((DOMLSInput*)source, 0, false, getMemoryManager());

    AbstractDOMParser::parse(isWrapper);

    return getDocumentAndAddGrammar();

}
#else
DOMDocument* XQillaBuilderImpl::parse(const DOMInputSource& source)
{
    initParser();
    
    Wrapper4DOMInputSource isWrapper((DOMInputSource*) &source, false, getMemoryManager());

    AbstractDOMParser::parse(isWrapper);

    return getDocumentAndAddGrammar();

}
#endif

void XQillaBuilderImpl::initParser() {
    //set it here in case someone has messed it up.
#if _XERCES_VERSION >= 30000
    setParameter(XMLUni::fgXercesParserUseDocumentFromImplementation, (void*)gXQilla);
#else
    setProperty(XMLUni::fgXercesParserUseDocumentFromImplementation, (void*)gXQilla);
#endif
}


DOMDocument* XQillaBuilderImpl::getDocumentAndAddGrammar() {
    DOMDocument *doc = 0;

#if _XERCES_VERSION >= 30000
    if (getParameter(XMLUni::fgXercesUserAdoptsDOMDocument) != 0)
#else
    if (getFeature(XMLUni::fgXercesUserAdoptsDOMDocument))
#endif
        doc = adoptDocument();
    else
        doc = getDocument();
    if(doc == 0) {
      return 0;
    }

    if(getParameter(XMLUni::fgXercesDOMHasPSVIInfo)) {
      //we copy this gramamr and reset the parser one in the process.
      XMLGrammarPool *oldGrPool = getGrammarResolver()->getGrammarPool();
      XQillaXMLGrammarPoolImpl *gr = new (getMemoryManager()) XQillaXMLGrammarPoolImpl(getMemoryManager());

      // manually copy string pool contents to work around XERCESC-1798.
      const XMLStringPool* src = oldGrPool->getURIStringPool();
      XMLStringPool* dst = gr->getURIStringPool();

      for (unsigned int i = 1; i < src->getStringCount() + 1; ++i)
        dst->addOrFind (src->getValueForId(i));

      RefHashTableOfEnumerator< Grammar> enumerator
        = oldGrPool->getGrammarEnumerator();

      while(enumerator.hasMoreElements()) {
        Grammar &g = enumerator.nextElement();
        gr->cacheGrammar(getGrammarResolver()->orphanGrammar(g.getGrammarDescription()->getGrammarKey()));
      }

      ((XQillaDocumentImpl*)doc)->setGrammarPool(gr, true);
    }

    return doc;
}

DOMDocument* XQillaBuilderImpl::parseURI(const XMLCh* const systemId)
{
    initParser();
    AbstractDOMParser::parse(systemId);
    return getDocumentAndAddGrammar();
}

DOMDocument* XQillaBuilderImpl::parseURI(const char* const systemId)
{
    initParser();
    AbstractDOMParser::parse(systemId);
    return getDocumentAndAddGrammar();
}
