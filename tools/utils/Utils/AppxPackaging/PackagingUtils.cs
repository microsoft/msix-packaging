//-----------------------------------------------------------------------
// <copyright file="PackagingUtils.cs" company="Microsoft">
//     Copyright (c) Microsoft. All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

namespace Microsoft.Packaging.SDKUtils.AppxPackaging
{
    using System.Text.RegularExpressions;
    using AppxPackagingInterop;

    /// <summary>
    /// Class that contains packaging helper methods
    /// </summary>
    public class PackagingUtils
    {
        private static Regex packageFullNameRegex = new Regex("^(.*?)_(.*?)_(.*?)_(.*?)_(.*?)$");

        /// <summary>
        /// Returns true if the package full name refers to a bundle, false otherwise
        /// </summary>
        /// <param name="packageFullName">the full name</param>
        /// <returns>true if the full name references a bundle, false otherwise</returns>
        public static bool IsPackageFullNameForBundle(string packageFullName)
        {
            return GetPackageResourceIdFromFullName(packageFullName) == "~";
        }

        /// <summary>
        /// Gets the package family name from the package full name
        /// </summary>
        /// <param name="packageFullName">The package full name</param>
        /// <returns>The package family name</returns>
        public static string GetPackageFamilyNameFromFullName(string packageFullName)
        {
            Match packageNameMatches = packageFullNameRegex.Match(packageFullName);
            return string.Format(
                "{0}_{1}",
                packageNameMatches.Groups[1].ToString(),
                packageNameMatches.Groups[5].ToString());
        }

        /// <summary>
        /// Gets the package version from the package full name
        /// </summary>
        /// <param name="packageFullName">The package full name</param>
        /// <returns>The package version</returns>
        public static VersionInfo GetPackageVersionFromFullName(string packageFullName)
        {
            Match packageNameMatches = packageFullNameRegex.Match(packageFullName);
            return new VersionInfo(packageNameMatches.Groups[2].ToString());
        }

        /// <summary>
        /// Gets the package architecture from the package full name
        /// </summary>
        /// <param name="packageFullName">The package full name</param>
        /// <returns>The package architecture</returns>
        public static string GetPackageArchitectureFromFullName(string packageFullName)
        {
            Match packageNameMatches = packageFullNameRegex.Match(packageFullName);
            return packageNameMatches.Groups[3].ToString();
        }

        /// <summary>
        /// Gets the package resource ID from the package full name
        /// </summary>
        /// <param name="packageFullName">The package full name</param>
        /// <returns>The package resource ID</returns>
        public static string GetPackageResourceIdFromFullName(string packageFullName)
        {
            Match packageNameMatches = packageFullNameRegex.Match(packageFullName);
            return packageNameMatches.Groups[4].ToString();
        }

        /// <summary>
        /// Gets a package reader for the file, properly handling for encrypted packages.
        /// </summary>
        /// <param name="packagePath">The path to the package file.</param>
        /// <param name="isEncrypted">True if the package is encrypted; false if not.</param>
        /// <returns>A package reader for the file.</returns>
        public static IAppxPackageReader GetPackageReader(string packagePath, bool isEncrypted = false)
        {
            IAppxPackageReader result = null;
            if (isEncrypted)
            {
                IAppxEncryptionFactory factory = (IAppxEncryptionFactory)new AppxEncryptionFactory();
                result = factory.CreateEncryptedPackageReader(StreamUtils.CreateInputStreamOnFile(packagePath), System.IntPtr.Zero);
            }
            else
            {
                IAppxFactory factory = (IAppxFactory)new AppxFactory();
                result = factory.CreatePackageReader(StreamUtils.CreateInputStreamOnFile(packagePath));
            }

            return result;
        }
    }
}
