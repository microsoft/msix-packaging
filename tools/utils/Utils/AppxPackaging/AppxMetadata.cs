﻿// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

namespace Microsoft.Msix.Utils.AppxPackaging
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Linq;
    using System.Runtime.InteropServices.ComTypes;
    using Microsoft.Msix.Utils.AppxPackagingInterop;

    /// <summary>
    /// Class that represents msix package information.
    /// </summary>
    public class AppxMetadata : PackageMetadata
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="AppxMetadata"/> class for an msix package.
        /// </summary>
        /// <param name="filePath">the path to the msix file</param>
        public AppxMetadata(string filePath)
        {
            if (string.IsNullOrEmpty(filePath))
            {
                throw new ArgumentNullException(nameof(filePath));
            }

            if (!File.Exists(filePath))
            {
                throw new FileNotFoundException($"{filePath} not found");
            }

            this.FilePath = filePath;
            IStream packageStream = StreamUtils.CreateInputStreamOnFile(this.FilePath);
            this.Initialize(packageStream);
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="AppxMetadata"/> class for an msix package.
        /// </summary>
        /// <param name="packageStream">the msix package stream.</param>
        public AppxMetadata(IStream packageStream)
        {
            if (packageStream == null)
            {
                throw new ArgumentNullException(nameof(packageStream));
            }

            this.Initialize(packageStream);
        }

        /// <summary>
        /// Gets the display name of the package
        /// </summary>
        public string DisplayName { get; private set; }

        /// <summary>
        /// Gets the package publisher display name.
        /// </summary>
        public string PublisherDisplayName { get; private set; }

        /// <summary>
        /// Gets the min OS versions for all the devices the package targets
        /// </summary>
        public Dictionary<string, VersionInfo> TargetDeviceFamiliesMinVersions { get; private set; }
            = new Dictionary<string, VersionInfo>();

        /// <summary>
        /// Gets the min OS version across all the devices this package targets
        /// </summary>
        public VersionInfo MinOSVersion { get; private set; }

        /// <summary>
        /// Gets the reader for this package
        /// </summary>
        public IAppxPackageReader AppxReader { get; private set; }

        /// <summary>
        /// Initializes this instance of the AppxMetadata class from an msix stream.
        /// </summary>
        /// <param name="packageStream">the msix package stream.</param>
        private void Initialize(IStream packageStream)
        {
            IAppxFactory packageFactory = (IAppxFactory)new AppxFactory();
            this.AppxReader = packageFactory.CreatePackageReader(packageStream);

            this.BlockmapStream = this.AppxReader.GetBlockMap().GetStream();

            IAppxManifestReader appxManifestReader = this.AppxReader.GetManifest();
            this.ManifestStream = appxManifestReader.GetStream();
            IAppxManifestPackageId packageId = appxManifestReader.GetPackageId();
            this.PackageName = packageId.GetName();
            this.PackageFamilyName = packageId.GetPackageFamilyName();
            this.PackageFullName = packageId.GetPackageFullName();
            this.Publisher = packageId.GetPublisher();
            this.Version = new VersionInfo(packageId.GetVersion());

            IAppxManifestProperties packageProperties = appxManifestReader.GetProperties();
            packageProperties.GetStringValue("DisplayName", out string displayName);
            this.DisplayName = displayName;

            packageProperties.GetStringValue("PublisherDisplayName", out string publisherDisplayName);
            this.PublisherDisplayName = publisherDisplayName;

            // Get the min versions
            IAppxManifestReader3 appxManifestReader3 = (IAppxManifestReader3)appxManifestReader;
            IAppxManifestTargetDeviceFamiliesEnumerator targetDeviceFamiliesEnumerator = appxManifestReader3.GetTargetDeviceFamilies();
            while (targetDeviceFamiliesEnumerator.GetHasCurrent())
            {
                IAppxManifestTargetDeviceFamily targetDeviceFamily = targetDeviceFamiliesEnumerator.GetCurrent();
                this.TargetDeviceFamiliesMinVersions.Add(
                    targetDeviceFamily.GetName(),
                    new VersionInfo(targetDeviceFamily.GetMinVersion()));

                targetDeviceFamiliesEnumerator.MoveNext();
            }

            this.MinOSVersion = this.TargetDeviceFamiliesMinVersions.OrderBy(t => t.Value).FirstOrDefault().Value;

            this.PopulateCommonFields();
        }
    }
}
