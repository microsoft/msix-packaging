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

#include "../config/xqilla_config.h"
#include <xqilla/xqts/TestSuiteResultListener.hpp>
#include <xqilla/xqts/TestSuiteParser.hpp>

#include <iostream>
#include <fstream>
#include <time.h>
#ifdef _MSC_VER
#define snprintf _snprintf
#endif

#include <xercesc/parsers/SAXParser.hpp>
#include <xercesc/sax/AttributeList.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/framework/LocalFileInputSource.hpp>

#include <xqilla/utils/XStr.hpp>
#include <xqilla/utils/UTF8Str.hpp>

// Added so xqilla will compile on SunOS 10 using STLPort
#include <stdio.h>

#if defined(XERCES_HAS_CPP_NAMESPACE)
XERCES_CPP_NAMESPACE_USE
#endif
using namespace std;

KnownErrorChecker::KnownErrorChecker(TestSuiteResultListener *results)
  : results_(results)
{
}

void KnownErrorChecker::reportVersion(const std::string &version, bool update)
{
  results_->reportVersion(version, update);
}

void KnownErrorChecker::startTestGroup(const std::string &name)
{
  results_->startTestGroup(name);
}

void KnownErrorChecker::endTestGroup()
{
  results_->endTestGroup();
}

void KnownErrorChecker::reportPass(const TestCase &testCase, const string &comment)
{
  map<string, Error>::iterator i = errors_.find(testCase.name);
  if(i != errors_.end() && i->second.reason != "inspect") {
    nowPass_[i->first] = i->second;
    errors_.erase(i);
  }

  results_->reportPass(testCase, comment);
}

void KnownErrorChecker::reportInspect(const TestCase &testCase, const string &actualResult,
                                      const std::list<std::string> &expectedResult, const string &comment)
{
  string newComment = comment;
  map<string, Error>::iterator i = errors_.find(testCase.name);
  if(i != errors_.end()) {
    if(i->second.comment != "") {
      newComment = i->second.comment;
    }
    if(i->second.action == "skip") {
      results_->reportSkip(testCase, i->second.comment);
      return;
    }
    else if(i->second.action == "pass") {
      results_->reportPass(testCase, i->second.comment);
      return;
    }
  }
  errors_[testCase.name].reason = "inspect";
  errors_[testCase.name].comment = newComment;

  results_->reportInspect(testCase, actualResult, expectedResult, newComment);
}

void KnownErrorChecker::reportSkip(const TestCase &testCase, const std::string &comment)
{
  results_->reportSkip(testCase, comment);
}

void KnownErrorChecker::reportFail(const TestCase &testCase, const string &actualResult,
                                   const std::list<std::string> &expectedResult, const string &comment)
{
  string newComment = comment;
  map<string, Error>::iterator i = errors_.find(testCase.name);
  if(i == errors_.end()) {
    map<string, Error>::iterator j = nowPass_.find(testCase.name);
    if(j != nowPass_.end()) {
      errors_[j->first] = j->second;
      nowPass_.erase(j);
    }
    else
      nowFail_.push_back(testCase.name);
  }
  else {
    if(i->second.comment != "") {
      newComment = i->second.comment;
    }

    if(i->second.action == "skip") {
      results_->reportSkip(testCase, i->second.comment);
      return;
    }
    else if(i->second.action == "pass") {
      results_->reportPass(testCase, i->second.comment);
      return;
    }
  }
  errors_[testCase.name].reason = "result failure";
  errors_[testCase.name].comment = newComment;

  results_->reportFail(testCase, actualResult, expectedResult, newComment);
}

void KnownErrorChecker::reportFailNoError(const TestCase &testCase, const string &actualResult,
                                          const string &comment)
{
  string newComment = comment;
  map<string, Error>::iterator i = errors_.find(testCase.name);
  if(i == errors_.end()) {
    map<string, Error>::iterator j = nowPass_.find(testCase.name);
    if(j != nowPass_.end()) {
      errors_[j->first] = j->second;
      nowPass_.erase(j);
    }
    else
      nowFail_.push_back(testCase.name);
  }
  else {
    if(i->second.comment != "") {
      newComment = i->second.comment;
    }

    if(i->second.action == "skip") {
      results_->reportSkip(testCase, i->second.comment);
      return;
    }
    else if(i->second.action == "pass") {
      results_->reportPass(testCase, i->second.comment);
      return;
    }
  }
  errors_[testCase.name].reason = "no error failure";
  errors_[testCase.name].comment = newComment;

  results_->reportFailNoError(testCase, actualResult, newComment);
}

