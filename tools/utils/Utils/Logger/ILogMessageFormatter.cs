// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

namespace Microsoft.Msix.Utils.Logger
{
    using System;

    /// <summary>
    /// Formats the message
    /// </summary>
    public interface ILogMessageFormatter
    {
        ILogMessage CreateMessage(Logger.LogLevels logLevel, string message, IMessageArg[] messageArgs);

        string GetExceptionMessage(Exception exception);
    }
}
