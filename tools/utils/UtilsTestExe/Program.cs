//----------------------------------------------------------------------------------------------------------------------
// <copyright file="Program.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//----------------------------------------------------------------------------------------------------------------------

namespace UtilsTestExe
{
    using System;
    using System.Threading;

    class Program
    {
        // A minimal application to test the process runner.
        static void Main(string[] args)
        {
            Console.WriteLine("Process Starting");
            Thread.Sleep(millisecondsTimeout: 10 * 1000);
            Console.WriteLine("Process Ending");
        }
    }
}
