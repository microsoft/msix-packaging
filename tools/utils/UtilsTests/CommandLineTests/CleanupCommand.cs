//-----------------------------------------------------------------------
// <copyright file="CleanupCommand.cs" company="Microsoft">
//     Copyright (c) Microsoft. All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

namespace UtilsTests
{
    using Microsoft.Extensions.CommandLineUtils;
    using Microsoft.Packaging.Utils.CommandLine;
    using WEX.Logging.Interop;

    /// <summary>
    /// Example of command.
    /// </summary>
    public class CleanupCommand : CommandBase
    {
        /// <summary>
        /// Gets or sets the name of the command.
        /// </summary>
        protected override string CommandName { get; set; } = "cleanup";

        /// <summary>
        /// Gets or sets the description of the command.
        /// </summary>
        protected override string CommandDescription { get; set; } = "Cleans up the tool's temporary files and artifacts.";

        /// <summary>
        /// Sets up the inputs supported by this command.
        /// </summary>
        /// <param name="commandLineApplication">The Microsoft.Extensions.CommandLineUtils command object</param>
        /// <returns>The configured command inputs</returns>
        protected override ConfiguredInputs SetupInputs(CommandLineApplication commandLineApplication)
        {
            // This command supports no inputs
            ConfiguredInputs configuredInputs = new ConfiguredInputs();
            return configuredInputs;
        }

        /// <summary>
        /// Method that gets invoked when this command is being executed with validated inputs.
        /// Note: This method must handle all exceptions and return appropriate exit code for the program.
        /// </summary>
        /// <param name="configuredInputs">The validated inputs</param>
        /// <returns>The program exit code</returns>
        protected override int OnExecute(ConfiguredInputs configuredInputs)
        {
            Log.Comment("Running OnExecute for command cleanup");
            return 0;
        }
    }
}
