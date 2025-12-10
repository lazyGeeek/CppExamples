#pragma once
#ifndef THREADS_BASIC_THREAD_HPP_
#define THREADS_BASIC_THREAD_HPP_

#include <chrono>
#include <iostream>
#include <format>
#include <thread>

namespace Threads
{
    class ThreadClass
    {
    public:
        void operator()(int& param1, int param2)
        {
            std::cout << std::format("ThreadClass operator() param1: {}; param2: {}", param1, param2) << std::endl;
        }

        void ThreadFunc(int& param1, int param2)
        {
            std::cout << std::format("ThreadClass ThreadFunc({}, {})", param1, param2) << std::endl;
        }
    };

    void ThreadFunc(int& param1, int param2)
    {
        std::cout << std::format("ThreadFunc(param1: {}; param2: {})", param1, param2) << std::endl;
    }

    void ThreadStaticFunc(int& param1, int param2)
    {
        // Thread local storage (TLS) is a feature introduced in C++ 11 that allows each thread in a multi-threaded program to have its own separate instance of a variable.
        // Each thread can access and modify its own copy of the variable without interfering with other threads.

        // The lifetime of a TLS variable begins when it is initialized and ends when the thread terminates.
        // TLS variables have visibility at the thread level.
        // TLS variables have scope depending on where they are declared

        // Other threads can't chage it
        thread_local int staticVar1 = 30;
        
        // Other threads can change it
        static thread_local int staticVar2 = 30;
        
        std::cout << std::format("ThreadStaticFunc(param1: {}; param2: {})", param1, param2) << std::endl;
    }

    void CreatThreadExamples()
    {
        int param1 = 10;
        int param2 = 20;

        std::thread thread1(ThreadFunc, std::ref(param1), param2);
        if (thread1.joinable())
            thread1.join();
        
        std::thread thread2([](int& param1, int param2)
        {
            std::cout << std::format("LambdaFunc param1: {}; param2: {}", param1, param2) << std::endl;
        }, std::ref(param1), param2);
        if (thread2.joinable())
            thread2.join();

        ThreadClass threadClass;
        std::thread thread3(threadClass, std::ref(param1), param2);
        if (thread3.joinable())
            thread3.join();
            
        std::thread thread4(&ThreadClass::ThreadFunc, threadClass, std::ref(param1), param2);

        std::thread thread5(ThreadStaticFunc, std::ref(param1), param2);
        if (thread5.joinable())
            thread5.join();

    }

    void ThreadSpecificOperations()
    {
        // Get thread ID of thread
        std::this_thread::get_id();

        // Give priority to other threads, pause execution
        std::this_thread::yield();

        // Sleep for some amount of time
        std::this_thread::sleep_for(std::chrono::seconds(1));

        // Sleep until some time
        std::chrono::system_clock::time_point time_point = std::chrono::system_clock::now()
                                                            + std::chrono::seconds(10);
        std::this_thread::sleep_until(time_point);
    }
}

#endif // THREADS_BASIC_THREAD_HPP_