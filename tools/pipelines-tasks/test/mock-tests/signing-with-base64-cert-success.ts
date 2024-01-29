import tmrm = require('azure-pipelines-task-lib/mock-run');
import fs = require('fs');
import path = require('path');
import helpers = require('common-helpers/helpers');
import { ToolRunner } from 'azure-pipelines-task-lib/toolrunner';
import testHelpers = require('../testhelpers');

const taskMockRunner = new tmrm.TaskMockRunner(testHelpers.TaskEntryPoints.SigningTask);
const HELPER_SCRIPT = 'C:/Users/zmazumder/source/repos/msix-packaging/tools/pipelines-tasks/encodeCert.ps1'

async function GetEncodedCertificate() {
    const powershellRunner: ToolRunner = helpers.getPowershellRunner(HELPER_SCRIPT);
    powershellRunner.arg(['-filePath', '\'' + testHelpers.testCertificate + '\'']);

    let execResult = await powershellRunner.execSync();
    const certUtf8 = execResult.stdout.trim();
    return certUtf8;
}

const encodedCertificate = GetEncodedCertificate();
taskMockRunner.setInput('package', testHelpers.testPackage);
taskMockRunner.setInput('certificateType', 'base64');
taskMockRunner.setInput('encodedCertificate', encodedCertificate);

// Don't mock call to signtool
taskMockRunner.run(/* noMockTask */ true);