#pragma once

#include <chrono>
#include <future>
#include <iostream>
#include <numeric>
#include <thread>
#include <vector>

namespace Threads::FuturePromise
{
   
    void Accumulate(std::vector<int>::iterator first,
                    std::vector<int>::iterator last,
                    std::promise<int> accumulatePromise)
    {
        std::cout << "Start Accumulate" << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(1));
        int sum = std::accumulate(first, last, 0);
        accumulatePromise.set_value(sum); // Notify future

        std::cout << "End Accumulate" << std::endl;
    }
    
    void DoWork(std::promise<void> barrier)
    {
        std::cout << "Start DoWork" << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(1));
        barrier.set_value();

        std::cout << "End DoWork" << std::endl;
    }
    
    void TestFuturePromise()
    {
        // Demonstrate using promise<int> to transmit a result between threads.
        std::vector<int> numbers = { 1, 2, 3, 4, 5, 6 };
        
        std::promise<int> accumulatePromise;
        std::future<int> accumulateFuture = accumulatePromise.get_future();
        
        std::cout << "Start workThread" << std::endl;

        std::thread workThread(Accumulate, numbers.begin(), numbers.end(),
                               std::move(accumulatePromise));
    
        // future::get() will wait until the future has a valid result and retrieves it.
        // Calling wait() before get() is not needed
        // accumulateFuture.wait(); // wait for result
        std::cout << "End of accumulateFuture\n";
        std::cout << "Result = " << accumulateFuture.get() << '\n';
        workThread.join(); // wait for thread completion
    
        // Demonstrate using promise<void> to signal state between threads.
        std::promise<void> barrier;
        std::future<void> barrierFuture = barrier.get_future();
        
        std::cout << "Start newWorkThread" << std::endl;

        std::thread newWorkThread(DoWork, std::move(barrier));

        barrierFuture.wait();
        newWorkThread.join();
    }
}
