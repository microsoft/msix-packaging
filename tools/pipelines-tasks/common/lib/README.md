# App Attach Framework
The App Attach Framework artifact (AppAttachKernel.nupkg) helps to Build, Package and App Attach your application. Following functionality are supported through App Attach Framework:
1. Build and MSIX Package: This functionality allows to build your source code, package, sign and create an App Attach ready artifact(VHDX)
2. AVD App Attach: This functionality allows to publish App Attach artifact to AVD.
3. Local App Attach: This functionality is supported for your local systems where it allows to local app attach your App Attach artifact.

It supports either entire or subset of these functionalities.

# How to Use
To install App Attach Framework one needs to install `AppAttachKernel.nupkg` and use it as mentioned below:

## Build and MSIX Package
``` json5
string inputJson_BuildAndPackage = 
{
	"taskType": "7", 
	"packageLocation": "<Output Package location>", 
	"projectFilePath": "<Path to project file path>", 
	"configuration": "<Build Configuration(x64 or x86)>", 
	"platform": "<Build Platform (Debug or Release)>", 
	"certificatePath": "<Path to Certificate file for MSIX signing>", 
	"certificatePassword": "<Certificate Password>", 
	"msbuildPath": "<Path to MS Build>", 
	"msixManagerPath": "<Path to MSIX Manager>", 
	"packageVersion": "<Package Version>",
	"clientType": "<Name of client from where App Attach is being done>",
	"clientVersion": "<Client Version>"
}; 

AppAttachKernelController appAttachKernel = new AppAttachKernelController(inputJson_BuildAndPackage);
var cancellationTokenSource = new CancellationTokenSource();
appAttachKernel.execute(cancellationTokenSource.Token);
```

## AVD App Attach
``` json5
string inputJson_AVDAppAttach = 
{
	"taskType": "8", 
	"appAttachImagePath": "<Path to VHDX app attach image>",
	"emailId": "<Email of target azure user to authenticate and app attach>", 
	"accessToken": "<Access token to authenticate and app attach>", 
	"azureSubscriptionId": "<Azure subscription Id>",
	"azureResourceGroup": "<Azure Resource group>",
	"azureStorageAccount": "<Azure storage account for App Attach>",
	"azureStorageKey": "<Storage key of target storage account to gain access to target file share", 
	"azureStorageFileShare": "<Azure File share to be used for app attach>",
	"azureHostPoolName": "<target AVD Host pool for app attach>",
	"azureWorkspace": "<target AVD work space for app attach>", 
	"azureApplicationGroupName": "<target AVD Application Group for app attach>",
	"clientType": "<Name of client from where App Attach is being done>",
	"clientVersion": "<Client Version>"
}

AppAttachKernelController appAttachKernel = new AppAttachKernelController(inputJson_AVDAppAttach);
var cancellationTokenSource = new CancellationTokenSource();
appAttachKernel.execute(cancellationTokenSource.Token);
```

## Local App Attach
``` json5
string inputJson_LocalAppAttach = 
{
	"taskType": "16", 
	"appAttachImagePath": "<Path to VHDX app attach image>",
	"clientType": "<Name of client from where App Attach is being done>",
	"clientVersion": "<Client Version>"
}

AppAttachKernelController appAttachKernel = new AppAttachKernelController(inputJson_LocalAppAttach);
var cancellationTokenSource = new CancellationTokenSource();
appAttachKernel.execute(cancellationTokenSource.Token);
```