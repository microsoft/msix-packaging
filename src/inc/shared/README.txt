Contains headers for consumption by both the product and *functional* tests; can only reference each other, public headers, and external dependencies.

Any functions used by tests must be implemented in the headers, as the tests will not be linking in anything.