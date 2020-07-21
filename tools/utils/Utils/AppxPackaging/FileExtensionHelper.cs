//-----------------------------------------------------------------------
// <copyright file="FileExtensionHelper.cs" company="Microsoft">
//     Copyright (c) Microsoft. All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

namespace Microsoft.Packaging.Utils.AppxPackaging
{
    public static class FileExtensionHelper
    {
        /// <summary>
        /// Checks if a file contains unencrypted package extension.
        /// </summary>
        /// <param name="path">Path to a file.</param>
        /// <returns>If the path contains unencrypted package extension.</returns>
        public static bool HasUnencryptedPackageExtension(string path)
        {
            return PackagingConstants.AppxFileExtension.Equals(FileSystemUtils.GetLowercaseExtension(path)) ||
                PackagingConstants.MsixFileExtension.Equals(FileSystemUtils.GetLowercaseExtension(path));
        }

        /// <summary>
        /// Checks if a file contains unencrypted bundle extension.
        /// </summary>
        /// <param name="path">Path to a file.</param>
        /// <returns>If the path contains unencrypted bundle extension.</returns>
        public static bool HasUnencryptedBundleExtension(string path)
        {
            return PackagingConstants.AppxBundleFileExtension.Equals(FileSystemUtils.GetLowercaseExtension(path)) ||
                PackagingConstants.MsixBundleFileExtension.Equals(FileSystemUtils.GetLowercaseExtension(path));
        }

        /// <summary>
        /// Checks if a file contains encrypted package extension.
        /// </summary>
        /// <param name="path">Path to a file.</param>
        /// <returns>If the path contains encrypted package extension.</returns>
        public static bool HasEncryptedPackageExtension(string path)
        {
            return PackagingConstants.EncryptedAppxFileExtension.Equals(FileSystemUtils.GetLowercaseExtension(path)) ||
                PackagingConstants.EncryptedMsixFileExtension.Equals(FileSystemUtils.GetLowercaseExtension(path));
        }

        /// <summary>
        /// Checks if a file contains encrypted bundle extension.
        /// </summary>
        /// <param name="path">Path to a file.</param>
        /// <returns>If the path contains encrypted bundle extension.</returns>
        public static bool HasEncryptedBundleExtension(string path)
        {
            return PackagingConstants.EncryptedAppxBundleFileExtension.Equals(FileSystemUtils.GetLowercaseExtension(path)) ||
                PackagingConstants.EncryptedMsixBundleFileExtension.Equals(FileSystemUtils.GetLowercaseExtension(path));
        }
    }
}
