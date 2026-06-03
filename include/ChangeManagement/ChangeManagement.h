#pragma once
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <memory>
#include <filesystem>

namespace AgentOS {

enum class ChangeState { Pending, Approved, Rejected, RolledBack };
enum class EmergencyLevel { None, Warning, Critical, Catastrophic };

struct DiffLine {
    enum Type { Same, Added, Removed };
    Type type;
    int oldLineNum;
    int newLineNum;
    std::string content;
};

struct DiffResult {
    std::vector<DiffLine> lines;
    int addedCount{ 0 };
    int removedCount{ 0 };
    std::string toString() const;
};

struct Snapshot {
    int id{ 0 };
    std::string agentName;
    std::string timestamp;
    std::string filePath;
    std::string reason;
    std::string hash;
    size_t fileSize{ 0 };
    std::string getStoragePath() const;
};

struct ChangeProposal {
    int id{ 0 };
    std::string agentName;
    std::string filePath;
    std::string originalContent;
    std::string newContent;
    std::string diffText;
    ChangeState state{ ChangeState::Pending };
    std::string timestamp;
    std::string approvedBy;
    int snapshotId{ 0 };
};

class SnapshotManager {
public:
    SnapshotManager();
    int createSnapshot(const std::string& agentName, const std::string& filePath,
                       const std::string& content, const std::string& reason);
    Snapshot getSnapshot(int id) const;
    std::vector<Snapshot> getSnapshots() const;
    std::vector<Snapshot> getSnapshots(const std::string& agentName) const;
    std::vector<Snapshot> getSnapshotsForFile(const std::string& filePath) const;
    bool restoreFromSnapshot(int id);
    bool restoreFromSnapshot(int id, const std::string& targetPath);
    int getTotalSnapshots() const { return (int)snapshots_.size(); }
    std::string getSnapshotsDir() const { return snapshotsDir_; }

private:
    bool saveSnapshotContent(int id, const std::string& content);
    std::string loadSnapshotContent(int id) const;
    std::string computeHash(const std::string& content) const;
    std::vector<Snapshot> snapshots_;
    std::string snapshotsDir_;
    std::string indexFile_;
    void loadIndex();
    void saveIndex();
    int nextId_{ 1 };
};

class DiffEngine {
public:
    DiffResult compare(const std::string& before, const std::string& after);
    std::string createUnifiedDiff(const std::string& filePath, const std::string& before, const std::string& after);
};

class ChangeManager {
public:
    ChangeManager(SnapshotManager& snapshotMgr);
    int proposeChange(const std::string& agentName, const std::string& filePath,
                      const std::string& originalContent, const std::string& newContent);
    bool approveChange(int changeId, const std::string& approvedBy);
    bool rejectChange(int changeId);
    bool rollbackChange(int changeId);
    ChangeProposal getChange(int changeId) const;
    std::vector<ChangeProposal> getChanges() const;
    std::vector<ChangeProposal> getChanges(ChangeState state) const;
    std::vector<ChangeProposal> getPendingChanges() const;
    int getPendingCount() const;

    std::function<void(const ChangeProposal&)> onChangeCreated;
    std::function<void(const ChangeProposal&)> onChangeApproved;
    std::function<void(const ChangeProposal&)> onChangeRejected;

private:
    void applyChange(const ChangeProposal& proposal);
    SnapshotManager& snapshotMgr_;
    std::vector<ChangeProposal> changes_;
    int nextId_{ 1 };
};

class RollbackManager {
public:
    RollbackManager(SnapshotManager& snapshotMgr);
    bool rollbackFile(int snapshotId, const std::string& filePath);
    bool rollbackGlobal(int snapshotId);
    bool rollbackToTimestamp(const std::string& timestamp);

    std::function<void(const std::string&)> onRollback;
    int getRollbackCount() const { return rollbackCount_; }

private:
    SnapshotManager& snapshotMgr_;
    int rollbackCount_{ 0 };
};

class RecoveryEngine {
public:
    RecoveryEngine(SnapshotManager& snapshotMgr, RollbackManager& rollbackMgr);
    void checkForIssues();
    void triggerEmergencyStop(const std::string& reason);
    EmergencyLevel assessLevel(const std::string& agentName) const;
    void emergencyStop();
    void recover();
    bool isEmergencyActive() const { return emergencyActive_; }
    std::string getLastEmergencyReason() const { return lastEmergencyReason_; }

    std::function<void(const std::string&)> onEmergencyStop;
    std::function<void()> onRecoveryComplete;

private:
    SnapshotManager& snapshotMgr_;
    RollbackManager& rollbackMgr_;
    bool emergencyActive_{ false };
    std::string lastEmergencyReason_;
    int recoveryCount_{ 0 };
};

class ChangeManagementEngine {
public:
    static ChangeManagementEngine& getInstance();

    void init();
    void shutdown();

    SnapshotManager& getSnapshotManager() { return snapshotMgr_; }
    DiffEngine& getDiffEngine() { return diffEngine_; }
    ChangeManager& getChangeManager() { return changeMgr_; }
    RollbackManager& getRollbackManager() { return rollbackMgr_; }
    RecoveryEngine& getRecoveryEngine() { return recoveryEngine_; }

    void applySnapshotBeforeModify(const std::string& agentName, const std::string& filePath,
                                   const std::string& content, const std::string& reason);
    int proposeAndSnapshot(const std::string& agentName, const std::string& filePath,
                           const std::string& originalContent, const std::string& newContent);

    std::function<void(const std::string&)> onStatusUpdate;

private:
    ChangeManagementEngine() : changeMgr_(snapshotMgr_), rollbackMgr_(snapshotMgr_), recoveryEngine_(snapshotMgr_, rollbackMgr_) {}
    ~ChangeManagementEngine() = default;
    ChangeManagementEngine(const ChangeManagementEngine&) = delete;
    ChangeManagementEngine& operator=(const ChangeManagementEngine&) = delete;

    std::string snapshotsDir_;
    SnapshotManager snapshotMgr_;
    DiffEngine diffEngine_;
    ChangeManager changeMgr_;
    RollbackManager rollbackMgr_;
    RecoveryEngine recoveryEngine_;
};

} // namespace AgentOS
