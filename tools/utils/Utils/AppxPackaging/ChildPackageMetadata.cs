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
        public ChildPackageMetadata(AppxBundleMetadata parentBundle, string packageFullName, string relativePath, APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE packageType, ulong packageSize, ulong packageVersion, string packageResourceId)
        {
            this.ParentBundle = parentBundle;
            this.RelativeFilePath = relativePath;
            this.PackageType = (PackageType)packageType;
            this.Size = packageSize;
            this.PackageFullName = packageFullName;
            this.Architecture = PackagingUtils.GetPackageArchitectureFromFullName(packageFullName);
            this.Version = new VersionInfo(packageVersion);
            this.ResourceId = packageResourceId;
        }

        /// <summary>
        /// Gets the information of the parent bundle which contains this reference
        /// </summary>
        public AppxBundleMetadata ParentBundle { get; private set; }

        /// <summary>
        /// Gets the relative path the package.
        /// </summary>
        public string RelativeFilePath { get; }

        /// <summary>
        /// Gets the package type.
        /// </summary>
        public PackageType PackageType { get; }

        /// <summary>
        /// Gets the package architecure.
        /// </summary>
        public string Architecture { get; }

        /// <summary>
        /// Gets the package size in bytes.
        /// </summary>
        public ulong Size { get; }

        /// <summary>
        /// Gets the package full name.
        /// </summary>
        public string PackageFullName { get; }

        /// <summary>
        /// Gets the package version.
        /// </summary>
        public VersionInfo Version { get; }

        /// <summary>
        /// Gets the package ResourceId, if any.
        /// </summary>
        public string ResourceId { get; }
    }
}
