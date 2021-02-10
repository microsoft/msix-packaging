import tmrm = require('azure-pipelines-task-lib/mock-run');
import fs = require('fs');
import testHelpers = require('../testhelpers');

const taskMockRunner = new tmrm.TaskMockRunner(testHelpers.TaskEntryPoints.SigningTask);

taskMockRunner.setInput('package', testHelpers.testPackage);
taskMockRunner.setInput('certificateType', 'base64');
taskMockRunner.setInput('encodedCertificate', fs.readFileSync(testHelpers.testEncodedCertificate).toString());

// Don't mock call to signtool
taskMockRunner.run(/* noMockTask */ true);