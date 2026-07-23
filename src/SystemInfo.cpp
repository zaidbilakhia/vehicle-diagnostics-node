#include "VehicleNode.hpp"

#include <fstream>
#include <sys/utsname.h>
#include <unistd.h>

SystemInfoResult SystemInfo::read() {
    SystemInfoResult result;

    char hostname[256]{};
    struct utsname kernelInfo {};
    std::ifstream uptime("/proc/uptime");

    const bool hostnameOk = gethostname(hostname, sizeof(hostname) - 1) == 0;
    const bool kernelOk = uname(&kernelInfo) == 0;
    const bool uptimeOk = static_cast<bool>(uptime >> result.uptimeSeconds);

    if (hostnameOk) {
        result.hostname = hostname;
    }
    if (kernelOk) {
        result.kernelVersion = kernelInfo.release;
    }
    result.readable = hostnameOk && kernelOk && uptimeOk;
    return result;
}
