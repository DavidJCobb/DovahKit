#include "file_load_order.h"
#include "threaded_load_order_use_info_builder.h"
#include "../../helpers/performance.h"
#include "../../helpers/strings.h"
#include "../../helpers/unordered_map.h"
#include "../form_stub.h"
#include "../form_stub_helpers.h"
#include "tes_file_reading/file.h"
#include "tes_file_reading/file_header.h"
#include "tes_file_writing/file_writer.h"
#include "../forms/factories/construct.h"
#include "../forms/factories/hardcoded.h"
#include "../forms/Form.h"
#include "../logging.h"
#include "bsa/bsa_load_order.h"
#include "../utils/get_ini_defined_bsa_list.h"
#include "../utils/get_user_language_name.h"
#include "../localization/localized_string_store.h"
#include <fstream>

namespace dovah {
   file_load_order::~file_load_order() {
      this->normalizer.delete_contents();
      this->queued_load.files.clear();
      this->queued_load.active_file.clear();
      //
      for (auto& f : this->forms.forms) {
         auto stub = f.second;
         if (stub)
            delete stub;
      }
      this->forms.forms.clear();
      for (auto& m : this->forms_by_type)
         m.forms.clear();
      this->active_file_forms.forms.clear();
      for (auto& m : this->active_file_forms_by_type)
         m.forms.clear();
      //
      for (auto* f : this->files)
         delete f;
      this->files.clear();
      this->active_file = nullptr;
      if (auto file = this->hardcoded_forms_file) {
         delete file;
         this->hardcoded_forms_file = nullptr;
      }
      //
      if (this->archives) {
         delete this->archives;
         this->archives = nullptr;
      }
   }

   file_load_order::_save_load_lock_guard::_save_load_lock_guard(file_load_order& o, file_load_order::save_load_type desired) : owner(o) {
      auto& lock  = owner.save_load_state.type;
      auto  prior = save_load_type::none;
      if (!lock.compare_exchange_strong(prior, desired))
         return;
      this->success = true;
   }
   file_load_order::_save_load_lock_guard::~_save_load_lock_guard() {
      if (this->success)
         owner.save_load_state.type = save_load_type::none;
   }

   void file_load_order::adopt_archive_list(bsa_load_order& list) noexcept {
      this->archives = &list;
   }

   #pragma region File loading
   void file_load_order::_make_hardcoded_forms() {
      this->hardcoded_forms_file = new tes_file_reading::file_reader(*this);
      this->hardcoded_forms_file->header.details |= tes_file_reading::file_reader::detail_flag::is_hardcoded_dummy;
      add_hardcoded_forms_to_load_order(*this);
   }
   void file_load_order::_accept_hardcoded_form(form_stub* stub) noexcept {
      auto& type = this->forms_by_type[stub->formType];
      std::lock_guard<std::mutex> guard_for_form_type(type.lock);
      std::lock_guard<std::mutex> guard_for_all_forms(this->forms.lock);
      //
      stub->file = this->hardcoded_forms_file;
      stub->flags |= form_stub::flag::is_hardcoded;
      //
      bare_form_id_t formID = stub->formID;
      type.forms[formID] = stub;
      this->forms.forms[formID] = stub;
   }
   void file_load_order::_build_use_info() {
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
      auto& builders = this->use_info_build_threads;
      {
         auto guard = std::lock_guard(this->use_info_build_threads_lock);
         for (auto& entry : builders)
            entry = new threaded_load_order_use_info_builder;
      }
      uint32_t which_thread = 0;
      // Inbound first, since we can multi-thread that
      for (auto it = this->forms.forms.begin(); it != this->forms.forms.end(); ++it) {
         form_stub* stub = it->second;
         builders[which_thread]->add_to_queue(stub);
         if (++which_thread > 7)
            which_thread = 0;
      }
      for (auto* thread : builders)
         thread->start();
      for (auto*& thread : builders)
         thread->wait_for();
      {
         auto guard = std::lock_guard(this->use_info_build_threads_lock);
         for (auto*& entry : builders) {
            delete entry;
            entry = nullptr;
         }
         this->save_load_state.flags |= save_load_flag::use_info_outbound_complete;
      }
      // Outbound next; has to be single-threaded
      for (auto it = this->forms.forms.begin(); it != this->forms.forms.end(); ++it) {
         it->second->send_inbound_refs();
      }
   }

   uint8_t file_load_order::load_order_prefix_for(const loaded_file* file) const noexcept {
      if (file == this->hardcoded_forms_file)
         return 0;
      uint8_t size = this->files.size();
      for (uint8_t i = 0; i < size; i++)
         if (file == this->files[i])
            return i;
      return invalid_load_prefix;
   }
   uint8_t file_load_order::guided_load_order_prefix_for(const loaded_file* file) const noexcept {
      //
      // Intended for use during loading. If (this->loadingIndex) is non-zero, then it's the 
      // index of the file currently being loaded; we check that index in the load order first 
      // before searching the full load order. (If we're currently loading file 00, then we 
      // start at zero, which is the same as searching the full load order anyway.)
      //
      if (file == this->hardcoded_forms_file)
         return 0;
      uint8_t size = this->files.size();
      uint8_t li   = this->save_load_state.loading_index;
      if (li && li < size)
         if (file == this->files[li])
            return li;
      for (uint8_t i = 0; i < size; i++)
         if (file == this->files[i])
            return i;
      return invalid_load_prefix;
   }

