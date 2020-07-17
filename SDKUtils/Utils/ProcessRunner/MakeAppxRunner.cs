//-----------------------------------------------------------------------
// <copyright file="MakeAppxRunner.cs" company="Microsoft">
//     Copyright (c) Microsoft. All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

namespace Microsoft.Packaging.SDKUtils.ProcessRunner
{
    using System;
    using System.Globalization;
    using System.IO;
    using System.Linq;

    /// <summary>
    ///  The class to generate an APPX file.
    /// </summary>
    public class MakeAppxRunner : SDKToolProcessRunner
    {
        /// <summary>
        ///  Initializes a new instance of the <see cref="MakeAppxRunner"/> class.
        /// </summary>
        public MakeAppxRunner() : base()
        {
        }

        /// <summary>
        ///  Initializes a new instance of the <see cref="MakeAppxRunner"/> class.
        /// </summary>
        /// <param name="makeAppxDirectory">The directory path of MakeAppx.exe.</param>
        public MakeAppxRunner(string makeAppxDirectory) :
            base(makeAppxDirectory)
        {
        }

        /// <summary>
        /// Gets or sets a value indicating whether the output file from MakeAppx should overwrite
        /// any existing files with the same name.
        /// This controls the flags /o and /no of MakeAppx.
        /// The default is false.
        /// </summary>
        public bool OverwriteExistingFiles { get; set; } = false;

        /// <summary>
        /// Gets or sets a value indicating whether MakeAppx should be run quietly or in verbose mode.
        /// </summary>
        public bool RunQuietly { get; set; } = false;

        protected override string ToolName
        {
            get
            {
                return "MakeAppx";
            }
        }

        /// <summary>
        ///  Performs the appx generation.
        /// </summary>
        /// <param name="projectRootFolderPath">The project root folder path.</param>
        /// <param name="outputAppxFilePath">The file path of the output APPX.</param>
        public void GenerateAppxFromRootFolder(
            string projectRootFolderPath,
            string outputAppxFilePath)
        {
            Logger.Logger.Log(this.LogProviders, "Starting making APPX, project root = {0}", projectRootFolderPath);

            if (string.IsNullOrEmpty(projectRootFolderPath))
            {
                throw new ArgumentException("Folder path is null or empty", "projectRootFolderPath");
            }

            if (!Directory.Exists(projectRootFolderPath))
            {
                throw new ArgumentException("Folder path does not exist", "projectRootFolderPath");
            }

            if (string.IsNullOrEmpty(outputAppxFilePath))
            {
                throw new ArgumentException("Folder path is null or empty", "outputAppxFilePath");
            }

            // In case output file already exists and we are not allowed to overwrite it
            // throw an exception
            if (File.Exists(outputAppxFilePath) && !this.OverwriteExistingFiles)
            {
                throw new ArgumentException(
                    "The output file already exists and overwrite flag is false",
                    "outputAppxFilePath");
            }

            // Run command "MakeAppx.exe pack /l /d <project root> /p <filepath>"
            // /l is used for localized packages. When enabled, the {image name}.{extension}
            //  used in manifest can be resolved with the {image name}.{scale qualifier}.{extension}
            string makeAppxArgs = string.Format(
                CultureInfo.InvariantCulture,
                "pack /l /d \"{0}\" /p \"{1}\" {2} {3}",
                projectRootFolderPath,
                outputAppxFilePath,
                this.OverwriteExistingFiles ? "/o" : "/no",
                this.RunQuietly ? string.Empty : "/v");

            this.SpawnToolProcess(makeAppxArgs);

            Logger.Logger.Log(this.LogProviders, "Making APPX succeeded, path = {0}", outputAppxFilePath);
        }

