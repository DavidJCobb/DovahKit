#pragma once
#include <atomic>
#include <chrono>
#include <filesystem>
#include <mutex>
#include <string>
#include <vector>
#include "lu/passkey.h"
namespace dovah {
   namespace bsa {
      class archive;
      class thread_for_parsing;
   }
   class asset_file_buffer;
}

namespace dovah::bsa {
   class archive_collection {
      public:
         ~archive_collection();

         struct load_options {
            size_t thread_count = 4;
         };
         struct load_results {
            std::chrono::high_resolution_clock::duration time_spent_loading;
            std::chrono::high_resolution_clock::duration time_spent_waiting;
         };

      protected:
         std::vector<bsa::archive*> _archives;
         std::filesystem::path      _data_directory;
         struct {
            std::vector<thread_for_parsing*> threads;
            std::mutex completion_check_lock;
            std::atomic<bool> aborted = false;
            bool              loading = false;
            struct {
               std::chrono::high_resolution_clock::time_point when_loading_started;
               std::chrono::high_resolution_clock::time_point when_waiting_started;
            } timestamps;
            load_options options;
            load_results results;
         } _load_state;

      public:
         constexpr const std::filesystem::path& set_data_directory() const noexcept { return this->_data_directory; }
         void set_data_directory(const std::filesystem::path&);

         void append_archive(const std::filesystem::path&);
         void prepend_archive(const std::filesystem::path&);
         void insert_archive(size_t, const std::filesystem::path&);

         void load_archives(const load_options& = {});
         load_results&& wait_for_load_to_finish();
         void abort_load();
         bool is_load_aborted() const noexcept;
         bool is_load_in_progress() const noexcept; // an aborted load is still "in progress" if any loader threads have yet to join

         asset_file_buffer lookup_file(const std::filesystem::path&, bool check_for_loose_file = true) const;
         asset_file_buffer lookup_file(const std::string&, bool check_for_loose_file = true) const;

      public: // passkeyed
         void _on_thread_complete(lu::passkey<thread_for_parsing>, thread_for_parsing&);
   };
}