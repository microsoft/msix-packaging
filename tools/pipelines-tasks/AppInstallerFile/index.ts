import path = require('path');
import tl = require('azure-pipelines-task-lib/task')

import helpers = require('common/helpers');

import appInstallerFile = require('./appinstallerfile');

interface Inputs {
    packagePath: string,
    outputPath: string,
    method: string,
    inputsForUpdate?: appInstallerFile.UpdateParameters,
    inputsForCreate?: appInstallerFile.CreateParameters
}

const readInputsForItem = (inputPrefix: string, isBundle: boolean): appInstallerFile.PackageOrBundle =>
{
    const Name: string = helpers.getInputWithErrorCheck(`${inputPrefix}Name`, `Input Error: ${inputPrefix} missing name.`);
    const Publisher: string = helpers.getInputWithErrorCheck(`${inputPrefix}Publisher`, `Input Error: ${inputPrefix} missing publisher.`);
    const Version: string = helpers.getInputWithErrorCheck(`${inputPrefix}Version`, `Input Error: ${inputPrefix} missing version.`);
    const ProcessorArchitecture: string | undefined = tl.getInput(`${inputPrefix}ProcessorArchitecture`);
    const Uri: string = helpers.getInputWithErrorCheck(`${inputPrefix}URI`, `Input Error: ${inputPrefix} missing URI.`);

    return { isBundle, packageOrBundle: { Name, Publisher, Version, ProcessorArchitecture, Uri } };
}

const readInputsForCreate = (): appInstallerFile.CreateParameters =>
{
    const uri: string = helpers.getInputWithErrorCheck('uri', 'The App Installer file URI is required but none was given');
    const fileVersion: string = helpers.getInputWithErrorCheck('fileVersion', 'The App Installer file version is required but none was given');
    const mainItemUri: string = helpers.getInputWithErrorCheck('mainItemUri', 'The URI of the main package/bundle is required but none was given');

    const updateOnLaunch: boolean = tl.getBoolInput('updateOnLaunch');
    let hoursBetweenUpdateChecks: string | undefined;
    let showPromptWhenUpdating: boolean | undefined;
    let updateBlocksActivation: boolean | undefined;
    if (updateOnLaunch)
    {
        hoursBetweenUpdateChecks = tl.getInput('hoursBetweenUpdateChecks');
        showPromptWhenUpdating = tl.getBoolInput('showPromptWhenUpdating');
        if (showPromptWhenUpdating)
        {
            updateBlocksActivation = tl.getBoolInput('updateBlocksActivation');
        }
    }

    const optionalItems: appInstallerFile.PackageOrBundle[] = [];
    const maximumOptionalItems: number = 3;
    for (let i = 1; i <= maximumOptionalItems; i++)
    {
        const optionalItemType: string | undefined = tl.getInput(`addOptionalItem${i}`);
        const optionalItemInputPrefix: string = `optionalItem${i}`;
        if (optionalItemType === 'package')
        {
            optionalItems.push(readInputsForItem(optionalItemInputPrefix, /* isBundle */ false));
        }
        else if (optionalItemType === 'bundle')
        {
            optionalItems.push(readInputsForItem(optionalItemInputPrefix, /* isBundle */ true));
        }
    }

    const dependencies: appInstallerFile.PackageOrBundle[] = [];
    const maximumDependencies: number = 3;
    for (let i = 1; i <= maximumDependencies; i++)
    {
        const dependencyType: string | undefined = tl.getInput(`addDependency${i}`);
        const dependencyInputPrefix: string = `dependency${i}`;
        if (dependencyType === 'package')
        {
            dependencies.push(readInputsForItem(dependencyInputPrefix, /* isBundle */ false));
        }
        else if (dependencyType === 'bundle')
        {
            dependencies.push(readInputsForItem(dependencyInputPrefix, /* isBundle */ true));
        }
    }

    return {
        uri,
        fileVersion,
        mainItemUri,
        updateOnLaunch,
        hoursBetweenUpdateChecks,
        showPromptWhenUpdating,
        updateBlocksActivation,
        optionalItems,
        dependencies
    }
}

