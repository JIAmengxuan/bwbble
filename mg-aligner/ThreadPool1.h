//
// Created by Mengxuan Jia on 2019/1/13.
//

#ifndef TP_THREADPOOL1_H
#define TP_THREADPOOL1_H

#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <queue>
#include <vector>
#include <iostream>
#include <type_traits>
#include <chrono>

class ThreadPool1 {
public:
    ThreadPool1(size_t = std::thread::hardware_concurrency());

    // Submit task into threadpool
    template<class Function, class... Args>
    std::future<std::invoke_result_t<Function, Args...>> submit(Function&&, Args&&...);

    // Waiting for all tasks to finish
    void waitAll();

    // Waiting for all tasks to finish for a given time duration.
    bool waitAllFor(std::chrono::milliseconds sleepDuration);

    // Shutdown all threads now and kick off all tasks from task queue.
    void shutDownNow();

    // Initiates an shutdown in which previously submitted tasks are executed, but no new tasks will be accepted.
    void shutDown(std::chrono::milliseconds = std::chrono::milliseconds::zero());

    ~ThreadPool1();

    ThreadPool1(ThreadPool1&&) = default;
    ThreadPool1(ThreadPool1&) = delete;

private:

    // Initialize thread
    void initThread();

    // used to keep the working threads
    std::vector<std::thread> workers;

    // tasks queue
    std::queue<std::function<void()>> tasks;

    // mutex for tasks queue
    std::mutex queueMutex;

    // only for the waitAll() function.
    std::mutex waitingMutex;

    // synchronization for fetching tasks from task queue
    std::condition_variable condForQueue;

    // notify the waitAll() function when the tasksNum is minus by one.
    std::condition_variable condForWaiting;

    // atomic number use to count the tasks that are not finished yet
    std::size_t tasksNum;

    // the state of ThreadPool
    bool isShutdown;
};

ThreadPool1::ThreadPool1(size_t threads) : tasksNum(0), isShutdown(false)
{
    for(int i = 0; i < threads; i++) {
        //std::cout << "From Thread--" << std::this_thread::get_id() << " saying: I am constructing" << std::endl;
        std::thread workThread(std::bind(&ThreadPool1::initThread, this));
        workers.push_back(std::move(workThread));
    }
}

ThreadPool1::~ThreadPool1() {
    if(!isShutdown) {
        shutDown();
    }
    //std::cout << "I am destructed" << std::endl;
}

void ThreadPool1::shutDownNow() {
    {
        std::lock_guard<std::mutex> lockGuard(queueMutex);
        isShutdown = true;

        tasks = {};// Same with: std::queue<std::function<void()>>().swap(tasks);
    }
    this->condForQueue.notify_all();
    for(std::thread& worker : workers) {
        worker.join();
    }
}

void ThreadPool1::shutDown(std::chrono::milliseconds waitTime) {
    //std::cout<< "Waiting all tasks for " << waitTime.count() << "ms and then collect resource" << std::endl;
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        isShutdown = true;
    }
    this->condForQueue.notify_all();

    std::this_thread::sleep_for(waitTime);
    for(std::thread& worker : this->workers) {
        if(worker.joinable()) {
            worker.join();
        } else {
            //TODO: ？？Don't know what to do here.
            worker.detach();
        }
    }
}


void ThreadPool1::initThread() {
    std::unique_lock<std::mutex> uniqueLock(this->queueMutex);
    // keep it waiting for task
    while(true) {
        if(!this->tasks.empty()) {
            auto task = std::move(this->tasks.front());
            this->tasks.pop();
            uniqueLock.unlock();
            task();
            {
                std::lock_guard<std::mutex> lockGuard(waitingMutex);
                tasksNum--;
                // It is more efficient that putting the cond.notify_one() into the lock_guard scope.
                // https://stackoverflow.com/questions/4544234/calling-pthread-cond-signal-without-locking-mutex
                this->condForWaiting.notify_one();
            }
            uniqueLock.lock();
        } else if(this->isShutdown){
            // Thread is going to kill itself
            break;
        } else {
            // Waiting for new task being added into the tasks queue
            this->condForQueue.wait(uniqueLock);
        }
    }
}

template<class Function, class... Args>
std::future<std::invoke_result_t<Function, Args...>> ThreadPool1::submit(Function&& f, Args&&... args) {
    {
        std::lock_guard<std::mutex> lockGuard(waitingMutex);
        tasksNum++;
    }
    using return_type = std::invoke_result_t<Function, Args...>;
    // Build a packaged_task
    auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<Function>(f), std::forward<Args>(args)...)
    );

    // get the future
    std::future<return_type> res = task->get_future();
    {
        // Lock the task queue，unlock the queue when get out from this code chunk
        std::lock_guard<std::mutex> lock(queueMutex);

        // Don't allow enqueueing after stopping the pool
        if(isShutdown){
            throw std::runtime_error("try to submit task to stopped ThreadPool");
        }
        // Push the task into the task queue
        this->tasks.emplace([task](){ (*task)();});

        // Notify one thread that is waiting for this condition.
        this->condForQueue.notify_one();
    }
    return std::move(res);
}


void ThreadPool1::waitAll() {
    std::unique_lock<std::mutex> unique_lock(waitingMutex);
    //std::cout<< "Waiting all tasks ot finish"<< std::endl;
    while(tasksNum != 0) {
        //std::cout << tasksNum << std::endl;
        // Keep waiting until the taskNum == 0, check the tasksNum only when be notified.
        this->condForWaiting.wait(unique_lock);
    }
    //std::cout<< "All tasks are finished" << std::endl;
}


bool ThreadPool1::waitAllFor(std::chrono::milliseconds sleepDuration) {
    //std::cout<< "Waiting all tasks ot finish for " << sleepDuration.count() << "ms" << std::endl;

    auto startTime = std::chrono::high_resolution_clock::now();
    auto endTime = startTime + sleepDuration;

    std::unique_lock<std::mutex> unique_lock(waitingMutex);
    while(tasksNum != 0 || std::chrono::high_resolution_clock::now() >= endTime) {
        this->condForWaiting.wait(unique_lock);
    }
    //std::cout<< (tasksNum == 0 ? "All tasks are finished" : "Time out!") << std::endl;
    return tasksNum == 0;
}

#endif //TP_THREADPOOL1_H
