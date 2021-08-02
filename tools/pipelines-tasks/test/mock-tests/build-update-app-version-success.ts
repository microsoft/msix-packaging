import tmrm = require('azure-pipelines-task-lib/mock-run');
import testHelpers = require('../testhelpers');

const taskMockRunner: tmrm.TaskMockRunner = new tmrm.TaskMockRunner(testHelpers.TaskEntryPoints.PackagingTask);

taskMockRunner.setInput('updateAppVersion', 'TRUE');
taskMockRunner.setInput('manifestFile', testHelpers.testProjectManifest);
taskMockRunner.setInput('appVersion', '1.2.3.4');
testHelpers.setUpBasicMSBuildArguments(taskMockRunner, __filename);

// Don't mock call to MSBuild
taskMockRunner.run(/* noMockTask */ true);