const readInputsForUpdate = (): appInstallerFile.UpdateParameters =>
{
    const existingFile: string = helpers.getInputWithErrorCheck('existingFile', 'The path to the existing App Installer file that requires updating is required, but none was given.');
    const versionUpdateMethod: string = helpers.getInputWithErrorCheck('versionUpdateMethod', 'An update method is required to update the version but none was given.');
    let fileVersion: string | undefined;
    if (versionUpdateMethod === 'manual')
    {
        fileVersion = helpers.getInputWithErrorCheck('fileVersionUpdateMethod', 'To manually update the version, a version is needed but none was given.');
    }

    return { existingFile, versionUpdateMethod, fileVersion };
}

/**
 * @returns A dictionary with the task arguments.
 */
const readInputs = (): Inputs =>
{
    const packagePath: string = helpers.getInputWithErrorCheck('package', 'Path to the package or bundle is required');
    const outputPath: string = helpers.getInputWithErrorCheck('outputPath', 'A name is required to save the App Installer to once created, but none was given');
    const method: string = helpers.getInputWithErrorCheck('method', 'Method for generating the App Installer file is required.')
    let inputsForCreate: appInstallerFile.CreateParameters | undefined;
    let inputsForUpdate: appInstallerFile.UpdateParameters | undefined;

    if (method === 'create')
    {
        inputsForCreate = readInputsForCreate();
    }
    else if (method === 'update')
    {
        inputsForUpdate = readInputsForUpdate();
    }

    return { packagePath, outputPath, method, inputsForCreate, inputsForUpdate };
}

/**
 * Returns the parsed manifest (appxmanifest.xml for a package or appxbundlemanifest.xml
 * for a bundle) by unpacking the package or bundle.  This will unpack it, parse the
 * manifest and delete the unpack directory.
 * @param packagePath path to the package to inspect
 * @param isBundle whether or not the given App is a bundle
 */
const getAppxManifestPathFromApp = async (packagePath: string, isBundle: boolean): Promise<any> =>
{
    const packageName: string = path.parse(packagePath).name;
    const extractionDirectory: string = path.join(helpers.getTempDirectory(), packageName);
    try
    {
        tl.debug(`Unpacking ${packagePath} to `);
        await helpers.unpackFile(packagePath, extractionDirectory, isBundle);
        let appxManifestPath: string;

        // here, we want to want to detect if the given app
        // is an MSIX package or bundle, specifically, if it's a bundle,
        // we need to look for a folder called AppxMetadata which should
        // contain the appxbundlemanifest.xml file
        if (isBundle)
        {
            appxManifestPath = path.join(extractionDirectory, 'AppxMetadata', 'AppxBundleManifest.xml');
        }
        else
        {
            // if it's just a package then the directory which we unpacked the
            // content to should contain the appxmanifest.xml file
            appxManifestPath = path.join(extractionDirectory, 'AppxManifest.xml');
        }

        return await helpers.parseXml(appxManifestPath);
    }
    finally
    {
        tl.rmRF(extractionDirectory);
    }
}

const getAppInformationFromPackage = async (packagePath: string): Promise<appInstallerFile.PackageOrBundle> =>
{
    const extension: string = path.extname(packagePath).toLowerCase();
    const isBundle: boolean = extension === '.appxbundle' || extension === '.msixbundle';

    // Read the manifest
    const manifest = await getAppxManifestPathFromApp(packagePath, isBundle);

    if (isBundle)
    {
        return { isBundle, packageOrBundle: manifest.Bundle.Identity[0].$ };
    }
    else
    {
        return { isBundle, packageOrBundle: manifest.Package.Identity[0].$ };
    }
}

/**
 * Main function for the task.
 */
const run = async () =>
{
    tl.setResourcePath(path.join(__dirname, 'task.json'));

    const inputs: Inputs = readInputs();

    // Get app info from the manifest in the app to use in the App Installer file
    const appInformation: appInstallerFile.PackageOrBundle = await getAppInformationFromPackage(inputs.packagePath);

    if (inputs.method === 'create')
    {
        // The main item was read from the package and does not yet have an URI
        appInformation.packageOrBundle.Uri = inputs.inputsForCreate!.mainItemUri;
        appInstallerFile.createNew(inputs.outputPath, inputs.inputsForCreate!, appInformation);
    }
    else if (inputs.method === 'update')
    {
        await appInstallerFile.updateExisting(inputs.outputPath, inputs.inputsForUpdate!, appInformation);
    }
}

run().catch(err =>
    {
        tl.setResult(tl.TaskResult.Failed, err.message);
    })