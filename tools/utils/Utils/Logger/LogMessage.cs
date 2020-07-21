// -----------------------------------------------------------------------
//  <copyright file="LogMessage.cs" company="Microsoft">
//      Copyright (c) Microsoft Corporation.  All rights reserved.
//  </copyright>
// -----------------------------------------------------------------------
namespace Microsoft.Packaging.SDKUtils.Logger
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.IO;
    using System.Text;

    /// <summary>
    /// Log message class
    /// </summary>
    public class LogMessage : ILogMessage
    {
        #region private members
        /// <summary>
        /// Log message arguments
        /// </summary>
        private readonly List<IMessageArg> logMessageArgs;

        /// <summary>
        /// Log message
        /// </summary>
        private string logMessage;

        /// <summary>
        /// Log time information
        /// </summary>
        private string logTimeInfo;

        /// <summary>
        /// Log level information
        /// </summary>
        private string logLevelInfo;

        /// <summary>
        /// Log detailed information, such as file name, line number etc.
        /// </summary>
        private string logDetailInfo;
        #endregion

        #region Ctors
        /// <summary>
        /// Initializes a new instance of the <see cref="LogMessage" /> class.
        /// </summary>
        /// <param name="logLevel">log level</param>
        /// <param name="logMessage">log message</param>
        /// <param name="funcStackFrame">function stack frame</param>
        /// <param name="messageArgs">message arguments</param>
        public LogMessage(
            Logger.LogLevels logLevel,
            string logMessage,
            StackFrame funcStackFrame,
            IEnumerable<IMessageArg> messageArgs)
        {
            this.LogLevel = logLevel;
            this.logMessage = logMessage;

            if (messageArgs != null)
            {
                this.logMessageArgs = new List<IMessageArg>(messageArgs);
            }

            this.FillInLogArgs(messageArgs);
            this.GenerateLogDecorations(funcStackFrame);
        }
        #endregion

        #region public members
        /// <summary>
        /// Gets the log level
        /// </summary>
        public Logger.LogLevels LogLevel { get; private set; }

        /// <summary>
        /// Gets the message arguments
        /// </summary>
        public IReadOnlyCollection<IMessageArg> LogMessageArgs
        {
            get
            {
                return this.logMessageArgs.AsReadOnly();
            }
        }
        #endregion

        #region GetLogMessage
        /// <summary>
        /// Processes and returns the log message
        /// </summary>
        /// <param name="logDecoration">log decoration</param>
        /// <param name="logLevel">log level</param>
        /// <returns>log message</returns>
        public string GetLogMessage(Logger.LogDecorations logDecoration, Logger.LogLevels logLevel)
        {
            StringBuilder messageBuilder = new StringBuilder(this.logMessage.Length);
            messageBuilder.Insert(0, this.logMessage);
            if (logDecoration != Logger.LogDecorations.None)
            {
                if (logDecoration.HasFlag(Logger.LogDecorations.Detail))
                {
                    messageBuilder.Insert(0, this.logDetailInfo);
                }

                if (logDecoration.HasFlag(Logger.LogDecorations.LogLevel))
                {
                    messageBuilder.Insert(0, this.logLevelInfo);
                }

                if (logDecoration.HasFlag(Logger.LogDecorations.Time))
                {
                    messageBuilder.Insert(0, this.logTimeInfo);
                }
            }

            // Check to see if we need to add exception info to message.
            if (logLevel.HasFlag(Logger.LogLevels.ExpStack) &&
                (this.logMessageArgs != null) &&
                (this.logMessageArgs[0] is ExpMessageArg))
            {
                messageBuilder.Append(Environment.NewLine + this.GetExceptionStackLog());
            }

            return messageBuilder.ToString();
        }
        #endregion

        #region GenerateTimeDecoration
        /// <summary>
        /// Generates time decoration
        /// </summary>
        private void GenerateTimeDecoration()
        {
            this.logTimeInfo = "[" + DateTime.Now + "] ";
        }
        #endregion

        #region GeneratelogLevelInfo
        /// <summary>
        /// Generates log level information
        /// </summary>
        private void GenerateLogLevelDecoration()
        {
            this.logLevelInfo = "[" + this.LogLevel + "] ";
        }
        #endregion

        #region GenerateDetailDecoration
        /// <summary>
        /// Generates detail decoration (file name, function name, etc.) including stack frame log
        /// </summary>
        /// <param name="funcStackFrame">function stack frame</param>
        private void GenerateDetailDecoration(StackFrame funcStackFrame)
        {
            // Make it the same format as VS debug string so we can double click and get to the right line.
            // Ex. filename.cpp(53) : message

            // First get the stack info on the function that called the logger.
            if (funcStackFrame != null)
            {
                int lineNum = funcStackFrame.GetFileLineNumber();
                string fileName = Path.GetFileName(funcStackFrame.GetFileName());
                string funcName = funcStackFrame.GetMethod().Name;
                this.logDetailInfo = fileName + "(" + lineNum + ",1): " +
                                     funcName + "()" + (string.IsNullOrEmpty(this.logMessage) ? string.Empty : " -> ");
            }
        }
        #endregion

        #region GenerateLogDecorations
        /// <summary>
        /// Generates log decorations
        /// </summary>
        /// <param name="funcStackFrame">function stack frame</param>
        private void GenerateLogDecorations(StackFrame funcStackFrame)
        {
            this.GenerateDetailDecoration(funcStackFrame);
            this.GenerateLogLevelDecoration();
            this.GenerateTimeDecoration();
        }
        #endregion

        #region GetExceptionStackLog
        /// <summary>
        /// Get the exception stack log
        /// </summary>
        /// <returns>exception message</returns>
        private string GetExceptionStackLog()
        {
            if (this.logMessageArgs == null || this.logMessageArgs.Count != 1 || !(this.logMessageArgs[0] is ExpMessageArg))
            {
                return string.Empty;
            }

            Exception logExp = (this.logMessageArgs[0] as ExpMessageArg).MessageArgument as Exception;

            // Since exception can come from any StackFrame - we should read the function
            // info from the exp stack frame info directly.
            // Check first to see if there is target info in the exp object.
            string expFileName = (logExp.TargetSite == null) ? "No Info on target site file name" : logExp.TargetSite.ReflectedType.FullName;
            string expFuncName = (logExp.TargetSite == null) ? "No Info on target site func name" : logExp.TargetSite.Name;
            return expFileName + ". " + expFuncName + "() -> " + logExp.Message +
                "\r\nStack Trace:\r\n" + logExp.StackTrace;
        }
        #endregion

        #region FillInLogArgs
        /// <summary>
        /// Fill in log arguments from message
        /// </summary>
        /// <param name="messageArgs">message arguments</param>
        private void FillInLogArgs(IEnumerable<IMessageArg> messageArgs)
        {
            if (messageArgs != null)
            {
                foreach (IMessageArg messageArg in messageArgs)
                {
                    if (messageArg is ExpMessageArg)
                    {
                        Exception logExp = messageArg.MessageArgument as Exception;
                        this.logMessage = logExp.Message;
                    }
                }
            }
        }
        #endregion
    }
}
