#include "./multi_directory_change_listener.h"
#include <cstdint>
#include "../windows.h"

#include "./async_operation_set.h"

// Based vaguely on: https://stackoverflow.com/a/43665226
// but with an attempt to support reconfiguring/removing/etc. directories from the listener

namespace cobb::win32 {
   namespace {
      constexpr const size_t change_buffer_size = 64 * 1024;

      constexpr DWORD filters_to_mask(const multi_directory_change_listener::notification_filters& f) {
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

   multi_directory_change_listener::~multi_directory_change_listener() {
      if (this->worker.joinable()) {
         SetEvent(this->events.begin_teardown);
         this->worker.join();
      }
      CloseHandle(this->events.begin_teardown);
      CloseHandle(this->events.operations_changed);
   }

   void multi_directory_change_listener::watch_directory(std::wstring_view path, const notification_filters& filters, bool subtree) {
      {
         std::lock_guard guard(this->lock);

         auto* prior = this->_lookup_op_for_directory(path);
         if (prior) {
            if (prior->filters == filters && prior->watch_subtree == subtree) {
               //
               // Already watching this directory with the specified options.
               //
               return;
            }
            prior->filters       = filters;
            prior->watch_subtree = subtree;
            prior->reconfigured  = true;
            //
            // Need to fall through to signalling the worker thread, so that it can redo the 
            // wait operations.
            //
         } else {
            HANDLE handle;
            if (path.size() < MAX_PATH) {
               handle = CreateFileW(
                  path.data(),
                  FILE_LIST_DIRECTORY | GENERIC_READ,
                  FILE_SHARE_WRITE | FILE_SHARE_READ,
                  nullptr,
                  OPEN_EXISTING,
                  FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
                  NULL
               );
            } else {
               auto safe = std::wstring(LR"(\\?\)");
               safe += path;

               handle = CreateFileW(
                  safe.data(),
                  FILE_LIST_DIRECTORY | GENERIC_READ,
                  FILE_SHARE_WRITE | FILE_SHARE_READ,
                  nullptr,
                  OPEN_EXISTING,
                  FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
                  NULL
               );
            }
            if (handle == INVALID_HANDLE_VALUE) {
               static_assert(false, "TODO: GetLastError() and throw exception");
            }

            auto& watch = this->operations.emplace_back();
            watch.operation_id = this->next_operation_id++;
            watch.directory = {
               .handle = handle,
               .path   = std::wstring(path),
            };
            watch.filters       = filters;
            watch.watch_subtree = subtree;
         }
      }
      this->_ensure_worker();
      SetEvent(this->events.operations_changed);
   }
   void multi_directory_change_listener::stop_watching_directory(std::wstring_view path) {
      {
         std::lock_guard guard(this->lock);

         auto* prior = this->_lookup_op_for_directory(path);
         if (!prior)
            return;
         prior->canceled = true;
      }
      this->_ensure_worker();
      SetEvent(this->events.operations_changed);
   }

   /*static*/ void multi_directory_change_listener::_build_change_notifications(std::vector<change_notification>& dst, const void* src) {

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
      // old name before we see a new name, then we'll start skipping new names since we 
      // have no guarantee that the renames aren't interleaved.
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
   }
   void multi_directory_change_listener::_dispatch_change_notifications(const std::vector<change_notification>& list) {
      for (auto& listener : this->listeners) {
         (listener)(list);
      }
   }
   void multi_directory_change_listener::_ensure_worker() {
      if (!this->worker.joinable()) {
         this->events.begin_teardown     = CreateEvent(nullptr, false, false, nullptr);
         this->events.operations_changed = CreateEvent(nullptr, false, false, nullptr);
         this->worker = std::thread(&_thread_handler, *this);
      }
   }

   /*static*/ void multi_directory_change_listener::_thread_handler(multi_directory_change_listener& self) {
      std::vector<uint32_t> buffer(change_buffer_size / 4); // uint32_t as underlying type to ensure correct alignment for ReadDirectoryChangesW

      async_operation_set async_ops;
      async_ops.add_extra_waitables(
         self.events.begin_teardown,
         self.events.operations_changed
      );

      bool proceed = true;
      do {
         {
            std::lock_guard guard(self.lock);

            for (auto& op : self.operations) {
               async_ops.queue_operation(op.operation_id, [&buffer, &op](HANDLE& file, OVERLAPPED& overlapped) -> bool {
                  return ReadDirectoryChangesW(
                     op.directory.handle,
                     buffer.data(),
                     buffer.size(),
                     op.watch_subtree,
                     filters_to_mask(op.filters),
                     nullptr, // DWORD* bytes returned for synchronous calls; unused/undefined for overlapped
                     &overlapped,
                     nullptr
                  );
               });
            }
         }
         async_ops.wait(
            [&self, &buffer](HANDLE file, OVERLAPPED& overlapped, DWORD bytes_received) {
               if (bytes_received == 0) {
                  self._dispatch_change_notifications({});
               } else {
                  std::vector<change_notification> results;
                  _build_change_notifications(results, buffer.data());
                  self._dispatch_change_notifications(results);
               }
            },
            [&self, &async_ops, &buffer, &proceed](HANDLE signalled) {
               if (signalled == self.events.begin_teardown) {
                  proceed = false;
               }
               if (signalled == self.events.operations_changed) {
                  std::lock_guard guard(self.lock);

                  for (auto& op : self.operations) {
                     if (op.canceled) {
                        async_ops.destroy_operation(op.operation_id);
                        continue;
                     }
                     if (op.reconfigured) {
                        async_ops.cancel_operation(op.operation_id);
                        async_ops.queue_operation(op.operation_id, [&buffer, &op](HANDLE& file, OVERLAPPED& overlapped) -> bool {
                           return ReadDirectoryChangesW(
                              op.directory.handle,
                              buffer.data(),
                              buffer.size(),
                              op.watch_subtree,
                              filters_to_mask(op.filters),
                              nullptr, // DWORD* bytes returned for synchronous calls; unused/undefined for overlapped
                              &overlapped,
                              nullptr
                           );
                        });
                     }
                  }
               }
            }
         );
      } while (proceed);
   }
}