void KnownErrorChecker::reportFailUnexpectedError(const TestCase &testCase, const string &unexpectedError,
                                                  const string &comment)
{
  string newComment = comment;
  map<string, Error>::iterator i = errors_.find(testCase.name);
  if(i == errors_.end()) {
    map<string, Error>::iterator j = nowPass_.find(testCase.name);
    if(j != nowPass_.end()) {
      errors_[j->first] = j->second;
      nowPass_.erase(j);
    }
    else
      nowFail_.push_back(testCase.name);
  }
  else {
    if(i->second.comment != "") {
      newComment = i->second.comment;
    }

    if(i->second.action == "skip") {
      results_->reportSkip(testCase, i->second.comment);
      return;
    }
    else if(i->second.action == "pass") {
      results_->reportPass(testCase, i->second.comment);
      return;
    }
  }
  errors_[testCase.name].reason = "error failure";
  errors_[testCase.name].comment = newComment;

  results_->reportFailUnexpectedError(testCase, unexpectedError, newComment);
}

bool KnownErrorChecker::printReport() const
{
  if(!nowFail_.empty() || !nowPass_.empty()) {
    cout << "************************************************************************" << endl;
    if(!nowFail_.empty()) {
      cout << (unsigned int)nowFail_.size() << " unexpected failures:" << endl;
      for(vector<string>::const_iterator i = nowFail_.begin(); i != nowFail_.end(); ++i) {
        cout << "\t" << *i << endl;
      }
    }

    if(!nowPass_.empty()) {
      cout << (unsigned int)nowPass_.size() << " unexpected passes:" << endl;
      for(map<string, Error>::const_iterator j = nowPass_.begin(); j != nowPass_.end(); ++j) {
        cout << "\t" << j->first << endl;
      }
    }
  }

  return nowFail_.empty();
}

class ErrorFileHandler : public HandlerBase
{
public:
  ErrorFileHandler(map<string, KnownErrorChecker::Error> &errors)
    : errors_(errors) {}

  virtual void startElement(const XMLCh* const name, AttributeList &attributes)
  {
    string szName = UTF8(name);
    if(szName == "error") {
      KnownErrorChecker::Error &error = errors_[UTF8(attributes.getValue("id"))];
      error.reason = UTF8(attributes.getValue("reason"));
      error.comment = UTF8(attributes.getValue("comment"));
      error.action = UTF8(attributes.getValue("action"));
    }
  }

private:
  map<string, KnownErrorChecker::Error> &errors_;
};


bool KnownErrorChecker::loadErrors(const std::string &errorFile)
{
  try {
    SAXParser parser;
    ErrorFileHandler handler(errors_);
    parser.setDocumentHandler(&handler);
    LocalFileInputSource is(X(errorFile.c_str()));
    parser.parse(is);
  }
  catch(const XMLException& e) {
    cout << "Error while reading error file: " << UTF8(e.getMessage()) << endl;
    return false;
  }

  return true;
}

bool KnownErrorChecker::saveErrors(const std::string &errorFile) const
{
  ofstream file(errorFile.c_str());
  if(!file.is_open()) return false;

  file << "<errors>" << endl;

  for(map<string, Error>::const_iterator i = errors_.begin();
      i != errors_.end(); ++i) {
    file << "  <error id=\"" << i->first
         << "\" reason=\"" << i->second.reason << "\"";

    if(i->second.action != "")
      file << " action=\"" << i->second.action << "\"";

    if(i->second.comment != "")
      file << " comment=\"" << i->second.comment << "\"";

    file << "/>" << endl;
  }

  file << "</errors>" << endl;

  file.close();

  return true;
}

bool KnownErrorChecker::isSkippedTest(const std::string &testName) const
{
  if(errors_.empty())
    return false;

  std::map<std::string, Error>::const_iterator it = errors_.find(testName);
 
  if(it != errors_.end() && (*it).second.action == "skip")
    return true;
  else
    return false;

}
////////////////////////////////////////////////////////////////////////////////////////////////////

ConsoleResultListener::ConsoleResultListener()
  : m_nTotalTests(0),
    m_nPassedTests(0),
    m_nSkippedTests(0),
    m_nInspectTests(0),
    testDepth_(0),
    needNewline_(false)
{
}

