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

#ifndef _TESTSUITEPARSERHANDLER_HPP
#define _TESTSUITEPARSERHANDLER_HPP

#include <string>
#include <list>
#include <map>
#include <set>

#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/util/XMLURL.hpp>

#include <xqilla/framework/XQillaExport.hpp>

class TestSuiteRunner;

class XQILLA_API TestCase
{
public:
  std::string name;
  std::string description;
  bool updateTest;
  bool xsltTest;
  int stateTime;
  std::string queryURL;
  std::string query;
  std::string contextItem;
  std::string defaultCollection;
  std::string templateName;
  std::map<std::string, std::string> inputURIVars;
  std::map<std::string, std::string> inputVars;
  std::map<std::string, std::string> extraVars;
  std::map<std::string, std::string> inputParams;
  std::map<std::string, std::string> outputFiles;
  std::list<std::pair<std::string, std::string> > moduleFiles;
  std::list<std::string> expectedErrors;
};

class XQILLA_API TestSuiteParser : private XERCES_CPP_NAMESPACE_QUALIFIER DefaultHandler
{
public:
  TestSuiteParser(const std::string &pathToTestSuite, TestSuiteRunner *runner);

  void run();

  void handleUnknownElement(const std::string &elementName);

private:
  virtual void startElement(const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname,
                            const XERCES_CPP_NAMESPACE_QUALIFIER Attributes&  attributes);
  virtual void endElement(const XMLCh* const uri, const XMLCh* const localname,	const XMLCh* const qname);
#if _XERCES_VERSION >= 30000
  virtual void characters(const XMLCh* const chars, const XMLSize_t length);
#else
  virtual void characters(const XMLCh* const chars, const unsigned int length);
#endif

  virtual void error(const XERCES_CPP_NAMESPACE_QUALIFIER SAXParseException& exc);
  virtual void fatalError(const XERCES_CPP_NAMESPACE_QUALIFIER SAXParseException& exc);

private:
  TestSuiteRunner *runner_;
  bool xslt_;
  std::string xsltGroupName_;

  XERCES_CPP_NAMESPACE_QUALIFIER XMLURL urlXQTSCatalog_, urlXQTSQueriesDirectory_, urlXQTSResultsDirectory_, 
    urlBasePath_, urlBasePathReferenceFiles_, urlQuery_;

  std::set<std::string> unknownElements_;

  bool readingChars_;
  std::string chars_;

  std::string variableBoundToInput_, compareMethod_, namespace_, collectionID_;

  TestCase testCase_;
};

#endif
