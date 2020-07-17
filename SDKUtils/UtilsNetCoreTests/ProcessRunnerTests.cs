//----------------------------------------------------------------------------------------------------------------------
// <copyright file="ProcessRunnerTests.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//----------------------------------------------------------------------------------------------------------------------

namespace UtilsNetCoreTests
{
    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using Microsoft.Packaging.SDKUtils.ProcessRunner;
    using System;

    [TestClass]
    public class ProcessRunnerTests : TestBase
    {
        /// <summary>
        /// Test Initialize.
        /// </summary>
        [TestInitialize]
        public new void TestInitialize()
        {
            TestContext.WriteLine("Initializing ProcessRunner test.");
        }

        /// <summary>
        /// Method to test the SDK Tool Process Runner, SDKDetector and MakeAppxRunner.
        /// </summary>
        [TestMethod]
        [ExpectedException(typeof(ArgumentException),
            "Expected exception, as no tool is present in the current directory.")]
        public void SDKToolProcessRunnerTest_InitializeWithNoTool_Failure()
        {
            TestContext.WriteLine("Testing SDKToolProcessRunner and MakeAppxRunner");

            // Test SDKToolProcessRunner and MakeAppxRunner. 
            MakeAppxRunner makeAppxRunner = new MakeAppxRunner();
        }
    }
}
