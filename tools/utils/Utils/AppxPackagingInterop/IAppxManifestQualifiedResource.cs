//-----------------------------------------------------------------------
// <copyright file="IAppxManifestQualifiedResource.cs" company="Microsoft">
//     Copyright (c) Microsoft. All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

namespace Microsoft.Msix.Utils.AppxPackagingInterop
{
    using System;
    using System.Diagnostics.CodeAnalysis;

    using System.Runtime.InteropServices;

    [SuppressMessage("StyleCop.CSharp.DocumentationRules", "*", Justification = "Interop")]
    [Guid("3b53a497-3c5c-48d1-9ea3-bb7eac8cd7d4"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAppxManifestQualifiedResource : IDisposable
    {
        [return: MarshalAs(UnmanagedType.LPWStr)]
        string GetLanguage();

        uint GetScale();

        DX_FEATURE_LEVEL GetDXFeatureLevel();
    }
}
