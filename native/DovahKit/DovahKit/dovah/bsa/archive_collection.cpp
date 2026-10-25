#include "./archive_collection.h"
#include <cassert>
#include <fstream>
#include "dovah/types/asset_file_buffer.h"
#include "dovah/utils/files/bsa/path_to_hashes.h"
#include "./archive.h"
#include "./thread_for_parsing.h"

namespace dovah::bsa {
   archive_collection::~archive_collection() {
      assert(!this->is_load_in_progress());
      for (auto* archive : this->_archives)
         delete archive;
      this->_archives.clear();
   }

   void archive_collection::set_data_directory(const std::filesystem::path& path) {
      this->_data_directory = path;
   }

   void archive_collection::append_archive(const std::filesystem::path& path) {
      this->insert_archive(this->_archives.size(), path);
   }
   void archive_collection::prepend_archive(const std::filesystem::path& path) {
      this->insert_archive(0, path);
   }
   void archive_collection::insert_archive(size_t at, const std::filesystem::path& path) {
      if (at > this->_archives.size())
         at = this->_archives.size();
      auto p = path;
      if (!p.is_absolute())
         p = this->_data_directory / p;
      if (!std::filesystem::exists(p))
         return;
      auto   it          = this->_archives.emplace(this->_archives.begin() + at, nullptr);
      auto*& archive_ptr = *it;
      archive_ptr = new archive;
      archive_ptr->set_path(std::move(p));
   }

   void archive_collection::load_archives(const load_options& options) {
      assert(options.thread_count > 0);
      if (this->_load_state.loading)
         return;

      auto& results = this->_load_state.results;
      auto& threads = this->_load_state.threads;

      this->_load_state.loading = true;
      this->_load_state.options = options;
      for (auto* loader : threads)
         if (loader)
            delete loader;
      threads.clear();
      threads.resize(options.thread_count);
      for (auto*& entry : threads)
         entry = new thread_for_parsing(*this);
      
      int i = 0;
      for (auto* archive : this->_archives) {
         this->_load_state.threads[i]->add_archive(*archive);
         if (++i >= this->_load_state.threads.size())
            i = 0;
      }

      results = {};
      this->_load_state.timestamps.when_loading_started = std::chrono::high_resolution_clock::now();
      for (auto* loader : this->_load_state.threads)
         loader->start();
   }
   archive_collection::load_results archive_collection::wait_for_load_to_finish() {
      if (!this->_load_state.loading)
         return load_results{};
      this->_load_state.timestamps.when_waiting_started = std::chrono::high_resolution_clock::now();
      for (auto* loader : this->_load_state.threads)
         if (loader)
            loader->wait_for();
      this->_load_state.results.time_spent_waiting = std::chrono::high_resolution_clock::now() - this->_load_state.timestamps.when_waiting_started;
      return this->_load_state.results;
   }
   void archive_collection::abort_load() {
      this->_load_state.aborted = true;
   }
   bool archive_collection::is_load_aborted() const noexcept {
      return this->_load_state.aborted;
   }
   bool archive_collection::is_load_in_progress() const noexcept {
      return this->_load_state.loading;
   }

   asset_file_buffer archive_collection::lookup_file(const std::filesystem::path& path, bool check_for_loose_file) const {
      return this->lookup_file(path.string(), check_for_loose_file);
   }
   asset_file_buffer archive_collection::lookup_file(const std::string& path, bool check_for_loose_file) const {
      if (check_for_loose_file && !this->_data_directory.empty()) {
         std::filesystem::path target = this->_data_directory / path;
         std::ifstream         stream(target, std::ios_base::in | std::ios_base::binary | std::ios_base::ate);
         if (stream.is_open()) {
            auto eof = stream.tellg();
            stream.seekg(0);
            
            lu::memory::basic_buffer buffer;
            buffer.resize(eof);
            stream.read((char*)buffer.data(), eof);

            return asset_file_buffer(std::move(buffer));
         }
         //
         // If we get here, then there was no (usable) loose file. Fall through to 
         // scanning the BSAs.
         //
      }

      auto opt_path_hashes = utils::files::bsa::path_to_hashes(path);
      if (!opt_path_hashes.has_value())
         return {};
      auto [folder_hash, file_hash] = opt_path_hashes.value();

      auto& list = this->_archives;
      for (auto it = list.rbegin(); it != list.rend(); ++it) {
         auto* archive = *it;
         if (!archive || !archive->is_open())
            continue;
         auto result = archive->lookup_file(folder_hash, file_hash);
         if (!result.empty())
            return result;
      }
      return {};
   }

   void archive_collection::_on_thread_complete(lu::passkey<thread_for_parsing>, thread_for_parsing& reader) {
      auto guard = std::lock_guard(this->_load_state.completion_check_lock);
      if (!this->_load_state.loading)
         return;
      for (auto* loader : this->_load_state.threads)
         if (!loader->is_complete())
            return;
      this->_load_state.results.time_spent_loading = std::chrono::high_resolution_clock::now() - this->_load_state.timestamps.when_loading_started;
      this->_load_state.loading = false;
   }
}