#include "Logger.h"

int main() {
    Logger& logger = Logger::getInstance();

    logger.setLogLevel(LogLevel::INFO);
    logger.setOutputFile("application.log");

    logger.log(LogLevel::INFO, "This is an info message.");
    logger.log(LogLevel::WARNING, "This is a warning message.");
    logger.log(LogLevel::ERROR, "This is an error message.");

    return 0;
}
