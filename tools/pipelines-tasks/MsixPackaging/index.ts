import path = require('path');
import tl = require('azure-pipelines-task-lib/task')
import { ToolRunner } from 'azure-pipelines-task-lib/toolrunner';

import helpers = require('common/helpers');

import msbuild = require('./msbuild');

/**
 * Reads the inputs needed for updating the app version in the manifest,
 * then edits it with the new version.
 */
const updateAppVersionInManifest = async () =>
{
    // read the input
    const manifestFile: string = helpers.getInputWithErrorCheck('manifestFile', 'To update the version of the app, the path to the manifest file is required to get the current version, but none was given.');

    // get the new version
    let newVersion: string = helpers.getInputWithErrorCheck('appVersion', 'To manually update the version of the app, a new version is required but none was given.');

    const parsedManifest: any = await helpers.parseXml(manifestFile);
    parsedManifest.Package.Identity[0].$.Version = newVersion;
    helpers.writeXml(parsedManifest, manifestFile);
}

/**
 * Gets the list of platforms to build from the input.
 */
const getPlatformsToBuild = (): string[] =>
{
    const allPlatforms: { [platform: string]: boolean } =
    {
        'x86': tl.getBoolInput('buildForX86'),
        'x64': tl.getBoolInput('buildForX64'),
        'ARM': tl.getBoolInput('buildForArm'),
        'Any CPU': tl.getBoolInput('buildForAnyCpu')
    };

    const selectedPlatforms: string[] = [];
    Object.keys(allPlatforms).forEach(key =>
    {
        if (allPlatforms[key])
        {
            selectedPlatforms.push(key);
        }
    });

    return selectedPlatforms;
}

/**
 * Reads the inputs for MSBuild and calls it to build the solution and
 * create the packages and bundle.
 * @param outputPath Save location for the produced package or bundle.
 * @param createBundle Whether we are creating a bundle or a package.
 */
const setUpAndRunMSBuild = async (outputPath: string, createBundle: boolean) =>
{
    const msbuildCommonParameters: msbuild.MSBuildCommonParameters = msbuild.readMSBuildInputs();

    if (createBundle)
    {
        const platformsToBuild: string[] = getPlatformsToBuild();
        if (!platformsToBuild.length)
        {
            throw Error('No platform was specified to be built.');
        }

        await msbuild.runMSBuild(createBundle, outputPath, platformsToBuild,  msbuildCommonParameters);
    }
    else
    {
        const platform: string = helpers.getInputWithErrorCheck('buildPlatform', 'Platform to build is required.');
        await msbuild.runMSBuild(createBundle, outputPath, [platform], msbuildCommonParameters);
    }
}

/**
 * Package prebuilt binaries.
 * @param outputPath Save location for the generated package or bundle.
 * @param createBundle Whether to produce a bundle or a package.
 */
const packageBuiltBinaries = async (outputPath: string, createBundle: boolean, inputDirectory: string) =>
{
    // The makemsix tool has a quirk where if the path ends with a slash, the tool
    // will fail, so we detect this and work around this problem by removing the
    // trailing slash if present
    const lastChar: string = inputDirectory[inputDirectory.length - 1];
    if (lastChar === '\\' || lastChar === '/')
    {
        console.log("inputDirectory ends with \\ trimming it...");
        inputDirectory = inputDirectory.slice(0, -1);
    }

    if (!createBundle)
    {
        // TODO: Replace makeappx by makemsix to make it cross-platform
        const makeAppxPackRunner: ToolRunner = tl.tool(helpers.MAKEAPPX_PATH);
        makeAppxPackRunner.arg('pack');
        makeAppxPackRunner.arg(['/o', '/v']);
        makeAppxPackRunner.arg(['/d', inputDirectory]);
        makeAppxPackRunner.arg(['/p', outputPath]);
        await makeAppxPackRunner.exec();
    }
    else
    {
        const makeAppxBundleRunner: ToolRunner = tl.tool(helpers.MAKEAPPX_PATH);
        makeAppxBundleRunner.arg('bundle');
        makeAppxBundleRunner.arg(['/o', '/v']);
        makeAppxBundleRunner.arg(['/d', inputDirectory]);
        makeAppxBundleRunner.arg(['/p', outputPath]);
        await makeAppxBundleRunner.exec();
    }
}

/**
 * Main function for the task.
 */
const run = async () =>
{
    tl.setResourcePath(path.join(__dirname, 'task.json'));

    // detect if the user will provide pre-built binaries or use MSBuild
    const buildSolution: boolean = tl.getBoolInput('buildSolution', /* required: */ true);
    let inputDirectory: string | undefined;
    if (!buildSolution)
    {
        inputDirectory = helpers.getInputWithErrorCheck('inputDirectory', 'To package pre-built binaries, a path to the directory containing valid binaries is required, but none was given.');
    }

    // read output path for the package or bundle.
    // resolve it to a full path to ensure it is the same in every place.
    // e.g. MSBuild seems to use the path relative to the solution dir in some cases.
    const outputPath: string = path.resolve(helpers.getInputWithErrorCheck('outputPath', 'An output path is required to save the package, but none was given.'));

    // whether to bundle or not is independent of whether or not to build from scratch
    // if the user gives the bundle option, check that they gave a path to save the output bundle.
    const generateBundle: boolean = tl.getBoolInput('generateBundle');

    // update the app version in the manifest
    const updateAppVersion: boolean = tl.getBoolInput('updateAppVersion');
    if (updateAppVersion)
    {
        await updateAppVersionInManifest();
    }

    // create the packages and bundle
    if (buildSolution)
    {
        await setUpAndRunMSBuild(outputPath, generateBundle);
    }
    else
    {
        await packageBuiltBinaries(outputPath, generateBundle, inputDirectory!);
    }
}

run().catch(err =>
    {
        tl.setResult(tl.TaskResult.Failed, err.message);
    })