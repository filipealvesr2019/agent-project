#include "ChangeManagement/ChangeManagement.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <ctime>

namespace fs = std::filesystem;

namespace AgentOS {

// ========== DiffResult ==========

std::string DiffResult::toString() const {
    std::string result;
    for (const auto& line : lines) {
        switch (line.type) {
            case DiffLine::Same:   result += " " + line.content + "\n"; break;
            case DiffLine::Added:  result += "+" + line.content + "\n"; break;
            case DiffLine::Removed: result += "-" + line.content + "\n"; break;
        }
    }
    return result;
}

// ========== Snapshot ==========

std::string Snapshot::getStoragePath() const {
    char buf[16];
    snprintf(buf, sizeof(buf), "%04d", id);
    return "snapshot_" + std::string(buf);
}

// ========== SnapshotManager ==========

SnapshotManager::SnapshotManager() {
    snapshotsDir_ = (fs::current_path() / ".snapshots").string();
    indexFile_ = (fs::path(snapshotsDir_) / "index.dat").string();
    try { fs::create_directories(snapshotsDir_); } catch (...) {}
    loadIndex();
}

int SnapshotManager::createSnapshot(const std::string& agentName, const std::string& filePath,
                                     const std::string& content, const std::string& reason) {
    Snapshot snap;
    snap.id = nextId_++;
    snap.agentName = agentName;
    snap.timestamp = "agora";
    {
        auto now = std::chrono::system_clock::now();
        auto t = std::chrono::system_clock::to_time_t(now);
        char buf[32];
        ctime_s(buf, sizeof(buf), &t);
        snap.timestamp = std::string(buf);
        if (!snap.timestamp.empty() && snap.timestamp.back() == '\n')
            snap.timestamp.pop_back();
    }
    snap.filePath = filePath;
    snap.reason = reason;
    snap.fileSize = content.size();
    snap.hash = computeHash(content);

    if (saveSnapshotContent(snap.id, content)) {
        snapshots_.push_back(snap);
        saveIndex();
    }

    return snap.id;
}

Snapshot SnapshotManager::getSnapshot(int id) const {
    for (const auto& s : snapshots_)
        if (s.id == id) return s;
    return {};
}

std::vector<Snapshot> SnapshotManager::getSnapshots() const {
    return snapshots_;
}

std::vector<Snapshot> SnapshotManager::getSnapshots(const std::string& agentName) const {
    std::vector<Snapshot> result;
    for (const auto& s : snapshots_)
        if (s.agentName == agentName) result.push_back(s);
    return result;
}

std::vector<Snapshot> SnapshotManager::getSnapshotsForFile(const std::string& filePath) const {
    std::vector<Snapshot> result;
    for (const auto& s : snapshots_)
        if (s.filePath == filePath) result.push_back(s);
    return result;
}

bool SnapshotManager::restoreFromSnapshot(int id) {
    auto snap = getSnapshot(id);
    if (snap.id == 0) return false;
    return restoreFromSnapshot(id, snap.filePath);
}

bool SnapshotManager::restoreFromSnapshot(int id, const std::string& targetPath) {
    auto content = loadSnapshotContent(id);
    if (content.empty()) return false;

    try {
        auto parent = fs::path(targetPath).parent_path();
        if (!parent.empty()) fs::create_directories(parent);
        std::ofstream file(targetPath);
        if (!file.is_open()) return false;
        file << content;
        file.close();
        return true;
    } catch (...) {
        return false;
    }
}

bool SnapshotManager::saveSnapshotContent(int id, const std::string& content) {
    try {
        char buf[16];
        snprintf(buf, sizeof(buf), "%04d", id);
        auto dir = (fs::path(snapshotsDir_) / ("snapshot_" + std::string(buf)));
        fs::create_directories(dir);
        auto filePath = dir / "content.bin";
        std::ofstream file(filePath, std::ios::binary);
        if (!file.is_open()) return false;
        file.write(content.data(), content.size());
        file.close();

        auto metaPath = dir / "meta.txt";
        std::ofstream meta(metaPath);
        if (meta.is_open()) {
            meta << "size=" << content.size() << "\n";
            meta.close();
        }
        return true;
    } catch (...) {
        return false;
    }
}

std::string SnapshotManager::loadSnapshotContent(int id) const {
    try {
        char buf[16];
        snprintf(buf, sizeof(buf), "%04d", id);
        auto filePath = (fs::path(snapshotsDir_) / ("snapshot_" + std::string(buf)) / "content.bin").string();
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open()) return "";
        std::stringstream ss;
        ss << file.rdbuf();
        return ss.str();
    } catch (...) {
        return "";
    }
}

