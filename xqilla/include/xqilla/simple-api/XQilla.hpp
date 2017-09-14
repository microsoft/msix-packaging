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

#ifndef _XQILLA_07637_HPP
#define _XQILLA_07637_HPP

#include <xqilla/framework/XQillaExport.hpp>

#include <xercesc/framework/MemoryManager.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMemory.hpp>

class DynamicContext;
class XPath2MemoryManager;
class XQillaConfiguration;
class Optimizer;
class DelayedFuncFactory;
class XQQuery;

XERCES_CPP_NAMESPACE_BEGIN
class InputSource;
class XMLBuffer;
XERCES_CPP_NAMESPACE_END

namespace CompileDelayedModule {
void compile(const XMLCh *queryFile);
}

/**
 * Provides factory methods for creating XQQuery and DynamicContext objects.
 * 
 * This class calls XQillaPlatformUtils::initialize() when it is constructed, and
 * XQillaPlatformUtils::terminate() when it destructs, so there is no need to seperately
 * initialize or terminate either Xerces or XQilla.
 */
class XQILLA_API XQilla : public XERCES_CPP_NAMESPACE_QUALIFIER XMemory
{
public:
  /// Flags used by the XQilla methods. These are used by bitwise OR-ing (|) their values together.
  enum Flags {
    NO_STATIC_RESOLUTION = 0x1, ///< Don't perform static resolution or any optimization
    NO_ADOPT_CONTEXT     = 0x2, ///< Don't adopt the context and delete it when the XQQuery is deleted
    DEBUG_QUERY          = 0x4, ///< Build debugging hooks into the query to enable the use of DebugListener
    NO_OPTIMIZATION      = 0x8, ///< Don't perform optimization
    NO_DEFAULT_MODULES   = 0x10 ///< Don't automatically import the default modules
  };

  /// Enumeration used to select a language to parse
  enum Language {
    XQUERY     = 0x00,
    XPATH2     = 0x01,
    FULLTEXT   = 0x02,
    UPDATE     = 0x04,
    EXTENSIONS = 0x08,
    XSLT2      = 0x10,
    VERSION3   = 0x20,

    XQUERY_FULLTEXT          = (XQUERY | FULLTEXT),
    XQUERY_UPDATE            = (XQUERY | UPDATE),
    XQUERY_FULLTEXT_UPDATE   = (XQUERY | FULLTEXT | UPDATE),
    XPATH2_FULLTEXT          = (XPATH2 | FULLTEXT),
    XSLT2_FULLTEXT           = (XSLT2 | FULLTEXT),

    XQUERY3                  = (XQUERY | VERSION3),
    XQUERY3_FULLTEXT         = (XQUERY | VERSION3 | FULLTEXT),
    XQUERY3_UPDATE           = (XQUERY | VERSION3 | UPDATE),
    XQUERY3_FULLTEXT_UPDATE  = (XQUERY | VERSION3 | FULLTEXT | UPDATE),
    XPATH3                   = (XPATH2 | VERSION3),
    XPATH3_FULLTEXT          = (XPATH2 | VERSION3 | FULLTEXT),
    XSLT3                    = (XSLT2 | VERSION3),
    XSLT3_FULLTEXT           = (XSLT2 | VERSION3 | FULLTEXT),
  };
                                                                                                                                                              
  /**
   * Constructs the object. In the process, XQillaPlatformUtils::initialize() is
   * called to initialize XQilla and Xerces.
   *
   * @param memMgr If provided, this is the MemoryManager used to initialize Xerces.
   */
  XQilla(XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager *memMgr = 0);

  /**
   * Destructs the object. In the process, XQillaPlatformUtils::terminate() is
   * called to terminate XQilla and Xerces.
   */
  ~XQilla();

	/** @name Parsing Methods */
	// @{