        /// <summary>
        ///  Performs the appxbundle generation.
        /// </summary>
        /// <param name="appxBundleVersion">The version of the bundle to be generated.</param>
        /// <param name="rootOfAllAppxs">Folder containing all the appx files to be bundled</param>
        /// <param name="outputAppxBundleFilePath">The file path of the output bundle</param>
        public void GenerateAppxBundleFromRootFolder(
            string appxBundleVersion,
            string rootOfAllAppxs,
            string outputAppxBundleFilePath)
        {
            if (string.IsNullOrEmpty(rootOfAllAppxs))
            {
                throw new ArgumentException("Root path is null or empty", "rootOfAllAppxs");
            }

            if (string.IsNullOrEmpty(outputAppxBundleFilePath))
            {
                throw new ArgumentException("Output path is null or empty", "outputAppxBundleFilePath");
            }

            // Run command "MakeAppx.exe bundle /bv <version> /d <project root> /p <bundle path> /no /v"
            string makeAppxArgs = string.Format(
                CultureInfo.InvariantCulture,
                "bundle /bv \"{0}\" /d \"{1}\" /p \"{2}\" {3} {4}",
                appxBundleVersion,
                rootOfAllAppxs,
                outputAppxBundleFilePath,
                this.OverwriteExistingFiles ? "/o" : "/no",
                this.RunQuietly ? string.Empty : "/v");

            this.SpawnToolProcess(makeAppxArgs);

            Logger.Logger.Log(this.LogProviders, "Making APPX Bundle succeeded, path = {0}", outputAppxBundleFilePath);
        }

        /// <summary>
        ///  Performs the bundle generation from a manifest file. THis is needed when the plain flat bundle manifest needs to be updated
        ///  to references the encrypted packages.
        ///  internalPackagesDirectoryPath usually would point to an empty directory for flat bundles, but it can contain appx files
        ///  if some appx files need to be internal.
        /// </summary>
        /// <param name="bundleVersion">The version of the bundle to be generated.</param>
        /// <param name="bundleManifestPath">Path to the bundle manifest path</param>\
        /// <param name="internalPackagesDirectoryPath">Path to the directory containing packages that should be included inside the bundle</param>
        /// <param name="outputBundleFilePath">The file path of the output bundle</param>
        public void GenerateBundleWithProvidedManifest(
            string bundleVersion,
            string bundleManifestPath,
            string internalPackagesDirectoryPath,
            string outputBundleFilePath)
        {
            if (string.IsNullOrEmpty(bundleVersion))
            {
                throw new ArgumentException("Bundle version is null or empty", "bundleVersion");
            }

            if (string.IsNullOrEmpty(bundleManifestPath) || !File.Exists(bundleManifestPath))
            {
                throw new ArgumentException("Bundle manifest file path must point to an existing file", "bundleManifestPath");
            }

            if (string.IsNullOrEmpty(internalPackagesDirectoryPath) || !Directory.Exists(internalPackagesDirectoryPath))
            {
                throw new ArgumentException("Internal packages directory path path must point to an existing directory", "internalPackagesDirectoryPath");
            }

            if (string.IsNullOrEmpty(outputBundleFilePath))
            {
                throw new ArgumentException("Output path is null or empty", "outputBundleFilePath");
            }

            // Run command "MakeAppx.exe bundle /bv <version> /d <directory> /p <bundle path> /obm <manifest path> /no /v"
            string makeAppxArgs = string.Format(
                CultureInfo.InvariantCulture,
                "bundle /bv \"{0}\" /d \"{1}\" /p \"{2}\" /obm \"{3}\" {4} {5}",
                bundleVersion,
                internalPackagesDirectoryPath,
                outputBundleFilePath,
                bundleManifestPath,
                this.OverwriteExistingFiles ? "/o" : "/no",
                this.RunQuietly ? string.Empty : "/v");

            this.SpawnToolProcess(makeAppxArgs);

            Logger.Logger.Log(this.LogProviders, "Making Bundle with provided manifest succeeded, path = {0}", outputBundleFilePath);
        }

