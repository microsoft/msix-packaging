// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

namespace Microsoft.Msix.Utils.Logger
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
