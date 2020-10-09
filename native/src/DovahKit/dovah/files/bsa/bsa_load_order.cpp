#include "bsa_load_order.h"
#include "bsa_archive.h"
#include "threads.h"

namespace dovah {
   bsa_load_order::~bsa_load_order() {
      for (auto* archive : this->archives)
         if (archive)
            delete archive;
      this->archives.clear();
   }

   bsa_archive* bsa_load_order::_make_archive(const std::filesystem::path& name) {
      auto* archive = new bsa_archive;
      if (name.is_absolute())
         archive->set_path(name);
      else {
         std::filesystem::path combined = this->base_path;
         combined /= name;
         archive->set_path(combined);
      }
      return archive;
   }
   void bsa_load_order::append_archive(const std::filesystem::path& name) {
      auto* archive = this->_make_archive(name);
      this->archives.push_back(archive);
   }
   void bsa_load_order::prepend_archive(const std::filesystem::path& name) {
      auto* archive = this->_make_archive(name);
      this->archives.insert(this->archives.begin(), archive);
   }
   bsa_archive* bsa_load_order::remove_archive(const std::filesystem::path& name, bool even_if_loaded, bool delete_archive) {
      std::filesystem::path resolved = name;
      if (!resolved.is_absolute())
         resolved = this->base_path / resolved;
      auto& list = this->archives;
      auto  it   = list.begin();
      for (; it != list.end(); ++it) {
         auto* archive = *it;
         if (!archive)
            continue;
         if (resolved == archive->get_path())
            break;
      }
      if (it == list.end())
         return nullptr;
      auto* archive = *it;
      if (!even_if_loaded && archive->is_open())
         return nullptr;
      if (delete_archive) {
         delete archive;
         return nullptr;
      }
      return archive;
   }

   void bsa_load_order::load_archives() {
      if (this->loading)
         return;
      this->loading = true;
      this->working_load_options = this->load_options;
      auto& options = this->working_load_options;
      //
      for (auto* loader : this->threads)
         if (loader)
            delete loader;
      this->threads.clear();
      this->threads.resize(options.thread_count);
      for (auto*& entry : this->threads)
         entry = new bsa_threaded_reader(*this);
      //
      int i = 0;
      for (auto* archive : this->archives) {
         if (!archive)
            continue;
         this->threads[i]->add_archive(archive);
         if (++i >= this->threads.size())
            i = 0;
      }
      for (auto* loader : this->threads)
         loader->start();
   }
   void bsa_load_order::wait_for_archive_load_to_finish() {
      if (!this->loading)
         return;
      for (auto* loader : this->threads)
         if (loader)
            loader->wait_for();
   }
   void bsa_load_order::abort_archive_load() {
      this->aborted = true;
   }
   bool bsa_load_order::is_archive_load_aborted() const noexcept {
      return this->aborted;
   }
   bool bsa_load_order::is_archive_load_in_progress() const noexcept {
      return this->loading;
   }
   void bsa_load_order::on_thread_complete(bsa_threaded_reader& reader) {
      if (!this->loading)
         return;
      auto guard = std::lock_guard(this->completion_check_lock);
      for (auto* loader : this->threads)
         if (!loader->is_complete())
            return;
      this->loading = false;
   }
}