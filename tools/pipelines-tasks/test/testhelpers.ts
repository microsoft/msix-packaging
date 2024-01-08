import assert = require('assert');
import tmrm = require('azure-pipelines-task-lib/mock-run');
import ttm = require('azure-pipelines-task-lib/mock-test');
import fs = require('fs');
import os = require('os');
import path = require('path');
import { tempdir } from 'shelljs';

/* Paths to test collaterals */

// Path to the directory containing the test collateral
export const assetsDirectory = path.join(__dirname, 'assets');

// Path to the solution file of a project to use for testing
export const testProjectSolution = path.join(assetsDirectory, 'HelloWorldUWPApp', 'HelloWorldApp.sln');

// Path to the manifest of the test project
export const testProjectManifest = path.join(assetsDirectory, 'HelloWorldUWPApp', 'Package.appxmanifest');

// Path to a directory with prebuilt binaries ready to package.
export const testPrebuiltDirectory = path.join(assetsDirectory, 'PrebuiltBinaries');

// Path to a valid package to use for testing
export const testPackage = path.join(assetsDirectory, 'existingPackage.msix');

// Path to a valid bundle to use for testing
export const testBundle = path.join(assetsDirectory, 'existingBundle.msixbundle');

// Path to a test certificate.
// This is not tracked by git and is created by the build scripts.
export const testCertificate = path.join(assetsDirectory, 'certificate.pfx');

// Path to a test certificate encoded as a bas64 string.
// This is not tracked by git and is created by the build scripts.
export const testEncodedCertificate = path.join(assetsDirectory, 'certificate.txt');

// Directory with the expected files to be created
export const expectedResultsDirectory = path.join(assetsDirectory, 'expected');

/* Helpers for output files */

export const tempDirectory = path.join(os.tmpdir(), 'MsixTasksTests');

// Directory used for the task's outputs
export const outputDirectory = path.join(tempDirectory, 'output');

// Path to avdinputs file
export const avdInputsJsonPath = path.join(assetsDirectory, 'avdinputs.json');

export const outputFilePath = (fileName: string): string =>
{
    return path.join(outputDirectory, fileName);
}

export const expectedFilePath = (fileName: string): string =>
{
    return path.join(expectedResultsDirectory, fileName);
}

/* Helpers for task setup/running */
export const TaskEntryPoints = {
    AppAttachTask: path.join(__dirname, '..', 'MsixAppAttach', 'index.js'),
    AppInstallerTask: path.join(__dirname, '..', 'AppInstallerFile', 'index.js'),
    AVDAppAttachPublishTask: path.join(__dirname, '..', 'AVDAppAttachPublish', 'main.js'),
    PackagingTask: path.join(__dirname, '..', 'MsixPackaging', 'index.js'),
    SigningTask: path.join(__dirname, '..', 'MsixSigning', 'index.js'),
}

/**
 * Runs an end to end test of the task.
 * @param testFileName Name of the file inside mock-tests with the test scenario.
 * This must be the compiled .js, not the source .ts.
 * @returns the MockTestRunner used to run the test.
 */
export const runMockTest = (testFileName: string): ttm.MockTestRunner =>
{
    const testPath: string = path.join(__dirname, 'mock-tests', testFileName);
    const testRunner: ttm.MockTestRunner = new ttm.MockTestRunner(testPath);

    testRunner.run();
    return testRunner;
}

export const setUpSecureFileDownloadMock = (taskMockRunner: tmrm.TaskMockRunner): void =>
{
    const downloadSecureFileMock = require('./predownloadsecurefile-mock');
    taskMockRunner.registerMock('./predownloadsecurefile', downloadSecureFileMock);
}

export const setUpCertificateInputs = (taskMockRunner: tmrm.TaskMockRunner) =>
{
    setUpSecureFileDownloadMock(taskMockRunner);
    taskMockRunner.setInput('certificate', testCertificate);

    // passwordVariable is the name of the secret variable that the user would set on the pipeline
    // with the certificate password. In this case the password variable 'helloworld' is the
    // secret variable with the password.
    taskMockRunner.setInput('passwordVariable', 'helloworld');
    taskMockRunner.setVariableName('helloworld', 'password', /* isSecret */ true)
}

export const setUpMSBuildLocationArguments = (taskMockRunner: tmrm.TaskMockRunner) =>
{
    taskMockRunner.setInput('msbuildLocationMethod', 'version');
    taskMockRunner.setInput('msbuildVersion', 'latest');
    taskMockRunner.setInput('msbuildArchitecture', 'x86');
}

