
# Crash dumper

This subsystem catches crashes, creates minidumps, and handles notifying the user about the problem. It relies on both `std::set_terminate` (for uncaught C++ exceptions) and `SetUnhandledExceptionFilter` (for hardware/OS exceptions) to intercept crashes.

We have to assume that heap allocation is no longer safe when these handlers run, and this influences how we go about doing some things.

## Kinds of errors

### C++ exceptions

Because the exception's text (`std::exception::what`) may be owned by the exception object, we have to either copy it out of the exception, or display it immediately. It's simpler to just display it immediately.

### Other exceptions

We don't know what thread the exception is being thrown from, nor whether the stack is in a usable state (as opposed to being close to overflow, having a mangled stack pointer, etc.). For that reason, we spawn up a "sentinel" thread at program startup (since each thread has its own stack); our unhandled exception filter will try to pass the exception information off to the sentinel thread so that it can handle everything for us.

## Other issues

### `std::terminate`

The `std::set_terminate` function has non-standards-conforming behavior in MSVC. There is supposed to be just one global `std::terminate_handler`, but under MSVC, each thread has its own terminate handler; there is no way to set a global handler. Microsoft has been aware of this issue [since 2019 at the latest](https://developercommunity.visualstudio.com/t/c-stdset-terminate-does-not-syncronize-and-effect/507132). They've expressed a reluctance to fix it due to backwards-compatibility issues, they've made no apparent attempt to offer a non-standard API that implements the standard behavior, and they've offered no workarounds.

As such, I have no other option but to edit every piece of multi-threaded code in DovahKit to set the same terminate handler manually, and I have to also pray that nothing in any of these libraries spawns threads that might fail.

The frontend relies on `dovahkit::subsystems::crash_dumper::register_new_thread()` to set the terminate handler for its own threads. I don't want the backend to have to "reach into" the frontend, so the backend has all of its threads set a common terminate handler via `dovah::worker_thread_termination_handler`, and the frontend just customizes that.