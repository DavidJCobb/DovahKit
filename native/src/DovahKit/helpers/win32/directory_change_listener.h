#pragma once
#include <functional>
#include <string>
#include <thread>
#include <vector>

namespace cobb::win32 {
   // NOTE: Not super optimal: one thread per instance.
   class directory_change_listener {
      private:
         using HANDLE = void*;
      public:
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
               bool created       = false;
               bool last_accessed = false;
               bool last_modified = false;
            } any_file_timestamps_changed;
            bool security_changed = false;
         };

         // If we're able to determine that something in the directory changed, but we can't determine 
         // precisely what, then the listener will be invoked with an empty vector. Your only recourse 
         // in that scenario is to re-crawl the directory manually.
         using listener_type = std::function<void(const std::vector<change_notification>&)>;

      public:
         directory_change_listener(std::wstring directory_path, const notification_filters&, bool watch_subtree = false);
         ~directory_change_listener();

         // WARNING: The listeners run on this class's worker thread, not on the thread that registered them!
         void add_listener(listener_type);

      protected:
         std::thread worker;

         HANDLE directory = NULL;
         struct {
            HANDLE begin_teardown = nullptr;
            HANDLE reconfigured   = nullptr;
         } events;
         notification_filters filters;
         std::vector<listener_type> listeners;

         static void _thread_handler(directory_change_listener& self);

         static std::vector<change_notification> _build_change_notifications(const void* src);
         void _dispatch_change_notifications(const std::vector<change_notification>&);
   };
}