  /**
   * Parse the expression contained in the given query string.
   *
   * @param query A string containing the expression to parse.
   * @param context If specified, the context to use for parsing this expression. A
   * default context is used if this parameter is 0.
   * @param queryFile The name of the file that query originates in. This is passed
   * back to the user in an XQException if an error occurs.
   * @param flags A bitwise OR of the Flags constants, that control aspects of how
   * the XQQuery object is created.
   * @param memMgr If specified, the memory manager used to create the XQQuery object.
   *
   * @return The object that represents the parsed expression.
   *
   * @exception XQException If a parse error occurs.
   */
  static XQQuery* parse(const XMLCh* query,
                        DynamicContext* context = 0,
                        const XMLCh* queryFile = NULL,
                        unsigned int flags = 0,
                        XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager *memMgr =
                        XERCES_CPP_NAMESPACE_QUALIFIER XMLPlatformUtils::fgMemoryManager,
                        XQQuery *result = 0);

  /**
   * Parse the expression from the given InputSource.
   *
   * @param querySrc An InputSource which the expression will be parsed from.
   * @param context If specified, the context to use for parsing this expression. A
   * default context is used if this parameter is 0.
   * @param flags A bitwise OR of the Flags constants, that control aspects of how
   * the XQQuery object is created.
   * @param memMgr If specified, the memory manager used to create the XQQuery object.
   *
   * @return The object that represents the parsed expression.
   *
   * @exception XQException If a parse error occurs.
   */
  static XQQuery* parse(const XERCES_CPP_NAMESPACE_QUALIFIER InputSource& querySrc,
                        DynamicContext* context = 0,
                        unsigned int flags = 0,
                        XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager *memMgr =
                        XERCES_CPP_NAMESPACE_QUALIFIER XMLPlatformUtils::fgMemoryManager,
                        XQQuery *result = 0);

  /**
   * Parse the expression residing at the given URL.
   *
   * @param queryFile The URL of the expression to parse.
   * @param context If specified, the context to use for parsing this expression. A
   * default context is used if this parameter is 0.
   * @param flags A bitwise OR of the Flags constants, that control aspects of how
   * the XQQuery object is created.
   * @param memMgr If specified, the memory manager used to create the XQQuery object.
   *
   * @return The object that represents the parsed expression.
   *
   * @exception XQException If a parse error occurs.
   */
  static XQQuery* parseFromURI(const XMLCh* queryFile,
                               DynamicContext* context = 0,
                               unsigned int flags = 0,
                               XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager *memMgr =
                               XERCES_CPP_NAMESPACE_QUALIFIER XMLPlatformUtils::fgMemoryManager,
                               XQQuery *result = 0);

  //@}

  /** @name Factory Methods */
  // @{

  /**
   * Creates a context suitable for parsing an expression with.
   * @param memMgr The memory manager used to create the DynamicContext.
   * @return An implementation of DynamicContext
   */
  static DynamicContext *createContext(Language language = XQUERY,
                                       XQillaConfiguration *conf = 0,
                                       XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager *memMgr =
                                       XERCES_CPP_NAMESPACE_QUALIFIER XMLPlatformUtils::fgMemoryManager);

  //@}

private:
  static bool readQuery(const XMLCh* queryFile, 
                        XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager* memMgr, 
                        XERCES_CPP_NAMESPACE_QUALIFIER XMLBuffer& queryText);
  static bool readQuery(const XERCES_CPP_NAMESPACE_QUALIFIER InputSource& querySrc, 
                        XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager* memMgr, 
                        XERCES_CPP_NAMESPACE_QUALIFIER XMLBuffer& queryText);
  static Optimizer *createOptimizer(DynamicContext *context, unsigned int flags);

  static void compileDelayedModule(const XMLCh* queryFile, XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager *memMgr =
                                   XERCES_CPP_NAMESPACE_QUALIFIER XMLPlatformUtils::fgMemoryManager);

  friend void CompileDelayedModule::compile(const XMLCh *queryFile);
  friend class DelayedFuncFactory;
};

#endif
