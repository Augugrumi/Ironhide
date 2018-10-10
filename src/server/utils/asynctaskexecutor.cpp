#include "asynctaskexecutor.h"

namespace utils {

AsyncTaskExecutor* AsyncTaskExecutor::inst = new AsyncTaskExecutor();

AsyncTaskExecutor::AsyncTaskExecutor() noexcept {
    thread_pool = std::unique_ptr<utils::ThreadPool>(new utils::ThreadPool(std::thread::hardware_concurrency()));
}

AsyncTaskExecutor::~AsyncTaskExecutor() {
    delete inst;
}

void AsyncTaskExecutor::submit_task(const std::function<void()> & task) const {
    thread_pool->enqueue(task);
}

AsyncTaskExecutor* AsyncTaskExecutor::instance() {
    return inst;
}

} // namespace utils
