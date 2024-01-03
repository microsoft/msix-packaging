import assert = require('assert');
import ttm = require('azure-pipelines-task-lib/mock-test');
import fs = require('fs');

import testHelpers = require('./testhelpers');

describe('AVD publish task tests', function () {
    it('Should succeed with basic inputs', function (done: Mocha.Done) {
        const testRunner: ttm.MockTestRunner = testHelpers.runMockTest('avdpublish-success.js');
        const fileContent = fs.readFileSync(testHelpers.avdInputsJsonPath, 'utf-8');
        const jsonData = JSON.parse(fileContent);

        const subscriptionId: string = jsonData["connectedServiceNameARM"];
        const clientId: string = jsonData["clientId"];
        const clientSecret: string = jsonData["clientSecret"];
        const tenantId: string = jsonData["tenantId"];

        if (!subscriptionId.trim() || !clientId.trim() || !clientSecret.trim() || !tenantId.trim()) {
            console.log("WARNING: Service connection parameters are not set. Please set service connection parameters are set in avdinputs.json or as pipeline variable when running through pipeline.");
        }
        else {
            testHelpers.assertTestRunnerSucceeded(testRunner);
        }

        done();
    });

    it('Should fail because VHDX path is not set', function (done: Mocha.Done) {
        const testRunner: ttm.MockTestRunner = testHelpers.runMockTest('avdpublish-no-vhdxpath-failure.js');
        testHelpers.assertTestRunnerFailedWithMessage(testRunner, 'Error: Input required: vhdxPath');
        done();
    });
});