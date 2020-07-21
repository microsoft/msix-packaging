//-----------------------------------------------------------------------
// <copyright file="SDKDetector.cs" company="Microsoft">
//     Copyright (c) Microsoft. All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

namespace Microsoft.Packaging.Utils.ProcessRunner
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using Microsoft.Packaging.Utils.Logger;
    using Microsoft.Win32;

    /// <summary>
    ///  A class that provides functionality for detecting the installed SDK location
    /// </summary>
    public class SDKDetector
    {
        private static readonly string SdkRootRegistryKey = @"SOFTWARE\Microsoft\Windows Kits\Installed Roots";
        private static readonly string SdkRootRegistry64Key = @"SOFTWARE\WOW6432Node\Microsoft\Windows Kits\Installed Roots";
        private static readonly string SdkRootValueName = "KitsRoot10";
        private static readonly string PlatformRelativePath = @"Platforms\UAP";
        private static readonly string SdkBinFolderName = "bin";

        private static Lazy<SDKDetector> factory = new Lazy<SDKDetector>(() => new SDKDetector());

        private string latestSdkBinDirectoryForCurrentArchitecture;
        private string latestSdkBinDirectoryForX86;

        private SDKDetector()
        {
            this.PopulateSdkInfo();
        }

        /// <summary>
        /// Gets the singleton instance of this class
        /// </summary>
        public static SDKDetector Instance
        {
            get { return factory.Value; }
        }

        /// <summary>
        /// Gets the path to the latest detected SDK bin folder
        /// </summary>
        public string LatestSDKBinPath
        {
            get
            {
                return this.latestSdkBinDirectoryForCurrentArchitecture;
            }
        }

        /// <summary>
        /// Inspects the user provided path for the tool and logs warnings about disparity with the SDK
        /// </summary>
        /// <param name="logProviders">Log providers to be used for logging warnings</param>
        /// <param name="userProvidedToolsPath">The use provided path</param>
        public void InspectUserProvidedPath(IReadOnlyList<ILogProvider> logProviders, string userProvidedToolsPath)
        {
            if (this.latestSdkBinDirectoryForCurrentArchitecture == null)
            {
                // If SDK is not detected to be installed, warn
                Logger.Warning(logProviders, "!!! WARNING: The Windows SDK was not detected to be installed.");
            }
            else if (FileSystemUtils.NormalizePath(userProvidedToolsPath) != FileSystemUtils.NormalizePath(this.latestSdkBinDirectoryForCurrentArchitecture)
                && FileSystemUtils.NormalizePath(userProvidedToolsPath) != FileSystemUtils.NormalizePath(this.latestSdkBinDirectoryForX86))
            {
                // If provided path doesn't match the latest detected SDK, warn
                Logger.Warning(
                    logProviders,
                    "!!! WARNING: The provided path for the tool {0} does not match the latest Windows SDK detected located under {1}.",
                    userProvidedToolsPath,
                    this.latestSdkBinDirectoryForCurrentArchitecture);
            }

            Logger.Info(logProviders, "The path {0} will be used for running the tool.", userProvidedToolsPath);
        }

        /// <summary>
        /// Gets the latest version sub directory of the given root. If the version is not in the form of
        /// major.minor[.build[.revision]] or not as integers, falls back by using the newest directory.
        /// </summary>
        /// <param name="platformRootDirectoryPath">Root directory</param>
        /// <returns>Path of subdirectory.</returns>
        public string GetLastInstalledPlatformDirectory(string platformRootDirectoryPath)
        {
            string[] platformDirectories = Directory.GetDirectories(platformRootDirectoryPath);
            string latestPlatformDirectory = null;
            DateTime maxWriteTime = DateTime.MinValue;
            Version latestVersion = new Version("0.0.0.0");

            try
            {
                foreach (string platformDirectory in platformDirectories)
                {
                    DirectoryInfo directoryInfo = new DirectoryInfo(platformDirectory);
                    Logger.Log("Detected installed SDK, version {0}", directoryInfo.Name);

                    Version sdkVersion = new Version(directoryInfo.Name);

                    if (sdkVersion > latestVersion)
                    {
                        latestVersion = sdkVersion;
                        latestPlatformDirectory = platformDirectory;
                    }
                }
            }
            catch (Exception e)
            {
                latestPlatformDirectory = null;
                Logger.Log($"Falling back to use latestWriteTime. Exception {e.ToString()}.");
                foreach (string platformDirectory in platformDirectories)
                {
                    DirectoryInfo directoryInfo = new DirectoryInfo(platformDirectory);
                    Logger.Log("Detected installed SDK, version {0}", directoryInfo.Name);

                    if (directoryInfo.LastWriteTime > maxWriteTime)
                    {
                        maxWriteTime = directoryInfo.LastWriteTime;
                        latestPlatformDirectory = platformDirectory;
                    }
                }
            }

            return latestPlatformDirectory;
        }

        private void PopulateSdkInfo()
        {
            try
            {
                string processorArch = Environment.GetEnvironmentVariable("PROCESSOR_ARCHITECTURE");
                bool is64BitOS = string.Compare(processorArch, "amd64", StringComparison.OrdinalIgnoreCase) == 0;

                // Find SDK root folder
                string registryKeyPath = is64BitOS ? SdkRootRegistry64Key : SdkRootRegistryKey;
                using (RegistryKey sdkKey = Registry.LocalMachine.OpenSubKey(registryKeyPath))
                {
                    if (sdkKey != null)
                    {
                        string sdkRootDirectory = (string)sdkKey.GetValue(SdkRootValueName);

                        if (sdkRootDirectory != null)
                        {
                            string platformRootDirectory = Path.Combine(sdkRootDirectory, PlatformRelativePath);
                            string binRootDirectory = Path.Combine(sdkRootDirectory, SdkBinFolderName);

                            // Find SDK version by choosing most recent folder under Platforms\UAP
                            string latestPlatformDirectoryPath = this.GetLastInstalledPlatformDirectory(platformRootDirectory);

                            if (latestPlatformDirectoryPath != null)
                            {
                                string latestSdkVersion = Path.GetFileName(latestPlatformDirectoryPath);
                                string latestBinRootDirectory = Path.Combine(binRootDirectory, latestSdkVersion);

                                string architecture = is64BitOS ? "x64" : "x86";
                                this.latestSdkBinDirectoryForCurrentArchitecture = Path.Combine(latestBinRootDirectory, architecture);
                                this.latestSdkBinDirectoryForX86 = Path.Combine(latestBinRootDirectory, "x86");
                            }
                        }
                    }
                }
            }
            catch (Exception exp)
            {
                // Unexpected error. Log and ignore
                Logger.Error("Error occurred while finding the location of the Windows SDK: ({0})", exp.Message);
            }
            finally
            {
                if (this.latestSdkBinDirectoryForCurrentArchitecture == null)
                {
                    Logger.Log("The Windows SDK was not detected to be installed");
                }
                else
                {
                    Logger.Log("The latest installed Windows 10 SDK detected is located under {0}", this.latestSdkBinDirectoryForCurrentArchitecture);
                }
            }
        }
    }
}
