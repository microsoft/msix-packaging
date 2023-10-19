import { ToolRunner } from 'azure-pipelines-task-lib/toolrunner';
import tl = require('azure-pipelines-task-lib/task');
import path = require('path');
import armStorage = require('azure-pipelines-tasks-azure-arm-rest/azure-arm-storage');
import { AzureRMEndpoint } from 'azure-pipelines-tasks-azure-arm-rest/azure-arm-endpoint';
import { AzureEndpoint, StorageAccount } from 'azure-pipelines-tasks-azure-arm-rest/azureModels';
import * as os from 'os';

const HELPER_SCRIPT = path.join(__dirname, 'publishAVD.ps1');
const TARGET_DLL = path.join(__dirname, '/AppAttachFrameworkDLL/AppAttachKernel.dll');

function isNonEmpty(str: string): boolean {
    return (!!str && !!str.trim());
}

function getResourceGroupNameFromUri(resourceUri: string): string {
    if (isNonEmpty(resourceUri)) {
        resourceUri = resourceUri.toLowerCase();
        return resourceUri.substring(resourceUri.indexOf("resourcegroups/") + "resourcegroups/".length, resourceUri.indexOf("/providers"));
    }

    return "";
}

const getPowershellRunner = (scriptPath: string): ToolRunner => {
	const powershellRunner: ToolRunner = tl.tool('powershell')
		.arg('-NoLogo')
		.arg('-NoProfile')
		.arg('-NonInteractive')
		.arg(['-ExecutionPolicy', 'Unrestricted']);

	// Quote the script path to allow for spaces.
	// Existing quotes need to be escaped.
	powershellRunner.arg(`& '${scriptPath.replace("'", "''")}'`);
	return powershellRunner;
}

async function run(): Promise<void> {
    try {
		let connectedServiceName = tl.getInput('ConnectedServiceNameARM', true)!;

		let subscriptionID: string = tl.getEndpointDataParameter(connectedServiceName, 'subscriptionid', true)!;

		let storageAccountName = tl.getInput('StorageAccount', true)!;
		var azureEndpoint: AzureEndpoint = await new AzureRMEndpoint(connectedServiceName).getEndpoint();
		let accessToken: string = (await azureEndpoint.applicationTokenCredentials.getToken()).toString();

		const storageArmClient = new armStorage.StorageManagementClient(azureEndpoint.applicationTokenCredentials, (azureEndpoint.subscriptionID)!);
		let storageAccount: StorageAccount = await storageArmClient.storageAccounts.get(storageAccountName);
		let storageAccountResourceGroupName = getResourceGroupNameFromUri(storageAccount.id);
		let accessKeys = await storageArmClient.storageAccounts.listKeys(storageAccountResourceGroupName, storageAccountName, null);
		let accessKey: string = accessKeys[0];
		let StorageAccountConnectionString: string = "DefaultEndpointsProtocol=https;AccountName="+storageAccountName+";AccountKey="+accessKey
														+";EndpointSuffix=core.windows.net";
		let username: string = (process.env.BUILD_REQUESTEDFOREMAIL)!;
		const PUBLISH_TO_AZURE_TASK = '8';
		
		const myObject = {
			'emailId': username, 'accessToken': accessToken,
			'azureStorageKey': StorageAccountConnectionString,
			'azureWorkspace': tl.getInput('WorkSpace', true),
			'azureStorageFileShare': tl.getInput('FileShare', true),
			'azureStorageAccount': tl.getInput('StorageAccount', true),
			'azureSubscriptionId': subscriptionID,
			'azureResourceGroup': tl.getInput('ResourceGroupName', true),
			'azureHostPoolName': tl.getInput('HostPool', true),
			'azureApplicationGroupName': tl.getInput('ApplicationGroup', true),
			'taskType': PUBLISH_TO_AZURE_TASK, 'appAttachImagePath': tl.getInput('VHDXPath', true)
		};

		const jsonString = JSON.stringify(myObject);	
		console.log(jsonString);

		const powershellRunner: ToolRunner = getPowershellRunner(HELPER_SCRIPT);
		powershellRunner.arg(['-inputJsonStr', '\'' + jsonString + '\'']);
		powershellRunner.arg(['-targetDLL', TARGET_DLL]);

		const execResult = await powershellRunner.execSync();

		// copy output log folder to artifact folder
		let artifactFolder: string = (process.env.BUILD_ARTIFACTSTAGINGDIRECTORY)!; 
		console.log("artifactFolder" + artifactFolder);
		const appAttachLogDir: string = path.join(os.tmpdir(), 'AppAttach');
		console.log("appAttachLogDir" + appAttachLogDir);
		tl.cp(appAttachLogDir, artifactFolder, '-rf', true);

		if (execResult.code) {
			throw execResult.stderr;
		}
    } catch(error) {
        console.log(tl.loc("AppAttachPublish Error", error));
    }
}

run().catch(err =>
    {
        tl.setResult(tl.TaskResult.Failed, err.message);
    })