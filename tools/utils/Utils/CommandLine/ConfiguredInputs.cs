//-----------------------------------------------------------------------
// <copyright file="ConfiguredInputs.cs" company="Microsoft">
//     Copyright (c) Microsoft. All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

namespace Microsoft.Msix.Utils.CommandLine
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using Microsoft.Extensions.CommandLineUtils;

    /// <summary>
    /// A class holding all the inputs associated with a command/application
    /// </summary>
    public class ConfiguredInputs
    {
        /// <summary>
        /// Gets the map of all configured inputs
        /// </summary>
        public Dictionary<string, InputConfigurationBase> Map { get; private set; }
            = new Dictionary<string, InputConfigurationBase>();

        /// <summary>
        /// Gets or sets a handler that allows consumers to define additional/more complex validation rules across
        /// the set of all inputs
        /// </summary>
        public Action ValidateAllInputs { get; set; }

        /// <summary>
        /// Validates the inputs specified
        /// </summary>
        /// <param name="commandLineApplication">The command whose inputs are being validated</param>
        internal void ValidateInputs(CommandLineApplication commandLineApplication)
        {
            foreach (KeyValuePair<string, InputConfigurationBase> entry in this.Map)
            {
                // Validate required inputs is specified
                if (entry.Value.IsRequired && !entry.Value.HasValue())
                {
                    throw new CommandParsingException(
                        commandLineApplication,
                        string.Format("{0} {1} is required.", entry.Value is OptionConfiguration ? "Option" : "Argument", entry.Key));
                }

                if (entry.Value.HasValue())
                {
                    // Validated disallowed switches are not specified with this option
                    List<string> disallowedOptionSwitchesSpecified = entry.Value.DisallowedSwitches
                        .Where(disallowedSwitch => this.Map[disallowedSwitch] is OptionConfiguration && this.Map[disallowedSwitch].HasValue())
                        .ToList();

                    if (disallowedOptionSwitchesSpecified.Count != 0)
                    {
                        throw new CommandParsingException(
                            commandLineApplication,
                            string.Format("{0} cannot be specified with option {1}.", disallowedOptionSwitchesSpecified[0], entry.Key));
                    }

                    // Validated disallowed switches are not keys to arguments
                    bool areDisallowedArgumentSwitchesSpecified = entry.Value.DisallowedSwitches
                        .Any(disallowedSwitch => this.Map[disallowedSwitch] is ArgumentConfiguration);

                    if (areDisallowedArgumentSwitchesSpecified)
                    {
                        throw new InvalidOperationException("Cannot specify an argument key for disallowedSwitches.");
                    }

                    // Validate required switches are specified with this option
                    List<string> requiredOptionSwitchesNotSpecified = entry.Value.RequiredSwitches
                        .Where(requiredSwitch => this.Map[requiredSwitch] is OptionConfiguration && !this.Map[requiredSwitch].HasValue())
                        .ToList();

                    if (requiredOptionSwitchesNotSpecified.Count != 0)
                    {
                        throw new CommandParsingException(
                            commandLineApplication,
                            string.Format("{0} must be specified with option {1}.", requiredOptionSwitchesNotSpecified[0], entry.Key));
                    }

                    // Validate required switches are not keys to arguments
                    // For now required argument switches is not allow. If the 
                    // functionality is needed in the future could be added.
                    bool areRequiredArgumentSwitchesNotSpecified = entry.Value.RequiredSwitches
                        .Any(requiredSwitch => this.Map[requiredSwitch] is ArgumentConfiguration);

                    if (areRequiredArgumentSwitchesNotSpecified)
                    {
                        throw new InvalidOperationException(
                            "Cannot specify an argument key for requiredSwitches.");
                    }

                    // Validate option value
                    entry.Value.Validate();
                }
            }
        }
    }
}