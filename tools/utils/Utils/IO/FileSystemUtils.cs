// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

namespace Microsoft.Msix.Utils
{
    using System;
    using System.Diagnostics.CodeAnalysis;
    using System.Globalization;
    using System.IO;
    using System.Reflection;

    public static class FileSystemUtils
    {
        /// <summary>
        /// Gets the file extension of a file in lowercase.
        /// </summary>
        /// <param name="path">Path to a file.</param>
        /// <returns>The extension of path in lowercase.</returns>
        public static string GetLowercaseExtension(string path)
        {
            return Path.GetExtension(path).ToLower(CultureInfo.InvariantCulture);
        }

        /// <summary>
        /// Get the first directory in a path.
        /// </summary>
        /// <param name="path">A path to a file.</param>
        /// <returns>The first directory appearing in the path, or null if there is none.</returns>
        public static string GetRootDirectory(string path)
        {
            // First check that the path is not empty and has at least one parent directory.
            if (string.IsNullOrWhiteSpace(path))
            {
                return null;
            }

            string parentDirectory = Path.GetDirectoryName(path);
            if (string.IsNullOrEmpty(parentDirectory))
            {
                return null;
            }

            // Now that we know the path has a parent directory,
            // keep removing directories until it does not.
            while (!string.IsNullOrEmpty(parentDirectory))
            {
                path = parentDirectory;
                parentDirectory = Path.GetDirectoryName(path);
            }

            return path;
        }

        /// <summary>
        /// Gets a file path of a found file name under a given folder path
        /// </summary>
        /// <param name="fileName">file name to search for</param>
        /// <param name="folderPath">folder path to search the file under</param>
        /// <returns>The path of the first found file if found otherwise throws an exception</returns>
        public static string GetFilePath(string fileName, string folderPath)
        {
            Logger.Logger.Log("Retrieving file name: {0}, under path {1}", fileName, folderPath);

            if (string.IsNullOrWhiteSpace(fileName))
            {
                throw new ArgumentException("File Name must be provided", "fileName");
            }

            if (string.IsNullOrWhiteSpace(folderPath))
            {
                throw new ArgumentException("Folder Path must be provided", "folderPath");
            }

            DirectoryInfo rootDirectoryInWhichToSearch = new DirectoryInfo(folderPath);
            string fileSearchPattern = "*" + fileName + "*";
            FileInfo[] filesInDir = rootDirectoryInWhichToSearch.GetFiles(
                fileSearchPattern,
                SearchOption.AllDirectories);

            if (filesInDir.Length > 0)
            {
                Logger.Logger.Log("File path found: {0}", filesInDir[0].FullName);
                return filesInDir[0].FullName;
            }

            throw new FileNotFoundException(fileName);
        }

        /// <summary>
        /// Gets a folder path under a given folder path
        /// </summary>
        /// <param name="folderName">folder name to search for</param>
        /// <param name="folderPath">folder path to search the folder under</param>
        /// <returns>folder path if found otherwise throws an exception</returns>
        public static string GetFolderPath(string folderName, string folderPath)
        {
            Logger.Logger.Log("Retreiving folder name: {0}, under path {1}", folderName, folderPath);

            if (string.IsNullOrWhiteSpace(folderName))
            {
                throw new ArgumentException("Folder Name must be provided", "folderName");
            }

            if (string.IsNullOrWhiteSpace(folderPath))
            {
                throw new ArgumentException("Folder Path must be provided", "folderPath");
            }

            DirectoryInfo rootDirectoryInWhichToSearch = new DirectoryInfo(folderPath);
            string fileSearchPattern = "*" + folderName + "*";
            DirectoryInfo[] directoriesFound = rootDirectoryInWhichToSearch.GetDirectories(
                fileSearchPattern,
                SearchOption.AllDirectories);

            if (directoriesFound.Length > 0)
            {
                Logger.Logger.Log("File path found: {0}", directoriesFound[0].FullName);
                return directoriesFound[0].FullName;
            }

            throw new DirectoryNotFoundException(folderName);
        }

