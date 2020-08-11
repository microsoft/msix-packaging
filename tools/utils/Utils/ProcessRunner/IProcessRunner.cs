//-----------------------------------------------------------------------
// <copyright file="IProcessRunner.cs" company="Microsoft">
//     Copyright (c) Microsoft. All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

namespace Microsoft.Msix.Utils.ProcessRunner
{
    using System;
    using System.Collections.Generic;
    using System.Text;
    using Microsoft.Msix.Utils.Logger;

    public interface IProcessRunner : IDisposable
    {
        /// <summary>
        /// Gets or sets the executable path for the process to run.
        /// </summary>
        string ExePath { get; set; }

        /// <summary>
        /// Gets or sets the arguments to pass to the process.
        /// </summary>
        string Arguments { get; set; }

        /// <summary>
        /// Gets or sets a value indicating whether to create a new window.
        /// </summary>
        bool CreateNoWindow { get; set; }

        /// <summary>
        /// Gets a value indicating whether the runner supports standard output redirection.
        /// </summary>
        bool SupportsStandardOutputRedirection { get; }

        /// <summary>
        /// Gets a value indicating whether the runner supports standard error redirection.
        /// </summary>
        bool SupportsStandardErrorRedirection { get; }

        /// <summary>
        /// Gets the log providers to be used when logging. Default is to use
        /// the registered providers with the Logger class.
        /// </summary>
        IReadOnlyList<ILogProvider> LogProviders { get; }

        /// <summary>
        ///  Gets the standard error. Each item in the list represents one line output.
        /// </summary>
        IReadOnlyList<string> StandardError { get; }

        /// <summary>
        ///  Gets the standard output. Each item in the list represents one line output.
        /// </summary>
        IReadOnlyList<string> StandardOutput { get; }

        /// <summary>
        /// Gets or sets the standard output encoding.
        /// </summary>
        Encoding StandardOutputEncoding { get; set; }

        /// <summary>
        /// Gets or sets the standard error encoding.
        /// </summary>
        Encoding StandardErrorEncoding { get; set; }

        /// <summary>
        ///  Gets the exit code of the process. If the process hasn't
        ///  finished running, -1 will be returned.
        /// </summary>
        int ExitCode { get; }

        /// <summary>
        ///  Runs the executable and wait INT.MaxValue for the process's exit.
        ///  Throws ProcessRunnerException if the process hasn't finished within the time allotted
        /// </summary>
        void Run();

        /// <summary>
        ///  Runs the executable and wait for the process's exit.
        ///  Throws ProcessRunnerException if the process hasn't finished within the time allotted
        /// </summary>
        /// <param name="milliseconds">
        ///  The amount of time, in milliseconds, to wait for the associated process to exit.
        ///  The maximum is the largest possible value of a 32-bit integer, which represents
        ///  infinity to the operating system.
        /// </param>
        void RunAndWait(int milliseconds);

        /// <summary>
        ///  Validates process exit code.
        ///  Throws ProcessRunnerException if the process hasn't returned success exit code
        /// </summary>
        /// <param name="validCodes">
        ///  Valid exit codes to accept.
        /// </param>
        void ValidateExitCode(int[] validCodes);
    }
}