/**
 * Set up the basic arguments for the build task.
 */
export const setUpBasicMSBuildArguments = (taskMockRunner: tmrm.TaskMockRunner, testFile: string, bundle: boolean = false): void =>
{
    const testName: string = path.basename(testFile, '.js');
    taskMockRunner.setInput('outputPath', outputFilePath(testName + (bundle ? '.msixbundle' : '.msix')));
    taskMockRunner.setInput('solution', testProjectSolution);
    taskMockRunner.setInput('buildSolution', 'true');
    taskMockRunner.setInput('buildConfiguration', 'release');

    if (bundle)
    {
        taskMockRunner.setInput('generateBundle', 'true');
        taskMockRunner.setInput('buildForX86', 'true');
        taskMockRunner.setInput('buildForX64', 'true');
    }
    else
    {
        taskMockRunner.setInput('generateBundle', 'false');
        taskMockRunner.setInput('buildPlatform', 'x86');
    }

    setUpMSBuildLocationArguments(taskMockRunner);
}

/**
 * Set up the basic arguments to package without building.
 */
export const setUpBasicPackageBinariesArguments = (taskMockRunner: tmrm.TaskMockRunner, testFile: string, bundle: boolean = false): void =>
{
    const testName: string = path.basename(testFile, '.js');
    taskMockRunner.setInput('outputPath', outputFilePath(testName + (bundle ? '.msixbundle' : '.msix')));
    taskMockRunner.setInput('buildSolution', 'false');

    if (bundle)
    {
        const bundleInputPath = path.join(tempDirectory, 'BundleInput');
        fs.mkdirSync(bundleInputPath, { recursive: true });
        fs.copyFileSync(testPackage, path.join(bundleInputPath, 'package.msix'));
        taskMockRunner.setInput('inputDirectory', testPrebuiltDirectory);
    }
    else
    {
        taskMockRunner.setInput('inputDirectory', testPrebuiltDirectory);
    }
}

export const setUpNewAppInstallerFileArguments = (taskMockRunner: tmrm.TaskMockRunner, testFile: string, packagePath: string): void =>
{
    const testName: string = path.basename(testFile, '.js');
    taskMockRunner.setInput('package', packagePath);
    taskMockRunner.setInput('outputPath', outputFilePath(testName + '.appinstaller'));

    taskMockRunner.setInput('method', 'create');
    taskMockRunner.setInput('uri', `https://example.com/${testName}`);
    taskMockRunner.setInput('fileVersion', '1.0.0.0');
    taskMockRunner.setInput('mainItemUri', path.basename(packagePath));

    // here, we simply test that the added dependencies + optionals will be formatted correctly in the installer
    // file rather than it actually working in the installer since we won't be running the actual installer as
    // part of the test; our app also doesn't need any dependencies + optionals either
    taskMockRunner.setInput('addOptionalItem1', 'package');
    taskMockRunner.setInput('optionalItem1Name', 'Goodbye World App');
    taskMockRunner.setInput('optionalItem1Publisher', 'CN=Goodbye World Inc.');
    taskMockRunner.setInput('optionalItem1Version', '2.2.3.3');
    taskMockRunner.setInput('optionalItem1ProcessorArchitecture', 'x86');
    taskMockRunner.setInput('optionalItem1URI', 'http://goodbyeworld.com/install');

    taskMockRunner.setInput('addDependency1', 'bundle');
    taskMockRunner.setInput('dependency1Name', 'Visual Studio');
    taskMockRunner.setInput('dependency1Publisher', 'CN=Microsoft');
    taskMockRunner.setInput('dependency1Version', '10.37.96.549');
    taskMockRunner.setInput('dependency1URI', 'https://visualstudio.microsoft.com/thank-you-downloading-visual-studio/?sku=Community&rel=16');
}

export const setUpUpdateAppInstallerFileArguments = (taskMockRunner: tmrm.TaskMockRunner, testFile: string, packagePath: string): void =>
{
    const testName: string = path.basename(testFile, '.js');
    taskMockRunner.setInput('package', packagePath);
    taskMockRunner.setInput('outputPath', outputFilePath(testName + '.appinstaller'));

    taskMockRunner.setInput('method', 'update');
    taskMockRunner.setInput('existingFile', path.join(assetsDirectory, `existing-${testName}.appinstaller`));
    taskMockRunner.setInput('versionUpdateMethod', 'revision');
}

