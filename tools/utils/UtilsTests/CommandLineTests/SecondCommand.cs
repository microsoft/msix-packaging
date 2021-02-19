//-----------------------------------------------------------------------
// <copyright file="SecondCommand.cs" company="Microsoft">
//     Copyright (c) Microsoft. All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

namespace UtilsTests
{
    using System;
    using System.Collections.Generic;
    using Microsoft.Extensions.CommandLineUtils;
    using Microsoft.Msix.Utils.CommandLine;
    using Microsoft.VisualStudio.TestTools.UnitTesting.Logging;

    /// <summary>
    /// Second command.
    /// </summary>
    public class SecondCommand : CommandBase
    {
        /// <summary>
        /// Gets or sets the name of the command.
        /// </summary>
        protected override string CommandName { get; set; } = "second-command";

        /// <summary>
        /// Gets or sets the description of the command.
        /// </summary>
        protected override string CommandDescription { get; set; } = "This is a second-command.";

        /// <summary>
        /// Sets up the inputs supported by this command.
        /// </summary>
        /// <param name="commandLineApplication">The Microsoft.Extensions.CommandLineUtils command object</param>
        /// <returns>The configured command inputs</returns>
        protected override ConfiguredInputs SetupInputs(CommandLineApplication commandLineApplication)
        {
            ConfiguredInputs configuredInputs = new ConfiguredInputs();

            // Setup --example option
            CommandOption exampleOption = commandLineApplication.Option(
                "--example",
                "--example description",
                CommandOptionType.SingleValue);

            Action<string> templateFileOptionValidator = (string data) =>
            {
                if ("invalid".Equals(data))
                {
                    throw new CommandParsingException(commandLineApplication, string.Format("Invalid value for option --example"));
                }
            };

            // --example don't declare any restriction
            configuredInputs.Map["--example"] = new OptionConfiguration(
                exampleOption,
                isRequired: false,
                validationRoutine: templateFileOptionValidator);

            // Setup --alone option
            CommandOption aloneOption = commandLineApplication.Option(
                "--alone",
                "--alone description",
                CommandOptionType.NoValue);

            // --alone cannot be declared with "--together" or "--example"
            configuredInputs.Map["--alone"] = new OptionConfiguration(
                aloneOption,
                isRequired: false,
                disallowedSwitches: new List<string>() { "--together", "--example" },
                validationRoutine: null);

            // Setup --together option
            CommandOption togheterOption = commandLineApplication.Option(
                "--together",
                "--together description",
                CommandOptionType.NoValue);

            // --together option require declare --example
            configuredInputs.Map["--together"] = new OptionConfiguration(
                togheterOption,
                isRequired: false,
                validationRoutine: null,
                requiredSwitches: new List<string>() { "--example" });

            return configuredInputs;
        }

        /// <summary>
        /// Method that gets invoked when this command is being executed with validated arguments.
        /// Note: This method must handle all exceptions and return appropriate exit code for the program.
        /// </summary>
        /// <param name="configuredInputs">The validated arguments</param>
        /// <returns>The program exit code</returns>
        protected override int OnExecute(ConfiguredInputs configuredInputs)
        {
            Logger.LogMessage("Running OnExecute for command second-command");
            return 0;
        }
    }
}
