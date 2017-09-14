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

#ifdef _MSC_VER
#pragma warning(disable: 4786)
#endif

#include <xqilla/xqts/TestSuiteParser.hpp>
#include <xqilla/xqts/TestSuiteRunner.hpp>
#include <xqilla/xqts/TestSuiteResultListener.hpp>

#include <iostream>
#include <stdlib.h>

#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/sax2/Attributes.hpp>
#include <xercesc/framework/URLInputSource.hpp>
#include <xercesc/util/BinFileInputStream.hpp>

#include <xqilla/utils/XStr.hpp>
#include <xqilla/utils/UTF8Str.hpp>

#if defined(XERCES_HAS_CPP_NAMESPACE)
XERCES_CPP_NAMESPACE_USE
#endif
using namespace std;

TestSuiteParser::TestSuiteParser(const string &pathToTestSuite, TestSuiteRunner *runner)
  : runner_(runner),
    xslt_(false)
{
  string szXQTSLocation = string("file:///") + pathToTestSuite;

  for(string::iterator i = szXQTSLocation.begin(); i != szXQTSLocation.end(); ++i) {
    if(*i == ' ') {
      i = szXQTSLocation.erase(i);
      szXQTSLocation = szXQTSLocation.insert(i - szXQTSLocation.begin(), "%20");
      i = szXQTSLocation.begin();
    }
    else if(*i == '\\') *i = '/';
  }

  urlXQTSCatalog_.setURL(X(szXQTSLocation.c_str()));
  readingChars_ = false;
}

void TestSuiteParser::handleUnknownElement(const string &elementName)
{
  unknownElements_.insert(elementName);
}

void TestSuiteParser::run()
{
  xslt_ = false;
  SAX2XMLReader* parser = XMLReaderFactory::createXMLReader();
  try {
    parser->setContentHandler(this);
    parser->setErrorHandler(this);
    URLInputSource is(urlXQTSCatalog_);
    parser->parse(is);
  }
  catch(const XMLException& e) {
    cerr << "Error while reading test catalog: " << UTF8(e.getMessage()) << endl;
  }
  delete parser;
}

void TestSuiteParser::error(const SAXParseException& exc)
{
  cerr << "ERROR parsing catalog: " << UTF8(exc.getMessage()) << endl;
}

void TestSuiteParser::fatalError(const SAXParseException& exc)
{
  cerr << "FATAL ERROR parsing catalog: " << UTF8(exc.getMessage()) << endl;
}

static XMLCh g_szName[]     = { chLatin_n, chLatin_a, chLatin_m, chLatin_e, chNull };
static XMLCh g_szPath[]     = { chLatin_F, chLatin_i, chLatin_l, chLatin_e, chLatin_P, chLatin_a, chLatin_t, chLatin_h, chNull };
static XMLCh g_szVar[]      = { chLatin_v, chLatin_a, chLatin_r, chLatin_i, chLatin_a, chLatin_b, chLatin_l, chLatin_e, chNull };
static XMLCh g_szDotXQ[]    = { chPeriod, chLatin_x, chLatin_q, chNull };
static XMLCh g_szCompare[]  = { chLatin_c, chLatin_o, chLatin_m, chLatin_p, chLatin_a, chLatin_r, chLatin_e, chNull };
static XMLCh g_szTime[]     = { chLatin_t, chLatin_i, chLatin_m, chLatin_e, chNull };
static XMLCh g_szRole[]     = { chLatin_r, chLatin_o, chLatin_l, chLatin_e, chNull };
static XMLCh g_szFile[]     = { chLatin_f, chLatin_i, chLatin_l, chLatin_e, chNull };
static XMLCh g_szType[]     = { chLatin_t, chLatin_y, chLatin_p, chLatin_e, chNull };

