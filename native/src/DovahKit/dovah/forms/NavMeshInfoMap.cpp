#include "NavMeshInfoMap.h"
#include "_common_cpp.h"
#include "../form_stub_use_info_builder_form_specific_data.h"

namespace dovah::loaded_forms {
   void NavMeshInfoMap::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      //
      uint32_t version = 0; // NVER
      //
      bool is_active_file = intfc.is_active_file();
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'NVER':
               subrecord.read(version);
               break;
            case 'NVMI':
               this->navmesh_infos.load_one(subrecord, intfc);
               break;
            case 'NVPP':
               this->precomputed_paths.load(subrecord, intfc, *this);
               {  // road markers
                  uint32_t count = 0;
                  subrecord.read(count);
                  for (uint32_t i = 0; i < count; ++i) {
                     auto& entry = this->road_markers.emplace_back();
                     entry.is_active_file_data = is_active_file;

                     if (auto& form = entry.navmesh; subrecord.read(form)) {
                        intfc.warn_if_ref_is_wrong_type(form, form_type::navmesh, subrecord.signature());
                     }
                     subrecord.read(entry.index);
                  }
               }
               break;
            case 'NVSI':
               static_assert(false, "TODO: Only retain the list from the winning record?");
               {
                  auto& list = is_active_file ? this->deleted_navmeshes.active_file : this->deleted_navmeshes.masters;
                  while (subrecord.is_in_bounds(4)) {
                     auto& form = list.emplace_back();
                     subrecord.unchecked_read(form);
                     intfc.warn_if_ref_is_wrong_type(form, form_type::navmesh, subrecord.signature());
                  }
               }
               break;

            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void NavMeshInfoMap::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'NVER':
               break;
            case 'NVMI':
               {
                  auto& dst_opt = uib.get_form_specific_data()->by_form_type.navmesh_info_map;
                  auto& dst     = dst_opt.has_value() ? dst_opt.value() : dst_opt.emplace();

                  auto& uses = dst.navmesh_info;
                  navmesh_info_collection::generate_use_info(subrecord, uses);
               }
               break;
            case 'NVPP':
               {
                  auto& dst_opt = uib.get_form_specific_data()->by_form_type.navmesh_info_map;
                  auto& dst     = dst_opt.has_value() ? dst_opt.value() : dst_opt.emplace();

                  auto& uses = dst.precomputed_paths;
                  precomputed_path_collection::generate_use_info(subrecord, uses);
               }
               {
                  uint32_t count = 0;
                  subrecord.read(count);
                  for (uint32_t i = 0; i < count; ++i) {
                     form_id_t target;
                     subrecord.read(target);
                     uib.add_outbound_reference(target);
                     subrecord.skip_bytes(4);
                  }
               }
               break;
            case 'NVSI':
               static_assert(false, "TODO: Only retain the list from the winning record?");
               while (subrecord.is_in_bounds(4)) {
                  form_id_t form;
                  subrecord.read(form);
                  uib.add_outbound_reference(form);
               }
               break;
         }
      }
   }
   void NavMeshInfoMap::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (NavMeshInfoMap*)out;
      
      copy->navmesh_infos.clone_from(this->navmesh_infos, *copy);
      copy->precomputed_paths.clone_from(this->precomputed_paths, *copy);

      {
         auto&  src_list = this->road_markers;
         auto&  dst_list = copy->road_markers;
         size_t size     = src_list.size();
         dst_list.resize(size);
         for (size_t i = 0; i < size; ++i) {
            auto& src_item = src_list[i];
            auto& dst_item = dst_list[i];
            dst_item.is_active_file_data = src_item.is_active_file_data;
            dst_item.navmesh.set(*copy, src_item.navmesh);
            dst_item.index = src_item.index;
         }
      }

      copy_form_reference_list(*copy, copy->deleted_navmeshes.active_file, this->deleted_navmeshes.active_file);
      copy_form_reference_list(*copy, copy->deleted_navmeshes.masters, this->deleted_navmeshes.masters);
   }
   void NavMeshInfoMap::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      {
         auto& subrecord = record.open_next_subrecord('NVER');
         subrecord.write((uint32_t)12);
         subrecord.close();
      }

      this->navmesh_infos.save_all(record, intfc);

      // NVPP
      {
         size_t pp_to_save = this->precomputed_paths.paths.size();
         size_t rm_to_save = 0;
         for (auto& item : this->road_markers)
            if (item.is_active_file_data)
               ++rm_to_save;
         if (pp_to_save || rm_to_save) {
            auto& subrecord = record.open_next_subrecord('NVPP');
            this->precomputed_paths.save(subrecord, intfc);
            subrecord.write((uint32_t)rm_to_save);
            for (auto& item : this->road_markers) {
               if (!item.is_active_file_data)
                  continue;
               auto& subrecord = record.get_current_subrecord();
               subrecord.write(item.navmesh);
               subrecord.write(item.index);
            }
            subrecord.close();
         }
      }
      if (auto& list = this->deleted_navmeshes.active_file; !list.empty()) {
         auto& subrecord = record.open_next_subrecord('NVSI');
         for (auto& form : list)
            subrecord.write(form);
         subrecord.close();
      }
   }
   void NavMeshInfoMap::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->navmesh_infos.sever_outbound_references_to(other, *this);
      this->precomputed_paths.sever_outbound_references_to(other, *this);

      for (auto& item : this->road_markers) {
         item.navmesh.clear_if(*this, other);
      }

      remove_form_from_reference_list(this->deleted_navmeshes.masters, other, *this);
      remove_form_from_reference_list(this->deleted_navmeshes.active_file, other, *this);
   }
   void NavMeshInfoMap::_clear_impl() noexcept {
      this->navmesh_infos.clear(*this);
      this->precomputed_paths.clear(*this);

      for (auto& item : this->road_markers) {
         item.navmesh.set(*this, nullptr);
      }
      this->road_markers.clear();

      clear_form_reference_list(this->deleted_navmeshes.masters, *this);
      clear_form_reference_list(this->deleted_navmeshes.active_file, *this);
   }
}