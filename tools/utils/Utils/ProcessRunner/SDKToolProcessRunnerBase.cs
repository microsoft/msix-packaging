//-----------------------------------------------------------------------
// <copyright file="SDKToolProcessRunnerBase.cs" company="Microsoft">
//     Copyright (c) Microsoft. All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

namespace Microsoft.Packaging.Utils.ProcessRunner
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Reflection;
    using Microsoft.Packaging.Utils.Logger;

    /// <summary>
    ///  The class to inherit from for all SDK related process runner tools.
    /// </summary>
    public abstract class SDKToolProcessRunnerBase
    {
        /// <summary>
        ///  Initializes a new instance of the <see cref="SDKToolProcessRunnerBase"/> class using either
        ///  a side-by-side executable of the tool, or, if not found, the one from the latest installed SDK.
        /// </summary>
        public SDKToolProcessRunnerBase()
        {
        }

        /// <summary>
        ///  Initializes a new instance of the <see cref="SDKToolProcessRunnerBase"/> class using the provided
        ///  path pointing to the tool.
        /// </summary>
        /// <param name="toolDirectory">The directory path of the tool</param>
        public SDKToolProcessRunnerBase(string toolDirectory)
        {
        }

        /// <summary>
        /// Gets or sets the directory path under which the tool output log will be saved.
        /// If empty, output will be redirected to console.
        /// </summary>
        public string OutputLogDirectory { get; set; }

        /// <summary>
        /// Gets or sets the log providers to be used when logging.
        /// </summary>
        public IReadOnlyList<ILogProvider> LogProviders { get; set; }

        /// <summary>
        /// Gets or sets a value indicating whether to log discreetly.
        /// </summary>
        public ProcessRunnerBase.ProcessRunnerLogLevel DiscreetLogging { get; set; } = ProcessRunnerBase.ProcessRunnerLogLevel.Full;

        /// <summary>
        ///  Gets the tool name
        /// </summary>
        protected abstract string ToolName { get; }

        /// <summary>
        ///  Gets or sets the tool path
        /// </summary>
        protected string ToolPath { get; set; }

        /// <summary>
        /// Sets the log providers for logging
        /// Default is to use the registered providers with the Logger class.
        /// </summary>
        /// <param name="logProviders">Log providers</param>
        public void SetLogProviders(IReadOnlyList<ILogProvider> logProviders)
        {
            this.LogProviders = logProviders;
        }

        /// <summary>
        /// Log standard output.
        /// </summary>
        protected abstract void PrintProcessOutputIfApplicable();

        protected abstract void SpawnToolProcess(string toolArgs);

        /// <summary>
        /// Validates the existence of tool in the directory passed.
        /// </summary>
        /// <param name="toolDirectory">Tool directory.</param>
        protected void ValidateToolDirectory(string toolDirectory)
        {
            this.ToolPath = toolDirectory;
            if (!this.ToolPath.EndsWith(this.ToolName + ".EXE", StringComparison.CurrentCultureIgnoreCase))
            {
                this.ToolPath = Path.Combine(toolDirectory, this.ToolName + ".EXE");
            }

            this.ValidateToolExistance();
        }

        /// <summary>
        /// Looks for the tool next to current assembly.
        /// </summary>
        protected void FindToolInCurrentAssembly()
        {
            this.ToolPath = Path.Combine(
                Path.GetDirectoryName(
                    Assembly.GetExecutingAssembly().Location),
                    this.ToolName + ".EXE");

            this.ValidateToolExistance();
        }

        protected void ValidateToolExistance()
        {
            Logger.Debug(this.LogProviders, this.ToolPath);

            // Validate tool path
            if (string.IsNullOrEmpty(this.ToolPath))
            {
                throw new ArgumentException("File path is null or empty", this.ToolPath);
            }

            // Validate executable name
            string fileName = Path.GetFileName(this.ToolPath);
            if (string.Compare(
                fileName,
                this.ToolName + ".EXE",
                StringComparison.OrdinalIgnoreCase) != 0)
            {
                Logger.Log(
                    this.LogProviders,
                    Logger.LogLevels.Error,
                    "Invalid Tool file name: " + fileName);

                throw new ArgumentException(this.ToolName + ".EXE" + " is expected", this.ToolPath);
            }

            // Validate executable exist
            if (!File.Exists(this.ToolPath))
            {
                Logger.Log(
                    this.LogProviders,
                    Logger.LogLevels.Error,
                    "{0} file can't be found under:: " + this.ToolPath,
                    Path.GetFileName(this.ToolPath));

                throw new ArgumentException(this.ToolName + ".EXE" + " is expected", this.ToolPath);
            }
        }
    }
}
