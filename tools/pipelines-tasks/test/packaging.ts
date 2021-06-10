import assert = require('assert');
import childProcess = require('child_process');
import fs = require('fs');
import os = require('os');
import path = require('path');
import ttm = require('azure-pipelines-task-lib/mock-test');

import testHelpers = require('./testhelpers');

import helpers = require('../common/helpers');

const runTestAndVerifyPackageIsCreated = (testName: string, bundle: boolean = false) =>
{
    const testRunner: ttm.MockTestRunner = testHelpers.runMockTest(testName + '.js');

    testHelpers.assertTestRunnerSucceeded(testRunner);
    testHelpers.assertOutputFileExists(testName + (bundle ? '.msixbundle' : '.msix'));
}

const assertDirectoriesHaveSameFiles = (actualDirectory: string, expectedDirectory: string) =>
{
    const expectedFiles: string[] = fs.readdirSync(expectedDirectory);
    const actualFiles: Set<string> = new Set(fs.readdirSync(actualDirectory));

    for (const file of expectedFiles)
    {
        assert.strictEqual(actualFiles.has(file), true, `Expected file ${file} not found in ${actualDirectory}`);

        const expectedFile = path.join(expectedDirectory, file);
        const actualFile = path.join(actualDirectory, file);
        assert.strictEqual(fs.statSync(actualFile).isDirectory(), fs.statSync(expectedFile).isDirectory(), 'Actual file should be a directory if expected file is a directory');

        if (fs.statSync(expectedFile).isDirectory())
        {
            assertDirectoriesHaveSameFiles(actualFile, expectedFile);
        }
    }
}

describe('MSIX packaging task tests', function ()
{
    before(function()
    {
        // Delete all existing packages in the output directory
        try
        {
            for (const file of fs.readdirSync(testHelpers.outputDirectory))
            {
                if (file.endsWith('.msix') || file.endsWith('.msixbundle'))
                {
                    fs.unlinkSync(file);
                }
            }
        }
        catch
        {
            // Ignore errors here
        }
    });

    it('Should succeed with basic inputs for an MSBuild run', function (done: Mocha.Done)
    {
        runTestAndVerifyPackageIsCreated('build-basic-success');
        done();
    });

    it('Should succeed with app version update and MSBuild run', async function ()
    {
        runTestAndVerifyPackageIsCreated('build-update-app-version-success');

        const newManifest: any = await helpers.parseXml(testHelpers.testProjectManifest);
        const newVersion: string = newManifest.Package.Identity[0].$.Version;

        assert.strictEqual('1.2.3.4', newVersion, 'App version was updated in the manifest');
    });

    it('Should succeed with MSBuild with bundle option', function (done: Mocha.Done)
    {
        runTestAndVerifyPackageIsCreated('build-bundle-success', /* bundle */ true);
        done();
    });

    it('Should fail with MSBuild because no solution was given', function (done: Mocha.Done)
    {
        const testRunner: ttm.MockTestRunner = testHelpers.runMockTest('build-no-solution-failure');
        testHelpers.assertTestRunnerFailedWithMessage(testRunner, 'Input Error: A path to a solution is required.');
        testHelpers.assertOutputFileDoesNotExist('App.msix');
        done();
    });

    it('Should fail with MSBuild because no platform was given', function (done: Mocha.Done)
    {
        const testRunner: ttm.MockTestRunner = testHelpers.runMockTest('build-no-platform-failure.js');
        testHelpers.assertTestRunnerFailedWithMessage(testRunner, 'Input Error: Platform to build is required.');
        testHelpers.assertOutputFileDoesNotExist('App.msix');
        done();
    });

    it('Should succeed with just packaging binaries functionality', function (done: Mocha.Done)
    {
        const testName: string = 'package-binaries-basic-success';
        runTestAndVerifyPackageIsCreated(testName);

        // Unpack package and verify it contains the expected files.
        const makeappx: string = path.resolve(path.join(__dirname, '..', 'common', 'lib', 'makeappx'));
        const extractDirectory = path.join(testHelpers.tempDirectory, 'package-binaries-basic-success-extracted');

        const unpackCommand = `${makeappx} unpack -p ${testHelpers.outputFilePath(testName + '.msix')} -d ${extractDirectory} -o`;
        childProcess.execSync(unpackCommand);

        assertDirectoriesHaveSameFiles(extractDirectory, testHelpers.testPrebuiltDirectory);
        done();
    });

    it('Should succeed with bundling input directory', function (done: Mocha.Done)
    {
        runTestAndVerifyPackageIsCreated('package-binaries-bundle-success', /* bundle */ true);
        done();
    });
});