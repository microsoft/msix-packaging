import * as path from "path";
import * as tl from "azure-pipelines-task-lib/task";
import * as fs from 'fs';
import { ToolRunner } from 'azure-pipelines-task-lib/toolrunner';
import helpers = require('./helpers');

const NUGET_INSTALL_SCRIPT = path.join(__dirname, 'NugetInstall.ps1');
const APPATTACH_FRAMEWORK_NUPKG_DIR = path.join(__dirname, 'lib');

/**
 * picks installed nuget and copies target DLLs into output folder
 * @param folderPath the path to the package(.nupkg or packages.config) file to be installed
 * @param targetNetFramework target .Net framework to pick target and dependency DLLs complying with the package to be installed
 */
async function copyTargetDlls(folderPath: string, targetNetFramework: string): Promise<void> {
    try {
        // Read the contents of the folder
        const folderContents = fs.readdirSync(folderPath);

        for (const nugetPackFolder of folderContents) {
            const libFolderContents = fs.readdirSync(path.join(folderPath, nugetPackFolder, 'lib'));

            const dotNetFrameworks: string[] = ['net11', 'net20', 'net35', 'net40', 'net403', 'net45', 'net451', 'net452', 'net46', 'net461', 'net462', 'net47', 'net471', 'net472',
                'net48', 'net5.0', 'net6.0', 'net7.0'];

            // Find the highest version less than equal to 4.7.2 (to be compatible with AppAttachFramework)
            let targetNetFolder: string | undefined;

            const targetFrameworkIndx = dotNetFrameworks.indexOf(targetNetFramework);
            for (let i = targetFrameworkIndx; i >= 0; i--) {
                if (libFolderContents.includes(dotNetFrameworks[i])) {
                    targetNetFolder = dotNetFrameworks[i];
                    break;
                }
            }

            if (targetNetFolder) {
                // Read the contents of the target lib folder
                const targetNetFolderPath = path.join(folderPath, nugetPackFolder, 'lib', targetNetFolder);
                const targetNetFolderContents = fs.readdirSync(targetNetFolderPath);

                // Filter DLL files
                const dllFiles = targetNetFolderContents.filter(item => /\.dll$/.test(item));

                // Add the full path to the DLLs
                dllFiles.forEach(dll => {
                    const dllPath = path.join(targetNetFolderPath, dll);
                    tl.cp(dllPath, folderPath, undefined, true);
                });
                tl.rmRF(path.join(folderPath, nugetPackFolder));
            }
            else {
                console.log("target .net dependency libraries for " + nugetPackFolder + " not found");
            }
        }
    } catch (error) {
        console.error("Error finding target framework Dlls: " + error);
    }
}

/**
 * installs input Nuget package into given output Location.
 * @param packagePath the path to the package(.nupkg or packages.config) file to be installed
 * @param outputPath output location to install the nuget package into
 * @param targetNetFramework target .Net framework to pick target and dependency DLLs complying with the package to be installed
 * @param packageId optional parameter, package id to be installed. Required if package install mechanism is through providing package id(instead of packages.config)
 * @param version optional parameter, package version to be installed. Required if package install mechanism is through providing package id(instead of packages.config)
 */
async function installNuget(packagePath: string, outputPath: string, targetNetFramework: string, packageId?: string, version?: string) {
    try {
        tl.pushd(packagePath);

        let powershellRunner: ToolRunner = helpers.getPowershellRunner(NUGET_INSTALL_SCRIPT);
        if (packageId && version) {
            powershellRunner.arg(['-packageId', packageId]);
            powershellRunner.arg(['-version', version]);
        }
        powershellRunner.arg(['-outputDirectory', outputPath]);
        let execResult = powershellRunner.execSync();
        if (execResult.code) {
            throw execResult.stderr;
        }

        await copyTargetDlls(outputPath, targetNetFramework);
    } catch (error) {
        console.error("Error finding target framework Dlls: " + error);
    }
    finally {
        tl.popd();
    }
}

function run(): void {
    try {
        const AppAttachFrameworkDll = path.join(__dirname, 'lib/AppAttachFrameworkDLL');
        if (tl.exist(AppAttachFrameworkDll)) {
            tl.rmRF(AppAttachFrameworkDll);
        }
        installNuget(APPATTACH_FRAMEWORK_NUPKG_DIR, AppAttachFrameworkDll, 'net472');
    } catch (error) {
        console.error(tl.loc("AppAttachFramework Install Error", error));
    }
}

run();