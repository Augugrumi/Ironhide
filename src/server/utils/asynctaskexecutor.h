#ifndef IRONHIDE_ASYNCTASKEXECUTOR_H
#define IRONHIDE_ASYNCTASKEXECUTOR_H

#include "config.h"

#include <memory>
#include <functional>
#include <thread>

#include "threadpool.h"
#include "log.h"

namespace utils{
class AsyncTaskExecutor {
public:
    static AsyncTaskExecutor* instance();
    void submit_task(const std::function<void()> &) const;
protected:
    AsyncTaskExecutor() noexcept;
    virtual ~AsyncTaskExecutor();
private:
    std::unique_ptr<utils::ThreadPool> thread_pool;
    static AsyncTaskExecutor* inst;
};

// TODO find something better
#define ASYNC_TASK(task) \
    utils::AsyncTaskExecutor::instance()->submit_task(task);

#define GO_ASYNC(task) \
    std::thread(task).detach();
} // namespace utils



#endif //IRONHIDE_ASYNCTASKEXECUTOR_H
