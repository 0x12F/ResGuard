# ResGuard – tiny C++ RAII wrapper

This single-header library gives you a drop-in RAII guard for **anything that needs to be released**.  
File descriptors, sockets, mutexes, `malloc`’d memory, CUDA handles, Win32 objects, … you name it.

## Why?

- No more forgotten `close`, `free`, `ReleaseMutex`, etc.  
- No accidental double-free or resource leak.  
- Works exactly like `std::unique_ptr`, but for non-pointer resources.

