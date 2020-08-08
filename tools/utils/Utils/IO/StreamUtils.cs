//-----------------------------------------------------------------------
// <copyright file="StreamUtils.cs" company="Microsoft">
//     Copyright (c) Microsoft. All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

namespace Microsoft.Msix.Utils
{
    using System;
    using System.Diagnostics.CodeAnalysis;
    using System.IO;
    using System.Reflection;
    using System.Runtime.InteropServices;
    using System.Runtime.InteropServices.ComTypes;

    using STATSTG = System.Runtime.InteropServices.ComTypes.STATSTG;

    /// <summary>
    /// Utility functions for working with IStream.
    /// </summary>
    public class StreamUtils
    {
        [SuppressMessage("StyleCop.CSharp.DocumentationRules", "SA1600:ElementsMustBeDocumented", Justification = "Interop")]
        [SuppressMessage("StyleCop.CSharp.DocumentationRules", "SA1602:EnumerationItemsMustBeDocumented", Justification = "Interop")]
        [Flags]
        private enum STGM
        {
            STGM_READ = 0x00000000,
            STGM_WRITE = 0x00000001,
            STGM_READWRITE = 0x00000002,

            STGM_SHARE_EXCLUSIVE = 0x00000010,
            STGM_CREATE = 0x00001000
        }

        /// <summary>
        /// Create an IStream to read from a file.
        /// </summary>
        /// <param name="fileName">Name of the file to open.</param>
        /// <param name="cloudStreamHandler">The instance object handling cloud stream.</param>
        /// <returns>An IStream to read from file fileName</returns>
        /// <exception cref="ArgumentNullException">fileName is null.</exception>
        /// <exception cref="FileNotFoundException">The file specified in fileName was not
        /// found.</exception>
        public static IStream CreateInputStreamOnFile(string fileName, object cloudStreamHandler = null)
        {
            var mode = STGM.STGM_READ;
            uint attributes = 0; // Doesn't matter; only useful when creating.
            bool create = false; // Don't create the file if it doesn't already exist.
            return CreateStreamOnFile(fileName, mode, attributes, create, cloudStreamHandler);
        }

        /// <summary>
        /// Create an IStream to write to a file.
        /// </summary>
        /// <param name="fileName">Name of the file to create.</param>
        /// <param name="cloudStreamHandler">The instance object handling cloud stream.</param>
        /// <returns>An IStream to write to file fileName</returns>
        /// <exception cref="ArgumentNullException">fileName is null.</exception>
        /// <exception cref="FileNotFoundException">The file specified in fileName was not
        /// found.</exception>
        public static IStream CreateOutputStreamOnFile(string fileName, object cloudStreamHandler = null)
        {
            var mode = STGM.STGM_WRITE | STGM.STGM_CREATE;
            uint attributes = 0; // Default attributes
            bool create = true;
            return CreateStreamOnFile(fileName, mode, attributes, create, cloudStreamHandler);
        }

        /// <summary>
        /// Create an IStream to read and write to a file.
        /// </summary>
        /// <param name="fileName">Name of the file.</param>
        /// <param name="create">Indicates whether the file should be created or opened as is.</param>
        /// <param name="cloudStreamHandler">The instance object handling cloud stream.</param>
        /// <returns>An IStream to access the file fileName</returns>
        /// <exception cref="ArgumentNullException">fileName is null.</exception>
        /// <exception cref="FileNotFoundException">The file specified in fileName was not
        /// found.</exception>
        public static IStream CreateInputOutputStreamOnFile(string fileName, bool create = true, object cloudStreamHandler = null)
        {
            var mode = STGM.STGM_READWRITE;
            if (create)
            {
                mode |= STGM.STGM_CREATE;
            }

            uint attributes = 0; // Default attributes
            return CreateStreamOnFile(fileName, mode, attributes, create, cloudStreamHandler);
        }

        /// <summary>
        /// Clone a stream and move the clone's seek pointer to the beginning of the stream.
        /// The clone stream uses the same data as the original, so the changes made in one are immediately visible in the other.
        /// </summary>
        /// <param name="original">The stream to clone.</param>
        /// <returns>A clone of the original stream with its seek pointer pointing to the beginning of the stream.</returns>
        public static IStream CloneAndResetStream(IStream original)
        {
            IStream clone;
            original.Clone(out clone);
            IStream_Reset(clone);
            return clone;
        }

        /// <summary>
        /// Get the size of a stream.
        /// </summary>
        /// <param name="stream">An IStream object.</param>
        /// <returns>The size of stream.</returns>
        public static long GetStreamSize(IStream stream)
        {
            STATSTG stats;
            const int StatFlag = 1; // STATFLAG_NONAME: Dont't return the name.
            stream.Stat(out stats, StatFlag);
            return stats.cbSize;
        }

        /// <summary>
        /// Copy the contents of one string into another one.
        /// </summary>
        /// <param name="source">The source stream.</param>
        /// <param name="target">The target stream.</param>
        public static void CopyContents(IStream source, IStream target)
        {
            IntPtr aux = IntPtr.Zero;
            source.CopyTo(target, long.MaxValue, aux, aux);
        }

        /// <summary>
        /// Write a stream to a file.
        /// </summary>
        /// <param name="source">Source stream.</param>
        /// <param name="fileName">Path of the target file.</param>
        public static void WriteStreamToFile(IStream source, string fileName)
        {
            Directory.CreateDirectory(Path.GetDirectoryName(fileName));
            source = CloneAndResetStream(source);
            using (var target = new FileStream(fileName, FileMode.OpenOrCreate))
            {
                const int BufferSize = 1024 * 1024;
                byte[] buffer = new byte[BufferSize];
                int left = (int)GetStreamSize(source);
                while (left > 0)
                {
                    source.Read(buffer, BufferSize, IntPtr.Zero);
                    int read = Math.Min(left, BufferSize);
                    left -= read;
                    target.Write(buffer, 0, read);
                }
            }
        }

