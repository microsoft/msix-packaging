//-----------------------------------------------------------------------
// <copyright file="ProcessRunnerException.cs" company="Microsoft">
//     Copyright (c) Microsoft. All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

namespace Microsoft.Packaging.Utils.ProcessRunner
{
    using System;
    using System.IO;

    /// <summary>
    /// Represents a failure on an execution of a process.
    /// </summary>
    public class ProcessRunnerException : Exception
    {
        /// <summary>
        /// Initializes a new instance of the ProcessRunnerException class
        /// with the name of the process executed and its exit code.
        /// </summary>
        /// <param name="name">Name of the process executed.</param>
        /// <param name="exitCode">Exit code of the process.</param>
        /// <param name="logDirectory">The log directory ProcessRunner used, if any.</param>
        public ProcessRunnerException(string name, int exitCode, string logDirectory) 
            : base(FormatExceptionMessage(name, exitCode, logDirectory))
        {
            this.Name = name;
            this.ExitCode = exitCode;
        }

        /// <summary>
        /// Gets the name of the process executed.
        /// </summary>
        public string Name { get; }

        /// <summary>
        /// Gets the exit code of the process.
        /// </summary>
        public int ExitCode { get; }

        private static string FormatExceptionMessage(string name, int exitCode, string logDirectory)
        {
            string message = string.Format("Process {0} failed with exit code {1}.", Path.GetFileName(name), exitCode);

            if (logDirectory != null)
            {
                message += string.Format("{0}Process logs were saved under {1}.", Environment.NewLine, logDirectory);
            }

            return message;
        }
    }
}
