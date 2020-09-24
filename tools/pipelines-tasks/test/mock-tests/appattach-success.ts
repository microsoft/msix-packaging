import tmrm = require('azure-pipelines-task-lib/mock-run');
import testHelpers = require('../testhelpers');

const taskMockRunner = new tmrm.TaskMockRunner(testHelpers.TaskEntryPoints.AppAttachTask);
taskMockRunner.setInput('package', testHelpers.testPackage);
taskMockRunner.setInput('vhdxOutputPath', testHelpers.outputFilePath('TestVhdx.vhdx'));
taskMockRunner.setInput('vhdxSize', '50');

// Don't mock call to script
taskMockRunner.run(/* noMockTask */ true);