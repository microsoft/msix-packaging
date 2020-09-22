import tmrm = require('azure-pipelines-task-lib/mock-run');
import testHelpers = require('../testhelpers');

const taskMockRunner: tmrm.TaskMockRunner = new tmrm.TaskMockRunner(testHelpers.TaskEntryPoints.PackagingTask);

testHelpers.setUpBasicPackageBinariesArguments(taskMockRunner, __filename);

// Don't mock call to makeappx
taskMockRunner.run(/* noMockTask */ true);