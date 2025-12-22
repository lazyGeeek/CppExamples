#pragma once
#ifndef THREADS_COROUTINES_HPP_
#define THREADS_COROUTINES_HPP_

// https://medium.com/@AlexanderObregon/understanding-c-coroutine-implementation-8e6e5a2c3edd

#include <coroutine>
#include <fstream>
#include <iostream>
#include <optional>
#include <queue>
#include <thread>

namespace Threads
{
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////// Return Value /////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    struct ReturnValue
    {
        struct promise_type
        {
            int Value = 0;

            ReturnValue get_return_object()
            {
                return ReturnValue{ std::coroutine_handle<promise_type>::from_promise(*this) };
            }

            std::suspend_never initial_suspend() noexcept { return { }; }
            std::suspend_always final_suspend() noexcept { return { }; } // keep frame alive
            
            void return_value(int value) noexcept { Value = value; }
            void unhandled_exception() { std::terminate(); }
        };

        std::coroutine_handle<promise_type> Handle;

        explicit ReturnValue(std::coroutine_handle<promise_type> handle) : Handle(handle) { }

        ReturnValue(ReturnValue&& other) noexcept : Handle(other.Handle) { other.Handle = {}; }
        ReturnValue(const ReturnValue& other) = delete;

        ReturnValue& operator=(const ReturnValue&) = delete;

        ~ReturnValue() { if (Handle) Handle.destroy(); }

        int Get() const { return Handle.promise().Value; }
    };

    ReturnValue ComputeValue()
    {
        co_return 128;
    }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////// Asynchronous I/O Operations //////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    struct FileReader
    {
        struct promise_type
        {
            FileReader get_return_object()
            {
                return FileReader { std::coroutine_handle<promise_type>::from_promise(*this) };
            }
            
            std::suspend_always initial_suspend() noexcept { return { }; }
            std::suspend_always final_suspend() noexcept { return { }; }
            
            void return_void() { }
            void unhandled_exception() { std::terminate(); }
        };
        
        std::coroutine_handle<promise_type> Handle;
        
        explicit FileReader(std::coroutine_handle<promise_type> handle) : Handle(handle) { }
        
        FileReader(FileReader&& other) noexcept : Handle(other.Handle) { other.Handle = { }; }
        FileReader(const FileReader& other) = delete;
        
        ~FileReader() { if (Handle) Handle.destroy(); }

    };

    struct FileReaderAwaiter
    {
        std::ifstream file;
        std::string line;
        bool finished = false;

        bool await_ready() const { return finished; }

        void await_suspend(std::coroutine_handle<> handle)
        {
            if (std::getline(file, line))   
                std::cout << "Processing: " << line << '\n';
            else
                finished = true;
            
            handle.resume();
        }

        void await_resume() { }
    };

    FileReaderAwaiter ReadAsync(const std::string& filename)
    {
        FileReaderAwaiter awaiter;
        awaiter.file.open(filename);
        return awaiter;
    }

    FileReader ProcessFiles()
    {
        FileReaderAwaiter r1 = ReadAsync("file1.txt");
        FileReaderAwaiter r2 = ReadAsync("file2.txt");

        while (!r1.finished || !r2.finished)
        {
            co_await r1;
            co_await r2;
        }
        co_return;
    }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////// Concurrency and Task Coordination ////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    struct Task
    {
        struct promise_type
        {
            Task get_return_object()
            {
                return Task { std::coroutine_handle<promise_type>::from_promise(*this) };
            }

            std::suspend_always initial_suspend() noexcept { return { }; }   // create suspended
            std::suspend_always final_suspend() noexcept { return { }; }     // keep frame alive
            
            void return_void() { }
            void unhandled_exception() { std::terminate(); }
        };

        std::coroutine_handle<promise_type> Handle;

        explicit Task(std::coroutine_handle<promise_type> handle) : Handle(handle) { }
        
        Task(Task&& other) noexcept : Handle(other.Handle) { other.Handle = { }; }
        Task(const Task& other) = delete;
        
        ~Task() { if (Handle) Handle.destroy(); }

        void start() { if (Handle && !Handle.done()) Handle.resume(); }

        struct TaskAwaiter
        {
            std::chrono::milliseconds Delay;
            bool await_ready() const { return false; }
            
            void await_suspend(std::coroutine_handle<> handle) const
            {
                auto delay = Delay;  // copy to extend lifetime
                std::thread([handle, delay]()
                {
                    std::this_thread::sleep_for(delay);
                    handle.resume();
                }).detach();
            }
            
            void await_resume() const { }
        };

        static TaskAwaiter MoveEntity(int id, int distance)
        {
            std::cout << "Entity " << id << " Moving " << distance << " units\n";
            return TaskAwaiter { std::chrono::milliseconds(500 * distance) };
        }

        static TaskAwaiter UpdateEntity(int id)
        {
            std::cout << "Entity " << id << " Updating\n";
            return TaskAwaiter { std::chrono::milliseconds(100) };
        }
    };

