#pragma once
#include <functional>
#include <mutex>
#include <string>
#include <string_view>
#include <optional>
#include <thread>
#include <vector>

// untested; overengineered; maybe worth throwing away?

namespace cobb::win32 {
   class multi_directory_change_listener {
      private:
         using HANDLE = void*;

      public:
         static constexpr const size_t max_watch_operations_per_instance = 62; // MAX_WAITABLE_OBJECTS minus the two events we use

         enum class watch_event {
            created,
            modified,
            renamed,
            removed,
         };

         struct change_notification {
            std::wstring name;
            watch_event  type;
            std::wstring old_name; // when renamed
         };

         struct notification_filters {
            bool file_list_changed      = false; // file added/renamed/removed
            bool subfolder_list_changed = false; // subfolder added/renamed/removed
            bool any_file_size_changed  = false;
            struct _ {
               constexpr const bool operator==(const _&) const noexcept = default; // this language is really stupid sometimes, man
               bool created       = false;
               bool last_accessed = false;
               bool last_modified = false;
            } any_file_timestamps_changed;
            bool security_changed = false;

            constexpr const bool operator==(const notification_filters&) const noexcept = default;
         };

      public:
         ~multi_directory_change_listener();

         void watch_directory(std::wstring_view path, const notification_filters&, bool subtree = false);
         void stop_watching_directory(std::wstring_view path);
         
      protected:
         struct _watch_operation {
            size_t operation_id = 0;
            struct {
               HANDLE       handle = NULL;
               std::wstring path;
            } directory;
            notification_filters filters;
            bool watch_subtree = false;

            bool canceled     = false;
            bool reconfigured = false;
         };

         std::thread worker;
         //
         mutable std::mutex lock;
         std::vector<_watch_operation> operations;
         size_t next_operation_id = 0;
         struct {
            HANDLE begin_teardown     = NULL;
            HANDLE operations_changed = NULL;
         } events;

         std::vector<std::function<void(const std::vector<change_notification>&)>> listeners;

         static void _build_change_notifications(std::vector<change_notification>& dst, const void* src);
         void _dispatch_change_notifications(const std::vector<change_notification>&);
         void _ensure_worker();
         _watch_operation* _lookup_op_for_directory(std::wstring_view path);

         static void _thread_handler(multi_directory_change_listener& self);
   };
}
