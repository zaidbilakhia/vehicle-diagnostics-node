#include "VehicleNode.hpp"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <thread>

namespace {
volatile std::sig_atomic_t keepRunning = 1;

void stopService(int) {
    keepRunning = 0;
}

std::string foundText(bool value) {
    return value ? "FOUND" : "NOT FOUND";
}

std::string okFail(bool value) {
    return value ? "OK" : "FAIL";
}

std::string displayBackend(const std::string& backend) {
    std::string normalized = backend;
    std::transform(normalized.begin(), normalized.end(), normalized.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return normalized == "socketcan" ? "SocketCAN" : backend;
}

std::string jsonEscape(const std::string& value) {
    std::string escaped;
    for (const unsigned char c : value) {
        switch (c) {
            case '"': escaped += "\\\""; break;
            case '\\': escaped += "\\\\"; break;
            case '\b': escaped += "\\b"; break;
            case '\f': escaped += "\\f"; break;
            case '\n': escaped += "\\n"; break;
            case '\r': escaped += "\\r"; break;
            case '\t': escaped += "\\t"; break;
            default:
                if (c < 0x20) {
                    constexpr char hex[] = "0123456789abcdef";
                    escaped += "\\u00";
                    escaped += hex[(c >> 4) & 0x0f];
                    escaped += hex[c & 0x0f];
                } else {
                    escaped += static_cast<char>(c);
                }
        }
    }
    return escaped;
}

bool canWriteDirectory(const std::filesystem::path& directory) {
    std::error_code error;
    std::filesystem::create_directories(directory, error);
    if (error) {
        return false;
    }

    const auto testPath = directory / ".vehicle-node-write-test.tmp";
    std::ofstream test(testPath, std::ios::trunc);
    if (!test.is_open()) {
        return false;
    }
    test << "test\n";
    test.flush();
    test.close();
    const bool success = static_cast<bool>(test);
    std::filesystem::remove(testPath, error);
    return success;
}

bool canWriteStatus(const std::filesystem::path& statusPath) {
    const bool existed = std::filesystem::exists(statusPath);
    std::ofstream output(statusPath, std::ios::app);
    const bool success = output.is_open();
    output.close();
    if (!existed) {
        std::error_code error;
        std::filesystem::remove(statusPath, error);
    }
    return success;
}

bool pathIsUnder(const std::filesystem::path& path, const std::filesystem::path& directory) {
    const auto relative = path.lexically_normal().lexically_relative(directory.lexically_normal());
    return !relative.empty() && relative != "." && *relative.begin() != "..";
}
}  // namespace

Paths Paths::select() {
    Paths paths;
    const char* configOverride = std::getenv("VEHICLE_NODE_CONFIG");
    const bool hasConfigOverride = configOverride != nullptr && *configOverride != '\0';
    const std::filesystem::path embeddedDirectory("/etc/vehicle-node");
    const std::filesystem::path embeddedConfig = embeddedDirectory / "vehicle-node.yaml";

    if (hasConfigOverride) {
        paths.config = configOverride;
    } else if (std::filesystem::exists(embeddedConfig)) {
        paths.config = embeddedConfig;
    } else {
        paths.config = "config/vehicle-node.yaml";
    }
    paths.embeddedMode = pathIsUnder(paths.config, embeddedDirectory);

    const char* runtimeOverride = std::getenv("VEHICLE_NODE_RUNTIME_DIR");
    const bool hasRuntimeOverride = runtimeOverride != nullptr && *runtimeOverride != '\0';
    if (hasRuntimeOverride) {
        paths.runtimeDirectory = runtimeOverride;
        paths.log = paths.runtimeDirectory / "vehicle-node.log";
    } else if (paths.embeddedMode) {
        paths.runtimeDirectory = "/var/lib/vehicle-node";
        paths.log = "/var/log/vehicle-node/vehicle-node.log";
    } else {
        paths.runtimeDirectory = "runtime";
        paths.log = paths.runtimeDirectory / "vehicle-node.log";
    }
    paths.status = paths.runtimeDirectory / "status.json";
    return paths;
}

bool HealthResult::ok() const {
    return configReadable && runtimeWritable && logWritable && statusWritable &&
           systemInfoReadable && canCheckExecutable;
}

VehicleNode::VehicleNode(Paths paths) : paths_(std::move(paths)), logger_(paths_.log) {}

bool VehicleNode::loadConfig() {
    config_ = Config{};
    return config_.load(paths_.config);
}

const Config& VehicleNode::config() const {
    return config_;
}

HealthResult VehicleNode::checkHealth() const {
    HealthResult result;
    std::ifstream configFile(paths_.config);
    result.configReadable = static_cast<bool>(configFile);
    result.runtimeWritable = canWriteDirectory(paths_.runtimeDirectory);
    result.logWritable = logger_.ensureReady();
    result.statusWritable = result.runtimeWritable && canWriteStatus(paths_.status);
    result.systemInfoReadable = SystemInfo::read().readable;
    (void)CanCheck::run(config_.canInterface, config_.fallbackCanInterface);
    result.canCheckExecutable = true;
    return result;
}

int VehicleNode::runCheckCan() const {
    const CanCheckResult can = CanCheck::run(config_.canInterface, config_.fallbackCanInterface);
    std::cout << "Vehicle Backend: " << displayBackend(config_.vehicleBackend) << '\n'
              << "Primary CAN Interface: " << can.primaryInterface << '\n'
              << "/sys/class/net/" << can.primaryInterface << ": "
              << foundText(can.primaryFound) << '\n'
              << "Fallback CAN Interface: " << can.fallbackInterface << '\n'
              << "/sys/class/net/" << can.fallbackInterface << ": "
              << foundText(can.fallbackFound) << '\n'
              << "ip command: " << foundText(can.ipFound) << '\n'
              << "candump: " << foundText(can.candumpFound) << '\n'
              << "cansend: " << foundText(can.cansendFound) << '\n'
              << "CAN Status: " << (can.canAvailable ? "Available" : "Not Available") << '\n';
    return 0;
}

int VehicleNode::runConfig() const {
    config_.print();
    return 0;
}

int VehicleNode::runPaths() const {
    std::cout << "Config Path: " << paths_.config.string() << '\n'
              << "Log Path: " << paths_.log.string() << '\n'
              << "Status Path: " << paths_.status.string() << '\n'
              << "Mode: " << (paths_.embeddedMode ? "embedded" : "local") << '\n';
    return 0;
}

int VehicleNode::runHealth() {
    const HealthResult health = checkHealth();
    std::cout << "Config: " << okFail(health.configReadable) << '\n'
              << "Runtime path: " << okFail(health.runtimeWritable) << '\n'
              << "Log file: " << okFail(health.logWritable) << '\n'
              << "Status file: " << okFail(health.statusWritable) << '\n'
              << "System info: " << okFail(health.systemInfoReadable) << '\n'
              << "CAN check: " << okFail(health.canCheckExecutable) << '\n'
              << "Overall Health: " << okFail(health.ok()) << '\n';
    return health.ok() ? 0 : 1;
}

bool VehicleNode::writeStatusJson(const CanCheckResult& can, bool healthy) const {
    std::error_code error;
    std::filesystem::create_directories(paths_.status.parent_path(), error);
    if (error) {
        std::cerr << "Failed to create status directory " << paths_.status.parent_path()
                  << ": " << error.message() << '\n';
        return false;
    }

    std::ofstream output(paths_.status, std::ios::out | std::ios::trunc);
    if (!output.is_open()) {
        std::cerr << "Failed to open status file for writing: " << paths_.status << '\n';
        return false;
    }
    output << "{\n"
           << "  \"device_name\": \"" << jsonEscape(config_.deviceName) << "\",\n"
           << "  \"platform_mode\": \"" << jsonEscape(config_.platformMode) << "\",\n"
           << "  \"vehicle_backend\": \"" << jsonEscape(config_.vehicleBackend) << "\",\n"
           << "  \"can_interface\": \"" << jsonEscape(config_.canInterface) << "\",\n"
           << "  \"can_available\": " << (can.canAvailable ? "true" : "false") << ",\n"
           << "  \"health\": \"" << (healthy ? "OK" : "FAIL") << "\",\n"
           << "  \"diagnostic_mode\": \"" << jsonEscape(config_.diagnosticMode) << "\",\n"
           << "  \"software_version\": \"" << jsonEscape(config_.softwareVersion) << "\"\n"
           << "}\n";
    output.flush();
    output.close();
    const bool success = static_cast<bool>(output);
    if (!success) {
        std::cerr << "Failed while writing status file: " << paths_.status << '\n';
    }
    return success;
}

int VehicleNode::runStatus() {
    const CanCheckResult can = CanCheck::run(config_.canInterface, config_.fallbackCanInterface);
    const HealthResult health = checkHealth();
    const bool statusWritten = writeStatusJson(can, health.ok());
    std::cout << "Device: " << config_.deviceName << '\n'
              << "Mode: " << config_.platformMode << '\n'
              << "Vehicle Backend: " << displayBackend(config_.vehicleBackend) << '\n'
              << "CAN Interface: " << config_.canInterface << '\n'
              << "CAN Status: " << (can.canAvailable ? "Available" : "Not Available") << '\n'
              << "Health: " << (health.ok() ? "OK" : "FAIL") << '\n'
              << "Version: " << config_.softwareVersion << '\n';
    if (!statusWritten) {
        std::cerr << "Failed to write status file: " << paths_.status << '\n';
        return 1;
    }
    return health.ok() ? 0 : 1;
}

int VehicleNode::runService() {
    keepRunning = 1;
    std::signal(SIGINT, stopService);
    std::signal(SIGTERM, stopService);
    std::cout << "vehicle-node service starting\n";

    while (keepRunning) {
        loadConfig();
        const HealthResult health = checkHealth();
        const CanCheckResult can = CanCheck::run(config_.canInterface, config_.fallbackCanInterface);
        const bool statusWritten = writeStatusJson(can, health.ok());
        const bool logWritten = logger_.log(
            "INFO", "vehicle-node service cycle completed: health=" +
                        std::string(health.ok() ? "OK" : "FAIL") + " can_available=" +
                        (can.canAvailable ? "true" : "false"));
        if (!statusWritten || !logWritten) {
            std::cerr << "vehicle-node service cycle could not write runtime output\n";
        }

        const int interval = std::max(1, config_.serviceIntervalSeconds);
        for (int elapsed = 0; elapsed < interval * 10 && keepRunning; ++elapsed) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    logger_.log("INFO", "vehicle-node service stopped");
    std::cout << "vehicle-node service shutting down\n";
    return 0;
}
