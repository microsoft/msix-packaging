//-----------------------------------------------------------------------
// <copyright file="InvalidFileTypeException.cs" company="Microsoft">
//     Copyright (c) Microsoft. All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

namespace Microsoft.Packaging.Utils
{
    using System;

    /// <summary>
    /// Represents an error that occurs when attempting to read a file with an invalid type.
    /// </summary>
    public class InvalidFileTypeException : Exception
    {
        /// <summary>
        /// Initializes a new instance of the InvalidFileTypeException class.
        /// </summary>
        /// <param name="path">Path of file that caused the exception</param>
        public InvalidFileTypeException(string path) : base()
        {
            this.Path = path;
        }

        /// <summary>
        /// Gets the path of the file with an invalid type that caused the exception.
        /// </summary>
        public string Path { get; }
    }
}
