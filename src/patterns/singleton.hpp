#pragma once
#ifndef PATTERNS_SINGLETON_HPP_
#define PATTERNS_SINGLETON_HPP_

#include <atomic>
#include <chrono>
#include <iostream>
#include <mutex>

namespace Patterns
{
    class Singleton
    {
    public:
        static Singleton* getInstance()
        {
            Singleton* singleton = m_instance.load(std::memory_order_acquire);

            if (!singleton)
            {
                std::lock_guard<std::mutex> myLock(m_mutex);
                singleton = m_instance.load(std::memory_order_relaxed);
                
                if(!singleton)
                {
                    singleton = new Singleton();
                    m_instance.store(singleton,std::memory_order_release);
                }
            }
            
            return singleton;
        }
        
    private:
        Singleton()                            = default;
        ~Singleton()                           = default;

        Singleton(const Singleton&)             = delete;
        Singleton(const Singleton&&)            = delete;
        Singleton& operator=(const Singleton&)  = delete;
        Singleton& operator=(const Singleton&&) = delete;

        inline static std::atomic<Singleton*> m_instance;
        inline static std::mutex m_mutex;
    };

    class MeyersSingleton
    {
    public:
        static MeyersSingleton& getInstance()
        {
            static MeyersSingleton instance;
            return instance;
        }

    private:
        MeyersSingleton()  = default;
        ~MeyersSingleton() = default;

        MeyersSingleton(const MeyersSingleton&)             = delete;
        MeyersSingleton(const MeyersSingleton&&)            = delete;
        MeyersSingleton& operator=(const MeyersSingleton&)  = delete;
        MeyersSingleton& operator=(const MeyersSingleton&&) = delete;
    };

    void TestSingleton()
    {
        auto then = std::chrono::high_resolution_clock::now();

        for (size_t i = 0; i < 10000000; ++i)
        {
            Singleton::getInstance();
        }

        auto now = std::chrono::high_resolution_clock::now();

        std::cout << "Time taken for thread safe singleton: "
                  << std::chrono::duration_cast<std::chrono::microseconds>(now - then).count() / 1000.0f
                  << " milliseconds" << std::endl;

        then = std::chrono::high_resolution_clock::now();

        for (size_t i = 0; i < 10000000; ++i)
        {
            MeyersSingleton::getInstance();
        }

        now = std::chrono::high_resolution_clock::now();

        std::cout << "Time taken for Meyers singleton: "
                  << std::chrono::duration_cast<std::chrono::microseconds>(now - then).count() / 1000.0f
                  << " milliseconds" << std::endl;
    }
}

#endif // PATTERNS_SINGLETON_HPP_