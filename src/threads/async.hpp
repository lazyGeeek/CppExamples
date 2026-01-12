#pragma once

#include <algorithm>
#include <future>
#include <iostream>
#include <mutex>
#include <numeric>
#include <string>
#include <vector>

namespace Threads::Async
{ 
    std::mutex m;
    
    struct X
    {
        void Foo(int i, const std::string& str)
        {
            std::lock_guard<std::mutex> lk(m);
            std::cout << str << ' ' << i << '\n';
        }
    
        void Bar(const std::string& str)
        {
            std::lock_guard<std::mutex> lk(m);
            std::cout << str << '\n';
        }
    
        int operator()(int i)
        {
            std::lock_guard<std::mutex> lk(m);
            std::cout << i << '\n';
            return i + 10;
        }
    };
    
    template<typename RandomIt>
    int ParallelSum(RandomIt beg, RandomIt end)
    {
        auto len = end - beg;
        if (len < 1000)
            return std::accumulate(beg, end, 0);
    
        RandomIt mid = beg + len / 2;
        auto handle = std::async(std::launch::async, ParallelSum<RandomIt>, mid, end);
        int sum = ParallelSum(beg, mid);
        return sum + handle.get();
    }
    
    void TestAsync()
    {
        std::vector<int> v(10000, 1);
        std::cout << "The sum is " << ParallelSum(v.begin(), v.end()) << '\n';
    
        X x;
        // Calls (&x)->foo(42, "Hello") with default policy:
        // may print "Hello 42" concurrently or defer execution
        auto a1 = std::async(&X::Foo, &x, 42, "Hello");

        // Calls x.bar("world!") with deferred policy
        // prints "world!" when a2.get() or a2.wait() is called
        auto a2 = std::async(std::launch::deferred, &X::Bar, x, "world!");
        
        // Calls X()(43); with async policy
        // prints "43" concurrently
        auto a3 = std::async(std::launch::async, X(), 43);
        a2.wait();                     // prints "world!"
        std::cout << a3.get() << '\n'; // prints "53"
    } // if a1 is not done at this point, destructor of a1 prints "Hello 42" here
}