std::string SnapshotManager::computeHash(const std::string& content) const {
    unsigned long hash = 5381;
    for (char c : content)
        hash = ((hash << 5) + hash) + (unsigned char)c;
    char buf[32];
    snprintf(buf, sizeof(buf), "%08lx", hash);
    return std::string(buf);
}

void SnapshotManager::loadIndex() {
    snapshots_.clear();
    std::ifstream file(indexFile_);
    if (!file.is_open()) return;
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        Snapshot s;
        std::string token;
        if (std::getline(ss, token, '|')) s.id = std::stoi(token);
        if (std::getline(ss, s.agentName, '|')) {}
        if (std::getline(ss, s.timestamp, '|')) {}
        if (std::getline(ss, s.filePath, '|')) {}
        if (std::getline(ss, s.reason, '|')) {}
        if (std::getline(ss, s.hash, '|')) {}
        std::string sizeStr;
        if (std::getline(ss, sizeStr, '|')) s.fileSize = std::stoul(sizeStr);
        snapshots_.push_back(s);
        if (s.id >= nextId_) nextId_ = s.id + 1;
    }
    file.close();
}

void SnapshotManager::saveIndex() {
    try {
        fs::create_directories(snapshotsDir_);
        std::ofstream file(indexFile_);
        if (!file.is_open()) return;
        for (const auto& s : snapshots_) {
            file << s.id << "|" << s.agentName << "|" << s.timestamp << "|"
                 << s.filePath << "|" << s.reason << "|" << s.hash << "|" << s.fileSize << "\n";
        }
        file.close();
    } catch (...) {}
}

// ========== DiffEngine ==========

DiffResult DiffEngine::compare(const std::string& before, const std::string& after) {
    DiffResult result;

    std::vector<std::string> beforeLines, afterLines;
    {
        std::stringstream ss(before);
        std::string line;
        while (std::getline(ss, line)) beforeLines.push_back(line);
    }
    {
        std::stringstream ss(after);
        std::string line;
        while (std::getline(ss, line)) afterLines.push_back(line);
    }

    size_t maxLen = std::max(beforeLines.size(), afterLines.size());
    for (size_t i = 0; i < maxLen; ++i) {
        DiffLine dl;
        if (i < beforeLines.size() && i < afterLines.size()) {
            if (beforeLines[i] == afterLines[i]) {
                dl.type = DiffLine::Same;
                dl.content = beforeLines[i];
            } else {
                dl.type = DiffLine::Removed;
                dl.content = beforeLines[i];
                result.removedCount++;
                result.lines.push_back(dl);
                dl.type = DiffLine::Added;
                dl.content = afterLines[i];
                result.addedCount++;
                result.lines.push_back(dl);
                continue;
            }
        } else if (i < beforeLines.size()) {
            dl.type = DiffLine::Removed;
            dl.content = beforeLines[i];
            result.removedCount++;
        } else {
            dl.type = DiffLine::Added;
            dl.content = afterLines[i];
            result.addedCount++;
        }
        dl.oldLineNum = (int)i + 1;
        dl.newLineNum = (int)i + 1;
        result.lines.push_back(dl);
    }

    return result;
}

std::string DiffEngine::createUnifiedDiff(const std::string& filePath, const std::string& before, const std::string& after) {
    auto diff = compare(before, after);
    std::string result;
    result += "--- " + filePath + "\n";
    result += "+++ " + filePath + "\n";
    result += diff.toString();
    return result;
}

