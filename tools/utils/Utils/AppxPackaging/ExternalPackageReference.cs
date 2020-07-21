//-----------------------------------------------------------------------
// <copyright file="ExternalPackageReference.cs" company="Microsoft">
//     Copyright (c) Microsoft. All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

namespace Microsoft.Packaging.SDKUtils.AppxPackaging
{
    using System;
    using System.IO;

    /// <summary>
    /// Class that represents information about a package (appx or appxbundle) referenced from a bundle.
    /// </summary>
    public class ExternalPackageReference
    {
        private Lazy<PackageMetadata> packageInfoFactory;

        /// <summary>
        /// Initializes a new instance of the ExternalPackageReference class.
        /// </summary>
        /// <param name="parentBundle">The parent bundle in which this reference was found</param>
        /// <param name="packageFullName">The full name of the package</param>
        /// <param name="relativePath">The relative path of this package as specified in the parent bundle</param>
        /// <param name="isOptional">a flag indicating if this package is optional w.r.t its parent</param>
        public ExternalPackageReference(
            AppxBundleMetadata parentBundle,
            string packageFullName,
            string relativePath,
            bool isOptional)
        {
            this.PackageFullName = packageFullName;
            this.FullPath = Path.Combine(Path.GetDirectoryName(parentBundle.FilePath), relativePath);
            this.ParentBundle = parentBundle;
            this.RelativePath = relativePath;
            this.IsOptional = isOptional;
            this.IsBundle = FileExtensionHelper.HasUnencryptedBundleExtension(this.FullPath);

            this.packageInfoFactory = new Lazy<PackageMetadata>(() =>
            {
                if (this.IsBundle)
                {
                    return new AppxBundleMetadata(this.FullPath);
                }
                else
                {
                    return new AppxMetadata(this.FullPath);
                }
            });
        }

        /// <summary>
        /// Gets the full name of the package
        /// </summary>
        public string PackageFullName { get; private set; }

        /// <summary>
        /// Gets the full path of this package.
        /// </summary>
        public string FullPath { get; private set; }

        /// <summary>
        /// Gets the parent bundle-relative path of this package.
        /// </summary>
        public string RelativePath { get; private set; }

        /// <summary>
        /// Gets a value indicating whether this package is optional w.r.t its parent.
        /// </summary>
        public bool IsOptional { get; private set; }

        /// <summary>
        /// Gets a value indicating whether the reference is for a bundle or a package.
        /// </summary>
        public bool IsBundle { get; private set; }

        /// <summary>
        /// Gets the information of the parent bundle which contains this reference
        /// </summary>
        public AppxBundleMetadata ParentBundle { get; private set; }

        /// <summary>
        /// Gets the information of the referenced package.
        /// The package will be lazily parsed when this property accessed.
        /// </summary>
        public PackageMetadata PackageInfo
        {
            get
            {
                return this.packageInfoFactory.Value;
            }
        }
    }
}
