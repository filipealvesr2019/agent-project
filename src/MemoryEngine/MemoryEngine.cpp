#include "MemoryEngine/MemoryEngine.h"
#include <sqlite3.h>
#include <iostream>

namespace AgentOS {

MemoryEngine::MemoryEngine(const std::string& dbPath) : m_dbPath(dbPath) {}

bool MemoryEngine::initDatabase() {
    sqlite3* db;
    if (sqlite3_open(m_dbPath.c_str(), &db) != SQLITE_OK) {
        std::cerr << "Não foi possível abrir o banco de memória.\n";
        return false;
    }

    // --- Persistence Hardening: Enable WAL Mode ---
    sqlite3_exec(db, "PRAGMA journal_mode=WAL;", 0, 0, 0);
    sqlite3_exec(db, "PRAGMA synchronous=NORMAL;", 0, 0, 0);

    const char* sqlTasks = R"(
        CREATE TABLE IF NOT EXISTS Tasks(
            taskId INTEGER PRIMARY KEY,
            description TEXT,
            status TEXT,
            agentName TEXT
        );
    )";

    const char* sqlFiles = R"(
        CREATE TABLE IF NOT EXISTS Files(
            path TEXT PRIMARY KEY,
            lastContent TEXT,
            lastModified TEXT
        );
    )";

    const char* sqlConv = R"(
        CREATE TABLE IF NOT EXISTS Conversations(
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            agentName TEXT,
            prompt TEXT,
            response TEXT,
            timestamp TEXT
        );
    )";

    const char* sqlProfiles = R"(
        CREATE TABLE IF NOT EXISTS AgentLearningProfiles(
            agentId TEXT PRIMARY KEY,
            reliabilityScore REAL,
            decisionWeight REAL,
            overrideAdjustment REAL,
            totalVotes INTEGER,
            correctVotes INTEGER,
            wrongVotes INTEGER
        );
    )";

    const char* sqlHierarchy = R"(
        CREATE TABLE IF NOT EXISTS PersonaHierarchy(
            supervisorId TEXT,
            subordinateId TEXT,
            PRIMARY KEY(supervisorId, subordinateId)
        );
    )";

    sqlite3_exec(db, sqlTasks, 0, 0, 0);
    sqlite3_exec(db, sqlFiles, 0, 0, 0);
    sqlite3_exec(db, sqlConv, 0, 0, 0);
    sqlite3_exec(db, sqlProfiles, 0, 0, 0);
    sqlite3_exec(db, sqlHierarchy, 0, 0, 0);

    sqlite3_close(db);
    
    startPersisterThread();
    return true;
}

MemoryEngine::~MemoryEngine() {
    stopPersisterThread();
}

void MemoryEngine::queueSQL(const std::string& sql) {
    std::lock_guard<std::mutex> lock(queueMutex_);
    sqlQueue_.push(sql);
    cv_.notify_one();
}

void MemoryEngine::startPersisterThread() {
    if (running_) return;
    running_ = true;
    persisterThread_ = std::thread(&MemoryEngine::persisterLoop, this);
}

void MemoryEngine::stopPersisterThread() {
    if (!running_) return;
    running_ = false;
    cv_.notify_all();
    if (persisterThread_.joinable()) {
        persisterThread_.join();
    }
}

void MemoryEngine::persisterLoop() {
    sqlite3* db;
    if (sqlite3_open(m_dbPath.c_str(), &db) != SQLITE_OK) return;
    
    while (running_) {
        std::vector<std::string> batch;
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            cv_.wait_for(lock, std::chrono::milliseconds(100), [this]() {
                return !sqlQueue_.empty() || !running_;
            });
            
            while (!sqlQueue_.empty()) {
                batch.push_back(sqlQueue_.front());
                sqlQueue_.pop();
            }
        }
        
        if (!batch.empty()) {
            sqlite3_exec(db, "BEGIN TRANSACTION;", 0, 0, 0);
            for (const auto& sql : batch) {
                sqlite3_exec(db, sql.c_str(), 0, 0, 0);
            }
            sqlite3_exec(db, "COMMIT;", 0, 0, 0);
        }
    }
    sqlite3_close(db);
}

void MemoryEngine::addTaskMemory(const TaskMemory& task) {
    std::string sql = "INSERT OR REPLACE INTO Tasks(taskId, description, status, agentName) VALUES(" +
                      std::to_string(task.taskId) + ", '" + task.description + "', '" + task.status + "', '" + task.agentName + "');";
    queueSQL(sql);
}

void MemoryEngine::updateTaskMemory(int taskId, const std::string& status) {
    std::string sql = "UPDATE Tasks SET status='" + status + "' WHERE taskId=" + std::to_string(taskId) + ";";
    queueSQL(sql);
}

std::vector<TaskMemory> MemoryEngine::getAgentTasks(const std::string& agentName) {
    sqlite3* db;
    sqlite3_open(m_dbPath.c_str(), &db);
    sqlite3_stmt* stmt;
    std::vector<TaskMemory> tasks;

    std::string sql = "SELECT taskId, description, status FROM Tasks WHERE agentName='" + agentName + "';";
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            TaskMemory t;
            t.taskId = sqlite3_column_int(stmt, 0);
            t.description = (const char*)sqlite3_column_text(stmt, 1);
            t.status = (const char*)sqlite3_column_text(stmt, 2);
            t.agentName = agentName;
            tasks.push_back(t);
        }
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return tasks;
}

void MemoryEngine::addFileMemory(const FileMemory& file) {
    std::string sql = "INSERT INTO Files(path, lastContent, lastModified) VALUES('" +
                      file.path + "', '" + file.lastContent + "', '" + file.lastModified + "');";
    queueSQL(sql);
}

