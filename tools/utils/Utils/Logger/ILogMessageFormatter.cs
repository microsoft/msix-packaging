// -----------------------------------------------------------------------
//  <copyright file="ILogMessageFormatter.cs" company="Microsoft">
//      Copyright (c) Microsoft Corporation.  All rights reserved.
//  </copyright>
// -----------------------------------------------------------------------
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
