// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

namespace UtilsTests
{
    using System;
    using Microsoft.Msix.Utils.Logger;
    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using static Microsoft.VisualStudio.TestTools.UnitTesting.Logging.Logger;

    [TestClass]
    internal class LoggerCustomProviderTests : TestBase
    {
        /// <summary>
        /// Test Initialize.
        /// </summary>
        [TestInitialize]
        public new void TestInitialize()
        {
            LogMessage("Initializing LoggerProviderTests test.");
        }

        /// <summary>
        /// Method to test the Custom provider.
        /// </summary>
        [TestMethod]
        public void CustomProviderLoggerTest_NoFormatter()
        {
            LogMessage("Testing the AddLogProvider without formatter.");

            ConsoleLog consoleLog = new ConsoleLog
            {
                LogLevels = Logger.LogLevels.All,
                LogDecorations = Logger.LogDecorations.All,
            };

            // Don't add a formatter.
            Logger.AddLogProvider(consoleLog);

            Assert.ThrowsException<ArgumentNullException>(() => { Logger.Error("Test logging"); });
        }

        /// <summary>
        /// Method to test the Custom provider.
        /// </summary>
        [TestMethod]
        public void CustomProviderLoggerTest()
        {
            LogMessage("Testing the AddLogProvider with formatter.");

            ConsoleLog consoleLog = new ConsoleLog
            {
                LogLevels = Logger.LogLevels.All,
                LogDecorations = Logger.LogDecorations.All,
            };

            // Add provider and formatter.
            Logger.AddLogProvider(consoleLog);
            Logger.RegisterFormatter(new LogMessageFormatter());

            Logger.Error("Test logging");
        }

        /// <summary>
        /// Test CleanUp.
        /// </summary>
        [TestCleanup]
        public new void TestCleanup()
        {
            Logger.Deinit();
        }

        /// <summary>
        /// Custom provider for test.
        /// </summary>
        internal class CustomConsoleLog : LogProvider
        {
            public override void Log(ILogMessage logMessage)
            {
                if (logMessage == null)
                {
                    throw new ArgumentNullException("logMessage");
                }

                Console.Error.WriteLine(logMessage.GetLogMessage(this.LogDecorations, this.LogLevels));
            }

            /// <summary>
            /// De-initializes the console log
            /// </summary>
            public override void DeinitLog()
            {
            }
        }
    }
}
