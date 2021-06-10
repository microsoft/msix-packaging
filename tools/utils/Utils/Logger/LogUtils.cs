// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

namespace Microsoft.Msix.Utils.Logger
{
    using System;
    using System.IO;

    /// <summary>
    /// Log utils
    /// </summary>
    public static class LogUtils
    {
        /// <summary>
        /// Setup the logger for the tool by adding the console logger and a temp file logger
        /// </summary>
        /// <param name="toolName">Tool name is used as the top level directory path</param>
        /// <param name="copyrightYear">The year to display in the copyright message.</param>
        /// <param name="verbose">whether or not to set log level to verbose</param>
        public static void SetupLogger(
            string toolName,
            int copyrightYear,
            bool verbose = false)
        {
            SetupLogger(toolName, copyrightYear, string.Empty, verbose);
        }

        /// <summary>
        /// Setup the logger for the tool by adding the console logger and a temp file logger
        /// </summary>
        /// <param name="toolName">Tool name is used as the top level directory path</param>
        /// <param name="copyrightYear">The year to display in the copyright message.</param>
        /// <param name="parentDirectory">Parent directory. if empty temp path will be used</param>
        /// <param name="verbose">whether or not to set log level to verbose</param>
        public static void SetupLogger(
            string toolName,
            int copyrightYear,
            string parentDirectory,
            bool verbose = false)
        {
            SetupConsoleLogger();
            LogCopyright(toolName, copyrightYear);

            string logFolder = string.Empty;
            if (string.IsNullOrWhiteSpace(parentDirectory))
            {
                string topLevelName = string.Format(@"{0}\{1}", toolName, "Logs");
                logFolder = FileSystemUtils.CreateTempDirectory(topLevelName);
            }
            else
            {
                logFolder = FileSystemUtils.CreateUniqueDirectory(parentDirectory, "Logs");
            }

            string logFile = Path.Combine(logFolder, "Log.txt");
            SetupFileLogger(logFile);

            if (verbose)
            {
                SetVerbose();
            }
        }

        /// <summary>
        /// Logs the copyright info.
        /// </summary>
        /// <param name="toolName">The name of the tool.</param>
        /// <param name="copyrightYear">The year to display in the copyright message.</param>
        public static void LogCopyright(string toolName, int copyrightYear)
        {
            Logger.Info("Microsoft (R) " + toolName + " Tool");
            Logger.Info("Copyright (C) " + copyrightYear + " Microsoft.  All rights reserved.");
        }

        /// <summary>
        /// Setup console logging for the tool
        /// </summary>
        public static void SetupConsoleLogger()
        {
            Logger.LogDecorations decorations = Logger.LogDecorations.None;

            // Normal log level for the tool, printing everything but debug messages.
            Logger.LogLevels levels = Logger.LogLevels.All & ~Logger.LogLevels.Debug;
#if DEBUG
            decorations = Logger.LogDecorations.All;
            levels = Logger.LogLevels.All;
#endif

            ConsoleLog consoleLog = new ConsoleLog
            {
                LogLevels = levels,
                LogDecorations = decorations
            };

            AddProvider(consoleLog);
        }

        /// <summary>
        /// Set up logging for the tool.
        /// </summary>
        /// <param name="fileLogPath">The path to the desired log file</param>
        /// <param name="flushEachLine">A flag controlling whether to flush each line</param>
        public static void SetupFileLogger(string fileLogPath, bool flushEachLine = true)
        {
            Logger.LogDecorations decorations = Logger.LogDecorations.None;

            // Normal log level for the tool, printing everything but debug messages.
            Logger.LogLevels levels = Logger.LogLevels.All & ~Logger.LogLevels.Debug;
#if DEBUG
            decorations = Logger.LogDecorations.All;
            levels = Logger.LogLevels.All;
#endif

            FileLog fileLog = new FileLog
            {
                LogLevels = levels,
                LogDecorations = decorations,
                LogFilePath = fileLogPath
            };

            fileLog.FlushEachLine = flushEachLine;

            Logger.Info(
                "Log file is located under: {0}",
                fileLog.LogFilePath);
            Logger.Info();

            AddProvider(fileLog);
        }

        /// <summary>
        /// Sets the logging to verbose.
        /// </summary>
        public static void SetVerbose()
        {
            foreach (LogProvider provider in Logger.GetLogProviders())
            {
                provider.LogLevels = Logger.LogLevels.All;
            }
        }

        public static void RegisterTraceLogger()
        {
            TraceLog traceLog = new TraceLog();
            traceLog.LogLevels = Logger.LogLevels.All;
            traceLog.LogDecorations = Logger.LogDecorations.All;
            AddProvider(traceLog);
        }

        public static void RegisterConsoleLogger()
        {
            // Register Console Log
            ConsoleLog consoleLog = new ConsoleLog();
            consoleLog.LogLevels = Logger.LogLevels.All;
            consoleLog.LogDecorations = Logger.LogDecorations.All;
            AddProvider(consoleLog);
        }

        public static void AddProvider(LogProvider provider)
        {
            Logger.RegisterFormatter(new LogMessageFormatter());
            Logger.AddLogProvider(provider);
        }

        public static void RemoveProvider(LogProvider logProvider)
        {
            Logger.RemoveLogProvider(logProvider);
        }

        public static void RemoveProvider(Type providerType)
        {
            Logger.RemoveLogProvider(providerType);
        }

        /// <summary>
        /// Indents each line in the provided string.
        /// </summary>
        /// <param name="originalString">the string to be indented</param>
        /// <param name="indentSpaces">number of spaces to add to each line</param>
        /// <returns>the indented string</returns>
        public static string IndentLines(string originalString, int indentSpaces)
        {
            string tab = new string(' ', indentSpaces);

            return tab + originalString.Replace(
                Environment.NewLine,
                Environment.NewLine + tab);
        }
    }
}
