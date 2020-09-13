#include "file_load_order.h"
#include "file_header.h"
#include "threaded_load_order_use_info_builder.h"
#include "../../helpers/strings.h"
#include "../form_stub.h"
#include "tes_file_reading/file.h"
#include "../forms/factories/hardcoded.h"
#include "../logging.h"

namespace dovah {
   file_load_order::~file_load_order() {
      for (auto* f : this->files)
         delete f;
      this->files.clear();
      this->active_file = nullptr;
      //
      this->normalizer.delete_contents();
      //
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
   }

   #pragma region File loading
   void file_load_order::_make_hardcoded_forms() {
      add_hardcoded_forms_to_load_order(*this);
   }
   void file_load_order::_accept_hardcoded_form(form_stub* stub) noexcept {
      auto& type = this->forms_by_type[stub->formType];
      std::lock_guard<std::mutex> guard_for_form_type(type.lock);
      std::lock_guard<std::mutex> guard_for_all_forms(this->forms.lock);
      //
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
      #if BENCHMARK_LOAD_ORDER_USE_INFO_BUILD == 1
         struct timeb bench_start;
         struct timeb bench_end;
         printf("Building Use Info...\n");
         ftime(&bench_start);
      #endif
      threaded_load_order_use_info_builder builders[8];
      uint32_t which_thread = 0;
      // Inbound first, since we can multi-thread that
      for (auto it = this->forms.forms.begin(); it != this->forms.forms.end(); ++it) {
         form_stub* stub = it->second;
         builders[which_thread].add_to_queue(stub);
         if (++which_thread > 7)
            which_thread = 0;
      }
      for (int i = 0; i < std::extent<decltype(builders)>::value; i++)
         builders[i].start();
      for (int i = 0; i < std::extent<decltype(builders)>::value; i++)
         builders[i].wait_for();
      #if BENCHMARK_LOAD_ORDER_USE_INFO_BUILD == 1
         ftime(&bench_end);
         printf("Time taken for outbound refs: %d ms\n", (uint32_t)(1000.0 * (bench_end.time - bench_start.time)) + (bench_end.millitm - bench_start.millitm));
         ftime(&bench_start);
      #endif
      // Outbound next; has to be single-threaded
      for (auto it = this->forms.forms.begin(); it != this->forms.forms.end(); ++it) {
         it->second->send_inbound_refs();
      }
      #if BENCHMARK_LOAD_ORDER_USE_INFO_BUILD == 1
         ftime(&bench_end);
         printf("Time taken for inbound refs: %d ms\n", (uint32_t)(1000.0 * (bench_end.time - bench_start.time)) + (bench_end.millitm - bench_start.millitm));
      #endif
   }

   uint8_t file_load_order::load_order_prefix_for(const loaded_file* file) const noexcept {
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
      this->normalizer.base_path = this->queued_load.base_path;
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
         auto it = std::find_if(list->begin(), list->end(), [&name](file_header* file) { return cobb::strieq(name, file->name); });
         assert(it != list->end() && "How is it not in the list?!");
         file_header* header = *it;
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
         #if COBB_ESP_BLOCK_ALLOCATE_MAP_PAIRS != 1
         this->forms.forms.reserve((size_t)(total * 1.1) + 0x800);
         #endif
      }//*/
      {
         dovah::logging::print_line("Final load order:");
         for (auto* header : this->normalizer.masters)
            dovah::logging::print_line("[M] %s", header->name.c_str());
         for (auto* header : this->normalizer.plugins)
            dovah::logging::print_line("[P] %s", header->name.c_str());
      }
      #if BENCHMARK_FORM_STUB_BUILD == 1
      struct timeb bench_start;
      struct timeb bench_end;
      dovah::logging::print_line("Building FormStubs...\n");
      ftime(&bench_start);
      #endif
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
      #if BENCHMARK_FORM_STUB_BUILD == 1
      ftime(&bench_end);
      printf("Time taken to build all FormStubs: %d ms\n", (uint32_t)(1000.0 * (bench_end.time - bench_start.time)) + (bench_end.millitm - bench_start.millitm));
      #endif
      this->loading_is_complete = true;
      this->loading_index = 0;
      //
      this->_build_use_info();
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
      if (this->active_file && formID >> 0x18 == this->active_file_index) {
         this->active_file_forms.forms[formID] = stub;
         auto& at = this->active_file_forms_by_type[stub->formType];
         at.forms[formID] = stub;
      }
      //
      return form_id_status::valid;
   }
   #pragma endregion

   bool file_load_order::has_form(uint32_t formID) const noexcept {
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
   form_stub* file_load_order::get_form(uint32_t formID) const noexcept {
      if (formID == 0)
         return nullptr;
      auto& list = this->forms.forms;
      auto  it   = list.find(formID);
      if (it != list.end())
         return it->second;
      return nullptr;
   }
   form_stub* file_load_order::get_form(form_type_t formType, uint32_t formID) const noexcept {
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
   form_stub* file_load_order::get_form_of_probable_type(form_type_t formType, uint32_t formID) const noexcept {
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
   file_load_order::form_id_status file_load_order::local_formID_to_global_formID(const loaded_file* file, uint32_t& id) const {
      if ((id & plugin_form_id_mask) == 0) { // hardcoded
         id = id & hardcoded_form_id_mask;
         return form_id_status::valid;
      }
      if (!file) {
         id = 0;
         return form_id_status::missing_master;
      }
      uint8_t local  = file->masters.size();
      uint8_t prefix = id >> 0x18;
      if (prefix == local) {
         id = id & 0x00FFFFFF | (this->guided_load_order_prefix_for(file) << 0x18);
         return form_id_status::valid;
      }
      if (prefix > local) {
         id = 0;
         return form_id_status::out_of_bounds;
      }
      auto&   name = file->masters[prefix].master;
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
      uint8_t local = file->masters.size();
      uint8_t prefix = stub->formID >> 0x18;
      if (prefix == local) {
         out = stub->formID & 0x00FFFFFF | (this->guided_load_order_prefix_for(file) << 0x18);
         return form_id_status::valid;
      }
      if (prefix > local) {
         out = 0;
         return form_id_status::out_of_bounds;
      }
      auto& name = file->masters[prefix].master;
      uint8_t j = this->index_of_loaded_file(name);
      if (j == invalid_load_prefix) {
         out = 0;
         return form_id_status::missing_master;
      }
      out = stub->formID & 0x00FFFFFF | (j << 0x18);
      return form_id_status::valid;
   }
}