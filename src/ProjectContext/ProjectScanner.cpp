#include "ProjectContext/ProjectScanner.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <set>
#include <map>

namespace AgentOS {

void ProjectScanner::scan(const std::string& rootPath) {
    rootPath_ = rootPath;
    summary_ = {};
    files_.clear();
    directories_.clear();

    fs::path root(rootPath);
    if (!fs::exists(root) || !fs::is_directory(root)) {
        std::cerr << "[ProjectScanner] Invalid root: " << rootPath << "\n";
        return;
    }

    std::set<std::string> ignoreDirs = {
        ".git", "build", "build_vs", "build_cli", "build_release", "build_check",
        ".vs", ".opencode", "libs", "node_modules", "__pycache__", ".cache"
    };

    std::set<std::string> ignoreExts = {
        ".exe", ".dll", ".lib", ".pdb", ".obj", ".o", ".a", ".so",
        ".png", ".jpg", ".jpeg", ".gif", ".ico", ".svg",
        ".ttf", ".otf", ".wav", ".mp3",
        ".bin"
    };

    try {
        fs::recursive_directory_iterator it(root);
        fs::recursive_directory_iterator end;
        while (it != end) {
            const auto& entry = *it;
            if (entry.is_directory()) {
                std::string dirName = entry.path().filename().string();
                if (ignoreDirs.count(dirName)) {
                    it.disable_recursion_pending();
                }
                ++it;
                continue;
            }

            if (entry.is_regular_file()) {
                std::string ext = entry.path().extension().string();
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

                if (ignoreExts.count(ext)) {
                    ++it;
                    continue;
                }

                countFile(entry.path());
            }
            ++it;
        }
    } catch (const std::exception& e) {
        std::cerr << "[ProjectScanner] Error scanning: " << e.what() << "\n";
    }
}

void ProjectScanner::countFile(const fs::path& path) {
    std::string ext = path.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    FileEntry entry;
    entry.path = path.string();
    entry.extension = ext;
    entry.sizeBytes = fs::file_size(path);
    entry.lineCount = countLines(path);

    if (ext == ".cpp" || ext == ".cxx" || ext == ".cc") {
        summary_.cppFiles++;
    } else if (ext == ".h" || ext == ".hpp" || ext == ".hxx") {
        summary_.headerFiles++;
    } else if (ext == ".cmake" || ext == ".txt" || path.filename() == "CMakeLists.txt") {
        summary_.cmakeFiles++;
    } else if (ext == ".md" || ext == ".markdown") {
        summary_.markdownFiles++;
    } else {
        summary_.otherFiles++;
    }

    files_.push_back(entry);
    summary_.totalFiles++;
}

size_t ProjectScanner::countLines(const fs::path& path) {
    std::error_code ec;
    auto size = fs::file_size(path, ec);
    if (ec || size == 0) return 0;

    std::ifstream file(path);
    if (!file.is_open()) return 0;

    size_t lines = 0;
    std::string line;
    while (std::getline(file, line)) {
        lines++;
    }
    return lines;
}

std::string ProjectScanner::generateProjectMap() const {
    std::string map;
    map += "Projeto: AgentOS\n";
    map += "Linguagem: C++20\n";
    map += "Framework: JUCE\n\n";

    map += "Resumo:\n";
    map += "  Arquivos: " + std::to_string(summary_.totalFiles) + "\n";
    map += "  C++: " + std::to_string(summary_.cppFiles) + "\n";
    map += "  Headers: " + std::to_string(summary_.headerFiles) + "\n";
    map += "  CMake: " + std::to_string(summary_.cmakeFiles) + "\n";
    map += "  Docs: " + std::to_string(summary_.markdownFiles) + "\n";
    map += "  Outros: " + std::to_string(summary_.otherFiles) + "\n\n";

    std::string root = rootPath_;
    std::map<std::string, std::vector<std::string>> dirMap;
    for (const auto& f : files_) {
        auto pos = f.path.rfind('\\');
        if (pos == std::string::npos) pos = f.path.rfind('/');
        std::string dir;
        if (pos != std::string::npos) {
            dir = f.path.substr(0, pos);
            if (dir.substr(0, root.length()) == root && root.length() + 1 <= dir.size()) {
                dir = dir.substr(root.length() + 1);
            }
        }
        std::string fn = (pos != std::string::npos) ? f.path.substr(pos + 1) : f.path;
        dirMap[dir].push_back(fn);
    }

    map += "Estrutura:\n";
    for (const auto& [dir, fileList] : dirMap) {
        if (dir.empty()) continue;
        int depth = 0;
        for (char c : dir) {
            if (c == '/' || c == '\\') depth++;
        }

        std::string indent(depth * 2, ' ');
        map += indent + dir + "/\n";

        std::string fileIndent((depth + 1) * 2, ' ');
        for (const auto& f : fileList) {
            map += fileIndent + f + "\n";
        }
    }

    return map;
}

} // namespace AgentOS