void ConsoleResultListener::startTestGroup(const string &name)
{
  if(needNewline_) cout << endl;

  string indent(testDepth_ * 2, ' ');
  cout << indent << name << ": " << flush;
  ++testDepth_;
  needNewline_ = true;

  if(m_szFullTestName != "")
    m_szFullTestName += ":";
  m_szFullTestName += name;
}

void ConsoleResultListener::endTestGroup()
{
  if(needNewline_) cout << endl;
  --testDepth_;
  needNewline_ = false;

  size_t nColonPos = m_szFullTestName.find_last_of(":");
  if(nColonPos != (size_t)-1)
    m_szFullTestName = string(m_szFullTestName.c_str(), nColonPos);
  else
    m_szFullTestName = "";
}

void ConsoleResultListener::reportPass(const TestCase &testCase, const string &comment)
{
  if(testsRecorded_.find(testCase.name) == testsRecorded_.end()) {
    testsRecorded_[testCase.name] = "p";
    ++m_nTotalTests;
    ++m_nPassedTests;
  }

  cout << "." << flush;
}

void ConsoleResultListener::reportInspect(const TestCase &testCase, const string &actualResult,
                                          const std::list<std::string> &expectedResult, const string &comment)
{
  if(testsRecorded_.find(testCase.name) == testsRecorded_.end()) {
    testsRecorded_[testCase.name] = "i";
    ++m_nTotalTests;
    ++m_nInspectTests;
  }
  else if(testsRecorded_[testCase.name] == "p") {
    --m_nPassedTests;
    ++m_nInspectTests;
  }

  cout << "," << flush;

  errorStream_ << "************************************************************************" << endl;
  errorStream_ << "* For inspection:" << endl;
  testCaseToErrorStream(testCase);
  errorStream_ << "********** Actual result: **********" << endl;
  errorStream_ << actualResult << endl;;
  errorStream_ << endl;
  for(std::list<std::string>::const_iterator it=expectedResult.begin();it!=expectedResult.end();it++)
  {
    errorStream_ << "********** Expected result: **********" << endl;;
    errorStream_ << *it << endl;
    errorStream_ << endl;
  }

  if(comment != "") {
	  errorStream_ << "********** Comment: **********" << endl;
	  errorStream_ << comment << endl;;
	  errorStream_ << endl;
  }
}

void ConsoleResultListener::reportSkip(const TestCase &testCase, const std::string &comment)
{
  if(testsRecorded_.find(testCase.name) == testsRecorded_.end()) {
    testsRecorded_[testCase.name] = "s";
    ++m_nTotalTests;
    ++m_nSkippedTests;
  }
  else if(testsRecorded_[testCase.name] == "p") {
    --m_nPassedTests;
    ++m_nSkippedTests;
  }

  cout << "^" << flush;
}

void ConsoleResultListener::reportFail(const TestCase &testCase, const string &actualResult,
                                       const std::list<std::string> &expectedResult, const string &comment)
{
  if(testsRecorded_.find(testCase.name) == testsRecorded_.end()) {
    testsRecorded_[testCase.name] = "f";
    ++m_nTotalTests;
  }
  else if(testsRecorded_[testCase.name] == "p") {
    --m_nPassedTests;
  }

  cout << "!" << flush;

  errorStream_ << "************************************************************************" << endl;
  testCaseToErrorStream(testCase);
  errorStream_ << "********** Actual result: **********" << endl;
  errorStream_ << actualResult << endl;;
  errorStream_ << endl;
  for(std::list<std::string>::const_iterator it=expectedResult.begin();it!=expectedResult.end();it++)
  {
    errorStream_ << "********** Expected result: **********" << endl;;
    errorStream_ << *it << endl;
    errorStream_ << endl;
  }

  if(comment != "") {
	  errorStream_ << "********** Comment: **********" << endl;
	  errorStream_ << comment << endl;;
	  errorStream_ << endl;
  }
}

void ConsoleResultListener::reportFailNoError(const TestCase &testCase, const string &actualResult,
                                              const string &comment)
{
  if(testsRecorded_.find(testCase.name) == testsRecorded_.end()) {
    testsRecorded_[testCase.name] = "f";
    ++m_nTotalTests;
  }
  else if(testsRecorded_[testCase.name] == "p") {
    --m_nPassedTests;
  }

  cout << "!" << flush;

  errorStream_ << "************************************************************************" << endl;
  testCaseToErrorStream(testCase);
  errorStream_ << "********** Actual result: **********" << endl;
  errorStream_ << actualResult << endl;;
  errorStream_ << endl;

  if(comment != "") {
	  errorStream_ << "********** Comment: **********" << endl;
	  errorStream_ << comment << endl;;
	  errorStream_ << endl;
  }
}

