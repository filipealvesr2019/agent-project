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

    sqlite3_exec(db, sqlTasks, 0, 0, 0);
    sqlite3_exec(db, sqlFiles, 0, 0, 0);
    sqlite3_exec(db, sqlConv, 0, 0, 0);

    sqlite3_close(db);
    return true;
}

void MemoryEngine::addTaskMemory(const TaskMemory& task) {
    sqlite3* db;
    sqlite3_open(m_dbPath.c_str(), &db);
    std::string sql = "INSERT OR REPLACE INTO Tasks(taskId, description, status, agentName) VALUES(" +
                      std::to_string(task.taskId) + ", '" + task.description + "', '" + task.status + "', '" + task.agentName + "');";
    sqlite3_exec(db, sql.c_str(), 0, 0, 0);
    sqlite3_close(db);
}

void MemoryEngine::updateTaskMemory(int taskId, const std::string& status) {
    sqlite3* db;
    sqlite3_open(m_dbPath.c_str(), &db);
    std::string sql = "UPDATE Tasks SET status='" + status + "' WHERE taskId=" + std::to_string(taskId) + ";";
    sqlite3_exec(db, sql.c_str(), 0, 0, 0);
    sqlite3_close(db);
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
    sqlite3* db;
    sqlite3_open(m_dbPath.c_str(), &db);
    std::string sql = "INSERT INTO Files(path, lastContent, lastModified) VALUES('" +
                      file.path + "', '" + file.lastContent + "', '" + file.lastModified + "');";
    sqlite3_exec(db, sql.c_str(), 0, 0, 0);
    sqlite3_close(db);
}

void MemoryEngine::updateFileMemory(const FileMemory& file) {
    sqlite3* db;
    sqlite3_open(m_dbPath.c_str(), &db);
    std::string sql = "UPDATE Files SET lastContent='" + file.lastContent + "', lastModified='" + file.lastModified + "' WHERE path='" + file.path + "';";
    sqlite3_exec(db, sql.c_str(), 0, 0, 0);
    sqlite3_close(db);
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
    sqlite3* db;
    sqlite3_open(m_dbPath.c_str(), &db);
    std::string sql = "INSERT INTO Conversations(agentName, prompt, response, timestamp) VALUES('" +
                      conv.agentName + "', '" + conv.prompt + "', '" + conv.response + "', '" + conv.timestamp + "');";
    sqlite3_exec(db, sql.c_str(), 0, 0, 0);
    sqlite3_close(db);
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

} // namespace AgentOS
