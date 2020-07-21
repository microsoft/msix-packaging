// -----------------------------------------------------------------------
//  <copyright file="FileLog.cs" company="Microsoft">
//      Copyright (c) Microsoft Corporation.  All rights reserved.
//  </copyright>
// -----------------------------------------------------------------------
namespace Microsoft.Packaging.Utils.Logger
{
    using System;
    using System.IO;

    /// <summary>
    /// File log class
    /// </summary>
    public class FileLog : LogProvider, IDisposable
    {
        /// <summary>
        /// File log stream writer
        /// </summary>
        private StreamWriter streamWriterLog;

        /// <summary>
        /// Gets or sets the log file path
        /// </summary>
        public string LogFilePath { get; set; }

        /// <summary>
        /// Gets or sets a value indicating whether to override previous log file name
        /// </summary>
        public bool Overwrite { get; set; }

        /// <summary>
        /// Gets or sets a value indicating whether or not to flush file stream to the disk
        /// </summary>
        public bool FlushEachLine { get; set; }

        #region InitLog
        /// <summary>
        /// Initializes the file log
        /// </summary>
        public override void InitLog()
        {
            this.CreateLogFile();
        }
        #endregion

        /// <summary>
        /// Log a message
        /// </summary>
        /// <param name="logMessage">log message</param>
        public override void Log(ILogMessage logMessage)
        {
            if (logMessage == null)
            {
                throw new ArgumentNullException("logMessage");
            }

            this.streamWriterLog.WriteLine(logMessage.GetLogMessage(this.LogDecorations, this.LogLevels));

            // If you want to debug crashing apps - then make sure to flush each and every log line.
            if (this.FlushEachLine)
            {
                this.streamWriterLog.Flush();
            }
        }

        /// <summary>
        /// De-initializes the file log
        /// </summary>
        public override void DeinitLog()
        {
            if (this.streamWriterLog != null)
            {
                this.streamWriterLog.Flush();
                this.streamWriterLog.Close();
                this.streamWriterLog = null;
            }

            this.DeleteFileIfEmpty();
        }

        #region IDisposable
        public void Dispose()
        {
            this.DeinitLog();
            GC.SuppressFinalize(this);
        }

        #endregion

        #region GetUniqueFileNameAsCount
        /// <summary>
        /// Cycle through until we find a filename that is not currently used.
        /// This will be in the format of filename#number.extension
        /// </summary>
        /// <param name="path">filename path</param>
        /// <returns>returns a filename string in format of filename#number.ext</returns>
        private static string GetUniqueFileNameAsCount(string path)
        {
            string uniquePath = path;

            // Get extension if it exists
            FileInfo fileInfo = new FileInfo(uniquePath);
            string filename = uniquePath;
            string extension = fileInfo.Extension;

            if (!string.IsNullOrEmpty(extension))
            {
                int lastIndex = fileInfo.FullName.LastIndexOf(extension, StringComparison.OrdinalIgnoreCase);
                filename = fileInfo.FullName.Substring(0, lastIndex);
            }

            int count = 0;

            while (File.Exists(uniquePath))
            {
                uniquePath = filename + count + extension;
                count++;
            }

            return uniquePath;
        }
        #endregion

        #region CreateLogFile
        /// <summary>
        /// Creates the log file
        /// </summary>
        private void CreateLogFile()
        {
            if (this.streamWriterLog == null)
            {
                string path = this.LogFilePath;
                if (!string.IsNullOrEmpty(path))
                {
                    if (!this.Overwrite)
                    {
                        path = GetUniqueFileNameAsCount(path);
                    }

                    this.streamWriterLog = File.CreateText(path);
                    this.LogFilePath = path;
                }
            }
        }
        #endregion

        #region DeleteFileIfEmpty
        /// <summary>
        /// Called during de-initialization to delete the log file in case it's empty
        /// </summary>
        private void DeleteFileIfEmpty()
        {
            // If the log file is empty, delete it
            if (File.Exists(this.LogFilePath))
            {
                FileInfo fileInfo = new FileInfo(this.LogFilePath);
                if (fileInfo != null)
                {
                    if (fileInfo.Length == 0)
                    {
                        File.Delete(this.LogFilePath);
                    }
                }
            }
        }
        #endregion
    }
}
