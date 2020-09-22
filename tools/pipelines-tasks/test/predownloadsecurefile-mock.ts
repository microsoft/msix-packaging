/**
 * Mock for downloadSecureFile.
 * Instead of receiving a secure file ID, this takes the file
 * path directly and returns it without changes.
 */
export const downloadSecureFile = async (secureFileId: string, retryCount: number): Promise<string> =>
{
    return secureFileId;
}