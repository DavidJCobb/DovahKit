#include "./directory_change_listener.h"
#include <array>
#include "../windows.h"

namespace {
   constexpr const size_t change_buffer_size = 64 * 1024;

   constexpr DWORD filters_to_mask(const cobb::win32::directory_change_listener::notification_filters& f) {
      DWORD mask = 0;
      if (f.file_list_changed)
         mask |= FILE_NOTIFY_CHANGE_FILE_NAME;
      if (f.subfolder_list_changed)
         mask |= FILE_NOTIFY_CHANGE_DIR_NAME;
      if (f.any_file_size_changed)
         mask |= FILE_NOTIFY_CHANGE_SIZE;
      if (f.any_file_timestamps_changed.last_modified)
         mask |= FILE_NOTIFY_CHANGE_LAST_WRITE;
      if (f.any_file_timestamps_changed.last_accessed)
         mask |= FILE_NOTIFY_CHANGE_LAST_ACCESS;
      if (f.any_file_timestamps_changed.created)
         mask |= FILE_NOTIFY_CHANGE_CREATION;
      if (f.security_changed)
         mask |= FILE_NOTIFY_CHANGE_SECURITY;
      return mask;
   }
}

namespace cobb::win32 {
   directory_change_listener::directory_change_listener(std::wstring directory_path, const notification_filters&, bool watch_subtree) {
      directory = CreateFileW(
         directory_path.data(),
         FILE_LIST_DIRECTORY | GENERIC_READ,
         FILE_SHARE_WRITE | FILE_SHARE_READ,
         nullptr,
         OPEN_EXISTING,
         FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
         NULL
      );
      if (directory == INVALID_HANDLE_VALUE) {
         throw std::system_error(std::error_code(GetLastError(), std::system_category()));
      }
      events.begin_teardown = CreateEvent(nullptr, true, false, nullptr);
      if (!events.begin_teardown)
         throw std::system_error(std::error_code(GetLastError(), std::system_category()));
      events.reconfigured   = CreateEvent(nullptr, false, false, nullptr);
      if (!events.reconfigured)
         throw std::system_error(std::error_code(GetLastError(), std::system_category()));

      worker = std::thread(&_thread_handler, *this);
   }
   directory_change_listener::~directory_change_listener() {
      if (worker.joinable()) {
         SetEvent(events.begin_teardown);
         worker.join();
      }
      CloseHandle(events.begin_teardown);
      CloseHandle(events.reconfigured);
      CloseHandle(directory);
   }

   void directory_change_listener::add_listener(listener_type lis) {
      this->listeners.push_back(lis);
   }

   /*static*/ void directory_change_listener::_thread_handler(directory_change_listener& self) {
      HANDLE     directory = NULL;
      OVERLAPPED overlapped;
      {
         memset(&overlapped, 0, sizeof(overlapped));
         overlapped.hEvent = CreateEvent(nullptr, true, false, nullptr);
         if (!overlapped.hEvent) {
            throw std::system_error(std::error_code(GetLastError(), std::system_category()));
         }
      }

      std::vector<uint32_t> buffer(change_buffer_size / 4); // uint32_t to ensure correct alignment

      DWORD bytes_transferred = 0;
      bool  pending = false;
      bool  proceed = true;
      std::array<HANDLE, 3> all_waitables = { overlapped.hEvent, self.events.begin_teardown, self.events.reconfigured };
      do {
         pending = ReadDirectoryChangesW(
            self.directory,
            buffer.data(),
            buffer.size(),
            true,
            filters_to_mask(self.filters),
            &bytes_transferred,
            &overlapped,
            nullptr
         );
         if (!pending) {
            // TODO: GetLastError()?
         }

         auto wait_result = WaitForMultipleObjects(all_waitables.size(), all_waitables.data(), false, INFINITE);
         switch (wait_result) {
            case WAIT_OBJECT_0:
               {
                  if (!GetOverlappedResult(self.directory, &overlapped, &bytes_transferred, true)) {
                     // TODO: GetLastError()?
                     break;
                  }
                  pending = false;
                  if (bytes_transferred == 0) {
                     self._dispatch_change_notifications({});
                  } else {
                     self._dispatch_change_notifications(_build_change_notifications(buffer.data()));
                  }
               }
               break;
            case WAIT_OBJECT_0 + 1: // teardown
               {
                  proceed = false;
               }
               break;
            case WAIT_OBJECT_0 + 2: // reconfigure
               //
               // Our desired filters or other parameters have changed, so let's cancel our ongoing 
               // wait for change notifications, so we can start a new wait with the new parameters.
               //
               {
                  CancelIo(self.directory);
                  if (!GetOverlappedResult(self.directory, &overlapped, &bytes_transferred, true)) {
                     break;
                  }
                  //
                  // Hm. We actually already have a result, so we may as well return it.
                  //
                  pending = false;
                  if (bytes_transferred == 0) {
                     self._dispatch_change_notifications({});
                  } else {
                     self._dispatch_change_notifications(_build_change_notifications(buffer.data()));
                  }
               }
               break;

            case WAIT_FAILED:
               // TODO: GetLastError()?
               break;
         }
      } while (proceed);

      if (pending) {
         //
         // You can cancel asynchronous I/O operations tied to a specific OVERLAPPED structure, 
         // but you still have to wait for the canceled operations to complete afterwards.
         // 
         CancelIo(self.directory);
         GetOverlappedResult(self.directory, &overlapped, &bytes_transferred, true);
      }
      CloseHandle(overlapped.hEvent);
   }

