import tmrm = require('azure-pipelines-task-lib/mock-run');
import testHelpers = require('../testhelpers');

const os = require('os');

process.env['AGENT_TEMPDIRECTORY'] = os.tmpdir();
const taskMockRunner: tmrm.TaskMockRunner = new tmrm.TaskMockRunner(testHelpers.TaskEntryPoints.AVDAppAttachPublishTask);

// read avd inputs from avdinputs.json file
testHelpers.setUpAvdPublishInputs(taskMockRunner);
taskMockRunner.run(/* noMockTask */ true);
