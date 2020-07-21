//-----------------------------------------------------------------------
// <copyright file="EmbeddedResourcesUtils.cs" company="Microsoft">
//     Copyright (c) Microsoft. All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

namespace Microsoft.Packaging.Utils.IO
{
    using System;
    using System.IO;
    using System.Reflection;

    /// <summary>
    /// Provides methods for accessing assembly embedded resources
    /// </summary>
    public class EmbeddedResourcesUtils
    {
        /// <summary>
        /// Returns the embedded resource as a stream.
        /// </summary>
        /// <param name="resourceName">the name of the resource</param>
        /// <returns>the stream containing the resource</returns>
        public static Stream GetResrouceAsStream(string resourceName)
        {
            Assembly assembly = Assembly.GetCallingAssembly();
            string[] embeddedResouceNames = assembly.GetManifestResourceNames();
            string resourceFullName = Array.Find(
                embeddedResouceNames,
                s => s.ToUpperInvariant().Contains(resourceName.ToUpperInvariant()));

            return assembly.GetManifestResourceStream(resourceFullName);
        }

        /// <summary>
        /// Returns the embedded resource as a temporary file.
        /// </summary>
        /// <param name="resourceName">the name of the resource</param>
        /// <returns>the path to the temporary file where the resource was copied</returns>
        public static string GetResrouceAsFile(string resourceName)
        {
            string tempFilePath = Path.GetTempFileName();
            using (FileStream file = File.Create(tempFilePath))
            {
                using (Stream fileStream = GetResrouceAsStream(resourceName))
                {
                    fileStream.Position = 0;
                    fileStream.CopyTo(file);
                }
            }

            return tempFilePath;
        }
    }
}
