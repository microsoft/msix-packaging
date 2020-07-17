// -----------------------------------------------------------------------
//  <copyright file="IMessageArg.cs" company="Microsoft">
//      Copyright (c) Microsoft Corporation.  All rights reserved.
//  </copyright>
// -----------------------------------------------------------------------
namespace Microsoft.Packaging.SDKUtils.Logger
{
    /// <summary>
    /// Message argument interface
    /// </summary>
    public interface IMessageArg
    {
        /// <summary>
        /// Gets the message argument
        /// </summary>
        object MessageArgument { get; }
    }
}
