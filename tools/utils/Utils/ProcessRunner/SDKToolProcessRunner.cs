//-----------------------------------------------------------------------
// <copyright file="SDKToolProcessRunner.cs" company="Microsoft">
//     Copyright (c) Microsoft. All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

namespace Microsoft.Msix.Utils.ProcessRunner
{
    using System;
    using System.IO;
    using System.Text;
    using Microsoft.Msix.Utils.Logger;

    /// <summary>
    ///  The class to inherit from for all SDK related process runner tools.
    /// </summary>
    public abstract class SDKToolProcessRunner : SDKToolProcessRunnerBase, IDisposable
    {
        private DesktopProcessRunner runner = new DesktopProcessRunner();

        /// <summary>
        ///  Initializes a new instance of the <see cref="SDKToolProcessRunner"/> class using either
        ///  a side-by-side executable of the tool, or, if not found, the one from the latest installed SDK.
        /// </summary>
        public SDKToolProcessRunner()
        {
            try
            {
                this.FindToolInCurrentAssembly();
            }
            catch (ArgumentException)
            {
                // Next look for tool in SDK folder
                if (SDKDetector.Instance.LatestSDKBinPath == null)
                {
                    throw new ArgumentException(string.Format(
                        "The tool {0} cannot be found because the Windows SDK was not detected to be installed",
                        this.ToolName + ".EXE"));
                }

                this.ToolPath = Path.Combine(SDKDetector.Instance.LatestSDKBinPath, this.ToolName + ".EXE");
                this.ValidateToolExistance();
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
            SDKDetector.Instance.InspectUserProvidedPath(this.LogProviders, toolDirectory);
            this.ValidateToolDirectory(toolDirectory);
        }

        public void TerminateProcessIfRunning()
        {
            this.runner.TerminateProcessIfRunning();
        }

        public void Dispose()
        {
            this.Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected override void SpawnToolProcess(string toolArgs)
        {
            Logger.Log(this.LogProviders, "Running Under: .NET CLR");

            // The output of the tool's execution is in ASCII.
            this.runner.StandardErrorEncoding = Encoding.ASCII;
            this.runner.StandardOutputEncoding = Encoding.ASCII;

            this.runner.ExePath = this.ToolPath;
            this.runner.Arguments = toolArgs;
            this.runner.SetLogProviders(this.LogProviders);
            this.runner.OutputLogDirectory = this.OutputLogDirectory;

            Logger.Log(this.LogProviders, "Running Process: " + this.runner.ExePath);

            // The arguments can include private data (i.e. passwords, paths with user info)
            // if discreet logging is specified - don't log
            if (this.DiscreetLogging > ProcessRunnerBase.ProcessRunnerLogLevel.Private)
            {
                Logger.Log(this.LogProviders, "With Args: " + this.runner.Arguments);
            }

            this.runner.Run();

            ((DesktopProcessRunner)this.runner).DumpStandardOutput();
            ((DesktopProcessRunner)this.runner).DumpStandardError();

            this.runner.ValidateExitCode();
        }

        /// <summary>
        /// Process Runner takes care of routing the output to Logger.
        /// </summary>
        protected override void PrintProcessOutputIfApplicable()
        {
            return;
        }

        protected void Dispose(bool disposing)
        {
            if (disposing)
            {
                if (this.runner != null)
                {
                    this.runner.Dispose();
                }
            }
        }
    }
}
