#include "ProjectContext/SymbolIndexStore.h"
#include <sqlite3.h>
#include <filesystem>
#include <iostream>
#include <sstream>

namespace fs = std::filesystem;

#define DB (static_cast<sqlite3*>(db_))

namespace AgentOS {

SymbolIndexStore::~SymbolIndexStore() {
    close();
}

bool SymbolIndexStore::open(const std::string& dbPath) {
    sqlite3* raw = nullptr;
    if (sqlite3_open(dbPath.c_str(), &raw) != SQLITE_OK) {
        std::cerr << "[SymbolIndexStore] Cannot open DB: " << dbPath << "\n";
        return false;
    }
    db_ = raw;
    sqlite3_exec(DB, "PRAGMA journal_mode=WAL;",   nullptr, nullptr, nullptr);
    sqlite3_exec(DB, "PRAGMA synchronous=NORMAL;", nullptr, nullptr, nullptr);
    createTables();
    return true;
}

void SymbolIndexStore::close() {
    if (db_) { sqlite3_close(DB); db_ = nullptr; }
}

void SymbolIndexStore::createTables() {
    const char* sql =
        "CREATE TABLE IF NOT EXISTS SymbolIndex ("
        "  id       INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  file     TEXT NOT NULL,"
        "  name     TEXT NOT NULL,"
        "  type     INTEGER NOT NULL,"
        "  line     INTEGER NOT NULL,"
        "  parent   TEXT NOT NULL DEFAULT '',"
        "  mod_time INTEGER NOT NULL"
        ");"
        "CREATE INDEX IF NOT EXISTS idx_sym_file ON SymbolIndex(file);"
        "CREATE INDEX IF NOT EXISTS idx_sym_name ON SymbolIndex(name);";
    sqlite3_exec(DB, sql, nullptr, nullptr, nullptr);
}

void SymbolIndexStore::putSymbols(const std::string& filePath, uint64_t modTime,
                                   const std::vector<SymbolEntry>& symbols) {
    if (!db_) return;

    // Delete old entries for this file
    const char* delSql = "DELETE FROM SymbolIndex WHERE file = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(DB, delSql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, filePath.c_str(), -1, SQLITE_STATIC);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    // Insert new entries
    const char* insSql =
        "INSERT INTO SymbolIndex (file, name, type, line, parent, mod_time)"
        " VALUES (?, ?, ?, ?, ?, ?);";
    if (sqlite3_prepare_v2(DB, insSql, -1, &stmt, nullptr) != SQLITE_OK) return;

    for (const auto& sym : symbols) {
        sqlite3_reset(stmt);
        sqlite3_bind_text (stmt, 1, filePath.c_str(),           -1, SQLITE_STATIC);
        sqlite3_bind_text (stmt, 2, sym.name.c_str(),           -1, SQLITE_STATIC);
        sqlite3_bind_int  (stmt, 3, static_cast<int>(sym.type));
        sqlite3_bind_int64(stmt, 4, static_cast<sqlite3_int64>(sym.line));
        sqlite3_bind_text (stmt, 5, sym.parentClass.c_str(),    -1, SQLITE_STATIC);
        sqlite3_bind_int64(stmt, 6, static_cast<sqlite3_int64>(modTime));
        sqlite3_step(stmt);
    }
    sqlite3_finalize(stmt);
}

bool SymbolIndexStore::isCached(const std::string& filePath, uint64_t modTime) const {
    if (!db_) return false;
    const char* sql = "SELECT mod_time FROM SymbolIndex WHERE file = ? LIMIT 1;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(DB, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_text(stmt, 1, filePath.c_str(), -1, SQLITE_STATIC);
    bool found = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        uint64_t cached = static_cast<uint64_t>(sqlite3_column_int64(stmt, 0));
        found = (cached == modTime);
    }
    sqlite3_finalize(stmt);
    return found;
}

static SymbolEntry rowToSymbol(sqlite3_stmt* stmt) {
    SymbolEntry sym;
    const char* f = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    const char* n = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    const char* p = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
    sym.file        = f ? f : "";
    sym.name        = n ? n : "";
    sym.type        = static_cast<SymbolType>(sqlite3_column_int(stmt, 2));
    sym.line        = static_cast<size_t>(sqlite3_column_int64(stmt, 3));
    sym.parentClass = p ? p : "";
    return sym;
}

std::vector<SymbolEntry> SymbolIndexStore::findSymbols(const std::string& query) const {
    std::vector<SymbolEntry> results;
    if (!db_) return results;

    const char* sql =
        "SELECT file, name, type, line, parent FROM SymbolIndex"
        " WHERE name LIKE ? COLLATE NOCASE"
        " ORDER BY file, line;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(DB, sql, -1, &stmt, nullptr) != SQLITE_OK) return results;

    std::string pattern = "%" + query + "%";
    sqlite3_bind_text(stmt, 1, pattern.c_str(), -1, SQLITE_STATIC);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        results.push_back(rowToSymbol(stmt));
    }
    sqlite3_finalize(stmt);
    return results;
}

std::vector<SymbolEntry> SymbolIndexStore::findSymbolsInFile(const std::string& filePath) const {
    std::vector<SymbolEntry> results;
    if (!db_) return results;

    const char* sql =
        "SELECT file, name, type, line, parent FROM SymbolIndex"
        " WHERE file = ?"
        " ORDER BY line;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(DB, sql, -1, &stmt, nullptr) != SQLITE_OK) return results;
    sqlite3_bind_text(stmt, 1, filePath.c_str(), -1, SQLITE_STATIC);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        results.push_back(rowToSymbol(stmt));
    }
    sqlite3_finalize(stmt);
    return results;
}

std::vector<std::string> SymbolIndexStore::findRelevantFiles(const std::string& query) const {
    std::vector<std::string> results;
    if (!db_) return results;

    const char* sql =
        "SELECT DISTINCT file FROM SymbolIndex"
        " WHERE name LIKE ? COLLATE NOCASE"
        " ORDER BY file;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(DB, sql, -1, &stmt, nullptr) != SQLITE_OK) return results;

    std::string pattern = "%" + query + "%";
    sqlite3_bind_text(stmt, 1, pattern.c_str(), -1, SQLITE_STATIC);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* f = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        if (f) results.push_back(f);
    }
    sqlite3_finalize(stmt);
    return results;
}

void SymbolIndexStore::purge(const std::string& workspaceRoot) {
    if (!db_) return;

    const char* selSql = "SELECT DISTINCT file FROM SymbolIndex;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(DB, selSql, -1, &stmt, nullptr) != SQLITE_OK) return;

    std::vector<std::string> toDelete;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* f = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        if (!f) continue;
        std::string path(f);
        if (!fs::exists(path) || path.find(workspaceRoot) == std::string::npos)
            toDelete.push_back(path);
    }
    sqlite3_finalize(stmt);

    const char* delSql = "DELETE FROM SymbolIndex WHERE file = ?;";
    if (sqlite3_prepare_v2(DB, delSql, -1, &stmt, nullptr) == SQLITE_OK) {
        for (const auto& p : toDelete) {
            sqlite3_reset(stmt);
            sqlite3_bind_text(stmt, 1, p.c_str(), -1, SQLITE_STATIC);
            sqlite3_step(stmt);
        }
        sqlite3_finalize(stmt);
    }
    std::cerr << "[SymbolIndexStore] Purged " << toDelete.size() << " stale files\n";
}

} // namespace AgentOS