    Task RunSimulation()
    {
        co_await Task::MoveEntity(1, 5);
        co_await Task::UpdateEntity(1);
        co_await Task::MoveEntity(2, 3);
        co_await Task::UpdateEntity(2);
        co_await Task::MoveEntity(1, 2);
    }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////// Building Event-Driven Systems ////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    struct Event
    {
        int Id;
        std::string Data;
    };

    struct EventQueue
    {
        std::queue<Event> Events;

        void push(const Event& event) { Events.push(event); }
        
        std::optional<Event> try_pop()
        {
            if (Events.empty())
                return std::nullopt;
            
            Event event = Events.front();
            Events.pop();
            return event;
        }
    };

    // Awaitable that resumes the awaiting coroutine once an event is available.
    // It polls on a small background worker to keep the example self-contained.
    struct WaitForEvent
    {
        EventQueue& EventQueue;
        Event Out{ -1, "" };

        bool await_ready() const { return false; }

        void await_suspend(std::coroutine_handle<> handle)
        {
            std::thread([this, handle]() mutable
            {
                for (;;)
                {
                    if (auto event = EventQueue.try_pop())
                    {
                        Out = *event;
                        handle.resume();
                        break;
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            }).detach();
        }

        Event await_resume() const { return Out; }
    };

    struct EventHandler
    {
        struct promise_type
        {
            std::coroutine_handle<promise_type> self()
            {
                return std::coroutine_handle<promise_type>::from_promise(*this);
            }
            
            EventHandler get_return_object() { return EventHandler{ self() }; }
            
            std::suspend_always initial_suspend() noexcept { return { }; }   // created suspended
            std::suspend_always final_suspend() noexcept { return { }; }     // keep frame for owner
            
            void return_void() { }
            void unhandled_exception() { std::terminate(); }
        };

        std::coroutine_handle<promise_type> Handle { };

        explicit EventHandler(std::coroutine_handle<promise_type> handle) : Handle(handle) { }
        
        EventHandler(EventHandler&& other) noexcept : Handle(other.Handle) { other.Handle = {}; }
        EventHandler(const EventHandler&) = delete;
        
        ~EventHandler() { if (Handle) Handle.destroy(); }

        void start() { if (Handle && !Handle.done()) Handle.resume(); }
    };

    EventHandler HandleEvents(EventQueue& queue)
    {
        while (true)
        {
            Event e = co_await WaitForEvent{ queue };
            std::cout << "Handling event " << e.Id << ": " << e.Data << std::endl;
        }
    }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////// Modeling State Machines with Coroutines //////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    struct StateStep
    {
        std::string State;

        bool await_ready() const { return false; }
        
        void await_suspend(std::coroutine_handle<> handle) const
        {
            // Simulate some work or waiting tied to this state
            std::cout << "Current state: " << State << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            handle.resume();
        }
        
        void await_resume() const { }
    };

    struct ConnectionFlow
    {
        struct promise_type
        {
            std::coroutine_handle<promise_type> self()
            {
                return std::coroutine_handle<promise_type>::from_promise(*this);
            }
            
            ConnectionFlow get_return_object() { return ConnectionFlow{ self() }; }
            
            std::suspend_always initial_suspend() noexcept { return { }; }   // created suspended
            std::suspend_always final_suspend() noexcept { return { }; }     // keep frame for owner
            
            void return_void() { }
            void unhandled_exception() { std::terminate(); }
        };

        std::coroutine_handle<promise_type> Handle { };

        explicit ConnectionFlow(std::coroutine_handle<promise_type> handle) : Handle(handle) { }
        
        ConnectionFlow(ConnectionFlow&& other) noexcept : Handle(other.Handle) { other.Handle = {}; }
        ConnectionFlow(const ConnectionFlow& other) = delete;
        
        ~ConnectionFlow() { if (Handle) Handle.destroy(); }

        void start() { if (Handle && !Handle.done()) Handle.resume(); }
    };

    ConnectionFlow ManageConnection()
    {
        co_await StateStep{ "Connecting" };
        co_await StateStep{ "Connected" };
        co_await StateStep{ "Disconnecting" };
        co_await StateStep{ "Disconnected" };
    }

    void TestCoroutines()
    {
        ReturnValue result = ComputeValue();
        std::cout << "Computed Value: " << result.Get() << std::endl;

        FileReader fileReader = ProcessFiles();
        fileReader.Handle.resume(); // start and run to completion

        Task task = RunSimulation();   // created suspended
        task.start();                  // kick it off
        std::this_thread::sleep_for(std::chrono::seconds(5));

        EventQueue queue;

        // Produce a few events in the background.
        std::thread producer([&]()
        {
            for (int i = 0; i < 5; ++i)
            {
                queue.push( { i, "EventData " + std::to_string(i) } );
                std::this_thread::sleep_for(std::chrono::milliseconds(300));
            }
        });
        producer.detach();

        EventHandler handler = HandleEvents(queue);  // created suspended
        handler.start();

        std::this_thread::sleep_for(std::chrono::seconds(3)); // let it run a bit

        ConnectionFlow flow = ManageConnection(); // created suspended
        flow.start();
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }

}

#endif // THREADS_COROUTINES_HPP_
