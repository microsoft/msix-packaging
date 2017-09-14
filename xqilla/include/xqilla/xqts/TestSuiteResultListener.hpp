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

#ifndef _TESTSUITERESULTLISTENER_HPP
#define _TESTSUITERESULTLISTENER_HPP

#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <list>

#include <xqilla/framework/XQillaExport.hpp>

class TestCase;

class XQILLA_API TestSuiteResultListener
{
public:
  virtual ~TestSuiteResultListener() {}

  virtual void reportVersion(const std::string &version, bool update) {}

  virtual void startTestGroup(const std::string &name) = 0;
  virtual void endTestGroup() = 0;

  virtual void reportPass(const TestCase &testCase, const std::string &comment) = 0;
  virtual void reportInspect(const TestCase &testCase, const std::string &actualResult,
                          const std::list<std::string> &expectedResult, const std::string &comment) = 0;
  virtual void reportSkip(const TestCase &testCase, const std::string &comment) = 0;
  virtual void reportFail(const TestCase &testCase, const std::string &actualResult,
                          const std::list<std::string> &expectedResult, const std::string &comment) = 0;
  virtual void reportFailNoError(const TestCase &testCase, const std::string &actualResult,
                                 const std::string &comment) = 0;
  virtual void reportFailUnexpectedError(const TestCase &testCase, const std::string &unexpectedError,
                                         const std::string &comment) = 0;
  
  virtual bool isSkippedTest(const std::string &testName) const { return false;}

protected:
  TestSuiteResultListener() {}
};

class XQILLA_API KnownErrorChecker : public TestSuiteResultListener
{
public:
  KnownErrorChecker(TestSuiteResultListener *results);

  virtual void reportVersion(const std::string &version, bool update);

  virtual void startTestGroup(const std::string &name);
  virtual void endTestGroup();

  virtual void reportPass(const TestCase &testCase, const std::string &comment);
  virtual void reportInspect(const TestCase &testCase, const std::string &actualResult,
                             const std::list<std::string> &expectedResult, const std::string &comment);
  virtual void reportSkip(const TestCase &testCase, const std::string &comment);
  virtual void reportFail(const TestCase &testCase, const std::string &actualResult,
                          const std::list<std::string> &expectedResult, const std::string &comment);
  virtual void reportFailNoError(const TestCase &testCase, const std::string &actualResult,
                                 const std::string &comment);
  virtual void reportFailUnexpectedError(const TestCase &testCase, const std::string &unexpectedError,
                                         const std::string &comment);

  bool printReport() const;

  bool loadErrors(const std::string &errorFile);
  bool saveErrors(const std::string &errorFile) const;
  bool isSkippedTest(const std::string &testName) const;

  class Error {
  public:
    std::string reason;
    std::string comment;
    std::string action;
  };

private:
  TestSuiteResultListener *results_;

  // testName -> (reason, comment, action)
  std::map<std::string, Error> errors_;
  std::map<std::string, Error> nowPass_;
  std::vector<std::string> nowFail_;
};

class XQILLA_API ConsoleResultListener : public TestSuiteResultListener
{
public:
  ConsoleResultListener();

  virtual void startTestGroup(const std::string &name);
  virtual void endTestGroup();

  virtual void reportPass(const TestCase &testCase, const std::string &comment);
  virtual void reportInspect(const TestCase &testCase, const std::string &actualResult,
                             const std::list<std::string> &expectedResult, const std::string &comment);
  virtual void reportSkip(const TestCase &testCase, const std::string &comment);
  virtual void reportFail(const TestCase &testCase, const std::string &actualResult,
                          const std::list<std::string> &expectedResult, const std::string &comment);
  virtual void reportFailNoError(const TestCase &testCase, const std::string &actualResult,
                                 const std::string &comment);
  virtual void reportFailUnexpectedError(const TestCase &testCase, const std::string &unexpectedError,
                                         const std::string &comment);

  bool printReport() const;

private:
  void testCaseToErrorStream(const TestCase &testCase);

private:
  std::string m_szFullTestName;

  // testname -> "p|s|i|f"
  std::map<std::string, std::string> testsRecorded_;
  unsigned int m_nTotalTests, m_nPassedTests, m_nSkippedTests, m_nInspectTests;

  int testDepth_;
  bool needNewline_;

  std::ostringstream errorStream_;
};

class XQILLA_API XMLReportResultListener : public TestSuiteResultListener
{
public:
  XMLReportResultListener();

  virtual void reportVersion(const std::string &version, bool update);

  virtual void startTestGroup(const std::string &name);
  virtual void endTestGroup();

  virtual void reportPass(const TestCase &testCase, const std::string &comment);
  virtual void reportInspect(const TestCase &testCase, const std::string &actualResult,
                             const std::list<std::string> &expectedResult, const std::string &comment);
  virtual void reportSkip(const TestCase &testCase, const std::string &comment);
  virtual void reportFail(const TestCase &testCase, const std::string &actualResult,
                          const std::list<std::string> &expectedResult, const std::string &comment);
  virtual void reportFailNoError(const TestCase &testCase, const std::string &actualResult,
                                 const std::string &comment);
  virtual void reportFailUnexpectedError(const TestCase &testCase, const std::string &unexpectedError,
                                         const std::string &comment);

  void setImplementation(const std::string &name, const std::string &version = "", const std::string &description = "");
  void setOrganization(const std::string &name, const std::string &website = "");
  void setSubmittor(const std::string &name, const std::string &email = "");
  void setTestRun(const std::string &transformation, const std::string &comments = "");
  void setAnonymous(bool anon);
  void addImplementationDefinedItem(const std::string &name, const std::string &value);
  void addFeature(const std::string &name, bool supported);
  void addContextProperty(const std::string &name, const std::string &contextType, const std::string &value);

  void printReport() const;

private:
  bool update_;

  std::string version_;
  std::string implName_, implVersion_, implDescription_;
  std::string orgName_, orgWebsite_;
  std::string submittorName_, submittorEmail_;
  std::string testTransformation_, testComments_;
  bool anonymous_;

  std::map<std::string, std::string> implDefinedItems_;
  std::map<std::string, bool> features_;
  std::map<std::string, std::pair<std::string, std::string> > contextProperties_;

  std::string previousTestName_;
  std::ostringstream tmpStream_;

  std::ostringstream outputStream_;
};

#endif
