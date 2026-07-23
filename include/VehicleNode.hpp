#pragma once

#include <filesystem>
#include <string>

struct Paths {
    std::filesystem::path config;
    std::filesystem::path runtimeDirectory;
    std::filesystem::path log;
    std::filesystem::path status;
    bool embeddedMode{false};

    static Paths select();
};

class Config {
public:
    std::string deviceName{"vehicle-gateway-node-01"};
    std::string platformMode{"local"};
    std::string vehicleBackend{"socketcan"};
    std::string canInterface{"vcan0"};
    std::string fallbackCanInterface{"can0"};
    std::string diagnosticMode{"simulated"};
    std::string softwareVersion{"1.0.0"};
    int serviceIntervalSeconds{5};

    bool load(const std::filesystem::path& path);
    void print() const;
};

class Logger {
public:
    explicit Logger(std::filesystem::path logPath);
    bool ensureReady() const;
    bool log(const std::string& level, const std::string& message) const;

private:
    std::filesystem::path logPath_;
};

struct SystemInfoResult {
    std::string hostname;
    std::string kernelVersion;
    double uptimeSeconds{0.0};
    bool readable{false};
};

class SystemInfo {
public:
    static SystemInfoResult read();
};

struct CanCheckResult {
    std::string primaryInterface;
    std::string fallbackInterface;
    bool primaryFound{false};
    bool fallbackFound{false};
    bool ipFound{false};
    bool candumpFound{false};
    bool cansendFound{false};
    bool canAvailable{false};
};

class CanCheck {
public:
    static CanCheckResult run(const std::string& primaryInterface,
                              const std::string& fallbackInterface);
};

struct HealthResult {
    bool configReadable{false};
    bool runtimeWritable{false};
    bool logWritable{false};
    bool statusWritable{false};
    bool systemInfoReadable{false};
    bool canCheckExecutable{false};

    bool ok() const;
};

class VehicleNode {
public:
    explicit VehicleNode(Paths paths);

    bool loadConfig();
    int runStatus();
    int runHealth();
    int runCheckCan() const;
    int runConfig() const;
    int runPaths() const;
    int runService();
    bool writeStatusJson(const CanCheckResult& can, bool healthy) const;
    const Config& config() const;

private:
    HealthResult checkHealth() const;

    Paths paths_;
    Config config_;
    Logger logger_;
};
