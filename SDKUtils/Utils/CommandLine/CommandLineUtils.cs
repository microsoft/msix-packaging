//-----------------------------------------------------------------------
// <copyright file="CommandLineUtils.cs" company="Microsoft">
//     Copyright (c) Microsoft. All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

// Please use CLIApplication instead, this class is being deprecated.
namespace Microsoft.Packaging.SDKUtils.CommandLine
{
    using System;
    using Microsoft.Packaging.SDKUtils.Logger;

    /// <summary>
    /// A class that contains helper functions for parsing command-line arguments.
    /// </summary>
    public class CommandLineUtils
    {
        /// <summary>
        /// Common prefix for all the option flags.
        /// </summary>
        public const string OptionPrefix = "-";

        /// <summary>
        /// Logs a message indenting it with whitespace.
        /// </summary>
        /// <param name="message">Message to print.</param>
        public static void PrintIndented(string message)
        {
            const int Indent = 4;
            string indentStr = new string(' ', Indent);
            Logger.Info(indentStr + message);
        }

        /// <summary>
        /// Prints the help message for a command.
        /// </summary>
        /// <param name="command">The command.</param>
        /// <param name="description">A description of the command.</param>
        public static void PrintCommand(string command, string description)
        {
            const int CommandPad = 16;
            string message = command.PadRight(CommandPad) + " -- " + description;
            PrintIndented(message);
        }

        /// <summary>
        /// Prints the help message for a flag option.
        /// </summary>
        /// <param name="option">A command line option.</param>
        /// <param name="description">Description of the option.</param>
        public static void PrintFlag(string option, string description)
        {
            const int FlagsPad = 8;
            string message = option.PadRight(FlagsPad) + description;
            PrintIndented(message);
        }

        /// <summary>
        /// Prints the help message for an option.
        /// </summary>
        /// <param name="option">A command line option.</param>
        /// <param name="argument">The name of the option's argument.</param>
        /// <param name="description">Description of the option.</param>
        public static void PrintOption(string option, string argument, string description)
        {
            const int OptionsPad = 28;
            string message = (option + " " + argument).PadRight(OptionsPad) + description;
            PrintIndented(message);
        }

        /// <summary>
        /// Reads a single parameter given for an option. Modifies optionIndex to point to the first
        /// argument after the option and its parameter.
        /// </summary>
        /// <param name="args">Program arguments.</param>
        /// <param name="optionIndex">Index at which the option appears.</param>
        /// <param name="messageMissing">Error message to show if the parameter is missing.</param>
        /// <returns>The parameter given for the option.</returns>
        public static string GetOptionParameter(string[] args, ref int optionIndex, string messageMissing)
        {
            optionIndex++;
            if (optionIndex == args.Length ||
                args[optionIndex].StartsWith(OptionPrefix, StringComparison.Ordinal))
            {
                throw new CommandLineException(messageMissing);
            }

            return args[optionIndex++];
        }
    }
}
