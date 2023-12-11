import tl = require('azure-pipelines-task-lib/task');
import { ToolRunner } from 'azure-pipelines-task-lib/toolrunner';
import path = require('path');
import armStorage = require('azure-pipelines-tasks-azure-arm-rest/azure-arm-storage');
import { AzureRMEndpoint } from 'azure-pipelines-tasks-azure-arm-rest/azure-arm-endpoint';
import { AzureEndpoint, StorageAccount } from 'azure-pipelines-tasks-azure-arm-rest/azureModels';
import * as telemetry from "azure-pipelines-tasks-utility-common/telemetry";
import * as os from 'os';
import * as url from 'url';
import * as fs from 'fs';
import helpers = require('common-helpers/helpers');

const HELPER_SCRIPT = path.join(__dirname, 'publishAVD.ps1');
const TARGET_DLL = path.join(__dirname, '/AppAttachFrameworkDLL/AppAttachKernel.dll');
const AppAttachFrameworkDll = path.join(__dirname, '/AppAttachFrameworkDLL');

const HELPER_SCRIPT_EXCEPTIONS_TELEMETRY = path.join(__dirname, 'ReportExceptionTelemetry.ps1');
const TARGET_DLL_TELEMETRY = path.join(__dirname, '/AppAttachFrameworkDLL/AppAttachTelemetry.dll');

function isNonEmpty(str: string): boolean {
    return (!!str && !!str.trim());
}

function getResourceGroupNameFromUri(resourceUri: string): string {
	if (isNonEmpty(resourceUri)) {
		const parsedUrl = url.parse(resourceUri, true);

		const pathname = parsedUrl.pathname || '';
		const segments = pathname.split('/');

		const resourceGroupIndex = segments.indexOf('resourceGroups');

		if (resourceGroupIndex !== -1 && resourceGroupIndex < segments.length - 1) {
			return segments[resourceGroupIndex + 1];
		}
	}

    return "";
}

async function run(): Promise<void> {
	let isSuccessful = true;
	try {
		await helpers.installNuget(helpers.APPATTACH_FRAMEWORK_NUPKG_DIR, AppAttachFrameworkDll, 'net472');

		let connectedServiceName = tl.getInput('connectedServiceNameARM', true)!;

		let subscriptionID: string = tl.getEndpointDataParameter(connectedServiceName, 'subscriptionid', true)!;

		let storageAccountName = tl.getInput('storageAccount', true)!;
		var azureEndpoint: AzureEndpoint = await new AzureRMEndpoint(connectedServiceName).getEndpoint();
		let accessToken: string = (await azureEndpoint.applicationTokenCredentials.getToken()).toString();

		const storageArmClient = new armStorage.StorageManagementClient(azureEndpoint.applicationTokenCredentials, (azureEndpoint.subscriptionID)!);
		let storageAccount: StorageAccount = await storageArmClient.storageAccounts.get(storageAccountName);
		let storageAccountResourceGroupName = getResourceGroupNameFromUri(storageAccount.id);

		let accessKeys = await storageArmClient.storageAccounts.listKeys(storageAccountResourceGroupName, storageAccountName, null);
		let accessKey: string = accessKeys[0];
		let StorageAccountConnectionString: string = "DefaultEndpointsProtocol=https;AccountName=" + storageAccountName + ";AccountKey=" + accessKey
			+ ";EndpointSuffix=core.windows.net";
		let username: string = (process.env.BUILD_REQUESTEDFOREMAIL)!;
		const PUBLISH_TO_AZURE_TASK = '8';

		const appattachConfig = {
			'emailId': username,
			'accessToken': accessToken,
			'azureStorageKey': StorageAccountConnectionString,
			'azureWorkspace': tl.getInput('workSpace', true),
			'azureStorageFileShare': tl.getInput('fileShare', true),
			'azureStorageAccount': storageAccountName,
			'azureSubscriptionId': subscriptionID,
			'azureResourceGroup': tl.getInput('resourceGroupName', true),
			'azureHostPoolName': tl.getInput('hostPool', true),
			'azureApplicationGroupName': tl.getInput('applicationGroup', true),
			'taskType': PUBLISH_TO_AZURE_TASK,
			'appAttachImagePath': tl.getInput('vhdxPath', true),
			'clientType': helpers.CLIENT_TYPE,
			'clientVersion': helpers.CLIENT_VERSION
		};

		const jsonString = JSON.stringify(appattachConfig);
		tl.debug(jsonString);

		const powershellRunner: ToolRunner = helpers.getPowershellRunner(HELPER_SCRIPT);
		powershellRunner.arg(['-inputJsonStr', '\'' + jsonString + '\'']);
		powershellRunner.arg(['-targetDLL', TARGET_DLL]);

		const execResult = await powershellRunner.execSync();

		if (execResult.code) {
			throw execResult.stderr;
		}

		// display log file content to console
		const appAttachLogDir: string = path.join(os.tmpdir(), 'AppAttach');
		const appAttachLogDirFiles = fs.readdirSync(appAttachLogDir);
		const appAttachlogFile = path.join(appAttachLogDir, appAttachLogDirFiles[0]);
		fs.readFile(appAttachlogFile, 'utf8', (err, data) => {
			if (err) {
				console.error('Error reading the file:', err);
			} else {
				console.log(appAttachlogFile, data);
			}
		});
	} catch (error) {
		isSuccessful = false;
		console.log(tl.loc("AppAttachPublish Error", error));
		const powershellRunner: ToolRunner = helpers.getPowershellRunner(HELPER_SCRIPT_EXCEPTIONS_TELEMETRY);
		powershellRunner.arg(['-exceptionMessage', "Test Exception Message"]);
		powershellRunner.arg(['-targetDLL', TARGET_DLL_TELEMETRY]);
		powershellRunner.arg(['-clientType', helpers.CLIENT_TYPE]);
		powershellRunner.arg(['-clientVersion', helpers.CLIENT_VERSION]);

		await powershellRunner.execSync();
	} finally {
		logTelemetry({
			Version: helpers.CLIENT_VERSION,
			AppAttachImagePath: tl.getInput('vhdxPath', true),
			IsSuccessful: isSuccessful
		});
	}
}

function logTelemetry(params: any) {
	try {
		telemetry.emitTelemetry("MSIX-Packaging", "AVDAppAttachPublish", params);
	} catch (err) {
		tl.debug(`Unable to log AVDAppAttachPublish task telemetry. Err:( ${err} )`);
	}
}

run().catch(err =>
    {
        tl.setResult(tl.TaskResult.Failed, err.message);
    })