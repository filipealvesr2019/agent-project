#pragma once
#include <string>
#include "TaskType.h"

namespace AgentOS {

class TaskAnalyzer
{
public:
    TaskAnalyzer() = default;

    TaskType analyze(const std::string& prompt) const;
};

} // namespace AgentOS
