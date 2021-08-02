// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

namespace Microsoft.Msix.Utils.Logger
{
    /// <summary>
    /// Log message
    /// </summary>
    public interface ILogMessage
    {
        Logger.LogLevels LogLevel { get; }

        string GetLogMessage(Logger.LogDecorations logDecoration, Logger.LogLevels logLevel);
    }
}
