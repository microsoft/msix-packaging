//-----------------------------------------------------------------------
// <copyright file="IAppxBlockMapReader.cs" company="Microsoft">
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
    [Guid("5efec991-bca3-42d1-9ec2-e92d609ec22a"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAppxBlockMapReader : IDisposable
    {
        IAppxBlockMapFile GetFile([In, MarshalAs(UnmanagedType.LPWStr)] string filename);

        IAppxBlockMapFilesEnumerator GetFiles();

        IUri GetHashMethod();

        IStream GetStream();
    }
}
