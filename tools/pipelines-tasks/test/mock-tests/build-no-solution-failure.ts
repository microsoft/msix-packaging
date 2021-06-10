import tmrm = require('azure-pipelines-task-lib/mock-run');
import testHelpers = require('../testhelpers');

const taskMockRunner: tmrm.TaskMockRunner = new tmrm.TaskMockRunner(testHelpers.TaskEntryPoints.PackagingTask);

taskMockRunner.setInput('buildSolution', 'TRUE');
taskMockRunner.setInput('outputPath', testHelpers.outputFilePath('App.msix'));
taskMockRunner.setInput('buildPlatform', 'x86');
taskMockRunner.setInput('buildConfiguration', 'release');
testHelpers.setUpMSBuildLocationArguments(taskMockRunner);

taskMockRunner.run();