// ========== ChangeManager ==========

ChangeManager::ChangeManager(SnapshotManager& snapshotMgr) : snapshotMgr_(snapshotMgr) {}

int ChangeManager::proposeChange(const std::string& agentName, const std::string& filePath,
                                  const std::string& originalContent, const std::string& newContent) {
    ChangeProposal prop;
    prop.id = nextId_++;
    prop.agentName = agentName;
    prop.filePath = filePath;
    prop.originalContent = originalContent;
    prop.newContent = newContent;
    prop.state = ChangeState::Pending;
    {
        auto now = std::chrono::system_clock::now();
        auto t = std::chrono::system_clock::to_time_t(now);
        char buf[32];
        ctime_s(buf, sizeof(buf), &t);
        prop.timestamp = std::string(buf);
        if (!prop.timestamp.empty() && prop.timestamp.back() == '\n')
            prop.timestamp.pop_back();
    }

    DiffEngine de;
    prop.diffText = de.createUnifiedDiff(filePath, originalContent, newContent);

    int snapId = snapshotMgr_.createSnapshot(agentName, filePath, originalContent,
                                              "Change proposal #" + std::to_string(prop.id));
    prop.snapshotId = snapId;

    changes_.push_back(prop);

    if (onChangeCreated) onChangeCreated(prop);
    return prop.id;
}

bool ChangeManager::approveChange(int changeId, const std::string& approvedBy) {
    for (auto& c : changes_) {
        if (c.id == changeId && c.state == ChangeState::Pending) {
            c.state = ChangeState::Approved;
            c.approvedBy = approvedBy;
            applyChange(c);
            if (onChangeApproved) onChangeApproved(c);
            return true;
        }
    }
    return false;
}

bool ChangeManager::rejectChange(int changeId) {
    for (auto& c : changes_) {
        if (c.id == changeId && c.state == ChangeState::Pending) {
            c.state = ChangeState::Rejected;
            if (onChangeRejected) onChangeRejected(c);
            return true;
        }
    }
    return false;
}

bool ChangeManager::rollbackChange(int changeId) {
    for (auto& c : changes_) {
        if (c.id == changeId && c.state == ChangeState::Approved) {
            bool ok = snapshotMgr_.restoreFromSnapshot(c.snapshotId, c.filePath);
            if (ok) c.state = ChangeState::RolledBack;
            return ok;
        }
    }
    return false;
}

ChangeProposal ChangeManager::getChange(int changeId) const {
    for (const auto& c : changes_)
        if (c.id == changeId) return c;
    return {};
}

std::vector<ChangeProposal> ChangeManager::getChanges() const {
    return changes_;
}

std::vector<ChangeProposal> ChangeManager::getChanges(ChangeState state) const {
    std::vector<ChangeProposal> result;
    for (const auto& c : changes_)
        if (c.state == state) result.push_back(c);
    return result;
}

std::vector<ChangeProposal> ChangeManager::getPendingChanges() const {
    return getChanges(ChangeState::Pending);
}

int ChangeManager::getPendingCount() const {
    int count = 0;
    for (const auto& c : changes_)
        if (c.state == ChangeState::Pending) count++;
    return count;
}

void ChangeManager::applyChange(const ChangeProposal& proposal) {
    try {
        auto parent = fs::path(proposal.filePath).parent_path();
        if (!parent.empty()) fs::create_directories(parent);
        std::ofstream file(proposal.filePath);
        if (file.is_open()) {
            file << proposal.newContent;
            file.close();
        }
    } catch (...) {}
}

// ========== RollbackManager ==========

RollbackManager::RollbackManager(SnapshotManager& snapshotMgr) : snapshotMgr_(snapshotMgr) {}

bool RollbackManager::rollbackFile(int snapshotId, const std::string& filePath) {
    bool ok = snapshotMgr_.restoreFromSnapshot(snapshotId, filePath);
    if (ok) {
        rollbackCount_++;
        if (onRollback) onRollback("Arquivo restaurado: " + filePath + " (snapshot #" + std::to_string(snapshotId) + ")");
    }
    return ok;
}

