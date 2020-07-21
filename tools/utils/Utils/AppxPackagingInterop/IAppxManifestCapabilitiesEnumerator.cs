//-----------------------------------------------------------------------
// <copyright file="IAppxManifestCapabilitiesEnumerator.cs" company="Microsoft">
//     Copyright (c) Microsoft. All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

namespace Microsoft.Packaging.Utils.AppxPackagingInterop
{
    using System;
    using System.Diagnostics.CodeAnalysis;
    using System.Runtime.InteropServices;

    [SuppressMessage("StyleCop.CSharp.DocumentationRules", "*", Justification = "Interop")]
    [Guid("11D22258-F470-42C1-B291-8361C5437E41"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAppxManifestCapabilitiesEnumerator : IDisposable
    {
        [return: MarshalAs(UnmanagedType.LPWStr)]
        string GetCurrent();

        bool GetHasCurrent();

        bool MoveNext();
    }
}
