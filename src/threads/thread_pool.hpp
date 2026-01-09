#pragma once
#ifndef THREADS_THREAD_POOL_HPP_
#define THREADS_THREAD_POOL_HPP_

#include <condition_variable>
#include <iostream>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace Threads
{
    class ThreadPool
    {
    public:
        ThreadPool(size_t numThreads = std::thread::hardware_concurrency())
        {
            // Creating worker threads
            for (size_t i = 0; i < numThreads; ++i)
            {
                m_threads.emplace_back([this]
                {
                    while (true)
                    {
                        std::function<void()> task;
                        // The reason for putting the below code
                        // here is to unlock the queue before
                        // executing the task so that other
                        // threads can perform enqueue tasks
                        {
                            // Locking the queue so that data
                            // can be shared safely
                            std::unique_lock<std::mutex> lock(m_queueMutex);

                            // Waiting until there is a task to
                            // execute or the pool is stopped
                            m_cv.wait(lock, [this]
                            {
                                return !m_tasks.empty() || m_stop;
                            });

                            // exit the thread in case the pool
                            // is stopped and there are no tasks
                            if (m_stop && m_tasks.empty())
                                return;

                            // Get the next task from the queue
                            task = std::move(m_tasks.front());
                            m_tasks.pop();
                        }

                        task();
                    }
                });
            }
        }

        ~ThreadPool()
        {
            {
                // Lock the queue to update the stop flag safely
                std::unique_lock<std::mutex> lock(m_queueMutex);
                m_stop = true;
            }

            // Notify all threads
            m_cv.notify_all();

            // Joining all worker threads to ensure they have
            // completed their tasks
            for (auto& thread : m_threads)
            {
                if (thread.joinable())
                    thread.join();
            }
        }

        void Enqueue(std::function<void()> task)
        {
            {
                std::unique_lock<std::mutex> lock(m_queueMutex);
                m_tasks.emplace(std::move(task));
            }
            m_cv.notify_one();
        }

    private:
        std::vector<std::thread> m_threads;
        std::queue<std::function<void()>> m_tasks;
        std::mutex m_queueMutex;
        std::condition_variable m_cv;

        bool m_stop = false;
    };

    int RuntTest()
    {
        // Create a thread pool with 4 threads
        Threads::ThreadPool pool(4);

        // Enqueue tasks for execution
        for (int i = 0; i < 5; ++i)
        {
            pool.Enqueue([i] {
                std::cout << "Task " << i << " is running on thread " << std::this_thread::get_id() << std::endl;
                // Simulate some work
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            });
        }
        return 0;
    }
}

#endif // THREADS_THREAD_POOL_HPP_