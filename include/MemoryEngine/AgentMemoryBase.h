#pragma once

#include <string>
#include <vector>
#include <map>
#include <chrono>

namespace AgentOS {

struct Message {
    std::string from;
    std::string to;
    std::string taskId; // Optional, to thread discussions around a specific task
    std::string content;
    std::chrono::system_clock::time_point timestamp;
};

struct ConversationThread {
    std::string conversationId; // TaskID or Topic
    std::vector<Message> messages;
};

struct ContextMemory {
    std::map<std::string, std::string> facts; 
};

struct RoleMemory {
    std::string roleName; 
    std::vector<std::string> allowedActions; 
};

struct FeedbackMemory {
    std::map<std::string, std::vector<std::string>> taskFeedback; // taskId -> comments
};

// Represents the brain's storage for a single agent
class AgentMemoryBase {
public:
    std::vector<ConversationThread> conversations;
    ContextMemory context;
    RoleMemory role;
    FeedbackMemory feedback;
    std::vector<Message> inbox; // raw chronological messages

    void storeMessage(const Message& msg) {
        inbox.push_back(msg);
        
        // Thread it if it has a taskId
        if (!msg.taskId.empty()) {
            bool found = false;
            for (auto& conv : conversations) {
                if (conv.conversationId == msg.taskId) {
                    conv.messages.push_back(msg);
                    found = true;
                    break;
                }
            }
            if (!found) {
                ConversationThread newConv;
                newConv.conversationId = msg.taskId;
                newConv.messages.push_back(msg);
                conversations.push_back(newConv);
            }
        }
    }
    
    void rememberFact(const std::string& key, const std::string& fact) {
        context.facts[key] = fact;
    }
};

} // namespace AgentOS
