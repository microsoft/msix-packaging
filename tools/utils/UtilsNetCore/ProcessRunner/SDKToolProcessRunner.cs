//-----------------------------------------------------------------------
// <copyright file="SDKToolProcessRunner.cs" company="Microsoft">
//     Copyright (c) Microsoft. All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

namespace Microsoft.Packaging.Utils.ProcessRunner
{
    using System;
    using System.IO;
    using Microsoft.Packaging.Utils.Logger;

    /// <summary>
    ///  The class to inherit from for all SDK related process runner tools.
    /// </summary>
    public abstract class SDKToolProcessRunner : SDKToolProcessRunnerBase
    {

        /// <summary>
        ///  Initializes a new instance of the <see cref="SDKToolProcessRunner"/> class using either 
        ///  a side-by-side executable of the tool, or, if not found, the one from the latest installed SDK.
        /// </summary>
        public SDKToolProcessRunner() : base()
        {
            try
            {
                this.FindToolInCurrentAssembly();
            }
            catch (ArgumentException)
            {
                throw new ArgumentException(string.Format(
                    "The tool {0} path cannot be found and cannot be queried from SDK Installation path" +
                    "(.Net Core doesn't have registry functionality, which is used to query path).",
                    this.ToolName + ".EXE"));
            }
        }

        /// <summary>
        ///  Initializes a new instance of the <see cref="SDKToolProcessRunner"/> class using the provided
        ///  path pointing to the tool.
        /// </summary>
        /// <param name="toolDirectory">The directory path of the tool</param>
        public SDKToolProcessRunner(string toolDirectory)
        {
            Logger.Debug(this.LogProviders, toolDirectory);
            this.ValidateToolDirectory(toolDirectory);
        }

        protected override void SpawnToolProcess(string toolArgs)
        {
            Logger.Log(this.LogProviders, "Running Under: .NET CORE");
            using (NetCoreProcessRunner runner = new NetCoreProcessRunner())
            {
                runner.ExePath = this.ToolPath;
                runner.Arguments = toolArgs;
                runner.SetLogProviders(this.LogProviders);
                runner.OutputLogDirectory = this.OutputLogDirectory;

                Logger.Log(this.LogProviders, "Running Process: " + runner.ExePath);
                Logger.Log(this.LogProviders, "With Args: " + runner.Arguments);

                runner.Run();
                runner.ValidateExitCode();
            }

            this.PrintProcessOutputIfApplicable();
        }

        /// <summary>
        /// Log the output explicitly as NetCore does not support std::output routing.
        /// Only log if the file exists.
        /// </summary>
        protected override void PrintProcessOutputIfApplicable()
        {
            // Log file path example:
            //     "C:\Windows\Temp\PackageEditor\818c7d8c-6ebc-4336-82e2-f50bbd103993\log.txt"
            string logDir = Path.Combine(Path.GetTempPath(), this.ToolName);
            string latestDir = FileSystemUtils.GetMostRecentDirectory(logDir);
            string logFile = Path.Combine(latestDir, "log.txt");
            if (File.Exists(logFile))
            {
                Logger.Log(this.LogProviders, "\n\n*******************************************\nOutput from {0} log file", logFile);
                foreach (var line in File.ReadLines(logFile))
                {
                    Logger.Log(this.LogProviders, line);
                }

                Logger.Log(this.LogProviders, "\n*******************************************\n");
            }
        }
    }
}
