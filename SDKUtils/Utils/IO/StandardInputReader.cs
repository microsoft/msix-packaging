//-----------------------------------------------------------------------
// <copyright file="StandardInputReader.cs" company="Microsoft">
//     Copyright (c) Microsoft. All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

namespace Microsoft.Packaging.SDKUtils.IO
{
    using System;
    using System.Collections.Generic;
    using System.Linq;

    /// <summary>
    /// Wrapper class around standard input to ease unit testing.
    /// </summary>
    public class StandardInputReader
    {
        private static List<char> injectedInput;

        /// <summary>
        /// Injects input.
        /// </summary>
        /// <param name="str">The input being injected</param>
        public static void InjectInput(string str)
        {
            if (injectedInput == null)
            {
                injectedInput = new List<char>();
            }

            injectedInput.AddRange(str.AsEnumerable());
        }

        /// <summary>
        /// Clears all injected input
        /// </summary>
        public static void ClearInjectedInput()
        {
            if (injectedInput != null)
            {
                injectedInput.Clear();
            }
        }

        /// <summary>
        /// Reads the next character (from standard input or injected input)
        /// </summary>
        /// <returns>the character read</returns>
        public static char GetNextChar()
        {
            if (injectedInput != null && injectedInput.Count > 0)
            {
                char injectedChar = injectedInput[0];
                injectedInput.RemoveAt(0);
                return injectedChar;
            }
            else
            {
                return Console.ReadKey().KeyChar;
            }
        }

        /// <summary>
        /// Reads the next character and returns it as upper case (from standard input or injected input)
        /// </summary>
        /// <returns>the character read</returns>
        public static char GetNextCharAsUpperCase()
        {
            return char.ToUpperInvariant(GetNextChar());
        }
    }
}
