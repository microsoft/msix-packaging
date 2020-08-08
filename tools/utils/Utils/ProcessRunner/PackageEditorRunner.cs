//-----------------------------------------------------------------------
// <copyright file="PackageEditorRunner.cs" company="Microsoft">
//     Copyright (c) Microsoft. All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

namespace Microsoft.Msix.Utils.ProcessRunner
{
    using System;
    using System.Globalization;
    using System.IO;
    using Microsoft.Msix.Utils.Logger;

    /// <summary>
    ///  The base class for Package Editor tool.
    /// </summary>
    public sealed class PackageEditorRunner : SDKToolProcessRunner
    {
        /// <summary>
        /// The prefix of test cloud stream uri.
        /// </summary>
        private const string CloudStreamPathPrefix = "http://cloudstreamtest?localpath=";

        /// <summary>
        ///  Initializes a new instance of the <see cref="PackageEditorRunner"/> class.
        /// </summary>
        public PackageEditorRunner() : base()
        {
        }

        /// <summary>
        ///  Initializes a new instance of the <see cref="PackageEditorRunner"/> class.
        /// </summary>
        /// <param name="packageEditorDirectory">The directory path of PackageEditor.exe.</param>
        public PackageEditorRunner(string packageEditorDirectory) :
            base(packageEditorDirectory)
        {
        }

        /// <summary>
        /// Gets or sets a value indicating whether the output file from PackageEditor should overwrite
        /// any existing file with the same name.
        /// The default is false.
        /// </summary>
        public bool OverwriteExistingOutput { get; set; } = true;

        protected override string ToolName
        {
            get
            {
                return "PackageEditor";
            }
        }

        /// <summary>
        /// Create delta package from updated package based on baseline package.
        /// </summary>
        /// <param name="updatedPackagePath">Path to updated msix package.</param>
        /// <param name="baselinePackagePath">Path to baseline package</param>
        /// <param name="deltaPackagePath">Path to delta OPC package.</param>
        /// <param name="workingDirectory">Output path for temporary files.</param>
        /// <param name="cloudStreamAssemblyName">The cloud stream handler.</param>
        /// <param name="cloudStreamClassTypeName">The cloud stream class type name.</param>
        public void CreateDeltaPackage(
            string updatedPackagePath,
            string baselinePackagePath,
            string deltaPackagePath,
            string workingDirectory,
            string cloudStreamAssemblyName = null,
            string cloudStreamClassTypeName = null)
        {
            string updatedPackagePathToValidate = updatedPackagePath;
            string baselinePackagePathToValidate = baselinePackagePath;
            bool isCloudStream;
            this.ValidateAndUpdateCloudParams(
                cloudStreamAssemblyName,
                cloudStreamClassTypeName,
                ref updatedPackagePathToValidate,
                ref baselinePackagePathToValidate,
                out isCloudStream);

            Logger.Log(this.LogProviders, "Starting Package Editor - CreateDeltaPackage");

            if (string.IsNullOrWhiteSpace(updatedPackagePathToValidate))
            {
                throw new ArgumentException("Updated package path is null or empty", "updatedPackagePath");
            }

            if (!File.Exists(updatedPackagePathToValidate))
            {
                throw new ArgumentException("Updated package path does not exist", "updatedPackagePath");
            }

            if (string.IsNullOrWhiteSpace(baselinePackagePathToValidate))
            {
                throw new ArgumentException("Baseline package path is null or empty", "baselinePackagePath");
            }

            if (!File.Exists(baselinePackagePathToValidate))
            {
                throw new ArgumentException("Baseline package path does not exist", "baselinePackagePath");
            }

            if (string.IsNullOrWhiteSpace(deltaPackagePath))
            {
                throw new ArgumentException("Delta Package path is null or empty", "deltaPackagePath");
            }

            if (string.IsNullOrWhiteSpace(workingDirectory))
            {
                throw new ArgumentException("Working directory path is null or empty", "workingDirectory");
            }

            string packageEditorArgs = string.Format(
                CultureInfo.InvariantCulture,
                "createDelta -up {0} -bp {1} -dp {2} -wd {3} {4} {5} {6}",
                updatedPackagePath,
                baselinePackagePath,
                deltaPackagePath,
                workingDirectory,
                isCloudStream ? "-ha " + cloudStreamAssemblyName : string.Empty,
                isCloudStream ? "-ht " + cloudStreamClassTypeName : string.Empty,
                this.OverwriteExistingOutput ? "-o" : string.Empty);

            this.SpawnToolProcess(packageEditorArgs);

            Logger.Log(
                this.LogProviders,
                "Package Editor succeeded, Created Delta package Path = {0}",
                deltaPackagePath);
        }

