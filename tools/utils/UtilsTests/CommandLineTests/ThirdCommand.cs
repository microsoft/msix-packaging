//----------------------------------------------------------------------------------------------------------------------
// <copyright file="ThirdCommand.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//----------------------------------------------------------------------------------------------------------------------

namespace UtilsTests
{
    using System;
    using System.IO;
    using Microsoft.Extensions.CommandLineUtils;
    using Microsoft.Msix.Utils.CommandLine;
    using WEX.Logging.Interop;

    /// <summary>
    /// Second command.
    /// </summary>
    public class ThirdCommand : CommandBase
    {
        /// <summary>
        /// Gets or sets the name of the command.
        /// </summary>
        protected override string CommandName { get; set; } = "third-command";

        /// <summary>
        /// Gets or sets the description of the command.
        /// </summary>
        protected override string CommandDescription { get; set; } = "This is a second-command.";

        /// <summary>
        /// On input validation error
        /// </summary>
        /// <param name="exception">The exception</param>
        /// <returns>the exception HRESULT</returns>
        protected override int OnInputValidationError(Exception exception)
        {
            return exception.HResult;
        }

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
                throw new FileNotFoundException(string.Format("File {0} not found", data));
            };

            // --example don't declare any restriction
            configuredInputs.Map["--example"] = new OptionConfiguration(
                exampleOption,
                isRequired: false,
                validationRoutine: templateFileOptionValidator);

            // Setup --example2 option
            CommandOption example2Option = commandLineApplication.Option(
                "--example2",
                "--example description",
                CommandOptionType.SingleValue);

            // --example don't declare any restriction
            configuredInputs.Map["--example2"] = new OptionConfiguration(
                example2Option,
                isRequired: false);

            configuredInputs.ValidateAllInputs = () =>
            {
                // Require at least one of the two switches to be present
                if (!configuredInputs.Map["--example"].HasValue() && !configuredInputs.Map["--example2"].HasValue())
                {
                    // InvalidOperationException has HRESULT = 0x80131509
                    throw new InvalidOperationException("Must specify at least one of --example and --example2");
                }
            };

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
            Log.Comment("Running OnExecute for command second-command");
            return 0;
        }
    }
}
