#pragma once
#include <thread>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>

class ThreadPool {
public:
    ThreadPool(size_t num_threads) {
        taskCount = 0;
        for (size_t i = 0; i < num_threads; ++i) {
            threads.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(queue_mutex);

                        cv.wait(lock, [this] {
                            return !tasks.empty() || stop;
                            });

                        if (stop && tasks.empty()) {
                            return;
                        }

                        task = std::move(tasks.front());
                        tasks.pop();
                    }

                    task();
                    if(--taskCount == 0){
                        std::unique_lock<std::mutex> lock(wait_mutex);
                        completion_cv.notify_all();
                    }
                }
            });
        }
    }

    ~ThreadPool()
    {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
        }

        cv.notify_all();

        for (auto& thread : threads) {
            thread.join();
        }
    }

    void enqueue(std::function<void()> task)
    {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            tasks.emplace(std::move(task));
        }
        {
            std::unique_lock<std::mutex> lock(wait_mutex);
            taskCount++;
        }
        cv.notify_one();
    }

    void waitForAll() {
        std::unique_lock<std::mutex> lock(wait_mutex);
        completion_cv.wait(lock, [this] { return taskCount == 0; });
    }
private:
    std::vector<std::thread> threads;
    std::queue<std::function<void()> > tasks;
    std::mutex queue_mutex;
    std::mutex wait_mutex;

    std::condition_variable cv;
    std::condition_variable completion_cv;
    std::atomic<unsigned int> taskCount{0};
    bool stop = false;
};

