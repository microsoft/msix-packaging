// -----------------------------------------------------------------------
//  <copyright file="LogProvider.cs" company="Microsoft">
//      Copyright (c) Microsoft Corporation.  All rights reserved.
//  </copyright>
// -----------------------------------------------------------------------

namespace Microsoft.Packaging.SDKUtils.Logger
{
    /// <summary>
    /// Log provider class (providers can be: Trace output, file, debug output, event viewer etc.)
    /// </summary>
    public abstract class LogProvider : ILogProvider
    {
        /// <summary>
        /// Gets or sets the log levels
        /// </summary>
        public Logger.LogLevels LogLevels { get; set; }

        /// <summary>
        /// Gets or sets the log decorations
        /// </summary>
        public Logger.LogDecorations LogDecorations { get; set; }

        /// <summary>
        /// Initializes the logger
        /// </summary>
        public virtual void InitLog()
        {
        }

        /// <summary>
        /// De-initializes the logger
        /// </summary>
        public abstract void DeinitLog();

        /// <summary>
        /// Logs a message
        /// </summary>
        /// <param name="logMessage">message to log</param>
        public abstract void Log(ILogMessage logMessage);
    }
}
