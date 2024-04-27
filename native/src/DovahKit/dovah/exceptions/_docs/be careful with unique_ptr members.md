
DovahKit sometimes throws exceptions in a worker thread: the worker thread catches them, and uses `std::current_exception()` to store them in a `std::exception_ptr`. Then, after all worker threads are joined, the main thread checks if any of them threw and if so, it uses `std::rethrow_exception()` to re-throw whichever exception seems the most relevant (e.g. the one paired with the earliest offset into a file being read).

**This can cause problems if an exception has a `std::unique_ptr` data member, and if the exception class does not define a copy-constructor and copy-assignment.**

The `std::current_exception()` function uses compiler magic to copy the exception: even if you `catch` one of the exception's base classes -- hell, even if you `catch (...)` -- that function knows the exception's original type and can invoke the copy constructor on that type. What happens, however, if the original type's copy constructor is deleted, e.g. because one of the data members (say, a `std::unique_ptr`) isn't copyable?

Well, as of April 28, 2024, MSVC, at least, gets a little wacky. It fails to copy the exception, but instead of giving you `std::bad_exception`, it gives you a dangling `std::exception_ptr` to a now-destructed exception -- one that was formerly of the correct (i.e. originally thrown) type; it's still identifiable in the debugger because the v-table pointer is still intact. Whether it's giving you a failed-and-destructed copy, or a dangling reference to the original, I do not know. Either way, the problem leads to basically an immediate crash on any try/catch handler that actually touches the exception (i.e. one that isn't `catch (...)`).

To avoid this problem, just make sure to define a copy-constructor and copy-assignment operator for exceptions that may be captured in `std::current_exception`, if their default copy functions are deleted (e.g. because they have a `std::unique_ptr` data member).
