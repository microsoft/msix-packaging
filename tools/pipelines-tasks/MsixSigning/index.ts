import path = require('path');
import tl = require('azure-pipelines-task-lib/task')
import { ToolRunner } from 'azure-pipelines-task-lib/toolrunner';

import helpers = require('common/helpers');

import download = require('./predownloadsecurefile');

const SIGNTOOL_PATH = path.join(__dirname, 'lib', 'signtool');
const IMPORT_CERT_SCRIPT_PATH = path.join(__dirname, 'ImportCert.ps1');

/**
 * Definition of how to sign with a kind of certificate (e.g. file or file encoded as string).
 * Implementations of this should read the needed task inputs during construction.
 */
interface SigningType
{
    /**
     * Prepares the certificate to use.
     */
    prepareCert(): Promise<void>,
    /**
     * Does any cleanup needed for the certificate.
     */
    cleanupCert(): void,
    /**
     * Adds the signtool.exe arguments that specify which certificate to use.
     */
    addSignToolCertOptions(signtoolRunner: ToolRunner): void
}

/**
 * Sign with a .pfx file downloaded from the pipeline's secure files.
 */
class SecureFileSigningType implements SigningType
{
    // ID of the secure .pfx file to download.
    secureFileId: string;
    // Password to the .pfx.
    password?: string;
    // Path to the downloaded file
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

    // Download the pfx file
    async prepareCert(): Promise<void>
    {
        this.certFilePath = await download.downloadSecureFile(this.secureFileId);
    }

    // Delete the downloaded file
    cleanupCert()
    {
        download.deleteSecureFile(this.secureFileId);
    }

    // Pass the cert path and password to signtool
    addSignToolCertOptions(signtoolRunner: ToolRunner): void
    {
        signtoolRunner.arg(['/f', this.certFilePath!]);
        if (this.password)
        {
            signtoolRunner.arg(['/p', this.password]);
        }
    }
}

/**
 * Sign with a pfx encoded as a string, as downloaded from Azure Key Vault
 */
class Base64EncodedCertSigningType implements SigningType
{
    // Certificate encoded as a string
    base64String: string;
    // Certificate hash/thumbprint for identification
    certThumbprint?: string;

    constructor()
    {
        this.base64String = tl.getInput('encodedCertificate', /* required */ true)!;
    }

    // Import the certificate into the cert store and get its thumbprint
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

    // Remove the certificate from the cert store
    cleanupCert()
    {
        const powershellRunner = helpers.getPowershellRunner(IMPORT_CERT_SCRIPT_PATH);
        powershellRunner.arg(this.base64String);
        powershellRunner.arg('-remove');
        powershellRunner.execSync();
    }

    // Pass the cert thumbprint to signtool
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

    // Get the certificate info.
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