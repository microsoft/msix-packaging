import tmrm = require('azure-pipelines-task-lib/mock-run');
import testHelpers = require('../testhelpers');

const taskMockRunner: tmrm.TaskMockRunner = new tmrm.TaskMockRunner(testHelpers.TaskEntryPoints.AppInstallerTask);
testHelpers.setUpUpdateAppInstallerFileArguments(taskMockRunner, __filename, testHelpers.testPackage);

// Don't mock call to makeappx to unpack
taskMockRunner.run(/* noMockTask */ true);