void ConsoleResultListener::reportFailUnexpectedError(const TestCase &testCase, const string &unexpectedError,
                                                      const string &comment)
{
  if(testsRecorded_.find(testCase.name) == testsRecorded_.end()) {
    testsRecorded_[testCase.name] = "f";
    ++m_nTotalTests;
  }
  else if(testsRecorded_[testCase.name] == "p") {
    --m_nPassedTests;
  }

  cout << "!" << flush;

  errorStream_ << "************************************************************************" << endl;
  testCaseToErrorStream(testCase);
  errorStream_ << "********** Actual error: **********" << endl;
  errorStream_ << unexpectedError << endl;;
  errorStream_ << endl;

  if(comment != "") {
	  errorStream_ << "********** Comment: **********" << endl;
	  errorStream_ << comment << endl;;
	  errorStream_ << endl;
  }
}

bool ConsoleResultListener::printReport() const
{
  cout << "************************************************************************" << endl;
  cout << m_nTotalTests << " Tests, "
       << m_nPassedTests << " passed, "
       << (m_nTotalTests - m_nPassedTests - m_nSkippedTests - m_nInspectTests) << " failed, "
       << m_nSkippedTests << " skipped, "
       << m_nInspectTests << " for inspection";
  cout << " (" << ((float)m_nPassedTests)/(m_nTotalTests-m_nSkippedTests-m_nInspectTests)*100 << "%)" << endl;

  cerr << errorStream_.str();

  return (m_nTotalTests - m_nPassedTests - m_nSkippedTests - m_nInspectTests) == 0;
}

