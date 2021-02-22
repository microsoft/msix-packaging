import tmrm = require('azure-pipelines-task-lib/mock-run');
import fs = require('fs');
import testHelpers = require('../testhelpers');

const taskMockRunner = new tmrm.TaskMockRunner(testHelpers.TaskEntryPoints.SigningTask);

const encodedCertificate = fs.readFileSync(testHelpers.testEncodedCertificate, { encoding: 'utf-8' }).toString().trim();
taskMockRunner.setInput('package', testHelpers.testPackage);
taskMockRunner.setInput('certificateType', 'base64');
taskMockRunner.setInput('encodedCertificate', encodedCertificate);

// Don't mock call to signtool
taskMockRunner.run(/* noMockTask */ true);