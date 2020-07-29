//-----------------------------------------------------------------------
// <copyright file="IAppxBlockMapFilesEnumerator.cs" company="Microsoft">
//     Copyright (c) Microsoft. All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

namespace Microsoft.Packaging.Utils.AppxPackagingInterop
{
    using System;
    using System.Diagnostics.CodeAnalysis;

    using System.Runtime.InteropServices;

    [SuppressMessage("StyleCop.CSharp.DocumentationRules", "*", Justification = "Interop")]
    [Guid("02b856a2-4262-4070-bacb-1a8cbbc42305"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAppxBlockMapFilesEnumerator : IDisposable
    {
        IAppxBlockMapFile GetCurrent();

        bool GetHasCurrent();

        bool MoveNext();
    }
}
