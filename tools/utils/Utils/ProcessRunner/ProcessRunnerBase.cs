//-----------------------------------------------------------------------
// <copyright file="ProcessRunnerBase.cs" company="Microsoft">
//     Copyright (c) Microsoft. All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

namespace Microsoft.Msix.Utils.ProcessRunner
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Text;
    using Microsoft.Msix.Utils.Logger;

    /// <summary>
    /// Provides a base from which all process runner implementations derive from.
    /// </summary>
    public abstract class ProcessRunnerBase : IProcessRunner
    {
        private readonly List<string> standardOutput = new List<string>();
        private readonly List<string> standardError = new List<string>();

        private readonly object syncObject = new object();

        public enum ProcessRunnerLogLevel
        {
            /// <summary>
            /// Hides Paths, username and process arguments
            /// </summary>
            Private = 0,

            /// <summary>
            /// Hides Paths and username
            /// </summary>
            Partial = 1,

            /// <summary>
            /// Hides nothing
            /// </summary>
            Full = 2
        }

        /// <summary>
        /// Gets the log providers to be used when logging.
        /// </summary>
        public IReadOnlyList<ILogProvider> LogProviders { get; private set; }

        /// <summary>
        /// Gets the StandardOutput list.
        /// </summary>
        public IReadOnlyList<string> StandardOutput
        {
            get
            {
                lock (this.syncObject)
                {
                    return (IReadOnlyList<string>)this.standardOutput;
                }
            }
        }

        /// <summary>
        /// Gets the StandardError list.
        /// </summary>
        public IReadOnlyList<string> StandardError
        {
            get
            {
                lock (this.syncObject)
                {
                    return (IReadOnlyList<string>)this.standardError;
                }
            }
        }

        /// <summary>
        /// Gets or sets a value indicating the ExitCode.
        /// </summary>
        public int ExitCode { get; protected set; }

        /// <summary>
        /// Gets or sets a value indicating the ExePath.
        /// </summary>
        public string ExePath { get; set; }

        /// <summary>
        /// Gets or sets a value indicating the Arguments.
        /// </summary>
        public string Arguments { get; set; }

        /// <summary>
        /// Gets or sets a value indicating whether to create a new window.
        /// </summary>
        public bool CreateNoWindow { get; set; } = false;

        /// <summary>
        /// Gets or sets a value indicating whether to log discreetly.
        /// </summary>
        public ProcessRunnerLogLevel DiscreetLogging { get; set; } = ProcessRunnerLogLevel.Full;

        /// <summary>
        /// Gets or sets a value indicating the StandardOutputEncoding.
        /// </summary>
        public Encoding StandardOutputEncoding { get; set; } = Encoding.ASCII;

        /// <summary>
        /// Gets or sets a value indicating the StandardErrorEncoding.
        /// </summary>
        public Encoding StandardErrorEncoding { get; set; } = Encoding.ASCII;

        /// <summary>
        /// Gets or sets a value indicating whether StandardOutputRedirection is supported.
        /// </summary>
        public bool SupportsStandardOutputRedirection { get; protected set; }

        /// <summary>
        /// Gets or sets a value indicating whether StandardErrorRedirection is supported.
        /// </summary>
        public bool SupportsStandardErrorRedirection { get; protected set; }

        /// <summary>
        /// Gets or sets the path under which the output log from the process 
        /// will be saved. If empty, output will be redirected to console.
        /// </summary>
        public string OutputLogDirectory { get; set; }

        /// <summary>
        /// Gets or sets the working directory for the process to be started.
        /// </summary>
        public string WorkingDirectory { get; set; }

        /// <summary>
        /// Gets or sets a value indicating whether the process has started.
        /// </summary>
        protected bool HasStarted { get; set; }

        /// <summary>
        /// Gets or sets a value indicating whether the process has finished.
        /// </summary>
        protected bool HasFinished { get; set; }

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
        /// Spawn the process and wait infinite time for it to finish
        /// </summary>
        public void Run()
        {
            // Int32.MaxValue represents infinity to the operating system.
            this.RunAndWait(int.MaxValue);
        }

        /// <summary>
        /// Spawn the process and wait for it to finish with timeout
        /// </summary>
        /// <param name="milliseconds">The amount of time, in milliseconds,
        /// to wait for the associated process to exit.
        /// The maximum is the largest possible value of a 32-bit integer,
        /// which represents infinity to the operating system.</param>
        public void RunAndWait(int milliseconds)
        {
            lock (this.syncObject)
            {
                if (this.HasStarted)
                {
                    throw new InvalidOperationException(
                        "Create a new instance of this class to run the process again.");
                }

                this.HasStarted = true;
            }

            this.CheckPaths();

            try
            {
                this.OnLaunchProcess();
                bool hasFinished = this.OnWaitForExitOrTimeout(milliseconds);
                if (!hasFinished)
                {
                    Logger.Log(
                        this.LogProviders,
                        Logger.LogLevels.Warning,
                        Path.GetFileName(this.ExePath) + " did not complete within the specified timeout.");

                    throw new ProcessRunnerException(Path.GetFileName(this.ExePath), -1, this.OutputLogDirectory);
                }
            }
            finally
            {
                this.TerminateProcessIfRunning();
            }
        }

        public void TerminateProcessIfRunning()
        {
            if (this.HasStarted && !this.HasFinished)
            {
                Logger.Log(this.LogProviders, Logger.LogLevels.Debug, "Terminating process");
                this.OnTerminateRunningProcess();
            }
        }

        public void ValidateExitCode(int[] validCodes = null)
        {
            if ((validCodes == null && this.ExitCode != 0)
                || (validCodes != null && !Array.Exists(validCodes, element => element == this.ExitCode)))
            {
                Logger.Log(
                    this.LogProviders,
                    Logger.LogLevels.Error,
                    "{0} failed, exit code = {1}",
                    Path.GetFileName(this.ExePath),
                    this.ExitCode);

                throw new ProcessRunnerException(
                    Path.GetFileName(this.ExePath),
                    this.ExitCode,
                    this.OutputLogDirectory);
            }
        }

        public void Dispose()
        {
            this.Dispose(true);
            GC.SuppressFinalize(this);
        }

        /// <summary>
        /// When overridden in a child class, executes the process.
        /// </summary>
        protected abstract void OnLaunchProcess();

        /// <summary>
        /// When overridden in a child class, waits for the current process to terminate or for a timeout to occur.
        /// </summary>
        /// <param name="timeoutMilliseconds">The timeout in milliseconds.</param>
        /// <returns>Whether the process completed within the timeout.</returns>
        protected abstract bool OnWaitForExitOrTimeout(int timeoutMilliseconds);

        /// <summary>
        /// When override in a child class, terminates the running process.
        /// </summary>
        protected abstract void OnTerminateRunningProcess();

        protected virtual void Dispose(bool disposing)
        {
        }

        protected void AddStandardOutputEntry(string entry)
        {
            lock (this.syncObject)
            {
                this.standardOutput.Add(entry);
            }
        }

        protected void AddStandardErrorEntry(string entry)
        {
            lock (this.syncObject)
            {
                this.standardError.Add(entry);
            }
        }

        private void CheckPaths()
        {
            // Only check path of executable if it is an actual path
            // e.g. for notepad.exe (which is part of environment) no need to check
            if (string.Compare(
                Path.GetFileName(this.ExePath),
                this.ExePath,
                StringComparison.OrdinalIgnoreCase) != 0)
            {
                if (!File.Exists(this.ExePath))
                {
                    throw new InvalidOperationException("File not found: " + this.ExePath);
                }
            }
        }
    }
}
