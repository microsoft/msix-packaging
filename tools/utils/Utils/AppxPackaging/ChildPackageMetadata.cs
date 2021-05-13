// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

namespace Microsoft.Msix.Utils.AppxPackaging
{
    using Microsoft.Msix.Utils.AppxPackagingInterop;

    /// <summary>
    /// The type of the package in a bundle.
    /// </summary>
    public enum PackageType
    {
        /// <summary>
        /// A package containing executable code.
        /// </summary>
        Application = 0,

        /// <summary>
        /// A resource package.
        /// </summary>
        Resource = 1,
    }

    /// <summary>
    /// Class that represents the information of a bundled child package.
    /// </summary>
    public class ChildPackageMetadata
    {
        public ChildPackageMetadata(string filePath, APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE packageType, ulong packageSize, string packageFullName, ulong packageVersion, string packageResourceId)
        {
            this.FilePath = filePath;
            this.PackageType = (PackageType)packageType;
            this.PackageSize = packageSize;
            this.PackageFullName = packageFullName;
            this.PackageArchitecture = PackagingUtils.GetPackageArchitectureFromFullName(packageFullName);
            this.PackageVersion = new VersionInfo(packageVersion);
            this.PackageResourceId = packageResourceId;
        }

        /// <summary>
        /// Gets the path the package.
        /// </summary>
        public string FilePath { get; }

        /// <summary>
        /// Gets the package type.
        /// </summary>
        public PackageType PackageType { get; }

        /// <summary>
        /// Gets the package architecure.
        /// </summary>
        public string PackageArchitecture { get; }

        /// <summary>
        /// Gets the package size in bytes.
        /// </summary>
        public ulong PackageSize { get; }

        /// <summary>
        /// Gets the package full name.
        /// </summary>
        public string PackageFullName { get; }

        /// <summary>
        /// Gets the package version.
        /// </summary>
        public VersionInfo PackageVersion { get; }

        /// <summary>
        /// Gets the package ResourceId, if any.
        /// </summary>
        public string PackageResourceId { get; }
    }
}