void TestSuiteParser::startElement(const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname, const Attributes&  attributes)
{
    string szName=UTF8(localname);
    string szURI=UTF8(uri);
//     if(szURI=="http://www.w3.org/2005/02/query-test-XQTSCatalog")
//     {
      if(szName=="test-suite") {
        runner_->getResultListener()->reportVersion(UTF8(attributes.getValue(X("version"))),
                                                    szURI == "http://www.w3.org/2005/02/query-test-update");

        urlXQTSQueriesDirectory_.setURL(urlXQTSCatalog_, attributes.getValue(X("XQueryQueryOffsetPath")));
        urlXQTSResultsDirectory_.setURL(urlXQTSCatalog_, attributes.getValue(X("ResultOffsetPath")));

        // detect runner's parser language and conf
        runner_->detectParserLang(szURI);
      }
      else if(szName=="testcases") {
        // XSLT test suite
        xslt_ = true;
        runner_->detectParserLang(szURI); 

        runner_->getResultListener()->reportVersion(UTF8(attributes.getValue(X("testSuiteVersion"))), false);

        urlXQTSQueriesDirectory_.setURL(urlXQTSCatalog_, attributes.getValue(X("InputOffsetPath")));
        urlXQTSResultsDirectory_.setURL(urlXQTSCatalog_, attributes.getValue(X("ResultOffsetPath")));
      }
      else if(szName=="test-group") {
        runner_->startTestGroup(UTF8(attributes.getValue(g_szName)));
      }
      else if(szName=="test-case") {
        testCase_.name = UTF8(attributes.getValue(g_szName));
        testCase_.description = "";
        testCase_.updateTest = false;
        testCase_.xsltTest = false;
        testCase_.stateTime = -1;
        testCase_.queryURL = "";
        testCase_.query = "";
        testCase_.contextItem = "";
        testCase_.defaultCollection = "";
        testCase_.templateName = "";
        testCase_.inputURIVars.clear();
        testCase_.inputParams.clear();
        testCase_.inputVars.clear();
        testCase_.extraVars.clear();
        testCase_.expectedErrors.clear();
        testCase_.outputFiles.clear();
        testCase_.moduleFiles.clear();

        variableBoundToInput_=compareMethod_="";
        urlQuery_=XMLURL();
        XMLBuffer buff;
        buff.set(attributes.getValue(g_szPath));
        buff.append('/');
        urlBasePath_.setURL(urlXQTSQueriesDirectory_, buff.getRawBuffer());
        urlBasePathReferenceFiles_.setURL(urlXQTSResultsDirectory_, buff.getRawBuffer());
      }
      else if(szName=="testcase") {
        // XSLT test case
        testCase_.name = "";
        testCase_.description = "";
        testCase_.updateTest = false;
        testCase_.xsltTest = true;
        testCase_.stateTime = -1;
        testCase_.queryURL = "";
        testCase_.query = "";
        testCase_.contextItem = "";
        testCase_.defaultCollection = "";
        testCase_.templateName = "";
        testCase_.inputURIVars.clear();
        testCase_.inputParams.clear();
        testCase_.inputVars.clear();
        testCase_.extraVars.clear();
        testCase_.expectedErrors.clear();
        testCase_.outputFiles.clear();
        testCase_.moduleFiles.clear();

        variableBoundToInput_=compareMethod_="";
        urlQuery_=XMLURL();
        XMLBuffer buff;
        buff.set(attributes.getValue(g_szPath));
        buff.append('/');
        urlBasePath_.setURL(urlXQTSQueriesDirectory_, buff.getRawBuffer());
        urlBasePathReferenceFiles_.setURL(urlXQTSResultsDirectory_, buff.getRawBuffer());
      }
      else if(szName=="state")
      {
        testCase_.updateTest = true;
        testCase_.xsltTest = false;
        testCase_.stateTime = atoi(UTF8(attributes.getValue(g_szTime)));
        testCase_.queryURL = "";
        testCase_.query = "";
        testCase_.contextItem = "";
        testCase_.defaultCollection = "";
        testCase_.templateName = "";
        testCase_.inputURIVars.clear();
        testCase_.inputParams.clear();
        testCase_.inputVars.clear();
        testCase_.extraVars.clear();
        testCase_.expectedErrors.clear();
        testCase_.outputFiles.clear();
        testCase_.moduleFiles.clear();

        variableBoundToInput_=compareMethod_="";
        urlQuery_=XMLURL();
      }
      else if(szName=="query")
      {
        XMLBuffer buff;
        const XMLCh* name=attributes.getValue(g_szName);
        buff.set(name);
        if(!XMLString::endsWith(name,g_szDotXQ))
            buff.append(g_szDotXQ);
        urlQuery_.setURL(urlBasePath_,buff.getRawBuffer());
        testCase_.queryURL = UTF8(urlQuery_.getURLText());

        try {
          testCase_.query = "#Not found";
          Janitor<BinFileInputStream> stream((BinFileInputStream*)URLInputSource(urlQuery_).makeStream());
          if(stream.get()) {
            unsigned int dwSize = (unsigned int) stream->getSize();
            testCase_.query.resize(dwSize);
            stream->readBytes((XMLByte*)testCase_.query.c_str(), dwSize);
          }
        } catch(...) {}
      }
      else if(szName=="stylesheet")
      {
        // XSLT test suite
        testCase_.xsltTest = true;

        const XMLCh *role = attributes.getValue(g_szRole);
        if(XMLString::equals(role, X("principal"))) {
          const XMLCh *file = attributes.getValue(g_szFile);
          urlQuery_.setURL(urlXQTSQueriesDirectory_, file);
          testCase_.queryURL = UTF8(urlQuery_.getURLText());

          try {
            testCase_.query = "#Not found";
            Janitor<BinFileInputStream> stream((BinFileInputStream*)URLInputSource(urlQuery_).makeStream());
            if(stream.get()) {
              unsigned int dwSize = (unsigned int) stream->getSize();
              testCase_.query.resize(dwSize);
              stream->readBytes((XMLByte*)testCase_.query.c_str(), dwSize);
            }
          } catch(...) {}
        }
      }
      else if(szName=="source-document")
      {
        // XSLT test suite
        testCase_.xsltTest = true;

        const XMLCh *role = attributes.getValue(g_szRole);
        if(XMLString::equals(role, X("principal"))) {
          const XMLCh *file = attributes.getValue(g_szFile);
          urlQuery_.setURL(urlXQTSQueriesDirectory_, file);
          testCase_.contextItem = UTF8(urlQuery_.getURLText());
        }
      }
      else if(szName=="result-document")
      {
        // XSLT test suite
        testCase_.xsltTest = true;

        const XMLCh *role = attributes.getValue(g_szRole);
        if(XMLString::equals(role, X("principal"))) {
          const XMLCh *file = attributes.getValue(g_szFile);
          urlQuery_.setURL(urlXQTSResultsDirectory_, file);

          const XMLCh *type = attributes.getValue(g_szType);
          testCase_.outputFiles[UTF8(urlQuery_.getURLText())]=UTF8(type);
        }
      }
      else if(szName=="entry-named-template")
      {
        // XSLT test suite
        testCase_.templateName = UTF8(attributes.getValue(X("qname")));
      }
      else if(szName=="error")
      {
        // XSLT test suite
        const XMLCh *id = attributes.getValue(X("error-id"));
        testCase_.expectedErrors.push_back(UTF8(id));
      }
      else if(szName=="input-query")
      {
        XMLBuffer buff;
        const XMLCh* name=attributes.getValue(g_szName);
        buff.set(name);
        if(!XMLString::endsWith(name,g_szDotXQ))
            buff.append(g_szDotXQ);
        XMLURL urlQuery(urlBasePath_,buff.getRawBuffer());
        string varQuery = UTF8(urlQuery.getURLText());

        testCase_.extraVars[UTF8(attributes.getValue(g_szVar))]=varQuery;
      }
      else if(szName=="input-file")
      {
        readingChars_=true;
        chars_="";
        variableBoundToInput_=UTF8(attributes.getValue(g_szVar));
      }
      else if(szName=="input-URI")
      {
        readingChars_=true;
        chars_="";
        variableBoundToInput_=UTF8(attributes.getValue(g_szVar));
      }
      else if(szName=="param")
      {
        readingChars_=true;
        chars_="";
        variableBoundToInput_=UTF8(attributes.getValue(X("qname")));
      }
      else if(szName=="contextItem")
      {
        readingChars_=true;
        chars_="";
      }
      else if(szName=="defaultCollection")
      {
        readingChars_=true;
        chars_="";
      }
      else if(szName=="output-file")
      {
        readingChars_=true;
        chars_="";
        compareMethod_=UTF8(attributes.getValue(g_szCompare));
      }
      else if(szName=="expected-error")
      {
        readingChars_=true;
        chars_="";
      }
      else if(szName=="name")
      {
        // XSLT test suite
        readingChars_=true;
        chars_="";
      }
      else if(szName=="description")
      {
        readingChars_=true;
        chars_="";
      }
      else if(szName=="source") {
        XMLURL realFile(urlXQTSCatalog_, attributes.getValue(X("FileName")));
        runner_->addSource(UTF8(attributes.getValue(X("ID"))), UTF8(realFile.getURLText()), UTF8(attributes.getValue(X("schema"))));
      }
      else if(szName=="schema" && !xslt_) {
        XMLURL realFile(urlXQTSCatalog_, attributes.getValue(X("FileName")));
        runner_->addSchema(UTF8(attributes.getValue(X("ID"))), UTF8(realFile.getURLText()), UTF8(attributes.getValue(X("uri"))));
      }
      else if(szName=="module") {
        if(attributes.getValue(X("FileName"))!=NULL)
        {
            XMLURL realFile(urlXQTSCatalog_, attributes.getValue(X("FileName")));
            runner_->addModule(UTF8(attributes.getValue(X("ID"))), UTF8(realFile.getURLText()));
        }
        else
        {
            readingChars_=true;
            chars_="";
            namespace_=UTF8(attributes.getValue(X("namespace")));
        }
      }
      else if(szName=="collection") {
        collectionID_ = UTF8(attributes.getValue(X("ID")));
        runner_->addCollection(collectionID_);
      }
      else if(szName=="input-document") {
        if(!collectionID_.empty()) {
          readingChars_=true;
          chars_="";
        }
      }
      else if(unknownElements_.find(szName) != unknownElements_.end()) {
        map<string, string> attrs;
        for(unsigned int i = 0; i < attributes.getLength(); ++i) {
          attrs[UTF8(attributes.getLocalName(i))] = UTF8(attributes.getValue(i));
        }

        runner_->unknownElement(szName, attrs);
      }
//     }
}

