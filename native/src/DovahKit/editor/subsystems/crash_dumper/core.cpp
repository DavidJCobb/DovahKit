#include "./core.h"
#include <string_view>
#include <QtLogging>

#include "helpers/windows.h"
#include <dbghelp.h> // MINIDUMP_EXCEPTION_INFORMATION
#pragma comment(lib, "Dbghelp.lib")

#include "dovah/worker_thread_termination_handler.h"

#include "../../dovahkit_version_macros.h"

#define STR(x) L###x
#define XSTR(x) STR(x)

#define VERSION_STRING XSTR(VER_MAJOR) L"." XSTR(VER_MINOR) L"." XSTR(VER_PATCH) L"." XSTR(VER_BUILD)

namespace {
   constexpr const bool only_referenced_modules = false;

   // Equivalent to WER_DUMP_TYPE::WerDumpTypeMiniDump
   constexpr const auto small_minidump_flags = (MINIDUMP_TYPE)(MiniDumpWithDataSegs | MiniDumpWithUnloadedModules | MiniDumpWithProcessThreadData | MiniDumpWithTokenInformation);

   // Equivalent to WER_DUMP_TYPE::WerDumpTypeHeapDump
   constexpr const auto large_minidump_flags = (MINIDUMP_TYPE)(MiniDumpWithDataSegs | MiniDumpWithProcessThreadData | MiniDumpWithHandleData | MiniDumpWithPrivateReadWriteMemory | MiniDumpWithUnloadedModules | MiniDumpWithFullMemoryInfo | MiniDumpWithThreadInfo | MiniDumpWithTokenInformation | MiniDumpWithPrivateWriteCopyMemory);
}

namespace dovahkit::subsystems::crash_dumper {
   core::core() {
      this->exception_filter_mutex = CreateMutex(NULL, FALSE, NULL);
      if (this->exception_filter_mutex == NULL) {
         qDebug("[Crash Dumper] Unable to create mutex. We will not set an unhandled exception filter; no minidumps can be produced.");
         return;
      }

      this->sentinel = std::thread(&_sentinel_thread_handler);
      SetUnhandledExceptionFilter(&_unhandled_exception_filter);

      std::set_terminate(&_terminate_handler);
      dovah::worker_thread_termination_handler::get().set_handler(&_terminate_handler);
   }
   core::~core() {
      CloseHandle(this->exception_filter_mutex);
      this->exception_filter_mutex = NULL;

      // Let the sentinel thread terminate.
      this->_dispatch_exception_information(nullptr);
      this->sentinel.join();
   }

   void core::register_new_thread() {
      std::set_terminate(&_terminate_handler);
   }

   static BOOL _minidump_callback(
      PVOID                            param,
      const PMINIDUMP_CALLBACK_INPUT   input,
      PMINIDUMP_CALLBACK_OUTPUT        output
   ) {
      switch (input->CallbackType) {
         case ModuleCallback:
            //
            // Omit code segments. We shouldn't need those; we should have the relevant 
            // modules on hand already.
            //
            if constexpr (only_referenced_modules) {
               output->ModuleWriteFlags &= ModuleReferencedByMemory;
               if (output->ModuleWriteFlags) {
                  output->ModuleWriteFlags = (
                     ModuleWriteModule |
                     ModuleWriteDataSeg |
                     ModuleReferencedByMemory |
                     ModuleWriteTlsData
                  );
               }
            } else {
               output->ModuleWriteFlags = (
                  ModuleWriteModule |
                  ModuleWriteDataSeg |
                  (output->ModuleWriteFlags & ModuleReferencedByMemory) |
                  ModuleWriteTlsData
               );
            }
            break;
      }
      return TRUE;
   }

