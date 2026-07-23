#include "VehicleNode.hpp"

#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>

Logger::Logger(std::filesystem::path logPath) : logPath_(std::move(logPath)) {}

bool Logger::ensureReady() const {
    std::error_code error;
    std::filesystem::create_directories(logPath_.parent_path(), error);
    if (error) {
        std::cerr << "Failed to create log directory " << logPath_.parent_path()
                  << ": " << error.message() << '\n';
        return false;
    }
    std::ofstream output(logPath_, std::ios::app);
    if (!output.is_open()) {
        std::cerr << "Failed to open log file: " << logPath_ << '\n';
        return false;
    }
    output.close();
    return static_cast<bool>(output);
}

bool Logger::log(const std::string& level, const std::string& message) const {
    if (!ensureReady()) {
        return false;
    }
    const auto now = std::chrono::system_clock::now();
    const std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
    std::tm localTime{};
    localtime_r(&currentTime, &localTime);

    std::ofstream output(logPath_, std::ios::app);
    if (!output.is_open()) {
        std::cerr << "Failed to open log file for writing: " << logPath_ << '\n';
        return false;
    }
    output << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S") << " [" << level << "] "
           << message << '\n';
    output.flush();
    output.close();
    const bool success = static_cast<bool>(output);
    if (!success) {
        std::cerr << "Failed while writing log file: " << logPath_ << '\n';
    }
    return success;
}
