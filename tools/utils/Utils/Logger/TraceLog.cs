// -----------------------------------------------------------------------
//  <copyright file="TraceLog.cs" company="Microsoft">
//      Copyright (c) Microsoft Corporation.  All rights reserved.
//  </copyright>
// -----------------------------------------------------------------------
namespace Microsoft.Packaging.SDKUtils.Logger
{
    using System;
    using System.Diagnostics;

    /// <summary>
    /// Trace log class
    /// </summary>
    public class TraceLog : LogProvider
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="TraceLog" /> class.
        /// </summary>
        public TraceLog()
        {
            // Put only detail info to trace window to fit Visual Studio output window format.
            this.LogDecorations = Logger.LogDecorations.Detail;
        }

        /// <summary>
        /// Log a message
        /// </summary>
        /// <param name="logMessage">log message</param>
        public override void Log(ILogMessage logMessage)
        {
            if (logMessage == null)
            {
                throw new ArgumentNullException("logMessage");
            }

            switch (logMessage.LogLevel)
            {
                case Logger.LogLevels.Warning:
                    Trace.TraceWarning(logMessage.GetLogMessage(this.LogDecorations, this.LogLevels));
                    break;
                case Logger.LogLevels.Error:
                case Logger.LogLevels.Exp:
                case Logger.LogLevels.ExpStack:
                    Trace.TraceError(logMessage.GetLogMessage(this.LogDecorations, this.LogLevels));
                    break;
                default:
                    Trace.TraceInformation(logMessage.GetLogMessage(this.LogDecorations, this.LogLevels));
                    break;
            }
        }

        /// <summary>
        /// De-initializes the trace log
        /// </summary>
        public override void DeinitLog()
        {
        }
    }
}