   [[noreturn]] /*static*/ void core::_terminate_handler() {
      constexpr const std::string_view text_before_with_exception = "DovahKit has encountered a fatal error and will crash. The std::current_exception text is:\n\n";
      constexpr const std::string_view text_before_sans_exception = "DovahKit has encountered a fatal error and will crash. No std::current_exception text is available.";
      constexpr const std::string_view text_after =
         "\n\nPlease press Ctrl + C on this message box to copy all of this text. Save it somewhere so you can send it to DovahKit's developer, "
         "and then click \"OK\" for further instructions."
      ;

      if (core::get().handled_any.exchange(true)) {
         // Returning triggers a call to `std::abort`. Don't.
         while (true) {};
      }

      static char message_text[512];

      auto e_ptr = std::current_exception();
      if (e_ptr) {
         try {
            std::rethrow_exception(e_ptr);
         } catch (const std::exception& e) {
            snprintf(
               message_text,
               std::extent<decltype(message_text)>::value,
               "%s%s%s",
               text_before_with_exception.data(),
               e.what(),
               text_after.data()
            );
         } catch (...) {
            MessageBoxA(NULL, "caught unknown", NULL, MB_OK | MB_ICONERROR | MB_SYSTEMMODAL | MB_SETFOREGROUND);
            snprintf(
               message_text,
               std::extent<decltype(message_text)>::value,
               "%s%s",
               text_before_sans_exception.data(),
               text_after.data()
            );
         }
      } else {
         snprintf(
            message_text,
            std::extent<decltype(message_text)>::value,
            "%s%s",
            text_before_sans_exception.data(),
            text_after.data()
         );
      }

      MessageBoxA(
         NULL,
         message_text,
         nullptr,
         MB_OK | MB_ICONERROR | MB_SYSTEMMODAL | MB_SETFOREGROUND
      );

      // mini-dump without exception information
      core::get()._process_exception_information();

      _set_abort_behavior(0, _WRITE_ABORT_MSG);
      std::abort();
   }
   
   /*static*/ long WINAPI core::_unhandled_exception_filter(LPEXCEPTION_POINTERS pointers) {
      auto& subsystem = core::get();
      if (subsystem.handled_any.exchange(true)) {
         return EXCEPTION_EXECUTE_HANDLER;
      }

      const auto wait_result = WaitForSingleObject(subsystem.exception_filter_mutex, INFINITE);
      if (wait_result == WAIT_FAILED) {
         return EXCEPTION_EXECUTE_HANDLER;
      }

      subsystem._dispatch_exception_information(pointers);
      __try {
         subsystem._wait_for_sentinel_acknowledge();
      } __finally {
         ReleaseMutex(subsystem.exception_filter_mutex);
      }
      return EXCEPTION_EXECUTE_HANDLER;
   }

   /*static*/ void core::_sentinel_thread_handler() {
      auto& subsystem = core::get();
      subsystem._wait_for_exception_information();
      if (subsystem.dispatch.exception_info) {
         subsystem._process_exception_information();
      }
      subsystem._sentinel_acknowledge();
   }

   void core::_wait_for_exception_information() {
      std::unique_lock<std::mutex> lock(this->dispatch.status.mutex);
      this->dispatch.cv.wait(lock, [this]() { return this->dispatch.status.sent; });
   }

   void core::_dispatch_exception_information(_EXCEPTION_POINTERS* ep) {
      {
         auto lock = std::lock_guard(this->dispatch.status.mutex);
         this->dispatch.exception_info = ep;
         this->dispatch.status.sent = true;
      }
      this->dispatch.cv.notify_all();
   }

