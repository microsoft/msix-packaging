import tmrm = require('azure-pipelines-task-lib/mock-run');
import testHelpers = require('../testhelpers');

const taskMockRunner: tmrm.TaskMockRunner = new tmrm.TaskMockRunner(testHelpers.TaskEntryPoints.AppInstallerTask);
testHelpers.setUpNewAppInstallerFileArguments(taskMockRunner, __filename, testHelpers.testPackage);
taskMockRunner.setInput("updateOnLaunch", "true");
taskMockRunner.setInput("hoursBetweenUpdateChecks", "24");
taskMockRunner.setInput("showPromptWhenUpdating", "true");
taskMockRunner.setInput("updateBlocksActivation", "true");

// Don't mock call to makeappx to unpack
taskMockRunner.run(/* noMockTask */ true);