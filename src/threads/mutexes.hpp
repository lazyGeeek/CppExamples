#pragma once
#ifndef THREADS_MUTEXES_HPP_
#define THREADS_MUTEXES_HPP_

#include <iostream>
#include <format>
#include <mutex>
#include <shared_mutex>
#include <thread>

namespace Threads
{
    class BasicMutex
    {
    public:
        void TestMutex()
        {
            std::thread t1(&BasicMutex::increment, this);
            std::thread t2(&BasicMutex::increment, this);

            if (t1.joinable())
                t1.join();
            
            if (t2.joinable())
                t2.join();
        }

    private:
        void increment()
        {
            for (int i = 0; i < 100000; ++i)
            {
                m_mutex.lock();
                ++m_counter;
                m_mutex.unlock();
            }
        }

        std::mutex m_mutex;
        int m_counter = 0;
    };

    class LockGuard
    {
    public:
        void TestLockGuard()
        {
            std::thread t1(&LockGuard::increment, this);
            std::thread t2(&LockGuard::increment, this);

            if (t1.joinable())
                t1.join();
            
            if (t2.joinable())
                t2.join();
        }
    private:
        void increment()
        {
            for (int i = 0; i < 100000; ++i)
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                ++m_counter;
            }            
        }

        std::mutex m_mutex;
        int m_counter = 0;
    };

    class FlexibleLockGuards
    {
    public:
        void TestFlexibleLockGuards()
        {
            std::thread t1(&FlexibleLockGuards::flexibleTask, this);
            std::thread t2(&FlexibleLockGuards::flexibleTask, this);

            if (t1.joinable())
                t1.join();
            
            if (t2.joinable())
                t2.join();
        }

    private:
        void flexibleTask()
        {
            // 1. Create unique_lock, but don't lock the mutex immediately
            std::unique_lock<std::mutex> lock(m_mutex, std::defer_lock);

            std::cout << "Thread " << std::this_thread::get_id() << ": Doing non-critical work (1)..." << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(20));

            lock.lock(); // 2. Explicitly acquire the lock
            ++m_sharedValue;
            std::cout << "Thread " << std::this_thread::get_id() << ": Locked, value: " << m_sharedValue << std::endl;
            lock.unlock(); // 3. Explicitly release the lock early

            std::cout << "Thread " << std::this_thread::get_id() << ": Doing more non-critical work (2)..." << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(20));

            if (lock.try_lock()) // 4. Try to re-acquire (non-blocking attempt)
            {    
                ++m_sharedValue;
                std::cout << "Thread " << std::this_thread::get_id() << ": Re-locked, value: " << m_sharedValue << std::endl;
            }
            else
            {
                std::cout << "Thread " << std::this_thread::get_id() << ": Couldn't re-lock, resource busy." << std::endl;
            }
        }

        std::mutex m_mutex;
        int m_sharedValue = 0;
    };

    class NonBlockingLock
    {
    public:
        void TestMutex()
        {
            m_resourceMutex.lock();
            std::cout << "Main: Resource is currently held by main." << std::endl;

            std::thread t1(&NonBlockingLock::workerTryLock, this); // t1 will likely find resource busy
            std::this_thread::sleep_for(std::chrono::milliseconds(20)); // Give t1 a chance to try_lock

            std::cout << "Main: Releasing resource, then launching another worker." << std::endl;
            m_resourceMutex.unlock(); // Now the resource is free for others

            std::thread t2(&NonBlockingLock::workerTryLock, this); // t2 will likely succeed now

            if (t1.joinable())
                t1.join();
            
            if (t2.joinable())
                t2.join();
        }

    private:
        void workerTryLock()
        {
            // Simulate some work before trying to acquire the resource
            std::this_thread::sleep_for(std::chrono::milliseconds(50));

            // Attempt to lock the resource_mutex without blocking
            if (m_resourceMutex.try_lock())
            {
                std::lock_guard<std::mutex> guard(m_printMutex); // Lock cout for this message
                std::cout << "Worker " << std::this_thread::get_id() << ": Acquired resource and doing work!" << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Simulate work with resource
                m_resourceMutex.unlock(); // IMPORTANT: Manually unlock after a successful try_lock
            }
            else
            {
                std::lock_guard<std::mutex> guard(m_printMutex); // Lock cout for this message
                std::cout << "Worker " << std::this_thread::get_id() << ": Resource busy, doing alternative tasks." << std::endl;
            }
        }

        std::mutex m_resourceMutex;
        std::mutex m_printMutex;
    };

    class AdvencedLockManagement
    {
    public:
        void TestLock()
        {
            std::thread t1(&AdvencedLockManagement::moveData, this, 1, 2, 500);
            std::thread t2(&AdvencedLockManagement::moveData, this, 2, 1, 50);

            if (t1.joinable())
                t1.join();

            if (t2.joinable())
                t2.join();
        }
    
    private:
        void moveData(int from, int to, int data)
        {
            // To prevent deadlock, we acquire both locks using std::lock.
            // The order of arguments (m_poolLock1, m_poolLock2) here doesn't matter
            // for deadlock prevention, as std::lock handles it safely.
            std::cout << "Thread " << std::this_thread::get_id()
                      << ": Attempting to transfer " << data << " from A" << from
                      << " to B" << to << std::endl;

            // Acquire both locks in a deadlock-safe manner
            std::lock(m_poolLock1, m_poolLock2);

            // Once std::lock returns, both mutexes are held by this thread.
            // Now, create unique_lock objects, telling them the mutexes are already locked.
            std::unique_lock<std::mutex> lock1(m_poolLock1, std::adopt_lock);
            std::unique_lock<std::mutex> lock2(m_poolLock2, std::adopt_lock);

            // Simulate fund transfer (critical section)
            std::cout << "Thread " << std::this_thread::get_id()
                      << ": Locks acquired. Transferring funds..." << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            
            // In a real scenario, this would involve modifying actual account balances
            // For demonstration, we just show locks held.
            std::cout << "Thread " << std::this_thread::get_id()
                      << ": Funds transferred. Releasing locks." << std::endl;

            // locks are automatically released by unique_lock destructors
        }

        std::mutex m_poolLock1;
        std::mutex m_poolLock2;
    };

    class MultiMutexLock
    {
    public:
        void TestLock()
        {
            std::thread t1(&MultiMutexLock::modifyMultipleResources, this, 1);
            std::thread t2(&MultiMutexLock::modifyMultipleResources, this, 2);

            if (t1.joinable())
                t1.join();

            if (t2.joinable())
                t2.join();
        }

    private:
        void modifyMultipleResources(int threadId)
        {
            std::cout << "Thread " << threadId << ": Trying to acquire locks for resources 1 and 2..." << std::endl;

            // This single line ensures both mutexes are acquired safely (deadlock-free)
            // and automatically unlocked via RAII when 'lock' goes out of scope.
            std::scoped_lock lock(m_mutex1, m_mutex2);

            std::cout << "Thread " << threadId << ": Acquired both locks. Performing combined operation." << std::endl;
            // Simulate critical work involving both resources
            std::this_thread::sleep_for(std::chrono::milliseconds(80));
            std::cout << "Thread " << threadId << ": Finished combined operation. Releasing locks." << std::endl;

        } // Both m_mutex1 and m_mutex2 are automatically released here

        std::mutex m_mutex1;
        std::mutex m_mutex2;

    };

    class SafeContainer
    {
    public:
        SafeContainer(int value) : m_value(value) { }

        void addAndDouble(int val)
        {
            std::lock_guard<std::recursive_mutex> lock(m_mutex); // Acquires lock (1st time)
            std::cout << "Thread " << std::this_thread::get_id()
                      << ": add_and_double, value before: " << m_value << std::endl;

            internalAdd(val); // This call attempts to lock the SAME mutex again
            internalAdd(m_value); // Double by adding current value

            std::cout << "Thread " << std::this_thread::get_id()
                      << ": add_and_double, value after: " << m_value << std::endl;
        }

        int GetValue()
        {
            std::lock_guard<std::recursive_mutex> lock(m_mutex);
            return m_value;
        }

    private:
        void internalAdd(int val)
        {
            std::lock_guard<std::recursive_mutex> lock(m_mutex); // Acquires lock (2nd time if called by add_and_double)
            m_value += val;
            std::cout << "Thread " << std::this_thread::get_id()
                      << ": Internal add, value now: " << m_value << std::endl;
        }

        std::recursive_mutex m_mutex;
        int m_value = 0;
    };

    class ReaderWriterLock
    {
    public:
        void TestLock()
        {
            std::thread r1(&ReaderWriterLock::readConfig, this, 1);
            std::thread r2(&ReaderWriterLock::readConfig, this, 2);
            std::thread w1(&ReaderWriterLock::writeConfig, this, 1, "Updated Config Alpha"); // Writer 1
            std::thread r3(&ReaderWriterLock::readConfig, this, 3);
            std::thread r4(&ReaderWriterLock::readConfig, this, 4);
            std::thread w2(&ReaderWriterLock::writeConfig, this, 2, "Final Config Beta");    // Writer 2
            std::thread r5(&ReaderWriterLock::readConfig, this, 5);

            if (r1.joinable())
                r1.join();

            if (r2.joinable())
                r2.join();

            if (w1.joinable())
                w1.join();

            if (r3.joinable())
                r3.join();

            if (r4.joinable())
                r4.join();

            if (w2.joinable())
                w2.join();

            if (r5.joinable())
                r5.join();
        }
    private:
        void readConfig(int id)
        {
            for (int i = 0; i < 2; ++i) // Each reader reads a couple of times
            {    
                // Acquire a shared (read) lock. Multiple readers can hold this simultaneously.
                std::shared_lock<std::shared_mutex> lock(m_configMutex);
                
                std::cout << "Reader " << id << ": Reads '" << m_sharedConfig << "'" << std::endl;

                std::this_thread::sleep_for(std::chrono::milliseconds(20)); // Simulate read time
            } // 'lock' releases the shared_mutex when it goes out of scope
        }

        void writeConfig(int id, const std::string& newData)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Give readers a head start

            // Acquire an exclusive (write) lock. Only one writer (or reader) at a time.
            std::unique_lock<std::shared_mutex> lock(m_configMutex);

            std::cout << "Writer " << id << ": Writing '" << newData << "'..." << std::endl;
            
            m_sharedConfig = newData; // Critical write operation
            
            std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Simulate write time
            
            std::cout << "Writer " << id << ": Write complete." << std::endl;
        } // 'lock' releases the shared_mutex when it goes out of scope

        std::string m_sharedConfig = "Initial Config Value";
        std::shared_mutex m_configMutex; // Our reader-writer lock
    };

    class TimeMutexLock
    {
    public:
        void TestLock()
        {
            m_resourceTimedMutex.lock(); // Main holds the lock
            std::thread t1(&TimeMutexLock::processWithTimeout, this, 1); // This thread will likely timeout
            std::this_thread::sleep_for(std::chrono::milliseconds(20)); // Give t1 a chance to try

            std::thread t2(&TimeMutexLock::processWithTimeout, this, 2); // This thread will also likely timeout
            std::this_thread::sleep_for(std::chrono::milliseconds(20));

            std::cout << "Main: Releasing m_resourceTimedMutex." << std::endl;
            m_resourceTimedMutex.unlock(); // Main releases the lock

            std::thread t3(&TimeMutexLock::processWithTimeout, this, 3); // This thread might succeed or wait briefly

            if (t1.joinable())
                t1.join();
            
            if (t2.joinable())
                t2.join();
            
            if (t3.joinable())
                t3.join();
        }

    private:
        void processWithTimeout(int id)
        {
            std::cout << std::format("Thread {}: Trying to acquire lock with timeout...", id) << std::endl;
            
            // Try to acquire the lock for a maximum of 50 milliseconds
            if (m_resourceTimedMutex.try_lock_for(std::chrono::milliseconds(50)))
            {
                // Lock acquired successfully
                ++m_dataToProcess;
                std::cout << std::format("Thread {}: Acquired lock! Processed data to {}", id, m_dataToProcess) << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Simulate work while holding lock
                m_resourceTimedMutex.unlock();
            }
            else
            {
                // Lock not acquired within the timeout
                std::cout << std::format("Thread {}: Failed to acquire lock within timeout. Doing something else.", id) << std::endl;
            }
        }

        std::timed_mutex m_resourceTimedMutex;
        int m_dataToProcess = 0;
    };
}

#endif // THREADS_MUTEXES_HPP_