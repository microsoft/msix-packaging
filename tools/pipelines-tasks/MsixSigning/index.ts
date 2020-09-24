import path = require('path');
import tl = require('azure-pipelines-task-lib/task')
import { ToolRunner } from 'azure-pipelines-task-lib/toolrunner';

import helpers = require('common/helpers');

import download = require('./predownloadsecurefile');

const SIGNTOOL_PATH = path.join(__dirname, 'lib', 'signtool');

/**
 * Signs a package or bundle.
 * @param packagePath Path to the package to sign.
 * @param certificateFilePath Path to the certificate to use for signing.
 * @param timeStampServer Time stamp server to use. Can be undefined if no time stamp is needed.
 * @param password Password for the certificate. Can be undefined if not needed.
 */
const signPackage = async (packagePath: string, certificateFilePath: string, password: string | undefined, timeStampServer: string | undefined) =>
{
    const signtoolRunner: ToolRunner = tl.tool(SIGNTOOL_PATH);

    // Base arguments
    signtoolRunner.line('sign /a /v /fd SHA256');

    // Certificate to use
    signtoolRunner.arg(['/f', certificateFilePath]);
    if (password)
    {
        signtoolRunner.arg(['/p', password]);
    }

    // Time stamp options
    if (timeStampServer)
    {
        signtoolRunner.arg(['/tr', timeStampServer, '/td', 'SHA256']);
    }

    signtoolRunner.arg(packagePath);
    await signtoolRunner.exec();
}

/**
 * Main function for the task.
 */
const run = async () =>
{
    tl.setResourcePath(path.join(__dirname, 'task.json'));

    const packagePathPattern: string = helpers.getInputWithErrorCheck('package', 'No package path specified.');

    // Download certificate stored as secure file
    const certificateSecureFileId: string = helpers.getInputWithErrorCheck('certificate', 'A certificate input is required for download to sign the app.');
    const certificateFilePath: string = await download.downloadSecureFile(certificateSecureFileId, /* retryCount */ 5);

    // Get the certificate password.
    // Instead of parsing a password for the certificate as a plain string, we attempt to get the password as
    // a secret variable saved to the pipeline from a variable group.
    // No password variable means the certificate doens't need a password.
    const passwordVariable: string | undefined = tl.getInput('passwordVariable');
    let password: string | undefined;
    if (passwordVariable)
    {
        password = tl.getVariable(passwordVariable);
        if (password === undefined)
        {
            throw Error('The secret variable given does not point to a valid password.');
        }
    }

    // No time stamp server means to not add a time stamp.
    const timeStampServer: string | undefined = tl.getInput('timeStampServer');

    // Resolve the package paths.
    const packagesToSign: string[] = tl.findMatch(/* defaultRoot */ '', packagePathPattern);

    // Sign the packages
    for (const packagePath of packagesToSign)
    {
        await signPackage(packagePath, certificateFilePath, password, timeStampServer);
    }
}

run().catch(err =>
    {
        tl.setResult(tl.TaskResult.Failed, err.message);
    })