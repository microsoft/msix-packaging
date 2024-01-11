import assert = require('assert');
import ttm = require('azure-pipelines-task-lib/mock-test');
import fs = require('fs');

import testHelpers = require('./testhelpers');

describe('AVD publish task tests', function () {
    it('Should succeed with basic inputs', function (done: Mocha.Done) {
        let subscriptionId: string = '';
        let clientId: string = '';
        let clientSecret: string = '';
        let tenantId: string = '';
        const testRunner: ttm.MockTestRunner = testHelpers.runMockTest('avdpublish-success.js');
        try {
            const fileContent = fs.readFileSync(testHelpers.avdInputsJsonPath, 'utf-8');
            const jsonData = JSON.parse(fileContent);

            subscriptionId= jsonData["connectedServiceNameARM"];
            clientId = jsonData["clientId"];
            clientSecret = jsonData["clientSecret"];
            tenantId = jsonData["tenantId"];
        }
        catch (error) {
            console.error('AVDPublish test error: ', error);
            done(new Error('Reading/Parsing avdinputs.json failed'));
        }

        if (!subscriptionId.trim() || !clientId.trim() || !clientSecret.trim() || !tenantId.trim()) {
            console.warn("WARNING: Service connection parameters are not set. Please set service connection parameters in avdinputs.json or as pipeline variable (when running CI pipeline)");
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
