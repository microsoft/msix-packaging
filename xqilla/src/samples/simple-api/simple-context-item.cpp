#include <iostream>
#include <xqilla/xqilla-simple.hpp>
#include <xqilla/runtime/Sequence.hpp>

int main(int argc, char *argv[]) {
  // Initialise Xerces-C and XQilla by creating the factory object
  XQilla xqilla;

  // Parse an XQuery expression
  // (AutoDelete deletes the object at the end of the scope)
  AutoDelete<XQQuery> query(xqilla.parse(X("foo/bar/@baz")));

  // Create a context object
  AutoDelete<DynamicContext> context(query->createDynamicContext());

  // Parse a document, and set it as the context item
  Sequence seq = context->resolveDocument(X("foo.xml"));
  if(!seq.isEmpty() && seq.first()->isNode()) {
    context->setContextItem(seq.first());
    context->setContextPosition(1);
    context->setContextSize(1);
  }

  // Execute the query, using the context
  Result result = query->execute(context);

  // Iterate over the results, printing them
  Item::Ptr item;
  while(item = result->next(context)) {
    std::cout << UTF8(item->asString(context)) << std::endl;
  }

  return 0;
}
