//----------------------------------------------------------------------------------------------------------------------
// <copyright file="TestBase.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//----------------------------------------------------------------------------------------------------------------------

namespace UtilsNetCoreTests
{
    using Microsoft.Msix.Utils.Logger;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    [TestClass]
    public class TestBase
    {
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
            LogUtils.SetupLogger("Peet Utils Tests", 2018, true);
        }

        /// <summary>
        /// Class cleanup method.
        /// </summary>
        [ClassCleanup]
        public static void ClassCleanup()
        {
            Logger.Deinit();
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
