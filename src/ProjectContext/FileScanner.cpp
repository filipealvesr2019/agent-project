#include "ProjectContext/FileScanner.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <set>

namespace AgentOS {

static const std::set<std::string> supportedExts = {
    ".txt", ".md", ".cpp", ".cxx", ".cc",
    ".h", ".hpp", ".hxx",
    ".json", ".yaml", ".yml", ".csv",
    ".py", ".ts", ".js", ".rs", ".java"
};

bool FileScanner::isSupported(const std::string& ext) {
    std::string lower = ext;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    return supportedExts.count(lower) > 0;
}

std::string FileScanner::readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "[FileScanner] Nao foi possivel abrir: " << path << "\n";
        return "";
    }

    std::string content((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());
    return content;
}

std::vector<std::string> FileScanner::readFiles(const std::vector<std::string>& filePaths) {
    std::vector<std::string> contents;
    for (const auto& path : filePaths) {
        std::string ext;
        auto dot = path.rfind('.');
        if (dot != std::string::npos) {
            ext = path.substr(dot);
        }

        if (!isSupported(ext)) {
            std::cerr << "[FileScanner] Extensao nao suportada: " << ext << " (" << path << ")\n";
            continue;
        }

        std::string content = readFile(path);
        if (!content.empty()) {
            contents.push_back(content);
        }
    }
    return contents;
}

} // namespace AgentOS
