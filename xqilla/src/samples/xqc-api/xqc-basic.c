#include <xqilla/xqilla-xqc.h>

int main(int argc, char *argv[])
{
  XQC_Implementation *impl;
  XQC_Expression *expr;
  XQC_Sequence *seq;
  XQC_Error err;
  const char *value;

  // XQilla specific way to create an XQC_Implementation struct
  impl = createXQillaXQCImplementation(XQC_VERSION_NUMBER);
  if(impl == 0) return 1;

  // Prepare an XQuery expression
  err = impl->prepare(impl, "1 to 100", 0, &expr);
  if(err != 0) goto free_impl;

  // Execute the query
  err = expr->execute(expr, 0, &seq);
  if(err != 0) goto free_expr;

  // Iterate over the results, printing them
  while((err = seq->next(seq)) == XQC_NO_ERROR) {
    seq->string_value(seq, &value);
    printf("%s\n", value);
  }

  if(err == XQC_END_OF_SEQUENCE)
    err = XQC_NO_ERROR;

  // free everything
  seq->free(seq);
free_expr:
  expr->free(expr);
free_impl:
  impl->free(impl);

  return err;
}
