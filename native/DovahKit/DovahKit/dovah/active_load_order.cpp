#include "./active_load_order.h"
#include "lu/arrays/make_filled.h"
#include "lu/ascii/equals_i.h"
#include "./_impl/form_stubs/make_one_way_use_bidirectional.h"
#include "./bsa/archive_collection.h"
#include "./exceptions/load_order_errors/active_file_is_a_master_and_there_are_plugins.h"
#include "./exceptions/load_order_errors/desired_active_file_was_not_in_the_load_order.h"
#include "./exceptions/load_order_errors/some_files_are_too_new.h"
#include "./facts/game/game_supports_light_plugins.h"
#include "./facts/game/hardcoded_form_ids_ignore_record_id_prefix.h"
#include "./facts/game_setting_types.h"
#include "./load_order_interfaces/file_load.h"
#include "./load_order_processes/build_form_stubs_from_file/process.h"
#include "./load_order_processes/reparent_persistent_refs/process.h"
#include "./load_order_processes/use_info_build/worker_thread.h"
#include "./localization/string_file_bundle.h"
#include "./tes/file.h"
#include "./tes/file_load_order.h"
#include "./types/file_prefix.h"
#include "./utils/files/tes/normalize_next_record_id.h"
#include "./utils/get_ini_defined_bsa_list.h"
#include "./form_stub.h"

namespace dovah {
   active_load_order::active_load_order() {
   }
   active_load_order::~active_load_order() {
      this->_files.active = nullptr;
      for (auto* file : this->_files.list) {
         delete file;
      }
      this->_files.list.clear();
      if (auto* p = this->_files.dummies.hardcoded) {
         this->_files.dummies.hardcoded = nullptr;
         delete p;
      }
      if (auto* p = this->_files.dummies.none_stubs) {
         this->_files.dummies.none_stubs = nullptr;
         delete p;
      }
      for (auto* bundle : this->_files.string_file_bundles) {
         delete bundle;
      }
      this->_files.string_file_bundles.clear();
      if (auto* p = this->_bsa_collection) {
         this->_bsa_collection = nullptr;
         delete p;
      }
   }

   bsa::archive_collection* active_load_order::get_or_create_bsa_collection() noexcept {
      auto* c = this->_bsa_collection;
      if (!c)
         c = this->_bsa_collection = new bsa::archive_collection;
      return c;
   }

