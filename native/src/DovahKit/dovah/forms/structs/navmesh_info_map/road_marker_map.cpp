#include "./road_marker_map.h"
#include "../../_common_cpp.h"

namespace dovah::loaded_forms::structs::navmesh_info_map {
   road_marker_map::entry& road_marker_map::_get_or_create_entry_during_load(form_stub* stub) {
      for (auto& item : this->data)
         if (item.navmesh == stub)
            return item;
      auto& item = this->data.emplace_back();
      item.navmesh.unmanaged_set(stub);
      return item;
   }

   std::optional<uint32_t> road_marker_map::get_index_for(form_stub& stub) const {
      for (auto& item : this->data)
         if (item.navmesh == &stub)
            return item.index;
      return {};
   }
   void road_marker_map::set_index_for(form_stub& stub, uint32_t v, Form& my_owner) {
      for (auto& item : this->data) {
         if (item.navmesh == &stub) {
            item.index = v;
            return;
         }
      }
      auto& item = this->data.emplace_back();
      item.navmesh.set(my_owner, &stub);
      item.index = v;
   }

   void road_marker_map::load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc) {
      uint32_t size = 0;
      subrecord.read(size);
      this->data.reserve(size);
      for (uint32_t i = 0; i < size; ++i) {
         form_reference_t form;
         uint32_t         index = 0;
         if (subrecord.read(form)) {
            intfc.warn_if_ref_is_wrong_type(form, form_type::navmesh, subrecord.signature());
         }
         subrecord.read(index);

         if (!form)
            continue;

         auto& entry = this->_get_or_create_entry_during_load(form.get_form_stub());
         entry.index = index;
      }
   }
   /*static*/ void road_marker_map::generate_use_info(tes_subrecord_reader& subrecord, form_specific_use_info_data& append_to) {
      uint32_t size = 0;
      subrecord.read(size);
      for (uint32_t i = 0; i < size; ++i) {
         form_id_t form;
         subrecord.read(form);
         subrecord.skip_bytes(sizeof(uint32_t));
         append_to.navmeshes.insert(form);
      }
   }
   void road_marker_map::save(tes_subrecord_writer& subrecord, load_order_interfaces::form_save& intfc) {
      subrecord.write((uint32_t)this->data.size());
      for (auto& item : this->data) {
         subrecord.write(item.navmesh);
         subrecord.write(item.index);
      }
   }

   void road_marker_map::clone_from(const road_marker_map& src, Form& my_owner) noexcept {
      this->clear(my_owner);
      for (auto& src_item : src.data) {
         auto& dst_item = this->data.emplace_back();
         dst_item.navmesh.set(my_owner, src_item.navmesh);
         dst_item.index = src_item.index;
      }
   }
   void road_marker_map::clear(Form& my_owner) noexcept {
      for (auto& item : this->data) {
         item.navmesh.set(my_owner, nullptr);
      }
      this->data.clear();
   }
   void road_marker_map::sever_outbound_references_to(form_stub& other, Form& my_owner) noexcept {
      bool removed_any = false;
      for (auto& item : this->data) {
         if (item.navmesh == &other) {
            item.navmesh.set(my_owner, nullptr);
            removed_any = true;
         }
      }
      if (removed_any) {
         std::erase_if(
            this->data,
            [](auto& item) {
               return item.navmesh == nullptr;
            }
         );
      }
   }
}