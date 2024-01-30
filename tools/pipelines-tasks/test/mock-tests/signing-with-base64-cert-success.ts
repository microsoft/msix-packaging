import tmrm = require('azure-pipelines-task-lib/mock-run');
import fs = require('fs');
import testHelpers = require('../testhelpers');
import { Buffer } from 'buffer';

const taskMockRunner = new tmrm.TaskMockRunner(testHelpers.TaskEntryPoints.SigningTask);

//const encodedCertificate = fs.readFileSync(testHelpers.testEncodedCertificate, { encoding: 'utf-8' }).toString().trim();
const base64String = fs.readFileSync(testHelpers.testEncodedCertificate, 'base64').toString().trim();
const encodedCertificate = Buffer.from(base64String, 'base64').toString('utf-8');
taskMockRunner.setInput('package', testHelpers.testPackage);
taskMockRunner.setInput('certificateType', 'base64');
taskMockRunner.setInput('encodedCertificate', encodedCertificate);

// Don't mock call to signtool
taskMockRunner.run(/* noMockTask */ true);