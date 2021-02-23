// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

namespace Microsoft.Msix.Utils.AppxPackaging
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Runtime.InteropServices.ComTypes;
    using Microsoft.Msix.Utils.AppxPackagingInterop;

    /// <summary>
    /// Class that represents msix bundle information.
    /// </summary>
    public class AppxBundleMetadata : PackageMetadata
    {
        private IAppxBundleManifestReader manifestReader;

        /// <summary>
        /// Initializes a new instance of the <see cref="AppxBundleMetadata"/> class.
        /// </summary>
        /// <param name="filePath">the path to the appxbundle file</param>
        public AppxBundleMetadata(string filePath)
        {
            this.Initialize(filePath);
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="AppxBundleMetadata"/> class.
        /// </summary>
        /// <param name="parentBundleInfo">the parent bundle from which this bundle is referenced</param>
        /// <param name="fileRelativePath">the relative path to the appxbundle file, as referenced
        /// from a parent appxbundle file</param>
        public AppxBundleMetadata(AppxBundleMetadata parentBundleInfo, string fileRelativePath)
        {
            this.RelativePath = fileRelativePath;
            string fullPath = Path.Combine(Path.GetDirectoryName(parentBundleInfo.FilePath), fileRelativePath);
            this.Initialize(fullPath);
        }

        /// <summary>
        /// Gets the relative path this bundle, if it was created by following a reference
        /// from a parent bundle.
        /// </summary>
        public string RelativePath { get; private set; }

        /// <summary>
        /// Gets the reader for this bundle
        /// </summary>
        public IAppxBundleReader AppxBundleReader { get; private set; }

        /// <summary>
        /// Gets the list of all the external msix packages referenced by the bundle, which belong to the same family.
        /// </summary>
        public IList<ExternalPackageReference> ExternalAppxPackages { get; private set; } = new List<ExternalPackageReference>();

        /// <summary>
        /// Gets the list of all the internal msix packages bundled in this bundle
        /// </summary>
        public IList<string> InternalAppxPackagesRelativePaths { get; private set; } = new List<string>();

        /// <summary>
        /// Gets the list of all the physical optional msix packages referenced by this bundle, which don't belong to
        /// a physical optional bundle file.
        /// </summary>
        public IList<ExternalPackageReference> OptionalAppxPackages { get; private set; } = new List<ExternalPackageReference>();

        /// <summary>
        /// Gets the list of all the physical optional bundles referenced by this bundle.
        /// </summary>
        public IList<ExternalPackageReference> OptionalAppxBundles { get; private set; } = new List<ExternalPackageReference>();

        private void Initialize(string filePath)
        {
            if (string.IsNullOrEmpty(filePath))
            {
                throw new ArgumentNullException("filePath");
            }

            if (!File.Exists(filePath))
            {
                throw new FileNotFoundException(string.Format("{0} not found", filePath));
            }

            this.FilePath = filePath;
            IStream bundleStream = StreamUtils.CreateInputStreamOnFile(this.FilePath);
            IAppxBundleFactory bundleFactory = (IAppxBundleFactory)new AppxBundleFactory();
            this.AppxBundleReader = bundleFactory.CreateBundleReader(bundleStream);

            this.BlockmapStream = this.AppxBundleReader.GetBlockMap().GetStream();

            this.manifestReader = this.AppxBundleReader.GetManifest();
            this.ManifestStream = this.manifestReader.GetStream();
            IAppxManifestPackageId packageId = this.manifestReader.GetPackageId();

            this.PackageName = packageId.GetName();
            this.PackageFamilyName = packageId.GetPackageFamilyName();
            this.PackageFullName = packageId.GetPackageFullName();
            this.Publisher = packageId.GetPublisher();
            this.Version = new VersionInfo(packageId.GetVersion());

            this.PopulateCommonFields();

            this.PopulateChildAppxPackages();
            this.PopulateOptionalAppxPackagesAndBundles();
        }

        /// <summary>
        /// Reads the manifest and populates the ExternalAppxPackagesRelativePaths and InternalAppxPackagesRelativePaths lists.
        /// </summary>
        private void PopulateChildAppxPackages()
        {
            IAppxBundleManifestPackageInfoEnumerator subPackagesEnumerator = this.manifestReader.GetPackageInfoItems();
            while (subPackagesEnumerator.GetHasCurrent())
            {
                IAppxBundleManifestPackageInfo subPackageInfo = subPackagesEnumerator.GetCurrent();

                // If the package is not contained within the bundle, the corresponding package element in the bundle manifest
                // would not have an offset field. In such a case and only in that case, the AppxPackaging APIs would return 0
                // when retrieving the offset.
                if (subPackageInfo.GetOffset() == 0)
                {
                    this.ExternalAppxPackages.Add(new ExternalPackageReference(
                        this,
                        subPackageInfo.GetPackageId().GetPackageFullName(),
                        subPackageInfo.GetFileName(),
                        isOptional: false));
                }
                else
                {
                    this.InternalAppxPackagesRelativePaths.Add(subPackageInfo.GetFileName());
                }

                subPackagesEnumerator.MoveNext();
            }
        }

        /// <summary>
        /// Reads the manifest and populates the OptionalAppxBundlesRelativePaths and OptionalAppxPackagesRelativePaths lists.
        /// </summary>
        private void PopulateOptionalAppxPackagesAndBundles()
        {
            IAppxBundleManifestReader2 bundleManifestReader2 = (IAppxBundleManifestReader2)this.manifestReader;

            // Iterate over all OptionalBundle elements in the manifest
            IAppxBundleManifestOptionalBundleInfoEnumerator optionalBundlesEnumerator = bundleManifestReader2.GetOptionalBundles();
            while (optionalBundlesEnumerator.GetHasCurrent())
            {
                IAppxBundleManifestOptionalBundleInfo optionalBundleInfo = optionalBundlesEnumerator.GetCurrent();

                if (optionalBundleInfo.GetFileName() != null)
                {
                    // If the file name of the OptionalBundle is not null, it points to a physical optional package bundle, in which
                    // case we recursively create the bundle info for the optional bundle.
                    this.OptionalAppxBundles.Add(new ExternalPackageReference(
                        this,
                        optionalBundleInfo.GetPackageId().GetPackageFullName(),
                        optionalBundleInfo.GetFileName(),
                        isOptional: true));
                }
                else
                {
                    // Otherwise, the PackageInfo child elements of the OptionalBundle element point to the optional msix packages.
                    IAppxBundleManifestPackageInfoEnumerator optionalPackagesEnumerator = optionalBundleInfo.GetPackageInfoItems();

                    while (optionalPackagesEnumerator.GetHasCurrent())
                    {
                        IAppxBundleManifestPackageInfo optionalPackageInfo = optionalPackagesEnumerator.GetCurrent();
                        this.OptionalAppxPackages.Add(new ExternalPackageReference(
                            this,
                            optionalPackageInfo.GetPackageId().GetPackageFullName(),
                            optionalPackageInfo.GetFileName(),
                            isOptional: true));

                        optionalPackagesEnumerator.MoveNext();
                    }
                }

                optionalBundlesEnumerator.MoveNext();
            }
        }
    }
}
