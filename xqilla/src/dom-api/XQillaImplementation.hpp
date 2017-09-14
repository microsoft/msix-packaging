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

#ifndef __XQILLAIMPLEMENTATION_HPP
#define __XQILLAIMPLEMENTATION_HPP

#include <xqilla/framework/XQillaExport.hpp>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMImplementationSource.hpp>

class XPath2MemoryManager;

XERCES_CPP_NAMESPACE_BEGIN
class XMLGrammarPool;

XERCES_CPP_NAMESPACE_END

/**
 * The <code>XQillaImplentation</code> class implements the 
 * <code>DOMImplementation</code> interface and the 
 * <code>DOMImplementationSource</code> interface and provides a number of methods
 * for performing operations that are independent of any particular instance
 * of the document object model.
 */
class XQILLA_API XQillaImplementation: public XERCES_CPP_NAMESPACE_QUALIFIER DOMImplementation,
                                          public XERCES_CPP_NAMESPACE_QUALIFIER DOMImplementationSource
{
protected:
  XQillaImplementation();
public:
  /** @name Destructor */
  //@{
  /**
   * Destructor
   *
   */
  virtual ~XQillaImplementation();
  //@}

  /**  Factory method for getting a XQillaImplementation object.
   *     The XQilla implementation retains ownership of the returned object.
   *     Application code should NOT delete it.
   */
  static XQillaImplementation* getDOMImplementationImpl();

  /**  Initialize the XQillaImplementation
   */
  static void initialize();

  /**  Terminate the XQillaImplementation
   */
  static void terminate();

  // DOMImplementationLS methods
#if _XERCES_VERSION >= 30000
    virtual XERCES_CPP_NAMESPACE_QUALIFIER DOMLSParser* createLSParser(const XERCES_CPP_NAMESPACE_QUALIFIER DOMImplementationLS::DOMImplementationLSMode mode,
                                        const XMLCh* const     schemaType,
                                        XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager* const   manager,
                                        XERCES_CPP_NAMESPACE_QUALIFIER XMLGrammarPool*  const gramPool);
    virtual XERCES_CPP_NAMESPACE_QUALIFIER DOMLSSerializer* createLSSerializer(XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager* const manager);
    virtual XERCES_CPP_NAMESPACE_QUALIFIER DOMLSInput* createLSInput(XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager* const manager);
    virtual XERCES_CPP_NAMESPACE_QUALIFIER DOMLSOutput* createLSOutput(XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager* const manager);
#else
  virtual XERCES_CPP_NAMESPACE_QUALIFIER DOMWriter* createDOMWriter(XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager* const manager = XERCES_CPP_NAMESPACE_QUALIFIER XMLPlatformUtils::fgMemoryManager);

  /**
   * We create our own DOMBulider that creates a XQillaDocument. It also sets various parser options so we can 
   * use the grammars correctly. Please note that if you pass your own XMLGrammarPool in then the special
   * XQuery types will not be available during parse
   **/
  virtual XERCES_CPP_NAMESPACE_QUALIFIER DOMBuilder* createDOMBuilder(const short mode,
                                                                      const XMLCh* const schemaType,
                                                                      XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager* const manager = XERCES_CPP_NAMESPACE_QUALIFIER XMLPlatformUtils::fgMemoryManager,
                                                                      XERCES_CPP_NAMESPACE_QUALIFIER XMLGrammarPool* const gramPool = 0);
  virtual XERCES_CPP_NAMESPACE_QUALIFIER DOMInputSource* createDOMInputSource();
#endif

  // DOMImplementationSource methods
  virtual XERCES_CPP_NAMESPACE_QUALIFIER DOMImplementation* getDOMImplementation(const XMLCh* features) const;
#if _XERCES_VERSION >= 30000
  virtual XERCES_CPP_NAMESPACE_QUALIFIER DOMImplementationList* getDOMImplementationList(const XMLCh* features) const;
#endif

  // DOMImplementation methods
  virtual bool hasFeature(const  XMLCh * feature,  const  XMLCh * version) const;
  virtual XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument* createDocument(const XMLCh *namespaceURI, 
                                      const XMLCh *qualifiedName, 
                                      XERCES_CPP_NAMESPACE_QUALIFIER DOMDocumentType *doctype, 
                                      XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager* const manager = XERCES_CPP_NAMESPACE_QUALIFIER XMLPlatformUtils::fgMemoryManager);
  virtual XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument* createDocument(XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager* const manager = XERCES_CPP_NAMESPACE_QUALIFIER XMLPlatformUtils::fgMemoryManager);
  virtual XERCES_CPP_NAMESPACE_QUALIFIER DOMDocumentType *createDocumentType(const XMLCh *qualifiedName,const XMLCh * publicId, const XMLCh *systemId);

#if _XERCES_VERSION >= 30000
  virtual void* getFeature(const XMLCh* feature, const XMLCh* version) const;
#else
  virtual XERCES_CPP_NAMESPACE_QUALIFIER DOMImplementation* getInterface(const XMLCh* feature);
#endif

  /** The "XPath2" feature 
   */
  static const XMLCh gXQilla[];

  /** The "3.0" version
   */
  static const XMLCh g3_0[];

  XERCES_CPP_NAMESPACE_QUALIFIER DOMImplementation *domImpl;
private:
	static XQillaImplementation* gDomimp;
};

#endif

