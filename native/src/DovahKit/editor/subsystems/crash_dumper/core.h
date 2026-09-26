#pragma once
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>
#include "helpers/singleton_ex.h"
#include "helpers/win32/forward_declare_handles.h"
#include "helpers/win32/WINAPI.define.h"
struct _EXCEPTION_POINTERS; // Win32

namespace dovahkit::subsystems::crash_dumper {
   class core : public cobb::singleton_ex<core> {
      protected:
         core();
         ~core();

         [[noreturn]] static void _terminate_handler();
         static long WINAPI _unhandled_exception_filter(_EXCEPTION_POINTERS*);
         static void _sentinel_thread_handler();

      public:
         void register_new_thread();

      protected:
         HANDLE exception_filter_mutex = NULL;

         std::thread sentinel;
         std::atomic<bool> handled_any = false;
         struct {
            std::condition_variable cv;
            _EXCEPTION_POINTERS* exception_info = nullptr;
            struct {
               bool       sent = false;
               std::mutex mutex;
            } status;
         } dispatch;
         struct {
            std::condition_variable cv;
            struct {
               bool       sent = false;
               std::mutex mutex;
            } status;
         } respond;

         // called by sentinel
         void _wait_for_exception_information();

         // called by unhandled exception filter
         void _dispatch_exception_information(_EXCEPTION_POINTERS*);

         // called by sentinel
         void _process_exception_information();

         // called by unhandled exception filter
         void _wait_for_sentinel_acknowledge();

         // called by sentinel
         void _sentinel_acknowledge();
   };
}

#undef WINAPI