#pragma once

#include <string>
#include <vector>

namespace AgentOS {

class FileScanner {
public:
    static std::vector<std::string> readFiles(const std::vector<std::string>& filePaths);

private:
    static std::string readFile(const std::string& path);
    static bool isSupported(const std::string& ext);
};

} // namespace AgentOS
