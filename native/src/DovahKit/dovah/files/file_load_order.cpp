#include "file_load_order.h"
#include "../../helpers/strings.h"
#include "../form_stub.h"
#include "tes_file_reading/file.h"
#include "../forms/factories/hardcoded.h"
#include "threaded_load_order_use_info_builder.h"

namespace dovah {
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

   file_load_order::form_id_status file_load_order::accept_form_stub(form_stub* stub) noexcept {
      auto& type = this->forms_by_type[stub->formType];
      std::lock_guard<std::mutex> guard_for_form_type(type.lock);
      std::lock_guard<std::mutex> guard_for_all_forms(this->forms.lock);
      //
      uint32_t formID;
      auto     result = this->local_formID_to_global_formID(stub->file, formID);
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
   void file_load_order::for_each_form_of_type(form_type_t formType, std::function<bool(form_stub*)> functor) {
      if (formType < this->forms_by_type.size()) {
         auto& list = this->forms_by_type[formType].forms;
         for (auto it = list.begin(); it != list.end(); ++it) {
            if (functor(it->second))
               break;
         }
      }
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
}