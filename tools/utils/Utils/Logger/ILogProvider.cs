// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

namespace Microsoft.Msix.Utils.Logger
{
    /// <summary>
    /// Log provider interface. Must be implemented by each logger
    /// </summary>
    public interface ILogProvider
    {
        /// <summary>
        /// Gets or sets the log level
        /// </summary>
        Logger.LogLevels LogLevels { get; set; }

        /// <summary>
        /// Gets or sets the log decoration
        /// </summary>
        Logger.LogDecorations LogDecorations { get; set; }

        /// <summary>
        /// Initializes the logger
        /// </summary>
        void InitLog();

        /// <summary>
        /// De-initializes the logger
        /// </summary>
        void DeinitLog();

        /// <summary>
        /// Logs a message
        /// </summary>
        /// <param name="logMessage">message to log</param>
        void Log(ILogMessage logMessage);
    }
}
