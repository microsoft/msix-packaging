//-----------------------------------------------------------------------
// <copyright file="ComparePackageRunner.cs" company="Microsoft">
//     Copyright (c) Microsoft. All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

namespace Microsoft.Packaging.Utils.ProcessRunner
{
    using System;
    using System.Globalization;
    using System.IO;
    using Microsoft.Packaging.Utils.Logger;

    /// <summary>
    ///  The class to run Compare package tool.
    /// </summary>
    public class ComparePackageRunner : SDKToolProcessRunner
    {
        /// <summary>
        ///  Initializes a new instance of the <see cref="ComparePackageRunner"/> class.
        /// </summary>
        public ComparePackageRunner() : base()
        {
        }

        /// <summary>
        ///  Initializes a new instance of the <see cref="ComparePackageRunner"/> class.
        /// </summary>
        /// <param name="comparePackageDirectory">The directory path of ComparePackage.exe.</param>
        public ComparePackageRunner(string comparePackageDirectory) :
            base(comparePackageDirectory)
        {
        }

        /// <summary>
        /// Gets or sets a value indicating whether the output file from ComparePackage should overwrite
        /// any existing file with the same name.
        /// The default is false.
        /// </summary>
        public bool OverwriteExistingOutputXmlFile { get; set; } = false;

        /// <summary>
        /// Gets or sets a value indicating whether ComparePackage should be run quietly or in verbose mode.
        /// </summary>
        public bool RunQuietly { get; set; } = false;

        /// <summary>
        /// Gets or sets the directory path under which the output log from ComparePackage will be saved
        /// if empty, output will be redirected to console.
        /// </summary>
        protected override string ToolName
        {
            get
            {
                return "ComparePackage";
            }
        }

        /// <summary>
        ///  Performs the packages comparison.
        ///  Note: Both paths should be of the same type.
        /// </summary>
        /// <param name="oldPackageOrBlockmapPath">The old package or blockmap path.</param>
        /// <param name="newPackageOrBlockmapPath">The new package or blockmap path.</param>
        /// <param name="xmlOutputPath">The xml output file path.</param>
        public void ComparePackages(
            string oldPackageOrBlockmapPath,
            string newPackageOrBlockmapPath,
            string xmlOutputPath)
        {
            Logger.Log(this.LogProviders, "Starting Compare Package");

            if (string.IsNullOrEmpty(oldPackageOrBlockmapPath))
            {
                throw new ArgumentException("Old package or blockmap path is null or empty", "oldPackageOrBlockmapPath");
            }

            if (!File.Exists(oldPackageOrBlockmapPath))
            {
                throw new ArgumentException("Old package or blockmap path does not exist", "oldPackageOrBlockmapPath");
            }

            if (string.IsNullOrEmpty(newPackageOrBlockmapPath))
            {
                throw new ArgumentException("New package or blockmap path is null or empty", "newPackageOrBlockmapPath");
            }

            if (!File.Exists(newPackageOrBlockmapPath))
            {
                throw new ArgumentException("New package or blockmap path does not exist", "newPackageOrBlockmapPath");
            }

            if (string.IsNullOrEmpty(xmlOutputPath))
            {
                throw new ArgumentException("XML output path is null or empty", "xmlOutputPath");
            }

            // In case output file already exists and we are not allowed to overwrite it
            // throw an exception
            if (File.Exists(xmlOutputPath) && !this.OverwriteExistingOutputXmlFile)
            {
                throw new ArgumentException(
                    "The output XML file already exists and overwrite flag is false",
                    xmlOutputPath);
            }

            // Run command ComparePackage mypackage_1.0_x64.appx mypackage_1.1_x64.appx -XML \"C:\\diffoutputs\\mypackage_1.04_1.05_diff.xml\ -o -v"
            string comparePackageArgs = string.Format(
                CultureInfo.InvariantCulture,
                "{0} {1} {2} {3} {4}",
                oldPackageOrBlockmapPath,
                newPackageOrBlockmapPath,
                string.IsNullOrWhiteSpace(xmlOutputPath) ? string.Empty : "-XML \"" + xmlOutputPath + "\"",
                this.OverwriteExistingOutputXmlFile ? "-o" : string.Empty,
                this.RunQuietly ? string.Empty : "-v");

            this.SpawnToolProcess(comparePackageArgs);

            Logger.Log(this.LogProviders, "Compare Package succeeded, XML Output Path = {0}", xmlOutputPath);
        }
    }
}
