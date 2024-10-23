The pfx files here are arbitary ones generated just for these tests; should you
get an alert related to them, feel free to ignore it.

These were generated as follows:

test-self-signed-pfx-no-pass.pfx:
openssl req -x509 -newkey rsa:4096 -keyout key.pem -out cert.pem -sha256 -days 36500 -noenc -subj "/C=US/ST=Washington/L=Redmond/O=Microsoft Corporation/CN=Microsoft Corporation" --addext "basicConstraints=critical,CA:FALSE" --addext "keyUsage=digitalSignature" --addext "extendedKeyUsage=codeSigning"
openssl pkcs12 -export -out test-self-signed-pfx-no-pass.pfx -inkey key.pem -in cert.pem -passout "pass:"

test-self-signed-pfx-simple-pass.pfx:
openssl req -x509 -newkey rsa:4096 -keyout key.pem -out cert.pem -sha256 -days 36500 -noenc -subj "/C=US/ST=Washington/L=Redmond/O=Microsoft Corporation/CN=Microsoft Corporation" --addext "basicConstraints=critical,CA:FALSE" --addext "keyUsage=digitalSignature" --addext "extendedKeyUsage=codeSigning"
openssl pkcs12 -export -out test-self-signed-pfx-simple-pass.pfx -inkey key.pem -in cert.pem -passout "pass:testPass"