        /// <summary>
        /// Create delta package from updated package based on baseline package.
        /// </summary>
        /// <param name="updatedPackagePath">Path to updated msix package.</param>
        /// <param name="baselineBlockMapPath">Path to baseline package blockmap.</param>
        /// <param name="baselinePackageFullName">Baseline package full name.</param>
        /// <param name="deltaPackagePath">Path to delta OPC package.</param>
        /// <param name="workingDirectory">Output path for temporary files.</param>
        /// <param name="cloudStreamAssemblyName">The cloud stream handler.</param>
        /// <param name="cloudStreamClassTypeName">The cloud stream class type name.</param>
        public void CreateDeltaPackageUsingBaselineBlockMap(
            string updatedPackagePath,
            string baselineBlockMapPath,
            string baselinePackageFullName,
            string deltaPackagePath,
            string workingDirectory,
            string cloudStreamAssemblyName = null,
            string cloudStreamClassTypeName = null)
        {
            string updatedPackagePathToValidate = updatedPackagePath;
            string baselineBlockMapPathToValidate = baselineBlockMapPath;
            bool isCloudStream;
            this.ValidateAndUpdateCloudParams(
                cloudStreamAssemblyName,
                cloudStreamClassTypeName,
                ref updatedPackagePathToValidate,
                ref baselineBlockMapPathToValidate,
                out isCloudStream);

            Logger.Log(this.LogProviders, "Starting Package Editor - CreateDeltaPackageUsingBaselineBlockMap");

            if (string.IsNullOrWhiteSpace(updatedPackagePathToValidate))
            {
                throw new ArgumentException("Updated package path is null or empty", "updatedPackagePathToValidate");
            }

            if (!File.Exists(updatedPackagePathToValidate))
            {
                throw new ArgumentException("Updated package path does not exist", "updatedPackagePathToValidate");
            }

            if (string.IsNullOrWhiteSpace(baselineBlockMapPathToValidate))
            {
                throw new ArgumentException("Baseline blockmap path is null or empty", "baselineBlockMapPathToValidate");
            }

            if (!File.Exists(baselineBlockMapPathToValidate))
            {
                throw new ArgumentException("Baseline blockmap path does not exist", "baselineBlockMapPathToValidate");
            }

            if (string.IsNullOrWhiteSpace(baselinePackageFullName))
            {
                throw new ArgumentException("Baseline package full name is null or empty", "baselinePackageFullName");
            }

            if (string.IsNullOrWhiteSpace(deltaPackagePath))
            {
                throw new ArgumentException("Delta Package path is null or empty", "deltaPackagePath");
            }

            if (string.IsNullOrWhiteSpace(workingDirectory))
            {
                throw new ArgumentException("Working directory path is null or empty", "workingDirectory");
            }

            string packageEditorArgs = string.Format(
                CultureInfo.InvariantCulture,
                "createDelta -up {0} -bb {1} -bn {2} -dp {3} -wd {4} {5} {6} {7}",
                updatedPackagePath,
                baselineBlockMapPath,
                baselinePackageFullName,
                deltaPackagePath,
                workingDirectory,
                isCloudStream ? "-ha " + cloudStreamAssemblyName : string.Empty,
                isCloudStream ? "-ht " + cloudStreamClassTypeName : string.Empty,
                this.OverwriteExistingOutput ? "-o" : string.Empty);

            this.SpawnToolProcess(packageEditorArgs);

            Logger.Log(
                this.LogProviders,
                "Package Editor succeeded, Created Delta package Path = {0}",
                deltaPackagePath);
        }

