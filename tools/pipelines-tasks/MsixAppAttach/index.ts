import path = require('path');
import tl = require('azure-pipelines-task-lib/task')

import { ToolRunner } from 'azure-pipelines-task-lib/toolrunner';

import helpers = require('common/helpers');

const MSIXMGR_PATH = path.join(__dirname, 'lib', 'msixmgr');
const GENERATE_VHDX_SCRIPT_PATH = path.join(__dirname, 'GenerateAppAttachVhdx.ps1')

/**
 * Main function for the task.
 */
const run = async () =>
{
    tl.setResourcePath(path.join(__dirname, 'task.json'));

    // Read the task's inputs
    const packagePath: string = helpers.getInputWithErrorCheck('package', 'No package path specified.');
    const vhdxPath: string = helpers.getInputWithErrorCheck('vhdxOutputPath', 'A path is needed to create a new VHDX file, but none was given.');
    const vhdxSize: string = helpers.getInputWithErrorCheck('vhdxSize', 'A size is needed to create a new VHDX file, but none was given.');

    // The script requires the command path to be absolute.
    const fullVhdxPath: string = path.resolve(vhdxPath);
    const powershellRunner: ToolRunner = helpers.getPowershellRunner(GENERATE_VHDX_SCRIPT_PATH);
    powershellRunner.arg(['-vhdxPath', fullVhdxPath]);
    powershellRunner.arg(['-vhdxSize', vhdxSize]);
    powershellRunner.arg(['-msixPackagePath', packagePath]);
    powershellRunner.arg(['-msixmgrPath', MSIXMGR_PATH]);

    // This script needs to be run as administrator.
    await powershellRunner.exec();
}

run().catch(err =>
    {
        tl.setResult(tl.TaskResult.Failed, err.message);
    })