        /// <summary>
        /// Generates packages from a given layout file
        /// </summary>
        /// <param name="layoutFilePath">The layout file</param>
        /// <param name="outputFolder">The folder which should contain the output packages</param>
        /// <param name="additionalArgs">Additional arguments to be passed to MakeAppx.exe build</param>
        public void GeneratePackagesFromLayoutFile(
            string layoutFilePath,
            string outputFolder,
            string additionalArgs)
        {
            if (string.IsNullOrEmpty(layoutFilePath))
            {
                throw new ArgumentException("Layout file is null", "layoutFilePath");
            }

            if (!File.Exists(layoutFilePath))
            {
                throw new ArgumentException("Layout path does not exist", "layoutFilePath");
            }

            if (string.IsNullOrEmpty(outputFolder))
            {
                throw new ArgumentException("Output path is null or empty", "outputFolder");
            }

            // Construct the overwrite flag and the verbosity flag based on the argument
            // Note: The passed in additionalArgs take precendence over OverwriteExistingFiles and over RunQuietly
            string[] additionalArgsArray = additionalArgs.Split(' ');

            string overwriteFlag = string.Empty;
            string verboseFlag = string.Empty;

            if (!ContainsOption(additionalArgsArray, "/o", "/overwrite") &&
                !ContainsOption(additionalArgsArray, "/no", "/noOverwrite"))
            {
                overwriteFlag = this.OverwriteExistingFiles ? "/o" : "/no";
            }

            if (!ContainsOption(additionalArgsArray, "/v", "/verbose"))
            {
                verboseFlag = this.RunQuietly ? string.Empty : "/v";
            }

            // Run command "MakeAppx.exe build /f <layout_file> /op <output_folder> <any_additional_arguments>"
            string makeAppxArgs = string.Format(
                CultureInfo.InvariantCulture,
                "build /f \"{0}\" /op \"{1}\" {2} {3} {4}",
                layoutFilePath,
                outputFolder,
                overwriteFlag,
                verboseFlag,
                additionalArgs ?? string.Empty).Trim();

            this.SpawnToolProcess(makeAppxArgs);

            Logger.Logger.Log(this.LogProviders, "Making Packages succeeded, path = {0}", outputFolder);
        }

        /// <summary>
        /// Encrypts a package or bundle
        /// </summary>
        /// <param name="packageOrBundleFilePath">The path to the package or bundle</param>
        /// <param name="outputFilePath">The output path</param>
        /// <param name="keyFilePath">The path to the key file to encrypt with. If null, the test key will be used</param>
        public void EncryptPackageOrBundle(string packageOrBundleFilePath, string outputFilePath, string keyFilePath = null)
        {
            if (string.IsNullOrEmpty(packageOrBundleFilePath) || !File.Exists(packageOrBundleFilePath))
            {
                throw new ArgumentException("Input package or bundle must point to an existing file", "packageOrBundleFilePath");
            }

            if (keyFilePath != null && !File.Exists(keyFilePath))
            {
                throw new ArgumentException("keyFile must point to an existing file when specified", "keyFilePath");
            }

            if (string.IsNullOrEmpty(outputFilePath))
            {
                throw new ArgumentException("Output path is null or empty", "outputFilePath");
            }

            string makeAppxArgs = string.Format(
                CultureInfo.InvariantCulture,
                "encrypt /p \"{0}\" /ep \"{1}\" {2} {3} {4}",
                packageOrBundleFilePath,
                outputFilePath,
                keyFilePath == null ? "/kt" : string.Format("/kf \"{0}\"", keyFilePath),
                this.OverwriteExistingFiles ? "/o" : "/no",
                this.RunQuietly ? string.Empty : "/v");

            this.SpawnToolProcess(makeAppxArgs);

            Logger.Logger.Log(this.LogProviders, "Encryption succeeded, path = {0}", outputFilePath);
        }

        private static bool ContainsOption(string[] args, string optionShortName, string optionLongName)
        {
            return args.Any(a => a.Equals(optionShortName, StringComparison.OrdinalIgnoreCase))
                || args.Any(a => a.Equals(optionLongName, StringComparison.OrdinalIgnoreCase));
        }
    }
}
