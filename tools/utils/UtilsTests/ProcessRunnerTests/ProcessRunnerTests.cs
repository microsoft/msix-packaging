// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

namespace UtilsTests
{
    using System;
    using System.IO;
    using System.Threading;
    using Microsoft.Msix.Utils.ProcessRunner;
    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using Microsoft.VisualStudio.TestTools.UnitTesting.Logging;

    [TestClass]
    public class ProcessRunnerTests : TestBase
    {
        private static string toolName = "MakeAppx.exe";
        private static string appxBundle = "TestAppxBundle.appxbundle";

        // Directory with the test data.
        private string testDataDirectory;
        private string testToolDirectory;

        // Directory from which the tests are run.
        private string testDirectory;

        /// <summary>
        /// Test Initialize.
        /// </summary>
        [TestInitialize]
        public new void TestInitialize()
        {
            Logger.LogMessage("Initializing ProcessRunner test.");

            // Copy the tool to the current directory for testing.
            this.testDirectory = this.TestContext.TestRunDirectory;
            this.testDataDirectory = Path.Combine(this.TestContext.TestDeploymentDir, "TestData");
            this.testToolDirectory = Path.Combine(this.testDataDirectory, "MakeAppx");
            try
            {
                if (!File.Exists(Path.Combine(this.testDirectory, toolName)))
                {
                    File.Copy(Path.Combine(this.testToolDirectory, toolName), Path.Combine(this.testDirectory, toolName));
                }
            }
            catch (Exception exception)
            {
                Logger.LogMessage("Error in Test Initialize." + exception.ToString());
            }
        }

        /// <summary>
        /// Test CleanUp.
        /// </summary>
        [TestCleanup]
        public new void TestCleanup()
        {
            TestHelper.DeleteFileIfExists(Path.Combine(this.testDirectory, toolName));
            TestHelper.DeleteFileIfExists(Path.Combine(this.testDirectory, appxBundle));
        }

        /// <summary>
        /// Method to test the SDKToolProcessRunner, SDKDetector and MakeAppxRunner.
        /// </summary>
        [TestMethod]
        public void SDKToolProcessRunnerTest_IntializeFromCurrentPath_Success()
        {
            Logger.LogMessage("Testing SDKToolProcessRunner, SDKDetector, MakeAppxRunner");
            Logger.LogMessage("MakeAppxRunner looks for the tool next to current assembly." +
                "If not found, looks for it in SDK Installation folder.");

            // Verify success of initialization.
            new MakeAppxRunner();
            Logger.LogMessage("MakeAppxRunner and SDKToolProcessRunner initialized successfully.");
        }

        /// <summary>
        /// Method to test the SDKToolProcessRunner, SDKDetector and MakeAppxRunner.
        /// Verifies that MakeAppxRunner throws an exception as the required tool is missing.
        /// </summary>
        /// <remarks>
        /// This test only succeeds if the machine running it does not have an installed SDK.
        /// This is ignored by default as test machines are likely to have an installed SDK.
        /// </remarks>
        [TestMethod]
        [Ignore]
        public void SDKToolProcessRunnerTest_InitializeFromToolPath_Failure()
        {
            // Delete the file from current path if it exists.
            if (File.Exists(Path.Combine(this.testDirectory, toolName)))
            {
                File.Delete(Path.Combine(this.testDirectory, toolName));
                Logger.LogMessage("File Deleted in the test directory");
            }

            // If the tool doesn't exist in the tool path and SDK is NOT installled,
            // Argument exception should be thrown.
            Action verifyOperation = () => { new MakeAppxRunner(); };
            Assert.ThrowsException<ArgumentException>(verifyOperation);

            Logger.LogMessage("MakeAppxRunner and SDKToolProcessRunner throw exception as expected.");
        }

        /// <summary>
        /// Method to test the SDKToolProcessRunner, SDKDetector and MakeAppxRunner
        /// by passing the tool directory.
        /// </summary>
        [TestMethod]
        public void SDKToolProcessRunnerTest_InitializeFromToolDirectory_GenerateAppxBundle_Success()
        {
            Logger.LogMessage("Testing SDKToolProcessRunner, SDKDetector, MakeAppxRunner" +
                " by passing the Tool Directory.");

            MakeAppxRunner runner = new MakeAppxRunner(this.testToolDirectory)
            {
                OverwriteExistingFiles = true,
            };
            Logger.LogMessage("MakeAppxRunner and SDKToolProcessRunner initialized successfully.");

            // Call GenerateAppxBundleFromRootFolder to test it.
            runner.GenerateAppxBundleFromRootFolder(
                "1.0.0.0",
                Path.Combine(this.testDataDirectory, "AppxBundleTest"),
                Path.Combine(this.testDirectory, appxBundle));

            runner.Dispose();

            Assert.AreEqual(true, File.Exists(Path.Combine(this.testDirectory, "TestAppxBundle.AppxBundle")));
            Logger.LogMessage("Tested MakeAppx Runner.");
        }

