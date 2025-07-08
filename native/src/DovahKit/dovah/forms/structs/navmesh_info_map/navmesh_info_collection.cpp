#include "./navmesh_info_collection.h"
#include "../../_common_cpp.h"

namespace dovah::loaded_forms::structs::navmesh_info_map {
   void navmesh_info_collection::load_one(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc) {
      navmesh_info info;
      info.load(subrecord, intfc);

      this->infos[info.navmesh.get_form_stub()] = std::move(info);
   }
   /*static*/ void navmesh_info_collection::generate_use_info(tes_subrecord_reader& subrecord, form_specific_use_info_data& fsuid) {
      navmesh_info::use_info_state state;
      state.generate_use_info(subrecord);
      fsuid.infos[state.navmesh] = std::move(state.as_combined_list());
   }
   void navmesh_info_collection::save_all(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      for (auto& pair : this->infos) {
         auto& info = pair.second;
         if (!info.is_active_file_data)
            continue;
         auto& subrecord = record.open_next_subrecord(navmesh_info::subrecord);
         info.save(subrecord, intfc);
         subrecord.close();
      }
   }
   void navmesh_info_collection::clone_from(const navmesh_info_collection& src, Form& my_owner) noexcept {
      this->clear(my_owner);
      for (auto& pair : src.infos) {
         auto& dst = this->infos[pair.first];
         dst.clone_from(pair.second, my_owner);
      }
   }
   void navmesh_info_collection::clear(Form& my_owner) noexcept {
      for (auto& pair : this->infos) {
         pair.second.clear(my_owner);
      }
      this->infos.clear();
   }
   void navmesh_info_collection::sever_outbound_references_to(form_stub& other, Form& my_owner) noexcept {
      bool removed_any = false;
      for (auto& pair : this->infos) {
         if (pair.first == &other) {
            removed_any = true;
            pair.second.clear(my_owner);
            continue;
         }
         pair.second.sever_outbound_references_to(other, my_owner);
      }
      if (removed_any) {
         this->infos.erase(&other);
      }
   }
}