// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

namespace Microsoft.Msix.Utils.CommandLine
{
    using System;

    /// <summary>
    /// Represents an error that occurs due to invalid command line arguments.
    /// </summary>
    public class CommandLineException : Exception
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="CommandLineException"/> class.
        /// </summary>
        /// <param name="argumentsSupplied">Whether command line arguments were supplied to the
        /// program.</param>
        public CommandLineException(bool argumentsSupplied = true)
        {
            this.ArgumentsSupplied = argumentsSupplied;
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="CommandLineException"/> class with the given message.
        /// </summary>
        /// <param name="message">Error message.</param>
        /// <param name="argumentsSupplied">Whether command line arguments were supplied to the
        /// program.</param>
        public CommandLineException(string message, bool argumentsSupplied = true) : base(message)
        {
            this.ArgumentsSupplied = argumentsSupplied;
        }

        /// <summary>
        /// Gets a value indicating whether command line arguments were supplied to the program.
        /// </summary>
        public bool ArgumentsSupplied { get; }
    }
}