        /// <summary>
        /// Iterates over all directories under a given path and returns the most recent one
        /// </summary>
        /// <param name="path">Path under which all sub folders will be iterate over</param>
        /// <returns>The most recent directory path</returns>
        public static string GetMostRecentDirectory(string path)
        {
            DateTime lastHigh = new DateTime(1900, 1, 1);
            string latestDir = string.Empty;
            foreach (string subDir in Directory.GetDirectories(path))
            {
                DirectoryInfo folderInfo = new DirectoryInfo(subDir);
                DateTime created = folderInfo.LastWriteTime;

                if (created > lastHigh)
                {
                    latestDir = subDir;
                    lastHigh = created;
                }
            }

            Logger.Logger.Log("Most recent directory: {0}", latestDir);

            return latestDir;
        }

        /// <summary>
        /// Create a unique directory under the temp user directory
        /// </summary>
        /// <param name="topLevelName">folder name for the top level directory</param>
        /// <returns>Path to the temporary folder just created.</returns>
        public static string CreateTempDirectory(string topLevelName)
        {
            return CreateUniqueDirectory(Path.GetTempPath(), topLevelName);
        }

        /// <summary>
        /// Copies an embedded resource from an assembly to a file.
        /// </summary>
        /// <param name="executingAssembly"> The executing assembly.</param>
        /// <param name="resourceName">Name of the assembly.</param>
        /// <param name="filePath"> The file path.</param>
        public static void CopyResourceToFile(Assembly executingAssembly, string resourceName, string filePath)
        {
            if (executingAssembly == null)
            {
                throw new ArgumentNullException("executingAssembly");
            }

            if (string.IsNullOrWhiteSpace(resourceName))
            {
                throw new ArgumentException("Resource Name must be provided", "resourceName");
            }

            if (string.IsNullOrWhiteSpace(filePath))
            {
                throw new ArgumentException("File path Name must be provided", "filePath");
            }

            string[] embeddedResouceNames = executingAssembly.GetManifestResourceNames();

            foreach (string resource in embeddedResouceNames)
            {
                if (resource.Contains(resourceName))
                {
                    using (var stream = executingAssembly.GetManifestResourceStream(resource))
                    {
                        using (var fileStream = File.Create(filePath))
                        {
                            stream.CopyTo(fileStream);
                            break;
                        }
                    }
                }
            }
        }

        [SuppressMessage("Microsoft.Design", "CA1031:DoNotCatchGeneralExceptionTypes", Justification = "Try pattern for delete")]
        public static bool TryDeleteFile(string filePath)
        {
            if (string.IsNullOrWhiteSpace(filePath))
            {
                Logger.Logger.Log("Invalid file path for deletion; returning");
                return false;
            }

            try
            {
                Logger.Logger.Log("Deleting file: {0}", filePath);
                File.Delete(filePath);
                return true;
            }
            catch (Exception ex)
            {
                Logger.Logger.Log("Error deleting file {0}; Exception: {1}", filePath, ex);
                return false;
            }
        }

        /// <summary>
        /// Creates a directory with the desired relative path under the specified directory.
        /// If a directory with the same name already exists, this method will append a number
        /// to the name to make it unique.
        /// Note: calling this method concurrently with the same parent directory/desired name
        /// can lead to races.
        /// </summary>
        /// <param name="parentDirectory">the directory under which the sub directory should be created</param>
        /// <param name="desiredSubDirRelativePath">the desired name of the sub directory</param>
        /// <returns>the full path to the created directory</returns>
        public static string CreateUniqueDirectory(string parentDirectory, string desiredSubDirRelativePath)
        {
            if (string.IsNullOrWhiteSpace(parentDirectory))
            {
                throw new ArgumentNullException("parentDirectory");
            }

            if (string.IsNullOrWhiteSpace(desiredSubDirRelativePath))
            {
                throw new ArgumentNullException("desiredSubDirRelativePath");
            }

            // change parentDirectory so that it points to the path of the parent directory of the bottom-most directory.
            // e.g. if parentDirectory is C:\Data, and desiredRelativeDirectory is Out\Dir, then change parentDirectory so
            // it's C:\Data\Out.
            string path = Path.Combine(parentDirectory, desiredSubDirRelativePath);
            string bottomMostDirName = new DirectoryInfo(path).Name;
            parentDirectory = Path.GetDirectoryName(path);

            // if the file already exists, append a number to its name
            int duplicateCount = 0;
            while (Directory.Exists(path))
            {
                duplicateCount++;
                path = Path.Combine(
                    parentDirectory,
                    string.Format("{0}_{1}", bottomMostDirName, duplicateCount));
            }

            Directory.CreateDirectory(path);

            Logger.Logger.Log("Unique directory path: {0}", path);

            return path;
        }

