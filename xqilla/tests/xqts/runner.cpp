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

#ifdef _MSC_VER
#pragma warning(disable: 4786)
#endif

#include <xqilla/xqts/TestSuiteParser.hpp>
#include <xqilla/xqts/TestSuiteResultListener.hpp>
#include <xqilla/xqts/TestSuiteRunner.hpp>

#include <set>
#include <iostream>
#include <fstream>
#include <stdio.h>

#include <xercesc/framework/URLInputSource.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/util/XMLEntityResolver.hpp>
#include <xercesc/framework/MemBufFormatTarget.hpp>
#include <xercesc/dom/DOMException.hpp>

#include <xqilla/xqilla-simple.hpp>
#include <xqilla/context/VariableStore.hpp>
#include <xqilla/utils/XQillaPlatformUtils.hpp>
#include <xqilla/events/ContentSequenceFilter.hpp>

#if defined(XERCES_HAS_CPP_NAMESPACE)
XERCES_CPP_NAMESPACE_USE
#endif
using namespace std;

class XQillaTestSuiteRunner : public TestSuiteRunner, private XMLEntityResolver, private ModuleResolver, private URIResolver
{
public:
  XQillaTestSuiteRunner(const string &singleTest, TestSuiteResultListener *results, XQillaConfiguration *conf, XQilla::Language lang, FastXDMConfiguration *fastConf = 0, XercesConfiguration *xercesConf = 0, bool userSetParserLang = false);
  virtual ~XQillaTestSuiteRunner();

  virtual void addSource(const string &id, const string &filename, const string &schema);
  virtual void addSchema(const string &id, const string &filename, const string &uri);
  virtual void addModule(const string &id, const string &filename);
  virtual void addCollectionDoc(const string &id, const string &filename);

  virtual void startTestGroup(const string &name);
  virtual void endTestGroup();

  virtual void runTestCase(const TestCase &testCase);
  virtual void detectParserLang(const string &testSuiteNamespace);

private:
  virtual InputSource* resolveEntity(XMLResourceIdentifier* resourceIdentifier);
  virtual bool resolveModuleLocation(VectorOfStrings* result, const XMLCh* nsUri, const StaticContext* context);
  virtual bool resolveDocument(Sequence &result, const XMLCh* uri, DynamicContext* context, const QueryPathNode *projection);
  virtual bool resolveCollection(Sequence &result, const XMLCh* uri, DynamicContext* context, const QueryPathNode *projection);
  virtual bool resolveDefaultCollection(Sequence &result, DynamicContext* context, const QueryPathNode *projection);
  virtual bool putDocument(const Node::Ptr &document, const XMLCh *uri, DynamicContext *context) { return true; }

  virtual void detectDefaultConf();
private:
  XQillaConfiguration *m_conf;
  FastXDMConfiguration *m_fastConf;
  XercesConfiguration *m_xercesConf;
  bool m_userSetParserLang;
  XQilla::Language m_lang;
  string m_szSingleTest;
  string m_szFullTestName;
  const TestCase* m_pCurTestCase;

  // id -> filename
  map<string, string> m_inputFiles;
  // schemaURL -> filename
  map<string, string> m_schemaFiles;
  // id -> filename
  map<string, string> m_moduleFiles;
  // id -> list of inputFiles ID
  map<string, list<string> > m_collections;

  AutoDelete<XQQuery> m_docQuery;
  map<string, Sequence> m_inputDocs;
};

void usage(const char *progname)
{
  const char *name = progname;
  while(*progname != 0) {
    if(*progname == '/' || *progname == '\\') {
      ++progname;
      name = progname;
    } else {
      ++progname;
    }
  }

  cout << "Usage: " << name << " [options] <location of the XQTS/XQUTS/XQFTTS suite> (<test group or case name>)?" << endl << endl;
  cout << "-e <file>      : Use the given file as a known error file" << endl;
  cout << "-E <file>      : Output an error file" << endl;
  cout << "-h             : Show this display" << endl;
  cout << "-r             : Output results as XML" << endl;
  cout << "-u             : Parse XQuery Update (also uses Xerces-C data model)" << endl;
  cout << "-s             : Parse XSLT 2.0" << endl;
  cout << "-f             : Parse XQuery Full Text 1.0" << endl;
  cout << "-3             : Parse XQuery 3.0" << endl;
  cout << "-x             : Use the Xerces-C data model (default is FastXDM)" << endl;
}

