//-----------------------------------------------------------------------
// <copyright file="ArgumentConfiguration.cs" company="Microsoft">
//     Copyright (c) Microsoft. All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

namespace Microsoft.Packaging.Utils.CommandLine
{
    using System;
    using System.Collections.Generic;
    using Microsoft.Extensions.CommandLineUtils;

    /// <summary>
    /// Class wrapping around Microsoft.Extensions.CommandLineUtils.CommandArgument containing
    /// additional information for validation.
    /// </summary>
    public class ArgumentConfiguration : InputConfigurationBase
    {
        /// <summary>
        /// Initializes a new instance of the ArgumentConfiguration class for a non-required option (with single or multiple values).
        /// </summary>
        /// <param name="argument">The CommandArgument object</param>
        /// <param name="isRequired">A flag indicating whether the argument is required</param>
        /// <param name="disallowedSwitches">A list of switches that are not allowed when this argument is specified</param>
        /// <param name="requiredSwitches">A list of switches that are required when this argument is specified</param>
        /// <param name="validationRoutine">The validation routine to be used to validate the value entered for this argument</param>
        /// <param name="validationRoutineMultipleValue">The validation routine for multiple value to be used to validate the value entered for this argument</param>
        public ArgumentConfiguration(
            CommandArgument argument,
            bool isRequired = false,
            List<string> disallowedSwitches = null,
            List<string> requiredSwitches = null,
            Action<string> validationRoutine = null,
            Action<List<string>> validationRoutineMultipleValue = null) :
            base(isRequired, disallowedSwitches, requiredSwitches, validationRoutine, validationRoutineMultipleValue)
        {
            this.Argument = argument;
            this.InternvalValidateValidatorApplicability(validationRoutineMultipleValue);
        }

        /// <summary>
        /// Initializes a new instance of the ArgumentConfiguration class for a non-required option (with single or multiple values).
        /// </summary>
        /// <param name="argument">The CommandArgument object</param>
        /// <param name="disallowedSwitches">A list of switches that are not allowed when this argument is specified</param>
        /// <param name="requiredSwitches">A list of switches that are required when this argument is specified</param>
        /// <param name="validationRoutine">The validation routine to be used to validate the value entered for this argument</param>
        /// <param name="validationRoutineMultipleValue">The validation routine for multiple value to be used to validate the value entered for this argument</param>
        public ArgumentConfiguration(
            CommandArgument argument,
            List<string> disallowedSwitches = null,
            List<string> requiredSwitches = null,
            Action<string> validationRoutine = null,
            Action<List<string>> validationRoutineMultipleValue = null) :
            base(false, disallowedSwitches, requiredSwitches, validationRoutine, validationRoutineMultipleValue)
        {
            this.Argument = argument;
            this.InternvalValidateValidatorApplicability(validationRoutineMultipleValue);
        }

        /// <summary>
        /// Gets the CommandOption object
        /// </summary>
        public CommandArgument Argument { get; }

        public override bool HasValue()
        {
            return this.Argument.Value != null || this.Argument.Values.Count > 0;
        }

        public override string Value()
        {
            return this.Argument.Value;
        }

        public override List<string> Values()
        {
            return this.Argument.Values;
        }

        public override void Validate()
        {
            if (this.Argument.MultipleValues)
            {
                if (this.ValidationRoutineMultipleValues != null)
                {
                    this.ValidationRoutineMultipleValues(this.Argument.Values);
                }
                else if (this.ValidationRoutineSingleValue != null)
                {
                    foreach (string value in this.Argument.Values)
                    {
                        this.ValidationRoutineSingleValue(value);
                    }
                }
            }
            else if (this.ValidationRoutineSingleValue != null)
            {
                this.ValidationRoutineSingleValue(this.Argument.Value);
            }
        }

        private void InternvalValidateValidatorApplicability(Action<List<string>> validationRoutineMultipleValue)
        {
            if (this.Argument.MultipleValues == false && validationRoutineMultipleValue != null)
            {
                throw new InvalidOperationException(string.Format("Validation routine for multiple values is not allowed with Argument with single value."));
            }
        }
    }
}
