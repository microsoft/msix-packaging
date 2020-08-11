//-----------------------------------------------------------------------
// <copyright file="IAppxBlockMapFile.cs" company="Microsoft">
//     Copyright (c) Microsoft. All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

namespace Microsoft.Msix.Utils.AppxPackagingInterop
{
    using System;
    using System.Diagnostics.CodeAnalysis;

    using System.Runtime.InteropServices;
    using System.Runtime.InteropServices.ComTypes;

    [SuppressMessage("StyleCop.CSharp.DocumentationRules", "*", Justification = "Interop")]
    [Guid("277672ac-4f63-42c1-8abc-beae3600eb59"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAppxBlockMapFile : IDisposable
    {
        IAppxBlockMapBlocksEnumerator GetBlocks();

        uint GetLocalFileHeaderSize();

        [return: MarshalAs(UnmanagedType.LPWStr)]
        string GetName();

        ulong GetUncompressedSize();

        bool ValidateFileHash([In] IStream fileStream);
    }
}