        /// <summary>
        /// Method to test the ProcessRunner terminate process method
        /// </summary>
        [TestMethod]
        public void ProcessRunnerTest_TerminateProcess_Success()
        {
            Logger.LogMessage("Testing Process Runner -> Terminate Process.");
            string exePath = Path.Combine(this.testDataDirectory, "UtilsTestExe.exe");

            using (DesktopProcessRunner processRunner = new DesktopProcessRunner())
            {
                processRunner.ExePath = exePath;
                ThreadStart threadStart = new ThreadStart(() => processRunner.Run());

                // This will run when we call TerminateProcessIfRunning (after the thread is finished execution)
                threadStart += () =>
                {
                    Logger.LogMessage($"Process exit code: {processRunner.ExitCode}");

                    string standardOutput = string.Join(string.Empty, processRunner.StandardOutput);
                    Assert.IsFalse(standardOutput.Contains("Process Ending"), "Verifying process didn't print end message");
                };

                Thread thread = new Thread(threadStart);
                thread.Start();

                // Give some time for the process to start and validate output
                Thread.Sleep(2000);
                string stdOut = string.Join(string.Empty, processRunner.StandardOutput);
                Assert.IsTrue(stdOut.Contains("Process Starting"), "Verifying process start message");

                processRunner.TerminateProcessIfRunning();
                thread.Join();
            }
        }

        /// <summary>
        /// Method to test the ValidateExitCode method
        /// </summary>
        [TestMethod]
        public void ProcessRunnerTest_ValidateExitCode_Success()
        {
            Logger.LogMessage("Testing ProcessRunner ValidateExitCode");
            string exePath = Path.Combine(this.testDataDirectory, "UtilsTestExe.exe");

            using (DesktopProcessRunner processRunner = new DesktopProcessRunner())
            {
                processRunner.ExePath = exePath;
                ThreadStart threadStart = new ThreadStart(() => processRunner.Run());

                // This will run when we call TerminateProcessIfRunning (after the thread is finished execution)
                threadStart += () =>
                {
                    Logger.LogMessage($"Process exit code: {processRunner.ExitCode}");

                    // Process should have exited with code -1.
                    bool invalid = false;
                    try
                    {
                        processRunner.ValidateExitCode();
                    }
                    catch
                    {
                        invalid = true;
                    }

                    Assert.IsTrue(invalid, "Verifying exit code is invalid when we expect 0.");

                    // Make -1 a valid exit code.
                    invalid = false;
                    try
                    {
                        processRunner.ValidateExitCode(new int[] { -1 });
                    }
                    catch
                    {
                        invalid = true;
                    }

                    Assert.IsFalse(invalid, "Verifying exit code is valid when we specify -1 as a valid code.");

                    // Make -1 an invalid exit code with a list of valid codes.
                    invalid = false;
                    try
                    {
                        processRunner.ValidateExitCode(new int[] { 0 });
                    }
                    catch
                    {
                        invalid = true;
                    }

                    Assert.IsTrue(invalid, "Verifying exit code is invalid when we specify 0 as a valid code.");
                };

                Thread thread = new Thread(threadStart);
                thread.Start();

                // Give some time for the process to start and validate output
                Thread.Sleep(2000);
                string stdOut = string.Join(string.Empty, processRunner.StandardOutput);
                Assert.IsTrue(stdOut.Contains("Process Starting"), "Verifying process start message");

                processRunner.TerminateProcessIfRunning();
                thread.Join();
            }
        }

        /// <summary>
        /// Method to test getting the latest version subfolder when the subfolders are in Version format.
        /// </summary>
        [TestMethod]
        public void SDKDector_GetLastInstalledPlatformDirectory_ByVersion()
        {
            string rootPath = Path.Combine(this.testDataDirectory, "SDKDetectorByVersion");
            Directory.CreateDirectory(rootPath);

            string subDir1 = Path.Combine(rootPath, "1.0.9.1"); // newest by version
            string subDir2 = Path.Combine(rootPath, "0.10.6.1");
            string subDir3 = Path.Combine(rootPath, "0.0.1.3");

            // Create different subdirectories following the Version format.
            Directory.CreateDirectory(subDir1);
            Directory.CreateDirectory(subDir2);
            Directory.CreateDirectory(subDir3);

            SDKDetector sdkDetector = SDKDetector.Instance;

            string latestVersion = sdkDetector.GetLastInstalledPlatformDirectory(rootPath);
            Assert.AreEqual(subDir1, latestVersion);
        }

        /// <summary>
        /// Method to test getting the latest version subfolder when the subfolders are not in Version format.
        /// </summary>
        [TestMethod]
        public void SDKDector_GetLastInstalledPlatformDirectory_ByLatestWrite()
        {
            string rootPath = Path.Combine(this.testDataDirectory, "SDKDetectorByLatestWrite");
            Directory.CreateDirectory(rootPath);

            string subDir1 = Path.Combine(rootPath, "1.0.9.1");
            string subDir2 = Path.Combine(rootPath, "0.10.6.1-alpha");
            string subDir3 = Path.Combine(rootPath, "0.0.1.3"); // newest by time.

            // Create different subdirectories following the Version format.
            Directory.CreateDirectory(subDir1);
            Directory.CreateDirectory(subDir2);
            Directory.CreateDirectory(subDir3);

            SDKDetector sdkDetector = SDKDetector.Instance;

            string latestVersion = sdkDetector.GetLastInstalledPlatformDirectory(rootPath);
            Assert.AreEqual(subDir3, latestVersion);
        }
    }
}
