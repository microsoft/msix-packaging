import fs = require('fs');
import os = require('os');
import path = require('path');
import tl = require('azure-pipelines-task-lib/task');
import xml = require('xml2js');

import { ToolRunner } from 'azure-pipelines-task-lib/toolrunner';
import nugetHelper = require('./nuget_helper');

export const MAKEAPPX_PATH = path.join(__dirname, 'lib', 'makeappx');

export const APPATTACH_FRAMEWORK_NUPKG_DIR = path.join(__dirname, 'lib');
export const CLIENT_TYPE = 'AzureDevOps';
export const CLIENT_VERSION = '2.0.0';
const NUGET_INSTALL_SCRIPT = path.join(__dirname, 'NugetInstall.ps1');

/**
 * When running on an agent, returns the value of Agent.TempDirectory which is cleaned after
 * each pipeline job.  When running locally, returns the local temp directory.
 */
export const getTempDirectory = (): string =>
{
    return tl.getVariable('Agent.TempDirectory') ?? os.tmpdir();
}

/**
 * Gets an input variable, checking that it is present and throwing if it's not.
 * @param variableName Name of the input variable to get.
 * @param errorMessage Message to include in the error if the variable is empty, null or undefined.
 * @returns The value of the input variable if it is set.
 */
export const getInputWithErrorCheck = (variableName: string, errorMessage: string): string =>
{
    const variableValue: string | undefined = tl.getInput(variableName);
    if (!variableValue)
    {
        throw Error('Input Error: ' + errorMessage);
    }

    return variableValue;
}

/**
 * Gets a ToolRunner for running a Powershell script.
 * Script arguments can be added later by the caller.
 * @param scriptPath Script to run.
 */
export const getPowershellRunner = (scriptPath: string): ToolRunner =>
{
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

/**
 * Increment the current version given by the method specified. A version
 * is in the form of (major).(minor).(build).(revision). The incremental
 * method specifies which part of the version to increment. For example,
 * if the current version given is 1.0.0.0 and the incrementalMethod = 'revision',
 * then '1.0.0.1' is returned.
 * @param currentVersion
 * @param incrementalMethod
 */
export const incrementVersion = (currentVersion: string[], incrementalMethod: string): string =>
{
    const methodToIndex: { [method: string]: number } =
    {
        'revision': 3,
        'build': 2,
        'minor': 1,
        'major': 0
    }

    if (!Object.keys(methodToIndex).includes(incrementalMethod))
    {
        throw Error(`"${incrementalMethod}" is not a valid method to increment the version.`);
    }

    // TODO: roll over if maximum reached?
    const index: number = methodToIndex[incrementalMethod];
    currentVersion[index] = (parseInt(currentVersion[index]) + 1).toString();
    for (let a: number = index + 1; a < 4; a++)
    {
        currentVersion[a] = '0';
    }

    return currentVersion.join('.')
}

/**
 * Unpack/unbundle a package or bundle and put the outputs into <outputDirectory>.
 * @param filePath the path to the file which we wish to unpack/unbundle
 * @param outputDirectory the directory where the content of the unpacked file will be deposited to
 * @param isBundle whether or not the given file is a bundle
 */
export const unpackFile = async (filePath: string, outputDirectory: string, isBundle: boolean) =>
{
    const makeAppxRunner: ToolRunner = tl.tool(MAKEAPPX_PATH);
    makeAppxRunner.arg(isBundle ? 'unbundle' : 'unpack');
    makeAppxRunner.arg(['-p', filePath]);
    makeAppxRunner.arg(['-d', outputDirectory]);
    makeAppxRunner.arg('-o');

    await makeAppxRunner.exec();
}

export const parseXml = async (filePath: string): Promise<any> =>
{
    const fileText: string = fs.readFileSync(filePath).toString();
    const parser = new xml.Parser();
    return await parser.parseStringPromise(fileText);
}

export const writeXml = (xmlObject: any, filePath: string) =>
{
    const xmlBuilder = new xml.Builder();
    fs.writeFileSync(filePath, xmlBuilder.buildObject(xmlObject));
}

/**
 * installs input Nuget package into given output Location.
 * @param packagePath the path to the package(.nupkg or packages.config) file to be installed
 * @param outputPath output location to install the nuget package into
 * @param targetNetFramework target .Net framework to pick target and dependency DLLs complying with the package to be installed
 * @param packageId optional parameter, package id to be installed. Required if package install mechanism is through providing package id(instead of packages.config)
 * @param version optional parameter, package version to be installed. Required if package install mechanism is through providing package id(instead of packages.config)
 */
export async function installNuget(packagePath: string, outputPath: string, targetNetFramework: string, packageId?: string, version?: string) {
    try {
        tl.pushd(packagePath);

        const nugetToolSrc: string = "https://dist.nuget.org/win-x86-commandline/latest/nuget.exe";

        await nugetHelper.downloadNugetTool(nugetToolSrc, packagePath);
        let powershellRunner: ToolRunner = getPowershellRunner(NUGET_INSTALL_SCRIPT);
        powershellRunner.arg(['-nugetToolPath', './nuget.exe']);
        if (packageId && version) {
            powershellRunner.arg(['-packageId', packageId]);
            powershellRunner.arg(['-version', version]);
        }
        powershellRunner.arg(['-outputDirectory', outputPath]);
        let execResult = powershellRunner.execSync();
        if (execResult.code) {
            throw execResult.stderr;
        }

        await nugetHelper.copyTargetDlls(outputPath, targetNetFramework);
        tl.rmRF('nuget.exe');
        tl.popd();
    } catch (error) {
        console.error("Error finding target framework Dlls: " + error);
    }
}