import os = require('os');
import path = require('path');
import tl = require('azure-pipelines-task-lib/task');
import { ToolRunner } from 'azure-pipelines-task-lib/toolrunner';

import helpers = require('common/helpers');

import msbuildHelpers = require('./MSBuildHelpers/msbuildhelpers');

const MSBUILD_PATH_HELPER_SCRIPT = path.join(__dirname, 'GetMSBuildPath.ps1');

export interface MSBuildCommonParameters
{
    solution: string;
    configuration: string;
    msbuildArguments: string;
    appxPackageBuildMode: string,
    clean: boolean;
    msbuildLocationMethod: string;
    msbuildLocation?: string;
    msbuildVersion?: string;
    msbuildArchitecture?: string;
}

/**
 * Reads the task's inputs related to MSBuild that are common to creating packages and bundle.
 */
export const readMSBuildInputs = (): MSBuildCommonParameters =>
{
    const solution: string = helpers.getInputWithErrorCheck('solution', 'A path to a solution is required.');
    const configuration: string = helpers.getInputWithErrorCheck('buildConfiguration', 'Build configuration is required.');
    const msbuildArguments: string = '/p:AppxPackageSigningEnabled=false';
    const clean: boolean = tl.getBoolInput('clean');
    const appxPackageBuildMode: string = tl.getInput('appPackageDistributionMode') ?? 'SideloadOnly'
    const msbuildLocationMethod: string = helpers.getInputWithErrorCheck('msBuildLocationMethod', 'Method to locate MSBuild is required.');

    let msbuildVersion: string | undefined;
    let msbuildArchitecture: string | undefined;
    if (msbuildLocationMethod === 'version')
    {
        msbuildVersion = helpers.getInputWithErrorCheck('msbuildVersion', 'Version of MSBuild to use is required.');
        msbuildArchitecture = helpers.getInputWithErrorCheck('msbuildArchitecture', 'Build architecture of MSBuild to use is required.');
    }

    let msbuildLocation: string | undefined;
    if (msbuildLocationMethod === 'location')
    {
        msbuildLocation = helpers.getInputWithErrorCheck('msbuildLocation', 'Location of MSBuild.exe is required.');
    }

    return {
        solution,
        configuration,
        msbuildArguments,
        appxPackageBuildMode,
        clean,
        msbuildLocationMethod,
        msbuildLocation,
        msbuildVersion,
        msbuildArchitecture
    };
}

const getMSBuildPathFromVersion = async (msbuildVersion: string, msbuildArchitecture: string): Promise<string> =>
{
    const osPlatform: string = os.platform();
    if (osPlatform === 'win32')
    {
        // The Powershell helper only works on Windows;
        // it looks in the Global Assembly Cache to find the right version.
        // We use a wrapper script to call the right function from the helper.
        const powershellRunner: ToolRunner = helpers.getPowershellRunner(MSBUILD_PATH_HELPER_SCRIPT);
        powershellRunner.arg(['-PreferredVersion', msbuildVersion]);
        powershellRunner.arg(['-Architecture', msbuildArchitecture]);

        const execResult = powershellRunner.execSync();
        if (execResult.code)
        {
            throw execResult.stderr;
        }

        return execResult.stdout.trim();
    }
    else
    {
        // The TypeScript helper only works on Mac and Linux.
        return await msbuildHelpers.getMSBuildPath(msbuildVersion);
    }
}

const getMSBuildToolRunner = (
    msbuildToolPath: string,
    solutionFile: string,
    createBundle: boolean,
    outputPath: string,
    platforms: string[],
    configuration: string,
    msbuildArguments: string,
    appxPackageBuildMode: string
    ) : ToolRunner =>
{
    const buildTool: ToolRunner = tl.tool(msbuildToolPath);
    buildTool.arg(solutionFile);

    // If we don't specify a platform when bundling, MSBuild may use one we are not building and cause an error.
    buildTool.arg('/p:Platform=' + platforms[0]);
    buildTool.arg('/p:Configuration=' + configuration);

    buildTool.arg('/p:UapAppxPackageBuildMode=' + appxPackageBuildMode);
    if (msbuildArguments)
    {
        buildTool.line(msbuildArguments);
    }

    // Add arguments specific for bundles/packages
    if (createBundle)
    {
        buildTool.arg('/p:AppxBundle=Always');
        buildTool.arg('/p:AppxBundleOutput=' + outputPath);
        buildTool.arg('/p:AppxBundlePlatforms=' + platforms.join('|'));
    }
    else
    {
        buildTool.arg('/p:AppxBundle=Never');
        buildTool.arg('/p:AppxPackageOutput=' + outputPath);
    }

    return buildTool;
}

export const runMSBuild = async (
    createBundle: boolean,
    outputPath: string,
    platforms: string[],
    {
        solution,
        configuration,
        msbuildArguments,
        appxPackageBuildMode,
        clean,
        msbuildLocationMethod,
        msbuildLocation,
        msbuildVersion,
        msbuildArchitecture,
    }: MSBuildCommonParameters) =>
{
    const msbuildTool: string = msbuildLocationMethod === 'location' ? msbuildLocation! : await getMSBuildPathFromVersion(msbuildVersion!, msbuildArchitecture!);

    const filesList: string[] = tl.findMatch('', solution, { followSymbolicLinks: false, followSpecifiedSymbolicLink: false, allowBrokenSymbolicLinks: false }, { matchBase: true });

     // We only build a single file.
     const file: string = filesList[0];
    if (clean)
    {
        const cleanTool: ToolRunner = getMSBuildToolRunner(msbuildTool, file, createBundle, outputPath, platforms, configuration, msbuildArguments, appxPackageBuildMode);
        cleanTool.arg('/t:Clean');
        await cleanTool.exec();
    }

    const buildTool: ToolRunner = getMSBuildToolRunner(msbuildTool, file, createBundle, outputPath, platforms, configuration, msbuildArguments, appxPackageBuildMode);
    await buildTool.exec();
}
