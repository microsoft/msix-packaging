#include <xqilla/xqilla-xqc.h>

int main(int argc, char *argv[])
{
  XQC_Implementation *impl;
  XQC_Expression *expr;
  XQC_DynamicContext *context;
  XQC_Sequence *seq, *doc;
  XQC_Error err;
  const char *value;

  // XQilla specific way to create an XQC_Implementation struct
  impl = createXQillaXQCImplementation(XQC_VERSION_NUMBER);
  if(impl == 0) return 1;

  // Parse an XQuery expression
  err = impl->prepare(impl, "foo/bar/@baz", 0, &expr);
  if(err != 0) goto free_impl;

  // Parse a document
  err = impl->parse_document(impl, "<foo><bar baz='hello'/></foo>", &doc);
  if(err != 0) goto free_expr;

  // Create a dynamic context
  err = expr->create_context(expr, &context);
  if(err != 0) goto free_doc;

  // Set the document as the context item
  doc->next(doc);
  context->set_context_item(context, doc);

  // Execute the query
  err = expr->execute(expr, context, &seq);
  if(err != 0) goto free_context;

  // Iterate over the results, printing them
  while((err = seq->next(seq)) == XQC_NO_ERROR) {
    seq->string_value(seq, &value);
    printf("%s\n", value);
  }

  if(err == XQC_END_OF_SEQUENCE)
    err = XQC_NO_ERROR;

  // free everything
  seq->free(seq);
free_context:
  context->free(context);
free_doc:
  doc->free(doc);
free_expr:
  expr->free(expr);
free_impl:
  impl->free(impl);

  return err;
}
