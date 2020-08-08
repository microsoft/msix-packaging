//-----------------------------------------------------------------------
// <copyright file="PackageMetadata.cs" company="Microsoft">
//     Copyright (c) Microsoft. All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

namespace Microsoft.Msix.Utils.AppxPackaging
{
    using System.Runtime.InteropServices.ComTypes;

    /// <summary>
    /// Class that represents the information of a package. Note that "Package"
    /// denotes an appx or an appxbundle file.
    /// </summary>
    public abstract class PackageMetadata
    {
        /// <summary>
        /// Gets or sets the path the package.
        /// </summary>
        public string FilePath { get; protected set; }

        /// <summary>
        /// Gets or sets the identity name of the package.
        /// </summary>
        public string PackageName { get; protected set; }

        /// <summary>
        /// Gets or sets the package version information.
        /// </summary>
        public VersionInfo Version { get; protected set; }

        /// <summary>
        /// Gets or sets the package publisher string.
        /// </summary>
        public string Publisher { get; protected set; }

        /// <summary>
        /// Gets or sets the package family name.
        /// </summary>
        public string PackageFamilyName { get; protected set; }

        /// <summary>
        /// Gets or sets the package full name.
        /// </summary>
        public string PackageFullName { get; protected set; }

        /// <summary>
        /// Gets or sets the Architecture part of package full name
        /// </summary>
        public string Architecture { get; protected set; }

        /// <summary>
        /// Gets or sets the ResourceID part of the package full name
        /// </summary>
        public string ResourceId { get; protected set; }

        /// <summary>
        /// Gets or sets the stream to the blockmap file
        /// </summary>
        public IStream BlockmapStream { get; protected set; }

        /// <summary>
        /// Gets or sets the stream to the manifest file
        /// </summary>
        public IStream ManifestStream { get; protected set; }

        /// <summary>
        /// Populates fields common to both appx and appxbundle files.
        /// </summary>
        protected void PopulateCommonFields()
        {
            // Populate ResourceId and Architecture
            this.Architecture = PackagingUtils.GetPackageArchitectureFromFullName(this.PackageFullName);
            this.ResourceId = PackagingUtils.GetPackageResourceIdFromFullName(this.PackageFullName);
        }
    }
}