        /// <summary>
        /// Update baseline package using delta package.
        /// </summary>
        /// <param name="baselinePackagePath">Path to baseline package</param>
        /// <param name="deltaPackagePath">Path to delta OPC package.</param>
        /// <param name="workingDirectory">Output path for temporary files.</param>
        /// <param name="cloudStreamAssemblyName">The cloud stream handler.</param>
        /// <param name="cloudStreamClassTypeName">The cloud stream class type name.</param>
        public void UpdatePackage(
            string baselinePackagePath,
            string deltaPackagePath,
            string workingDirectory,
            string cloudStreamAssemblyName = null,
            string cloudStreamClassTypeName = null)
        {
            string baselinePackagePathToValidate = baselinePackagePath;
            string deltaPackagePathToValidate = deltaPackagePath;
            bool isCloudStream;
            this.ValidateAndUpdateCloudParams(
                cloudStreamAssemblyName,
                cloudStreamClassTypeName,
                ref deltaPackagePathToValidate,
                ref baselinePackagePathToValidate,
                out isCloudStream);

            Logger.Log(this.LogProviders, "Starting Package Editor - UpdatePackage");

            if (string.IsNullOrWhiteSpace(baselinePackagePathToValidate))
            {
                throw new ArgumentException("Baseline package path is null or empty", "baselinePackagePathToValidate");
            }

            if (!File.Exists(baselinePackagePathToValidate))
            {
                throw new ArgumentException("Baseline package path does not exist", "baselinePackagePathToValidate");
            }

            if (string.IsNullOrWhiteSpace(deltaPackagePathToValidate))
            {
                throw new ArgumentException("Delta Package path is null or empty", "deltaPackagePathToValidate");
            }

            if (string.IsNullOrWhiteSpace(workingDirectory))
            {
                throw new ArgumentException("Working directory path is null or empty", "workingDirectory");
            }

            string packageEditorArgs = string.Format(
                CultureInfo.InvariantCulture,
                "update -appendDelta -bp {0} -dp {1} -wd {2} {3} {4}",
                baselinePackagePath,
                deltaPackagePath,
                workingDirectory,
                isCloudStream ? "-ha " + cloudStreamAssemblyName : string.Empty,
                isCloudStream ? "-ht " + cloudStreamClassTypeName : string.Empty);

            this.SpawnToolProcess(packageEditorArgs);

            Logger.Log(
                this.LogProviders,
                "Package Editor succeeded, Updated package Path = {0}",
                deltaPackagePath);
        }

        /// <summary>
        /// Update baseline encrypted package using delta appended unencrypted package.
        /// </summary>
        /// <param name="baselineEncryptedPackagePath">Path to baseline package</param>
        /// <param name="deltaAppendedPackagePath">Path to delta appended unencrypted package.</param>
        /// <param name="keyFilePath">Encryption key file path. If not specified, using global developer test key</param>
        /// <param name="workingDirectory">Output path for temporary files.</param>
        /// <param name="cloudStreamAssemblyName">The cloud stream handler.</param>
        /// <param name="cloudStreamClassTypeName">The cloud stream class type name.</param>
        public void UpdateEncrypted(
            string baselineEncryptedPackagePath,
            string deltaAppendedPackagePath,
            string keyFilePath,
            string workingDirectory,
            string cloudStreamAssemblyName = null,
            string cloudStreamClassTypeName = null)
        {
            string baselineEncryptedPackagePathToValidate = baselineEncryptedPackagePath;
            string deltaAppendedPackagePathToValidate = deltaAppendedPackagePath;
            bool isCloudStream;
            this.ValidateAndUpdateCloudParams(
                cloudStreamAssemblyName,
                cloudStreamClassTypeName,
                ref deltaAppendedPackagePathToValidate,
                ref baselineEncryptedPackagePathToValidate,
                out isCloudStream);

            Logger.Log(this.LogProviders, "Starting Package Editor - UpdateEncrypted");

            if (string.IsNullOrWhiteSpace(baselineEncryptedPackagePathToValidate))
            {
                throw new ArgumentException("Baseline encrypted package path is null or empty", "baselineEncryptedPackagePathToValidate");
            }

            if (!File.Exists(baselineEncryptedPackagePathToValidate))
            {
                throw new ArgumentException("Baseline encrypted package path does not exist", "baselineEncryptedPackagePathToValidate");
            }

            if (string.IsNullOrWhiteSpace(workingDirectory))
            {
                throw new ArgumentException("Working directory path is null or empty", "workingDirectory");
            }

            if ((!string.IsNullOrWhiteSpace(keyFilePath)) && (!File.Exists(keyFilePath)))
            {
                throw new ArgumentException(
                    "Encryption key file is specified but can't be found",
                    "keyFilePath");
            }

            string packageEditorArgs = string.Format(
                CultureInfo.InvariantCulture,
                "updateEncrypted -appendDelta -bep {0} -dap {1} {2} -wd {3} {4} {5}",
                baselineEncryptedPackagePath,
                deltaAppendedPackagePath,
                !string.IsNullOrWhiteSpace(keyFilePath) ? "-kf " + keyFilePath : "-kt",
                workingDirectory,
                isCloudStream ? "-ha " + cloudStreamAssemblyName : string.Empty,
                isCloudStream ? "-ht " + cloudStreamClassTypeName : string.Empty);

            this.SpawnToolProcess(packageEditorArgs);

            Logger.Log(
                this.LogProviders,
                "Package Editor succeeded, Updated package Path = {0}",
                deltaAppendedPackagePath);
        }

