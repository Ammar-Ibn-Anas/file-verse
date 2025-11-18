#include "../include/logger.hpp"
#include "../include/log_macros.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <vector>
#include <limits>
#include <stdexcept>

using namespace ofs;
using namespace std::literals;

#define TEST_MODULE "LOGGER_TEST"

void test_basic_logging()
{
    LOG_INFO(TEST_MODULE, 100, "Starting basic logging test.");
    LOG_DEBUG(TEST_MODULE, 101, "A debug message.");
    LOG_WARN(TEST_MODULE, 200, "A potential issue observed.");
    LOG_ERROR(TEST_MODULE, 300, "A non-critical operation failed.");
    LOG_INFO(TEST_MODULE, 102, "Basic logging test complete.");
}

void test_multithreaded_logging(int thread_id)
{
    for (int i = 0; i < 5; ++i)
    {
        std::string msg = "Thread " + std::to_string(thread_id) + " writing log entry " + std::to_string(i);
        LOG_INFO(TEST_MODULE, 150 + thread_id, msg);
        std::this_thread::sleep_for(10ms);
    }
}

void run_multithread_test()
{
    LOG_INFO(TEST_MODULE, 110, "Starting multithreaded logging test.");
    
    std::vector<std::thread> threads;
    for (int i = 0; i < 5; ++i)
    {
        threads.emplace_back(test_multithreaded_logging, i);
    }

    for (auto& t : threads)
    {
        t.join();
    }
    LOG_INFO(TEST_MODULE, 111, "Multithreaded logging test complete.");
}

void demonstrate_error_handling()
{
    LOG_INFO(TEST_MODULE, 120, "Interactive error demonstration started.");
    
    int num1, num2;
    std::cout << "Enter Numerator (integer): ";
    std::cin >> num1;
    
    // Clear input stream state after attempted read
    if (std::cin.fail()) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        LOG_ERROR(TEST_MODULE, 310, "Invalid input for numerator. Skipping division test.");
        return;
    }
    
    std::cout << "Enter Denominator (integer): ";
    std::cin >> num2;
    
    if (std::cin.fail()) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        LOG_ERROR(TEST_MODULE, 311, "Invalid input for denominator. Skipping division test.");
        return;
    }

    try
    {
        if (num2 == 0)
        {
            LOG_ERROR(TEST_MODULE, 400, "Attempted division by zero detected.");
            
            // Re-throw or handle as necessary. For this demonstration, we throw a custom exception.
            throw std::runtime_error("Division by zero operation blocked.");
        }
        
        int result = num1 / num2;
        LOG_INFO(TEST_MODULE, 121, "Division successful. Result=" + std::to_string(result));
    }
    catch (const std::exception& e)
    {
        // Catch the exception and log it at a critical level
        LOG_ERROR(TEST_MODULE, 500, "Exception caught during division: " + std::string(e.what()));
    }
}

// NOTE: We cannot safely test LOG_FATAL here as it calls std::exit() and terminates the program.

int main()
{
    std::cout << "--- Logger System Test ---\n";

    // Set a custom log file and small size for rotation test
    Logger::get_instance().set_log_file("./logs/test_logger.log");
    LOG_INFO(TEST_MODULE, 10, "Logger initialized with custom settings.");

    // Run automated tests
    test_basic_logging();
    run_multithread_test();
    
    std::cout << "\n--- Interactive Test Menu ---\n";
    std::cout << "1. Run Interactive Error Demo (Division)\n";
    std::cout << "2. Log a WARNING message\n";
    std::cout << "3. Log a FATAL message (will terminate the program)\n";
    std::cout << "4. Exit\n";
    
    int choice = 0;
    while (choice != 4)
    {
        std::cout << "\nEnter choice: ";
        std::cin >> choice;

        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input. Please enter a number.\n";
            continue;
        }

        switch (choice)
        {
            case 1:
                demonstrate_error_handling();
                break;
            case 2:
                LOG_WARN(TEST_MODULE, 210, "User-triggered warning.");
                break;
            case 3:
                std::cout << "Logging FATAL (Program will terminate now)...\n";
                // This call will terminate the process immediately.
                LOG_FATAL(TEST_MODULE, 999, "User-triggered fatal error.");
                break; // Unreachable
            case 4:
                std::cout << "Exiting interactive test.\n";
                break;
            default:
                std::cout << "Invalid choice.\n";
        }
    }
    
    std::cout << "\nTests finished. Check the ./logs/ directory for 'test_logger.log' and rotated files.\n";

    return 0;
}