   void active_load_order::load(
      tes::file_load_order& flo,
      std::string_view active_file_name,
      const callback_sets::data_load& callbacks
   ) {
      //
      // Check file versions.
      //
      {
         const auto max_version = facts::max_file_version(this->_game);

         std::vector<std::string> too_new_files;
         for (auto& info_ptr : flo.masters)
            if (info_ptr->file_version > max_version)
               too_new_files.push_back(info_ptr->filename);
         for (auto& info_ptr : flo.plugins)
            if (info_ptr->file_version > max_version)
               too_new_files.push_back(info_ptr->filename);

         if (!too_new_files.empty()) {
            throw exceptions::load_order_errors::some_files_are_too_new(this->_data_directory, this->_game, std::move(too_new_files));
         }
      }
      //
      // Force the active file to the end of the load order.
      //
      if (!active_file_name.empty()) {
         const bool is_master = flo.has_master(active_file_name);
         if (is_master && !flo.plugins.empty()) {
            throw exceptions::load_order_errors::active_file_is_a_master_and_there_are_plugins(this->_data_directory, this->_game);
         }
         auto& list = is_master ? flo.masters : flo.plugins;
         auto  it   = std::find_if(list.begin(), list.end(), [&active_file_name](const auto& info_ptr) {
            return lu::ascii::equals_i(active_file_name, info_ptr->filename);
         });
         if (it != list.end()) {
            // NOTE: This is an internal program error, not an error of circumstance.
            throw exceptions::load_order_errors::desired_active_file_was_not_in_the_load_order(this->_data_directory, this->_game, active_file_name);
         }

         auto info_ptr = std::move(*it);
         list.erase(it);
         list.push_back(std::move(info_ptr));
      }
      if (this->_bsa_collection) {
         //
         // If we have a BSA collection, then prepend the Skyrim.ini-specified archives followed by the 
         // archives for each loaded file. We add to the start of the list in order to allow a frontend 
         // to load additional archives that override the INI and file ones.
         //
         this->_bsa_collection->set_data_directory(this->_data_directory);
         //
         uint32_t count = 0;
         //
         auto list = utils::get_ini_defined_bsa_list(this->_game);
         for (auto& path : list)
            this->_bsa_collection->insert_archive(count++, path);
         //
         for (auto& info_ptr : flo.masters) {
            const auto& filename = info_ptr->filename;
            if (filename.empty())
               continue;
            std::filesystem::path name = filename;
            name.replace_extension(".bsa");
            this->_bsa_collection->insert_archive(count++, name);
         }
         for (auto& info_ptr : flo.plugins) {
            const auto& filename = info_ptr->filename;
            if (filename.empty())
               continue;
            std::filesystem::path name = filename;
            name.replace_extension(".bsa");
            this->_bsa_collection->insert_archive(count++, name);
         }
         //
         this->_bsa_collection->load_archives();
      }
      this->_make_hardcoded_forms();
      {  // Ensure enough space to store all forms without reallocating
         uint32_t total = 0;
         for (auto& info_ptr : flo.masters)
            total += info_ptr->record_and_group_count;
         for (auto& info_ptr : flo.plugins)
            total += info_ptr->record_and_group_count;
         this->_forms._reserve({}, total);
      }
      //
      // The load order itself is fine. Start loading the files!
      //
      load_order_interfaces::file_load intfc(*this, callbacks);
      {
         using list_t = decltype(flo.masters);
         //
         auto lambda = [this, &active_file_name, &callbacks, &intfc](list_t& list) {
            for (auto& info_ptr : list) {
               auto* file = new tes::file({}, *this);
               this->_files.list.push_back(file);
               if (!active_file_name.empty() && lu::ascii::equals_i(active_file_name, info_ptr->filename)) {
                  this->_files.active = file;
               }
               try {
                  file->open(this->_data_directory / info_ptr->filename);
                  {
                     load_order_processes::build_form_stubs_from_file::process process({
                        *this,
                        callbacks,
                        *file
                     });
                     process.execute();
                  }
               } catch (...) {
                  if (this->_bsa_collection)
                     this->_bsa_collection->abort_load();
                  throw;
               }
               //
               // TODO: Split file loading into these steps:
               //
               // 1. Load the header.
               // 2. Verify that there aren't any unexpected masters (i.e. file wasn't altered 
               //    between constructing the load order and now). If there are, fail.
               // 3. Load the rest of the file.
               //
            }
         };
         lambda(flo.masters);
         lambda(flo.plugins);
      }
      this->_build_use_info(); // may throw
      {
         load_order_processes::reparent_persistent_refs::process process;
         process.execute(this->_forms);
      }
      if (!facts::game_supports_light_plugins(this->_game)) {
         //
         // If the user chooses to save a Skyrim Classic file as a Skyrim Special file, then we will 
         // at that time (attempt to) enable light plug-in support. If any already-loaded files were 
         // wrongfully flagged as "light," then doing so will cause tons of errors: we'll retroactively 
         // honor their "light" flag, but without having moved their forms to the 0xFE range. The 
         // solution? Clear the flag for all files during a Classic-only load.
         //
         for (auto* file : this->_files.list)
            file->header().flags.unset_each_of<tes::file_header_flag::light>();
      }
      //
      // Now let's deal with the active file.
      //
      if (!this->_files.active && this->_files.list.size() < (facts::game_supports_light_plugins(this->_game) ? 253 : 254)) {
         auto* file = new tes::file({}, *this);
         this->_files.active = file;
         this->_files.list.push_back(file);
      }
      if (this->_files.active) {
         auto first_free = this->_forms.find_first_free_form_id_in_active_file();
         if (first_free) {
            this->_files.active->header().nextFormID = utils::files::tes::normalize_next_record_id(first_free, this->file_prefix_for(*this->_files.active));
         }
      }
      //
      // Wait for BSAs to finish loading, and then load localized strings.
      // TODO: We could multi-thread this.
      //
      if (this->_bsa_collection) {
         this->_bsa_collection->wait_for_load_to_finish();

         const auto&  tes_list = this->_files.list;
         auto&        str_list = this->_files.string_file_bundles;
         const size_t size     = tes_list.size();
         str_list.reserve(size);
         for (size_t i = 0; i < size; ++i) {
            auto* p = new localization::string_file_bundle(tes_list[i]->path().filename().string());
            str_list.push_back(p);
         }
         for (size_t i = 0; i < size; ++i) {
            str_list[i]->load(*this->_bsa_collection);
         }
      }
      //
      // We're done!
      //
   }

