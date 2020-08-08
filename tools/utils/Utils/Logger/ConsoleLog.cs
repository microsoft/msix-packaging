// -----------------------------------------------------------------------
//  <copyright file="ConsoleLog.cs" company="Microsoft">
//      Copyright (c) Microsoft Corporation.  All rights reserved.
//  </copyright>
// -----------------------------------------------------------------------
namespace Microsoft.Msix.Utils.Logger
{
    using System;
    using System.Reflection;

    /// <summary>
    /// Console log class
    /// </summary>
    public class ConsoleLog : LogProvider
    {
        /// <summary>
        /// Late binding for Console.ForegroundColor
        /// </summary>
        private PropertyInfo consoleForegroundColor = null;

        /// <summary>
        /// Late binding for Console.ResetColor
        /// </summary>
        private MethodInfo consoleResetColor = null;

        public ConsoleLog()
        {
            this.consoleForegroundColor = typeof(Console).GetProperty("ForegroundColor");
            this.consoleResetColor = typeof(Console).GetMethod("ResetColor");
        }

        /// <summary>
        /// Log a message
        /// </summary>
        /// <param name="logMessage">log message</param>
        public override void Log(ILogMessage logMessage)
        {
            if (logMessage == null)
            {
                throw new ArgumentNullException("logMessage");
            }

            this.SafeSetConsoleColorFromLogLevel(logMessage.LogLevel);

            // Output the message.
            switch (logMessage.LogLevel)
            {
                case Logger.LogLevels.Error:
                case Logger.LogLevels.Exp:
                case Logger.LogLevels.Critical:
                    Console.Error.WriteLine(logMessage.GetLogMessage(this.LogDecorations, this.LogLevels));
                    break;

                case Logger.LogLevels.InfoPrompt:
                    Console.Write(logMessage.GetLogMessage(this.LogDecorations, this.LogLevels));
                    break;

                case Logger.LogLevels.Warning:
                    Console.WriteLine(logMessage.GetLogMessage(this.LogDecorations, this.LogLevels));
                    break;

                default:
                    Console.WriteLine(logMessage.GetLogMessage(this.LogDecorations, this.LogLevels));
                    break;
            }

            // Always reset the console color
            this.SafeResetColor();
        }

        /// <summary>
        /// De-initializes the console log
        /// </summary>
        public override void DeinitLog()
        {
        }

        /// <summary>
        /// If available, sets the console's foreground color based on the level.
        /// </summary>
        /// <param name="level">The level to use when setting the foreground color.</param>
        private void SafeSetConsoleColorFromLogLevel(Logger.LogLevels level)
        {
            ConsoleColor targetColor = ConsoleColor.White;

                switch (level)
                {
                case Logger.LogLevels.Error:
                case Logger.LogLevels.Exp:
                case Logger.LogLevels.Critical:
                    targetColor = ConsoleColor.Red;
                        break;

                    case Logger.LogLevels.Warning:
                        targetColor = ConsoleColor.Yellow;
                        break;
                }

                this.consoleForegroundColor.SetValue(null, targetColor);
        }

        /// <summary>
        /// If available, resets the console's colors.
        /// </summary>
        private void SafeResetColor()
        {
            if (!object.ReferenceEquals(this.consoleResetColor, null))
            {
                this.consoleResetColor.Invoke(null, null);
            }
        }
    }
}
