//-----------------------------------------------------------------------
// <copyright file="IAppxEncryptionFactory.cs" company="Microsoft">
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
    [SuppressMessage("StyleCop.CSharp.MaintainabilityRules", "*", Justification = "Dummy type")]
    [Flags]
    public enum APPX_ENCRYPTED_PACKAGE_OPTIONS
    {
        APPX_ENCRYPTED_PACKAGE_OPTION_NONE = 0,
        APPX_ENCRYPTED_PACKAGE_OPTION_DIFFUSION = 0x1,
        APPX_ENCRYPTED_PACKAGE_OPTION_PAGE_HASHING = 0x2,
    }

    [SuppressMessage("StyleCop.CSharp.DocumentationRules", "*", Justification = "Interop")]
    [Guid("80E8E04D-8C88-44AE-A011-7CADF6FB2E72"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAppxEncryptionFactory : IDisposable
    {
        void EncryptPackage(
            [In] IStream inputStream,
            [In] IStream outputStream,
            [In] APPX_ENCRYPTED_PACKAGE_SETTINGS settings,
            [In] /*APPX_KEY_INFO* */ IntPtr keyInfo,
            [In] APPX_ENCRYPTED_EXEMPTIONS exemptedFiles);

        void DecryptPackage([In] IStream inputStream, [In] IStream outputStream, [In] /*APPX_KEY_INFO* */ IntPtr keyInfo);

        IAppxEncryptedPackageWriter CreateEncryptedPackageWriter(
            [In] IStream outputStream,
            [In] IStream manifestStream,
            [In] APPX_ENCRYPTED_PACKAGE_SETTINGS settings,
            [In] /*APPX_KEY_INFO* */ IntPtr keyInfo,
            [In] APPX_ENCRYPTED_EXEMPTIONS exemptedFiles);

        IAppxPackageReader CreateEncryptedPackageReader([In] IStream inputStream, [In] /*APPX_KEY_INFO* */ IntPtr keyInfo);

        void EncryptBundle(
            [In] IStream inputStream,
            [In] IStream outputStream,
            [In] APPX_ENCRYPTED_PACKAGE_SETTINGS settings,
            [In] /*APPX_KEY_INFO* */ IntPtr keyInfo,
            [In] APPX_ENCRYPTED_EXEMPTIONS exemptedFiles);

        void DecryptBundle([In] IStream inputStream, [In] IStream outputStream, [In] /*APPX_KEY_INFO* */ IntPtr keyInfo);

        IAppxEncryptedBundleWriter CreateEncryptedBundleWriter(
            [In] IStream outputStream,
            [In] ulong bundleVersion,
            [In] APPX_ENCRYPTED_PACKAGE_SETTINGS settings,
            [In] /*APPX_KEY_INFO* */ IntPtr keyInfo,
            [In] APPX_ENCRYPTED_EXEMPTIONS exemptedFiles);

        IAppxBundleReader CreateEncryptedBundleReader([In] IStream inputStream, [In] /*APPX_KEY_INFO* */ IntPtr keyInfo);
    }

    // Empty declarations of unused types for building project
    [SuppressMessage("StyleCop.CSharp.DocumentationRules", "*", Justification = "Interop")]
    public interface IAppxEncryptedPackageWriter
    {
    }

    [SuppressMessage("StyleCop.CSharp.DocumentationRules", "*", Justification = "Interop")]
    public interface IAppxEncryptedBundleWriter
    {
    }

    [SuppressMessage("StyleCop.CSharp.DocumentationRules", "*", Justification = "Interop")]
    [SuppressMessage("StyleCop.CSharp.MaintainabilityRules", "*", Justification = "Dummy type")]
    [StructLayout(LayoutKind.Sequential)]
    public struct APPX_ENCRYPTED_PACKAGE_SETTINGS2
    {
        public uint KeyLength;
        public string EncryptionAlgorithm;
        public IUri BlockMapHashAlgorithm;
        public uint Options;
    }

    [SuppressMessage("StyleCop.CSharp.DocumentationRules", "*", Justification = "Interop")]
    [SuppressMessage("StyleCop.CSharp.MaintainabilityRules", "*", Justification = "Dummy type")]
    [StructLayout(LayoutKind.Sequential)]
    public struct APPX_KEY_INFO
    {
        public uint KeyLength;
        public uint KeyIdLength;
        public IntPtr Key;
        public IntPtr KeyId;
    }

    [SuppressMessage("StyleCop.CSharp.DocumentationRules", "*", Justification = "Interop")]
    [SuppressMessage("StyleCop.CSharp.MaintainabilityRules", "*", Justification = "Dummy type")]
    public class APPX_ENCRYPTED_PACKAGE_SETTINGS
    {
    }

    [SuppressMessage("StyleCop.CSharp.DocumentationRules", "*", Justification = "Interop")]
    [SuppressMessage("StyleCop.CSharp.MaintainabilityRules", "*", Justification = "Dummy type")]
    public class APPX_ENCRYPTED_EXEMPTIONS
    {
    }
}
