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

#ifndef XQQUERY_HPP
#define XQQUERY_HPP

#include <xqilla/framework/XQillaExport.hpp>
#include <vector>
#include <string>
#include <xercesc/util/XMemory.hpp>
#include <xercesc/sax/InputSource.hpp>
#include <xqilla/runtime/ResultImpl.hpp>
#include <xqilla/context/StaticContext.hpp>

#include <xercesc/util/RefHashTableOf.hpp>

class DynamicContext;
class XQUserFunction;
class XQGlobalVariable;
class XQQuery;
class DelayedModule;
class DelayedFuncFactory;
class StaticTyper;

typedef std::vector<XQGlobalVariable*, XQillaAllocator<XQGlobalVariable*> > GlobalVariables;
typedef std::vector<XQQuery*, XQillaAllocator<XQQuery*> > ImportedModules;
typedef std::vector<DelayedFuncFactory*, XQillaAllocator<DelayedFuncFactory*> > DelayedFunctions;

typedef XERCES_CPP_NAMESPACE_QUALIFIER RefHashTableOf<XQQuery> ModuleMap;

class XQILLA_API ModuleCache : public XERCES_CPP_NAMESPACE_QUALIFIER XMemory
{
public:
  ModuleCache(XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager *mm);

  void put(XQQuery *module);
  XQQuery *getByURI(const XMLCh *uri) const;
  XQQuery *getByNamespace(const XMLCh *ns) const;

  ModuleMap byURI_;
  ModuleMap byNamespace_;
  ImportedModules ordered_;
};

/**
 * Encapsulates a query expression. XQQuery objects are thread safe, and can be
 * executed from multiple threads at the same time - provided a DynamicContext is
 * used per thread.
 *
 * Instances of this class can be created by using the parsing methods on the XQilla
 * object.
 *
 * This class is dependant on the DynamicContext that was used to create it. This
 * context must not be deleted before the XQQuery object. Normally this is acheived
 * by the XQQuery adopting the DynamicContext and deleting it when it is deleted.
 * However, this behaviour can be disabled by using the XQilla::NO_ADOPT_CONTEXT
 * flag when creating the XQQuery.
 */
class XQILLA_API XQQuery : public XERCES_CPP_NAMESPACE_QUALIFIER XMemory
{
public:
  /// Destructor.
  ~XQQuery();

	/** @name Runtime Methods */
	// @{

  /**
   * Creates a DynamicContext based on the static context used to parse this query.
   * This DynamicContext will be suitable for using to call execute().
   *
   * @param memMgr The memory manager used to create the DynamicContext.
   *
   * @return An implementation of DynamicContext
   */
  DynamicContext *createDynamicContext(XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager *memMgr =
                                       XERCES_CPP_NAMESPACE_QUALIFIER XMLPlatformUtils::fgMemoryManager) const;

  /**
   * Executes the query using the given DynamicContext, returning
   * a lazy iterator over the results.
   *
   * @param context The DynamicContext to use to execute the
   * query. This contains runtime values for variables, the value
   * of the context item, and other relevent information.
   *
   * @return The Result object - a lazy iterator over the query
   * results.
   *
   * @exception XQException If a runtime error occurs
   */
  Result execute(DynamicContext* context) const;

  /**
   * Executes the query or stylesheet using the given context item and DynamicContext, returning
   * a lazy iterator over the results.
   *
   * @param contextItem The context item to use to execute the query.
   * @param context The DynamicContext to use to execute the
   * query. This contains runtime values for variables, the value
   * of the context item, and other relevent information.
   *
   * @return The Result object - a lazy iterator over the query
   * results.
   *
   * @exception XQException If a runtime error occurs
   */
  Result execute(const Item::Ptr &contextItem, DynamicContext *context) const;

