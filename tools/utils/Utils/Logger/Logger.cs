// -----------------------------------------------------------------------
//  <copyright file="Logger.cs" company="Microsoft">
//      Copyright (c) Microsoft Corporation.  All rights reserved.
//  </copyright>
// -----------------------------------------------------------------------
namespace Microsoft.Msix.Utils.Logger
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;

    /// <summary>
    /// The logger class. Used as singleton
    /// </summary>
    public static class Logger
    {
        #region Private members
        /// <summary>
        /// This is the way to lock a static function.
        /// Since the Logger class is static, only one instance will be created for the whole app.
        /// For threaded applications, we need to lock this to make it thread safe.
        /// lock(typeof(Logger)) --> this is bad for performance (locks the whole AppDomain).
        /// We could lock on type of this, but the first is bad for performance and the second bad for security,
        /// We create this object as private for its internal CS object created by the CLR for us.
        /// </summary>
        private static readonly object SyncObject = new object();

        /// <summary>
        /// The list of ILogProviders to maintain - Observer design pattern.
        /// </summary>
        private static List<ILogProvider> registeredLogProviders;

        /// <summary>
        /// Map of file paths and file log provider information.
        /// </summary>
        private static Dictionary<string, JobLogInfo> m_logJobsInfoMap = new Dictionary<string, JobLogInfo>();

        /// <summary>
        /// Log folder to store all file logs.
        /// </summary>
        private static string m_logFolder;
        #endregion

        #region Events
        /// <summary>
        /// Gets or sets the MessageLogged event handler. Used to subscribe to log events fired
        /// anytime a message is logged, except for logs made within the Logger.RunWithoutLogEvents
        /// lambda argument.
        /// </summary>
        public static event EventHandler<LogEventArgs> MessageLoggedEvent;
        #endregion

        /// <summary>
        /// Log decorations
        /// </summary>
        [Flags]
        public enum LogDecorations
        {
            /// <summary>
            /// No decoration
            /// </summary>
            None = 0x00,

            /// <summary>
            /// decorate with detailed info (file name, line number etc.)
            /// </summary>
            Detail = 0x01,

            /// <summary>
            /// decorate with time stamp
            /// </summary>
            Time = 0x02,

            /// <summary>
            /// decorate with log level
            /// </summary>
            LogLevel = 0x04,

            /// <summary>
            /// decorate with all of the above
            /// </summary>
            All = 0xFF,
        }

        /// <summary>
        /// Log levels, used to determine what types of messages to log
        /// levels can be for example: warning, error, information etc.
        /// </summary>
        [Flags]
        public enum LogLevels
        {
            /// <summary>
            /// no log level
            /// </summary>
            None = 0x00,

            /// <summary>
            /// log level is debug
            /// </summary>
            Debug = 0x01,

            /// <summary>
            /// log level is info
            /// </summary>
            Info = 0x02,

            /// <summary>
            /// log level is warning
            /// </summary>
            Warning = 0x04,

            /// <summary>
            /// log level is error
            /// </summary>
            Error = 0x08,

            /// <summary>
            /// log level is exception
            /// </summary>
            Exp = 0x10,

            /// <summary>
            /// log level is exception stack
            /// </summary>
            ExpStack = 0x20,

            /// <summary>
            /// log level is info but printed on a single line
            /// (no line return)
            /// </summary>
            InfoPrompt = 0x40,

            /// <summary>
            /// log level is critical
            /// </summary>
            Critical = 0x80,

            /// <summary>
            /// log level is all of the above
            /// </summary>
            All = 0xFF,
        }

        /// <summary>
        /// Flags that can be passed to logging methods
        /// </summary>
        public enum LogFlags
        {
            /// <summary>
            /// No flags
            /// </summary>
            None = 0x00,

            /// <summary>
            /// Used to disable firing log events, in case a subscriber is attached.
            /// This can be used if a consumer that subscribes to log events needs to log in its handler,
            /// preventing reentrancy.
            /// </summary>
            DisableLogEventsFiring = 0x01
        }

        #region Properties
        /// <summary>
        /// Gets or sets the value indicating Log folder.
        /// If the value is not a Rooted path, it will be considered as a relative path
        /// to the Temporary folder path.
        /// </summary>
        public static string LogFolder
        {
            get
            {
                if (m_logFolder == null)
                {
                    throw new NullReferenceException("Root Log Folder not set.");
                }

                return m_logFolder;
            }

            set
            {
                string basepath;
                if (!Path.IsPathRooted(value))
                {
                    basepath = Path.Combine(Path.GetTempPath(), value);

                    if (!Directory.Exists(basepath))
                    {
                        basepath = FileSystemUtils.CreateTempDirectory(value);
                    }
                }
                else
                {
                    basepath = value;
                }

                m_logFolder = Path.Combine(basepath, "Log" + System.DateTime.Now.ToString("MM_dd_yyyTHH_mm_ss"));
            }
        }

        /// <summary>
        /// Gets the registered Log message formatter
        /// </summary>
        public static ILogMessageFormatter Formatter { get; private set; }
        #endregion

        #region GetProvider(s)
        /// <summary>
        /// Get provider
        /// </summary>
        /// <param name="providerType">provider type</param>
        /// <returns>the provider</returns>
        public static LogProvider GetLogProvider(Type providerType)
        {
            lock (SyncObject)
            {
                if (registeredLogProviders != null)
                {
                    foreach (LogProvider logProvider in registeredLogProviders)
                    {
                        if (logProvider.GetType().Equals(providerType))
                        {
                            return logProvider;
                        }
                    }
                }
            }

            return null;
        }

        /// <summary>
        /// Get log providers
        /// </summary>
        /// <returns>all registered log providers</returns>
        public static IReadOnlyList<ILogProvider> GetLogProviders()
        {
            lock (SyncObject)
            {
                if (registeredLogProviders != null)
                {
                    return registeredLogProviders.AsReadOnly();
                }
            }

            return null;
        }
        #endregion

        #region Log with default provider
        /// <summary>
        /// Logs a message
        /// </summary>
        /// <param name="logLevel">log level</param>
        /// <param name="message">log message</param>
        /// <param name="messageArgs">message arguments</param>
        /// <param name="flags">The log flags</param>
        public static void Log(LogLevels logLevel, string message, IMessageArg[] messageArgs, LogFlags flags = LogFlags.None)
        {
            DoLog(registeredLogProviders, logLevel, message, messageArgs, flags);
        }

        /// <summary>
        /// Logs a message
        /// </summary>
        /// <param name="logLevel">log level</param>
        /// <param name="message">log message</param>
        /// <param name="messageArg">message argument</param>
        /// <param name="flags">The log flags</param>
        public static void Log(LogLevels logLevel, string message, IMessageArg messageArg, LogFlags flags = LogFlags.None)
        {
            DoLog(registeredLogProviders, logLevel, message, new[] { messageArg }, flags);
        }

        /// <summary>
        ///  Logs the text representation of the specified array of objects using
        ///  the specified format information.
        /// </summary>
        /// <param name="logLevel">log level</param>
        /// <param name="format">A composite format string.</param>
        /// <param name="flags">The log flags</param>
        /// <param name="args">An array of objects to write using format.</param>
        public static void Log(LogLevels logLevel, string format, LogFlags flags, params object[] args)
        {
            DoLog(registeredLogProviders, logLevel, string.Format(CultureInfo.InvariantCulture, format, args), null, flags);
        }

        /// <summary>
        ///  Logs the text representation of the specified array of objects using
        ///  the specified format information.
        /// </summary>
        /// <param name="logLevel">log level</param>
        /// <param name="format">A composite format string.</param>
        /// <param name="args">An array of objects to write using format.</param>
        public static void Log(LogLevels logLevel, string format, params object[] args)
        {
            DoLog(registeredLogProviders, logLevel, string.Format(CultureInfo.InvariantCulture, format, args), null, LogFlags.None);
        }

        /// <summary>
        /// Logs a message
        /// </summary>
        /// <param name="logLevel">log level</param>
        /// <param name="message">log message</param>
        /// <param name="flags">The log flags</param>
        public static void Log(LogLevels logLevel, string message, LogFlags flags = LogFlags.None)
        {
            DoLog(registeredLogProviders, logLevel, message, null, flags);
        }

        /// <summary>
        /// Logs a blank message
        /// </summary>
        /// <param name="logLevel">log level</param>
        /// <param name="flags">The log flags</param>
        public static void Log(LogLevels logLevel, LogFlags flags = LogFlags.None)
        {
            DoLog(registeredLogProviders, logLevel, string.Empty, null, flags);
        }

        /// <summary>
        /// Logs a message
        /// </summary>
        /// <param name="message">log message</param>
        /// <param name="messageArgs">message arguments</param>
        /// <param name="flags">The log flags</param>
        public static void Log(string message, IMessageArg[] messageArgs, LogFlags flags = LogFlags.None)
        {
            DoLog(registeredLogProviders, LogLevels.Debug, message, messageArgs, flags);
        }

        /// <summary>
        /// Logs a message
        /// </summary>
        /// <param name="message">log message</param>
        /// <param name="messageArgs">message argument</param>
        /// <param name="flags">The log flags</param>
        public static void Log(string message, IMessageArg messageArgs, LogFlags flags = LogFlags.None)
        {
            DoLog(registeredLogProviders, LogLevels.Debug, message, new[] { messageArgs }, flags);
        }

        /// <summary>
        /// Logs a message
        /// </summary>
        /// <param name="message">log message</param>
        /// <param name="flags">The log flags</param>
        public static void Log(string message, LogFlags flags = LogFlags.None)
        {
            DoLog(registeredLogProviders, LogLevels.Debug, message, null, flags);
        }

        /// <summary>
        ///  Logs the text representation of the specified array of objects using
        ///  the specified format information.
        /// </summary>
        /// <param name="format">A composite format string.</param>
        /// <param name="flags">The log flags</param>
        /// <param name="args">An array of objects to write using format.</param>
        public static void Log(string format, LogFlags flags, params object[] args)
        {
            DoLog(registeredLogProviders, LogLevels.Debug, string.Format(CultureInfo.InvariantCulture, format, args), null, flags);
        }

        /// <summary>
        ///  Logs the text representation of the specified array of objects using
        ///  the specified format information.
        /// </summary>
        /// <param name="format">A composite format string.</param>
        /// <param name="args">An array of objects to write using format.</param>
        public static void Log(string format, params object[] args)
        {
            DoLog(registeredLogProviders, LogLevels.Debug, string.Format(CultureInfo.InvariantCulture, format, args), null, LogFlags.None);
        }

        /// <summary>
        /// Logs an exception
        /// </summary>
        /// <param name="exp">exception to log</param>
        /// <param name="flags">The log flags</param>
        public static void Log(Exception exp, LogFlags flags = LogFlags.None)
        {
            if (exp == null)
            {
                return;
            }

            string message = Formatter.GetExceptionMessage(exp);

            DoLog(registeredLogProviders, LogLevels.Exp, message, new IMessageArg[] { new ExpMessageArg(exp) }, flags);
        }

        /// <summary>
        /// Logs an exception for a specified log level
        /// </summary>
        /// <param name="logLevel">log level</param>
        /// <param name="exp">exception to log</param>
        /// <param name="flags">The log flags</param>
        public static void Log(LogLevels logLevel, Exception exp, LogFlags flags = LogFlags.None)
        {
            DoLog(registeredLogProviders, logLevel, string.Empty, new IMessageArg[] { new ExpMessageArg(exp) }, flags);
        }

        /// <summary>
        /// Stack debugging style.
        /// </summary>
        /// <param name="flags">The log flags</param>
        public static void Log(LogFlags flags = LogFlags.None)
        {
            DoLog(registeredLogProviders, LogLevels.Debug, string.Empty, null, flags);
        }
        #endregion

        #region Log with custom provider
        /// <summary>
        /// Logs a message
        /// </summary>
        /// <param name="logProviders">Log providers this message should be logged to</param>
        /// <param name="logLevel">log level</param>
        /// <param name="message">log message</param>
        /// <param name="messageArgs">message arguments</param>
        /// <param name="flags">The log flags</param>
        public static void Log(IReadOnlyList<ILogProvider> logProviders, LogLevels logLevel, string message, IMessageArg[] messageArgs, LogFlags flags = LogFlags.None)
        {
            DoLog(logProviders, logLevel, message, messageArgs, flags);
        }

        /// <summary>
        /// Logs a message
        /// </summary>
        /// <param name="logProviders">Log providers this message should be logged to</param>
        /// <param name="logLevel">log level</param>
        /// <param name="message">log message</param>
        /// <param name="messageArg">message argument</param>
        /// <param name="flags">The log flags</param>
        public static void Log(IReadOnlyList<ILogProvider> logProviders, LogLevels logLevel, string message, IMessageArg messageArg, LogFlags flags = LogFlags.None)
        {
            DoLog(logProviders, logLevel, message, new[] { messageArg }, flags);
        }

        /// <summary>
        ///  Logs the text representation of the specified array of objects using
        ///  the specified format information.
        /// </summary>
        /// <param name="logProviders">Log providers this message should be logged to</param>
        /// <param name="logLevel">log level</param>
        /// <param name="format">A composite format string.</param>
        /// <param name="flags">The log flags</param>
        /// <param name="args">An array of objects to write using format.</param>
        public static void Log(IReadOnlyList<ILogProvider> logProviders, LogLevels logLevel, string format, LogFlags flags, params object[] args)
        {
            DoLog(logProviders, logLevel, string.Format(CultureInfo.InvariantCulture, format, args), null, flags);
        }

        /// <summary>
        ///  Logs the text representation of the specified array of objects using
        ///  the specified format information.
        /// </summary>
        /// <param name="logProviders">Log providers this message should be logged to</param>
        /// <param name="logLevel">log level</param>
        /// <param name="format">A composite format string.</param>
        /// <param name="args">An array of objects to write using format.</param>
        public static void Log(IReadOnlyList<ILogProvider> logProviders, LogLevels logLevel, string format, params object[] args)
        {
            DoLog(logProviders, logLevel, string.Format(CultureInfo.InvariantCulture, format, args), null, LogFlags.None);
        }

        /// <summary>
        /// Logs a message
        /// </summary>
        /// <param name="logProviders">Log providers this message should be logged to</param>
        /// <param name="logLevel">log level</param>
        /// <param name="message">log message</param>
        /// <param name="flags">The log flags</param>
        public static void Log(IReadOnlyList<ILogProvider> logProviders, LogLevels logLevel, string message, LogFlags flags = LogFlags.None)
        {
            DoLog(logProviders, logLevel, message, null, flags);
        }

        /// <summary>
        /// Logs a blank message
        /// </summary>
        /// <param name="logProviders">Log providers this message should be logged to</param>
        /// <param name="logLevel">log level</param>
        /// <param name="flags">The log flags</param>
        public static void Log(IReadOnlyList<ILogProvider> logProviders, LogLevels logLevel, LogFlags flags = LogFlags.None)
        {
            DoLog(logProviders, logLevel, string.Empty, null, flags);
        }

        /// <summary>
        /// Logs a message
        /// </summary>
        /// <param name="logProviders">Log providers this message should be logged to</param>
        /// <param name="message">log message</param>
        /// <param name="messageArgs">message arguments</param>
        /// <param name="flags">The log flags</param>
        public static void Log(IReadOnlyList<ILogProvider> logProviders, string message, IMessageArg[] messageArgs, LogFlags flags = LogFlags.None)
        {
            DoLog(logProviders, LogLevels.Debug, message, messageArgs, flags);
        }

        /// <summary>
        /// Logs a message
        /// </summary>
        /// <param name="logProviders">Log providers this message should be logged to</param>
        /// <param name="message">log message</param>
        /// <param name="messageArgs">message argument</param>
        /// <param name="flags">The log flags</param>
        public static void Log(IReadOnlyList<ILogProvider> logProviders, string message, IMessageArg messageArgs, LogFlags flags = LogFlags.None)
        {
            DoLog(logProviders, LogLevels.Debug, message, new[] { messageArgs }, flags);
        }

        /// <summary>
        /// Logs a message
        /// </summary>
        /// <param name="logProviders">Log providers this message should be logged to</param>
        /// <param name="message">log message</param>
        /// <param name="flags">The log flags</param>
        public static void Log(IReadOnlyList<ILogProvider> logProviders, string message, LogFlags flags = LogFlags.None)
        {
            DoLog(logProviders, LogLevels.Debug, message, null, flags);
        }

        /// <summary>
        ///  Logs the text representation of the specified array of objects using
        ///  the specified format information.
        /// </summary>
        /// <param name="logProviders">Log providers this message should be logged to</param>
        /// <param name="format">A composite format string.</param>
        /// <param name="flags">The log flags</param>
        /// <param name="args">An array of objects to write using format.</param>
        public static void Log(IReadOnlyList<ILogProvider> logProviders, string format, LogFlags flags, params object[] args)
        {
            DoLog(logProviders, LogLevels.Debug, string.Format(CultureInfo.InvariantCulture, format, args), null, flags);
        }

        /// <summary>
        ///  Logs the text representation of the specified array of objects using
        ///  the specified format information.
        /// </summary>
        /// <param name="logProviders">Log providers this message should be logged to</param>
        /// <param name="format">A composite format string.</param>
        /// <param name="args">An array of objects to write using format.</param>
        public static void Log(IReadOnlyList<ILogProvider> logProviders, string format, params object[] args)
        {
            DoLog(logProviders, LogLevels.Debug, string.Format(CultureInfo.InvariantCulture, format, args), null, LogFlags.None);
        }

        /// <summary>
        /// Logs an exception
        /// </summary>
        /// <param name="logProviders">Log providers this message should be logged to</param>
        /// <param name="exp">exception to log</param>
        /// <param name="flags">The log flags</param>
        public static void Log(IReadOnlyList<ILogProvider> logProviders, Exception exp, LogFlags flags = LogFlags.None)
        {
            if (exp == null)
            {
                return;
            }

            string message = Formatter.GetExceptionMessage(exp);

            DoLog(logProviders, LogLevels.Exp, message, new IMessageArg[] { new ExpMessageArg(exp) }, flags);
        }

        /// <summary>
        /// Logs an exception for a specified log level
        /// </summary>
        /// <param name="logProviders">Log providers this message should be logged to</param>
        /// <param name="logLevel">log level</param>
        /// <param name="exp">exception to log</param>
        /// <param name="flags">The log flags</param>
        public static void Log(IReadOnlyList<ILogProvider> logProviders, LogLevels logLevel, Exception exp, LogFlags flags = LogFlags.None)
        {
            DoLog(logProviders, logLevel, string.Empty, new IMessageArg[] { new ExpMessageArg(exp) }, flags);
        }

        /// <summary>
        /// Stack debugging style.
        /// </summary>
        /// <param name="logProviders">Log providers this message should be logged to</param>
        /// <param name="flags">The log flags</param>
        public static void Log(IReadOnlyList<ILogProvider> logProviders, LogFlags flags = LogFlags.None)
        {
            DoLog(logProviders, LogLevels.Debug, string.Empty, null, flags);
        }
        #endregion

        #region Log wrapper methods with default provider
        /// <summary>
        /// Logs a message at the Debug level.
        /// </summary>
        /// <param name="message">Message to be logged.</param>
        /// <param name="flags">The log flags</param>
        public static void Debug(string message, LogFlags flags = LogFlags.None)
        {
            Log(LogLevels.Debug, message, flags);
        }

        /// <summary>
        /// Logs a formatted message at the Debug level.
        /// </summary>
        /// <param name="messageFormat">A format string for the message to log.</param>
        /// <param name="flags">The log flags</param>
        /// <param name="args">An object array that contains the objects to format.</param>
        public static void Debug(string messageFormat, LogFlags flags, params object[] args)
        {
            Log(LogLevels.Debug, string.Format(messageFormat, args), flags);
        }

        /// <summary>
        /// Logs a formatted message at the Debug level.
        /// </summary>
        /// <param name="messageFormat">A format string for the message to log.</param>
        /// <param name="args">An object array that contains the objects to format.</param>
        public static void Debug(string messageFormat, params object[] args)
        {
            Log(LogLevels.Debug, string.Format(messageFormat, args), LogFlags.None);
        }

        /// <summary>
        /// Logs a blank line at the Debug level.
        /// </summary>
        /// <param name="flags">The log flags</param>
        public static void Debug(LogFlags flags = LogFlags.None)
        {
            Log(LogLevels.Debug, flags);
        }

        /// <summary>
        /// Logs a message at the Info level.
        /// </summary>
        /// <param name="message">Message to be logged.</param>
        /// <param name="flags">The log flags</param>
        public static void Info(string message, LogFlags flags = LogFlags.None)
        {
            Log(LogLevels.Info, message, flags);
        }

        /// <summary>
        /// Logs a formatted message at the Info level.
        /// </summary>
        /// <param name="messageFormat">A format string for the message to log.</param>
        /// <param name="flags">The log flags</param>
        /// <param name="args">An object array that contains the objects to format.</param>
        public static void Info(string messageFormat, LogFlags flags, params object[] args)
        {
            Log(LogLevels.Info, string.Format(messageFormat, args), flags);
        }

        /// <summary>
        /// Logs a formatted message at the Info level.
        /// </summary>
        /// <param name="messageFormat">A format string for the message to log.</param>
        /// <param name="args">An object array that contains the objects to format.</param>
        public static void Info(string messageFormat, params object[] args)
        {
            Log(LogLevels.Info, string.Format(messageFormat, args), LogFlags.None);
        }

        /// <summary>
        /// Logs a message at the Info level without returning to a new line.
        /// </summary>
        /// <param name="message">Message to be logged.</param>
        /// <param name="flags">The log flags</param>
        public static void InfoPrompt(string message, LogFlags flags = LogFlags.None)
        {
            Log(LogLevels.InfoPrompt, message, flags);
        }

        /// <summary>
        /// Logs a formatted message at the Info level without returning to a new line.
        /// </summary>
        /// <param name="messageFormat">A format string for the message to log.</param>
        /// <param name="flags">The log flags</param>
        /// <param name="args">An object array that contains the objects to format.</param>
        public static void InfoPrompt(string messageFormat, LogFlags flags, params object[] args)
        {
            Log(LogLevels.InfoPrompt, string.Format(messageFormat, args), flags);
        }

        /// <summary>
        /// Logs a formatted message at the Info level without returning to a new line.
        /// </summary>
        /// <param name="messageFormat">A format string for the message to log.</param>
        /// <param name="args">An object array that contains the objects to format.</param>
        public static void InfoPrompt(string messageFormat, params object[] args)
        {
            Log(LogLevels.InfoPrompt, string.Format(messageFormat, args), LogFlags.None);
        }

        /// <summary>
        /// Logs a blank line at the Info level.
        /// </summary>
        /// <param name="flags">The log flags</param>
        public static void Info(LogFlags flags = LogFlags.None)
        {
            Log(LogLevels.Info, flags);
        }

        /// <summary>
        /// Logs a message at the Warning level.
        /// </summary>
        /// <param name="message">Message to be logged.</param>
        /// <param name="flags">The log flags</param>
        public static void Warning(string message, LogFlags flags = LogFlags.None)
        {
            Log(LogLevels.Warning, message, flags);
        }

        /// <summary>
        /// Logs a formatted message at the Warning level.
        /// </summary>
        /// <param name="messageFormat">A format string for the message to log.</param>
        /// <param name="flags">The log flags</param>
        /// <param name="args">An object array that contains the objects to format.</param>
        public static void Warning(string messageFormat, LogFlags flags, params object[] args)
        {
            Log(LogLevels.Warning, string.Format(messageFormat, args), flags);
        }

        /// <summary>
        /// Logs a formatted message at the Warning level.
        /// </summary>
        /// <param name="messageFormat">A format string for the message to log.</param>
        /// <param name="args">An object array that contains the objects to format.</param>
        public static void Warning(string messageFormat, params object[] args)
        {
            Log(LogLevels.Warning, string.Format(messageFormat, args), LogFlags.None);
        }

        /// <summary>
        /// Logs a blank line at the Warning level.
        /// </summary>
        /// <param name="flags">The log flags</param>
        public static void Warning(LogFlags flags = LogFlags.None)
        {
            Log(LogLevels.Warning, flags);
        }

        /// <summary>
        /// Logs a message at the Error level.
        /// </summary>
        /// <param name="message">Message to be logged.</param>
        /// <param name="flags">The log flags</param>
        public static void Error(string message, LogFlags flags = LogFlags.None)
        {
            Log(LogLevels.Error, message, flags);
        }

        /// <summary>
        /// Logs a formatted message at the Error level.
        /// </summary>
        /// <param name="messageFormat">A format string for the message to log.</param>
        /// <param name="flags">The log flags</param>
        /// <param name="args">An object array that contains the objects to format.</param>
        public static void Error(string messageFormat, LogFlags flags, params object[] args)
        {
            Log(LogLevels.Error, string.Format(messageFormat, args), flags);
        }

        /// <summary>
        /// Logs a formatted message at the Error level.
        /// </summary>
        /// <param name="messageFormat">A format string for the message to log.</param>
        /// <param name="args">An object array that contains the objects to format.</param>
        public static void Error(string messageFormat, params object[] args)
        {
            Log(LogLevels.Error, string.Format(messageFormat, args), LogFlags.None);
        }

        /// <summary>
        /// Logs a blank line at the Error level.
        /// </summary>
        /// <param name="flags">The log flags</param>
        public static void Error(LogFlags flags = LogFlags.None)
        {
            Log(LogLevels.Error, flags);
        }

        /// <summary>
        /// Logs a message at the Critical level.
        /// </summary>
        /// <param name="message">Message to be logged.</param>
        /// <param name="flags">The log flags</param>
        public static void Critical(string message, LogFlags flags = LogFlags.None)
        {
            Log(LogLevels.Critical, message, flags);
        }

        /// <summary>
        /// Logs a formatted message at the Critical level.
        /// </summary>
        /// <param name="messageFormat">A format string for the message to log.</param>
        /// <param name="flags">The log flags</param>
        /// <param name="args">An object array that contains the objects to format.</param>
        public static void Critical(string messageFormat, LogFlags flags, params object[] args)
        {
            Log(LogLevels.Critical, string.Format(messageFormat, args), flags);
        }

        /// <summary>
        /// Logs a formatted message at the Critical level.
        /// </summary>
        /// <param name="messageFormat">A format string for the message to log.</param>
        /// <param name="args">An object array that contains the objects to format.</param>
        public static void Critical(string messageFormat, params object[] args)
        {
            Log(LogLevels.Critical, string.Format(messageFormat, args), LogFlags.None);
        }

        /// <summary>
        /// Logs a blank line at the Critical level.
        /// </summary>
        /// <param name="flags">The log flags</param>
        public static void Critical(LogFlags flags = LogFlags.None)
        {
            Log(LogLevels.Critical, flags);
        }
        #endregion

        #region Log wrapper methods with custom provider
        /// <summary>
        /// Logs a message at the Debug level.
        /// </summary>
        /// <param name="logProviders">Log providers this message should be logged to</param>
        /// <param name="message">Message to be logged.</param>
        /// <param name="flags">The log flags</param>
        public static void Debug(IReadOnlyList<ILogProvider> logProviders, string message, LogFlags flags = LogFlags.None)
        {
            Log(logProviders, LogLevels.Debug, message, flags);
        }

        /// <summary>
        /// Logs a formatted message at the Debug level.
        /// </summary>
        /// <param name="logProviders">Log providers this message should be logged to</param>
        /// <param name="messageFormat">A format string for the message to log.</param>
        /// <param name="flags">The log flags</param>
        /// <param name="args">An object array that contains the objects to format.</param>
        public static void Debug(IReadOnlyList<ILogProvider> logProviders, string messageFormat, LogFlags flags, params object[] args)
        {
            Log(logProviders, LogLevels.Debug, string.Format(messageFormat, args), flags);
        }

        /// <summary>
        /// Logs a formatted message at the Debug level.
        /// </summary>
        /// <param name="logProviders">Log providers this message should be logged to</param>
        /// <param name="messageFormat">A format string for the message to log.</param>
        /// <param name="args">An object array that contains the objects to format.</param>
        public static void Debug(IReadOnlyList<ILogProvider> logProviders, string messageFormat, params object[] args)
        {
            Log(logProviders, LogLevels.Debug, string.Format(messageFormat, args), LogFlags.None);
        }

        /// <summary>
        /// Logs a blank line at the Debug level.
        /// </summary>
        /// <param name="logProviders">Log providers this message should be logged to</param>
        /// <param name="flags">The log flags</param>
        public static void Debug(IReadOnlyList<ILogProvider> logProviders, LogFlags flags = LogFlags.None)
        {
            Log(logProviders, LogLevels.Debug, flags);
        }

        /// <summary>
        /// Logs a message at the Info level.
        /// </summary>
        /// <param name="logProviders">Log providers this message should be logged to</param>
        /// <param name="message">Message to be logged.</param>
        /// <param name="flags">The log flags</param>
        public static void Info(IReadOnlyList<ILogProvider> logProviders, string message, LogFlags flags = LogFlags.None)
        {
            Log(logProviders, LogLevels.Info, message, flags);
        }

        /// <summary>
        /// Logs a formatted message at the Info level.
        /// </summary>
        /// <param name="logProviders">Log providers this message should be logged to</param>
        /// <param name="messageFormat">A format string for the message to log.</param>
        /// <param name="flags">The log flags</param>
        /// <param name="args">An object array that contains the objects to format.</param>
        public static void Info(IReadOnlyList<ILogProvider> logProviders, string messageFormat, LogFlags flags, params object[] args)
        {
            Log(logProviders, LogLevels.Info, string.Format(messageFormat, args), flags);
        }

        /// <summary>
        /// Logs a formatted message at the Info level.
        /// </summary>
        /// <param name="logProviders">Log providers this message should be logged to</param>
        /// <param name="messageFormat">A format string for the message to log.</param>
        /// <param name="args">An object array that contains the objects to format.</param>
        public static void Info(IReadOnlyList<ILogProvider> logProviders, string messageFormat, params object[] args)
        {
            Log(logProviders, LogLevels.Info, string.Format(messageFormat, args), LogFlags.None);
        }

        /// <summary>
        /// Logs a message at the Info level without returning to a new line.
        /// </summary>
        /// <param name="logProviders">Log providers this message should be logged to</param>
        /// <param name="message">Message to be logged.</param>
        /// <param name="flags">The log flags</param>
        public static void InfoPrompt(IReadOnlyList<ILogProvider> logProviders, string message, LogFlags flags = LogFlags.None)
        {
            Log(logProviders, LogLevels.InfoPrompt, message, flags);
        }

        /// <summary>
        /// Logs a formatted message at the Info level without returning to a new line.
        /// </summary>
        /// <param name="logProviders">Log providers this message should be logged to</param>
        /// <param name="messageFormat">A format string for the message to log.</param>
        /// <param name="flags">The log flags</param>
        /// <param name="args">An object array that contains the objects to format.</param>
        public static void InfoPrompt(IReadOnlyList<ILogProvider> logProviders, string messageFormat, LogFlags flags, params object[] args)
        {
            Log(logProviders, LogLevels.InfoPrompt, string.Format(messageFormat, args), flags);
        }

        /// <summary>
        /// Logs a formatted message at the Info level without returning to a new line.
        /// </summary>
        /// <param name="logProviders">Log providers this message should be logged to</param>
        /// <param name="messageFormat">A format string for the message to log.</param>
        /// <param name="args">An object array that contains the objects to format.</param>
        public static void InfoPrompt(IReadOnlyList<ILogProvider> logProviders, string messageFormat, params object[] args)
        {
            Log(logProviders, LogLevels.InfoPrompt, string.Format(messageFormat, args), LogFlags.None);
        }

        /// <summary>
        /// Logs a blank line at the Info level.
        /// </summary>
        /// <param name="logProviders">Log providers this message should be logged to</param>
        /// <param name="flags">The log flags</param>
        public static void Info(IReadOnlyList<ILogProvider> logProviders, LogFlags flags = LogFlags.None)
        {
            Log(logProviders, LogLevels.Info, flags);
        }

        /// <summary>
        /// Logs a message at the Warning level.
        /// </summary>
        /// <param name="logProviders">Log providers this message should be logged to</param>
        /// <param name="message">Message to be logged.</param>
        /// <param name="flags">The log flags</param>
        public static void Warning(IReadOnlyList<ILogProvider> logProviders, string message, LogFlags flags = LogFlags.None)
        {
            Log(logProviders, LogLevels.Warning, message, flags);
        }

        /// <summary>
        /// Logs a formatted message at the Warning level.
        /// </summary>
        /// <param name="logProviders">Log providers this message should be logged to</param>
        /// <param name="messageFormat">A format string for the message to log.</param>
        /// <param name="flags">The log flags</param>
        /// <param name="args">An object array that contains the objects to format.</param>
        public static void Warning(IReadOnlyList<ILogProvider> logProviders, string messageFormat, LogFlags flags, params object[] args)
        {
            Log(logProviders, LogLevels.Warning, string.Format(messageFormat, args), flags);
        }

        /// <summary>
        /// Logs a formatted message at the Warning level.
        /// </summary>
        /// <param name="logProviders">Log providers this message should be logged to</param>
        /// <param name="messageFormat">A format string for the message to log.</param>
        /// <param name="args">An object array that contains the objects to format.</param>
        public static void Warning(IReadOnlyList<ILogProvider> logProviders, string messageFormat, params object[] args)
        {
            Log(logProviders, LogLevels.Warning, string.Format(messageFormat, args), LogFlags.None);
        }

        /// <summary>
        /// Logs a blank line at the Warning level.
        /// </summary>
        /// <param name="logProviders">Log providers this message should be logged to</param>
        /// <param name="flags">The log flags</param>
        public static void Warning(IReadOnlyList<ILogProvider> logProviders, LogFlags flags = LogFlags.None)
        {
            Log(logProviders, LogLevels.Warning, flags);
        }

        /// <summary>
        /// Logs a message at the Error level.
        /// </summary>
        /// <param name="logProviders">Log providers this message should be logged to</param>
        /// <param name="message">Message to be logged.</param>
        /// <param name="flags">The log flags</param>
        public static void Error(IReadOnlyList<ILogProvider> logProviders, string message, LogFlags flags = LogFlags.None)
        {
            Log(logProviders, LogLevels.Error, message, flags);
        }

        /// <summary>
        /// Logs a formatted message at the Error level.
        /// </summary>
        /// <param name="logProviders">Log providers this message should be logged to</param>
        /// <param name="messageFormat">A format string for the message to log.</param>
        /// <param name="flags">The log flags</param>
        /// <param name="args">An object array that contains the objects to format.</param>
        public static void Error(IReadOnlyList<ILogProvider> logProviders, string messageFormat, LogFlags flags, params object[] args)
        {
            Log(logProviders, LogLevels.Error, string.Format(messageFormat, args), flags);
        }

        /// <summary>
        /// Logs a formatted message at the Error level.
        /// </summary>
        /// <param name="logProviders">Log providers this message should be logged to</param>
        /// <param name="messageFormat">A format string for the message to log.</param>
        /// <param name="args">An object array that contains the objects to format.</param>
        public static void Error(IReadOnlyList<ILogProvider> logProviders, string messageFormat, params object[] args)
        {
            Log(logProviders, LogLevels.Error, string.Format(messageFormat, args), LogFlags.None);
        }

        /// <summary>
        /// Logs a blank line at the Error level.
        /// </summary>
        /// <param name="logProviders">Log providers this message should be logged to</param>
        /// <param name="flags">The log flags</param>
        public static void Error(IReadOnlyList<ILogProvider> logProviders, LogFlags flags = LogFlags.None)
        {
            Log(logProviders, LogLevels.Error, flags);
        }

        /// <summary>
        /// Logs a message at the Critical level.
        /// </summary>
        /// <param name="logProviders">Log providers this message should be logged to</param>
        /// <param name="message">Message to be logged.</param>
        /// <param name="flags">The log flags</param>
        public static void Critical(IReadOnlyList<ILogProvider> logProviders, string message, LogFlags flags = LogFlags.None)
        {
            Log(logProviders, LogLevels.Critical, message, flags);
        }

        /// <summary>
        /// Logs a formatted message at the Critical level.
        /// </summary>
        /// <param name="logProviders">Log providers this message should be logged to</param>
        /// <param name="messageFormat">A format string for the message to log.</param>
        /// <param name="flags">The log flags</param>
        /// <param name="args">An object array that contains the objects to format.</param>
        public static void Critical(IReadOnlyList<ILogProvider> logProviders, string messageFormat, LogFlags flags, params object[] args)
        {
            Log(logProviders, LogLevels.Critical, string.Format(messageFormat, args), flags);
        }

        /// <summary>
        /// Logs a formatted message at the Critical level.
        /// </summary>
        /// <param name="logProviders">Log providers this message should be logged to</param>
        /// <param name="messageFormat">A format string for the message to log.</param>
        /// <param name="args">An object array that contains the objects to format.</param>
        public static void Critical(IReadOnlyList<ILogProvider> logProviders, string messageFormat, params object[] args)
        {
            Log(logProviders, LogLevels.Critical, string.Format(messageFormat, args), LogFlags.None);
        }

        /// <summary>
        /// Logs a blank line at the Critical level.
        /// </summary>
        /// <param name="logProviders">Log providers this message should be logged to</param>
        /// <param name="flags">The log flags</param>
        public static void Critical(IReadOnlyList<ILogProvider> logProviders, LogFlags flags = LogFlags.None)
        {
            Log(logProviders, LogLevels.Critical, flags);
        }
        #endregion

        #region RemoveProvider(s)
        /// <summary>
        /// Remove provider from list
        /// </summary>
        /// <param name="logProvider">provider to remove</param>
        public static void RemoveLogProvider(LogProvider logProvider)
        {
            if (logProvider == null)
            {
                throw new ArgumentNullException("logProvider");
            }

            lock (SyncObject)
            {
                if (registeredLogProviders == null)
                {
                    return;
                }

                registeredLogProviders.Remove(logProvider);
                logProvider.DeinitLog();
            }
        }

        /// <summary>
        /// Remove provider from list based on provider type
        /// </summary>
        /// <param name="providerType">provider type</param>
        public static void RemoveLogProvider(Type providerType)
        {
            RemoveLogProvider(GetLogProvider(providerType));
        }
        #endregion

        #region AddProvider
        /// <summary>
        /// Adds a provider to the list
        /// </summary>
        /// <param name="logProvider">provider to add</param>
        public static void AddLogProvider(LogProvider logProvider)
        {
            if (logProvider == null)
            {
                throw new ArgumentNullException("logProvider");
            }

            lock (SyncObject)
            {
                if (registeredLogProviders == null)
                {
                    registeredLogProviders = new List<ILogProvider>();
                }

                // Only add provider if not already exists
                if (GetLogProvider(logProvider.GetType()) == null)
                {
                    logProvider.InitLog();
                    registeredLogProviders.Add(logProvider);
                }
            }
        }
        #endregion

        #region RegisterFormatter

        /// <summary>
        /// Registers new log message formatter
        /// </summary>
        /// <param name="newFormatter">New formatter to be used</param>
        public static void RegisterFormatter(ILogMessageFormatter newFormatter)
        {
            if (newFormatter == null)
            {
                throw new ArgumentNullException("newFormatter");
            }

            lock (SyncObject)
            {
                if (newFormatter != null)
                {
                    Formatter = newFormatter;
                }
            }
        }
        #endregion

        #region DeInit
        /// <summary>
        /// De-initializes the class (removes all providers)
        /// </summary>
        public static void Deinit()
        {
            RemoveProviders();
        }
        #endregion

        #region Custom Methods

        /// <summary>
        /// Changes the file log provider used by the logger. If no file log provider is already registered,
        /// this function does nothing.
        /// </summary>
        /// <param name="relativePath">The relative path to the user specified log folder</param>
        /// <param name="isVerbose">Verbosity of the log.</param>
        public static void ChangeGlobalFileLogProvider(string relativePath, bool isVerbose = false)
        {
            lock (SyncObject)
            {
                if (GetLogProvider(typeof(FileLog)) != null)
                {
                    if (relativePath == null)
                    {
                        throw new ArgumentNullException("path");
                    }

                    // Check if a file name with extension is provided in the Path
                    // If not provided, append it with a default log file path.
                    if (Path.GetExtension(relativePath) == string.Empty)
                    {
                        relativePath = Path.Combine(relativePath, "log.txt");
                    }

                    relativePath = Path.Combine(LogFolder, relativePath);

                    if (!m_logJobsInfoMap.ContainsKey(relativePath.ToLower()))
                    {
                        CreateLogProviderForPath(relativePath.ToLower(), isVerbose);
                    }

                    // Remove the existing file log provider from the registered log providers.
                    registeredLogProviders.Remove(GetLogProvider(typeof(FileLog)));

                    // Add the current file log provider.
                    registeredLogProviders.Add(m_logJobsInfoMap[relativePath.ToLower()].FileLogProvider);
                }
            }
        }
        #endregion

        #region ProvidersLog
        /// <summary>
        /// Spawns each provider to log the specified message
        /// </summary>
        /// <param name="logProviders">the list of log providers this message should be sent to</param>
        /// <param name="logMessage">message to log</param>
        private static void ProvidersLog(IReadOnlyList<ILogProvider> logProviders, ILogMessage logMessage)
        {
            if (logProviders != null)
            {
                foreach (LogProvider provider in logProviders)
                {
                    if ((provider.LogLevels & logMessage.LogLevel) != 0)
                    {
                        provider.Log(logMessage);
                    }
                }
            }
        }
        #endregion

        #region Private Methods
        /// <summary>
        /// Determines whether or not we can skip a specific log provider based on log level
        /// </summary>
        /// <param name="logLevel">log level</param>
        /// <returns>whether or not we can skip</returns>
        private static bool CanSkipThisLogLevel(LogLevels logLevel)
        {
            if (registeredLogProviders != null)
            {
                foreach (LogProvider provider in registeredLogProviders)
                {
                    if ((provider.LogLevels & logLevel) != 0)
                    {
                        return false;
                    }
                }
            }

            return true;
        }

        /// <summary>
        /// Do the actual logging
        /// </summary>
        /// <param name="logProviders">the list of log providers this message should be sent to</param>
        /// <param name="logLevel">log level</param>
        /// <param name="message">log message</param>
        /// <param name="messageArgs">message arguments</param>
        /// <param name="flags">The log flags</param>
        private static void DoLog(IReadOnlyList<ILogProvider> logProviders, LogLevels logLevel, string message, IMessageArg[] messageArgs, LogFlags flags)
        {
            lock (SyncObject)
            {
                logProviders = logProviders ?? registeredLogProviders;

                // If no provider needs this message - skip logging it.
                if (CanSkipThisLogLevel(logLevel))
                {
                    return;
                }

                if (Formatter == null)
                {
                    throw new ArgumentNullException("Formatter not set.");
                }

                ILogMessage logMessage = Formatter.CreateMessage(logLevel, message, messageArgs);
                ProvidersLog(logProviders, logMessage);
            }

            // Raise the message logged event outside of the lock. The code below will
            // invoke the handlers synchronously.
            if (!flags.HasFlag(LogFlags.DisableLogEventsFiring))
            {
                OnMessageLogged(new LogEventArgs
                {
                    LogLevel = logLevel,
                    Message = message,
                    MessageArgs = messageArgs
                });
            }
        }

        private static void OnMessageLogged(LogEventArgs e)
        {
            if (MessageLoggedEvent != null)
            {
                MessageLoggedEvent(null, e);
            }
        }

        /// <summary>
        /// Removes providers from the list
        /// </summary>
        private static void RemoveProviders()
        {
            lock (SyncObject)
            {
                if (registeredLogProviders != null)
                {
                    foreach (LogProvider provider in registeredLogProviders)
                    {
                        provider.DeinitLog();
                    }

                    registeredLogProviders.Clear();
                }

                registeredLogProviders = null;

                foreach (JobLogInfo jobLogInfo in m_logJobsInfoMap.Values)
                {
                    jobLogInfo.FileLogProvider.DeinitLog();
                }
            }
        }

        /// <summary>
        /// Creates the file log provider for a given log file under the user specified folder.
        /// Note: this method must be called while the lock is held.
        /// </summary>
        /// <param name="fullPath">The log file path for which a log provider should be created</param>
        /// <param name="isVerbose">Verbosity of the log.</param>
        private static void CreateLogProviderForPath(string fullPath, bool isVerbose = false)
        {
            // Create the sub directory under the Path provided.
            Directory.CreateDirectory(Path.GetDirectoryName(fullPath));

            FileLog fileLogger = new FileLog
            {
                LogLevels = isVerbose ? Logger.LogLevels.All : Logger.LogLevels.All & ~Logger.LogLevels.Debug,
                LogDecorations = Logger.LogDecorations.None,
                LogFilePath = fullPath
            };

            fileLogger.FlushEachLine = true;

            fileLogger.InitLog();

            m_logJobsInfoMap[fullPath] = new JobLogInfo();
            m_logJobsInfoMap[fullPath].FileLogProvider = fileLogger;
            m_logJobsInfoMap[fullPath].FolderPath = Path.GetDirectoryName(fullPath);
            m_logJobsInfoMap[fullPath].MainLogPath = fileLogger.LogFilePath;
        }
        #endregion

        /// <summary>
        /// Class defining the args passed to the event raised
        /// when a message is logged.
        /// </summary>
        public class LogEventArgs : EventArgs
        {
            public LogLevels LogLevel { get; set; }

            public string Message { get; set; }

            public IMessageArg[] MessageArgs { get; set; }
        }

        private class JobLogInfo
        {
            public ILogProvider FileLogProvider { get; set; }

            public string FolderPath { get; set; }

            public string MainLogPath { get; set; }
        }
    }
}
