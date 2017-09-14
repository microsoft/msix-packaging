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

#include <iostream>

#include <xercesc/dom/DOM.hpp>
#include <xercesc/framework/StdOutFormatTarget.hpp>

#include <xqilla/xqilla-dom3.hpp>

XERCES_CPP_NAMESPACE_USE;
using namespace std;

int main(int argc, char *argv[])
{
  if(argc != 2) {
    cerr << "Data file not specified." << endl;
    return 1;
  }

  ///////////////////////
  // Initialise XQilla //
  ///////////////////////

  // 1. Initialize XQilla
  //
  // Note that this initialisation takes care of initialising
  // xerces-c as well
  try {
    
    XQillaPlatformUtils::initialize();
    
  } catch (const XMLException& eXerces) {
    cerr << "Error during Xerces-C initialisation.\n"
              << "Xerces exception message: "
              << UTF8(eXerces.getMessage()) << endl;
    return 1;
  }

  try {

    // 2. Obtain a DOM3 XPath2 implementation.  This is a XQilla-specific 
    //    implementation that overrides the standard DOMDocument, the standard 
    //    DOMWriter and the standard DOMBuilder
    DOMImplementation* xqillaImplementation = 
      DOMImplementationRegistry::getDOMImplementation(X("XPath2 3.0"));

    // 3. Obtain a parser and set 'do namespaces', 'use schema' and 'validate' to 
    //    true.
    // Create a DOMLSParser object
    AutoRelease<DOMLSParser> parser(xqillaImplementation->createLSParser(DOMImplementationLS::MODE_SYNCHRONOUS, 0));
    parser->getDomConfig()->setParameter(XMLUni::fgDOMNamespaces, true);
    parser->getDomConfig()->setParameter(XMLUni::fgXercesSchema, true);
    parser->getDomConfig()->setParameter(XMLUni::fgDOMValidateIfSchema, true);

    ////////////////////////////
    // Parse our XML document //
    ////////////////////////////
  
    DOMDocument *document = parser->parseURI(argv[1]);
    if(document == 0) {
      cerr << "Document not found: " << argv[1] << endl;
      return 1;
    }

    ////////////////////////////////
    // Create a parsed expression //
    // (compiled) and evaluate it //
    ////////////////////////////////


    // 1. Create a Namespace Resolver.  This holds a map of namespace prefixes 
    //    to namespace URIs.
    AutoRelease<DOMXPathNSResolver> resolver(document->createNSResolver(document->getDocumentElement()));
    resolver->addNamespaceBinding(X("my"), X("http://example.com/myURI"));
  
    // **************** Example 1: max() function ****************** //

    {  
      // 2. Create a parsed expression
      const char* expression = "max(/Catalogue/Book/Price)";
      AutoRelease<DOMXPathExpression> parsedExpression(document->createExpression(X(expression), resolver));

      // 3. Evaluate the expression. We choose to have a first result, since we
      //    know the answer will have only one item. Could also choose
      //    XPath2Result::SNAPSHOT_RESULT and XPath2Result::ITERATOR_RESULT
      AutoRelease<DOMXPathResult> firstResult(parsedExpression->evaluate(document->getDocumentElement(), DOMXPathResult::FIRST_RESULT_TYPE, 0));

      // 4. Work with the result: output it to the screen in this case
      cout << "The answer for expression '" << expression << "' is: " << firstResult->getNumberValue() << endl;
    }

    // **************** Example 2: output of nodes ****************** //

    {
      // 2. Create a parsed expression
      const char* expression2 = "//Magazine";
      AutoRelease<DOMXPathExpression> parsedExpression(document->createExpression(X(expression2), resolver));

      // 3. Evaluate the expression. We choose to have an iterator result
      AutoRelease<DOMXPathResult> iteratorResult(parsedExpression->evaluate(document->getDocumentElement(), DOMXPathResult::ITERATOR_RESULT_TYPE, 0));

      // 4. Work with the result: output it to the screen in this case
  
      // Create a DOMLSSerializer to output the nodes
      AutoRelease<DOMLSSerializer> serializer(xqillaImplementation->createLSSerializer());
      AutoRelease<DOMLSOutput> output(xqillaImplementation->createLSOutput());
      StdOutFormatTarget target;
      output->setByteStream(&target);

      cout << "The answer for expression '" << expression2 << "' is: " <<endl;

      int i = 0;
      while(iteratorResult->iterateNext()) {
        if(iteratorResult->isNode()) {
          cout << "Node " << i++ << ": "<< flush;
          serializer->write(iteratorResult->getNodeValue(), output);
          cout << endl;
        } else {
          cerr << "Expected a node but received an atomic value!"<< endl;
        }
      }
    }

  }
  catch(DOMException &e) {
    cerr << "DOMException: " << UTF8(e.getMessage()) << endl;
    return 1;
  }

  ///////////////
  // Terminate //
  ///////////////

  XQillaPlatformUtils::terminate();
  
  return 0;
}
