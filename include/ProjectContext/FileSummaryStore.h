#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace AgentOS {

// ─────────────────────────────────────────────────────────────────────────────
// Level 2 — file summary
// Generated once by the LLM (128-token response), cached by mod_time.
// ─────────────────────────────────────────────────────────────────────────────
struct FileSummary {
    std::string path;       // absolute path
    std::string summary;    // LLM-generated 1–3 sentence description
    uint64_t    modTime;    // last_write_time when summary was generated
};

// ─────────────────────────────────────────────────────────────────────────────
// Level 1 — module summary (group of files under a directory)
// ─────────────────────────────────────────────────────────────────────────────
struct ModuleSummary {
    std::string moduleName; // e.g. "ProjectContext"
    std::string modulePath; // absolute directory path
    std::string summary;    // derived from file summaries in that directory
};

// ─────────────────────────────────────────────────────────────────────────────
// Level 0 — project summary (derived from all modules)
// ─────────────────────────────────────────────────────────────────────────────
struct ProjectSummary {
    std::string projectName;    // directory name of workspace root
    std::string architecture;   // brief description of tech stack
    std::vector<std::string> modules; // list of top-level module names
};

// ─────────────────────────────────────────────────────────────────────────────
// FileSummaryStore — SQLite-backed persistence for FileSummary objects.
// One row per file.  Lookup by path; invalidated when modTime changes.
// ─────────────────────────────────────────────────────────────────────────────
class FileSummaryStore {
public:
    // Opens (or creates) the SQLite database at `dbPath`.
    bool open(const std::string& dbPath);
    void close();

    // Returns true if a summary exists for `path` with the exact `modTime`.
    bool isCached(const std::string& path, uint64_t modTime) const;

    // Retrieve cached summary.  Returns empty string if not cached.
    std::string get(const std::string& path) const;

    // Persist a new summary (inserts or replaces).
    void put(const FileSummary& summary);

    // Load all summaries for a set of paths (used to build module summaries).
    std::vector<FileSummary> getAll(const std::vector<std::string>& paths) const;

    // Delete stale entries for files that no longer exist.
    void purge(const std::string& workspaceRoot);

    // ── ModuleSummary ────────────────────────────────────────────────────────
    bool putModule(const ModuleSummary& ms);
    ModuleSummary getModule(const std::string& modulePath) const;
    std::vector<ModuleSummary> getAllModules() const;

    // ── ProjectSummary ───────────────────────────────────────────────────────
    bool putProject(const ProjectSummary& ps);
    ProjectSummary getProject() const;

    bool isOpen() const { return db_ != nullptr; }

private:
    void* db_ = nullptr;
    void createTables();
};

} // namespace AgentOS
