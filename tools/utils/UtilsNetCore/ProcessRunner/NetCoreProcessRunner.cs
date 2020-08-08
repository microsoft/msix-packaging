//-----------------------------------------------------------------------
// <copyright file="NetCoreProcessRunner.cs" company="Microsoft">
//     Copyright (c) Microsoft. All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

namespace Microsoft.Msix.Utils.ProcessRunner
{
    using System;
    using System.IO;
    using System.Runtime.InteropServices;
    using Microsoft.Msix.Utils.Logger;

    /// <summary>
    /// Implements a ProcessRunnerBase which runs on NetCore.
    /// Does not support redirection of standard output or standard error.
    /// </summary>
    public class NetCoreProcessRunner : ProcessRunnerBase
    {
        private Win32NativeMethods.PROCESS_INFORMATION procInfo;
        private Win32NativeMethods.STARTUPINFO startUpInfo;

        protected override void OnLaunchProcess()
        {
            this.procInfo = new Win32NativeMethods.PROCESS_INFORMATION();
            this.startUpInfo = new Win32NativeMethods.STARTUPINFO();

            Logger.Log(this.LogProviders, "\nExecuting: {0} {1}", this.ExePath, this.Arguments);

            // The space prefix before the arguments is necessary
            // Otherwise the call will fail
            bool result = Win32NativeMethods.CreateProcess(
                this.ExePath,
                " " + this.Arguments,
                IntPtr.Zero,
                IntPtr.Zero,
                false,
                Win32NativeMethods.CREATE_NEW_CONSOLE,
                IntPtr.Zero,
                Path.GetDirectoryName(this.ExePath),
                ref this.startUpInfo,
                out this.procInfo);

            if (!result)
            {
                int lastError = Marshal.GetLastWin32Error();
                Logger.Log(this.LogProviders, "ProcessRunner->OnLaunchProcess Failed to create process. Last Error = {0}.", lastError);
                throw new Win32InteropException("Could not create process.", lastError);
            }

            this.HasStarted = true;
        }

        protected override bool OnWaitForExitOrTimeout(int timeoutMilliseconds)
        {
            Logger.Log(this.LogProviders, "\nProcess Runner - Waiting for: {0} to finish...", this.ExePath);

            uint waitResult = Win32NativeMethods.WaitForSingleObject(
                this.procInfo.hProcessHandle,
                timeoutMilliseconds);
            int lastError = Marshal.GetLastWin32Error();
            if (waitResult == Win32NativeMethods.WAIT_FAILED)
            {
                Logger.Log(this.LogProviders, "ProcessRunner->OnWaitForExitOrTimeout process hasn't exited yet. Last Error = {0}.", lastError);
                throw new Win32InteropException("Could not wait on process.", lastError);
            }

            int exitCode = 0;
            bool result = Win32NativeMethods.GetExitCodeProcess(this.procInfo.hProcessHandle, out exitCode);
            lastError = Marshal.GetLastWin32Error();
            if (result)
            {
                if (exitCode == Win32NativeMethods.STILL_ACTIVE)
                {
                    // Process has overrun
                    return false;
                }
                else
                {
                    this.ExitCode = exitCode;
                    this.HasFinished = true;
                    Logger.Log(this.LogProviders, "Process Runner - Finished execution. Exit code = {0}\n", this.ExitCode);
                    return true;
                }
            }
            else
            {
                throw new Win32InteropException("Could not get exit code for process.", lastError);
            }
        }

        protected override void OnTerminateRunningProcess()
        {
            if (!Win32NativeMethods.TerminateProcess(this.procInfo.hProcessHandle, 0))
            {
                Logger.Log(
                    this.LogProviders,
                    "ProcessRunner->OnTerminateRunningProcess Error terminating process with identifier {0}.",
                    this.procInfo.hProcessHandle);
            }
        }
    }
}
