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

#ifndef _TESTSUITERUNNER_HPP
#define _TESTSUITERUNNER_HPP

#include <string>
#include <list>
#include <map>

#include <xqilla/framework/XQillaExport.hpp>

class TestSuiteResultListener;
class TestCase;

class XQILLA_API TestSuiteRunner
{
public:
  virtual ~TestSuiteRunner() {}

  TestSuiteResultListener *getResultListener() { return m_results; }

  virtual void unknownElement(const std::string &name, std::map<std::string, std::string> &attrs) {}

  virtual void addSource(const std::string &id, const std::string &filename, const std::string &schema) = 0;
  virtual void addSchema(const std::string &id, const std::string &filename, const std::string &uri) = 0;
  virtual void addModule(const std::string &id, const std::string &filename) = 0;
  virtual void addCollection(const std::string &id) {}
  virtual void addCollectionDoc(const std::string &id, const std::string &filename) = 0;

  virtual void startTestGroup(const std::string &name) = 0;
  virtual void endTestGroup() = 0;

  virtual void runTestCase(const TestCase &testCase) = 0;
  virtual void detectParserLang(const std::string &testSuiteNamespace) = 0;

protected:
  TestSuiteRunner(TestSuiteResultListener *results) : m_results(results) {}

  void testResults(const TestCase &testCase, const std::string &results) const;
  void testErrors(const TestCase &testCase, const std::string &actualError) const;

protected:
  TestSuiteResultListener *m_results;
};

#endif
