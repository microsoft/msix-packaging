#include <iostream>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/framework/StdOutFormatTarget.hpp>
#include <xqilla/xqilla-dom3.hpp>

XERCES_CPP_NAMESPACE_USE;

int main(int argc, char *argv[]) {
  // Initialise Xerces-C and XQilla using XQillaPlatformUtils
  XQillaPlatformUtils::initialize();

  // Get the XQilla DOMImplementation object
  DOMImplementation *xqillaImplementation =
    DOMImplementationRegistry::getDOMImplementation(X("XPath2 3.0"));

  try {
    // Create a DOMLSParser object
    AutoRelease<DOMLSParser> parser(xqillaImplementation->createLSParser(DOMImplementationLS::MODE_SYNCHRONOUS, 0));
    parser->getDomConfig()->setParameter(XMLUni::fgDOMNamespaces, true);
    parser->getDomConfig()->setParameter(XMLUni::fgXercesSchema, true);
    parser->getDomConfig()->setParameter(XMLUni::fgDOMValidateIfSchema, true);

    // Parse a DOMDocument
    DOMDocument *document = parser->parseURI("foo.xml");
    if(document == 0) {
      std::cerr << "Document not found." << std::endl;
      return 1;
    }

    // Parse an XPath 2 expression
    AutoRelease<DOMXPathExpression> expression(document->createExpression(X("foo/bar/@baz"), 0));

    // Execute the query
    AutoRelease<DOMXPathResult> result(expression->evaluate(document, DOMXPathResult::ITERATOR_RESULT_TYPE, 0));

    // Create a DOMLSSerializer to output the nodes
    AutoRelease<DOMLSSerializer> serializer(xqillaImplementation->createLSSerializer());
    AutoRelease<DOMLSOutput> output(xqillaImplementation->createLSOutput());
    StdOutFormatTarget target;
    output->setByteStream(&target);

    // Iterate over the results, printing them
    while(result->iterateNext()) {
      serializer->write(result->getNodeValue(), output);
      std::cout << std::endl;
    }
  }
  catch(XQillaException &e) {
    std::cerr << "XQillaException: " << UTF8(e.getString()) << std::endl;
    return 1;
  }

  // Terminate Xerces-C and XQilla using XQillaPlatformUtils
  XQillaPlatformUtils::terminate();

  return 0;
}
