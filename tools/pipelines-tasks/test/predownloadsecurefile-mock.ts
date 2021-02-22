/**
 * Mock for downloadSecureFile.
 * Instead of receiving a secure file ID, this takes the file
 * path directly and returns it without changes.
 */
export const downloadSecureFile = async (secureFileId: string): Promise<string> =>
{
    return secureFileId;
}

/**
 * Mock for deleteSecureFile; does nothing as we didn't download anything.
 */
export const deleteSecureFile = (secureFileId: string) =>
{
}