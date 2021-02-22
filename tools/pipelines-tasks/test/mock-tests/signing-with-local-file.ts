import tmrm = require('azure-pipelines-task-lib/mock-run');
import testHelpers = require('../testhelpers');

const taskMockRunner = new tmrm.TaskMockRunner(testHelpers.TaskEntryPoints.SigningTask);

taskMockRunner.setInput('package', testHelpers.testPackage);
testHelpers.setUpCertificateInputs(taskMockRunner);

// Don't mock call to signtool
taskMockRunner.run(/* noMockTask */ true);