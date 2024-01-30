import tmrm = require('azure-pipelines-task-lib/mock-run');
import fs = require('fs');
import testHelpers = require('../testhelpers');
import path = require('path');
//import helpers = require('common-helpers/helpers');
//import helpers = require('common/helpers');
import { ToolRunner } from 'azure-pipelines-task-lib/toolrunner';
import tl = require('azure-pipelines-task-lib/task');

const taskMockRunner = new tmrm.TaskMockRunner(testHelpers.TaskEntryPoints.SigningTask);
const HELPER_SCRIPT = 'C:/Users/zmazumder/source/repos/msix-packaging/tools/pipelines-tasks/EncodeCert.ps1'
/**
 * Gets a ToolRunner for running a Powershell script.
 * Script arguments can be added later by the caller.
 * @param scriptPath Script to run.
 */
export const getPowershellRunner = (scriptPath: string): ToolRunner => {
    const powershellRunner: ToolRunner = tl.tool('powershell')
        .arg('-NoLogo')
        .arg('-NoProfile')
        .arg('-NonInteractive')
        .arg(['-ExecutionPolicy', 'Unrestricted']);

    // Quote the script path to allow for spaces.
    // Existing quotes need to be escaped.
    powershellRunner.arg(`& '${scriptPath.replace("'", "''")}'`);
    return powershellRunner;
}
async function GetEncodedCertificate() {
    const powershellRunner: ToolRunner = getPowershellRunner(HELPER_SCRIPT);
    powershellRunner.arg(['-filePath', '\'' + testHelpers.testCertificate + '\'']);

    let execResult = await powershellRunner.execSync();
    const certUtf8 = execResult.stdout.trim();
    console.log(certUtf8);
    return certUtf8;
}

const encodedCertificate = GetEncodedCertificate();
/*const base64String = fs.readFileSync(testHelpers.testEncodedCertificate, 'base64').toString().trim();
const encodedCertificate = Buffer.from(base64String, 'base64').toString('utf-8');*/
taskMockRunner.setInput('package', testHelpers.testPackage);
taskMockRunner.setInput('certificateType', 'base64');
taskMockRunner.setInput('encodedCertificate', encodedCertificate);

// Don't mock call to signtool
taskMockRunner.run(/* noMockTask */ true);