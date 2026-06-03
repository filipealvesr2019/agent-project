#include "LocalRuntime/TaskScheduler.h"

namespace AgentOS {

TaskScheduler& TaskScheduler::getInstance() {
    static TaskScheduler instance;
    return instance;
}

void TaskScheduler::init(size_t numThreads) {
    std::unique_lock<std::mutex> lock(queueMutex_);
    if (!workers_.empty()) return; // Already initialized

    for (size_t i = 0; i < numThreads; ++i) {
        workers_.emplace_back([this] {
            for (;;) {
                std::function<void()> task;

                {
                    std::unique_lock<std::mutex> lock(this->queueMutex_);
                    this->condition_.wait(lock, [this] {
                        return this->stop_ || !this->tasks_.empty();
                    });
                    if (this->stop_ && this->tasks_.empty()) {
                        return;
                    }
                    task = std::move(this->tasks_.front());
                    this->tasks_.pop();
                }

                task();
            }
        });
    }
}

void TaskScheduler::shutdown() {
    {
        std::unique_lock<std::mutex> lock(queueMutex_);
        stop_ = true;
    }
    condition_.notify_all();
    for (std::thread& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    workers_.clear();
}

TaskScheduler::~TaskScheduler() {
    shutdown();
}

size_t TaskScheduler::getPendingTasksCount() const {
    std::unique_lock<std::mutex> lock(queueMutex_);
    return tasks_.size();
}

} // namespace AgentOS
