// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

namespace Microsoft.Msix.Utils.CommandLine
{
    using System;
    using System.Collections.Generic;
    using System.Text;
    using Microsoft.Extensions.CommandLineUtils;

    /// <summary>
    /// Class used for configuring and executing a command line application
    /// </summary>
    public class CLIApplication
    {
        private CommandLineApplication m_commandLineApplication;
        private Action m_onCommandPreExecute;
        private CommandOption m_helpOption;
        private CommandOption m_versionOption;
        private string m_versionString;

        /// <summary>
        /// Initializes a new instance of the <see cref="CLIApplication"/> class.
        /// </summary>
        /// <param name="applicationName">The name of the application</param>
        /// <param name="description">The description of the application</param>
        /// <param name="versionString">The version string of the application</param>
        /// <param name="exampleCommands">List of valid commands.</param>
        /// <param name="onCommandPreExecute"> Gets or sets the global pre-execute routine for all commands.
        /// Used to set up logging etc. after the CLI inputs have been parsed and before execution starts.</param>
        public CLIApplication(
            string applicationName,
            string description,
            string versionString,
            List<string> exampleCommands,
            Action onCommandPreExecute)
        {
            this.m_commandLineApplication = new CommandLineApplication();

            // Setup general name, description, help option for the tool
            this.m_commandLineApplication.Name = applicationName;
            this.m_commandLineApplication.Description = description;
            this.m_helpOption = this.m_commandLineApplication.Option(
                HelpOptionSwitches,
                HelpDescriptionText,
                CommandOptionType.NoValue);

            this.m_versionOption = this.m_commandLineApplication.Option(
                VersionOptionSwitch,
                VersionDescriptionText,
                CommandOptionType.NoValue);
            this.m_versionString = versionString;

            if (exampleCommands != null && exampleCommands.Count > 0)
            {
                StringBuilder stringBuilder = new StringBuilder();
                stringBuilder.Append(Environment.NewLine);
                stringBuilder.Append("Examples:");
                stringBuilder.Append(Environment.NewLine);

                foreach (string exampleCommand in exampleCommands)
                {
                    stringBuilder.Append(exampleCommand);
                    stringBuilder.Append(Environment.NewLine);
                }

                this.m_commandLineApplication.ExtendedHelpText = stringBuilder.ToString();
            }

            this.m_onCommandPreExecute = onCommandPreExecute;
        }

        /// <summary>
        /// Gets or sets the description for the --help option. Can be overwritten to provide localized description
        /// </summary>
        public static string HelpDescriptionText { get; set; } = "Show help information";

        /// <summary>
        /// Gets or sets the description for the --version option. Can be overwritten to provide localized description
        /// </summary>
        public static string VersionDescriptionText { get; set; } = "Show version information";

        /// <summary>
        /// Gets or sets the help option switches
        /// </summary>
        public static string HelpOptionSwitches { get; set; } = "-h";

        /// <summary>
        /// Gets or sets the version option switches
        /// </summary>
        public static string VersionOptionSwitch { get; set; } = "-v";

        /// <summary>
        /// Gets or sets up the function to Setup the Inputs.
        /// </summary>
        public Func<CommandLineApplication, ConfiguredInputs> SetupInputs { get; set; }

        /// <summary>
        /// Gets or sets the function that gets invoked when this command is being executed with validated inputs.
        /// Note: This method must handle all exceptions and return appropriate exit code for the program.
        /// </summary>
        public Func<ConfiguredInputs, int> OnExecute { get; set; }

        /// <summary>
        /// Gets or sets the function that is used to translate an exception into an exit code, as desired.
        /// Typically, consumers need to handle exceptions and translate them into error codes inside OnExecute
        /// method, so this handler is not needed. However if specific exit codes are needed for certain argument
        /// validation exceptions, this method can define the translation.
        /// </summary>
        public Func<Exception, int> OnInputValidationError { get; set; }

        /// <summary>
        /// Configures a concrete CommandBase object.
        /// </summary>
        /// <typeparam name="T">The concrete CommandBase type</typeparam>
        /// <returns>The concrete CommandBase object</returns>
        public T ConfigureCommand<T>() where T : CommandBase, new()
        {
            return CommandBase.Configure<T>(this.m_commandLineApplication, this.m_onCommandPreExecute);
        }

        /// <summary>
        /// Executes the CLI application. Handles all exceptions and returns appropriate exit code
        /// </summary>
        /// <param name="args">The inputs to the program</param>
        /// <returns>Return 0 if the function succeeded, other value if failed</returns>
        public int Execute(string[] args)
        {
            int exitCode = 1;

            try
            {
                ConfiguredInputs configuredInputs = null;

                if (this.SetupInputs != null)
                {
                    configuredInputs = this.SetupInputs.Invoke(this.m_commandLineApplication);
                }

                this.m_commandLineApplication.OnExecute(() =>
                {
                    if (this.m_versionOption.HasValue())
                    {
                        Console.WriteLine(this.m_versionString);
                        return 0;
                    }
                    else if (this.m_helpOption.HasValue())
                    {
                        ShowHelp();
                        return 0;
                    }

                    if (configuredInputs != null)
                    {
                        try
                        {
                            configuredInputs.ValidateInputs(this.m_commandLineApplication);
                        }
                        catch (Exception exp)
                        {
                            if (this.OnInputValidationError != null)
                            {
                                exitCode = this.OnInputValidationError.Invoke(exp);
                            }
                            else
                            {
                                // This can happen if caller doesn't want to use OnInputValidationError - and so
                                // CLIApplication will default to just returning 1 as the exit code
                                Console.Error.WriteLine("Error: {0}", exp.Message);
                                exitCode = 1;
                            }

                            ShowHelp();
                            return exitCode;
                        }

                        if (this.OnExecute != null)
                        {
                            return this.OnExecute.Invoke(configuredInputs);
                        }
                        else
                        {
                            ShowHelp();
                            return 1;
                        }
                    }
                    else
                    {
                        ShowHelp();
                        return 1;
                    }
                });

                exitCode = this.m_commandLineApplication.Execute(args);
            }
            catch (CommandParsingException exp)
            {
                // This will get invoked if the "command" part is not successfully parsed
                Console.Error.WriteLine(exp.Message);
                this.m_commandLineApplication.ShowHelp();
                exitCode = 1;
            }

            return exitCode;
        }

        private void ShowHelp()
        {
            Console.WriteLine(this.m_versionString);
            Console.WriteLine();
            Console.Write(this.m_commandLineApplication.Description);
            this.m_commandLineApplication.ShowHelp();
        }
    }
}
