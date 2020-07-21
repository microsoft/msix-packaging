//-----------------------------------------------------------------------
// <copyright file="IAppxManifestProperties.cs" company="Microsoft">
//     Copyright (c) Microsoft. All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

namespace Microsoft.Packaging.Utils.AppxPackagingInterop
{
    using System;
    using System.Diagnostics.CodeAnalysis;

    using System.Runtime.InteropServices;

    [SuppressMessage("StyleCop.CSharp.DocumentationRules", "*", Justification = "Interop")]
    [Guid("03faf64d-f26f-4b2c-aaf7-8fe7789b8bca"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAppxManifestProperties : IDisposable
    {
        bool GetBoolValue(string name);

        void GetStringValue(string name, [Out, MarshalAs(UnmanagedType.LPWStr)] out string value);
    }
}