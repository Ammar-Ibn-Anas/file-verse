Omni File System (OFS) Logging Standard

The OFS logging system is designed for high-performance, thread-safe, structured logging to ensure easy parsing, auditing, and concurrent debugging across all modules.

1. Core Principles

1.1 Centralized Output

Log Destination: All log entries must be written exclusively to the single, designated file: ./logs/ofs.log.

Usage Mandate: All logging must be performed via the provided C++ macros (LOG_INFO, LOG_ERROR, etc.) which interface with the ofs::Logger singleton. Direct use of standard streams (std::cout, std::cerr) for system events is strictly forbidden.

Thread Safety: The ofs::Logger is implemented as a thread-safe singleton, guaranteeing integrity during concurrent writes from multiple server threads.

1.2 Log Rotation

The log file automatically manages its size to prevent disk overflow.

Rotation Size: The log file will be rotated (archived and restarted) when its size exceeds 1 MiB (1048576 bytes).

Archiving: Old log files are renamed with a high-precision UTC timestamp suffix: ofs.log.YYYY-MM-DDTHH:MM:SS.mmmZ.old.

2. Structured Log Format

Every log entry must adhere to the following strict, structured, space-separated key-value format.

Mandatory Format:

[TIMESTAMP] level=[LEVEL] module=[MODULE_NAME] code=[CODE] msg="[MESSAGE]" file="[SOURCE_FILE]" line=[LINE_NUMBER]


Mandatory Log Components:

+-----------+------------+-------------------------------------------------+---------------------------------+
| Component | Type       | Description                                     | Standard                        |
+===========+============+=================================================+=================================+
| level     | LogLevel   | The severity of the event. (Handled by macro)   | See Section 3.2.                |
| module    | std::string| A unique, uppercase name identifying the source | MANDATORY (See Section 3.1)     |
|           |            | component.                                      |                                 |
| code      | int        | A unique numeric ID for the specific event type.| MANDATORY (See Section 3.2)     |
| msg       | std::string| The descriptive message for the event.          | Keep concise and informative.   |
| file      | const char*| The source filename where the log occurred.     | Auto-filled by `__FILE__`.      |
| line      | int        | The line number in the source file.             | Auto-filled by `__LINE__`.      |
+-----------+------------+-------------------------------------------------+---------------------------------+


3. Implementation and Usage

3.1 Module Naming Convention

Each C++ file that logs data must define a unique, descriptive, uppercase module name using a preprocessor directive, typically near the top of the .cpp file:

#define MODULE_NAME "UCONF_PARSER"


3.2 Standard Log Levels and Codes

Log severity must be selected carefully to reflect the impact of the event. Unique integer codes are used to categorize events.

Standard Log Levels:

+-----------+----------+-------------------------------------------------------------------------------------------------------------------------------------------------+------------------+
| Level     | Severity | Purpose                                                                                                                                         | Macro            |
+===========+==========+=================================================================================================================================================+==================+
| FATAL     | Highest  | System-ending, unrecoverable failures. Triggers `std::exit(code)`.                                                                                | LOG_FATAL        |
| ERROR     | High     | Critical failures (e.g., file I/O error, failed memory operation). Prevents an operation from completing.                                         | LOG_ERROR        |
| WARN      | Medium   | Non-critical issues or exceptions (e.g., falling back to a default, minor invalid user input).                                                    | LOG_WARN         |
| INFO      | Low      | Key operational milestones (e.g., successful system initialization, connection accepted, configuration loaded).                                   | LOG_INFO         |
| DEBUG     | Lowest   | Detailed information for deep development tracing (e.g., variable values, function entry/exit).                                                   | LOG_DEBUG        |
+-----------+----------+-------------------------------------------------------------------------------------------------------------------------------------------------+------------------+


Standard Error Code Ranges:

+-----------+--------------------------------+------------------------------------------------------------------------------------------------+
| Range     | Event Type                     | Example Codes                                                                                  |
+===========+================================+================================================================================================+
| 1-99      | INFO & DEBUG                   | (10) Startup, (11) Config Load Success, (50) Calculation Result                                |
| 100-299   | WARN                           | (201) Block Alignment Warning, (202) Invalid User Input                                        |
| 300-499   | ERROR                          | (301) File Open Failure, (401) Validation Structural Failure                                   |
| 500+      | FATAL                          | (501) Mutex Lock Failure, (502) Unhandled Signal                                               |
+-----------+--------------------------------+------------------------------------------------------------------------------------------------+
