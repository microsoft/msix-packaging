// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

namespace UtilsTests
{
    using System;
    using System.IO;
    using Microsoft.VisualStudio.TestTools.UnitTesting.Logging;

    internal class TestHelper : TestBase
    {
        /// <summary>
        /// Helper method to delete a file in the given path.
        /// </summary>
        /// <param name="filePath">Path of the file to be deleted.</param>
        public static void DeleteFileIfExists(string filePath)
        {
            if (File.Exists(filePath))
            {
                try
                {
                    File.Delete(filePath);
                    Logger.LogMessage("{0} Tool deleted in the file directory", filePath);
                }
                catch (Exception exception)
                {
                    Logger.LogMessage("Exception caught while deleting file from test directory."
                        + " File Path: " + filePath + " Exception:"
                        + exception.ToString());
                }
            }
        }
    }
}
