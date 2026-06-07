#include "ProjectContext/FileSummaryStore.h"
#include <sqlite3.h>
#include <filesystem>
#include <iostream>
#include <sstream>

namespace fs = std::filesystem;

// Convenience cast — avoids including sqlite3.h in the public header
#define DB (static_cast<sqlite3*>(db_))

namespace AgentOS {

// ─────────────────────────────────────────────────────────────────────────────
bool FileSummaryStore::open(const std::string& dbPath) {
    sqlite3* raw = nullptr;
    if (sqlite3_open(dbPath.c_str(), &raw) != SQLITE_OK) {
        std::cerr << "[FileSummaryStore] Cannot open DB: " << dbPath << "\n";
        return false;
    }
    db_ = raw;
    sqlite3_exec(DB, "PRAGMA journal_mode=WAL;",    nullptr, nullptr, nullptr);
    sqlite3_exec(DB, "PRAGMA synchronous=NORMAL;",  nullptr, nullptr, nullptr);
    createTables();
    return true;
}

void FileSummaryStore::close() {
    if (db_) { sqlite3_close(DB); db_ = nullptr; }
}

void FileSummaryStore::createTables() {
    const char* fileSql =
        "CREATE TABLE IF NOT EXISTS FileSummaries ("
        "  path     TEXT PRIMARY KEY,"
        "  summary  TEXT NOT NULL,"
        "  mod_time INTEGER NOT NULL"
        ");";
    sqlite3_exec(DB, fileSql, nullptr, nullptr, nullptr);

    const char* modSql =
        "CREATE TABLE IF NOT EXISTS ModuleSummaries ("
        "  module_path TEXT PRIMARY KEY,"
        "  module_name TEXT NOT NULL,"
        "  summary     TEXT NOT NULL"
        ");";
    sqlite3_exec(DB, modSql, nullptr, nullptr, nullptr);

    const char* projSql =
        "CREATE TABLE IF NOT EXISTS ProjectInfo ("
        "  id           INTEGER PRIMARY KEY CHECK (id = 1),"
        "  project_name TEXT NOT NULL,"
        "  architecture TEXT NOT NULL,"
        "  modules      TEXT NOT NULL"
        ");";
    sqlite3_exec(DB, projSql, nullptr, nullptr, nullptr);
}

// ─────────────────────────────────────────────────────────────────────────────
bool FileSummaryStore::isCached(const std::string& path, uint64_t modTime) const {
    if (!db_) return false;
    const char* sql =
        "SELECT mod_time FROM FileSummaries WHERE path = ? LIMIT 1;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(DB, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_text(stmt, 1, path.c_str(), -1, SQLITE_STATIC);
    bool found = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        uint64_t cached = static_cast<uint64_t>(sqlite3_column_int64(stmt, 0));
        found = (cached == modTime);
    }
    sqlite3_finalize(stmt);
    return found;
}

std::string FileSummaryStore::get(const std::string& path) const {
    if (!db_) return "";
    const char* sql =
        "SELECT summary FROM FileSummaries WHERE path = ? LIMIT 1;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(DB, sql, -1, &stmt, nullptr) != SQLITE_OK) return "";
    sqlite3_bind_text(stmt, 1, path.c_str(), -1, SQLITE_STATIC);
    std::string result;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* txt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        if (txt) result = txt;
    }
    sqlite3_finalize(stmt);
    return result;
}