   #pragma region Finding files
      file_prefix active_load_order::file_prefix_for(const std::filesystem::path& filename) const noexcept {
         uint8_t     heavy = 0xFF;   // start at -1
         uint16_t    light = 0xFFFF; // start at -1
         file_prefix prefix;
         for (auto* f : this->_files.list) {
            bool is_light = facts::game_supports_light_plugins(this->_game) && f->header().flags.has_all_of<tes::file_header_flag::light>();
            bool is_equal = f->path().filename() == filename;
            if (is_light) {
               ++light;
               if (is_equal)
                  prefix = file_prefix::make_light(light);
            } else {
               ++heavy;
               if (is_equal)
                  prefix = file_prefix::make_heavy(heavy);
            }
            if (is_equal) {
               if (!facts::hardcoded_form_ids_ignore_record_id_prefix(this->_game, f->header().file_version))
                  prefix.set_can_co_opt_hardcoded_range(true);
               break;
            }
         }
         return prefix;
      }
      file_prefix active_load_order::file_prefix_for(const tes::file& file) const noexcept {
         uint8_t     heavy = 0xFF;   // start at -1
         uint16_t    light = 0xFFFF; // start at -1
         file_prefix prefix;
         for (auto* f : this->_files.list) {
            bool is_light = facts::game_supports_light_plugins(this->_game) && f->header().flags.has_all_of<tes::file_header_flag::light>();
            bool is_equal = f == &file;
            if (is_light) {
               ++light;
               if (is_equal)
                  prefix = file_prefix::make_light(light);
            } else {
               ++heavy;
               if (is_equal)
                  prefix = file_prefix::make_heavy(heavy);
            }
            if (is_equal) {
               if (!facts::hardcoded_form_ids_ignore_record_id_prefix(this->_game, f->header().file_version))
                  prefix.set_can_co_opt_hardcoded_range(true);
               break;
            }
         }
         return prefix;
      }
      file_prefix active_load_order::expected_active_file_prefix_post_save(std::optional<bool> pretend_is_or_isnt_light) const noexcept {
         if (!this->_files.active)
            return {};

         bool can_co_opt_hardcoded_id_space = false;
         if (!facts::hardcoded_form_ids_always_ignore_record_id_prefix(this->_game)) {
            //
            // Even in a game where hardcoded form IDs aren't prefix-agnostic, the active file 
            // can only *use* that ID space if it has at least one master. If the active file 
            // has no masters, then its own record ID prefix is 0x00, and it still has to avoid 
            // the range [00000001, 000007FF].
            //
            if (this->_files.list.size() > 1) {
               can_co_opt_hardcoded_id_space = true;
            }
         }
         file_prefix prefix;
         {
            uint8_t  heavy = 0xFF;   // start at -1
            uint16_t light = 0xFFFF; // start at -1
            for (auto* f : this->_files.list) {
               bool is_light = facts::game_supports_light_plugins(this->_game) && f->header().flags.has_all_of<tes::file_header_flag::light>();
               bool is_equal = f == this->_files.active;
               if (is_equal && pretend_is_or_isnt_light.has_value()) {
                  is_light = pretend_is_or_isnt_light.value();
               }
               if (is_light) {
                  ++light;
                  prefix = file_prefix::make_light(light);
               } else {
                  ++heavy;
                  prefix = file_prefix::make_heavy(heavy);
               }
               if (is_equal)
                  break;
            }
         }
         prefix.set_can_co_opt_hardcoded_range(can_co_opt_hardcoded_id_space);
         return prefix;
      }
      //
      int active_load_order::index_of_file(const tes::file& f) const noexcept {
         size_t size = this->_files.list.size();
         for (size_t i = 0; i < size; ++i)
            if (this->_files.list[i] == &f)
               return (int)i;
         return -1;
      }
      int active_load_order::index_of_prefix(file_prefix prefix) const noexcept {
         if (prefix.is_undefined())
            return -1;
         size_t size = this->_files.list.size();
         if (prefix.is_light()) {
            uint16_t target = prefix.light_prefix();
            uint16_t count  = -1;
            for (size_t i = 0; i < size; ++i) {
               auto* file = this->_files.list[i];
               if (file->header().flags.has_all_of<tes::file_header_flag::light>()) {
                  if (++count == target)
                     return (int)i;
               }
            }
            return -1;
         }
         uint8_t target = prefix.load_prefix();
         uint8_t count  = -1;
         for (size_t i = 0; i < size; ++i) {
            auto* file = this->_files.list[i];
            if (!file->header().flags.has_all_of<tes::file_header_flag::light>()) {
               if (++count == target)
                  return (int)i;
            }
         }
         return -1;
      }
      //
      const tes::file* active_load_order::get_file_by_prefix(file_prefix p) const noexcept {
         auto i = this->index_of_prefix(p);
         if (i < 0)
            return nullptr;
         return this->get_file_by_index(i);
      }
      const tes::file* active_load_order::get_file_by_index(int i) const noexcept {
         auto size = this->_files.list.size();
         if (i < 0) {
            i += (int)size;
            if (i < 0)
               return nullptr;
         } else if (i >= size)
            return nullptr;
         return this->_files.list[i];
      }
      //
      bool active_load_order::file_is_active(const tes::file& file) const noexcept {
         return &file == this->_files.active;
      }
      bool active_load_order::has_file(const std::filesystem::path& filename) const noexcept {
         for (auto* f : this->_files.list)
            if (f->path().filename() == filename)
               return true;
         return false;
      }
      bool active_load_order::has_non_active_file(const std::filesystem::path& filename) const noexcept {
         if (auto* f = this->_files.active)
            if (f->path().filename() == filename)
               return false;
         return this->has_file(filename);
      }
   #pragma endregion
   
