import assert = require('assert');
import ttm = require('azure-pipelines-task-lib/mock-test');

import testHelpers = require('./testhelpers');

describe('MSIX app attach task tests', function ()
{
    it('Should succeed with basic inputs', function (done: Mocha.Done)
    {
        const testRunner: ttm.MockTestRunner = testHelpers.runMockTest('appattach-success.js');
        testHelpers.assertTestRunnerSucceeded(testRunner);
        assert.strictEqual(testHelpers.outputFileExists('TestVhdx.vhdx'), true, 'The VHDX should have been created');
        done();
    });
});