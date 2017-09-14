#include <iostream>
#include <xqilla/xqilla-simple.hpp>

int main(int argc, char *argv[]) {
  // Initialise Xerces-C and XQilla by creating the factory object
  XQilla xqilla;

  // Parse an XQuery expression
  // (AutoDelete deletes the object at the end of the scope)
  AutoDelete<XQQuery> query(xqilla.parse(X("1 to 100")));

  // Create a context object
  AutoDelete<DynamicContext> context(query->createDynamicContext());

  // Execute the query, using the context
  Result result = query->execute(context);

  // Iterate over the results, printing them
  Item::Ptr item;
  while(item = result->next(context)) {
    std::cout << UTF8(item->asString(context)) << std::endl;
  }

  return 0;
}