        /// <summary>
        /// Update the manifest within the given package.
        /// </summary>
        /// <param name="packagePath">Path to the package to be updated.</param>
        /// <param name="packageIsEncrypted">Whether the package is encrypted.</param>
        /// <param name="newManifestPath">Path to the manifest to place within the package.</param>
        /// <param name="cloudStreamAssemblyName">The cloud stream handler.</param>
        /// <param name="cloudStreamClassTypeName">The cloud stream class type name.</param>
        /// <param name="skipManifestValidation">True to skip validating that the updated package will be deployable.</param>
        /// <param name="localized">Whether the manifest is localized.</param>
        public void UpdateManifestInPackage(
            string packagePath,
            bool packageIsEncrypted,
            string newManifestPath,
            string cloudStreamAssemblyName = null,
            string cloudStreamClassTypeName = null,
            bool skipManifestValidation = false,
            bool localized = false)
        {
            Logger.Log(this.LogProviders, "Starting Package Editor - UpdateManifest");

            if (string.IsNullOrWhiteSpace(packagePath))
            {
                throw new ArgumentException("Package path is null or empty", "packagePath");
            }

            if (!packagePath.ToLower(CultureInfo.InvariantCulture).StartsWith("http", StringComparison.Ordinal) &&
                !File.Exists(packagePath))
            {
                throw new ArgumentException("Package path does not exist", "packagePath");
            }

            string packageEditorArgs = string.Format(
                CultureInfo.InvariantCulture,
                "updateManifest {0} {1} -m {2} {3} {4} {5} {6}",
                packageIsEncrypted ? "-ep" : "-p",
                packagePath,
                newManifestPath,
                !string.IsNullOrWhiteSpace(cloudStreamAssemblyName) ? "-ha " + cloudStreamAssemblyName : string.Empty,
                !string.IsNullOrWhiteSpace(cloudStreamClassTypeName) ? "-ht " + cloudStreamClassTypeName : string.Empty,
                skipManifestValidation ? "-nv" : string.Empty,
                localized ? "-l" : string.Empty);

            this.SpawnToolProcess(packageEditorArgs);

            Logger.Log(
                this.LogProviders,
                "Package Editor succeeded, New manifest Path = {0}",
                newManifestPath);
        }

        private void ValidateAndUpdateCloudParams(
            string cloudStreamAssemblyName,
            string cloudStreamClassTypeName,
            ref string updatedPackagePath,
            ref string baselinePackageOrBlockMapPath,
            out bool isCloudStream)
        {
            isCloudStream = !string.IsNullOrWhiteSpace(cloudStreamAssemblyName) ||
                !string.IsNullOrWhiteSpace(cloudStreamClassTypeName);

            Logger.Log(
                this.LogProviders,
                "Is Cloud Stream: {0}, Cloud Stream Assembly Name: {1}, Cloud Stream Class Type Name: {2}",
                isCloudStream,
                cloudStreamAssemblyName,
                cloudStreamClassTypeName);

            if (isCloudStream)
            {
                if (string.IsNullOrWhiteSpace(cloudStreamAssemblyName) ||
                    string.IsNullOrWhiteSpace(cloudStreamClassTypeName))
                {
                    throw new ArgumentException("Both Cloud Stream Assmebly name and Class type name should not be null or empty");
                }

                Logger.Log(
                    this.LogProviders,
                    "Cloud Stream paths (before), updatedPackagePath: {0}, baselinePackageOrBlockMapPath: {1}",
                    updatedPackagePath,
                    baselinePackageOrBlockMapPath);

                // Our test cloud stream has a prefix: http://cloudstreamtest?localpath=,
                // This needs to be trimmed, as it actually points to a local file.
                if (updatedPackagePath.StartsWith(CloudStreamPathPrefix, StringComparison.OrdinalIgnoreCase))
                {
                    updatedPackagePath = updatedPackagePath.Replace(CloudStreamPathPrefix, string.Empty);
                }

                if (baselinePackageOrBlockMapPath.StartsWith(CloudStreamPathPrefix, StringComparison.OrdinalIgnoreCase))
                {
                    baselinePackageOrBlockMapPath = baselinePackageOrBlockMapPath.Replace(CloudStreamPathPrefix, string.Empty);
                }

                Logger.Log(
                    this.LogProviders,
                    "Cloud Stream paths (after), updatedPackagePath: {0}, baselinePackagePath: {1}",
                    updatedPackagePath,
                    baselinePackageOrBlockMapPath);
            }
        }
    }
}
