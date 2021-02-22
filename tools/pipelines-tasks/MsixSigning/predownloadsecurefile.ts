import tl = require('azure-pipelines-task-lib/task')

import secureFilesCommon = require('./securefiles-common/securefiles-common');

/**
 * A small wrapper around the secure file helpers.
 * This exists for easy mocking in unit tests.
 */
export const downloadSecureFile = async (secureFileId: string): Promise<string> =>
{
    // For local debugging.
    if (process.argv.includes("--mockSecureFileDownload"))
    {
        return secureFileId;
    }

    console.log("downloading file with secureFileId:", secureFileId);
    const secureFileHelpers = new secureFilesCommon.SecureFileHelpers();
    const secureFilePath: string = await secureFileHelpers.downloadSecureFile(secureFileId);
    console.log("file downloaded");
    return secureFilePath;
}

export const deleteSecureFile = (secureFileId: string): void =>
{
    // For local debugging don't delete.
    if (process.argv.includes("--mockSecureFileDownload"))
    {
        return;
    }

    const secureFileHelpers = new secureFilesCommon.SecureFileHelpers();
    secureFileHelpers.deleteSecureFile(secureFileId);
}