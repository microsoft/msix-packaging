import testHelpers = require('./testhelpers');

describe('MSIX signing task tests', function ()
{
    it('Should succeed with basic inputs', function (done: Mocha.Done)
    {
        const testRunner = testHelpers.runMockTest('signing-basic-success.js');
        testHelpers.assertTestRunnerSucceeded(testRunner);
        done();
    });

    it('Should succeed signing with timestamp', function (done: Mocha.Done)
    {
        const testRunner = testHelpers.runMockTest('signing-with-timestamp-success.js');
        testHelpers.assertTestRunnerSucceeded(testRunner);
        done();
    });
});