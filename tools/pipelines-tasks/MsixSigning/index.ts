import path = require('path');
import tl = require('azure-pipelines-task-lib/task')
import { ToolRunner } from 'azure-pipelines-task-lib/toolrunner';

import helpers = require('common/helpers');

import download = require('./predownloadsecurefile');

const SIGNTOOL_PATH = path.join(__dirname, 'lib', 'signtool');
const IMPORT_CERT_SCRIPT_PATH = path.join(__dirname, 'ImportCert.ps1');

interface SigningType
{
    prepareCert(): Promise<void>,
    cleanupCert(): void,
    addSignToolCertOptions(signtoolRunner: ToolRunner): void
}

class SecureFileSigningType implements SigningType
{
    secureFileId: string;
    password?: string;
    certFilePath?: string;

    constructor()
    {
        this.secureFileId = tl.getInput('certificate', /* required */ true)!;

        // Get the certificate password.
        // Instead of parsing a password for the certificate as a plain string, we attempt to get the password as
        // a secret variable saved to the pipeline from a variable group.
        // No password variable means the certificate doesn't need a password.
        const passwordVariable: string | undefined = tl.getInput('passwordVariable');
        if (passwordVariable)
        {
            this.password = tl.getVariable(passwordVariable);
            if (this.password === undefined)
            {
                throw Error('The secret variable given does not point to a valid password.');
            }
        }
    }

    async prepareCert(): Promise<void>
    {
        this.certFilePath = await download.downloadSecureFile(this.secureFileId);
    }

    cleanupCert()
    {
        download.deleteSecureFile(this.secureFileId);
    }

    addSignToolCertOptions(signtoolRunner: ToolRunner): void
    {
        signtoolRunner.arg(['/f', this.certFilePath!]);
        if (this.password)
        {
            signtoolRunner.arg(['/p', this.password]);
        }
    }
}

class Base64EncodedCertSigningType implements SigningType
{
    base64String: string;
    certThumbprint?: string;

    constructor()
    {
        this.base64String = tl.getInput('encodedCertificate', /* required */ true)!;
    }

    async prepareCert(): Promise<void>
    {
        const powershellRunner = helpers.getPowershellRunner(IMPORT_CERT_SCRIPT_PATH);
        powershellRunner.arg(this.base64String);

        const result = powershellRunner.execSync();
        if (result.code)
        {
            throw result.stderr;
        }

        this.certThumbprint = result.stdout.trim();
        tl.debug('cert thumbprint: ' + this.certThumbprint);
    }

    cleanupCert()
    {
        const powershellRunner = helpers.getPowershellRunner(IMPORT_CERT_SCRIPT_PATH);
        powershellRunner.arg(this.base64String);
        powershellRunner.arg('-remove');
        powershellRunner.exec();
    }

    addSignToolCertOptions(signtoolRunner: ToolRunner): void
    {
        signtoolRunner.arg(['/sha1', this.certThumbprint!]);
    }
}

/**
 * Signs a package or bundle.
 * @param packagePath Path to the package to sign.
 * @param certificateFilePath Path to the certificate to use for signing.
 * @param timeStampServer Time stamp server to use. Can be undefined if no time stamp is needed.
 * @param password Password for the certificate. Can be undefined if not needed.
 */
const signPackage = async (packagePath: string, signingType: SigningType, timeStampServer: string | undefined) =>
{
    const signtoolRunner: ToolRunner = tl.tool(SIGNTOOL_PATH);

    // Base arguments
    signtoolRunner.line('sign /debug /v /a /fd SHA256');

    // Certificate to use.
    signingType.addSignToolCertOptions(signtoolRunner);

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

    const packagePathPattern: string = tl.getInput('package', /* required */ true)!;

    // Get the certificate.
    const certificateType: string | undefined = tl.getInput('certificateType');
    var signingType: SigningType;
    if (certificateType == 'base64')
    {
        tl.debug('Using base64-encoded certificate');
        signingType = new Base64EncodedCertSigningType();
    }
    else
    {
        tl.debug('Using secure file certificate');
        signingType = new SecureFileSigningType();
    }

    // No time stamp server means to not add a time stamp.
    const timeStampServer: string | undefined = tl.getInput('timeStampServer');

    // Resolve the package paths.
    const packagesToSign: string[] = tl.findMatch(/* defaultRoot */ '', packagePathPattern);

    try
    {
        await signingType.prepareCert();

        // Sign the packages
        for (const packagePath of packagesToSign)
        {
            tl.debug('signing ' + packagePath);
            await signPackage(packagePath, signingType, timeStampServer);
        }
    }
    finally
    {
        signingType.cleanupCert();
    }
}

run().catch(err =>
    {
        tl.setResult(tl.TaskResult.Failed, err.message);
    })