#if _XERCES_VERSION >= 30000
void TestSuiteParser::characters(const XMLCh *const chars, const XMLSize_t length)
#else
void TestSuiteParser::characters(const XMLCh *const chars, const unsigned int length)
#endif
{
    XMLBuffer buff;
    buff.set(chars,length);
    if(readingChars_)
        chars_+=UTF8(buff.getRawBuffer());
}

void TestSuiteParser::endElement(const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname)
{
    string szName=UTF8(localname);
    string szURI=UTF8(uri);
//     if(szURI=="http://www.w3.org/2005/02/query-test-XQTSCatalog")
//     {
      if(szName == "test-group") {
        runner_->endTestGroup();
      }
      else if(szName=="testcases") {
        if(xsltGroupName_ != "") runner_->endTestGroup();
      }
      else if(szName == "test-case") {
        if(!testCase_.updateTest) {
          if(testCase_.query == "#Not found") {
            runner_->getResultListener()->reportFail(testCase_, "", list<string>(), "Bad test! Query not found: " + testCase_.queryURL);
          }
          else {
            runner_->runTestCase(testCase_);
          }
        }
      }
      else if(szName == "testcase") {
        size_t pos = 0;
        while((testCase_.name[pos] >= 'A' && testCase_.name[pos] <= 'Z') ||
              (testCase_.name[pos] >= 'a' && testCase_.name[pos] <= 'z')) ++pos;

        string group = testCase_.name.substr(0, pos);
        if(xsltGroupName_ != group) {
          if(xsltGroupName_ != "") runner_->endTestGroup();
          xsltGroupName_ = group;
          if(xsltGroupName_ != "") runner_->startTestGroup(xsltGroupName_);
        }

        if(testCase_.query == "#Not found") {
          runner_->getResultListener()->reportFail(testCase_, "", list<string>(), "Bad test! Query not found: " + testCase_.queryURL);
        }
        else {
          runner_->runTestCase(testCase_);
        }
      }
      else if(szName == "state") {
        if(testCase_.query == "#Not found") {
          runner_->getResultListener()->reportFail(testCase_, "", list<string>(), "Bad test! Query not found: " + testCase_.queryURL);
        }
        else {
          runner_->runTestCase(testCase_);
        }
      }
      else if(szName == "input-file")
      {
        readingChars_=false;
        testCase_.inputVars[variableBoundToInput_]=chars_;
        variableBoundToInput_="";
      }
      else if(szName=="input-URI")
      {
        readingChars_=false;
        testCase_.inputURIVars[variableBoundToInput_]=chars_;
        variableBoundToInput_="";
      }
      else if(szName=="param")
      {
        readingChars_=false;
        testCase_.inputParams[variableBoundToInput_]=chars_;
        variableBoundToInput_="";
      }
      else if(szName=="contextItem")
      {
        readingChars_=false;
        testCase_.contextItem=chars_;
      }
      else if(szName=="defaultCollection")
      {
        readingChars_=false;
        testCase_.defaultCollection=chars_;
      }
      else if(szName == "output-file")
      {
        readingChars_=false;
        if(chars_ != "") {
          chars_=UTF8(XMLURL(urlBasePathReferenceFiles_, chars_.c_str()).getURLText());
          testCase_.outputFiles[chars_]=compareMethod_;
        }
        compareMethod_="";
      }
      else if(szName == "expected-error")
      {
        readingChars_=false;
        testCase_.expectedErrors.push_back(chars_);
      }
      else if(szName == "name")
      {
        readingChars_=false;
        testCase_.name = chars_;
      }
      else if(szName == "description")
      {
        readingChars_=false;
        testCase_.description = chars_;
      }
      else if(szName == "module")
      {
        readingChars_=false;
        testCase_.moduleFiles.push_back(pair<string, string>(namespace_, chars_));
      }
      else if(szName=="collection") {
        collectionID_ = "";
      }
      else if(szName=="input-document") {
        if(!collectionID_.empty()) {
          readingChars_=false;
          runner_->addCollectionDoc(collectionID_, chars_);
        }
      }
//     }
}
