//-----------------------------------------------------------------------
// <copyright file="IAppxBlockMapBlock.cs" company="Microsoft">
//     Copyright (c) Microsoft. All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

namespace Microsoft.Msix.Utils.AppxPackagingInterop
{
    using System;
    using System.Diagnostics.CodeAnalysis;

    using System.Runtime.InteropServices;

    [SuppressMessage("StyleCop.CSharp.DocumentationRules", "*", Justification = "Interop")]
    [Guid("75cf3930-3244-4fe0-a8c8-e0bcb270b889"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAppxBlockMapBlock : IDisposable
    {
        IntPtr GetHash([Out] out uint bufferSize);

        uint GetCompressedSize();
    }
}
