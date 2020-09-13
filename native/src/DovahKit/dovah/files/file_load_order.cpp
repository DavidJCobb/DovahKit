#include "file_load_order.h"
#include "../../helpers/strings.h"
#include "tes_file_reading/file.h"

namespace dovah {
   #pragma region File loading
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