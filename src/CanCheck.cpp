#include "VehicleNode.hpp"

#include <cstdlib>
#include <sstream>
#include <string>
#include <unistd.h>

namespace {
bool executableOnPath(const std::string& command) {
    const char* pathValue = std::getenv("PATH");
    if (pathValue == nullptr) {
        return false;
    }

    std::stringstream paths(pathValue);
    std::string directory;
    while (std::getline(paths, directory, ':')) {
        if (directory.empty()) {
            directory = ".";
        }
        const std::filesystem::path candidate = std::filesystem::path(directory) / command;
        if (access(candidate.c_str(), X_OK) == 0) {
            return true;
        }
    }
    return false;
}

bool interfaceExists(const std::string& interfaceName) {
    if (interfaceName.empty() || interfaceName.find('/') != std::string::npos) {
        return false;
    }
    std::error_code error;
    return std::filesystem::exists(std::filesystem::path("/sys/class/net") / interfaceName,
                                   error);
}
}  // namespace

CanCheckResult CanCheck::run(const std::string& primaryInterface,
                             const std::string& fallbackInterface) {
    CanCheckResult result;
    result.primaryInterface = primaryInterface;
    result.fallbackInterface = fallbackInterface;
    result.primaryFound = interfaceExists(primaryInterface);
    result.fallbackFound = interfaceExists(fallbackInterface);
    result.ipFound = executableOnPath("ip");
    result.candumpFound = executableOnPath("candump");
    result.cansendFound = executableOnPath("cansend");
    result.canAvailable = result.primaryFound || result.fallbackFound;
    return result;
}