   void active_load_order::_build_use_info() {
      auto* dummy = this->_files.dummies.none_stubs = tes::file::_make_none_stubs_dummy_file({}, this->_game);
      //
      // We generate Use Info using two passes. First, we divide all forms across multiple 
      // threads; each thread generates outbound Use Info for the forms. Then, a single 
      // thread scans the outbound Use Info and uses that to generate inbound Use Info.
      //
      // If we want to generate outbound Use Info for a given form, then we only need to 
      // update that form. However, if we want to generate *inbound* Use Info *from* a 
      // given form, we must update each form it refers to. As such, we can't multi-thread 
      // the generation of inbound Use Info unless we put a lock on each individual form.
      // 
      // Note that parent form relationships, tracked during load, exist as outbound use 
      // info.
      //
      {
         auto workers = lu::arrays::make_filled<load_order_processes::use_info_build::worker_thread, 8>(*this);
         {
            auto count_per = this->_forms.count_all_forms() / workers.size();
            for (auto& entry : workers)
               entry.reserve(count_per + 1);
         }
         int which_thread = 0;
         // Inbound first, since we can multi-thread that
         this->_forms.for_each_form([&workers, &which_thread](form_stub& stub) {
            workers[which_thread].add_to_queue(&stub);
            if (++which_thread >= workers.size())
               which_thread = 0;
         });
         for (auto& thread : workers)
            thread.start();
         for (auto& thread : workers)
            thread.wait_for();
         {
            const size_t no_offset = workers.size();

            size_t earliest_offset = no_offset;
            for (size_t i = 0; i < workers.size(); ++i) {
               auto& thread = workers[i];
               if (!thread.exception.captured)
                  continue;

               if (earliest_offset == no_offset) {
                  earliest_offset = i;
               } else {
                  if (thread.exception.thrown_at_file_offset < workers[earliest_offset].exception.thrown_at_file_offset)
                     earliest_offset = i;
               }
            }
            if (earliest_offset != no_offset) {
               auto captured = std::move(workers[earliest_offset].exception.captured);
               std::rethrow_exception(captured);
            }
         }
      }
      //
      // These next steps have to be single-threaded. First, we need to build any 
      // none-stubs that are needed; then, we need to use outbound use info to 
      // build inbound use info. None-stubs need to be built first so that the 
      // references to them are properly made bidirectional.
      //
      this->_forms._make_none_stubs({}, *this->_files.dummies.none_stubs);
      this->_forms.for_each_form([](form_stub& user) {
         if (user.form_type == form_type::none) // none-type forms (which are usually, but not always, none-stubs) should never use other forms, so we can skip them
            return;
         impl::_form_stubs::make_all_one_way_uses_bidirectional({}, user);
      });
   }
   void active_load_order::_make_hardcoded_forms() {
      auto* dummy = this->_files.dummies.hardcoded = tes::file::_make_hardcoded_dummy_file({}, this->_game);
      this->_forms._make_hardcoded_forms({}, *dummy);
   }
}