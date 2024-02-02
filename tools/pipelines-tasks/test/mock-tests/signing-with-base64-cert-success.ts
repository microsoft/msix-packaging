import tmrm = require('azure-pipelines-task-lib/mock-run');
import fs = require('fs');
import testHelpers = require('../testhelpers');
import path = require('path');
const pem = require("pem");

process.env['OPENSSL_BIN'] = path.join(__dirname, "..\\..\\node_modules\\pem\\openssl");

//Function to convert pfx to base64 encoded string
function convertPfxToBase64() {
    const pfx = fs.readFileSync(testHelpers.testCertificate);
    return new Promise((resolve, reject) => {
        pem.readPkcs12(pfx, { p12Password: "password" }, (err: string, cert: any) => {
            if (err) {
                reject(err);
            } else {
                const pfxCert = cert.cert as string;
                const utf8Cert = pfxCert.replace(/\n/g, '')
                    .replace(/-----BEGIN CERTIFICATE-----/, '')
                    .replace(/-----END CERTIFICATE-----/, '');
                resolve(utf8Cert);
            }
        });
    });
}

//Call the function to convert pfx to base64 and then mock the task
convertPfxToBase64().then((encodedCertificate) => {
    const taskMockRunner = new tmrm.TaskMockRunner(testHelpers.TaskEntryPoints.SigningTask);
    taskMockRunner.setInput('package', testHelpers.testPackage);
    taskMockRunner.setInput('certificateType', 'base64');
    taskMockRunner.setInput('encodedCertificate', encodedCertificate as string);

    // Don't mock call to signtool
    taskMockRunner.run(/* noMockTask */ true);
});