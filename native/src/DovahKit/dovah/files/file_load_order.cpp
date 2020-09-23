#include "file_load_order.h"
#include "threaded_load_order_use_info_builder.h"
#include "../../helpers/strings.h"
#include "../form_stub.h"
#include "tes_file_reading/file.h"
#include "tes_file_reading/file_header.h"
#include "tes_file_writing/file_writer.h"
#include "../forms/factories/hardcoded.h"
#include "../logging.h"

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
         this->use_info_outbound_complete = true;
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
      uint8_t li   = this->loading_index;
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
      this->load_error          = file_read_error();
      this->loading_is_complete = false;
      if (!this->queued_load.base_path.empty()) {
         char end = *this->queued_load.base_path.rbegin();
         if (end != '/' && end != '\\')
            this->queued_load.base_path += '/';
      }
      //
      this->normalizer.base_path   = this->queued_load.base_path;
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
         std::string path = this->queued_load.base_path + header->name;
         auto file = new tes_file_reading::file_reader(*this);
         this->loading_index = this->files.size();
         this->files.push_back(file);
         if (!this->queued_load.active_file.empty() && cobb::strieq(this->queued_load.active_file, header->name)) {
            this->active_file = file;
            this->active_file_index = this->loading_index;
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
         std::string path = this->queued_load.base_path + header->name;
         auto file = new tes_file_reading::file_reader(*this);
         this->loading_index = this->files.size();
         this->files.push_back(file);
         if (!this->queued_load.active_file.empty() && cobb::strieq(this->queued_load.active_file, header->name)) {
            this->active_file = file;
            this->active_file_index = this->loading_index;
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
      this->loading_is_complete = true;
      this->loading_index = 0;
      //
      this->_build_use_info();
      this->use_info_build_is_complete = true;
      //
      if (!this->active_file && this->files.size() < 254) { // TODO: the cutoff should be 253 if any of the loaded files are ESLs or SSE files
         auto file = new tes_file_reading::file_reader(*this);
         this->active_file = file;
         // TODO: How should we handle file_reader::nextFormID?
         this->files.push_back(file);
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
      bool l = this->loading_is_complete;
      bool u = this->use_info_build_is_complete;
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
         if (this->use_info_outbound_complete)
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
      float progress = per_file * this->loading_index;
      auto* current  = this->files[this->loading_index];
      if (!current)
         return progress;
      auto this_file = current->assess_load_progress();
      if (!isnan(this_file))
         progress += per_file * this_file;
      //
      progress *= (1.0F - use_info_proportion);
      return progress;
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
   //
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

   bool file_load_order::save_active_file(std::filesystem::path name_to_use_if_nameless) {
      this->save_error   = file_write_error();
      this->save_warning = file_write_warning();
      if (!this->active_file) {
         this->save_error.code = file_write_error::error_code::no_active_file;
         return false;
      }
      if (this->is_loading()) {
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
         this->active_file->set_path(std::filesystem::path(this->queued_load.base_path) / filename);
      }
      //
      filename = std::filesystem::path(this->queued_load.base_path) / filename;
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
      tes_file_writing::file_writer writer(*this, *this->active_file);
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
}