   void core::_process_exception_information() {
      constexpr const std::wstring_view app_name    = L"DovahKit";
      constexpr const std::wstring_view app_version = VERSION_STRING;

      bool user_wants_large = false;
      {
         int response = MessageBoxW(
            NULL,
            L"DovahKit has encountered an irrecoverable error (basically, a crash).\n\n"
            L"(You may be able to fiddle with the program in the background, below this message "
            L"box, as a side-effect of how we catch and log crashes. Please don't.)\n\n"
            L"DovahKit will attempt to save error information into \"minidump\" files, which you "
            L"can send to the developer to help them investigate the crash. DovahKit will save a "
            L"small minidump. If you want, it can also save a full minidump containing everything "
            L"DovahKit has in memory, but that could potentially take up multiple gigabytes.\n\n"
            L"Do you want to save both minidumps?",
            nullptr,
            MB_YESNO | MB_ICONERROR | MB_SYSTEMMODAL | MB_SETFOREGROUND
         );
         user_wants_large = response == IDYES;
      }

      wchar_t temp_folder[MAX_PATH]; // %TEMP%
      wchar_t filename[MAX_PATH];    // varies from moment to moment
      wchar_t error_dialog_text[512 + (std::extent<decltype(filename)>::value * 2)];

      SYSTEMTIME local_time;
      GetLocalTime(&local_time);

      // Create a folder to stopre our minidumps in.
      GetTempPathW(std::extent<decltype(temp_folder)>::value, temp_folder);
      _snwprintf_s(filename, std::extent<decltype(filename)>::value, L"%s%s", temp_folder, app_name.data());
      CreateDirectoryW(filename, NULL);

      const auto process_handle = GetCurrentProcess();
      const auto process_id     = GetCurrentProcessId();
      const auto thread_id      = GetCurrentThreadId();


      MINIDUMP_CALLBACK_INFORMATION callback_info = {
         .CallbackRoutine = &_minidump_callback,
         .CallbackParam   = 0,
      };
      MINIDUMP_EXCEPTION_INFORMATION exception_info = {
         .ThreadId          = thread_id,
         .ExceptionPointers = this->dispatch.exception_info,
         .ClientPointers    = TRUE,
      };

      auto p_exception_info = &exception_info;
      if (!this->dispatch.exception_info)
         p_exception_info = nullptr;

      bool saved_small = false;
      bool saved_large = false;

      {
         _snwprintf_s(
            filename,
            std::extent<decltype(filename)>::value,
            L"%s%s\\v%s-%04d%02d%02d-%02d%02d%02d-PID%ld-TID%ld-SMALL.dmp",
            temp_folder,
            app_name.data(),
            app_version.data(),
            local_time.wYear,
            local_time.wMonth,
            local_time.wDay,
            local_time.wHour,
            local_time.wMinute,
            local_time.wSecond,
            process_id,
            thread_id
         );
         HANDLE dump_file = CreateFileW(
            filename,
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_WRITE | FILE_SHARE_READ,
            nullptr,
            CREATE_ALWAYS,
            0,
            NULL
         );
         if (dump_file != INVALID_HANDLE_VALUE)
            saved_small = MiniDumpWriteDump(process_handle, process_id, dump_file, small_minidump_flags, p_exception_info, nullptr, &callback_info);
      }
      if (user_wants_large) {
         _snwprintf_s(
            filename,
            std::extent<decltype(filename)>::value,
            L"%s%s\\v%s-%04d%02d%02d-%02d%02d%02d-PID%ld-TID%ld-LARGE.dmp",
            temp_folder,
            app_name.data(),
            app_version.data(),
            local_time.wYear,
            local_time.wMonth,
            local_time.wDay,
            local_time.wHour,
            local_time.wMinute,
            local_time.wSecond,
            process_id,
            thread_id
         );
         HANDLE dump_file = CreateFileW(
            filename,
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_WRITE | FILE_SHARE_READ,
            nullptr,
            CREATE_ALWAYS,
            0,
            NULL
         );
         if (dump_file != INVALID_HANDLE_VALUE)
            saved_large = MiniDumpWriteDump(process_handle, process_id, dump_file, large_minidump_flags, p_exception_info, nullptr, &callback_info);
      }

      _snwprintf_s(
         filename,
         std::extent<decltype(filename)>::value,
         L"%s%s\\v%s-%04d%02d%02d-%02d%02d%02d-PID%ld-TID%ld-",
         temp_folder,
         app_name.data(),
         app_version.data(),
         local_time.wYear,
         local_time.wMonth,
         local_time.wDay,
         local_time.wHour,
         local_time.wMinute,
         local_time.wSecond,
         process_id,
         thread_id
      );

      #define HOW_TO_COPY_ONE_FILE_PATH \
         L"If you press Ctrl + C while this dialog is open, you should be able to copy its text, to get the file path."

      #define HOW_TO_COPY_MULTIPLE_FILE_PATHS \
         L"If you press Ctrl + C while this dialog is open, you should be able to copy its text, to get the file paths."

      #define DONT_CLOBBER_PREVIOUS_ERROR_INFORMATION \
         L"If DovahKit asked you to copy any other error information, then make sure you save that somewhere first, " \
         L"so you can send everything to DovahKit's developer."

      #define PLEASE_SAVE_COPIES_OF_YOUR_FILES \
         L"If you were working with any of your own files at the time of the crash (e.g. a mod you were making), " \
         L"please also save a copy of those files somewhere, in case DovahKit's developer asks to look at those " \
         L"files' exact data (i.e. at the time of the crash) later on."

      if (saved_small) {
         if (saved_large) {
            _snwprintf_s(
               error_dialog_text,
               std::extent<decltype(error_dialog_text)>::value,
               (
                  L"DovahKit has crashed. The \"small\" minidump was saved to this file:\n\n"
                  L"%sSMALL.dmp\n\n"
                  L"The \"large\" minidump was saved to this file:\n\n"
                  L"%sLARGE.dmp\n\n"
                  HOW_TO_COPY_MULTIPLE_FILE_PATHS
                  L" " DONT_CLOBBER_PREVIOUS_ERROR_INFORMATION
                  L" " PLEASE_SAVE_COPIES_OF_YOUR_FILES
               ),
               filename,
               filename
            );
         } else if (!user_wants_large) {
            _snwprintf_s(
               error_dialog_text,
               std::extent<decltype(error_dialog_text)>::value,
               (
                  L"DovahKit has crashed. The \"small\" minidump was saved to this file:\n\n"
                  L"%sSMALL.dmp\n\n"
                  HOW_TO_COPY_ONE_FILE_PATH
                  L" " DONT_CLOBBER_PREVIOUS_ERROR_INFORMATION
                  L" " PLEASE_SAVE_COPIES_OF_YOUR_FILES
               ),
               filename
            );
         } else {
            _snwprintf_s(
               error_dialog_text,
               std::extent<decltype(error_dialog_text)>::value,
               (
                  L"DovahKit has crashed. The \"small\" minidump was saved to this file:\n\n"
                  L"%sSMALL.dmp\n\n"
                  L"DovahKit failed to save the \"large\" minidump.\n\n"
                  HOW_TO_COPY_ONE_FILE_PATH
                  L" " DONT_CLOBBER_PREVIOUS_ERROR_INFORMATION
                  L" " PLEASE_SAVE_COPIES_OF_YOUR_FILES
               ),
               filename
            );
         }
      } else if (saved_large) {
         _snwprintf_s(
            error_dialog_text,
            std::extent<decltype(error_dialog_text)>::value,
            (
               L"DovahKit has crashed. It failed to save a \"small\" minidump, but strangely, it seems to have "
               L"saved the \"large\" minidump just fine. You can find the \"large\" minidump here:\n\n"
               L"%sLARGE.dmp\n\n"
               HOW_TO_COPY_ONE_FILE_PATH
               L" " DONT_CLOBBER_PREVIOUS_ERROR_INFORMATION
               L" " PLEASE_SAVE_COPIES_OF_YOUR_FILES
            ),
            filename
         );
      } else {
         wcscpy_s(
            error_dialog_text,
            std::extent<decltype(error_dialog_text)>::value,
            L"DovahKit has crashed. Unfortunately, it was unable to output any \"minidump\" files that could "
            L"be used to debug the crash.\n\n"
            L"Please contact DovahKit's developer and tell them about this crash. Try to remember as much as "
            L"you can about what you were doing when the crash happened."
            L" " L"If DovahKit asked you to copy any other error information, please send that to DovahKit's developer."
            L" " PLEASE_SAVE_COPIES_OF_YOUR_FILES
         );
      }
      MessageBoxW(
         NULL,
         error_dialog_text,
         nullptr,
         MB_OK | MB_ICONERROR | MB_SYSTEMMODAL | MB_SETFOREGROUND
      );
   }

   void core::_wait_for_sentinel_acknowledge() {
      std::unique_lock<std::mutex> lock(this->respond.status.mutex);
      this->respond.cv.wait(lock, [this]() { return this->respond.status.sent; });
   }

   void core::_sentinel_acknowledge() {
      {
         auto lock = std::lock_guard(this->respond.status.mutex);
         this->respond.status.sent = true;
      }
      this->respond.cv.notify_all();
   }
}
