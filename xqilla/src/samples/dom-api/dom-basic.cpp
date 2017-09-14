#include <iostream>
#include <xercesc/dom/DOM.hpp>
#include <xqilla/xqilla-dom3.hpp>

XERCES_CPP_NAMESPACE_USE;

int main(int argc, char *argv[]) {
  // Initialise Xerces-C and XQilla using XQillaPlatformUtils
  XQillaPlatformUtils::initialize();

  // Get the XQilla DOMImplementation object
  DOMImplementation *xqillaImplementation =
    DOMImplementationRegistry::getDOMImplementation(X("XPath2 3.0"));

  // Make sure the DOM objects are released before we call XQillaPlatformUtils::terminate()
  {
    // Create a DOMDocument
    AutoRelease<DOMDocument> document(xqillaImplementation->createDocument());

    // Parse an XPath 2 expression
    AutoRelease<DOMXPathExpression> expression(document->createExpression(X("1 to 100"), 0));

    // Execute the query
    AutoRelease<DOMXPathResult> result(expression->evaluate(0, DOMXPathResult::ITERATOR_RESULT_TYPE, 0));

    // Iterate over the results, printing them
    while(result->iterateNext()) {
      std::cout << result->getIntegerValue() << std::endl;
    }
  }

  // Terminate Xerces-C and XQilla using XQillaPlatformUtils
  XQillaPlatformUtils::terminate();

  return 0;
}
