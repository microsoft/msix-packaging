//-----------------------------------------------------------------------
// <copyright file="Win32InteropException.cs" company="Microsoft">
//     Copyright (c) Microsoft. All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

namespace Microsoft.Msix.Utils.ProcessRunner
{
    using System;
    using System.Globalization;

    /// <summary>
    /// Custom exception for Win32 errors.
    /// This is needed as the regular Win32Exception class doesn't exist under CoreCLR
    /// </summary>
    public class Win32InteropException : Exception
    {
        public Win32InteropException(int errorCode)
            : base("A Win32 error has occurred. Error Code =" +
                  errorCode.ToString(CultureInfo.InvariantCulture))
        {
            this.HResult = errorCode;
        }

        public Win32InteropException(string errorMessage, int errorCode)
            : base(errorMessage)
        {
        }

        public Win32InteropException(string errorMessage)
            : base(errorMessage)
        {
        }
    }
}