        /// <summary>
        /// Write the stream to a new temporary file.
        /// </summary>
        /// <param name="source">Source stream.</param>
        /// <returns>Path to the new temporary file.</returns>
        public static string WriteStreamToTempFile(IStream source)
        {
            string tempFileName = Path.GetTempFileName();
            WriteStreamToFile(source, tempFileName);
            return tempFileName;
        }

        /// <summary>
        /// Reads the stream to a byte array.
        /// </summary>
        /// <param name="source">Source stream.</param>
        /// <returns>A new byte array with the contents of the stream.</returns>
        public static byte[] ReadStreamToByteArray(IStream source)
        {
            source = CloneAndResetStream(source);
            int size = (int)GetStreamSize(source);
            byte[] target = new byte[size];
            source.Read(target, size, IntPtr.Zero);
            return target;
        }

        /// <summary>
        /// Reads from a stream.
        /// </summary>
        /// <param name="stream">The stream to read from.</param>
        /// <param name="buffer">The buffer to write to.</param>
        /// <returns>The number of bytes read.</returns>
        public static int ReadStream(IStream stream, byte[] buffer)
        {
            IntPtr bytesReadPtr = Marshal.AllocCoTaskMem(Marshal.SizeOf(typeof(int)));
            try
            {
                Marshal.WriteInt32(bytesReadPtr, 0);
                stream.Read(buffer, buffer.Length, bytesReadPtr);
                return Marshal.ReadInt32(bytesReadPtr);
            }
            finally
            {
                Marshal.FreeCoTaskMem(bytesReadPtr);
            }
        }

        /// <summary>
        /// Opens a file and retrieves a stream to read or write to that file.
        /// </summary>
        /// <param name="fileName">File name.</param>
        /// <param name="mode">STGM values that specify the file access mode.</param>
        /// <param name="attributes">Flag values that specify file attributes in case that a new
        /// file is created.</param>
        /// <param name="create">Value that specify how existing files should be treated.</param>
        /// <param name="cloudStreamHandler">The instance object handling cloud stream.</param>
        /// <returns>the stream to the file</returns>
        /// <remarks>
        /// This is a wrapper around SHCreateStreamOnFileEx that makes the errors easier to
        /// understand. See the documentation of SHCreateStreamOnFileEx for details on the parameters.
        /// </remarks>
        /// <exception cref="ArgumentNullException">fileName is null.</exception>
        /// <exception cref="FileNotFoundException">The file specified in fileName was not
        /// found.</exception>
        [SuppressMessage("StyleCop.CSharp.ReadabilityRules", "SA1121:UseBuiltInTypeAlias", Justification = "Interop")]
        private static IStream CreateStreamOnFile(
            string fileName,
            STGM mode,
            uint attributes,
            bool create,
            object cloudStreamHandler)
        {
            if (fileName == null)
            {
                throw new ArgumentNullException("fileName");
            }

            try
            {
                if (IsValidCloudStreamUri(fileName) && cloudStreamHandler != null)
                {
                    Type cloudStreamHandlerType = cloudStreamHandler.GetType();
                    MethodInfo getStreamMethod = cloudStreamHandlerType.GetMethod("GetStream");

                    // MethodInfo equal operator is not defined on TestNet, so we cast to object to do null check
                    if ((object)getStreamMethod == null)
                    {
                        throw new MissingMethodException("The GetStream method cannot be found.");
                    }

                    object[] args = new object[] { fileName, (UInt32)mode, null };
                    getStreamMethod.Invoke(cloudStreamHandler, args);
                    if (args[2] != null)
                    {
                        return (IStream)args[2];
                    }
                    else
                    {
                        throw new FileNotFoundException("Failed to create cloud stream on input address.", fileName);
                    }
                }
                else
                {
                    return SHCreateStreamOnFileEx(fileName, mode, attributes, create, null);
                }
            }
            catch (FileNotFoundException e)
            {
                // Ensure that the exception has a file name attached.
                if (string.IsNullOrEmpty(e.FileName))
                {
                    throw new FileNotFoundException(e.Message, fileName, e.InnerException);
                }
                else
                {
                    throw e;
                }
            }
        }

        /// <summary>
        /// Determines if a path is a cloud stream url by checking url schemes.
        /// </summary>
        /// <param name="inputPath">The input path.</param>
        /// <returns>True if the path is a valid cloud stream uri, false otherwise.</returns>
        private static bool IsValidCloudStreamUri(string inputPath)
        {
            // The Uri related functions are not available in TestNet, so we only check uri schemes by String.StartsWith
            return inputPath.StartsWith("http://", StringComparison.OrdinalIgnoreCase) ||
                inputPath.StartsWith("https://", StringComparison.OrdinalIgnoreCase);
        }

        [SuppressMessage("StyleCop.CSharp.DocumentationRules", "SA1600:ElementsMustBeDocumented", Justification = "Interop")]
        [DllImport("shlwapi.dll", CharSet = CharSet.Unicode, PreserveSig = false)]
        private static extern IStream SHCreateStreamOnFileEx(
            [In] string fileName,
            [In] STGM mode,
            [In] uint attributes,
            [In] bool create,
            [In] IStream template);

        [SuppressMessage("StyleCop.CSharp.DocumentationRules", "SA1600:ElementsMustBeDocumented", Justification = "Interop")]
        [DllImport("shlwapi.dll", PreserveSig = false)]
        private static extern void IStream_Reset([In] IStream stream);
    }
}
