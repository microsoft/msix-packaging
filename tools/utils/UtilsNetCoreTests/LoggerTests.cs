//----------------------------------------------------------------------------------------------------------------------
// <copyright file="LoggerTests.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//----------------------------------------------------------------------------------------------------------------------

namespace UtilsNetCoreTests
{
    using Microsoft.Msix.Utils.Logger;
    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using System;
    using System.IO;

    [TestClass]
    public class LoggerTests : TestBase
    {
        private string fileName = "UtilsTestFileLogger.txt";
        private string filePath;

        /// <summary>
        /// Test Initialize.
        /// </summary>
        [TestInitialize]
        public new void TestInitialize()
        {
            TestContext.WriteLine("Initializing Logger test.");
            filePath = Path.Combine(Environment.CurrentDirectory.ToString(), fileName);
        }

        /// <summary>
        /// Method to test the Console Logger.
        /// Test fails if any of the methods called in the test method fail.
        /// </summary>
        [TestMethod]
        public void ConsoleLoggerTest()
        {
            TestContext.WriteLine("Testing the Console Logger.");
            LogUtils.SetupConsoleLogger();

            LogProvider consoleLogProvider = Logger.GetLogProvider(typeof(ConsoleLog));
            Assert.IsNotNull(consoleLogProvider);

            consoleLogProvider.LogLevels = Logger.LogLevels.All;

            // Test fails if any of the method calls throw exceptions.
            try
            {
                TestContext.WriteLine("Initialize Console Logger.");
                consoleLogProvider.InitLog();

                TestContext.WriteLine("Logging text to the Console Logger.");
                Logger.Error("This is a trial error message logged to the Console Logger");

                TestContext.WriteLine("Deinitializing the Console Logger.");
                consoleLogProvider.DeinitLog();
                Assert.IsTrue(true);
            }
            catch (Exception exception)
            {
                Assert.IsTrue(false, exception.ToString());
            }
        }

        /// <summary>
        /// Method to test the File Logger.
        /// Test fails if any of the methods called in the test method fail.
        /// </summary>
        [TestMethod]
        public void FileLoggerTest()
        {
            TestContext.WriteLine("Testing the File Logger.");
            string logOutput = "This is an error message logged to the File Logger";

            LogUtils.SetupFileLogger(filePath);

            LogProvider fileLogProvider = Logger.GetLogProvider(typeof(FileLog));
            Assert.IsNotNull(fileLogProvider);

            fileLogProvider.LogLevels = Logger.LogLevels.All;

            // Test fails if any of the method calls throw exceptions.
            try
            {
                TestContext.WriteLine("Initialize the File Logger.");
                fileLogProvider.InitLog();

                TestContext.WriteLine("Logging text to the File Logger.");
                Logger.Error(logOutput);

                // Check if the log file is created.
                TestContext.WriteLine("Verifying the log file creation.");
                Assert.IsTrue(File.Exists(filePath));
                TestContext.WriteLine("Log File Created Successfully.");

                fileLogProvider.DeinitLog();

                // Check the contents of the file.
                TestContext.WriteLine("Verifying the log file contents.");
                if (File.ReadAllText(filePath).Contains(logOutput))
                {
                    TestContext.WriteLine("Log ouput verified.");
                }
                Assert.IsTrue(true);
            }
            catch (Exception exception)
            {
                Assert.IsTrue(false, exception.ToString());
            }
        }

        /// <summary>
        /// Test CleanUp.
        /// </summary>
        [TestCleanup]
        public new void TestCleanup()
        {
            Logger.Deinit();

            // Delete the file created in TestFileLogger
            if (File.Exists(filePath))
            {
                File.Delete(filePath);
            }
        }
    }
}
