#pragma once

#include <string>
#include <vector>
#include <map>

namespace AgentOS {

enum class CommandType {
    BuildProject,
    RunTests,
    GitStatus,
    GenerateReport,
    Unknown
};

struct Command {
    CommandType type;
    std::string targetId;
    std::map<std::string, std::string> parameters;
};

class CommandValidator {
public:
    static bool validate(const Command& cmd) {
        if (cmd.type == CommandType::Unknown) return false;
        if (cmd.targetId.empty()) return false;
        
        // Directory traversal protection
        if (cmd.targetId.find("..") != std::string::npos || cmd.targetId.find("/") != std::string::npos) return false;
        if (cmd.targetId.find("\\") != std::string::npos) return false;
        
        return true;
    }
};

class Executor {
public:
    static bool executeCommand(const Command& cmd) {
        if (!CommandValidator::validate(cmd)) return false;
        
        // In a real system, the Executor maps the structured Command
        // to controlled OS APIs (e.g. execve) avoiding shell interpreters and injection.
        return true; 
    }
};

} // namespace AgentOS