void FileSummaryStore::put(const FileSummary& s) {
    if (!db_) return;
    const char* sql =
        "INSERT OR REPLACE INTO FileSummaries (path, summary, mod_time)"
        " VALUES (?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(DB, sql, -1, &stmt, nullptr) != SQLITE_OK) return;
    sqlite3_bind_text (stmt, 1, s.path.c_str(),    -1, SQLITE_STATIC);
    sqlite3_bind_text (stmt, 2, s.summary.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 3, static_cast<sqlite3_int64>(s.modTime));
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

std::vector<FileSummary> FileSummaryStore::getAll(
    const std::vector<std::string>& paths) const
{
    std::vector<FileSummary> result;
    if (!db_ || paths.empty()) return result;

    const char* sql =
        "SELECT path, summary, mod_time FROM FileSummaries WHERE path = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(DB, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return result;

    for (const auto& p : paths) {
        sqlite3_reset(stmt);
        sqlite3_bind_text(stmt, 1, p.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            FileSummary fs;
            const char* path = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            const char* summ = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            fs.path    = path ? path : "";
            fs.summary = summ ? summ : "";
            fs.modTime = static_cast<uint64_t>(sqlite3_column_int64(stmt, 2));
            if (!fs.path.empty()) result.push_back(std::move(fs));
        }
    }
    sqlite3_finalize(stmt);
    return result;
}

void FileSummaryStore::purge(const std::string& workspaceRoot) {
    if (!db_) return;
    // Collect all paths in DB
    const char* sql = "SELECT path FROM FileSummaries;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(DB, sql, -1, &stmt, nullptr) != SQLITE_OK) return;

    std::vector<std::string> toDelete;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* p = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        if (!p) continue;
        std::string path(p);
        // Delete if file no longer exists or is outside workspace
        if (!fs::exists(path) || path.find(workspaceRoot) == std::string::npos)
            toDelete.push_back(path);
    }
    sqlite3_finalize(stmt);

    const char* delSql = "DELETE FROM FileSummaries WHERE path = ?;";
    if (sqlite3_prepare_v2(DB, delSql, -1, &stmt, nullptr) == SQLITE_OK) {
        for (const auto& p : toDelete) {
            sqlite3_reset(stmt);
            sqlite3_bind_text(stmt, 1, p.c_str(), -1, SQLITE_STATIC);
            sqlite3_step(stmt);
        }
        sqlite3_finalize(stmt);
    }
    std::cerr << "[FileSummaryStore] Purged " << toDelete.size() << " stale entries\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// ModuleSummary CRUD
// ─────────────────────────────────────────────────────────────────────────────

bool FileSummaryStore::putModule(const ModuleSummary& ms) {
    if (!db_) return false;
    const char* sql =
        "INSERT OR REPLACE INTO ModuleSummaries (module_path, module_name, summary)"
        " VALUES (?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(DB, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_text(stmt, 1, ms.modulePath.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, ms.moduleName.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, ms.summary.c_str(),    -1, SQLITE_STATIC);
    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return ok;
}

ModuleSummary FileSummaryStore::getModule(const std::string& modulePath) const {
    ModuleSummary ms;
    if (!db_) return ms;
    const char* sql =
        "SELECT module_name, summary FROM ModuleSummaries WHERE module_path = ? LIMIT 1;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(DB, sql, -1, &stmt, nullptr) != SQLITE_OK) return ms;
    sqlite3_bind_text(stmt, 1, modulePath.c_str(), -1, SQLITE_STATIC);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        const char* summ = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        ms.modulePath = modulePath;
        ms.moduleName = name ? name : "";
        ms.summary    = summ ? summ : "";
    }
    sqlite3_finalize(stmt);
    return ms;
}

std::vector<ModuleSummary> FileSummaryStore::getAllModules() const {
    std::vector<ModuleSummary> result;
    if (!db_) return result;
    const char* sql = "SELECT module_path, module_name, summary FROM ModuleSummaries;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(DB, sql, -1, &stmt, nullptr) != SQLITE_OK) return result;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        ModuleSummary ms;
        const char* p = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        const char* n = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        const char* s = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        ms.modulePath = p ? p : "";
        ms.moduleName = n ? n : "";
        ms.summary    = s ? s : "";
        if (!ms.modulePath.empty()) result.push_back(std::move(ms));
    }
    sqlite3_finalize(stmt);
    return result;
}

// ─────────────────────────────────────────────────────────────────────────────
// ProjectSummary CRUD  (single-row table, id=1)
// ─────────────────────────────────────────────────────────────────────────────

bool FileSummaryStore::putProject(const ProjectSummary& ps) {
    if (!db_) return false;
    // Serialize modules vector as JSON array string
    std::string modulesJson = "[";
    for (size_t i = 0; i < ps.modules.size(); ++i) {
        if (i > 0) modulesJson += ",";
        modulesJson += "\"" + ps.modules[i] + "\"";
    }
    modulesJson += "]";

    const char* sql =
        "INSERT OR REPLACE INTO ProjectInfo (id, project_name, architecture, modules)"
        " VALUES (1, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(DB, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_text(stmt, 1, ps.projectName.c_str(),   -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, ps.architecture.c_str(),   -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, modulesJson.c_str(),       -1, SQLITE_STATIC);
    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return ok;
}

ProjectSummary FileSummaryStore::getProject() const {
    ProjectSummary ps;
    if (!db_) return ps;
    const char* sql =
        "SELECT project_name, architecture, modules FROM ProjectInfo WHERE id = 1;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(DB, sql, -1, &stmt, nullptr) != SQLITE_OK) return ps;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        const char* arch = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        const char* mods = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        ps.projectName  = name ? name : "";
        ps.architecture = arch ? arch : "";
        // Parse modules JSON array (simple comma-split between quotes)
        if (mods) {
            std::string m(mods);
            size_t pos = 0;
            while ((pos = m.find('"', pos)) != std::string::npos) {
                size_t end = m.find('"', pos + 1);
                if (end == std::string::npos) break;
                ps.modules.push_back(m.substr(pos + 1, end - pos - 1));
                pos = end + 1;
            }
        }
    }
    sqlite3_finalize(stmt);
    return ps;
}

} // namespace AgentOS
