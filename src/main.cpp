#include "VehicleNode.hpp"

#include <iostream>
#include <string>

namespace {
void printHelp(const char* program) {
    std::cout << "Usage: " << program << " COMMAND\n\n"
              << "Commands:\n"
              << "  --service      Run continuously as a service\n"
              << "  --status       Print and write current vehicle status\n"
              << "  --health       Run health diagnostics\n"
              << "  --check-can    Check SocketCAN/vCAN availability\n"
              << "  --config       Print parsed configuration\n"
              << "  --paths        Print selected runtime paths\n"
              << "  --version      Print software version\n"
              << "  --help         Show this help\n";
}
}  // namespace

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printHelp(argv[0]);
        return argc == 1 ? 0 : 1;
    }

    const std::string command = argv[1];
    if (command == "--help") {
        printHelp(argv[0]);
        return 0;
    }

    VehicleNode node(Paths::select());
    const bool configLoaded = node.loadConfig();
    if (!configLoaded && command != "--version") {
        std::cerr << "Warning: configuration could not be loaded; using defaults\n";
    }

    if (command == "--version") {
        std::cout << "vehicle-node version " << node.config().softwareVersion << '\n';
        return 0;
    }
    if (command == "--config") {
        return node.runConfig();
    }
    if (command == "--paths") {
        return node.runPaths();
    }
    if (command == "--check-can") {
        return node.runCheckCan();
    }
    if (command == "--health") {
        return node.runHealth();
    }
    if (command == "--status") {
        return node.runStatus();
    }
    if (command == "--service") {
        return node.runService();
    }

    std::cerr << "Unknown command: " << command << '\n';
    printHelp(argv[0]);
    return 1;
}
