// -----------------------------------------------------------------------
//  <copyright file="AppxPackagingEventSinkToLogger.cs" company="Microsoft">
//      Copyright (c) Microsoft Corporation.  All rights reserved.
//  </copyright>
// -----------------------------------------------------------------------

namespace Microsoft.Msix.Utils.Logger
{
    using System.Collections.Generic;
    using System.Runtime.InteropServices;
    using Microsoft.Msix.Utils.AppxPackagingInterop;

    public class AppxPackagingEventSinkToLogger : IAppxPackagingDiagnosticEventSink
    {
        private bool disposedValue = false; // To detect redundant calls

        public void ReportContextChange(
            [In] APPX_PACKAGING_CONTEXT_CHANGE_TYPE changeType,
            [In] int contextId,
            [In, MarshalAs(UnmanagedType.LPStr)] string contextName,
            [In, MarshalAs(UnmanagedType.LPWStr)] string contextMessage,
            [In, MarshalAs(UnmanagedType.LPWStr)] string detailsMessage)
        {
            switch (changeType)
            {
                case APPX_PACKAGING_CONTEXT_CHANGE_TYPE.APPX_PACKAGING_CONTEXT_CHANGE_TYPE_START:
                    Logger.Debug(contextMessage);
                    break;

                case APPX_PACKAGING_CONTEXT_CHANGE_TYPE.APPX_PACKAGING_CONTEXT_CHANGE_TYPE_CHANGE:
                    Logger.Debug("  {0}", contextMessage);
                    break;

                case APPX_PACKAGING_CONTEXT_CHANGE_TYPE.APPX_PACKAGING_CONTEXT_CHANGE_TYPE_DETAILS:
                    Logger.Debug("    {0}", detailsMessage);
                    break;

                case APPX_PACKAGING_CONTEXT_CHANGE_TYPE.APPX_PACKAGING_CONTEXT_CHANGE_TYPE_END:
                    Logger.Debug(contextMessage);
                    break;
            }
        }

        public void ReportError([In, MarshalAs(UnmanagedType.LPWStr)] string errorMessage)
        {
            // Log only to the file
            LogProvider fileLogger = Logger.GetLogProvider(typeof(FileLog));

            if (fileLogger != null)
            {
                List<ILogProvider> providers = new List<ILogProvider>();
                providers.Add(fileLogger);
                IReadOnlyList<ILogProvider> readOnlyProviders = providers.AsReadOnly();
                Logger.Log(readOnlyProviders, Logger.LogLevels.All, "[Diagnostic Error Information]");
                Logger.Log(readOnlyProviders, Logger.LogLevels.All, errorMessage);
            }
        }

        #region IDisposable Support
        // This code added to correctly implement the disposable pattern.
        public void Dispose()
        {
            // Do not change this code. Put cleanup code in Dispose(bool disposing) below.
            this.Dispose(true);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (!this.disposedValue)
            {
                this.disposedValue = true;
            }
        }
        #endregion
    }
}
