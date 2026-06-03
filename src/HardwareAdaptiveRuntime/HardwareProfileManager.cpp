#include "HardwareAdaptiveRuntime/HardwareProfileManager.h"
#include "LocalRuntime/TaskScheduler.h"
#include "LocalRuntime/ModelPoolManager.h"
#include <iostream>
#include <thread>

#ifdef _WIN32
#include <windows.h>
#endif

namespace AgentOS {

HardwareProfileManager& HardwareProfileManager::getInstance() {
    static HardwareProfileManager instance;
    return instance;
}

void HardwareProfileManager::autoDetectAndApply() {
    profile_.cpuThreads = std::thread::hardware_concurrency();
    if (profile_.cpuThreads == 0) profile_.cpuThreads = 4;
    
    profile_.ramGB = 8; // Default fallback
    
#ifdef _WIN32
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&memInfo)) {
        profile_.ramGB = memInfo.ullTotalPhys / (1024 * 1024 * 1024);
    }
#endif

    // For demonstration, mock VRAM based on RAM (if RAM > 32, we assume it's a strong machine with VRAM)
    if (profile_.ramGB >= 32) {
        profile_.tier = HardwareTier::LARGE;
        profile_.vramGB = 24;
    } else if (profile_.ramGB >= 16) {
        profile_.tier = HardwareTier::MEDIUM;
        profile_.vramGB = 8;
    } else {
        profile_.tier = HardwareTier::SMALL;
        profile_.vramGB = 2; // Very limited VRAM
    }
    
    // Apply Settings
    if (profile_.tier == HardwareTier::LARGE) {
        TaskScheduler::getInstance().init(profile_.cpuThreads);
        ModelPoolManager::getInstance().setVramLimit(profile_.vramGB * 1000); 
    } else if (profile_.tier == HardwareTier::MEDIUM) {
        TaskScheduler::getInstance().init((std::max)(4, profile_.cpuThreads / 2));
        ModelPoolManager::getInstance().setVramLimit(profile_.vramGB * 1000);
    } else {
        // SMALL: Extreme constraints
        TaskScheduler::getInstance().init(2); // Only 2 concurrent inference threads to avoid OS freeze
        ModelPoolManager::getInstance().setVramLimit(2000); // 2 GB limit (1 small model max)
    }
}

HardwareProfile HardwareProfileManager::getProfile() const {
    return profile_;
}

void HardwareProfileManager::printProfile() const {
    std::cout << "\n=== AgentOS Hardware Profile ===\n";
    std::cout << "RAM: " << profile_.ramGB << " GB\n";
    std::cout << "Logical Cores: " << profile_.cpuThreads << "\n";
    std::cout << "Estimated VRAM: " << profile_.vramGB << " GB\n";
    std::cout << "Tier: " << (profile_.tier == HardwareTier::LARGE ? "LARGE" : 
                              (profile_.tier == HardwareTier::MEDIUM ? "MEDIUM" : "SMALL")) << "\n";
    std::cout << "================================\n\n";
}

} // namespace AgentOS
