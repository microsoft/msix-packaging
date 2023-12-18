import * as path from "path";
import * as tl from "azure-pipelines-task-lib/task";
import * as fs from 'fs';
import fetch from 'node-fetch';

/**
 * downloads nuget tool(nuget.exe).
 * @param url weblink to download nuget tool
 * @param downloadLoc location to download the nuget too into
 */
export async function downloadNugetTool(url: string, downloadLoc: string): Promise<void> {
    try {
        const response = await fetch(url);

        if (!response.ok) {
            throw new Error(`Failed to download file. Status: ${response.status} - ${response.statusText}`);
        }

        const fileStream = fs.createWriteStream(path.join(downloadLoc, 'nuget.exe'));
        await new Promise<void>((resolve, reject) => {
            response.body.pipe(fileStream);
            response.body.on('error', (err: any) => {
                reject(err);
            });
            fileStream.on('finish', function () {
                resolve();
            });
        });
    } catch (error) {
        console.error("Error downloading file: " + error);
    }
}

/**
 * picks installed nuget and copies target DLLs into output folder
 * @param folderPath the path to the package(.nupkg or packages.config) file to be installed
 * @param targetNetFramework target .Net framework to pick target and dependency DLLs complying with the package to be installed
 */
export async function copyTargetDlls(folderPath: string, targetNetFramework: string): Promise<void> {
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