void MemoryEngine::updateFileMemory(const FileMemory& file) {
    std::string sql = "UPDATE Files SET lastContent='" + file.lastContent + "', lastModified='" + file.lastModified + "' WHERE path='" + file.path + "';";
    queueSQL(sql);
}

FileMemory MemoryEngine::getFileMemory(const std::string& path) {
    sqlite3* db;
    sqlite3_open(m_dbPath.c_str(), &db);
    sqlite3_stmt* stmt;
    FileMemory file;

    std::string sql = "SELECT path, lastContent, lastModified FROM Files WHERE path='" + path + "';";
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            file.path = (const char*)sqlite3_column_text(stmt, 0);
            file.lastContent = (const char*)sqlite3_column_text(stmt, 1);
            file.lastModified = (const char*)sqlite3_column_text(stmt, 2);
        }
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return file;
}

void MemoryEngine::addConversation(const ConversationMemory& conv) {
    std::string sql = "INSERT INTO Conversations(agentName, prompt, response, timestamp) VALUES('" +
                      conv.agentName + "', '" + conv.prompt + "', '" + conv.response + "', '" + conv.timestamp + "');";
    queueSQL(sql);
}

std::vector<ConversationMemory> MemoryEngine::getAgentConversations(const std::string& agentName) {
    sqlite3* db;
    sqlite3_open(m_dbPath.c_str(), &db);
    sqlite3_stmt* stmt;
    std::vector<ConversationMemory> convs;

    std::string sql = "SELECT agentName, prompt, response, timestamp FROM Conversations WHERE agentName='" + agentName + "';";
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            ConversationMemory c;
            c.agentName = (const char*)sqlite3_column_text(stmt, 0);
            c.prompt = (const char*)sqlite3_column_text(stmt, 1);
            c.response = (const char*)sqlite3_column_text(stmt, 2);
            c.timestamp = (const char*)sqlite3_column_text(stmt, 3);
            convs.push_back(c);
        }
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return convs;
}

void MemoryEngine::rollbackFileMemory(const std::string& path, int version) {
    // Mock rollback
    std::cout << "[MemoryEngine] Rolled back file " << path << " to version " << version << "\n";
}

void MemoryEngine::updateAgentState(const AgentStateMemory& state) {
    std::lock_guard<std::mutex> lock(stateMutex_);
    stateCache_[state.agentName] = state;
}

AgentStateMemory MemoryEngine::getAgentState(const std::string& agentName) {
    std::lock_guard<std::mutex> lock(stateMutex_);
    if (stateCache_.find(agentName) != stateCache_.end()) {
        return stateCache_[agentName];
    }
    AgentStateMemory state;
    state.agentName = agentName;
    state.state = "Idle";
    state.lastUpdate = "now";
    return state;
}

void MemoryEngine::updatePerformance(const AgentMetrics& metrics) {
    std::cout << "[MemoryEngine] Updated performance for " << metrics.agentName << "\n";
}

AgentMetrics MemoryEngine::getPerformance(const std::string& agentName) {
    AgentMetrics pm;
    pm.agentName = agentName;
    return pm;
}

void MemoryEngine::saveLearningProfile(const AgentLearningProfile& profile) {
    std::string sql = "INSERT OR REPLACE INTO AgentLearningProfiles(agentId, reliabilityScore, decisionWeight, overrideAdjustment, totalVotes, correctVotes, wrongVotes) VALUES('" +
                      profile.agentId + "', " + std::to_string(profile.reliabilityScore) + ", " +
                      std::to_string(profile.decisionWeight) + ", " + std::to_string(profile.overrideAdjustment) + ", " +
                      std::to_string(profile.totalVotes) + ", " + std::to_string(profile.correctVotes) + ", " + std::to_string(profile.wrongVotes) + ");";
    queueSQL(sql);
}

std::vector<AgentLearningProfile> MemoryEngine::loadLearningProfiles() {
    sqlite3* db;
    sqlite3_open(m_dbPath.c_str(), &db);
    sqlite3_stmt* stmt;
    std::vector<AgentLearningProfile> profiles;

    std::string sql = "SELECT agentId, reliabilityScore, decisionWeight, overrideAdjustment, totalVotes, correctVotes, wrongVotes FROM AgentLearningProfiles;";
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            AgentLearningProfile p;
            p.agentId = (const char*)sqlite3_column_text(stmt, 0);
            p.reliabilityScore = sqlite3_column_double(stmt, 1);
            p.decisionWeight = sqlite3_column_double(stmt, 2);
            p.overrideAdjustment = sqlite3_column_double(stmt, 3);
            p.totalVotes = sqlite3_column_int(stmt, 4);
            p.correctVotes = sqlite3_column_int(stmt, 5);
            p.wrongVotes = sqlite3_column_int(stmt, 6);
            profiles.push_back(p);
        }
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return profiles;
}

void MemoryEngine::saveHierarchyRelationship(const std::string& supervisorId, const std::string& subordinateId) {
    std::string sql = "INSERT OR IGNORE INTO PersonaHierarchy(supervisorId, subordinateId) VALUES('" + supervisorId + "', '" + subordinateId + "');";
    queueSQL(sql);
}

std::vector<PersonaRelationship> MemoryEngine::loadHierarchy() {
    sqlite3* db;
    sqlite3_open(m_dbPath.c_str(), &db);
    sqlite3_stmt* stmt;
    std::vector<PersonaRelationship> rels;

    std::string sql = "SELECT supervisorId, subordinateId FROM PersonaHierarchy;";
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            PersonaRelationship r;
            r.supervisorId = (const char*)sqlite3_column_text(stmt, 0);
            r.subordinateId = (const char*)sqlite3_column_text(stmt, 1);
            rels.push_back(r);
        }
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return rels;
}

} // namespace AgentOS
