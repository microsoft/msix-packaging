// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

namespace UtilsTests
{
    using System;
    using System.IO;
    using Microsoft.Msix.Utils.Logger;
    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using static Microsoft.VisualStudio.TestTools.UnitTesting.Logging.Logger;

    [TestClass]
    public class LoggerTests : TestBase
    {
        private string fileName = "UtilsTestFileLogger.txt";
        private string fileDirectory = Environment.CurrentDirectory.ToString();
        private string filePath;

        /// <summary>
        /// Test Initialize.
        /// </summary>
        [TestInitialize]
        public new void TestInitialize()
        {
            LogMessage("Initializing Logger test.");
            this.filePath = Path.Combine(this.fileDirectory, this.fileName);
        }

        /// <summary>
        /// Method to test the Console Logger.
        /// </summary>
        [TestMethod]
        public void ConsoleLoggerTest()
        {
            LogMessage("Testing the Console Logger.");

            // Several of these operations are only to ensure they succeed, i.e. do not throw.
            LogUtils.SetupConsoleLogger();

            LogProvider consoleLogProvider = Logger.GetLogProvider(typeof(ConsoleLog));
            Assert.IsNotNull(consoleLogProvider);

            consoleLogProvider.LogLevels = Logger.LogLevels.All;

            LogMessage("Initialize Console Logger.");
            consoleLogProvider.InitLog();

            LogMessage("Logging text to the Console Logger.");
            Logger.Error("This is a trial error message logged to the Console Logger");

            LogMessage("Logging malformed string to the Console Logger.");
            Action logStringOperation = () =>
            {
                Logger.Error("This is a malformed {0} string logged to the Console Logger", null);
            };
            Assert.ThrowsException<Exception>(logStringOperation);

            LogMessage("Deinitializing the Console Logger.");
            consoleLogProvider.DeinitLog();
        }

        /// <summary>
        /// Method to test the Console Logger log level critical.
        /// </summary>
        [TestMethod]
        public void ConsoleLoggerCriticalLogLevelTest()
        {
            LogMessage("Testing the Console Logger critical log level.");

            // Several of these operations are only to ensure they succeed, i.e. do not throw.
            LogUtils.SetupConsoleLogger();

            LogProvider consoleLogProvider = Logger.GetLogProvider(typeof(ConsoleLog));
            Assert.IsNotNull(consoleLogProvider);

            consoleLogProvider.LogLevels = Logger.LogLevels.All;

            LogMessage("Initialize Console Logger.");
            consoleLogProvider.InitLog();

            LogMessage("Logging text to the Console Logger.");
            Logger.Critical("This is a trial critical message logged to the Console Logger");

            LogMessage("Logging malformed string to the Console Logger.");
            Action logStringOperation = () =>
            {
                Logger.Critical("This is a malformed {0} string logged to the Console Logger", null);
            };
            Assert.ThrowsException<Exception>(logStringOperation);

            LogMessage("Deinitializing the Console Logger.");
            consoleLogProvider.DeinitLog();
        }

        /// <summary>
        /// Method to test the File Logger.
        /// </summary>
        [TestMethod]
        public void FileLoggerTest()
        {
            LogMessage("Testing the File Logger.");

            LogUtils.SetupFileLogger(this.filePath);
            LogProvider fileLogProvider = Logger.GetLogProvider(typeof(FileLog));
            fileLogProvider.LogLevels = Logger.LogLevels.All;

            LogMessage("Initialize the File Logger.");
            fileLogProvider.InitLog();

            LogMessage("Logging text to the File Logger.");
            string logOutput = "This is an error message logged to the File Logger";
            Logger.Error(logOutput);

            // Check if the log file is created.
            LogMessage("Verifying the log file creation.");
            Assert.IsTrue(File.Exists(this.filePath));
            fileLogProvider.DeinitLog();

            // Check the contents of the file.
            LogMessage("Verifying the log file contents.");
            Assert.IsTrue(File.ReadAllText(this.filePath).Contains(logOutput));
        }

        /// <summary>
        /// Method to test the File Logger critical log level.
        /// </summary>
        [TestMethod]
        public void FileLoggerCriticalLogLevelTest()
        {
            LogMessage("Testing the File Logger critical log level.");

            LogUtils.SetupFileLogger(this.filePath);
            LogProvider fileLogProvider = Logger.GetLogProvider(typeof(FileLog));
            fileLogProvider.LogLevels = Logger.LogLevels.All;

            LogMessage("Initialize the File Logger.");
            fileLogProvider.InitLog();

            LogMessage("Logging text to the File Logger.");
            string logOutput = "This is a critical message logged to the File Logger";
            Logger.Critical(logOutput);

            // Check if the log file is created.
            LogMessage("Verifying the log file creation.");
            Assert.IsTrue(File.Exists(this.filePath));
            fileLogProvider.DeinitLog();

            // Check the contents of the file.
            LogMessage("Verifying the log file contents.");
            Assert.IsTrue(File.ReadAllText(this.filePath).Contains(logOutput));
        }

        /// <summary>
        /// Method to test the logger event
        /// </summary>
        [TestMethod]
        public void LogEventTest()
        {
            LogMessage("Testing log events");
            LogUtils.SetupFileLogger(this.filePath);

            int x = 0;
            Logger.MessageLoggedEvent += (object source, Logger.LogEventArgs eventArgs) =>
            {
                x++;

                Logger.Info("This is logged inside the event callback", Logger.LogFlags.DisableLogEventsFiring);
            };

            Logger.Info("Message 1");
            Logger.Info("Message 2");
            Logger.Info("Message 3");

            Assert.AreEqual(3, x, "Verifying the callback was called 3 times");

            Logger.Deinit();

            string logText = File.ReadAllText(this.filePath);
            Assert.IsTrue(logText.Contains("Message 1"), "Verifying message 1 was printed");
            Assert.IsTrue(logText.Contains("Message 2"), "Verifying message 2 was printed");
            Assert.IsTrue(logText.Contains("Message 3"), "Verifying message 3 was printed");
            Assert.IsTrue(logText.Contains("inside the event callback"), "Verifying log inside callback was printed");
        }

        /// <summary>
        /// Test CleanUp.
        /// </summary>
        [TestCleanup]
        public new void TestCleanup()
        {
            Logger.Deinit();

            // Delete the file created in TestFileLogger
            TestHelper.DeleteFileIfExists(this.filePath);
        }
    }
}