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

#include <xqilla/xqts/TestSuiteRunner.hpp>
#include <xqilla/xqts/TestSuiteParser.hpp>
#include <xqilla/xqts/TestSuiteResultListener.hpp>

#include <fstream>
#include <iostream>

#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/BinFileInputStream.hpp>
#include <xercesc/framework/URLInputSource.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/util/XMLChar.hpp>

#if defined(XERCES_HAS_CPP_NAMESPACE)
XERCES_CPP_NAMESPACE_USE
#endif
using namespace std;

static string loadExpectedResult(const string &file);
static bool compareNodes(DOMNode* node1, DOMNode* node2);

void TestSuiteRunner::testResults(const TestCase &testCase, const std::string &xmlResult) const
{
  if(testCase.outputFiles.empty()) {
    if(!testCase.expectedErrors.empty())
      m_results->reportFailNoError(testCase, xmlResult, "");
    // Otherwise pass - it's probably the first stage of an update test
    else m_results->reportPass(testCase, "");
  }
  else {
    bool passed = false;
    std::string compareMethod, outputResult;
    std::list<std::string> allExpectedResults;
    for(std::map<std::string, std::string>::const_iterator i=testCase.outputFiles.begin();i!=testCase.outputFiles.end();i++) {
      compareMethod=(*i).second;
      string expectedResult = loadExpectedResult((*i).first);
      if(expectedResult == "#Not found") {

//         std::ofstream output(i->first.c_str() + 8); // Take off the "file:///" from the begining
//         output << xmlResult;

        m_results->reportFail(testCase, xmlResult, allExpectedResults, "Bad test! Expected result not found: " + (*i).first);
        return;
      }

//       <xs:enumeration value="xml"/>
//       <xs:enumeration value="xml-output"/>
//       <xs:enumeration value="html-output"/>
//       <xs:enumeration value="xhtml-output"/>
//       <xs:enumeration value="xml-frag"/>
//       <xs:enumeration value="text"/>
//       <xs:enumeration value="ignore"/>
//       <xs:enumeration value="inspect"/>                  

      if(compareMethod=="Text" || compareMethod=="Fragment" || compareMethod=="XML" ||
         compareMethod=="text" || compareMethod=="xml-frag" || compareMethod=="xml" ||
         compareMethod=="xml-output" || compareMethod=="xhtml-output") {
        outputResult = xmlResult;
        if(compareMethod=="Text" || compareMethod=="Fragment" ||
           compareMethod=="text" || compareMethod=="xml-frag") {
          expectedResult="<wrapper>"+expectedResult+"</wrapper>";
          outputResult="<wrapper>"+outputResult+"</wrapper>";
        }
        // force XML 1.1 parsing
        outputResult = "<?xml version='1.1'?>" + outputResult;
        expectedResult = "<?xml version='1.1'?>" + expectedResult;

        try {
          XercesDOMParser parser;
          parser.setDoNamespaces(true);

          MemBufInputSource memSrc1((XMLByte*)outputResult.c_str(), (unsigned int)outputResult.size(), "", false);
          parser.parse(memSrc1);
          DOMDocument* doc1 = parser.getDocument();

          MemBufInputSource memSrc2((XMLByte*)expectedResult.c_str(), (unsigned int)expectedResult.size(), "", false);
          parser.parse(memSrc2);
          DOMDocument* doc2 = parser.getDocument();
          passed = compareNodes(doc1, doc2);
        }
        catch(...) {}
      }
      else if(compareMethod == "Inspect" || compareMethod == "inspect") {
        // Try if they match 
        if(expectedResult==xmlResult)
          passed=true;
      }
      else {
        // TODO
        cout << "Test-case '" << testCase.name << "': Unsupported comparison method " << compareMethod << endl;
      }
      allExpectedResults.push_back(expectedResult);
      if(passed) break;
    }

    if(passed) {
      m_results->reportPass(testCase, "");
    }
    else if(compareMethod == "Inspect") {
      m_results->reportInspect(testCase, xmlResult, allExpectedResults, "");
    }
    else {
      m_results->reportFail(testCase, outputResult, allExpectedResults, "");
    }
  }
}

void TestSuiteRunner::testErrors(const TestCase &testCase, const std::string &actualError) const
{
  if(testCase.expectedErrors.empty()) {
    m_results->reportFailUnexpectedError(testCase, actualError, "");
  }
  else {
    bool bFound = false;
    for(list<string>::const_iterator i=testCase.expectedErrors.begin();i!=testCase.expectedErrors.end();i++) {
      if(*i=="*" || actualError.find(*i) != string::npos) {
        bFound = true;
        break;
      }
    }

    if(bFound) {
      m_results->reportPass(testCase, "");
    }
    else {
      m_results->reportFailUnexpectedError(testCase, actualError, "");
    }
  }
}

