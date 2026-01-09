The key idea behind std::atomic is **atomicity** – making operations indivisible or instantaneous relative to threads. Atomicity prevents nasty race conditions where threads interleave shared data access incorrectly.

The std::atomic wrapper makes operations on a value atomic. Under the hood, it uses hardware primitives like compare-and-swap instead of mutexes.

`std::atomic` overloads many operators like `++`, `–`, `=`, `+=`, `==`, `<` etc. to provide atomic semantics when dealing with wrapped values

In addition, `std::atomic` provides explicit atomic operations:

- **load()** – Atomically loads the current value.
- **store()** – Atomically writes a new value.
- **exchange()** – Atomically swaps in a new value and returns old one.
- **compare_exchange_weak() / compare_exchange_strong()** – Atomically compares value and exchanges if equal.

### Memory Ordering

Memory ordering refers to constraints on how atomically updated values propagate across threads.

By default, `std::atomic` uses sequential consistency ordering which provides very strong guarantees. Values updated atomically are guaranteed to become visible to all threads simultaneously.

The **`std::atomic`** template provides several options for memory ordering:

- **memory_order_relaxed** – Allows operations to be performed without concerns for the relative ordering with respect to other operations. Use it for performance, but only when you are sure it won’t cause issues.
- **memory_order_release / memory_order_acquire** – These ensure that all prior operations are completed before a subsequent operation (acquire) and enforce all following operations to see the effects of a preceding operation (release).
- **memory_order_seq_cst** – This is the default option and ensures all memory operations are seen in a specific sequence across threads, eliminating race conditions.

You can specify memory ordering semantics on atomic operations like `load()` and `store()`.


### Best Practices for Using std::atomic

When using **`std::atomic`**, it's essential to understand when to use it over mutexes.

- Use atomics when you have simple data types that can be modified independently.
- For complex manipulations, consider using mutexes to prevent potential race conditions.
- Avoid mixing atomic and non-atomic accesses to the same variable or data structure; it can lead to unexpected behavior.

### Atomic Operations


| Function       | Description                                                                                           |
| -------------- | ----------------------------------------------------------------------------------------------------- |
| `load()`       | This function loads the value stored in the atomic object.                                            |
| `store()`      | The function stores the value in the atomic object.                                                   |
| `exchange()`   | This function replaces the value stored in the atomic object and returns the previously stored value. |
| `wait()`       | This function is used to block the thread.                                                            |
| `notify_one()` | Notifies one of the threads that was waiting for the atomic object.                                   |
| `notify_all()` | Notifies all the threads that were waiting for the atomic object.                                     |
| `fetch_add()`  | Gets the current value stored and adds the given value to the atomic object's value.                  |
| `fetch_sub()`  | Gets the current value stored and subtracts the given value from the atomic object's value.           |
| `fetch_and()`  | Gets the current value stored and performs bitwise AND operation with atomic value and given value.   |
| `fetch_or()`   | Gets the current value stored and performs bitwise OR operation with atomic value and given value.    |
| `fetch_xor()`  | Gets the current value stored and performs bitwise XOR operation with atomic value and given value.   |

### Atomic Flag

A `std::atomic_flag` is a simple atomic boolean type that can be used as a lock-free flag. It is a specialization of `atomic<bool>` type that is guaranteed to be always lock-free. `std::atomic_flag` provides a specialized atomic boolean type that is lock-free and size-efficient.

Atomic operations:

| Method           | Description                                    |
| ---------------- | ---------------------------------------------- |
| `test()`         | Gets the value stored in the atomic flag.      |
| `test_and_set()` | Gets the value and sets the value of the flag. |
| `clear()`        | Reset the value of the atomic flag.            |

