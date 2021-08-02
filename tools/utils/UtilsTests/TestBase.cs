﻿// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

namespace UtilsTests
{
    using Microsoft.Msix.Utils.Logger;
    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using static Microsoft.VisualStudio.TestTools.UnitTesting.Logging.Logger;

    [TestClass]
    public class TestBase
    {
        /// <summary>
        /// Gets or sets the test context; set by the test framework at runtime for each test.
        /// </summary>
        public TestContext TestContext { get; set; }

        /// <summary>
        /// Class initialize method.
        /// </summary>
        /// <param name="testContext">Test context. Unused, required by ClassInitialize.</param>
        [ClassInitialize]
        public static void ClassInitialize(TestContext testContext)
        {
            LogMessage("\n************************************************************");
            LogMessage("Class Initialize");

            LogUtils.SetupLogger("MSIX Utils Tests", 2018, true);
            LogMessage("Log file is located under: " + ((FileLog)Logger.GetLogProvider(typeof(FileLog))).LogFilePath);

            LogMessage("Ending Class Initialize");
            LogMessage("************************************************************\n");
        }

        /// <summary>
        /// Class cleanup method.
        /// </summary>
        [ClassCleanup]
        public static void ClassCleanup()
        {
            LogMessage("\n************************************************************");
            LogMessage("Test Base Class Cleanup");

            Logger.Deinit();

            LogMessage("Ending Test Base Class Cleanup");
            LogMessage("************************************************************\n");
        }

        /// <summary>
        /// Test Initialize.
        /// </summary>
        [TestInitialize]
        public void TestInitialize()
        {
        }

        /// <summary>
        /// Test Cleanup.
        /// </summary>
        [TestCleanup]
        public void TestCleanup()
        {
        }
    }
}
