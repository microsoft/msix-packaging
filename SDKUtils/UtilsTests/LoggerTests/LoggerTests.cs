//----------------------------------------------------------------------------------------------------------------------
// <copyright file="LoggerTests.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//----------------------------------------------------------------------------------------------------------------------

namespace UtilsTests
{
    using System;
    using System.IO;
    using Microsoft.Packaging.SDKUtils.Logger;
    using WEX.Logging.Interop;
    using WEX.TestExecution;
    using WEX.TestExecution.Markup;

    [TestClass]
    internal class LoggerTests : TestBase
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
            Log.Comment("Initializing Logger test.");
            this.filePath = Path.Combine(this.fileDirectory, this.fileName);
        }

        /// <summary>
        /// Method to test the Console Logger.
        /// </summary>
        [TestMethod]
        public void ConsoleLoggerTest()
        {
            Log.Comment("Testing the Console Logger.");

            // VerfiyOperation makes sure no exceptions are thrown my SetUpConsoleLogger, which
            // indicates the success of the method call.
            VerifyOperation setUpConsoleLogger = new VerifyOperation(LogUtils.SetupConsoleLogger);
            Verify.NoThrow(setUpConsoleLogger);

            LogProvider consoleLogProvider = Logger.GetLogProvider(typeof(ConsoleLog));
            Verify.IsNotNull(consoleLogProvider);

            consoleLogProvider.LogLevels = Logger.LogLevels.All;

            Log.Comment("Initialize Console Logger.");
            VerifyOperation initLog = new VerifyOperation(consoleLogProvider.InitLog);
            Verify.NoThrow(initLog);

            Log.Comment("Logging text to the Console Logger.");
            VerifyOperation logOperation = delegate
            {
                Logger.Error("This is a trial error message logged to the Console Logger");
            };
            Verify.NoThrow(logOperation);

            Log.Comment("Logging malformed string to the Console Logger.");
            VerifyOperation logStringOperation = delegate
            {
                Logger.Error("This is a malformed {0} string logged to the Console Logger", null);
            };
            Verify.Throws<Exception>(logStringOperation);

            Log.Comment("Deinitializing the Console Logger.");
            consoleLogProvider.DeinitLog();
        }

        /// <summary>
        /// Method to test the Console Logger log level critical.
        /// </summary>
        [TestMethod]
        public void ConsoleLoggerCriticalLogLevelTest()
        {
            Log.Comment("Testing the Console Logger critical log level.");

            // VerfiyOperation makes sure no exceptions are thrown my SetUpConsoleLogger, which
            // indicates the success of the method call.
            VerifyOperation setUpConsoleLogger = new VerifyOperation(LogUtils.SetupConsoleLogger);
            Verify.NoThrow(setUpConsoleLogger);

            LogProvider consoleLogProvider = Logger.GetLogProvider(typeof(ConsoleLog));
            Verify.IsNotNull(consoleLogProvider);

            consoleLogProvider.LogLevels = Logger.LogLevels.All;

            Log.Comment("Initialize Console Logger.");
            VerifyOperation initLog = new VerifyOperation(consoleLogProvider.InitLog);
            Verify.NoThrow(initLog);

            Log.Comment("Logging text to the Console Logger.");
            VerifyOperation logOperation = delegate
            {
                Logger.Critical("This is a trial critical message logged to the Console Logger");
            };
            Verify.NoThrow(logOperation);

            Log.Comment("Logging malformed string to the Console Logger.");
            VerifyOperation logStringOperation = delegate
            {
                Logger.Critical("This is a malformed {0} string logged to the Console Logger", null);
            };
            Verify.Throws<Exception>(logStringOperation);

            Log.Comment("Deinitializing the Console Logger.");
            consoleLogProvider.DeinitLog();
        }

        /// <summary>
        /// Method to test the File Logger.
        /// </summary>
        [TestMethod]
        public void FileLoggerTest()
        {
            Log.Comment("Testing the File Logger.");

            LogUtils.SetupFileLogger(this.filePath);
            LogProvider fileLogProvider = Logger.GetLogProvider(typeof(FileLog));
            fileLogProvider.LogLevels = Logger.LogLevels.All;

            Log.Comment("Initialize the File Logger.");
            VerifyOperation initLog = new VerifyOperation(fileLogProvider.InitLog);
            Verify.NoThrow(initLog);

            Log.Comment("Logging text to the File Logger.");
            string logOutput = "This is an error message logged to the File Logger";
            Logger.Error(logOutput);

            // Check if the log file is created.
            Log.Comment("Verifying the log file creation.");
            Verify.IsTrue(File.Exists(this.filePath));
            fileLogProvider.DeinitLog();

            // Check the contents of the file.
            Log.Comment("Verifying the log file contents.");
            Verify.IsTrue(File.ReadAllText(this.filePath).Contains(logOutput));
        }

        /// <summary>
        /// Method to test the File Logger critical log level.
        /// </summary>
        [TestMethod]
        public void FileLoggerCriticalLogLevelTest()
        {
            Log.Comment("Testing the File Logger critical log level.");

            LogUtils.SetupFileLogger(this.filePath);
            LogProvider fileLogProvider = Logger.GetLogProvider(typeof(FileLog));
            fileLogProvider.LogLevels = Logger.LogLevels.All;

            Log.Comment("Initialize the File Logger.");
            VerifyOperation initLog = new VerifyOperation(fileLogProvider.InitLog);
            Verify.NoThrow(initLog);

            Log.Comment("Logging text to the File Logger.");
            string logOutput = "This is a critical message logged to the File Logger";
            Logger.Critical(logOutput);

            // Check if the log file is created.
            Log.Comment("Verifying the log file creation.");
            Verify.IsTrue(File.Exists(this.filePath));
            fileLogProvider.DeinitLog();

            // Check the contents of the file.
            Log.Comment("Verifying the log file contents.");
            Verify.IsTrue(File.ReadAllText(this.filePath).Contains(logOutput));
        }

        /// <summary>
        /// Method to test the logger event
        /// </summary>
        [TestMethod]
        public void LogEventTest()
        {
            Log.Comment("Testing log events");
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

            Verify.AreEqual(3, x, "Verifying the callback was called 3 times");

            Logger.Deinit();

            string logText = File.ReadAllText(this.filePath);
            Verify.IsTrue(logText.Contains("Message 1"), "Verifying message 1 was printed");
            Verify.IsTrue(logText.Contains("Message 2"), "Verifying message 2 was printed");
            Verify.IsTrue(logText.Contains("Message 3"), "Verifying message 3 was printed");
            Verify.IsTrue(logText.Contains("inside the event callback"), "Verifying log inside callback was printed");
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