  /**
   * Executes the stylesheet starting at the named template using the given DynamicContext, returning
   * a lazy iterator over the results.
   *
   * @param templateQName The name of the template to start executing. If the QName
   * includes a prefix, it is resolved using the in-scope namespaces for the stylesheet.
   * @param context The DynamicContext to use to execute the
   * query. This contains runtime values for variables, the value
   * of the context item, and other relevent information.
   *
   * @return The Result object - a lazy iterator over the query
   * results.
   *
   * @exception XQException If a runtime error occurs
   */
  Result execute(const XMLCh *templateQName, DynamicContext *context) const;

  /**
   * Executes the query using the given DynamicContext, sending
   * the output of the query to the given EventHandler.
   *
   * @param context The DynamicContext to use to execute the
   * query. This contains runtime values for variables, the value
   * of the context item, and other relevent information.
   *
   * @exception XQException If a runtime error occurs
   */
  void execute(EventHandler *events, DynamicContext* context) const;

  /**
   * Executes the query or stylesheet using the given context item and  DynamicContext, sending
   * the output of the query to the given EventHandler.
   *
   * @param contextItem The context item to use to execute the query.
   * @param context The DynamicContext to use to execute the
   * query. This contains runtime values for variables, the value
   * of the context item, and other relevent information.
   *
   * @exception XQException If a runtime error occurs
   */
  void execute(EventHandler *events, const Item::Ptr &contextItem, DynamicContext *context) const;

  /**
   * Executes the stylesheet starting at the named template using the given DynamicContext, sending
   * the output of the query to the given EventHandler.
   *
   * @param templateQName The name of the template to start executing. If the QName
   * includes a prefix, it is resolved using the in-scope namespaces for the stylesheet.
   * @param context The DynamicContext to use to execute the
   * query. This contains runtime values for variables, the value
   * of the context item, and other relevent information.
   *
   * @exception XQException If a runtime error occurs
   */
  void execute(EventHandler *events, const XMLCh *templateQName, DynamicContext *context) const;

  /**
   * Perform static resolution on the query. This operation is not
   * thread safe, and should only be performed once.
   *
   * Static resolution resolves prefixes to namespace URIs, variable
   * names to variables, function names to functions, and performs
   * some basic query optimisation.
   *
   * Normally this is done as part of parsing the query expression.
   * However, if the XQilla::NO_STATIC_RESOLUTION flag was specified
   * when the query was parsed, this method will have to be called to
   * perform this step manually.
   *
   * @param context If specified, the StaticContext to use to statically
   * resolve this object. By default, the context used to parse the query
   * is used. This should not need to be changed under normal usage.
   *
   * @exception XQException If a static resolution time error occurs
   */
  void staticResolution();

  /**
   * Perform type calculation and related optimizations. This operation
   * is safe to call multiple times.
   *
   * Normally this step is done as part of static resolution.
   *
   * @param context If specified, the StaticContext to use to statically
   * resolve this object. By default, the context used to parse the query
   * is used. This should not need to be changed under normal usage.
   *
   * @exception XQException If a static resolution time error occurs
   */
  void staticTyping(StaticTyper *styper = 0);
  bool staticTypingOnce(StaticTyper *styper = 0);

  //@}

  /** @name Query Information */
  // @{

  /// Returns the expression that was parsed to create this XQQuery object.
  const XMLCh* getQueryText() const { return m_szQueryText; }
  void setQueryText(const XMLCh *v);

  /// Returns the query plan as XML. This is designed for debug and informative purposes only.
  std::string getQueryPlan() const;

  //@}

  /** @name Getters and Setters */
  // @{

  /// Get the query body as an ASTNode
  ASTNode* getQueryBody() const;
  /// Set the query body to an ASTNode
  void setQueryBody(ASTNode* query);

  /// Adds an XQUserFunction to the query (also adds it as a template if necessary)
  void addFunction(XQUserFunction* fnDef);
  /// Returns a vector of all XQUserFunction objects from the query
  const UserFunctions &getFunctions() const { return m_userDefFns; }

