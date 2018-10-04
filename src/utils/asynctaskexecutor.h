//
// Created by zanna on 04/10/18.
//

#ifndef IRONHIDE_ASYNCTASKEXECUTOR_H
#define IRONHIDE_ASYNCTASKEXECUTOR_H


#include <memory>
#include <functional>
#include <thread>

#if HAS_BOOST_THREAD
#include <boost/asio/thread_pool.hpp>
#include <boost/asio.hpp>
#else
#include "threadpool.h"
#endif


namespace utils{
class AsyncTaskExecutor {
public:
    static AsyncTaskExecutor* instance();
    void submit_task(const std::function<void()> &) const;
protected:
    AsyncTaskExecutor() noexcept;
    virtual ~AsyncTaskExecutor();
private:
#if HAS_BOOST_THREAD
    std::unique_ptr<boost::asio::thread_pool> thread_pool;
#else
    std::unique_ptr<utils::ThreadPool> thread_pool;
#endif
    static AsyncTaskExecutor* inst;
};

#define ASYNC_TASK(task) \
    utils::AsyncTaskExecutor::instance()->submit_task(task);
} // namespace utils



#endif //IRONHIDE_ASYNCTASKEXECUTOR_H