/**
 * sets up mock for azure ARM library functions
 * @param taskMockRunner input taskMockRunner to register mock functions
 */
export const setUpAzureArmRestMock = (taskMockRunner: tmrm.TaskMockRunner): void => {
    const azureArmRestMock = require('./azure-arm-rest-mock');
    taskMockRunner.registerMock("azure-pipelines-tasks-azure-arm-rest/azure-arm-endpoint", azureArmRestMock);
    taskMockRunner.registerMock("azure-pipelines-tasks-azure-arm-rest/azureModels", azureArmRestMock);
    taskMockRunner.registerMock("azure-pipelines-tasks-azure-arm-rest/azure-arm-common", azureArmRestMock);
    taskMockRunner.registerMock("azure-pipelines-tasks-azure-arm-rest/azure-arm-storage", azureArmRestMock);
}

/**
 * sets up mock for getEndpointDataParameter in azure pipelines task lib
 * @param taskMockRunner input taskMockRunner to register mock functions
 * @param subscriptionId subscription ID to be used to get end Point
 */
export const setUpEndPointDataParamMock = (taskMockRunner: tmrm.TaskMockRunner, subscriptionId: string): void =>
{
    const tl = require('azure-pipelines-task-lib/task');
    const tlClone = Object.assign({}, tl);

    tlClone.getEndpointDataParameter = function (id: string, key: string, optional: boolean) {
        key = key.toUpperCase();
        if (key == 'SUBSCRIPTIONID') {
            return subscriptionId;
        }
        throw new Error(`Endpoint auth data not present: ${id}`);
    }
    taskMockRunner.registerMock('azure-pipelines-task-lib/task', tlClone);

}

/**
 * sets up mock for getEndpointDataParameter in azure pipelines task lib
 * @param taskMockRunner input taskMockRunner to register mock functions
 */
export const setUpAvdPublishInputs = (taskMockRunner: tmrm.TaskMockRunner): boolean =>
{
    const fileContent = fs.readFileSync(avdInputsJsonPath, 'utf-8');
    const jsonData = JSON.parse(fileContent);

    // TestVhdx.vhdx is created from appattach-success test and is input to this avd publish task
    taskMockRunner.setInput("vhdxPath", path.join(outputDirectory, 'TestVhdx.vhdx')); 
    const inputs = [
        "connectedServiceNameARM",
        "resourceGroupName",
        "storageAccount",
        "fileShare",
        "hostPool",
        "workSpace",
        "applicationGroup"
    ];

    for (const input of inputs) {
        taskMockRunner.setInput(input, jsonData[input]);
    }
    process.env.BUILD_REQUESTEDFOREMAIL = jsonData["email"];

    setUpEndPointDataParamMock(taskMockRunner, jsonData["connectedServiceNameARM"]);
    setUpAzureArmRestMock(taskMockRunner);
    return true;
}

/* Helpers for validating task results */
export const assertTestRunnerSucceeded = (testRunner: ttm.MockTestRunner) =>
{
    // Extra output for debugging failures
    if (testRunner.stderr.length > 0 || testRunner.errorIssues.length > 0 || !testRunner.succeeded)
    {
        console.log('Test output:\n' + testRunner.stdout);
    }

    assert.strictEqual(testRunner.stderr.length, 0, 'Nothing should have been written to stderr, got:\n' + testRunner.stderr);
    assert.strictEqual(testRunner.errorIssues.length, 0, 'Should have gotten 0 errors, got:\n' + testRunner.errorIssues.join('\n'));
    assert.strictEqual(testRunner.succeeded, true, 'The task should have succeeded');
}

export const assertTestRunnerFailedWithMessage = (testRunner: ttm.MockTestRunner, expectedError: string) =>
{
    assert.strictEqual(testRunner.errorIssues.length, 1, 'Should have gotten 1 error\nErrors: ' + testRunner.errorIssues);
    assert.strictEqual(testRunner.errorIssues[0], expectedError, 'Task failed with unexpected error message');
    assert.strictEqual(testRunner.succeeded, false, 'The task should have failed');
}

export const outputFileExists = (fileName: string): boolean =>
{
    return fs.existsSync(path.join(outputDirectory, fileName));
}

export const assertOutputFileExists = (fileName: string) =>
{
    assert.strictEqual(outputFileExists(fileName), true, `File ${fileName} should have been created.`);
}

export const assertOutputFileDoesNotExist = (fileName: string) =>
{
    assert.strictEqual(outputFileExists(fileName), false, `File ${fileName} should not have been created.`);
}