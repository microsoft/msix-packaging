//----------------------------------------------------------------------------------------------------------------------
// <copyright file="LoggerCustomProviderTests.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//----------------------------------------------------------------------------------------------------------------------

namespace UtilsTests
{
    using System;
    using Microsoft.Packaging.SDKUtils.Logger;
    using WEX.Logging.Interop;
    using WEX.TestExecution;
    using WEX.TestExecution.Markup;

    [TestClass]
    internal class LoggerCustomProviderTests : TestBase
    {
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

        /// <summary>
        /// Test Initialize.
        /// </summary>
        [TestInitialize]
        public new void TestInitialize()
        {
            Log.Comment("Initializing LoggerProviderTests test.");
        }

        /// <summary>
        /// Method to test the Custom provider.
        /// </summary>
        [TestMethod]
        public void CustomProviderLoggerTest_NoFormatter()
        {
            Log.Comment("Testing the AddLogProvider without formatter.");

            ConsoleLog consoleLog = new ConsoleLog
            {
                LogLevels = Logger.LogLevels.All,
                LogDecorations = Logger.LogDecorations.All,
            };

            // Don't add a formatter.
            Logger.AddLogProvider(consoleLog);

            Verify.Throws<ArgumentNullException>(() => { Logger.Error("Test logging"); });
        }

        /// <summary>
        /// Method to test the Custom provider.
        /// </summary>
        [TestMethod]
        public void CustomProviderLoggerTest()
        {
            Log.Comment("Testing the AddLogProvider with formatter.");

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
    }
}
