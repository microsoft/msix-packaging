// -----------------------------------------------------------------------
//  <copyright file="LogMessageFormatter.cs" company="Microsoft">
//      Copyright (c) Microsoft Corporation.  All rights reserved.
//  </copyright>
// -----------------------------------------------------------------------

namespace Microsoft.Packaging.SDKUtils.Logger
{
    using System;
    using System.Diagnostics;
    using System.Globalization;

    public class LogMessageFormatter : ILogMessageFormatter
    {
        #region ILogMessageFormatter

        public virtual ILogMessage CreateMessage(Logger.LogLevels logLevel, string message, IMessageArg[] messageArgs)
        {
            StackTrace st = new StackTrace(true);

            // We use stack frame = 4 since we need the previous 4th function call stack that called us.
            LogMessage logMessage = new LogMessage(logLevel, message, st.GetFrame(4), messageArgs);
            return logMessage;
        }

        public virtual string GetExceptionMessage(Exception exception)
        {
            if (exception != null)
            {
                if (!string.IsNullOrWhiteSpace(exception.Message))
                {
                    return string.Format(CultureInfo.InvariantCulture, "{0}, {1}, {2}", exception.Message, exception.GetType().Name, exception.StackTrace);
                }
                else
                {
                    return string.Format(CultureInfo.InvariantCulture, "{0}, {1}", exception.GetType().Name, exception.StackTrace);
                }
            }

            return string.Empty;
        }

        #endregion
    }
}