        /// <summary>
        /// NOTE: This method isn't supported within .NET modules
        /// the reason is that we use .NET 4.6 in Utils (for back compatible)
        /// Only use this method when you intend to pass paths to the native side
        /// Prepends \\?\ or \\?\UNC\ to an absolute path, if it doesn't already start with it, so that the native
        /// file system functions do not fail for long paths. Used for passing paths to unmanaged components.
        /// </summary>
        /// <param name="path">The path</param>
        /// <returns>the path with the prepended token</returns>
        public static string PrependLongPathToken(string path)
        {
            const string UncPathPrefix = @"\\";
            const string LongPathPrefixForLocalFiles = @"\\?\";
            const string LongPathPrefixForUNCFiles = @"\\?\UNC\";

            // We only prepend to rooted paths, not relative paths.
            if (Path.IsPathRooted(path))
            {
                // Only change the path if it's not already formatted for long paths support
                if (!path.StartsWith(LongPathPrefixForLocalFiles, StringComparison.OrdinalIgnoreCase) &&
                    !path.StartsWith(LongPathPrefixForUNCFiles, StringComparison.OrdinalIgnoreCase))
                {
                    if (path.StartsWith(UncPathPrefix, StringComparison.OrdinalIgnoreCase))
                    {
                        path = LongPathPrefixForUNCFiles + path.Substring(UncPathPrefix.Length);
                    }
                    else
                    {
                        path = LongPathPrefixForLocalFiles + path;
                    }
                }
            }

            Logger.Logger.Log("Log directory path: {0}", path);

            return path;
        }

        /// <summary>
        /// Deleting all files and directories under a given path
        /// </summary>
        /// <param name="directoryPath">Path to empty</param>
        public static void EmptyDirectory(string directoryPath)
        {
            if (Directory.Exists(directoryPath))
            {
                Logger.Logger.Log("Deleting all files and directories under: {0}", directoryPath);
                DirectoryInfo di = new DirectoryInfo(directoryPath);

                foreach (FileInfo file in di.GetFiles())
                {
                    file.Delete();
                }

                foreach (DirectoryInfo dir in di.GetDirectories())
                {
                    dir.Delete(true);
                }
            }
        }

        /// <summary>
        /// Deletes the given directory; will retry if necessary and wait for things to clean up a bit.
        /// </summary>
        /// <param name="directoryPath">The directory path to delete.</param>
        public static void DeleteDirectoryWithWaitAndRetry(string directoryPath)
        {
            if (Directory.Exists(directoryPath))
            {
                Logger.Logger.Log("Deleting directory under: {0}", directoryPath);
                for (int i = 0; i < 5; ++i)
                {
                    try
                    {
                        Directory.Delete(directoryPath, true);
                        break;
                    }
                    catch
                    {
                        GC.Collect();
                        System.Threading.Thread.Sleep(200);
                    }
                }
            }
        }

        /// <summary>
        /// Normalizes a path for comparison
        /// </summary>
        /// <param name="path">provided path</param>
        /// <returns>The normalized path</returns>
        public static string NormalizePath(string path)
        {
            return Path.GetFullPath(new Uri(path).LocalPath)
                       .TrimEnd(Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar)
                       .ToUpperInvariant();
        }
    }
}
