// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

namespace Microsoft.Msix.Utils.CommandLine
{
    using System;
    using System.Collections.Generic;

    /// <summary>
    /// Base class wrapping around Microsoft.Extensions.CommandLineUtils.CommandOption and
    /// Microsoft.Extensions.CommandLineUtils.CommandArgument containing additional information
    /// for validation.
    /// </summary>
    public abstract class InputConfigurationBase
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="InputConfigurationBase"/> class.
        /// </summary>
        /// <param name="isRequired">A flag indicating whether the option is required</param>
        /// <param name="disallowedSwitches">A list of switches that are not allowed when this option is specified</param>
        /// <param name="requiredSwitches">A list of switches that are required when this option is specified</param>
        /// <param name="validationRoutineSingleValue">The validation routine for single value to be used to validate the value entered for this option</param>
        /// <param name="validationRoutineMultipleValue">The validation routine to be used to validate multiple values entered for this option</param>
        public InputConfigurationBase(
            bool isRequired,
            List<string> disallowedSwitches = null,
            List<string> requiredSwitches = null,
            Action<string> validationRoutineSingleValue = null,
            Action<List<string>> validationRoutineMultipleValue = null)
        {
            this.IsRequired = isRequired;
            this.DisallowedSwitches = disallowedSwitches == null ? new List<string>() : disallowedSwitches;
            this.RequiredSwitches = requiredSwitches == null ? new List<string>() : requiredSwitches;

            if (validationRoutineSingleValue != null && validationRoutineMultipleValue != null)
            {
                throw new InvalidOperationException("Input Arguments and Options don't allow define single and multiple value rutines at the same time.");
            }

            this.ValidationRoutineSingleValue = validationRoutineSingleValue;
            this.ValidationRoutineMultipleValues = validationRoutineMultipleValue;
        }

        /// <summary>
        /// Gets a value indicating whether this option is required
        /// </summary>
        public bool IsRequired { get; }

        /// <summary>
        /// Gets a list of all option switches that cannot be specified when this option is specified.
        /// </summary>
        public List<string> DisallowedSwitches { get; }

        /// <summary>
        /// Gets a list of all option switches that must be specified when this option is specified.
        /// </summary>
        public List<string> RequiredSwitches { get; }

        /// <summary>
        ///  Gets the function to validate the entry
        /// </summary>
        protected Action<string> ValidationRoutineSingleValue { get; }

        /// <summary>
        ///  Gets the function to validate the entry for MultipleValue
        /// </summary>
        protected Action<List<string>> ValidationRoutineMultipleValues { get; }

        /// <summary>
        /// Function who validate if the CommandOption or CommandArgument has value.
        /// </summary>
        /// <returns>Return true if the input has a value.</returns>
        public abstract bool HasValue();

        /// <summary>
        /// Function who return the value from CommandOption or CommandArgument.
        /// </summary>
        /// <returns>Return the input value</returns>
        public abstract string Value();

        /// <summary>
        /// Function that returns the values from CommandOption or CommandArgument.
        /// </summary>
        /// <returns>Return the input values</returns>
        public abstract List<string> Values();

        /// <summary>
        /// Function that validates the value specified for this option. Throws an exception for invalid entries.
        /// </summary>
        public abstract void Validate();
    }
}