int main(int argc, char *argv[])
{
  string testSuitePath;
  string singleTest;
  string errorFile;
  string outputErrorFile;

  bool xmlResults = false;
  bool userSetParserLang = false;
  XQilla::Language lang = XQilla::XQUERY;

  XercesConfiguration xercesConf;
  FastXDMConfiguration fastConf;
  XQillaConfiguration *conf = NULL;

  for(int i = 1; i < argc; ++i) {
    if(*argv[i] == '-' && argv[i][2] == '\0' ){

      switch(argv[i][1]) {
      case 'h': {
        usage(argv[0]);
        return 0;
      }
      case 'e': {
        i++;
        if(i == argc) {
          cout << "Missing argument to option 'e'" << endl;
          return 1;
        }
        errorFile = argv[i];
        break;
      }
      case 'E': {
        i++;
        if(i == argc) {
          cout << "Missing argument to option 'E'" << endl;
          return 1;
        }
        outputErrorFile = argv[i];
        break;
      }
      case 'f': {
        lang = (XQilla::Language)(lang | XQilla::FULLTEXT);
        userSetParserLang = true;
        break;
      }
      case 'r': {
        xmlResults = true;
        break;
      }
      case 'u': {
        lang = (XQilla::Language)(lang | XQilla::UPDATE);
        userSetParserLang = true;
        conf = &xercesConf;
        break;
      }
      case '3': {
        lang = (XQilla::Language)(lang | XQilla::VERSION3);
        userSetParserLang = true;
        break;
      }
      case 's': {
        lang = (XQilla::Language)(lang | XQilla::XSLT2);
        userSetParserLang = true;
        break;
      }
      case 'x': {
        conf = &xercesConf;
        break;
      }
      default: {
        cout << "Unknown option: " << argv[i] << endl;
        usage(argv[0]);
        return 1;
      }
      }
    }
    else if(testSuitePath == "") {
      testSuitePath = argv[i];
    }
    else if(singleTest == "") {
      singleTest = argv[i];
    }
    else {
      usage(argv[0]);
      return 1;
    }
  }

  if(testSuitePath == "") {
    cout << "Test suite path not specified!" << endl;
    usage(argv[0]);
    return 1;
  }

  XQillaPlatformUtils::enableExtendedPrecision(false);
  XQilla xqilla;

  Janitor<TestSuiteResultListener> results(0);
  if(xmlResults) {
    results.reset(new XMLReportResultListener());
    XMLReportResultListener *xmlreport = (XMLReportResultListener*)results.get();
    xmlreport->setImplementation("XQilla", "2.0");
    xmlreport->setOrganization("XQilla", "http://xqilla.sourceforge.net");

    if(lang != XQilla::XQUERY_UPDATE) {
      xmlreport->addImplementationDefinedItem("expressionUnicode", "UTF-16");
      xmlreport->addImplementationDefinedItem("implicitTimezone", "Defined by the system clock");
      xmlreport->addImplementationDefinedItem("XMLVersion", "1.1");
      xmlreport->addImplementationDefinedItem("axes", "Full axis support");
      xmlreport->addImplementationDefinedItem("defaultOrderEmpty", "empty least");
      xmlreport->addImplementationDefinedItem("normalizationForms", "NFC, NFD, NFKC, NFKD");
      xmlreport->addImplementationDefinedItem("docProcessing", "schema validation");
    }

    xmlreport->addFeature("Minimal Conformance", true);
    if(lang != XQilla::XQUERY_UPDATE) {
      xmlreport->addFeature("Schema Import", true);
      xmlreport->addFeature("Schema Validation", true);
      xmlreport->addFeature("Static Typing", false);
      xmlreport->addFeature("Static Typing Extensions", false);
      xmlreport->addFeature("Full Axis", true);
      xmlreport->addFeature("Module", true);
      xmlreport->addFeature("Serialization", false);
      xmlreport->addFeature("Trivial XML Embedding", false);
    }

    xmlreport->setSubmittor("John Snelson", "john.snelson@oracle.com");
  } 
  else {
    results.reset(new ConsoleResultListener());
  }

  KnownErrorChecker knownErrors(results.get());
  if(errorFile != "" && !knownErrors.loadErrors(errorFile)) {
    return 1;
  }
  
  XQillaTestSuiteRunner runner(singleTest, &knownErrors, conf, lang, &fastConf, &xercesConf, userSetParserLang);
  TestSuiteParser parser(testSuitePath, &runner);

  parser.run();

  bool passed = true;
  if(xmlResults) {
    ((XMLReportResultListener*)results.get())->printReport();
  }
  else {
    passed = ((ConsoleResultListener*)results.get())->printReport();
  }

  if(errorFile != "") {
    passed = knownErrors.printReport();
  }

  if(outputErrorFile != "" && !knownErrors.saveErrors(outputErrorFile)) {
    cout << "Unable to open error file: " << outputErrorFile << endl;
    return 1;
  }

  return passed ? 0 : 1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

XQillaTestSuiteRunner::XQillaTestSuiteRunner(const string &singleTest, TestSuiteResultListener *results, XQillaConfiguration *conf, XQilla::Language lang, FastXDMConfiguration *fastConf, XercesConfiguration *xercesConf, bool userSetParserLang)
  : TestSuiteRunner(results),
    m_conf(conf),
    m_fastConf(fastConf),
    m_xercesConf(xercesConf),
    m_userSetParserLang(userSetParserLang),
    m_lang(lang),
    m_szSingleTest(singleTest),
    m_pCurTestCase(NULL),
    m_docQuery(0)
{
}

XQillaTestSuiteRunner::~XQillaTestSuiteRunner()
{
}

void XQillaTestSuiteRunner::startTestGroup(const string &name)
{
  if(m_szFullTestName != "")
    m_szFullTestName += ":";
  m_szFullTestName += name;

  m_results->startTestGroup(name);
}

void XQillaTestSuiteRunner::endTestGroup()
{
  string::size_type nColonPos = m_szFullTestName.find_last_of(":");
  if(nColonPos != string::npos)
    m_szFullTestName = string(m_szFullTestName.c_str(), nColonPos);
  else
    m_szFullTestName = "";

  m_results->endTestGroup();
}

void XQillaTestSuiteRunner::addSource(const string &id, const string &filename, const string &schema)
{
  m_inputFiles[id] = filename;
}

void XQillaTestSuiteRunner::addSchema(const string &id, const string &filename, const string &uri)
{
  m_schemaFiles[uri] = filename;
}

void XQillaTestSuiteRunner::addModule(const string &id, const string &filename)
{
  m_moduleFiles[id] = filename;
}

void XQillaTestSuiteRunner::addCollectionDoc(const string &id, const string &filename)
{
  m_collections[id].push_back(filename);
}

void XQillaTestSuiteRunner::runTestCase(const TestCase &testCase)
{
  if(m_szSingleTest != "" &&
     testCase.name.find(m_szSingleTest) == string::npos &&
     m_szFullTestName.find(m_szSingleTest) == string::npos) {
    m_results->reportSkip(testCase, "Not run");
    return;
  }

  if(m_results->isSkippedTest(testCase.name)) {
    m_results->reportSkip(testCase, "Skip");
    return;
  }

  if(m_szFullTestName.substr(0,21)=="Optional:StaticTyping" && !testCase.updateTest) {
    m_results->reportSkip(testCase, "Static typing not supported");
    return;
  }
  if(m_szFullTestName.substr(0,26)=="Optional:TrivialEmbedding") {
    m_results->reportSkip(testCase, "TrivialEmbedding not supported");
    return;
  }

  XQilla xqilla;

  m_pCurTestCase=&testCase;

  DynamicContext *context = xqilla.createContext(m_lang, m_conf);
  AutoDelete<DynamicContext> contextGuard(context);

  try {
    context->setImplicitTimezone(context->getItemFactory()->
                                 createDayTimeDuration(X("PT0S"), context));
    context->setXMLEntityResolver(this);
    context->setModuleResolver(this);
    context->registerURIResolver(this, /*adopt*/false);
    if(testCase.xsltTest || testCase.updateTest)
      context->setBaseURI(X(testCase.queryURL.c_str()));

    if(testCase.updateTest) {
      if(testCase.stateTime == 0) {
        m_inputDocs.clear();
        m_docQuery.set(0);
      }
      else if(m_docQuery.get() == 0) {
        m_results->reportSkip(testCase, "State 0 failed");
        return;
      }
    }

    XQQuery *parsedQuery = xqilla.parseFromURI(X(testCase.queryURL.c_str()), contextGuard.adopt());
    AutoDelete<XQQuery> parsedQueryGuard(parsedQuery);

    if(testCase.updateTest && testCase.stateTime == 0) {
      m_docQuery.set(parsedQuery);
      parsedQueryGuard.adopt();
    }

    map<string, string>::const_iterator v;
    for(v=testCase.extraVars.begin();v!=testCase.extraVars.end();v++) {
      AutoDelete<XQQuery> pInnerQuery(xqilla.parseFromURI(X(v->second.c_str())));
      AutoDelete<DynamicContext> ictxt(context->createModuleDynamicContext(pInnerQuery->getStaticContext()));
      Sequence doc=pInnerQuery->execute(ictxt)->toSequence(ictxt);
      context->setExternalVariable(X(v->first.c_str()), doc);
    }
    for(v=testCase.inputVars.begin();v!=testCase.inputVars.end();v++) {
      string filename = v->second;
      Sequence doc;

      if(testCase.updateTest) {
        if(m_inputDocs.find(v->first) != m_inputDocs.end()) {
          doc = m_inputDocs[v->first];
        }
        else {
          doc = const_cast<DynamicContext*>(m_docQuery->getStaticContext())->
            resolveDocument(X(filename.c_str()), 0);
          m_inputDocs[v->first] = doc;
        }
      }
      else {
        doc = context->resolveDocument(X(filename.c_str()), 0);
      }

      context->setExternalVariable(X(v->first.c_str()), doc);
    }
    for(v=testCase.inputURIVars.begin();v!=testCase.inputURIVars.end();v++) {
      Item::Ptr uri = context->getItemFactory()->createString(X(v->second.c_str()),context);
      context->setExternalVariable(X(v->first.c_str()), uri);
    }
    for(v=testCase.inputParams.begin();v!=testCase.inputParams.end();v++) {
      Item::Ptr value = context->getItemFactory()->createUntypedAtomic(X(v->second.c_str()),context);
      context->setExternalVariable(X(v->first.c_str()), value);
    }
    if(testCase.templateName != "") {
      Item::Ptr value = context->getItemFactory()->createUntypedAtomic(X(testCase.templateName.c_str()),context);
      context->setExternalVariable(X("http://xqilla.sourceforge.net/Functions"), X("name"), value);
    }
    if(!testCase.contextItem.empty())
    {
      Sequence doc=context->resolveDocument(X(testCase.contextItem.c_str()), 0);
      context->setContextItem(doc.first());
    }
    context->setContextPosition(1);
    context->setContextSize(1);
    time_t curTime;
    context->setCurrentTime(time(&curTime));

    // Emulate the XQuery serialization spec
    MemBufFormatTarget target;
    EventSerializer writer("UTF-8", "1.1", &target, context->getMemoryManager());
    NSFixupFilter nsfilter(&writer, context->getMemoryManager());
    ContentSequenceFilter csfilter(&nsfilter);

    parsedQuery->execute(&csfilter, context);
    testResults(testCase, (char*)target.getRawBuffer());
  }
  catch(XQException& e) {
    ostringstream oss;
//     if(e.getXQueryLine() == 0) {
//       oss << "No line number:" << std::endl << UTF8(e.getError()) << std::endl;
//       oss << "at " << UTF8(e.getXQueryFile()) << ":" << e.getXQueryLine() << ":" << e.getXQueryColumn() << std::endl;
//       oss << "at " << e.getCppFile() << ":" << e.getCppLine() << std::endl;
//       m_results->reportFailUnexpectedError(testCase, oss.str(), "XXX");
//     }
//     else if(e.getXQueryColumn() == 0) {
//       oss << "No column number:" << std::endl << UTF8(e.getError()) << std::endl;
//       oss << "at " << UTF8(e.getXQueryFile()) << ":" << e.getXQueryLine() << ":" << e.getXQueryColumn() << std::endl;
//       oss << "at " << e.getCppFile() << ":" << e.getCppLine() << std::endl;
//       m_results->reportFailUnexpectedError(testCase, oss.str(), "XXX");
//     }
//     else if(e.getXQueryFile() == 0) {
//       oss << "No file name:" << std::endl << UTF8(e.getError()) << std::endl;
//       oss << "at " << UTF8(e.getXQueryFile()) << ":" << e.getXQueryLine() << ":" << e.getXQueryColumn() << std::endl;
//       oss << "at " << e.getCppFile() << ":" << e.getCppLine() << std::endl;
//       m_results->reportFailUnexpectedError(testCase, oss.str(), "XXX");
//     }
//     else {
      oss << UTF8(e.getError()) << std::endl;
      oss << "at " << UTF8(e.getXQueryFile()) << ":" << e.getXQueryLine() << ":" << e.getXQueryColumn() << std::endl;
      oss << "at " << e.getCppFile() << ":" << e.getCppLine() << std::endl;
      testErrors(testCase, oss.str());
//     }
  }
  catch(DOMException &de) {
    testErrors(testCase, string("DOMException: ") + UTF8(de.getMessage()));
  }
  catch(...) {
    testErrors(testCase, "[Unknown exception]");
  }
  m_pCurTestCase=NULL;
}

InputSource* XQillaTestSuiteRunner::resolveEntity(XMLResourceIdentifier* resourceIdentifier)
{
    const XMLCh* systemId=resourceIdentifier->getSystemId();
    if((systemId==NULL || *systemId==0) && 
       resourceIdentifier->getResourceIdentifierType()==XMLResourceIdentifier::SchemaGrammar) {
	    
      map<string, string>::const_iterator i =
        m_schemaFiles.find(UTF8(resourceIdentifier->getNameSpace()));
      if(i != m_schemaFiles.end()) {
        return new URLInputSource(X(i->second.c_str()));
      }
    }
    else if(resourceIdentifier->getResourceIdentifierType()==XMLResourceIdentifier::UnKnown) {
      list<std::pair<string, string> >::const_iterator i;
      for(i=m_pCurTestCase->moduleFiles.begin(); i!=m_pCurTestCase->moduleFiles.end(); i++)
      {
        if(i->first == UTF8(resourceIdentifier->getNameSpace()) && 
           i->second == UTF8(resourceIdentifier->getSystemId()))
        {
          map<string, string>::const_iterator i2 = m_moduleFiles.find(i->second);
          if(i2 != m_moduleFiles.end()) {
            string file=i2->second+".xq";
            return new URLInputSource(X(file.c_str()));
          }
        }
      }
    }

    // Don't hit the W3C for the XHTML DTD!
    if(string("http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd") ==
       UTF8(resourceIdentifier->getSystemId())) {
      return new MemBufInputSource((XMLByte*)"", 0, "", false);
    }

    return NULL;
}

bool XQillaTestSuiteRunner::resolveModuleLocation(VectorOfStrings* result, const XMLCh* nsUri, const StaticContext* context)
{
  bool bFound=false;
  list<std::pair<string, string> >::const_iterator i;
  for(i=m_pCurTestCase->moduleFiles.begin(); i!=m_pCurTestCase->moduleFiles.end(); i++)
  {
    if(i->first == UTF8(nsUri))
    {
      result->push_back(context->getMemoryManager()->getPooledString(i->second.c_str()));
      bFound=true;
    }
  }
  return bFound;
}

bool XQillaTestSuiteRunner::resolveDocument(Sequence &result, const XMLCh* uri, DynamicContext* context, const QueryPathNode *projection)
{
  std::map<std::string, std::string>::iterator it=m_inputFiles.find(UTF8(uri));
  if(it!=m_inputFiles.end())
  {
    result=context->resolveDocument(X(it->second.c_str()), 0, projection);
    return true;
  }
  return false;
}

bool XQillaTestSuiteRunner::resolveCollection(Sequence &result, const XMLCh* uri, DynamicContext* context, const QueryPathNode *projection)
{
  std::map<std::string, std::list<std::string> >::iterator it=m_collections.find(UTF8(uri));
  if(it!=m_collections.end())
  {
    for(std::list<std::string>::iterator s=it->second.begin();s!=it->second.end();s++)
    {
      result.joinSequence(context->resolveDocument(X(s->c_str()), 0, projection));
    }
    return true;
  }
  return false;
}

bool XQillaTestSuiteRunner::resolveDefaultCollection(Sequence &result, DynamicContext* context, const QueryPathNode *projection)
{
  if(!m_pCurTestCase->defaultCollection.empty())
    return resolveCollection(result, X(m_pCurTestCase->defaultCollection.c_str()), context, projection);
  return false;
}

void XQillaTestSuiteRunner::detectParserLang(const string &testSuiteNS)
{

  // if the user specified the parser language, don't do auto-detection 
  if(m_userSetParserLang){
    detectDefaultConf();
    m_lang = (XQilla::Language)(m_lang | XQilla::EXTENSIONS);
    return; 
  }

  if(testSuiteNS == "http://www.w3.org/2005/02/query-test-XQTSCatalog")
     m_lang = XQilla::XQUERY;
  else if(testSuiteNS == "http://www.w3.org/2005/02/query-test-update")
     m_lang = XQilla::XQUERY_UPDATE;
  else if(testSuiteNS == "http://www.w3.org/2005/02/query-test-full-text")
     m_lang = XQilla::XQUERY_FULLTEXT;
  else if(testSuiteNS == "http://www.w3.org/2005/05/xslt20-test-catalog")
     m_lang = XQilla::XSLT2;

  detectDefaultConf();

  m_lang = (XQilla::Language)(m_lang | XQilla::EXTENSIONS);
}

void XQillaTestSuiteRunner::detectDefaultConf()
{
  // if the user has set the m_conf, do nothing
  if(m_conf != NULL)
    return;

  if(m_lang & XQilla::UPDATE){
    m_conf = m_xercesConf;
  } 
  else {
    m_conf = m_fastConf;
  }
}

