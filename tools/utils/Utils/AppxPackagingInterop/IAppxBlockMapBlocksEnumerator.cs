//-----------------------------------------------------------------------
// <copyright file="IAppxBlockMapBlocksEnumerator.cs" company="Microsoft">
//     Copyright (c) Microsoft. All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

namespace Microsoft.Packaging.SDKUtils.AppxPackagingInterop
{
    using System;
    using System.Diagnostics.CodeAnalysis;

    using System.Runtime.InteropServices;

    [SuppressMessage("StyleCop.CSharp.DocumentationRules", "*", Justification = "Interop")]
    [Guid("6b429b5b-36ef-479e-b9eb-0c1482b49e16"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAppxBlockMapBlocksEnumerator : IDisposable
    {
        IAppxBlockMapBlock GetCurrent();

        bool GetHasCurrent();

        bool MoveNext();
    }
}
