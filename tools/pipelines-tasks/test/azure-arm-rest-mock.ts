import path = require('path');
import fs = require('fs');
import { ClientSecretCredential } from "@azure/identity";
import { TokenCredential } from "@azure/core-auth";
import { StorageManagementClient as CustomStorageClient}  from '@azure/arm-storage';

let bearerAccessToken: string | undefined;
let credential: TokenCredential;

export class AzureRMEndpoint {
    private _connectedServiceName: any;
    private endpoint: any;
    public subscriptionID: string;
    constructor(connectedServiceName: any) {
        this._connectedServiceName = connectedServiceName;
        this.endpoint = new AzureEndpoint();
        this.subscriptionID = connectedServiceName;
    }

    getEndpoint() {
        this.endpoint.applicationTokenCredentials = new ApplicationTokenCredentials();
        this.endpoint.subscriptionID = this._connectedServiceName;
        return this.endpoint;
    }
}

export class AzureEndpoint {
    public applicationTokenCredentials: any;
    public subscriptionID: any;
}

export class ApplicationTokenCredentials {
    async getToken() {
        try {
            credential = new ClientSecretCredential(tenantId, clientId, clientSecret);
            const armResourceEndpoint = 'https://management.azure.com/.default';
            const tokenResponse = await credential.getToken(armResourceEndpoint);
            const accessToken = tokenResponse?.token;
            bearerAccessToken = accessToken;
            return accessToken;
        } catch (error) {
            console.error('Error:', error);
        }
    }
}

export class StorageManagementClient {
    private storageAccounts;
    constructor(credentials: any, subscriptionId: any, baseUri: any, options: any) {
        this.storageAccounts = new StorageAccounts();
        this.storageAccounts.subscriptionId = subscriptionId;
    }
}

export class StorageAccount {
    public id: string | undefined = "resourceGroups/appattach-test-rg";
}

export class StorageAccounts {
    public subscriptionId: any;
    get(storageAccountName: any) {
        return new StorageAccount();
    }

    async listKeys(resourceGroupName: string, accountName: string, options: any, storageAccountType?: string) {
        try {
            const storageClient = new CustomStorageClient(credential, subscriptionId);
            const keys = await storageClient.storageAccounts.listKeys(inpResourceGroupName, accountName);
            let storageAccountKeys: string[] = [];
            if (keys.keys && keys.keys.length > 0) {
                keys.keys.forEach((key, index) => {
                    storageAccountKeys.push(key.value!);
                });
            } else {
                console.log('No keys found for the storage account.');
            }
            return storageAccountKeys; 
        } catch (error) {
            console.error("AVD Publish Test Error: " + error);
        }
    }
}

const avdInputs = path.join(__dirname, 'assets', 'avdinputs.json');
const fileContent = fs.readFileSync(avdInputs, 'utf-8');
const jsonData = JSON.parse(fileContent);
const subscriptionId = jsonData["connectedServiceNameARM"];
const inpResourceGroupName = jsonData["resourceGroupName"];
const clientId = jsonData["clientId"];
const clientSecret = jsonData["clientSecret"];
const tenantId = jsonData["tenantId"];

