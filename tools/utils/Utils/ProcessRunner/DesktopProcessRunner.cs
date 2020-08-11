//-----------------------------------------------------------------------
// <copyright file="DesktopProcessRunner.cs" company="Microsoft">
//     Copyright (c) Microsoft. All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

namespace Microsoft.Msix.Utils.ProcessRunner
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using Microsoft.Msix.Utils.Logger;

    /// <summary>
    /// Implements a process runner which runs on Desktop
    /// and supports redirection of standard output and error.
    /// </summary>
    public class DesktopProcessRunner : ProcessRunnerBase
    {
        private Process process = null;

        public DesktopProcessRunner()
        {
            this.SupportsStandardOutputRedirection = true;
            this.SupportsStandardErrorRedirection = true;
        }

        public void DumpStandardOutput()
        {
            if (this.SupportsStandardOutputRedirection)
            {
                this.DumpLog(this.StandardOutput, isErrorLog: false);
            }
        }

        public void DumpStandardError()
        {
            if (this.SupportsStandardErrorRedirection)
            {
                this.DumpLog(this.StandardError, isErrorLog: true);
            }
        }

        protected override void Dispose(bool disposing)
        {
            if (disposing)
            {
                if (this.process != null)
                {
                    this.process.Dispose();
                }
            }

            base.Dispose(disposing);
        }

        protected override void OnLaunchProcess()
        {
            this.process = new Process();
            this.process.StartInfo = this.BuildProcessStartInfo();
            this.process.OutputDataReceived += this.OnOutputDataReceived;
            this.process.ErrorDataReceived += this.OnErrorDataReceived;

            Logger.Log(this.LogProviders, "About to start process");
            Logger.Log(this.LogProviders, "    - File name:                 " + this.process.StartInfo.FileName);

            if (this.DiscreetLogging > ProcessRunnerLogLevel.Partial)
            {
                Logger.Log(this.LogProviders, "    - Working directory:         " + this.process.StartInfo.WorkingDirectory);
            }

            if (this.DiscreetLogging > ProcessRunnerLogLevel.Private)
            {
                Logger.Log(this.LogProviders, "    - Arguments:                 " + this.process.StartInfo.Arguments);
            }

            if (this.DiscreetLogging > ProcessRunnerLogLevel.Partial)
            {
                Logger.Log(this.LogProviders, "    - Username:                  " + Environment.UserName);
            }

            this.process.Start();
            this.HasStarted = true;

            // Start asynchronously reading standard output/error streams
            // The events will be raised when data is written to the streams.
            this.process.BeginOutputReadLine();
            this.process.BeginErrorReadLine();
        }

        protected override void OnTerminateRunningProcess()
        {
            try
            {
                Logger.Log(this.LogProviders, Logger.LogLevels.Debug, "Terminating process");
                this.process.Kill();
            }
            catch (Exception ex)
            {
                // Process.Kill may fail in a few cases.
                Logger.Log(this.LogProviders, "Process kill failed: " + ex.Message);
                
                throw;
            }
        }

        protected override bool OnWaitForExitOrTimeout(int timeoutMilliseconds)
        {
            this.HasFinished = this.process.WaitForExit(timeoutMilliseconds);

            if (this.HasFinished)
            {
                // Call the WaitForExit second time to make sure that
                // standard output redirection handlers are completed.
                // Refer https://msdn.microsoft.com/en-us/library/ty0d8k56.aspx.
                this.process.WaitForExit();

                this.ExitCode = this.process.ExitCode;
            }

            return this.HasFinished;
        }

        private ProcessStartInfo BuildProcessStartInfo()
        {
            ProcessStartInfo startInfo = new ProcessStartInfo();
            startInfo.FileName = this.ExePath;
            startInfo.WorkingDirectory = Path.GetDirectoryName(this.ExePath);
            startInfo.Arguments = this.Arguments;
            startInfo.StandardOutputEncoding = this.StandardOutputEncoding;
            startInfo.StandardErrorEncoding = this.StandardErrorEncoding;
            startInfo.CreateNoWindow = this.CreateNoWindow;

            // To redirect standard output and/or error UseShellExecute must be set to false.
            startInfo.UseShellExecute = false;
            startInfo.RedirectStandardOutput = true;
            startInfo.RedirectStandardError = true;

            return startInfo;
        }

        /// <summary>
        ///   The event handler being called each time the process writes a line to the redirect stream.
        /// </summary>
        /// <param name="sender">The sender of the event.</param>
        /// <param name="e">
        ///   The event data from which the line written to the stream can be fetched.
        /// </param>
        private void OnOutputDataReceived(object sender, DataReceivedEventArgs e)
        {
            if (!string.IsNullOrWhiteSpace(e.Data))
            {
                this.AddStandardOutputEntry(e.Data);
            }
        }

        /// <summary>
        ///   The event handler being called each time the process writes a line to the redirect stream.
        /// </summary>
        /// <param name="sender">The sender of the event.</param>
        /// <param name="e">
        ///   The event data from which the line written to the stream can be fetched.
        /// </param>
        private void OnErrorDataReceived(object sender, DataReceivedEventArgs e)
        {
            if (!string.IsNullOrWhiteSpace(e.Data))
            {
                this.AddStandardErrorEntry(e.Data);
            }
        }

        private void DumpLog(IReadOnlyList<string> logs, bool isErrorLog)
        {
            if ((logs == null) || (logs.Count == 0))
            {
                return;
            }

            if (string.IsNullOrWhiteSpace(this.OutputLogDirectory))
            {
                foreach (string oneLineLog in logs)
                {
                    Logger.Log(
                        this.LogProviders,
                        isErrorLog ? Logger.LogLevels.Error : Logger.LogLevels.Info,
                        oneLineLog);
                }
            }
            else
            {
                string logFilePath = this.GetLogFilePath(isErrorLog);
                using (TextWriter tw = new StreamWriter(logFilePath))
                {
                    foreach (string oneLineLog in logs)
                    {
                        tw.WriteLine(oneLineLog);
                    }
                }
            }
        }

        private string GetLogFilePath(bool isErrorLog)
        {
            string processName = Path.GetFileNameWithoutExtension(this.ExePath);

            string logFileName = string.Format(
                CultureInfo.InvariantCulture,
                "{0}_{1}_{2}.txt",
                processName,
                isErrorLog ? "Error" : "Output",
                Guid.NewGuid().ToString());

            string logFilePath = Path.Combine(this.OutputLogDirectory, logFileName);

            if (File.Exists(logFilePath))
            {
                if ((File.GetAttributes(logFilePath) & FileAttributes.ReadOnly) ==
                    FileAttributes.ReadOnly)
                {
                    File.SetAttributes(logFilePath, FileAttributes.Normal);
                }

                File.Delete(logFilePath);
            }

            Logger.Log(
                this.LogProviders,
                Logger.LogLevels.Debug,
                "{0} log file is located under: {1}",
                isErrorLog ? "Error" : "Output",
                logFilePath);

            return logFilePath;
        }
    }
}