void ConsoleResultListener::testCaseToErrorStream(const TestCase &testCase)
{
  errorStream_ << "* Test-case '" << m_szFullTestName << ":" << testCase.name << "':" << endl;
  errorStream_ << "* Query URL: " << testCase.queryURL << endl;
  if(!testCase.contextItem.empty())
    errorStream_ << "* Context Item: " << testCase.contextItem << endl;
  if(!testCase.defaultCollection.empty())
    errorStream_ << "* Default collection: " << testCase.defaultCollection << endl;
  std::map<std::string, std::string>::const_iterator i;
  std::list<std::pair<std::string, std::string> >::const_iterator j;
  for(i = testCase.inputVars.begin(); i != testCase.inputVars.end(); ++i) {
    errorStream_ << "* Input: " << i->first << " -> " << i->second << endl;
  }

  for(i = testCase.inputURIVars.begin(); i != testCase.inputURIVars.end(); ++i) {
    errorStream_ << "* Input URI: " << i->first << " -> " << i->second << endl;
  }

  for(i = testCase.extraVars.begin(); i != testCase.extraVars.end(); ++i) {
    errorStream_ << "* Variable: " << i->first << " -> " << i->second << endl;
  }

  for(j = testCase.moduleFiles.begin(); j != testCase.moduleFiles.end(); ++j) {
    errorStream_ << "* Module: " << j->first << " -> " << j->second << endl;
  }

  for(i = testCase.outputFiles.begin(); i != testCase.outputFiles.end(); ++i) {
    errorStream_ << "* Output: " << i->second << " -> " << i->first << endl;
  }

  if(!testCase.expectedErrors.empty()) {
    errorStream_ << "* Errors:";
    for(std::list<std::string>::const_iterator k = testCase.expectedErrors.begin();
        k != testCase.expectedErrors.end(); ++k) {
      errorStream_ << " " << *k;
    }
    errorStream_ << endl;
  }

  errorStream_ << endl;
  errorStream_ << "********** Query: **********" << endl;
  errorStream_ << testCase.query << endl;
  errorStream_ << endl;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

XMLReportResultListener::XMLReportResultListener()
  : update_(false),
    anonymous_(false)
{
}

void XMLReportResultListener::reportVersion(const std::string &version, bool update)
{
  version_ = version;
  update_ = update;
}

void XMLReportResultListener::startTestGroup(const std::string &name)
{
}

void XMLReportResultListener::endTestGroup()
{
}

void XMLReportResultListener::reportPass(const TestCase &testCase, const string &comment)
{
  if(previousTestName_ != testCase.name) {
    outputStream_ << tmpStream_.str();
  }

  tmpStream_.str("");
  previousTestName_ = testCase.name;

  tmpStream_ << "  <test-case";
  tmpStream_ << " name=\"" << testCase.name << "\"";
  tmpStream_ << " result=\"pass\"";
  if(comment != "")
    tmpStream_ << " comment=\"" << comment << "\"";
  tmpStream_ << "/>" << endl;
}

void XMLReportResultListener::reportInspect(const TestCase &testCase, const std::string &actualResult,
                                            const std::list<std::string> &expectedResult, const string &comment)
{
  if(previousTestName_ != testCase.name) {
    outputStream_ << tmpStream_.str();
  }

  tmpStream_.str("");
  previousTestName_ = testCase.name;

  tmpStream_ << "  <test-case";
  tmpStream_ << " name=\"" << testCase.name << "\"";
  tmpStream_ << " result=\"cannot tell\"";
  if(comment != "")
    tmpStream_ << " comment=\"" << comment << "\"";
  else 
    tmpStream_ << " comment=\"result has not been inspected\"";
  tmpStream_ << "/>" << endl;
}

void XMLReportResultListener::reportSkip(const TestCase &testCase, const std::string &comment)
{
  if(previousTestName_ != testCase.name) {
    outputStream_ << tmpStream_.str();
  }

  tmpStream_.str("");
  previousTestName_ = testCase.name;

  tmpStream_ << "  <test-case";
  tmpStream_ << " name=\"" << testCase.name << "\"";
  tmpStream_ << " result=\"not applicable\"";
  if(comment != "")
    tmpStream_ << " comment=\"" << comment << "\"";
  tmpStream_ << "/>" << endl;
}

void XMLReportResultListener::reportFail(const TestCase &testCase, const std::string &actualResult,
                                         const std::list<std::string> &expectedResult, const string &comment)
{
  if(previousTestName_ != testCase.name) {
    outputStream_ << tmpStream_.str();
  }

  tmpStream_.str("");
  previousTestName_ = testCase.name;

  tmpStream_ << "  <test-case";
  tmpStream_ << " name=\"" << testCase.name << "\"";
  tmpStream_ << " result=\"fail\"";
  if(comment != "")
    tmpStream_ << " comment=\"" << comment << "\"";
  tmpStream_ << "/>" << endl;
}

void XMLReportResultListener::reportFailNoError(const TestCase &testCase, const std::string &actualResult,
                                                const string &comment)
{
  if(previousTestName_ != testCase.name) {
    outputStream_ << tmpStream_.str();
  }

  tmpStream_.str("");
  previousTestName_ = testCase.name;

  tmpStream_ << "  <test-case";
  tmpStream_ << " name=\"" << testCase.name << "\"";
  tmpStream_ << " result=\"fail\"";
  if(comment != "")
    tmpStream_ << " comment=\"" << comment << "\"";
  tmpStream_ << "/>" << endl;
}

void XMLReportResultListener::reportFailUnexpectedError(const TestCase &testCase, const std::string &unexpectedError,
                                                        const string &comment)
{
  if(previousTestName_ != testCase.name) {
    outputStream_ << tmpStream_.str();
  }

  tmpStream_.str("");
  previousTestName_ = testCase.name;

  tmpStream_ << "  <test-case";
  tmpStream_ << " name=\"" << testCase.name << "\"";
  tmpStream_ << " result=\"fail\"";
  if(comment != "")
    tmpStream_ << " comment=\"" << comment << "\"";
  tmpStream_ << "/>" << endl;
}

void XMLReportResultListener::setImplementation(const std::string &name, const std::string &version, const std::string &description)
{
  implName_ = name;
  implVersion_ = version;
  implDescription_ = description;
}

void XMLReportResultListener::setOrganization(const std::string &name, const std::string &website)
{
  orgName_ = name;
  orgWebsite_ = website;
}

void XMLReportResultListener::setSubmittor(const std::string &name, const std::string &email)
{
  submittorName_ = name;
  submittorEmail_ = email;
}

void XMLReportResultListener::setTestRun(const std::string &transformation, const std::string &comments)
{
  testTransformation_ = transformation;
  testComments_ = comments;
}

void XMLReportResultListener::setAnonymous(bool anon)
{
  anonymous_ = anon;
}

void XMLReportResultListener::addImplementationDefinedItem(const std::string &name, const std::string &value)
{
  implDefinedItems_[name] = value;
}

void XMLReportResultListener::addFeature(const std::string &name, bool supported)
{
  features_[name] = supported;
}

void XMLReportResultListener::addContextProperty(const std::string &name, const std::string &contextType, const std::string &value)
{
  contextProperties_[name].first = contextType;
  contextProperties_[name].second = value;
}

void XMLReportResultListener::printReport() const
{
  cout << "<test-suite-result xmlns=\"";
  if(update_) {
    cout << "http://www.w3.org/2005/02/query-test-XQUTSResult";
  }
  else {
    cout << "http://www.w3.org/2005/02/query-test-XQTSResult";
  }
  cout << "\">" << endl;
  cout << endl;
  cout << "  <implementation name=\"" << implName_ << "\"";
  if(implVersion_ != "")
    cout << " version=\"" << implVersion_ << "\"";
  if(anonymous_)
    cout << " anonymous-result-column=\"true\"";
  cout << ">" << endl;
  cout << endl;
  cout << "    <organization name=\"" << orgName_ << "\"";
  if(orgWebsite_ != "")
    cout << " website=\"" << orgWebsite_ << "\"";
  if(anonymous_)
    cout << " anonymous=\"true\"";
  cout << "/>" << endl;
  cout << endl;
  cout << "    <submittor name=\"" << submittorName_ << "\"";
  if(submittorEmail_ != "")
    cout << " email=\"" << submittorEmail_ << "\"";
  cout << "/>" << endl;
  cout << endl;
  if(implDescription_ != "") {
    cout << "    <description>" << implDescription_ << "</description>" << endl;
    cout << endl;
  }

  if(!implDefinedItems_.empty()) {
    cout << "    <implementation-defined-items>" << endl;
    for(map<string, string>::const_iterator i = implDefinedItems_.begin(); i != implDefinedItems_.end(); ++i) {
      cout << "      <implementation-defined-item name=\"" << i->first << "\" value=\"" << i->second << "\"/>" << endl;
    }
    cout << "    </implementation-defined-items>" << endl;
    cout << endl;
  }

  if(!features_.empty()) {
    cout << "    <features>" << endl;
    for(map<string, bool>::const_iterator i = features_.begin(); i != features_.end(); ++i) {
      cout << "      <feature name=\"" << i->first << "\" supported=\"" << (i->second ? "true" : "false") << "\"/>" << endl;
    }
    cout << "    </features>" << endl;
    cout << endl;
  }

  if(!contextProperties_.empty()) {
    cout << "    <context-properties>" << endl;
    for(map<string, pair<string, string> >::const_iterator i = contextProperties_.begin(); i != contextProperties_.end(); ++i) {
      cout << "      <context-property name=\"" << i->first << "\" context-type=\"" << i->second.first << "\" value=\"" << i->second.second << "\"/>" << endl;
    }
    cout << "    </context-properties>" << endl;
    cout << endl;
  }

  cout << "  </implementation>" << endl;
  cout << endl;
  cout << "  <syntax>XQuery</syntax>" << endl;
  cout << endl;

  time_t currentTime = ::time(0);
  struct tm *curLocalDate = ::localtime(&currentTime);
  char szDate[256];
  snprintf(szDate, 256,"%04d-%02d-%02d",curLocalDate->tm_year+1900, curLocalDate->tm_mon+1, curLocalDate->tm_mday);

  cout << "  <test-run dateRun=\"" << szDate << "\">" << endl;
  cout << "    <test-suite version=\"" << version_ << "\"/>" << endl;
  if(testTransformation_ != "")
    cout << "    <transformation>" << testTransformation_ << "</transformation>" << endl;
  cout << "    <comparison><p>XML and fragment comparisons are performed by serializing the results,";
  cout << " re-parsing them, and using a custom deep-equal like function. This function currently ignores";
  cout << " any text nodes that consist entirely of whitespace.</p></comparison>" << endl;
  if(testComments_ != "")
    cout << "    <otherComments>" << testComments_ << "</otherComments>" << endl;
  cout << "  </test-run>" << endl;
  cout << endl;
  cout << outputStream_.str();
  cout << endl;
  cout << "</test-suite-result>" << endl;
}
