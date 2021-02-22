// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

namespace Microsoft.Msix.Utils.AppxPackagingInterop
{
    using System;
    using System.Diagnostics.CodeAnalysis;
    using System.Runtime.InteropServices;
    using System.Runtime.InteropServices.ComTypes;

    [SuppressMessage("StyleCop.CSharp.DocumentationRules", "*", Justification = "Interop")]
    [Flags]
    public enum APPX_PACKAGE_EDITOR_UPDATE_PACKAGE_OPTION
    {
        APPX_PACKAGE_EDITOR_UPDATE_PACKAGE_OPTION_APPEND_DELTA = 0
    }

    [SuppressMessage("StyleCop.CSharp.DocumentationRules", "*", Justification = "Interop")]
    [Flags]
    public enum APPX_PACKAGE_EDITOR_UPDATE_PACKAGE_MANIFEST_OPTIONS
    {
        APPX_PACKAGE_EDITOR_UPDATE_PACKAGE_MANIFEST_OPTION_NONE = 0x00,
        APPX_PACKAGE_EDITOR_UPDATE_PACKAGE_MANIFEST_OPTION_SKIP_VALIDATION = 0x01,
        APPX_PACKAGE_EDITOR_UPDATE_PACKAGE_MANIFEST_OPTION_LOCALIZED = 0x02
    }

    [SuppressMessage("StyleCop.CSharp.DocumentationRules", "*", Justification = "Interop")]
    [Guid("E2ADB6DC-5E71-4416-86B6-86E5F5291A6B"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAppxPackageEditor : IDisposable
    {
        void SetWorkingDirectory([In] string workingDirectory);

        void CreateDeltaPackage(
            [In] IStream updatedPackageStream,
            [In] IStream baselinePackageStream,
            [In] IStream deltaPackageStream);

        void CreateDeltaPackageUsingBaselineBlockMap(
           [In] IStream updatedPackageStream,
           [In] IStream baselineBlockMapStream,
           [In] string baselinePackageFullName,
           [In] IStream deltaPackageStream);

        void UpdatePackage(
            [In] IStream baselinePackageStream,
            [In] IStream deltaPackageStream,
            [In] APPX_PACKAGE_EDITOR_UPDATE_PACKAGE_OPTION updateOption);

        void UpdateEncryptedPackage(
            [In] IStream baselineEncryptedPackageStream,
            [In] IStream deltaAppendedPackageStream,
            [In] APPX_PACKAGE_EDITOR_UPDATE_PACKAGE_OPTION updateOption,
            [In] /*APPX_ENCRYPTED_PACKAGE_SETTINGS2* */ IntPtr settings,
            [In] /*APPX_KEY_INFO* */ IntPtr keyInfo);

        void UpdatePackageManifest(
            [In] IStream packageStream,
            [In] IStream updatedManifestStream,
            [In] bool isPackageEncrypted,
            [In] APPX_PACKAGE_EDITOR_UPDATE_PACKAGE_MANIFEST_OPTIONS options);
    }
}