bool RollbackManager::rollbackGlobal(int snapshotId) {
    auto snap = snapshotMgr_.getSnapshot(snapshotId);
    if (snap.id == 0) return false;

    auto allSnapshots = snapshotMgr_.getSnapshots();
    std::vector<Snapshot> toRestore;
    for (const auto& s : allSnapshots) {
        if (s.id <= snapshotId && !s.filePath.empty()) {
            bool alreadyAdded = false;
            for (const auto& r : toRestore) {
                if (r.filePath == s.filePath && r.id > s.id) {
                    alreadyAdded = true;
                    break;
                }
            }
            if (!alreadyAdded) toRestore.push_back(s);
        }
    }

    int restored = 0;
    for (const auto& s : toRestore) {
        if (snapshotMgr_.restoreFromSnapshot(s.id, s.filePath))
            restored++;
    }

    rollbackCount_++;
    if (onRollback)
        onRollback("Rollback global para snapshot #" + std::to_string(snapshotId) + " (" + std::to_string(restored) + " arquivos restaurados)");
    return restored > 0;
}

bool RollbackManager::rollbackToTimestamp(const std::string& timestamp) {
    auto allSnapshots = snapshotMgr_.getSnapshots();
    int targetId = 0;
    for (const auto& s : allSnapshots) {
        if (s.timestamp <= timestamp) targetId = s.id;
    }
    if (targetId == 0) return false;
    return rollbackGlobal(targetId);
}

// ========== RecoveryEngine ==========

RecoveryEngine::RecoveryEngine(SnapshotManager& snapshotMgr, RollbackManager& rollbackMgr)
    : snapshotMgr_(snapshotMgr), rollbackMgr_(rollbackMgr) {}

void RecoveryEngine::checkForIssues() {}

void RecoveryEngine::triggerEmergencyStop(const std::string& reason) {
    lastEmergencyReason_ = reason;
    emergencyActive_ = true;
    emergencyStop();
}

EmergencyLevel RecoveryEngine::assessLevel(const std::string& agentName) const {
    return EmergencyLevel::None;
}

void RecoveryEngine::emergencyStop() {
    auto snapId = snapshotMgr_.createSnapshot("SYSTEM", "EMERGENCY", "", lastEmergencyReason_);
    rollbackMgr_.rollbackGlobal(snapId);

    if (onEmergencyStop)
        onEmergencyStop("EMERGENCY STOP acionado: " + lastEmergencyReason_);

    emergencyActive_ = true;
}

void RecoveryEngine::recover() {
    if (!emergencyActive_) return;
    emergencyActive_ = false;
    recoveryCount_++;
    if (onRecoveryComplete) onRecoveryComplete();
}

// ========== ChangeManagementEngine ==========

ChangeManagementEngine& ChangeManagementEngine::getInstance() {
    static ChangeManagementEngine instance;
    return instance;
}

void ChangeManagementEngine::init() {
    if (onStatusUpdate) onStatusUpdate("ChangeManagement: Sistema de snapshots pronto");
    if (onStatusUpdate) onStatusUpdate("ChangeManagement: Gerenciamento de mudanças ativo");
    if (onStatusUpdate) onStatusUpdate("ChangeManagement: Engine de recuperação carregada");
}

void ChangeManagementEngine::shutdown() {
    if (onStatusUpdate) onStatusUpdate("ChangeManagement finalizado");
}

void ChangeManagementEngine::applySnapshotBeforeModify(const std::string& agentName,
                                                        const std::string& filePath,
                                                        const std::string& content,
                                                        const std::string& reason) {
    snapshotMgr_.createSnapshot(agentName, filePath, content, reason);
}

int ChangeManagementEngine::proposeAndSnapshot(const std::string& agentName, const std::string& filePath,
                                                const std::string& originalContent, const std::string& newContent) {
    return changeMgr_.proposeChange(agentName, filePath, originalContent, newContent);
}

} // namespace AgentOS
