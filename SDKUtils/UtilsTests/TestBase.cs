//----------------------------------------------------------------------------------------------------------------------
// <copyright file="TestBase.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//----------------------------------------------------------------------------------------------------------------------

namespace UtilsTests
{
    using Microsoft.Packaging.SDKUtils.Logger;
    using WEX.Logging.Interop;
    using WEX.TestExecution.Markup;

    [TestClass]
    internal class TestBase
    {
        // This property is needed to be able to work with runtime parameters from within the test
        // TAEF will automatically set this property at runtime to the current TestContext object.
        private TestContext testContext;

        /// <summary>
        /// Gets or sets the test context; set by TAEF at runtime for each test.
        /// </summary>
        public TestContext TestContext
        {
            get { return this.testContext; }
            set { this.testContext = value; }
        }

        /// <summary>
        /// Class initialize method.
        /// </summary>
        [ClassInitialize]
        public static void ClassInitialize()
        {
            Log.Comment("\n************************************************************");
            Log.Comment("Class Initialize");

            LogUtils.SetupLogger("Peet Utils Tests", 2018, true);
            Log.Comment("Log file is located under: " + ((FileLog)Logger.GetLogProvider(typeof(FileLog))).LogFilePath);

            Log.Comment("Ending Class Initialize");
            Log.Comment("************************************************************\n");
        }

        /// <summary>
        /// Class cleanup method.
        /// </summary>
        [ClassCleanup]
        public static void ClassCleanup()
        {
            Log.Comment("\n************************************************************");
            Log.Comment("Test Base Class Cleanup");

            Logger.Deinit();

            Log.Comment("Ending Test Base Class Cleanup");
            Log.Comment("************************************************************\n");
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