static string loadExpectedResult(const string &file) {
  string expectedResult = "#Not found";

  try {
    Janitor<BinFileInputStream> stream((BinFileInputStream*)URLInputSource(file.c_str()).makeStream());
    if(stream.get()) {
      unsigned int dwSize = (unsigned int)stream->getSize();
      expectedResult.resize(dwSize);
      stream->readBytes((XMLByte*)expectedResult.c_str(), dwSize);
      if(dwSize>3 && 
         ((unsigned char)expectedResult[0])==0xEF && 
         ((unsigned char)expectedResult[1])==0xBB && 
         ((unsigned char)expectedResult[2])==0xBF)
        expectedResult.erase(0,3);

      for(string::iterator c=expectedResult.begin();c!=expectedResult.end();)
        if(*c==0xD)
          c=expectedResult.erase(c);
        else
          c++;
      while(expectedResult.size()>0 && expectedResult[expectedResult.size()-1]==0xA)
        expectedResult.erase(expectedResult.size()-1);

      if(expectedResult.find("<?xml") == 0) {
        size_t end = expectedResult.find("?>");
        expectedResult.erase(0, end + 2);
      }
    }
  } catch(...) {}

  return expectedResult;
}

static bool isIgnorableWS(DOMNode* node)
{
  return node!=NULL &&
    node->getNodeType()==DOMNode::TEXT_NODE && 
    XMLChar1_0::isAllSpaces(node->getNodeValue(), XMLString::stringLen(node->getNodeValue())) &&
    (node->getPreviousSibling()==NULL || (node->getPreviousSibling()->getNodeType()==DOMNode::ELEMENT_NODE || 
                                          node->getPreviousSibling()->getNodeType()==DOMNode::PROCESSING_INSTRUCTION_NODE ||
                                          node->getPreviousSibling()->getNodeType()==DOMNode::COMMENT_NODE)) &&
    (node->getNextSibling()==NULL || (node->getNextSibling()->getNodeType()==DOMNode::ELEMENT_NODE || 
                                      node->getNextSibling()->getNodeType()==DOMNode::PROCESSING_INSTRUCTION_NODE ||
                                      node->getNextSibling()->getNodeType()==DOMNode::COMMENT_NODE));
}

static bool compareNodes(DOMNode* node1, DOMNode* node2)
{
//   cerr << (node1->getNodeType() == DOMNode::ATTRIBUTE_NODE ? "@" : "") << UTF8(node1->getNodeName())
//        << ", " << (node2->getNodeType() == DOMNode::ATTRIBUTE_NODE ? "@" : "") << UTF8(node2->getNodeName()) << " -> ";

  if(node1->getNodeType()!=node2->getNodeType())
  {
//     cerr << "false (node type)\n";
    return false;
  }
  if(node1->hasChildNodes() != node2->hasChildNodes())
  {
//     cerr << "false (child nodes)\n";
    return false;
  }
  if(node1->getNodeType()==DOMNode::ELEMENT_NODE)
  {
    DOMElement* e1=(DOMElement*)node1;
    DOMElement* e2=(DOMElement*)node2;
    if(!XMLString::equals(e1->getNamespaceURI(), e2->getNamespaceURI()))
    {
//       cerr << "false (uri)\n";
      return false;
    }
    if(!XMLString::equals(e1->getLocalName(), e2->getLocalName()))
    {
//       cerr << "false (localname)\n";
      return false;
    }
    DOMNamedNodeMap* map1=e1->getAttributes();
    DOMNamedNodeMap* map2=e2->getAttributes();

    if(map1->getLength()!=map2->getLength()) {
//       cerr << "false (num attrs)\n";
      return false;
    }
    for(unsigned int i=0;i<map1->getLength();i++)
    {
      DOMNode* a1=map1->item(i);
      DOMNode* a2=map2->getNamedItemNS(a1->getNamespaceURI(),a1->getLocalName());
      if(a2==NULL)
      {
//         cerr << "false (no a2 attr)\n";
        return false;
      }
      if(!compareNodes(a1,a2)) {
//         cerr << "false (attr)\n";
        return false;
      }
    }
  }
  else
  {
    if(!XMLString::equals(node1->getNodeName(),node2->getNodeName()))
    {
//       cerr << "false (node name)\n";
      return false;
    }
    if(!XMLString::equals(node1->getNodeValue(),node2->getNodeValue()))
    {
//       cerr << "false (node value)\n";
//       cerr << "  \"" << UTF8(node1->getNodeValue()) << "\" != \"" << UTF8(node2->getNodeValue()) << "\"\n";
      return false;
    }
  }
  DOMNode* n1=node1->getFirstChild();
  DOMNode* n2=node2->getFirstChild();
  while(n1 && n2)
  {
    if(isIgnorableWS(n1))
    {
      n1=n1->getNextSibling();
    }
    if(isIgnorableWS(n2))
    {
      n2=n2->getNextSibling();
    }
    if(n1==NULL || n2==NULL)
      break;

    if(!compareNodes(n1, n2)) {
//       cerr << "false (child node)\n";
      return false;
    }
    n1=n1->getNextSibling();
    n2=n2->getNextSibling();
  }
  if(isIgnorableWS(n1))
  {
    n1=n1->getNextSibling();
  }
  if(isIgnorableWS(n2))
  {
    n2=n2->getNextSibling();
  }

  if(n1!=NULL || n2!=NULL) {
//     cerr << "false (extra children)\n";
    return false;
  }
//   cerr << "true\n";
  return true;
}
