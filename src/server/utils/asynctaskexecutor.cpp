#include "asynctaskexecutor.h"

utils::AsyncTaskExecutor *utils::AsyncTaskExecutor::inst = new utils::AsyncTaskExecutor();

utils::AsyncTaskExecutor::AsyncTaskExecutor() noexcept {
    thread_pool = std::unique_ptr<utils::ThreadPool>(
            new utils::ThreadPool(std::thread::hardware_concurrency()));
}

utils::AsyncTaskExecutor::~AsyncTaskExecutor() {
    delete inst;
}

void
utils::AsyncTaskExecutor::submit_task(const std::function<void()> &task) const {
    thread_pool->enqueue(task);
}

utils::AsyncTaskExecutor *utils::AsyncTaskExecutor::instance() {
    return inst;
}
