#include "./navmesh_info.h"
#include "../../_common_cpp.h"

#include "../../../notices/form_load_warnings/by_form_type/navmesh_info_map/navmesh_info_pathing_cell_bad_crc.h"
#include "../../../notices/form_load_warnings/by_form_type/navmesh_info_map/navmesh_info_pathing_cell_is_improper_exterior.h"
#include "../../../notices/form_load_warnings/by_form_type/navmesh_info_map/navmesh_info_pathing_door_bad_crc.h"

namespace {
   namespace specific_load_warnings {
      using namespace dovah::notices::form_load_warnings::by_type::navmesh_info_map;
   }
}

namespace dovah::loaded_forms::structs::navmesh_info_map {
   void navmesh_info::load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc) {
      this->is_active_file_data = intfc.is_active_file();

      auto _warn_on_bad_crc = [this, &intfc]<typename Warning>(uint32_t expected, const std::string_view expected_str, uint32_t seen) {
         if (seen == expected)
            return;
         Warning notice(
            intfc.target_stub,
            this->navmesh.get_form_stub(),
            seen,
            expected,
            expected_str
         );
         intfc.log_load_warning(notice);
      };

      if (auto& form = this->navmesh; subrecord.read(form)) {
         intfc.warn_if_ref_is_wrong_type(form, form_type::navmesh, subrecord.signature());
      }
      subrecord.read(this->category);
      subrecord.read(this->unk08);
      {
         auto&    list = this->links.edges;
         uint32_t size = 0;
         subrecord.read(size);
         for (uint32_t i = 0; i < size; ++i) {
            auto& form = list.emplace_back();
            if (subrecord.read(form))
               intfc.warn_if_ref_is_wrong_type(form, form_type::navmesh, subrecord.signature());
         }
      }
      {
         auto&    list = this->links.preferred_edges;
         uint32_t size = 0;
         subrecord.read(size);
         for (uint32_t i = 0; i < size; ++i) {
            auto& form = list.emplace_back();
            if (subrecord.read(form))
               intfc.warn_if_ref_is_wrong_type(form, form_type::navmesh, subrecord.signature());
         }
      }
      {
         auto&    list = this->links.doors;
         uint32_t size = 0;
         subrecord.read(size);
         for (uint32_t i = 0; i < size; ++i) {
            auto& item = list.emplace_back();
            subrecord.read(item.crc);
            _warn_on_bad_crc.operator()<specific_load_warnings::navmesh_info_pathing_door_bad_crc>(0xE48B73F3, "PathingDoor", item.crc);
            if (auto& form = item.door; subrecord.read(form))
               intfc.warn_if_ref_is_wrong_type(form, form_type::reference, subrecord.signature());
         }
      }
      {
         bool present = false;
         subrecord.read(present);
         if (present) {
            auto& island = this->island.emplace();
            subrecord.read(island.min.x);
            subrecord.read(island.min.y);
            subrecord.read(island.min.z);
            subrecord.read(island.max.x);
            subrecord.read(island.max.y);
            subrecord.read(island.max.z);
            {
               auto&    list = island.triangles;
               uint32_t size = 0;
               subrecord.read(size);
               list.resize(size);
               for (uint32_t i = 0; i < size; ++i) {
                  auto& item = list[i];
                  subrecord.read(item.vertices);
               }
            }
            {
               auto&    list = island.vertices;
               uint32_t size = 0;
               subrecord.read(size);
               list.resize(size);
               for (uint32_t i = 0; i < size; ++i) {
                  auto& item = list[i];
                  subrecord.read(item.x);
                  subrecord.read(item.y);
                  subrecord.read(item.z);
               }
            }
         }
      }


      subrecord.read(this->pathing_cell.crc);
      _warn_on_bad_crc.operator()<specific_load_warnings::navmesh_info_pathing_cell_bad_crc>(0xA5E9A03C, "PathingCell", this->pathing_cell.crc);
      //
      form_reference_t world;
      if (subrecord.read(world)) {
         if (world) {
            intfc.warn_if_ref_is_wrong_type(world, form_type::worldspace, subrecord.signature());

            auto& data = this->pathing_cell.data.emplace<pathing_cell_exterior>();
            data.parent_world.unmanaged_set(world.get_form_stub());
            subrecord.read(data.grid_y); // NOT a mistake; Y comes before X here.
            subrecord.read(data.grid_x);
         } else {
            auto& data = this->pathing_cell.data.emplace<pathing_cell_interior>();
            if (auto& form = data.cell; subrecord.read(form)) {
               intfc.warn_if_ref_is_wrong_type(form, form_type::cell, subrecord.signature());
               if (auto* stub = form.get_form_stub(); stub && stub->form_type == form_type::cell) {
                  if (stub->is_exterior_cell()) {
                     specific_load_warnings::navmesh_info_pathing_cell_is_improper_exterior notice(
                        intfc.target_stub,
                        this->navmesh.get_form_stub(),
                        *stub
                     );
                     intfc.log_load_warning(notice);
                  }
               }
            }
         }
      }
   }
   void navmesh_info::save(tes_subrecord_writer& subrecord, load_order_interfaces::form_save& intfc) {
      subrecord.write(this->navmesh);
      subrecord.write(this->category);
      subrecord.write(this->unk08);
      {
         auto& list = this->links.edges;
         subrecord.write((uint32_t)list.size());
         for (auto& form : list)
            subrecord.write(form);
      }
      {
         auto& list = this->links.preferred_edges;
         subrecord.write((uint32_t)list.size());
         for (auto& form : list)
            subrecord.write(form);
      }
      {
         auto& list = this->links.doors;
         subrecord.write((uint32_t)list.size());
         for (auto& item : list) {
            subrecord.write(item.crc);
            subrecord.write(item.door);
         }
      }
      bool has_island = this->island.has_value();
      subrecord.write(has_island);
      if (has_island) {
         auto& island = this->island.value();
         subrecord.write(island.min.x);
         subrecord.write(island.min.y);
         subrecord.write(island.min.z);
         subrecord.write(island.max.x);
         subrecord.write(island.max.y);
         subrecord.write(island.max.z);
         {
            auto& list = island.triangles;
            subrecord.write((uint32_t)list.size());
            for (auto& item : list) {
               subrecord.write(item.vertices);
            }
         }
         {
            auto& list = island.vertices;
            subrecord.write((uint32_t)list.size());
            for (auto& item : list) {
               subrecord.write(item.x);
               subrecord.write(item.y);
               subrecord.write(item.z);
            }
         }
      }
      subrecord.write(this->pathing_cell.crc);
      if (auto* casted = std::get_if<pathing_cell_exterior>(&this->pathing_cell.data)) {
         subrecord.write(casted->parent_world);
         subrecord.write(casted->grid_y); // NOT a mistake; Y comes before X here
         subrecord.write(casted->grid_x);
      } else if (auto* casted = std::get_if<pathing_cell_interior>(&this->pathing_cell.data)) {
         subrecord.write((uint32_t)0);
         subrecord.write(casted->cell);
      } else {
         subrecord.write((uint32_t)0);
         subrecord.write((uint16_t)0);
         subrecord.write((uint16_t)0);
      }
   }

   void navmesh_info::clone_from(const navmesh_info& src, Form& my_owner) noexcept {
      this->clear(my_owner);

      this->is_active_file_data = src.is_active_file_data;

      this->navmesh.set(my_owner, src.navmesh);
      copy_form_reference_list(my_owner, this->links.edges, src.links.edges);
      copy_form_reference_list(my_owner, this->links.preferred_edges, src.links.preferred_edges);
      {
         auto&  src_list = src.links.doors;
         auto&  dst_list = this->links.doors;
         size_t size     = src_list.size();
         dst_list.resize(size);
         for (size_t i = 0; i < size; ++i) {
            auto& src_item = src_list[i];
            auto& dst_item = dst_list[i];
            dst_item.crc = src_item.crc;
            dst_item.door.set(my_owner, src_item.door);
         }
      }
      this->island = src.island;
      {
         this->pathing_cell.crc = src.pathing_cell.crc;

         auto& src_var = src.pathing_cell.data;
         auto& dst_var = this->pathing_cell.data;
         if (auto* casted = std::get_if<pathing_cell_exterior>(&src_var)) {
            auto& casted_dst = dst_var.emplace<pathing_cell_exterior>();
            casted_dst.parent_world.set(my_owner, casted->parent_world);
            casted_dst.grid_x = casted->grid_x;
            casted_dst.grid_y = casted->grid_y;
         } else if (auto* casted = std::get_if<pathing_cell_interior>(&src_var)) {
            auto& casted_dst = dst_var.emplace<pathing_cell_interior>();
            casted_dst.cell.set(my_owner, casted->cell);
         }
      }
   }
   void navmesh_info::clear(Form& my_owner) noexcept {
      this->navmesh.set(my_owner, nullptr);
      clear_form_reference_list(this->links.edges, my_owner);
      clear_form_reference_list(this->links.preferred_edges, my_owner);
      for (auto& ld : this->links.doors)
         ld.door.set(my_owner, nullptr);
      this->links.doors.clear();

      if (auto* casted = std::get_if<pathing_cell_exterior>(&this->pathing_cell.data)) {
         casted->parent_world.set(my_owner, nullptr);
      } else if (auto* casted = std::get_if<pathing_cell_interior>(&this->pathing_cell.data)) {
         casted->cell.set(my_owner, nullptr);
      }
   }
   void navmesh_info::sever_outbound_references_to(form_stub& other, Form& my_owner) noexcept {
      this->navmesh.clear_if(my_owner, other);
      remove_form_from_reference_list(this->links.edges, other, my_owner);
      remove_form_from_reference_list(this->links.preferred_edges, other, my_owner);
      {
         auto& list    = this->links.doors;
         bool  changed = false;
         for (auto& item : list) {
            if (item.door == &other) {
               item.door.set(my_owner, nullptr);
               changed = true;
            }
         }
         if (changed) {
            std::erase_if(
               this->links.doors,
               [](auto& item) {
                  return item.door == nullptr;
               }
            );
         }
      }
      if (auto* casted = std::get_if<pathing_cell_exterior>(&this->pathing_cell.data)) {
         casted->parent_world.clear_if(my_owner, other);
      } else if (auto* casted = std::get_if<pathing_cell_interior>(&this->pathing_cell.data)) {
         casted->cell.clear_if(my_owner, other);
      }
   }

   #pragma region navmesh_info::use_info_state
   void navmesh_info::use_info_state::generate_use_info(tes_subrecord_reader& subrecord) {
      subrecord.read(this->navmesh);

      subrecord.skip_bytes(sizeof(navmesh_info::category));
      subrecord.skip_bytes(sizeof(float) * 4);

      {
         auto&    list = this->links.edges;
         uint32_t size = 0;
         subrecord.read(size);
         for (uint32_t i = 0; i < size; ++i) {
            auto& form = list.emplace_back();
            subrecord.read(form);
         }
      }
      {
         auto&    list = this->links.preferred_edges;
         uint32_t size = 0;
         subrecord.read(size);
         for (uint32_t i = 0; i < size; ++i) {
            auto& form = list.emplace_back();
            subrecord.read(form);
         }
      }
      {
         auto&    list = this->links.doors;
         uint32_t size = 0;
         subrecord.read(size);
         for (uint32_t i = 0; i < size; ++i) {
            auto& form = list.emplace_back();
            subrecord.skip_bytes(sizeof(door_link::crc));
            subrecord.read(form);
         }
      }
      {
         bool present = false;
         subrecord.read(present);
         if (present) {
            subrecord.skip_bytes(sizeof(float) * 6);
            {
               uint32_t size = 0;
               subrecord.read(size);
               subrecord.skip_bytes((sizeof(uint16_t) * 3) * size);
            }
            {
               uint32_t size = 0;
               subrecord.read(size);
               subrecord.skip_bytes((sizeof(float) * 3) * size);
            }
         }
      }

      subrecord.skip_bytes(4);

      subrecord.read(this->pathing_cell.parent_world);
      if (this->pathing_cell.parent_world) {
         subrecord.skip_bytes(4);
      } else {
         subrecord.read(this->pathing_cell.interior_cell);
      }
   }
   void navmesh_info::use_info_state::commit_to(form_stub_use_info_builder& uib) {
      if (this->navmesh)
         uib.add_outbound_reference(this->navmesh);
      for (auto& id : this->links.edges)
         uib.add_outbound_reference(id);
      for (auto& id : this->links.preferred_edges)
         uib.add_outbound_reference(id);
      for (auto& id : this->links.doors)
         uib.add_outbound_reference(id);
      uib.add_outbound_reference(this->pathing_cell.parent_world);
      uib.add_outbound_reference(this->pathing_cell.interior_cell);
   }
   std::vector<form_id_t> navmesh_info::use_info_state::as_combined_list() const {
      std::vector<form_id_t> out;
      out.reserve(
         1 +
         this->links.edges.size() +
         this->links.preferred_edges.size() +
         this->links.doors.size() +
         (!!this->pathing_cell.parent_world || !!this->pathing_cell.interior_cell) ? 1 : 0
      );
      out.push_back(this->navmesh);
      for (auto id : this->links.edges)
         out.push_back(id);
      for (auto id : this->links.preferred_edges)
         out.push_back(id);
      for (auto id : this->links.doors)
         out.push_back(id);
      if (auto id = this->pathing_cell.parent_world)
         out.push_back(id);
      else if (auto id = this->pathing_cell.interior_cell)
         out.push_back(id);
      return out;
   }
   #pragma endregion
}