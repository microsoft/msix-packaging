//-----------------------------------------------------------------------
// <copyright file="VersionInfo.cs" company="Microsoft">
//     Copyright (c) Microsoft. All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

namespace Microsoft.Msix.Utils.AppxPackaging
{
    using System;

    /// <summary>
    /// Represents the version of a package.
    /// </summary>
    public class VersionInfo : IComparable
    {
        /// <summary>
        /// Version encoded as a UINT64 as follows:
        /// If the version information is missing, version will be set to null. Otherwise,
        /// As described in the documentation for IAppxManifestPackageId.GetVersion:
        ///  * The highest 16 bits contain the major version
        ///  * The next 16 bits contain the minor version
        ///  * The next 16 bits contain the build number
        ///  * The lowest 16 bits contain the revision
        /// </summary>
        private ulong? version;

        /// <summary>
        /// Initializes a new instance of the VersionInfo class.
        /// </summary>
        /// <param name="version">Version encoded as a UINT64</param>
        public VersionInfo(ulong? version)
        {
            this.version = version;
        }

        /// <summary>
        /// Initializes a new instance of the VersionInfo class.
        /// </summary>
        /// <param name="versionAsStr">Version represented in a string</param>
        public VersionInfo(string versionAsStr)
        {
            // Use the System.Version class to convert the version into a UINT64 encoding
            Version versionWrapper = new Version(versionAsStr);
            this.version = (((ulong)versionWrapper.Major) << 48)
                + (((ulong)versionWrapper.Minor) << 32)
                + ((ulong)versionWrapper.Build << 16)
                + (ulong)versionWrapper.Revision;
        }

        /// <summary>
        /// Gets the major version.
        /// </summary>
        public ulong? MajorVersion
        {
            get
            {
                return this.version >> 48;
            }
        }

        /// <summary>
        /// Gets the minor version.
        /// </summary>
        public ulong? MinorVersion
        {
            get
            {
                return (this.version >> 32) & 0xffff;
            }
        }

        /// <summary>
        /// Gets the build number.
        /// </summary>
        public ulong? BuildNumber
        {
            get
            {
                return (this.version >> 16) & 0xffff;
            }
        }

        /// <summary>
        /// Gets the revision.
        /// </summary>
        public ulong? Revision
        {
            get
            {
                return this.version & 0xffff;
            }
        }

        /// <summary>
        /// Converts the version to a string in quad notation, i.e. "major.minor.build.revision".
        /// </summary>
        /// <returns>Version in quad notation</returns>
        public override string ToString()
        {
            if (this.version == null)
            {
                return "N/A";
            }

            return string.Format(
                "{0}.{1}.{2}.{3}",
                this.MajorVersion,
                this.MinorVersion,
                this.BuildNumber,
                this.Revision);
        }

        /// <summary>
        /// Compares this instance to a specified object and returns an indication of
        /// their relative values.
        /// </summary>
        /// <param name="obj">An object to compare</param>
        /// <returns>Value indicating relative order of the objects being compared</returns>
        public int CompareTo(object obj)
        {
            if (obj == null)
            {
                return 1;
            }

            VersionInfo otherVersion = obj as VersionInfo;

            if (otherVersion == null)
            {
                throw new ArgumentException("Object is not a VersionInfo");
            }

            if (otherVersion.version == null || this.version == null)
            {
                // if version info is missing assume second package is newer
                return 1;
            }

            return ((ulong)this.version).CompareTo((ulong)otherVersion.version);
        }

        public override bool Equals(object obj)
        {
            if (obj == null)
            {
                return false;
            }

            VersionInfo other = obj as VersionInfo;
            if (other == null)
            {
                return false;
            }

            // This handles correctly both the null and non null cases.
            return this.version == other.version;
        }

        public override int GetHashCode()
        {
            if (this.version == null)
            {
                return 0;
            }

            uint high = (uint)(this.version >> 32);
            uint low = (uint)this.version;
            return (int)(high ^ low);
        }
    }
}