   /*static*/ std::vector<directory_change_listener::change_notification> directory_change_listener::_build_change_notifications(const void* src) {

      std::vector<change_notification> dst;

      //
      // The documentation for ReadDirectoryChangesW doesn't actually make any guarantees 
      // about the sequence of the separate notifications for renaming (one for the file's 
      // old name and one for its new name). Most of the time, you'll get the old-name 
      // notification immediately before the new-name notification. However, here's a 
      // seventeen-year-old forum post from a guy who saw unusual behavior in that regard:
      // 
      //    https://microsoft.public.win32.programmer.kernel.narkive.com/VRvo8ZZ5/readdirectorychangesw-renaming-notifications
      // 
      //    I can say based on my experiments of watching an entire volume that usually
      //    (99.9%?) you get FILE_ACTION_RENAMED_OLD_NAME immediately followed by
      //    FILE_ACTION_RENAMED_NEW_NAME. But there have been times that I've seen a
      //    FILE_ACTION_MODIFIED for some unrelated file in between. Obviously you've 
      //    got to have other file activity going on for intervening events to appear.
      // 
      //    Are the two rename events guaranteed to be in the same read buffer?
      // 
      //    Are they guaranteed to always come in the old name, new name order?
      // 
      //    Are they guaranteed to not be interleaved with other rename events?
      // 
      // So since the documentation makes no promises about any of these edge cases, we 
      // have to do some cursed nonsense. We'll keep track of the last old name we saw, 
      // but only act on it when we find a corresponding new name. If we see a second 
      // old name before we see a new name, then we'll start swallowing new names since 
      // at that point there'll be at least two interleaved renames and no way to sort 
      // them out.
      //
      std::wstring_view last_seen_old_name;
      size_t interleaved_renames = 0;
            
      auto* info = (const FILE_NOTIFY_INFORMATION*)src;
      do {
         auto filename = std::wstring_view(info->FileName, info->FileNameLength);

         if (info->Action == FILE_ACTION_RENAMED_OLD_NAME) {
            if (last_seen_old_name.empty()) {
               last_seen_old_name = filename;
            } else {
               last_seen_old_name = {};
               ++interleaved_renames;
            }
         } else if (info->Action == FILE_ACTION_RENAMED_NEW_NAME) {
            if (interleaved_renames) {
               --interleaved_renames;
            } else {
               dst.emplace_back(change_notification{
                  .name = std::wstring(filename),
                  .type = watch_event::renamed,

                  .old_name = std::wstring(last_seen_old_name),
               });
               last_seen_old_name = {};
            }
         } else {
            auto& result = dst.emplace_back();
            result.name = filename;
            switch (info->Action) {
               case FILE_ACTION_ADDED:
                  result.type = watch_event::created;
                  break;
               case FILE_ACTION_REMOVED:
                  result.type = watch_event::removed;
                  break;
               case FILE_ACTION_MODIFIED:
                  result.type = watch_event::modified;
                  break;
            }
         }

         if (info->NextEntryOffset == 0)
            break;
         info = (const FILE_NOTIFY_INFORMATION*)((const uint8_t*)info + info->NextEntryOffset);
      } while (true);

      return dst;
   }
   void directory_change_listener::_dispatch_change_notifications(const std::vector<change_notification>& list) {
      for (auto& listener : this->listeners)
         listener(list);
   }
}