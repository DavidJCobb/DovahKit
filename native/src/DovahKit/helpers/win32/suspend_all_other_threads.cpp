#include "./suspend_all_other_threads.h"
#include <windows.h>
#include <tlhelp32.h>

namespace cobb::win32 {
   extern void suspend_all_other_threads() {
      HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, NULL);
      if (snapshot == INVALID_HANDLE_VALUE)
         return;
      THREADENTRY32 thread_entry;
      thread_entry.dwSize = sizeof(thread_entry);
      if (Thread32First(snapshot, &thread_entry)) {
         //
         // We'll loop over every thread (across basically every process in the system, so we 
         // do need to specifically check that any given thread belongs to *our process*).
         //
         const auto this_thread  = GetCurrentThreadId();
         const auto this_process = GetCurrentProcessId();
         do {
            if (thread_entry.dwSize >= FIELD_OFFSET(THREADENTRY32, th32OwnerProcessID) + sizeof(thread_entry.th32OwnerProcessID)) {
               if (
                  thread_entry.th32OwnerProcessID == this_process &&
                  thread_entry.th32ThreadID != this_thread
               ) {
                  const auto thread_handle = OpenThread(THREAD_SUSPEND_RESUME, FALSE, thread_entry.th32ThreadID);
                  if (thread_handle != INVALID_HANDLE_VALUE)
                     SuspendThread(thread_handle);
               }
            }
            thread_entry.dwSize = sizeof(thread_entry);
         } while (Thread32Next(snapshot, &thread_entry));
      }
      CloseHandle(snapshot);
   }
}