// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

namespace Microsoft.Msix.Utils.CommandLine
{
    using System;
    using Microsoft.Extensions.CommandLineUtils;

    /// <summary>
    /// Base class for configuring and executing commands
    /// command can have arguments and options.
    /// </summary>
    public abstract class CommandBase
    {
        private CommandLineApplication m_commandLineApplication;
        private Action m_onPreExecute;

        /// <summary>
        /// Gets or sets the name of the command.
        /// Must be implemented by concreted derived classes
        /// </summary>
        protected abstract string CommandName { get; set; }

        /// <summary>
        /// Gets or sets the description of the command.
        /// Must be implemented by concreted derived classes
        /// </summary>
        protected abstract string CommandDescription { get; set; }

        /// <summary>
        /// Configures a concrete CommandBase object. Internal method used by CLIApplication
        /// Note: this static factory method is needed to avoid code duplication across the concrete
        /// CommandBase object, as we need to bass in the CommandLineApplication input to this object
        /// </summary>
        /// <typeparam name="T">The concrete CommandBase type</typeparam>
        /// <param name="commandLineApplication">The root application object</param>
        /// <param name="onPreExecute">The pre-execution routine to invoke after the inputs have been successfully parsed</param>
        /// <returns>The concrete CommandBase object</returns>
        internal static T Configure<T>(CommandLineApplication commandLineApplication, Action onPreExecute)
            where T : CommandBase, new()
        {
            T command = new T();
            command.m_commandLineApplication = commandLineApplication;
            command.m_onPreExecute = onPreExecute;

            command.ConfigureInternal();

            return command;
        }

        /// <summary>
        /// Method that must be implemented by the concreted derived classes to set up the inputs
        /// supported by this command.
        /// </summary>
        /// <param name="subCommandLineApplication">The Microsoft.Extensions.CommandLineUtils command object</param>
        /// <returns>The configured command inputs</returns>
        protected abstract ConfiguredInputs SetupInputs(CommandLineApplication subCommandLineApplication);

        /// <summary>
        /// Method that gets invoked when this command is being executed with validated inputs.
        /// Must be implemented by the concreted derived classes.
        /// Note: This method must handle all exceptions and return appropriate exit code for the program.
        /// </summary>
        /// <param name="configuredInputs">The validated inputs</param>
        /// <returns>The program exit code</returns>
        protected abstract int OnExecute(ConfiguredInputs configuredInputs);

        /// <summary>
        /// Method that gets invoked when the command validation thrown an exception.
        /// The implemented class could override this method to return different
        /// exit code.
        /// </summary>
        /// <param name="exception">Exception thrown by the validation call.</param>
        /// <returns>The program exit code</returns>
        protected virtual int OnInputValidationError(Exception exception)
        {
            Console.Error.WriteLine("Error: {0}", exception.Message);
            return 1;
        }

        /// <summary>
        /// Configures this command by hooking up the input setup and the execution callback.
        /// </summary>
        private void ConfigureInternal()
        {
            this.m_commandLineApplication.Command(
                this.CommandName,
                (subCommandLineApplication) =>
                {
                    // This callback is called by the CommandLineUtils library to configure description/inputs,
                    // and handle the exeuction logic
                    subCommandLineApplication.Description = this.CommandDescription;
                    CommandOption helpOption = subCommandLineApplication.Option(
                        CLIApplication.HelpOptionSwitches,
                        CLIApplication.HelpDescriptionText,
                        CommandOptionType.NoValue);

                    ConfiguredInputs configuredInputs = SetupInputs(subCommandLineApplication);

                    subCommandLineApplication.OnExecute(() =>
                    {
                        int exitCode = 1;

                        // Display help information and return the appropriate exit code if either no inputs
                        // where passed to a command that expects inputs, or a command parsing error occurred
                        try
                        {
                            configuredInputs.ValidateInputs(subCommandLineApplication);
                            configuredInputs.ValidateAllInputs?.Invoke();
                        }
                        catch (Exception ex)
                        {
                            exitCode = this.OnInputValidationError(ex);
                            subCommandLineApplication.ShowHelp();
                            return exitCode;
                        }

                        try
                        {
                            if (helpOption.HasValue())
                            {
                                Console.Write($"{this.CommandName}\t{this.CommandDescription}");
                                subCommandLineApplication.ShowHelp();
                                return 0;
                            }

                            // Execute the pre-execution routine if it is set
                            if (m_onPreExecute != null)
                            {
                                this.m_onPreExecute();
                            }

                            exitCode = OnExecute(configuredInputs);
                        }
                        catch (CommandParsingException exp)
                        {
                            // This will get invoked if the "command" part is not successfully parsed
                            Console.Error.Write(exp.Message);
                            subCommandLineApplication.ShowHelp();
                            exitCode = 1;
                        }

                        return exitCode;
                    });
                });
        }
    }
}
