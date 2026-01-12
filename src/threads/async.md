The function template `std::async` runs the function `f` asynchronously (potentially in a separate thread which might be a part of a thread pool) and returns a `std::future` that will eventually hold the result of that function call.
1) Behaves as if (2) is called with policy being `std::launch::async` | `std::launch::deferred`.
2) Calls a function f with arguments args according to a specific launch policy policy.
The return type of `std::async` is `std::future<V>`, where `V` is:
```C++
typename std::result_of<typename std::decay<F>::type(
                        typename std::decay<Args>::type...)>::type.

...

std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>...>.
```

### Policy selection

`std::async` can accept different launch policies that influence how tasks are executed. There are two primary options:
- **std::launch::async:** Forces the task to run on a new thread. This ensures that the function executes immediately in parallel.
- **std::launch::deferred:** The function execution is deferred until the `future` object’s `get()` method is called. In this case, the task runs in the same thread that calls `get()`.

If more than one flag is set, it is implementation-defined which policy is selected. For the default (both the `std::launch::async` and `std::launch::deferred` flags are set in policy), standard recommends (but does not require) utilizing available concurrency, and deferring any additional tasks.

If the `std::launch::async` policy is chosen,
- a call to a waiting function on an asynchronous return object that shares the shared state created by this `std::async` call blocks until the associated thread has completed, as if joined, or else time out; and
- the associated thread completion _synchronizes-with_ the successful return from the first function that is waiting on the shared state, or with the return of the last function that releases the shared state, whichever comes first.
### Notes

The implementation may extend the behavior of the first overload of `std::async` by enabling additional (implementation-defined) bits in the default launch policy.

Examples of implementation-defined launch policies are the sync policy (execute immediately, within the `std::async` call) and the task policy (similar to `std::async`, but thread-locals are not cleared)

If the `std::future` obtained from `std::async` is not moved from or bound to a reference, the destructor of the `std::future` will block at the end of the full expression until the asynchronous operation completes, essentially making code such as the following synchronous:

```C++
std::async(std::launch::async, []{ f(); }); // temporary's dtor waits for f()
std::async(std::launch::async, []{ g(); }); // does not start until f() completes
```