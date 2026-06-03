#pragma once
#include <string>
#include <cstdint>

namespace AgentOS {

enum class HardwareTier {
    SMALL,
    MEDIUM,
    LARGE
};

struct HardwareProfile {
    uint64_t ramGB;
    uint64_t vramGB; // Mocked detection or retrieved via CUDA
    int cpuThreads;
    HardwareTier tier;
};

class HardwareProfileManager {
public:
    static HardwareProfileManager& getInstance();

    // Detect hardware automatically and apply limits to TaskScheduler and ModelPoolManager
    void autoDetectAndApply();

    HardwareProfile getProfile() const;
    void printProfile() const;

private:
    HardwareProfileManager() = default;
    ~HardwareProfileManager() = default;

    HardwareProfile profile_;
};

} // namespace AgentOS
