//-----------------------------------------------------------------------
// <copyright file="IAppxManifestApplication.cs" company="Microsoft">
//     Copyright (c) Microsoft. All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

namespace Microsoft.Packaging.Utils.AppxPackagingInterop
{
    using System;
    using System.Diagnostics.CodeAnalysis;

    using System.Runtime.InteropServices;
    using System.Runtime.InteropServices.ComTypes;

    [SuppressMessage("StyleCop.CSharp.DocumentationRules", "*", Justification = "Interop")]
    [Guid("5da89bf4-3773-46be-b650-7e744863b7e8"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAppxManifestApplication : IDisposable
    {
        void GetStringValue(string name, [Out, MarshalAs(UnmanagedType.LPWStr)] out string value);

        [return: MarshalAs(UnmanagedType.LPWStr)]
        string GetAppUserModelId();
    }
}
