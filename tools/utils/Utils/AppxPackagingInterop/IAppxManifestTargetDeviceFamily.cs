//-----------------------------------------------------------------------
// <copyright file="IAppxManifestTargetDeviceFamily.cs" company="Microsoft">
//     Copyright (c) Microsoft. All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

namespace Microsoft.Packaging.Utils.AppxPackagingInterop
{
    using System;
    using System.Diagnostics.CodeAnalysis;
    using System.Runtime.InteropServices;

    [SuppressMessage("StyleCop.CSharp.DocumentationRules", "*", Justification = "Interop")]
    [Guid("9091B09B-C8D5-4F31-8687-A338259FAEFB"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAppxManifestTargetDeviceFamily : IDisposable
    {
        [return: MarshalAs(UnmanagedType.LPWStr)]
        string GetName();

        ulong GetMinVersion();

        ulong GetMaxVersionTested();
    }
}
