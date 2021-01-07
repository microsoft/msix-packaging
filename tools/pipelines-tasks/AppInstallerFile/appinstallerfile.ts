import helpers = require('common/helpers');

/* This file contains helpers for creating or modifying an .appinstaller file */

export interface UpdateParameters
{
    existingFile: string,
    versionUpdateMethod: string,
    fileVersion?: string
}

export interface CreateParameters
{
    uri: string,
    fileVersion: string,
    mainItemUri: string,
    updateOnLaunch: boolean,
    hoursBetweenUpdateChecks?: string,
    showPromptWhenUpdating?: boolean,
    updateBlocksActivation?: boolean,
    optionalItems: PackageOrBundle[],
    dependencies: PackageOrBundle[]
}

// Identity information about a package and bundle.
// This can represent a main item, an optional item or a dependency.
// The inner property is used to produce the XML, so the names of the
// properties must be kept named exactly as they are.
export interface PackageOrBundle
{
    isBundle: boolean,
    packageOrBundle:
    {
        Name: string,
        Publisher: string,
        Version: string,
        ProcessorArchitecture?: string,
        Uri: string
    }
}

/**
 * Update the version of the App Installer file.
 * Save the new App Installer file to outputPath.
 * @param outputPath the path to save the new App Installer content to
 * @param existingFile the path to the existing App Installer file that needs updating
 * @param versionUpdateMethod how to update the version of the file
 * @param fileVersion new version of the file, if the update method is manual
 * @param mainApp identity of the main app, used to update the version
 */
export const updateExisting = async (
    outputPath: string,
    {
        existingFile,
        versionUpdateMethod,
        fileVersion
    }: UpdateParameters,
    mainApp: PackageOrBundle) =>
{
    const appInstallerFile = await helpers.parseXml(existingFile);

    let newFileVersion: string;
    if (versionUpdateMethod !== 'manual')
    {
        const existingFileVersion: string[] = appInstallerFile.AppInstaller.$.Version.split('.');
        newFileVersion = helpers.incrementVersion(existingFileVersion, versionUpdateMethod);
    }
    else
    {
        newFileVersion = fileVersion!;
    }

    // Update the version of the file.
    appInstallerFile.AppInstaller.$.Version = newFileVersion;

    // Update the version of the main app
    const tagToEdit: string = mainApp.isBundle ? 'MainBundle' : 'MainPackage';
    appInstallerFile.AppInstaller[tagToEdit][0].$.Version = mainApp.packageOrBundle.Version;

    helpers.writeXml(appInstallerFile, outputPath);
}

const getXmlElementsForItems = (items: PackageOrBundle[]): any =>
{
    const packages: any[] = [];
    const bundles: any[] = [];

    for (const item of items)
    {
        // The properties of the package/bundle are stored as attributes in the XML
        const xmlItem = { $: item.packageOrBundle };
        if (item.isBundle)
        {
            bundles.push(xmlItem);
        }
        else
        {
            packages.push(xmlItem);
        }
    }

    const element: any = {};
    if (bundles.length > 0)
    {
        element.Bundle = bundles;
    }

    if (packages.length > 0)
    {
        element.Package = packages;
    }

    return element;
}

export const createNew = (
    outputPath:string,
    {
        fileVersion,
        uri,
        optionalItems,
        dependencies,
        updateOnLaunch,
        hoursBetweenUpdateChecks,
        showPromptWhenUpdating,
        updateBlocksActivation
    }: CreateParameters,
    mainApp: PackageOrBundle) =>
{
    // Object representing the new XML file.
    // All of the property names here must be keep exactly as they are, as they will be translated to XML.
    // Initialize it with the root element.
    const newFile: any = { AppInstaller: {} };

    // Attributes of <AppInstaller>
    newFile.AppInstaller.$ = {
        xmlns: 'http://schemas.microsoft.com/appx/appinstaller/2018',
        Version: fileVersion,
        Uri: uri
    };

    // MainPackage/MainBundle
    newFile.AppInstaller[mainApp.isBundle ? 'MainBundle' : 'MainPackage'] =
    {
        $: mainApp.packageOrBundle
    }

    // Optional items
    if (optionalItems.length > 0)
    {
        newFile.AppInstaller.OptionalPackages = getXmlElementsForItems(optionalItems);
    }

    // Dependencies
    if (dependencies.length > 0)
    {
        newFile.AppInstaller.Dependencies = getXmlElementsForItems(dependencies);
    }

    // Update settings
    if (updateOnLaunch)
    {
        const onLaunchSettings: any = { $: {} };
        newFile.AppInstaller.UpdateSettings =
        {
            OnLaunch: onLaunchSettings
        };

        if (hoursBetweenUpdateChecks)
        {
            onLaunchSettings.$.HoursBetweenUpdateChecks = hoursBetweenUpdateChecks;
        }

        if (showPromptWhenUpdating)
        {
            onLaunchSettings.$.ShowPrompt = showPromptWhenUpdating;
        }

        if (updateBlocksActivation)
        {
            onLaunchSettings.$.UpdateBlocksActivation = updateBlocksActivation;
        }
    }

    helpers.writeXml(newFile, outputPath);
}