   void file_load_order::queue_file(const std::string& name) {
      auto& list = this->queued_load.files;
      auto it = std::find(list.begin(), list.end(), name);
      if (it == list.end())
         list.push_back(name);
   }
   void file_load_order::unqueue_file(const std::string& name) {
      auto& list = this->queued_load.files;
      auto it = std::find(list.begin(), list.end(), name);
      if (it != list.end())
         list.erase(it);
   }
   void file_load_order::queue_active_file(const std::string& name) {
      this->queued_load.active_file = name;
   }
   bool file_load_order::load_queued_files() {
      this->load_error = file_read_error();
      //
      _save_load_lock_guard save_load_lock_guard(*this, save_load_type::is_loading);
      if (!save_load_lock_guard) {
         this->load_error.code = file_read_error::error_code::cannot_load_right_now;
         return false;
      }
      this->save_load_state.flags = save_load_flag::none;
      //
      if (!this->base_path.empty()) {
         char end = *this->base_path.rbegin();
         if (end != '/' && end != '\\')
            this->base_path += '/';
      }
      //
      this->normalizer.base_path   = this->base_path;
      this->normalizer.active_file = this->queued_load.active_file;
      for (auto it = this->queued_load.files.begin(); it != this->queued_load.files.end(); ++it) {
         if (!this->normalizer.add(this->load_error, *it))
            return false;
      }
      if (this->load_error.defined())
         return false;
      if (!this->queued_load.active_file.empty()) {  // Force the active file to the end of the load order
         //
         // The active file must be at the end of the load order, and cannot be the master of any 
         // other file. Both of these limitations stem from the fact that we only keep the last-
         // loaded version of any record in memory. If the active file is the master to another 
         // file, then that file can override forms defined in the active file; if we save those 
         // overrides into the active file, then we can end up with dangling references between 
         // forms (if the overrides referred to other forms in the overriding file) or with a 
         // cyclical reference (if we try to make the overriding file a master of the active file).
         //
         // If the active file A and another file B both override the same form in some common 
         // master C, and if B loads after A, then we'll have a problem: because we only retain 
         // the last override to load, when we encounter B's override, we will delete A's override 
         // from memory, leaving a dangling pointer in the active file form maps. We could solve 
         // this by testing every form against those maps, but that's not performant.
         //
         auto& name = this->queued_load.active_file;
         bool isMaster = this->normalizer.has_master(name);
         if (isMaster && !this->normalizer.plugins.empty()) {
            this->load_error.code    = file_read_error::error_code::active_file_is_master_and_there_are_plugins;
            this->load_error.file    = this->queued_load.active_file;
            this->load_error.message = "The active file must be at the end of the load order. However, it is impossible to move it there, because it is ESM-flagged and there are non-ESM-flagged files in the load order.";
            return false;
         }
         auto* list = &this->normalizer.plugins;
         if (isMaster)
            list = &this->normalizer.masters;
         //
         auto it = std::find_if(list->begin(), list->end(), [&name](loaded_header* file) { return cobb::strieq(name, file->name); });
         assert(it != list->end() && "How is it not in the list?!");
         loaded_header* header = *it;
         list->erase(it);
         list->push_back(header);
      }
      if (this->archives) {
         this->archives->set_base_path(this->base_path);
         //
         auto list = utils::get_ini_defined_bsa_list();
         for (auto& path : list)
            this->archives->prepend_archive(path);
         //
         for (auto* header : this->normalizer.masters) {
            if (header->name.empty())
               continue;
            std::filesystem::path name = header->name;
            name.replace_extension(".bsa");
            {
               std::filesystem::path path = this->archives->get_base_path() / name;
               std::ifstream stream(path);
               if (!stream.good())
                  continue;
            }
            this->archives->prepend_archive(name);
         }
         for (auto* header : this->normalizer.plugins) {
            if (header->name.empty())
               continue;
            std::filesystem::path name = header->name;
            name.replace_extension(".bsa");
            {
               std::filesystem::path path = this->archives->get_base_path() / name;
               std::ifstream stream(path);
               if (!stream.good())
                  continue;
            }
            this->archives->prepend_archive(name);
         }
         //
         this->archives->load_archives();
      }
      this->_make_hardcoded_forms();
      {  // Ensure enough space to store all forms without reallocating
         uint32_t total = 0;
         for (auto* header : this->normalizer.masters)
            total += header->record_and_group_count;
         for (auto* header : this->normalizer.plugins)
            total += header->record_and_group_count;
         this->forms.forms.reserve((size_t)(total * 1.1) + 0x800);
      }
      {
         dovah::logging::print_line("Final load order:");
         for (auto* header : this->normalizer.masters)
            dovah::logging::print_line("[M] %s", header->name.c_str());
         for (auto* header : this->normalizer.plugins)
            dovah::logging::print_line("[P] %s", header->name.c_str());
      }
      for (auto* header : this->normalizer.masters) {
         std::string path = this->base_path + header->name;
         auto file = new tes_file_reading::file_reader(*this);
         this->save_load_state.loading_index = this->files.size();
         this->files.push_back(file);
         if (!this->queued_load.active_file.empty() && cobb::strieq(this->queued_load.active_file, header->name)) {
            this->active_file = file;
            this->active_file_index = this->save_load_state.loading_index;
         }
         if (!file->load(path.c_str())) {
            auto fn = header->name;
            this->load_error.file = fn;
            if (file->error.defined()) {
               this->load_error = file->error;
            } else {
               this->load_error.code    = file_read_error::error_code::unknown_error;
               this->load_error.message = "Failed to load a master.";
            }
            if (this->archives)
               this->archives->abort_archive_load();
            return false;
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
      for (auto* header : this->normalizer.plugins) {
         std::string path = this->base_path + header->name;
         auto file = new tes_file_reading::file_reader(*this);
         this->save_load_state.loading_index = this->files.size();
         this->files.push_back(file);
         if (!this->queued_load.active_file.empty() && cobb::strieq(this->queued_load.active_file, header->name)) {
            this->active_file = file;
            this->active_file_index = this->save_load_state.loading_index;
         }
         if (!file->load(path.c_str())) {
            auto fn = header->name;
            this->load_error.file = fn;
            if (file->error.defined()) {
               this->load_error = file->error;
            } else {
               this->load_error.code    = file_read_error::error_code::unknown_error;
               this->load_error.message = "Failed to load a master.";
            }
            if (this->archives)
               this->archives->abort_archive_load();
            return false;
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
      this->save_load_state.flags |= save_load_flag::loading_is_complete;
      this->save_load_state.loading_index = 0;
      //
      this->_build_use_info();
      this->save_load_state.flags |= save_load_flag::use_info_build_is_complete;
      //
      if (!this->active_file && this->files.size() < 254) { // TODO: the cutoff should be 253 if any of the loaded files are ESLs or SSE files
         auto file = new tes_file_reading::file_reader(*this);
         this->active_file = file;
         // TODO: How should we handle file_reader::nextFormID?
         this->files.push_back(file);
      }
      if (this->active_file) {
         auto first_free = this->find_first_free_form_id_in_active_file();
         if (first_free)
            this->active_file->header.nextFormID = first_free;
      }
      //
      if (this->archives) {
         this->archives->wait_for_archive_load_to_finish();
         //
         auto language = utils::get_user_language_name();
         for (auto* file : this->files) {
            if (!file || (file->header.flags & tes_file_flag::localized_string_table) == 0)
               continue;
            file->localization_data = new localized_string_store(*this->archives, file->get_filename());
            file->localization_data->set_default_language(language);
            file->localization_data->open_language_files(language);
         }
      }
      //
      return !this->load_error.defined();
   }

   file_load_order::form_id_status file_load_order::accept_form_stub(form_stub* stub) noexcept {
      auto& type = this->forms_by_type[stub->formType];
      std::lock_guard<std::mutex> guard_for_form_type(type.lock);
      std::lock_guard<std::mutex> guard_for_all_forms(this->forms.lock);
      //
      uint32_t formID;
      auto     result = this->local_formID_to_global_formID(stub, formID);
      switch (result) {
         case form_id_status::out_of_bounds:
         case form_id_status::missing_master:
            return result;
      }
      if (formID == 0)
         return form_id_status::null_is_not_allowed;
      if ((formID & plugin_form_id_mask) == 0)
         stub->flags |= form_stub::flag::is_hardcoded; // This FormStub overrides a hardcoded form.
      //
      // update the map of forms by type:
      //
      form_stub*& target = type.forms[formID];
      if (target) // is this an override?
         delete target; // delete the overridden form stub
      target = stub;
      //
      // update the map of all forms as well:
      //
      this->forms.forms[formID] = stub;
      //
      stub->formID = formID;
      //
      if (this->active_file && (stub->file == this->active_file || (formID >> 0x18) == this->active_file_index)) {
         this->active_file_forms.forms[formID] = stub;
         auto& at = this->active_file_forms_by_type[stub->formType];
         at.forms[formID] = stub;
      }
      //
      return form_id_status::valid;
   }
   #pragma endregion

   float file_load_order::assess_load_progress() const noexcept {
      constexpr float use_info_proportion = 0.2F;
      //
      bool l = this->save_load_state.flags & save_load_flag::loading_is_complete;
      bool u = this->save_load_state.flags & save_load_flag::use_info_build_is_complete;
      if (l) {
         if (u)
            return 1.0F;
         float progress = 0.0F;
         //
         {
            auto guard = std::lock_guard(this->use_info_build_threads_lock);
            for (auto* thread : this->use_info_build_threads)
               if (thread)
                  progress += thread->assess_load_progress();
         }
         if (this->save_load_state.flags & save_load_flag::use_info_outbound_complete)
            return NAN;
         progress /= this->use_info_build_threads.size();
         //
         progress *= use_info_proportion;
         progress += (1.0F - use_info_proportion);
         return progress;
      }
      int count = this->files.size();
      if (this->active_file && this->active_file->get_filename().empty())
         //
         // If we added an implicit active file to the load order, don't count it.
         //
         --count;
      if (count <= 0)
         return 0.0F;
      //
      float per_file = 1.0F / count;
      float progress = per_file * this->save_load_state.loading_index;
      auto* current  = this->files[this->save_load_state.loading_index];
      if (!current)
         return progress;
      auto this_file = current->assess_load_progress();
      if (!isnan(this_file))
         progress += per_file * this_file;
      //
      progress *= (1.0F - use_info_proportion);
      return progress;
   }

   bool file_load_order::is_form_loading_blocked(const form_stub* stub) const noexcept {
      if (this->save_load_state.type != save_load_type::none)
         return true;
      return false;
   }

   uint32_t file_load_order::count_forms_of_type(form_type_t ft) const noexcept {
      if (ft < this->forms_by_type.size()) {
         auto& list = this->forms_by_type[ft].forms;
         return list.size();
      }
      return 0;
   }
   bool file_load_order::has_form(bare_form_id_t formID) const noexcept {
      if (formID == 0)
         return false;
      auto& list = this->forms.forms;
      auto  it   = list.find(formID);
      return (it != list.end());
   }
   uint8_t file_load_order::index_of_loaded_file(const std::string& filename) const noexcept {
      auto size = this->files.size();
      for (uint8_t i = 0; i < size; i++) {
         auto& name = this->files[i]->get_filename();
         if (cobb::strieq(name, filename))
            return i;
      }
      return invalid_load_prefix;
   }
   form_stub* file_load_order::get_form(bare_form_id_t formID) const noexcept {
      if (formID == 0)
         return nullptr;
      auto& list = this->forms.forms;
      auto  it   = list.find(formID);
      if (it != list.end())
         return it->second;
      return nullptr;
   }
   form_stub* file_load_order::get_form(form_type_t formType, bare_form_id_t formID) const noexcept {
      if (formID == 0)
         return nullptr;
      if (formType < this->forms_by_type.size()) {
         auto& list = this->forms_by_type[formType].forms;
         auto  it   = list.find(formID);
         if (it != list.end())
            return it->second;
      }
      return nullptr;
   }
   form_stub* file_load_order::get_form_of_probable_type(form_type_t formType, bare_form_id_t formID) const noexcept {
      if (formID == 0)
         return nullptr;
      if (formType < this->forms_by_type.size()) {
         auto& list = this->forms_by_type[formType].forms;
         auto  it   = list.find(formID);
         if (it != list.end())
            return it->second;
      }
      return this->get_form(formID);
   }
   bool file_load_order::for_each_form_of_type(form_type_t formType, std::function<bool(form_stub*)> functor) {
      if (formType < this->forms_by_type.size()) {
         auto& list = this->forms_by_type[formType].forms;
         for (auto it = list.begin(); it != list.end(); ++it) {
            if (functor(it->second))
               return true;
         }
      }
      return false;
   }
   bool file_load_order::form_is_from_active_file(const form_stub* stub) const noexcept {
      if (stub->is_edited())
         return true;
      return stub->file == this->active_file;
   }
   bool file_load_order::form_is_from_active_file(bare_form_id_t formID) const noexcept {
      auto stub = this->get_form(formID);
      return this->form_is_from_active_file(stub);
   }

   bool file_load_order::active_file_has_name() const noexcept {
      if (!this->active_file)
         return false;
      if (this->active_file->get_filename().empty())
         return false;
      return true;
   }
   uint32_t file_load_order::active_file_form_count() const noexcept {
      uint32_t total = 0;
      for (auto& map : this->active_file_forms_by_type)
         total += map.forms.size();
      return total;
   }
   bool file_load_order::active_file_has_forms_of_type(form_type_t form_type) const noexcept {
      if (form_type < this->active_file_forms_by_type.size()) {
         auto& list = this->active_file_forms_by_type[form_type].forms;
         return !list.empty();
      }
      return false;
   }
   bare_form_id_t file_load_order::find_first_free_form_id_in_active_file(bare_form_id_t id) const noexcept {
      id &= 0x00FFFFFF;
      if (id < 0x00000800)
         id = 0x00000800;
      //
      bare_form_id_t prefix = this->index_of_active_file();
      auto& map = this->active_file_forms.forms;
      if (prefix == invalid_load_prefix)
         return 0;
      if (map.size() >= (0x00FFFFFF - 0x00000800)) // no form IDs available
         return 0;
      prefix = prefix << 0x18;
      id    |= prefix;
      bare_form_id_t max = 0x00FFFFFF | prefix;
      //
      auto& reservations = this->form_creation_request_info.reserved_formIDs;
      auto  guard        = std::lock_guard(this->form_creation_request_info.lock);
      //
      for (; id < max; ++id) {
         if (!cobb::unordered_map_contains(map, id)) {
            auto it = std::find(reservations.begin(), reservations.end(), id);
            if (it != reservations.end())
               continue;
            return id;
         }
      }
      return 0;
   }
   bool file_load_order::for_each_active_file_form_of_type(form_type_t form_type, std::function<bool(form_stub*)> functor) {
      if (form_type < this->active_file_forms_by_type.size()) {
         auto& list = this->active_file_forms_by_type[form_type].forms;
         for (auto it = list.begin(); it != list.end(); ++it) {
            if (functor(it->second))
               return true;
         }
      }
      return false;
   }
   bool file_load_order::for_each_active_file_override_of_type(form_type_t form_type, std::function<bool(form_stub*)> functor) {
      if (form_type >= this->active_file_forms_by_type.size())
         return false;
      auto  prefix = this->index_of_active_file();
      auto& list   = this->active_file_forms_by_type[form_type].forms;
      for (auto it = list.begin(); it != list.end(); ++it) {
         auto stub = it->second;
         if ((stub->formID >> 0x18) == prefix)
            continue;
         if (functor(stub))
            return true;
      }
      return false;
   }
   bool file_load_order::for_each_top_level_form_needing_save(form_type_t form_type, std::function<bool(form_stub*)> functor) {
      if (form_type >= this->forms_by_type.size())
         return false;
      //
      // A form needs to be saved if it has been edited during the current session, 
      // if it was defined in or overridden by the active file, or if any of these 
      // things are true for any of its child or descendant forms.
      //
      // In practice, this means that for form types that can have child forms, we 
      // need to iterate over all loaded forms to check for descendants that meet 
      // the criteria, whereas for form types that cannot have child forms, we only 
      // need to loop over active file forms.
      //
      if (form_types[form_type].flags & form_type_info::flag::can_have_children) {
         auto& list = this->forms_by_type[form_type].forms;
         for (auto it = list.begin(); it != list.end(); ++it) {
            auto* stub = it->second;
            if (stub->file == this->active_file || stub->is_edited() || stub->does_descendant_form_need_save())
               if (functor(stub))
                  return true;
         }
      } else {
         auto& list = this->active_file_forms_by_type[form_type].forms;
         for (auto it = list.begin(); it != list.end(); ++it) {
            auto* stub = it->second;
            if (stub->file == this->active_file || stub->is_edited())
               if (functor(stub))
                  return true;
         }
      }
      return false;
   }
   void file_load_order::get_active_file_name(std::filesystem::path& out) const noexcept {
      out.clear();
      if (this->active_file)
         out = this->active_file->get_filename();
   }
   uint8_t file_load_order::index_of_active_file() const noexcept {
      auto size = this->files.size();
      for (uint8_t i = 0; i < size; i++) {
         if (this->files[i] == this->active_file)
            return i;
      }
      return invalid_load_prefix;
   }
   bool file_load_order::is_defined_or_overridden_in_active_file(const form_stub* stub) const noexcept {
      return stub->file == this->active_file;
   }

   form_stub* file_load_order::create_form_of_type(form_type_t ft) noexcept {
      auto request = this->request_form_creation(ft);
      if (!request.is_valid())
         return nullptr;
      return this->commit_form_creation_request(request);
   }
   form_creation_request file_load_order::request_form_creation(form_type_t ft) noexcept {
      form_creation_request result(*this);
      result.form_type = ft;
      //
      if (!this->active_file) {
         result.error = form_creation_request::error_code::no_active_file;
         return result;
      }
      if (ft >= form_types.size()) {
         result.error = form_creation_request::error_code::bad_form_type_requested;
         return result;
      }
      auto  formID = this->active_file->header.nextFormID;
      auto  prefix = this->index_of_active_file();
      if (prefix == invalid_load_prefix) {
         result.error = form_creation_request::error_code::no_active_file;
         return result;
      }
      formID = (formID & 0x00FFFFFF) | (prefix << 0x18);
      //
      auto  guard = std::lock_guard(this->form_creation_request_info.lock);
      auto& list  = this->form_creation_request_info.reserved_formIDs;
      {
         auto guard = std::lock_guard(this->forms.lock);
         if (std::find(list.begin(), list.end(), formID) != list.end()) // if the form ID is reserved, then we need to find a new one
            formID = 0;
         if (!(formID & 0x00FFFFFF) || cobb::unordered_map_contains(this->forms.forms, formID)) {
            formID = this->find_first_free_form_id_in_active_file();
            if (!formID) { // no form ID available
               result.error = form_creation_request::error_code::no_form_id_available;
               return result;
            }
         }
      }
      //
      list.push_back(formID);
      result.formID = formID;
      return result;
   }
   form_stub* file_load_order::commit_form_creation_request(form_creation_request& request) noexcept {
      bare_form_id_t formID = request.formID;
      if (!formID)
         return nullptr;
      assert(!cobb::unordered_map_contains(this->forms.forms, formID) && "We should have reserved this form ID when the request was initialized. How did it end up taken?");
      //
      if (request.child_of) {
         auto parent_type = request.child_of->formType;
         auto child_type  = request.form_type;
         bool error       = false;
         //
         if (form_type_info::form_type_is_reference(child_type)) { // validate parent/child relationships
            error = parent_type != form_type::cell;
         } else if (child_type == form_type::cell) {
            error = parent_type != form_type::worldspace;
         } else if (child_type == form_type::topic_info) {
            error = parent_type != form_type::topic;
         } else {
            error = true;
         }
         //
         if (error) {
            request.error = form_creation_request::error_code::invalid_parent_child_relationship;
            return nullptr;
         }
         //
         if (child_type == form_type::cell) { // validate worldspace grid coordinates
            auto* existing = form_stub_helpers::get_worldspace_cell_by_grid(request.child_of, request.cell_grid_coordinates.x, request.cell_grid_coordinates.y);
            if (existing) {
               request.error = form_creation_request::error_code::exterior_grid_coordinates_already_taken;
               return nullptr;
            }
         }
      } else {
         if (form_type_info::form_type_is_reference(request.form_type)) {
            request.error = form_creation_request::error_code::cannot_create_reference_with_no_parent_cell;
            return nullptr;
         }
      }
      if (request.clone_of && request.form_type == form_type::cell) {
         if (request.child_of) {
            if (request.clone_of->groupInfo.parentFormID == 0) {
               request.error = form_creation_request::error_code::interior_cell_clone_cannot_have_parent;
               return nullptr;
            }
         } else {
            if (request.clone_of->groupInfo.parentFormID != 0) {
               request.error = form_creation_request::error_code::exterior_cell_clone_must_have_parent;
               return nullptr;
            }
         }
      }
      //
      loaded_forms::Form* loaded = nullptr;
      if (!request.clone_of) {
         loaded = create_blank_loaded_form_by_type(request.form_type);
         if (!loaded) {
            request.error = form_creation_request::error_code::unsupported_form_type_requested;
            return nullptr;
         }
      }
      auto* stub = new form_stub;
      stub->formType = request.form_type;
      stub->form     = loaded;
      stub->file     = this->active_file;
      stub->offset   = 0;
      stub->formID   = formID;
      stub->editorID = request.editorID;
      stub->set_edited(true);
      if (loaded)
         loaded->stub = stub;
      //
      stub->groupInfo.gridX = request.cell_grid_coordinates.x;
      stub->groupInfo.gridY = request.cell_grid_coordinates.y;
      //
      {
         auto guard = std::lock_guard(this->forms.lock);
         this->forms.forms[formID] = stub;
         this->forms_by_type[stub->formType].forms[formID] = stub;
         this->active_file_forms.forms[formID] = stub;
         this->active_file_forms_by_type[stub->formType].forms[formID] = stub;
      }
      //
      if (request.child_of) {
         bare_form_id_t parentID = request.child_of->formID;
         stub->groupInfo.parentFormID = parentID;
         stub->replace_outbound_reference(bare_form_id_t(0), parentID, use_info_entry::flag::i_am_child_of); // NOT form_stub::add_outbound_reference; see documentation for that function for why
      }
      //
      if (request.clone_of) {
         auto original = request.clone_of->load();
         if (original) {
            bool result = false;
            loaded = original->clone(*stub, &result);
            if (!result)
               request.error = form_creation_request::error_code::form_created_but_clone_failed;
         }
      } else {
         loaded->setup(*this);
      }
      //
      if (this->active_file) {
         this->active_file->header.nextFormID = this->find_first_free_form_id_in_active_file(request.formID + 1);
      }
      //
      {
         auto  guard = std::lock_guard(this->form_creation_request_info.lock);
         auto& list  = this->form_creation_request_info.reserved_formIDs;
         //
         auto it = std::find(list.begin(), list.end(), formID);
         assert(it != list.end() && "Wait, did we just create a form stub for a form ID that wasn't reserved? That shouldn't have happened!");
         if (it != list.end())
            list.erase(it);
      }
      request.formID = 0;
      if (this->on_form_create)
         (this->on_form_create)(stub);
      return stub;
   }
   form_duplication_request file_load_order::request_form_duplication() noexcept {
      return form_duplication_request(*this);
   }
   form_deletion_request file_load_order::request_form_deletion(form_stub& target) noexcept {
      return form_deletion_request(*this, target);
   }
   
   bool file_load_order::for_each_load_order_filename(std::function<bool(std::filesystem::path, bool is_active_file)> functor) {
      std::filesystem::path filename;
      for (auto* file : this->files) {
         filename = file->get_filename();
         if (functor(filename, file == this->active_file))
            return true;
      }
      return false;
   }

   void file_load_order::stub_flagged_as_edited(form_stub* stub) noexcept {
      this->active_file_forms.forms[stub->formID] = stub;
      auto& at = this->active_file_forms_by_type[stub->formType];
      at.forms[stub->formID] = stub;
   }

   file_load_order::form_id_status file_load_order::local_formID_to_global_formID(const loaded_file* file, uint32_t& id) const {
      if ((id & plugin_form_id_mask) == 0) { // hardcoded
         id = id & hardcoded_form_id_mask;
         return form_id_status::valid;
      }
      if (!file) {
         id = 0;
         return form_id_status::missing_master;
      }
      uint8_t local  = file->header.masters.size();
      uint8_t prefix = id >> 0x18;
      if (prefix == local) {
         id = id & 0x00FFFFFF | (this->guided_load_order_prefix_for(file) << 0x18);
         return form_id_status::valid;
      }
      if (prefix > local) {
         id = 0;
         return form_id_status::out_of_bounds;
      }
      auto&   name = file->header.masters[prefix].master;
      uint8_t j    = this->index_of_loaded_file(name);
      if (j == invalid_load_prefix) {
         id = 0;
         return form_id_status::missing_master;
      }
      id = id & 0x00FFFFFF | (j << 0x18);
      return form_id_status::valid;
   }
   file_load_order::form_id_status file_load_order::local_formID_to_global_formID(form_stub* stub, uint32_t& out) const {
      if ((stub->formID & plugin_form_id_mask) == 0) { // Handle form IDs for hardcoded forms.
         //
         // All forms between xx000001 and xx0007FF, inclusive, are hardcoded forms and the 
         // load order prefix is ignored.
         //
         out = stub->formID & hardcoded_form_id_mask;
         return form_id_status::valid;
      }
      if (stub->formType == form_type::setting) {
         //
         // Skyrim.esm contains a GMST record with incorrect form ID 0123C00E. Accordingly, 
         // since GMST form IDs clearly don't matter, we need to just make sure we store 
         // them consistently and otherwise not validate them in any way.
         //
         out = stub->formID & 0x00FFFFFF;
         return form_id_status::valid;
      }
      auto    file = stub->file;
      uint8_t local = file->header.masters.size();
      uint8_t prefix = stub->formID >> 0x18;
      if (prefix == local) {
         out = stub->formID & 0x00FFFFFF | (this->guided_load_order_prefix_for(file) << 0x18);
         return form_id_status::valid;
      }
      if (prefix > local) {
         out = 0;
         return form_id_status::out_of_bounds;
      }
      auto& name = file->header.masters[prefix].master;
      uint8_t j = this->index_of_loaded_file(name);
      if (j == invalid_load_prefix) {
         out = 0;
         return form_id_status::missing_master;
      }
      out = stub->formID & 0x00FFFFFF | (j << 0x18);
      return form_id_status::valid;
   }

   tes_file_header* file_load_order::get_active_file_header() const noexcept {
      if (!this->active_file)
         return nullptr;
      return &this->active_file->header;
   }

   bool file_load_order::save_active_file(std::filesystem::path name_to_use_if_nameless, const dovah::tes_file_writing::write_config* cfg) {
      this->save_error   = file_write_error();
      this->save_warning = file_write_warning();
      if (!this->active_file) {
         this->save_error.code = file_write_error::error_code::no_active_file;
         return false;
      }
      _save_load_lock_guard save_load_lock_guard(*this, save_load_type::is_saving);
      if (!save_load_lock_guard) {
         this->save_error.code = file_write_error::error_code::cannot_save_right_now;
         return false;
      }
      //
      std::filesystem::path filename = this->active_file->get_filename();
      if (filename.empty()) {
         filename = name_to_use_if_nameless;
         if (filename.empty()) {
            this->save_error.code = file_write_error::error_code::no_filename_specified;
            return false;
         }
         this->active_file->set_path(std::filesystem::path(this->base_path) / filename);
      }
      //
      filename = std::filesystem::path(this->base_path) / filename;
      {  // opening the file for writing will clear its contents (which is bad for the user and will break our reading/writing), so we want to ALWAYS write to a temporary file first!
         auto ext = filename.extension().string();
         if (_stricmp(ext.data(), ".tes") == 0) {
            //
            // This normally should never happen. The editor should never allow you to open a *.TES 
            // file directly. You can end up working with one e.g. if a save is successful but we are 
            // unable to replace the file being saved over, but when that happens, we shouldn't be 
            // updating the file_reader's stored filename, so that should still point to the old name.
            //
         } else {
            filename.replace_extension(".tes");
         }
      }
      //
      tes_file_writing::file_writer writer(*this, *this->active_file, cfg);
      writer.open(filename);
      if (writer.write()) {
         //
         // Okay, so we have successfully written the updated form data to a temporary file. Now, we 
         // need to do a few things: we need to close the active file's mapped file view; we need to 
         // replace the old file with our temporary one; and we need to reopen the mapped file view 
         // on the updated file. (The file view needs to be closed because it's shared read access; 
         // it *should* prevent the file from being modified.)
         //
         writer.close(); // so we can move the new file
         this->active_file->close(); // so we can replace the old file
         //
         std::error_code code;
         std::filesystem::rename(filename, this->active_file->get_path(), code);
         bool reopen_result = false;
         if (code) {
            this->save_warning.code     = file_write_warning::warning_code::save_complete_but_to_temporary_file;
            this->save_warning.filename = filename.filename();
            reopen_result = this->active_file->open_mapped_file(filename.string().c_str());
            assert(this->active_file->get_filename() != filename.string() && "file_reader::open_mapped_file should not change the file's stored name. The file should know what it's *supposed* to be called even if, due to an unexpected issue, we have to actually read its contents from a different name.");
         } else {
            reopen_result = this->active_file->open_mapped_file();
         }
         if (!reopen_result) {
            //
            // We were unable to reopen the mapped file view after fully updating the active file, 
            // so we can't load form content for active file form stubs anymore. In other words, the 
            // save completed, but further editing is not possible.
            //
            auto& error = this->save_error;
            error.code = file_write_error::error_code::save_complete_but_reopen_failed;
            return false;
         }
         //
         // The last step, before editing can resume, is to update all of the form stubs that were 
         // saved to the new file.
         //
         for (auto& pair : writer.fixup_data.form_stubs) {
            auto& info = pair.second;
            auto* stub = info.stub;
            stub->offset = info.offset;
            stub->file   = this->active_file;
            if (!stub->is_edited()) {
               //
               // If the form stub was written to the file despite not having been flagged as edited, 
               // it would be because a child/descendant form or parent/ancestor form was edited. 
               // We need to add this form to the active file form list.
               //
               this->active_file_forms.forms[stub->formID] = stub;
               auto& at = this->active_file_forms_by_type[stub->formType];
               at.forms[stub->formID] = stub;
            }
            stub->set_edited(false);
         }
         #if _DEBUG
            for (auto& pair : this->active_file_forms.forms) {
               auto* stub = pair.second;
               assert(!stub->is_edited() && "Somehow we didn't fully commit changes to an active file form stub after a save operation.");
            }
         #endif
      }
      this->save_error = writer.error;
      return !writer.error.defined();
   }

   #pragma region form_creation_request
   form_creation_request::form_creation_request(file_load_order& o) : owner(o) {
   }
   form_creation_request::form_creation_request(form_creation_request&& other) : owner(other.owner) {
      this->formID    = other.formID;
      this->form_type = other.form_type;
      this->child_of  = other.child_of;
      this->clone_of  = other.clone_of;
      this->error     = other.error;
      //
      this->editorID  = other.editorID;
      this->cell_grid_coordinates = other.cell_grid_coordinates;
      //
      other.formID = 0;
   }
   form_creation_request::~form_creation_request() {
      if (!this->formID)
         return;
      //
      auto  guard = std::lock_guard(this->owner.form_creation_request_info.lock);
      auto& list  = this->owner.form_creation_request_info.reserved_formIDs;
      //
      auto it = std::find(list.begin(), list.end(), this->formID);
      if (it != list.end())
         list.erase(it);
      //
      this->formID = 0;
   }
   void form_creation_request::set_parent_form(form_stub* parent) {
      this->child_of = parent;
   }
   void form_creation_request::set_parent_form(bare_form_id_t parentID) {
      if (!parentID) {
         this->child_of = nullptr;
         return;
      }
      this->child_of = this->owner.get_form(parentID);
   }
   void form_creation_request::queue_clone(form_stub* original) {
      if (this->clone_of == original)
         return;
      if (original) {
         if (original->formType != this->form_type)
            return;
      }
      this->clone_of = original;
   }
   form_stub* form_creation_request::commit() {
      return this->owner.commit_form_creation_request(*this);
   }
   #pragma endregion

   #pragma region form_duplication_request
   form_duplication_request::form_duplication_request(file_load_order& o) : owner(o) {
   }
   form_duplication_request::form_duplication_request(form_duplication_request&& other) : owner(other.owner) {
      this->main_request = other.main_request;
      other.main_request = nullptr;
      //
      this->child_requests.clear();
      size_t size = other.child_requests.size();
      this->child_requests.resize(size);
      for(size_t i = 0; i < size; ++i) {
         this->child_requests[i] = other.child_requests[i];
         other.child_requests[i] = nullptr;
      }
   }
   form_duplication_request::~form_duplication_request() {
      for (auto* request : this->child_requests)
         if (request)
            delete request;
      this->child_requests.clear();
   }
   //
   void form_duplication_request::set_target(form_stub* original) {
      if (this->main_request && original == this->main_request->clone_of)
         return;
      //
      for (auto* request : this->child_requests)
         if (request)
            delete request;
      this->child_requests.clear();
      //
      if (this->main_request) {
         delete this->main_request;
         this->main_request = nullptr;
      }
      if (!original)
         return;
      //
      this->main_request = new form_creation_request(this->owner.request_form_creation(original->formType));
      this->main_request->set_parent_form(this->parent);
      this->main_request->queue_clone(original);
      //
      form_stub_helpers::for_each_child_form(original, [this](form_stub* child) {
         auto* request = new form_creation_request(this->owner.request_form_creation(child->formType));
         request->queue_clone(child);
         this->child_requests.push_back(request);
         return false;
      });
      if (original->formType == form_type::quest) {
         form_stub_helpers::for_each_quest_topic(original, [this](form_stub* child) {
            auto* request = new form_creation_request(this->owner.request_form_creation(child->formType));
            request->queue_clone(child);
            this->child_requests.push_back(request);
            return false;
         });
      }
   }
   //
   void form_duplication_request::set_parent_form(form_stub* parent) {
      this->parent = parent;
      if (this->main_request)
         this->main_request->set_parent_form(parent);
   }
   void form_duplication_request::set_parent_form(bare_form_id_t parentID) {
      this->parent = this->owner.get_form(parentID);
      if (this->main_request)
         this->main_request->set_parent_form(this->parent);
   }
   //
   form_stub* form_duplication_request::commit() {
      if (!this->main_request)
         return nullptr;
      this->main_request->editorID = this->editorID;
      if (!this->parent) {
         auto parentID = this->main_request->clone_of->groupInfo.parentFormID;
         if (parentID)
            this->main_request->set_parent_form(parentID);
      }
      auto* result = this->main_request->commit();
      if (!result)
         return nullptr;
      //
      for (auto* request : this->child_requests) {
         if (!request)
            continue;
         request->set_parent_form(result);
         request->commit();
      }
      //
      return result;
   }
   form_duplication_request::error_code form_duplication_request::get_main_form_error_code() const noexcept {
      if (this->main_request)
         return this->main_request->error;
      return error_code::none;
   }
   std::vector<form_duplication_request::error_code> form_duplication_request::get_child_form_error_codes() const noexcept {
      std::vector<form_duplication_request::error_code> out;
      for (auto* request : this->child_requests)
         if (request && request->error != error_code::none)
            out.push_back(request->error);
      return out;
   }
   std::vector<form_duplication_request::error_code> form_duplication_request::get_error_codes() const noexcept {
      std::vector<form_duplication_request::error_code> out;
      if (this->main_request) {
         out.push_back(this->main_request->error);
         for (auto* request : this->child_requests)
            if (request)
               out.push_back(request->error);
      }
      return out;
   }
   bool form_duplication_request::has_error() const noexcept {
      if (this->main_request) {
         if (this->main_request->error != error_code::none)
            return true;
         for (auto* request : this->child_requests)
            if (request && request->error != error_code::none)
               return true;
      }
      return false;
   }
   bool form_duplication_request::is_valid() const noexcept {
      if (!this->main_request)
         return false;
      return this->main_request->is_valid();
   }
   unsigned int form_duplication_request::get_total_form_count() const noexcept {
      if (!this->main_request)
         return 0;
      return 1 + this->child_requests.size();
   }
   #pragma endregion

   #pragma region form_deletion_request
   form_deletion_request::form_deletion_request(file_load_order& o, form_stub& t) : owner(o), target(t) {
      if (this->target.is_hardcoded() || this->target.formID < minimum_plugin_form_id) {
         this->result = result_code::error_cannot_delete_hardcoded_form;
         return;
      }
      this->active_file_index = this->owner.index_of_active_file();
      //
      if (_form_should_be_flagged(this->target)) {
         this->forms_needing_flag.insert(&this->target);
      } else {
         this->forms_needing_delete.insert(&this->target);
      }
      this->seen_stubs.insert(&this->target);
      this->_gather_others(&this->target);
   }
   form_deletion_request::form_deletion_request(form_deletion_request&& other) : owner(other.owner), target(other.target) {
      this->result = other.result;
      std::swap(this->forms_needing_delete, other.forms_needing_delete);
      std::swap(this->seen_stubs, other.seen_stubs);
      //
      this->force_delete_overrides = other.force_delete_overrides;
      //
      this->active_file_index = this->owner.index_of_active_file();
   }
   bool form_deletion_request::_form_should_be_flagged(form_stub& stub) {
      if (this->force_delete_overrides)
         return false;
      if (stub.is_hardcoded()) // we don't currently allow any kind of deletion of hardcoded forms, but it never hurts to be prepared for what might change
         return true;
      if ((stub.formID >> 0x18) != this->active_file_index)
         return true;
      return false;
   }
   void form_deletion_request::_gather_others(form_stub* start) {
      //
      // Recursively find all descendant forms that need to be deleted, and ensure that we can load 
      // not only all of the to-be-deleted forms, but also all of their users, in order to sever 
      // uses and set "deleted" flags as necessary.
      //
      if (this->result != result_code::pending)
         return;
      if (!start)
         start = &this->target;
      //
      if (!start->load()) {
         this->result = result_code::error_cannot_load_form;
         return;
      }
      //
      for (auto& pair : start->inbound) {
         auto& entry = pair.second;
         auto* other = entry.other;
         if (this->seen_stubs.find(other) != this->seen_stubs.end())
            continue;
         this->seen_stubs.insert(entry.other);
         //
         if (!entry.other->load()) {
            this->result = result_code::error_cannot_load_user;
            return;
         }
         //
         if (entry.flags & use_info_entry::flag::i_am_parent_of) {
            assert(!entry.other->is_hardcoded() && "How is a hardcoded form a descendant of a form that can be deleted (and in fact is currently being deleted)?");
            if (_form_should_be_flagged(*entry.other)) {
               this->forms_needing_flag.insert(entry.other);
            } else {
               this->forms_needing_delete.insert(entry.other);
            }
            this->_gather_others(entry.other);
            if (this->result != result_code::pending)
               return;
         }
      }
   }
   void form_deletion_request::_prep_for_delete(form_stub& stub) {
      stub.sever_all_outbound_references();
      //
      std::vector<form_stub*> pending;
      //
      for (auto& pair : stub.inbound) {
         auto& entry = pair.second;
         auto* other = entry.other;
         auto  form = other->load();
         pending.push_back(other); // gather forms to process later. we don't want to sever refs now, as that will change use info and potentially invalidate iterators during the loop
         other->set_edited(true);
      }
      for (auto* user : pending) {
         auto form = user->load();
         form->sever_outbound_references_to(stub);
      }
   }
   std::vector<form_stub*> form_deletion_request::get_forms_pending_delete(bool include_flagged) const noexcept {
      std::vector<form_stub*> out;
      if (include_flagged)
         out.reserve(this->forms_needing_delete.size() + this->forms_needing_flag.size());
      else
         out.reserve(this->forms_needing_delete.size());
      for (auto* stub : this->forms_needing_delete)
         out.push_back(stub);
      if (include_flagged)
         for (auto* stub : this->forms_needing_flag)
            out.push_back(stub);
      return out;
   }
   std::vector<form_stub*> form_deletion_request::get_forms_pending_flagging() const noexcept {
      std::vector<form_stub*> out;
      out.reserve(this->forms_needing_flag.size());
      for (auto* stub : this->forms_needing_flag)
         out.push_back(stub);
      return out;
   }
   void form_deletion_request::commit() {
      auto* file = owner.active_file;
      bare_form_id_t lowestID = 0xFFFFFFFF;
      for (auto* stub : this->forms_needing_delete) {
         this->_prep_for_delete(*stub);
         //
         auto formID = stub->formID;
         if (formID < lowestID)
            lowestID = formID;
         this->owner.forms.forms.erase(formID);
         this->owner.forms_by_type[stub->formType].forms.erase(formID);
         this->owner.active_file_forms.forms.erase(formID);
         this->owner.active_file_forms_by_type[stub->formType].forms.erase(formID);
         //
         delete stub;
      }
      if (auto* file = owner.active_file) {
         if (file->header.nextFormID > lowestID)
            file->header.nextFormID = lowestID;
      }
      for (auto* stub : this->forms_needing_flag) {
         this->_prep_for_delete(*stub);
         //
         stub->load()->friendly_delete_override(this->owner);
         stub->set_edited(true);
      }
      this->result = result_code::success;
   }
   #pragma endregion
}