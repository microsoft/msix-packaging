//----------------------------------------------------------------------------------------------------------------------
// <copyright file="ProcessRunnerTests.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//----------------------------------------------------------------------------------------------------------------------

namespace UtilsTests
{
    using System;
    using System.IO;
    using System.Threading;
    using Microsoft.Packaging.Utils.ProcessRunner;
    using WEX.Logging.Interop;
    using WEX.TestExecution;
    using WEX.TestExecution.Markup;

    [TestClass]
    internal class ProcessRunnerTests : TestBase
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
            Log.Comment("Initializing ProcessRunner test.");

            // Copy the tool to the current directory for testing.
            this.testDirectory = TestContext.TestDeploymentDir;
            this.testDataDirectory = Environment.CurrentDirectory + "\\TestData\\";
            this.testToolDirectory = this.testDataDirectory + "MakeAppx\\";
            try
            {
                if (!File.Exists(this.testDirectory + toolName))
                {
                    File.Copy(this.testToolDirectory + toolName, this.testDirectory + toolName);
                }
            }
            catch (Exception exception)
            {
                Log.Comment("Error in Test Initialize." + exception.ToString());
            }
        }

        /// <summary>
        /// Test CleanUp.
        /// </summary>
        [TestCleanup]
        public new void TestCleanup()
        {
            TestHelper.DeleteFileIfExists(this.testDirectory + toolName);
            TestHelper.DeleteFileIfExists(this.testDirectory + appxBundle);
        }

        /// <summary>
        /// Method to test the SDKToolProcessRunner, SDKDetector and MakeAppxRunner.
        /// </summary>
        [TestMethod]
        public void SDKToolProcessRunnerTest_IntializeFromCurrentPath_Success()
        {
            Log.Comment("Testing SDKToolProcessRunner, SDKDetector, MakeAppxRunner");
            Log.Comment("MakeAppxRunner looks for the tool next to current assembly." +
                "If not found, looks for it in SDK Installation folder.");

            VerifyOperation verifyOperation = delegate { new MakeAppxRunner(); };
            Verify.NoThrow(verifyOperation);
            Log.Comment("MakeAppxRunner and SDKToolProcessRunner initialized successfully.");
        }

        /// <summary>
        /// Method to test the SDKToolProcessRunner, SDKDetector and MakeAppxRunner.
        /// Verifies that MakeAppxRunner throws an exception as the required tool is missing.
        /// </summary>
        [TestMethod]
        public void SDKToolProcessRunnerTest_InitializeFromToolPath_Failure()
        {
            // Delete the file from current path if it exists.
            if (File.Exists(this.testDirectory + toolName))
            {
                File.Delete(this.testDirectory + toolName);
                Log.Comment("File Deleted in the test directory");
            }

            // If the tool doesn't exist in the tool path and SDK is NOT installled,
            // Argument exception should be thrown.
            VerifyOperation verifyOperation = delegate { new MakeAppxRunner(); };
            Verify.Throws<ArgumentException>(verifyOperation);

            Log.Comment("MakeAppxRunner and SDKToolProcessRunner throw exception as expected.");
        }

        /// <summary>
        /// Method to test the SDKToolProcessRunner, SDKDetector and MakeAppxRunner
        /// by passing the tool directory.
        /// </summary>
        [TestMethod]
        public void SDKToolProcessRunnerTest_InitializeFromToolDirectory_GenerateAppxBundle_Success()
        {
            MakeAppxRunner runner;

            Log.Comment("Testing SDKToolProcessRunner, SDKDetector, MakeAppxRunner" +
                " by passing the Tool Directory.");

            VerifyOperation verifyOperation = delegate { runner = new MakeAppxRunner(this.testToolDirectory); };
            Verify.NoThrow(verifyOperation);

            runner = new MakeAppxRunner(this.testToolDirectory)
            {
                OverwriteExistingFiles = true
            };
            Log.Comment("MakeAppxRunner and SDKToolProcessRunner initialized successfully.");

            // Call GenerateAppxBundleFromRootFolder to test it.
            runner.GenerateAppxBundleFromRootFolder(
                "1.0.0.0",
                this.testDataDirectory + "AppxBundleTest",
                this.testDirectory + appxBundle);

            runner.Dispose();

            Verify.AreEqual(true, File.Exists(this.testDirectory + "TestAppxBundle.AppxBundle"));
            Log.Comment("Tested MakeAppx Runner.");
        }

        /// <summary>
        /// Method to test the ProcessRunner terminate process method
        /// </summary>
        [TestMethod]
        public void ProcessRunnerTest_TerminateProcess_Success()
        {
            Log.Comment("Testing Process Runner -> Terminate Process.");
            string exePath = Path.Combine(this.testDataDirectory, "UtilsTestExe.exe");

            using (DesktopProcessRunner processRunner = new DesktopProcessRunner())
            {
                processRunner.ExePath = exePath;
                ThreadStart threadStart = new ThreadStart(() => processRunner.Run());

                // This will run when we call TerminateProcessIfRunning (after the thread is finished execution)
                threadStart += () =>
                {
                    Log.Comment($"Process exit code: {processRunner.ExitCode}");

                    string standardOutput = string.Join(string.Empty, processRunner.StandardOutput);
                    Verify.IsFalse(standardOutput.Contains("Process Ending"), "Verifying process didn't print end message");
                };

                Thread thread = new Thread(threadStart);
                thread.Start();

                // Give some time for the process to start and validate output
                Thread.Sleep(2000);
                string stdOut = string.Join(string.Empty, processRunner.StandardOutput);
                Verify.IsTrue(stdOut.Contains("Process Starting"), "Verifying process start message");

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
            Log.Comment("Testing ProcessRunner ValidateExitCode");
            string exePath = Path.Combine(this.testDataDirectory, "UtilsTestExe.exe");

            using (DesktopProcessRunner processRunner = new DesktopProcessRunner())
            {
                processRunner.ExePath = exePath;
                ThreadStart threadStart = new ThreadStart(() => processRunner.Run());

                // This will run when we call TerminateProcessIfRunning (after the thread is finished execution)
                threadStart += () =>
                {
                    Log.Comment($"Process exit code: {processRunner.ExitCode}");

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

                    Verify.IsTrue(invalid, "Verifying exit code is invalid when we expect 0.");

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

                    Verify.IsFalse(invalid, "Verifying exit code is valid when we specify -1 as a valid code.");

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

                    Verify.IsTrue(invalid, "Verifying exit code is invalid when we specify 0 as a valid code.");
                };

                Thread thread = new Thread(threadStart);
                thread.Start();

                // Give some time for the process to start and validate output
                Thread.Sleep(2000);
                string stdOut = string.Join(string.Empty, processRunner.StandardOutput);
                Verify.IsTrue(stdOut.Contains("Process Starting"), "Verifying process start message");

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
            Verify.AreEqual(subDir1, latestVersion);
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
            Verify.AreEqual(subDir3, latestVersion);
        }
    }
}
