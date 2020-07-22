//----------------------------------------------------------------------------------------------------------------------
// <copyright file="CommandLineTests.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//----------------------------------------------------------------------------------------------------------------------

namespace UtilsTests
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using Microsoft.Extensions.CommandLineUtils;
    using Microsoft.Packaging.Utils.CommandLine;
    using WEX.Logging.Interop;
    using WEX.TestExecution;
    using WEX.TestExecution.Markup;

    [TestClass]
    internal class CommandLineTests : TestBase
    {
        private static CLIApplication m_application;
        private static bool m_isCalled = false;
        private static int m_numberOfElements = 0;

        /// <summary>
        /// Test Initialize.
        /// </summary>
        [TestInitialize]
        public new void TestInitialize()
        {
            Log.Comment("Initializing CommandLineTests test.");
            CLIApplication.HelpDescriptionText = "Help description.";
            CLIApplication.VersionDescriptionText = "Version description";
            CLIApplication.HelpOptionSwitches = "-h";
            CLIApplication.VersionOptionSwitch = "-v";

            m_application = new CLIApplication(
                applicationName: "MyTool.exe",
                description: "Example for running the program",
                versionString: "Version 2.0",
                exampleCommands: new List<string>() { "MyTool.exe -wa a", "MyTool.exe -xa" },
                onCommandPreExecute: null);

            m_application.SetupInputs = (CommandLineApplication commandLineApplication) =>
            {
                ConfiguredInputs configuredInputs = new ConfiguredInputs();

                // Setup arguments
                CommandArgument firstArgument = commandLineApplication.Argument(
                    "firstArgument",
                    "this is the first argument");

                Action<string> firstArgumentValidator = (string data) =>
                {
                    if (data == "invalidFirstArgument")
                    {
                        throw new CommandParsingException(commandLineApplication, "Error in first argument invalidArgument.");
                    }
                };

                configuredInputs.Map["firstArgument"] = new ArgumentConfiguration(
                    firstArgument,
                    isRequired: false,
                    validationRoutine: firstArgumentValidator);

                CommandArgument secondArgument = commandLineApplication.Argument(
                    "secondArgument",
                    "this is the second argument");

                Action<string> secondArgumentValidator = (string data) =>
                {
                    if (data == "invalidSecondArgument")
                    {
                        throw new CommandParsingException(commandLineApplication, "Error in second argument invalidArgument.");
                    }
                };

                configuredInputs.Map["secondArgument"] = new ArgumentConfiguration(
                    secondArgument,
                    isRequired: false,
                    validationRoutine: secondArgumentValidator);

                // Setup -wa option
                CommandOption waOption = commandLineApplication.Option(
                    "-wa",
                    "MyTool.exe -wa something",
                    CommandOptionType.SingleValue);

                Action<string> waOptionValidator = (string data) =>
                {
                    if (data == "invalid")
                    {
                        throw new CommandParsingException(commandLineApplication, "Error in command wa.");
                    }
                };

                configuredInputs.Map["wa"] = new OptionConfiguration(
                    waOption,
                    isRequired: false,
                    validationRoutine: waOptionValidator);

                // Setup -bad option
                CommandOption badOption = commandLineApplication.Option(
                    "-bad",
                    "MyTool.exe -bad something",
                    CommandOptionType.SingleValue);

                Action<string> badOptionValidator = (string data) =>
                {
                    Log.Comment("Running bad command.");
                    throw new FileNotFoundException(string.Format("File {0} not found", data));
                };

                configuredInputs.Map["bad"] = new OptionConfiguration(
                    badOption,
                    isRequired: false,
                    validationRoutine: badOptionValidator);

                // Setup -multipleValue option
                CommandOption multipleValueOption = commandLineApplication.Option(
                    "-multipleValue",
                    "MyTool.exe -multipleValue something1 -multipleValue something2",
                    CommandOptionType.MultipleValue);

                Action<List<string>> multipleValueValidator = (List<string> values) =>
                {
                    Log.Comment("Running multipleValue.");
                    foreach (string value in values)
                    {
                        m_numberOfElements++;
                    }
                };

                configuredInputs.Map["multipleValue"] = new OptionConfiguration(
                    multipleValueOption,
                    isRequired: false,
                    validationRoutineMultipleValue: multipleValueValidator);

                // Setup -multipleValue2 option
                CommandOption multipleValue2Option = commandLineApplication.Option(
                    "-multipleValue2",
                    "MyTool.exe -multipleValue2 something1 -multipleValue2 something2",
                    CommandOptionType.MultipleValue);

                Action<string> multipleValue2Validator = (string values) =>
                {
                    Log.Comment("Running multipleValue.");
                    m_numberOfElements++;
                };

                configuredInputs.Map["multipleValue2"] = new OptionConfiguration(
                    multipleValue2Option,
                    isRequired: false,
                    validationRoutine: multipleValue2Validator);

                // Setup -xa option
                CommandOption xaOption = commandLineApplication.Option(
                    "-xa",
                    "MyTool.exe -xa",
                    CommandOptionType.NoValue);

                configuredInputs.Map["xa"] = new OptionConfiguration(
                    xaOption,
                    isRequired: false,
                    validationRoutine: null);

                return configuredInputs;
            };

            m_application.OnExecute = (ConfiguredInputs configuredInputs) =>
            {
                Log.Comment("Running on excecute for options.");

                return 0;
            };

            m_application.ConfigureCommand<CleanupCommand>();
        }

        /// <summary>
        /// Validate option wa and xa command
        /// </summary>
        [TestMethod]
        public void CommandLine_Valid_Option_Valid_Command()
        {
            Log.Comment("Testing valid option '-wa a -xa'");

            // Validate option wa and xa command
            ValidateCommand(true, m_application, new string[] { "-wa", "a", "-xa" });

            // Validate command cleanup
            ValidateCommand(true, m_application, new string[] { "cleanup" });
        }

        /// <summary>
        /// Make sure invalid scenario for wa command with no argument
        /// </summary>
        [TestMethod]
        public void CommandLine_No_Argument_WaOption()
        {
            Log.Comment("Testing invalid option '-wa' with no argument");

            // Invalid option wa with no argument
            ValidateCommand(false, m_application, new string[] { "-wa" });
        }

        /// <summary>
        /// Make sure invalid scenario for wa command
        /// </summary>
        [TestMethod]
        public void CommandLine_Invalid_Value_WaOption()
        {
            Log.Comment("Testing invalid option '-wa invalid'");

            // Invalid option wa with 'invalid' value
            ValidateCommand(false, m_application, new string[] { "-wa invalid" });
        }

        /// <summary>
        /// Validate wa option command
        /// </summary>
        [TestMethod]
        public void CommandLine_Valid_Wa_Option()
        {
            Log.Comment("Testing valid option '-wa a'");

            // Validate option wa with value 'a'
            ValidateCommand(true, m_application, new string[] { "-wa", "a" });
        }

        /// <summary>
        /// Validate xa option command
        /// </summary>
        [TestMethod]
        public void CommandLine_Valid_Xa_Option()
        {
            Log.Comment("Testing valid option '-xa'");

            // Validate option xa
            ValidateCommand(true, m_application, new string[] { "-xa" });
        }

        /// <summary>
        /// Validate help option
        /// </summary>
        [TestMethod]
        public void CommandLine_Valid_Help_Option()
        {
            Log.Comment("Testing valid option '-h'");

            // Validate option -h (help)
            ValidateCommand(true, m_application, new string[] { "-h" });
        }

        /// <summary>
        /// Validate help option
        /// </summary>
        [TestMethod]
        public void CommandLine_Valid_Help_Different_Switch_Option()
        {
            Log.Comment("Testing valid option '-h'");
            CLIApplication.HelpOptionSwitches = "-help";
            m_application = new CLIApplication(
                applicationName: "MyTool.exe",
                description: "Example for running the program",
                versionString: "Version 2.0",
                exampleCommands: new List<string>() { "MyTool.exe -wa a", "MyTool.exe -xa" },
                onCommandPreExecute: null);

            // Validate option -help
            ValidateCommand(true, m_application, new string[] { "-help" });
        }

        /// <summary>
        /// Validate version option
        /// </summary>
        [TestMethod]
        public void CommandLine_Valid_Version_Option()
        {
            Log.Comment("Testing valid option '-v'");

            // Validate option -v (version)
            ValidateCommand(true, m_application, new string[] { "-v" });
        }

        /// <summary>
        /// Validate version option different switch
        /// </summary>
        [TestMethod]
        public void CommandLine_Valid_Version_Different_Switch_Option()
        {
            Log.Comment("Testing valid option '-v'");
            CLIApplication.VersionOptionSwitch = "-version";
            m_application = new CLIApplication(
                applicationName: "MyTool.exe",
                description: "Example for running the program",
                versionString: "Version 2.0",
                exampleCommands: new List<string>() { "MyTool.exe -wa a", "MyTool.exe -xa" },
                onCommandPreExecute: null);

            // Validate option version
            ValidateCommand(true, m_application, new string[] { "-version" });
        }

        /// <summary>
        /// Validate onCommandPreExecute delegate is being executed.
        /// </summary>
        [TestMethod]
        public void CommandLine_Validate_OnCommandPreExecute()
        {
            Log.Comment("Testing valid option '-v'");
            m_isCalled = false;
            CLIApplication.VersionOptionSwitch = "-version";
            m_application = new CLIApplication(
                applicationName: "MyTool.exe",
                description: "Example for running the program",
                versionString: "Version 2.0",
                exampleCommands: new List<string>() { "MyTool.exe -wa a", "MyTool.exe -xa" },
                onCommandPreExecute: () =>
                {
                    m_isCalled = true;
                });
            m_application.ConfigureCommand<CleanupCommand>();

            Verify.IsFalse(m_isCalled, "isCalled should be false before the call is done.");

            // Validate onCommandPreExecute is being executed
            ValidateCommand(true, m_application, new string[] { "cleanup" });
            Verify.IsTrue(m_isCalled, "isCalled should be true after the call is done.");
        }

        /// <summary>
        /// / Validate RequiredSwitches for valid scenario
        /// </summary>
        [TestMethod]
        public void CommandLine_Validate_RequiredSwitches_With_SecondCommand_Valid()
        {
            Log.Comment("Testing valid option 'second-command --together --example a'");
            m_application.ConfigureCommand<SecondCommand>();

            // Validate RequiredSwitches is working with valid scenario
            ValidateCommand(true, m_application, new string[] { "second-command", "--together", "--example", "a" });
        }

        /// <summary>
        /// Validate RequiredSwitches for invalid scenario
        /// </summary>
        [TestMethod]
        public void CommandLine_Validate_RequiredSwitches_With_SecondCommand_Invalid()
        {
            Log.Comment("Testing invalid option 'second-command --together'");
            m_application.ConfigureCommand<SecondCommand>();

            // Validate RequiredSwitches is working with invalid scenario
            ValidateCommand(false, m_application, new string[] { "second-command", "--together" });
        }

        /// <summary>
        /// Validate DisallowedSwitches for valid scenario
        /// </summary>
        [TestMethod]
        public void CommandLine_Validate_DisallowedSwitches_With_SecondCommand_Valid()
        {
            Log.Comment("Testing valid option 'second-command --alone'");
            m_application.ConfigureCommand<SecondCommand>();

            // Validate RequiredSwitches is working with valid scenario
            ValidateCommand(true, m_application, new string[] { "second-command", "--alone" });
        }

        /// <summary>
        /// Validate DisallowedSwitches for invalid scenario
        /// </summary>
        [TestMethod]
        public void CommandLine_Validate_DisallowedSwitches_With_SecondCommand_Invalid()
        {
            Log.Comment("Testing valid option 'second-command --alone'");
            m_application.ConfigureCommand<SecondCommand>();

            // Validate RequiredSwitches is working with invalid scenario
            ValidateCommand(false, m_application, new string[] { "second-command", "--alone", "--together" });
        }

        /// <summary>
        /// Validate first argument valid scenario
        /// </summary>
        [TestMethod]
        public void CommandLine_Validate_FirstArgument_Valid()
        {
            Log.Comment("Testing valid argument 'firstArgument'");

            // Validate first argument is working with valid scenario
            ValidateCommand(true, m_application, new string[] { "firstArgument" });
        }

        /// <summary>
        /// Validate first argument invalid scenario
        /// </summary>
        [TestMethod]
        public void CommandLine_Validate_FirstArgument_Invalid()
        {
            Log.Comment("Testing invalid argument 'invalidFirstArgument'");

            // Validate first argument is working with invalid scenario
            ValidateCommand(false, m_application, new string[] { "invalidFirstArgument" });
        }

        /// <summary>
        /// Validate second argument valid scenario
        /// </summary>
        [TestMethod]
        public void CommandLine_Validate_SecondArgument_Valid()
        {
            Log.Comment("Testing valid argument 'firstArgument secondArgument'");

            // Validate second argument is working with valid scenario
            ValidateCommand(true, m_application, new string[] { "firstArgument", "secondArgument" });
        }

        /// <summary>
        /// Validate second argument invalid scenario
        /// </summary>
        [TestMethod]
        public void CommandLine_Validate_SecondArgument_Invalid()
        {
            Log.Comment("Testing invalid argument 'firstArgument invalidSecondArgument'");

            // Validate second argument is working with invalid scenario
            ValidateCommand(false, m_application, new string[] { "firstArgument", "invalidSecondArgument" });
        }

        /// <summary>
        /// Validate second argument valid scenario
        /// </summary>
        [TestMethod]
        public void CommandLine_Validate_FirstArgument_XaOption_Valid()
        {
            Log.Comment("Testing valid argument with option 'firstArgument -xa'");

            // Validate second argument is working with valid scenario
            ValidateCommand(true, m_application, new string[] { "firstArgument", "-xa" });
        }

        /// <summary>
        /// Validate second argument valid scenario
        /// </summary>
        [TestMethod]
        public void CommandLine_Validate_FirstArgument_SecondArgument_XaOption_Valid()
        {
            Log.Comment("Testing valid argument with option 'firstArgument secondArgument -xa'");

            // Validate second argument is working with valid scenario
            ValidateCommand(true, m_application, new string[] { "firstArgument", "secondArgument", "-xa" });
        }

        /// <summary>
        /// Validate OnInputValidationError return expected errors.
        /// </summary>
        [TestMethod]
        public void CommandLine_Validate_OnInputValidationError()
        {
            Log.Comment("Testing the function OnInputValidationError is returning expected errors.");

            bool onInputValidationErrorIsCalled = false;
            m_application.OnInputValidationError = (Exception ex) =>
            {
                onInputValidationErrorIsCalled = true;
                return ex.HResult;
            };

            // Validate bad option thrown the expected HResult.
            Verify.AreEqual(m_application.Execute(new string[] { "-bad", "something.xml" }), -2147024894);
            Verify.IsTrue(onInputValidationErrorIsCalled);
        }

        /// <summary>
        /// Validate OnInputValidationError for CommandBase classes return expected errors.
        /// </summary>
        [TestMethod]
        public void CommandLine_Validate_CommandBase_OnInputValidationError()
        {
            Log.Comment("Testing the function OnInputValidationError is returning expected errors for CommandBase command.");
            m_application.ConfigureCommand<ThirdCommand>();

            // Validate command base OnInputValidationError thrown the expected HResult.
            Verify.AreEqual(m_application.Execute(new string[] { "third-command", "--example", "something.xml" }), -2147024894);
        }

        /// <summary>
        /// Validate OnInputValidationError for CommandBase classes return expected errors.
        /// </summary>
        [TestMethod]
        public void CommandLine_Validate_CommandBase_TestValidateAllInputs()
        {
            Log.Comment("Testing the property ValidateAllInputs to setup more custom validation rules");
            m_application.ConfigureCommand<ThirdCommand>();

            // Validate command base OnInputValidationError thrown the expected HResult (InvalidOperationException as defined in
            // the ValidateAllInputs in ThirdCommand.SetupInputs
            Verify.AreEqual(m_application.Execute(new string[] { "third-command" }), -2146233079);
        }

        /// <summary>
        /// Validate option with multiple values multiple value validator
        /// </summary>
        [TestMethod]
        public void CommandLine_Validate_Option_MultipleValue_MultipleValue_Validator_Valid()
        {
            Log.Comment("Testing valid option with multiple value with multiple validator '-multipleValue something1 -multipleValue something2'");

            // Validate Option with multiple values is working with valid scenario with mutiple value validator
            m_numberOfElements = 0;
            Verify.AreEqual(m_numberOfElements, 0);
            ValidateCommand(true, m_application, new string[] { "-multipleValue", "something1", "-multipleValue", "something2" });
            Verify.AreEqual(m_numberOfElements, 2);
        }

        /// <summary>
        /// Validate option with multiple values single value validator
        /// </summary>
        [TestMethod]
        public void CommandLine_Validate_Option_MultipleValue_SingleValue_Validator_Valid()
        {
            Log.Comment("Testing valid option with multiple value with single validator '-multipleValue2 something1 -multipleValue2 something2'");

            // Validate Option with multiple values is working with valid scenario with single value validator
            m_numberOfElements = 0;
            Verify.AreEqual(m_numberOfElements, 0);
            ValidateCommand(true, m_application, new string[] { "-multipleValue2", "something1", "-multipleValue2", "something2" });
            Verify.AreEqual(m_numberOfElements, 2);
        }

        /// <summary>
        /// Validate argument with multiple values multiple value validator
        /// </summary>
        [TestMethod]
        public void CommandLine_Validate_Argument_MultipleValue_MultipleValue_Validator_Valid()
        {
            Log.Comment("Testing valid argument with multiple value with multiple value validator 'argumentone argumenttwo'");
            Log.Comment("Initializing CommandLineTests test.");

            m_application.SetupInputs = (CommandLineApplication commandLineApplication) =>
            {
                ConfiguredInputs configuredInputs = new ConfiguredInputs();

                // Setup arguments
                CommandArgument firstArgument = commandLineApplication.Argument(
                    "firstArgument",
                    "this is the first argument",
                    multipleValues: true);

                Action<List<string>> firstArgumentValidator = (List<string> values) =>
                {
                    foreach (string value in values)
                    {
                        m_numberOfElements++;
                    }
                };

                configuredInputs.Map["firstArgument"] = new ArgumentConfiguration(
                    firstArgument,
                    isRequired: false,
                    validationRoutineMultipleValue: firstArgumentValidator);

                return configuredInputs;
            };

            // Validate argument with multiple values is working with valid scenario with mutiple value validator
            m_numberOfElements = 0;
            Verify.AreEqual(m_numberOfElements, 0);
            ValidateCommand(true, m_application, new string[] { "argumenone", "argumentwo" });
            Verify.AreEqual(m_numberOfElements, 2);
        }

        /// <summary>
        /// Validate argument with multiple values single value validator
        /// </summary>
        [TestMethod]
        public void CommandLine_Validate_MultipleValue_SingleValue_Validator_Valid()
        {
            Log.Comment("Testing valid agument with multiple value with single value validator 'argumentone argumenttwo'");

            m_application.SetupInputs = (CommandLineApplication commandLineApplication) =>
            {
                ConfiguredInputs configuredInputs = new ConfiguredInputs();

                // Setup arguments
                CommandArgument firstArgument = commandLineApplication.Argument(
                    "firstArgument",
                    "this is the first argument",
                    multipleValues: true);

                Action<string> firstArgumentValidator = (string data) =>
                {
                    m_numberOfElements++;
                };

                configuredInputs.Map["firstArgument"] = new ArgumentConfiguration(
                    firstArgument,
                    isRequired: false,
                    validationRoutine: firstArgumentValidator);

                return configuredInputs;
            };

            // Validate Argument with multiple values is working with valid scenario with single value validator
            m_numberOfElements = 0;
            Verify.AreEqual(m_numberOfElements, 0);
            ValidateCommand(true, m_application, new string[] { "argumenone", "argumentwo" });
            Verify.AreEqual(m_numberOfElements, 2);
        }

        /// <summary>
        /// Test CleanUp.
        /// </summary>
        [TestCleanup]
        public new void TestCleanup()
        {
            Log.Comment("Cleaning CommandLineTests test.");
            m_application = null;
        }

        private static void ValidateCommand(bool isValidScenario, CLIApplication application, string[] args)
        {
            try
            {
                if (isValidScenario)
                {
                    // the command should be valid
                    Verify.AreEqual(application.Execute(args), 0);
                }
                else
                {
                    // the command should be invalid
                    Verify.AreNotEqual(application.Execute(args), 0);
                }
            }
            catch (Exception e)
            {
                Verify.Fail(e.Message);
            }
        }
    }
}