  /// Adds a function defined in XQuery syntax. Parsing is delayed until the function
  /// is actually needed.
  void addDelayedFunction(const XMLCh *uri, const XMLCh *name, size_t numArgs,
                          const XMLCh *functionDeclaration, bool isPrivate = false,
                          int line = 1, int column = 1);
  void addDelayedFunction(const XMLCh *uri, const XMLCh *name, size_t numArgs,
                          const char *functionDeclaration, bool isPrivate = false,
                          int line = 1, int column = 1);
  /// Returns a vector of all XQUserFunction objects from the query
  const DelayedFunctions &getDelayedFunctions() const { return m_delayedFunctions; }

  /// Adds a XQGlobalVariable to the query
  void addVariable(XQGlobalVariable* varDef);
  /// Returns a vector of all XQGlobalVariable objects from the query
  const GlobalVariables &getVariables() const { return m_userDefVars; }

  /// Returns a vector of all XQGlobalVariable objects from the query
  const ImportedModules &getImportedModules() const { return m_importedModules; }

  ModuleCache *getModuleCache() const { return m_moduleCache; }
  bool isModuleCacheOwned() const { return m_moduleCacheOwned; }

  /// Returns the name of the file that this query was parsed from.
  const XMLCh* getFile() const { return m_szCurrentFile; }

  /// Sets the name of the file that this query was parsed from.
  void setFile(const XMLCh* file);

  bool getVersion3() const { return m_version3; }
  void setVersion3(bool v) { m_version3 = v; }

  const DynamicContext *getStaticContext() const { return m_context; }

  XQQuery *getNext() const { return m_next; }
  void setNext(XQQuery *n) { m_next = n; }

  //@}

  /** @name Module Methods */
  // @{

  /// Sets whether this query is a module or not
  void setIsLibraryModule(bool bIsModule=true);
  /// Returns whether this query is a module or not
  bool getIsLibraryModule() const;
  /// Sets the module target namespace for this query
  void setModuleTargetNamespace(const XMLCh* uri);
  /// Gets the module taget namespace for this query
  const XMLCh* getModuleTargetNamespace() const;
  /// Performs a module import from the given target namespace and locations
  void importModule(const XMLCh* szUri, VectorOfStrings* locations, const LocationInfo *location);
  void importModule(XQQuery *module);

  XQQuery *findModuleForVariable(const XMLCh *uri, const XMLCh *name);
  XQQuery *findModuleForFunction(const XMLCh *uri, const XMLCh *name, int numArgs);

  //@}

private:
  /// Private constructor - Can be accessed by an XQilla object, as it is a friend.
  XQQuery(DynamicContext *context, bool contextOwned, ModuleCache *moduleCache,
          XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager *memMgr);

  XQQuery(const XQQuery &);
  XQQuery &operator=(const XQQuery &);

  XQQuery *parseModule(const XMLCh *ns, const XMLCh *at, const LocationInfo *location) const;
  void executeProlog(DynamicContext *context) const;

private:
  /// The memory manager used to create this object
  XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager* m_memMgr;

  /// The context used to create this query
  DynamicContext *m_context;
  bool m_contextOwned;

  ASTNode* m_query;

  bool m_bIsLibraryModule;
  const XMLCh* m_szTargetNamespace;
  const XMLCh* m_szQueryText;
  const XMLCh* m_szCurrentFile;

  UserFunctions m_userDefFns;
  DelayedFunctions m_delayedFunctions;
  GlobalVariables m_userDefVars;
  ImportedModules m_importedModules;

  ModuleCache *m_moduleCache;
  bool m_moduleCacheOwned;

  bool m_version3;

  enum { BEFORE, DURING, AFTER } m_staticTyped;

  // The next module with the same target namespace
  XQQuery *m_next;

  friend class QueryResult;
  friend class XQilla;
  friend class DelayedModule;
};

#endif
