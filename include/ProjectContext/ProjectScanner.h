#pragma once

#include <string>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

namespace AgentOS {

struct FileEntry {
    std::string path;
    std::string extension;
    size_t      sizeBytes;
    size_t      lineCount;
};

struct ScanSummary {
    int totalFiles;
    int totalDirs;
    int cppFiles;
    int headerFiles;
    int cmakeFiles;
    int markdownFiles;
    int otherFiles;
};

class ProjectScanner {
public:
    void        scan(const std::string& rootPath);
    
    const ScanSummary&       summary() const { return summary_; }
    const std::vector<FileEntry>& files() const { return files_; }
    const std::vector<std::string>& directories() const { return directories_; }

    std::string generateProjectMap() const;

private:
    ScanSummary            summary_{};
    std::vector<FileEntry> files_;
    std::vector<std::string> directories_;
    std::string            rootPath_;

    void scanDirectory(const fs::path& dir);
    void countFile(const fs::path& path);
    size_t countLines(const fs::path& path);
};

} // namespace AgentOS
