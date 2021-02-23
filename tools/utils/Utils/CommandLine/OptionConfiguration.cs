// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

namespace Microsoft.Msix.Utils.CommandLine
{
    using System;
    using System.Collections.Generic;
    using Microsoft.Extensions.CommandLineUtils;

    /// <summary>
    /// Class wrapping around Microsoft.Extensions.CommandLineUtils.CommandOption containing
    /// additional information for validation.
    /// </summary>
    public class OptionConfiguration : InputConfigurationBase
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="OptionConfiguration"/> class (with no value or single or multiple values).
        /// </summary>
        /// <param name="option">The CommandOption object</param>
        /// <param name="isRequired">A flag indicating whether the option is required</param>
        /// <param name="disallowedSwitches">A list of switches that are not allowed when this option is specified</param>
        /// <param name="requiredSwitches">A list of switches that are required when this option is specified</param>
        /// <param name="validationRoutine">The validation routine to be used to validate the value entered for this option</param>
        /// <param name="validationRoutineMultipleValue">The validation routine to be used to validate multiple values entered for this option</param>
        public OptionConfiguration(
            CommandOption option,
            bool isRequired,
            List<string> disallowedSwitches = null,
            List<string> requiredSwitches = null,
            Action<string> validationRoutine = null,
            Action<List<string>> validationRoutineMultipleValue = null) :
            base(isRequired, disallowedSwitches, requiredSwitches, validationRoutine, validationRoutineMultipleValue)
        {
            this.Option = option;
            this.InternvalValidateValidatorApplicability(validationRoutine, validationRoutineMultipleValue);
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="OptionConfiguration"/> class for a non-required option (with no value or single or multiple values).
        /// </summary>
        /// <param name="option">The CommandOption object</param>
        /// <param name="disallowedSwitches">A list of switches that are not allowed when this option is specified</param>
        /// <param name="requiredSwitches">A list of switches that are required when this option is specified</param>
        /// <param name="validationRoutine">The validation routine to be used to validate the value entered for this option</param>
        /// <param name="validationRoutineMultipleValue">The validation routine to be used to validate multiple values entered for this option</param>
        public OptionConfiguration(
            CommandOption option,
            List<string> disallowedSwitches = null,
            List<string> requiredSwitches = null,
            Action<string> validationRoutine = null,
            Action<List<string>> validationRoutineMultipleValue = null) :
            base(false, disallowedSwitches, requiredSwitches, validationRoutine, validationRoutineMultipleValue)
        {
            this.Option = option;
            this.InternvalValidateValidatorApplicability(validationRoutine, validationRoutineMultipleValue);
        }

        /// <summary>
        /// Gets the CommandOption object
        /// </summary>
        public CommandOption Option { get; }

        public override bool HasValue()
        {
            return this.Option.HasValue();
        }

        public override string Value()
        {
            return this.Option.HasValue() ? this.Option.Value() : string.Empty;
        }

        public override List<string> Values()
        {
            return this.Option.HasValue() ? this.Option.Values : null;
        }

        public override void Validate()
        {
            if (this.Option.OptionType == CommandOptionType.MultipleValue)
            {
                if (this.ValidationRoutineMultipleValues != null)
                {
                    this.ValidationRoutineMultipleValues(this.Option.Values);
                }
                else if (this.ValidationRoutineSingleValue != null)
                {
                    foreach (string value in this.Option.Values)
                    {
                        this.ValidationRoutineSingleValue(value);
                    }
                }
            }
            else if (this.ValidationRoutineSingleValue != null)
            {
                this.ValidationRoutineSingleValue(this.Option.Value());
            }
        }

        private void InternvalValidateValidatorApplicability(Action<string> validationRoutineSingleValue, Action<List<string>> validationRoutineMultipleValues)
        {
            if (validationRoutineMultipleValues != null && (this.Option.OptionType == CommandOptionType.NoValue || this.Option.OptionType == CommandOptionType.SingleValue))
            {
                throw new InvalidOperationException(string.Format(
                    "Validation routine for multiple values is not allowed with CommandOptionType.{0}.",
                    this.Option.OptionType.ToString()));
            }
            else if (validationRoutineSingleValue != null && this.Option.OptionType == CommandOptionType.NoValue)
            {
                throw new InvalidOperationException("Validation routine for multiple values is not allowed with CommandOptionType.NoValue.");
            }
        }
    }
}
