//-----------------------------------------------------------------------
// <copyright file="IncompatibleFileTypesException.cs" company="Microsoft">
//     Copyright (c) Microsoft. All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

namespace Microsoft.Packaging.SDKUtils
{
    using System;

    /// <summary>
    /// Represents an error that occurs when attempting to perform an operation on files of incompatible types.
    /// </summary>
    public class IncompatibleFileTypesException : Exception
    {
        /// <summary>
        /// Initializes a new instance of the IncompatibleFileTypesException class.
        /// </summary>
        /// <param name="oldExtension">Extension of the older file</param>
        /// <param name="newExtension">Extension of the newer file</param>
        public IncompatibleFileTypesException(string oldExtension, string newExtension) : base()
        {
            this.OldExtension = oldExtension;
            this.NewExtension = newExtension;
        }

        /// <summary>
        /// Gets the extension of the older file.
        /// </summary>
        public string OldExtension { get; }

        /// <summary>
        /// Gets the extension of the newer file.
        /// </summary>
        public string NewExtension { get; }
    }
}
