// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

namespace Microsoft.Msix.Utils.AppxPackaging
{
    using System.Collections.Generic;

    /// <summary>
    /// Contains definitions of constants used for handling packages.
    /// </summary>
    public static class PackagingConstants
    {
        /// <summary>
        /// File extension for appx packages.
        /// </summary>
        public static readonly string AppxFileExtension = ".appx";

        /// <summary>
        /// File extension for appxbundle packages.
        /// </summary>
        public static readonly string AppxBundleFileExtension = ".appxbundle";

        /// <summary>
        /// File extension for encrypted appx packages.
        /// </summary>
        public static readonly string EncryptedAppxFileExtension = ".eappx";

        /// <summary>
        /// File extension for encrypted appxbundle packages.
        /// </summary>
        public static readonly string EncryptedAppxBundleFileExtension = ".eappxbundle";

        /// <summary>
        /// File extension for msix packages.
        /// </summary>
        public static readonly string MsixFileExtension = ".msix";

        /// <summary>
        /// File extension for msixbundle packages.
        /// </summary>
        public static readonly string MsixBundleFileExtension = ".msixbundle";

        /// <summary>
        /// File extension for encrypted msix packages.
        /// </summary>
        public static readonly string EncryptedMsixFileExtension = ".emsix";

        /// <summary>
        /// File extension for encrypted msixbundle packages.
        /// </summary>
        public static readonly string EncryptedMsixBundleFileExtension = ".emsixbundle";

        /// <summary>
        /// File extension for xml.
        /// </summary>
        public static readonly string XmlFileExtension = ".xml";

        /// <summary>
        /// File extension for delta appx files.
        /// </summary>
        public static readonly string DeltaAppxFileExtension = ".dapx";

        /// <summary>
        /// Set of extensions used for packages or bundles.
        /// </summary>
        public static readonly HashSet<string> AllPackageOrBundleExtensions = new HashSet<string>(new string[]
        {
            AppxFileExtension,
            AppxBundleFileExtension,
            EncryptedAppxFileExtension,
            EncryptedAppxBundleFileExtension,
            MsixFileExtension,
            MsixBundleFileExtension,
            EncryptedMsixFileExtension,
            EncryptedMsixBundleFileExtension
        });

        /// <summary>
        /// Set of extensions used for unencrypted packages or bundles.
        /// </summary>
        public static readonly HashSet<string> UnencryptedPackageOrBundleExtensions = new HashSet<string>(new string[]
        {
            AppxFileExtension,
            AppxBundleFileExtension,
            MsixFileExtension,
            MsixBundleFileExtension
        });

        /// <summary>
        /// Set of extensions used for encrypted packages or bundles.
        /// </summary>
        public static readonly HashSet<string> EncryptedPackageOrBundleExtensions = new HashSet<string>(new string[]
        {
            EncryptedAppxFileExtension,
            EncryptedAppxBundleFileExtension,
            EncryptedMsixFileExtension,
            EncryptedMsixBundleFileExtension
        });

        /// <summary>
        /// Name of the directory containing the delta files in an appx package.
        /// </summary>
        public static readonly string AppxDeltaDirectory = "AppxDelta";

        /// <summary>
        /// Common packaging error codes
        /// </summary>
        public enum ErrorCode
        {
            /// <summary>
            /// The package's AppxBlockMap.xml file isn't valid.
            /// </summary>
            APPX_E_INVALID_BLOCKMAP = unchecked((int)0x80080205)
        }
    }
}
