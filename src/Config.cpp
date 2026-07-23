#include "VehicleNode.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>

namespace {
std::string trim(std::string value) {
    const auto notSpace = [](unsigned char c) { return !std::isspace(c); };
    value.erase(value.begin(), std::find_if(value.begin(), value.end(), notSpace));
    value.erase(std::find_if(value.rbegin(), value.rend(), notSpace).base(), value.end());
    return value;
}

int parsePositiveInt(const std::unordered_map<std::string, std::string>& values,
                     const std::string& key, int fallback, bool allowZero = false) {
    const auto entry = values.find(key);
    if (entry == values.end()) {
        return fallback;
    }
    try {
        std::size_t consumed = 0;
        const int parsed = std::stoi(entry->second, &consumed);
        if (consumed == entry->second.size() && (parsed > 0 || (allowZero && parsed == 0))) {
            return parsed;
        }
    } catch (const std::exception&) {
    }
    return fallback;
}
}  // namespace

bool Config::load(const std::filesystem::path& path) {
    std::ifstream input(path);
    if (!input) {
        return false;
    }

    std::unordered_map<std::string, std::string> values;
    std::string line;
    while (std::getline(input, line)) {
        line = trim(line);
        if (line.empty() || line.front() == '#') {
            continue;
        }
        const auto separator = line.find(':');
        if (separator == std::string::npos) {
            continue;
        }
        const std::string key = trim(line.substr(0, separator));
        const std::string value = trim(line.substr(separator + 1));
        if (!key.empty()) {
            values[key] = value;
        }
    }

    const auto setString = [&values](const std::string& key, std::string& target) {
        const auto entry = values.find(key);
        if (entry != values.end() && !entry->second.empty()) {
            target = entry->second;
        }
    };

    setString("device_name", deviceName);
    setString("platform_mode", platformMode);
    setString("vehicle_backend", vehicleBackend);
    setString("can_interface", canInterface);
    setString("fallback_can_interface", fallbackCanInterface);
    setString("diagnostic_mode", diagnosticMode);
    setString("software_version", softwareVersion);
    serviceIntervalSeconds = parsePositiveInt(values, "service_interval_seconds", serviceIntervalSeconds);
    return !input.bad();
}

void Config::print() const {
    std::cout << "Device Name: " << deviceName << '\n'
              << "Platform Mode: " << platformMode << '\n'
              << "Vehicle Backend: " << vehicleBackend << '\n'
              << "CAN Interface: " << canInterface << '\n'
              << "Fallback CAN Interface: " << fallbackCanInterface << '\n'
              << "Diagnostic Mode: " << diagnosticMode << '\n'
              << "Software Version: " << softwareVersion << '\n'
              << "Service Interval: " << serviceIntervalSeconds << " seconds\n";
}
