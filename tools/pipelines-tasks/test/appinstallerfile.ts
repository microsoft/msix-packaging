import assert = require('assert');
import childProcess = require('child_process');
import fs = require('fs');
import ttm = require('azure-pipelines-task-lib/mock-test');

import testHelpers = require('./testhelpers');

const runTestAndVerifyFileIsAsExpected = (testName: string) =>
{
    const testRunner: ttm.MockTestRunner = testHelpers.runMockTest(testName + '.js');

    testHelpers.assertTestRunnerSucceeded(testRunner);
    testHelpers.assertOutputFileExists(testName + '.appinstaller');

    const createdFilePath = testHelpers.outputFilePath(testName + '.appinstaller');
    const expectedFilePath = testHelpers.expectedFilePath(testName + '.appinstaller');

    // Using Compare-Object instead of comparing the files ourselves
    // prevents errors due to mismatched line endings.
    const diff = childProcess.execSync(`powershell Compare-Object (Get-Content ${createdFilePath}) (Get-Content ${createdFilePath})`).toString();
    assert.strictEqual(diff, '', `There should be no difference between the expected .appinstaller '${expectedFilePath}' and created .appinstaller '${createdFilePath}'.`);
}

describe('App Installer file task tests', function ()
{
    it('Should succeed creating a new App Installer file from a bundle', function (done: Mocha.Done)
    {
        runTestAndVerifyFileIsAsExpected('appinstallerfile-new-from-bundle-success');
        done();
    });

    it('Should succeed updating an exsiting App Installer file from a bundle', function (done: Mocha.Done)
    {
        runTestAndVerifyFileIsAsExpected('appinstallerfile-update-from-bundle-success');
        done();
    });

    it('Should succeed creating a new App Installer file from a package', function (done: Mocha.Done)
    {
        runTestAndVerifyFileIsAsExpected('appinstallerfile-new-from-package-success');
        done();
    });

    it('Should succeed updating an existing App Installer file from a package', function (done: Mocha.Done)
    {
        runTestAndVerifyFileIsAsExpected('appinstallerfile-update-from-package-success');
        done();
    });
});