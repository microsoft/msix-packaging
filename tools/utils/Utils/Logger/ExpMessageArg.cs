// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

namespace Microsoft.Msix.Utils.Logger
{
    using System;

    /// <summary>
    /// Exception message argument
    /// </summary>
    public sealed class ExpMessageArg : IMessageArg
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="ExpMessageArg" /> class
        /// </summary>
        /// <param name="logExp">log exception</param>
        public ExpMessageArg(Exception logExp)
        {
            this.MessageArgument = logExp;
        }

        /// <summary>
        /// Gets the message argument for the log exception
        /// </summary>
        public object MessageArgument { get; private set; }
    }
}
