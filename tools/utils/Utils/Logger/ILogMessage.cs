// -----------------------------------------------------------------------
//  <copyright file="ILogMessage.cs" company="Microsoft">
//      Copyright (c) Microsoft Corporation.  All rights reserved.
//  </copyright>
// -----------------------------------------------------------------------
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
