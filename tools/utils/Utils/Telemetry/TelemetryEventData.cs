// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

namespace Microsoft.Msix.Utils.Telemetry
{
    using System;
    using System.Diagnostics.Tracing;

    /// <summary>
    /// A class containing the event data types
    /// </summary>
    public class TelemetryEventData
    {
        /// <summary>
        /// Class that represents the event data for the SessionStart event
        /// </summary>
        [EventData]
        public class SessionStartEventData
        {
            /// <summary>
            /// Gets or sets the correlation id
            /// </summary>
            public Guid CorrelationId { get; set; }

            /// <summary>
            /// Gets or sets the time
            /// </summary>
            public long Time { get; set; }

            /// <summary>
            /// Gets or sets the command line args
            /// </summary>
            public string CommandLineArgs { get; set; }

            /// <summary>
            /// Gets or sets the version
            /// </summary>
            public string Version { get; set; }

            /// <summary>
            /// Gets or sets the OS version
            /// </summary>
            public string OsVersion { get; set; }
        }

        /// <summary>
        /// Class that represents the event data for the SessionEnd event
        /// </summary>
        [EventData]
        public class SessionEndEventData
        {
            /// <summary>
            /// Gets or sets the correlation id
            /// </summary>
            public Guid CorrelationId { get; set; }

            /// <summary>
            /// Gets or sets the time
            /// </summary>
            public long Time { get; set; }

            /// <summary>
            /// Gets or sets the exit code
            /// </summary>
            public int ExitCode { get; set; }
        }

        /// <summary>
        /// Class that represents the event data for the ExceptionThrown event
        /// </summary>
        [EventData]
        public class ExceptionThrownEventData
        {
            /// <summary>
            /// Gets or sets the correlation id
            /// </summary>
            public Guid CorrelationId { get; set; }

            /// <summary>
            /// Gets or sets the time
            /// </summary>
            public long Time { get; set; }

            /// <summary>
            /// Gets or sets the exception message
            /// </summary>
            public string Message { get; set; }

            /// <summary>
            /// Gets or sets the stack trace
            /// </summary>
            public string StackTrace { get; set; }

            /// <summary>
            /// Gets or sets the activity id
            /// </summary>
            public int? ActivityId { get; set; }
        }

        /// <summary>
        /// Class that represents the event data for an activity event
        /// </summary>
        /// <typeparam name="T">the type of the activity data object</typeparam>
        [EventData]
        public class ActivityEventData<T>
        {
            /// <summary>
            /// Gets or sets the correlation id
            /// </summary>
            public Guid CorrelationId { get; set; }

            /// <summary>
            /// Gets or sets the time
            /// </summary>
            public long Time { get; set; }

            /// <summary>
            /// Gets or sets the activity id
            /// </summary>
            public int ActivityId { get; set; }

            /// <summary>
            /// Gets or sets the activity data
            /// </summary>
            public T ActivityData { get; set; }
        }
    }
}
