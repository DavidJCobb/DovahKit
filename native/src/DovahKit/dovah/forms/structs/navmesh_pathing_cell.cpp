#include "./navmesh_pathing_cell.h"
#include "../_common_cpp.h"

#include "../../notices/form_load_warnings/by_form_component/navmesh_pathing_cell/bad_crc.h"

namespace {
   namespace specific_load_warnings {
      using namespace dovah::notices::form_load_warnings::by_component::navmesh_pathing_cell;
   }
}

namespace dovah::loaded_forms::structs {
   void navmesh_pathing_cell::load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc, form_stub* owning_navmesh) {
      auto _warn_on_bad_crc = [this, &intfc, &owning_navmesh]<typename Warning>(uint32_t expected, const std::string_view expected_str, uint32_t seen) {
         if (seen == expected)
            return;
         Warning notice(
            intfc.target_stub,
            owning_navmesh,
            seen,
            expected,
            expected_str
         );
         intfc.log_load_warning(notice);
      };

      subrecord.read(this->crc);
      _warn_on_bad_crc.operator()<specific_load_warnings::bad_crc >(0xA5E9A03C, "PathingCell", this->crc);
      
      form_reference_t world;
      if (subrecord.read(world)) {
         if (world) {
            intfc.warn_if_ref_is_wrong_type(world, form_type::worldspace, subrecord.signature());

            auto& data = this->data.emplace<pathing_cell_exterior>();
            data.parent_world.unmanaged_set(world.get_form_stub());
            subrecord.read(data.grid_y); // NOT a mistake; Y comes before X here.
            subrecord.read(data.grid_x);
         } else {
            auto& data = this->data.emplace<pathing_cell_interior>();
            if (auto& form = data.cell; subrecord.read(form)) {
               intfc.warn_if_ref_is_wrong_type(form, form_type::cell, subrecord.signature());
            }
         }
      }
   }
   void navmesh_pathing_cell::save(tes_subrecord_writer& subrecord, load_order_interfaces::form_save& intfc) {
      subrecord.write(this->crc);
      if (auto* casted = std::get_if<pathing_cell_exterior>(&this->data)) {
         subrecord.write(casted->parent_world);
         subrecord.write(casted->grid_y); // NOT a mistake; Y comes before X here
         subrecord.write(casted->grid_x);
      } else if (auto* casted = std::get_if<pathing_cell_interior>(&this->data)) {
         subrecord.write((uint32_t)0);
         subrecord.write(casted->cell);
      } else {
         subrecord.write((uint32_t)0);
         subrecord.write((uint16_t)0);
         subrecord.write((uint16_t)0);
      }
   }

   void navmesh_pathing_cell::clone_from(const navmesh_pathing_cell& src, Form& my_owner) noexcept {
      this->clear(my_owner);
      {
         this->crc = src.crc;

         auto& src_var = src.data;
         auto& dst_var = this->data;
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
   void navmesh_pathing_cell::clear(Form& my_owner) noexcept {
      if (auto* casted = std::get_if<pathing_cell_exterior>(&this->data)) {
         casted->parent_world.set(my_owner, nullptr);
      } else if (auto* casted = std::get_if<pathing_cell_interior>(&this->data)) {
         casted->cell.set(my_owner, nullptr);
      }
   }
   void navmesh_pathing_cell::sever_outbound_references_to(form_stub& other, Form& my_owner) noexcept {
      if (auto* casted = std::get_if<pathing_cell_exterior>(&this->data)) {
         casted->parent_world.clear_if(my_owner, other);
      } else if (auto* casted = std::get_if<pathing_cell_interior>(&this->data)) {
         casted->cell.clear_if(my_owner, other);
      }
   }

   #pragma region navmesh_info::use_info_state
   void navmesh_pathing_cell::use_info_state::generate_use_info(tes_subrecord_reader& subrecord) {
      subrecord.skip_bytes(sizeof(crc));
      subrecord.read(this->parent_world);
      if (this->parent_world) {
         subrecord.skip_bytes(4);
      } else {
         subrecord.read(this->interior_cell);
      }
   }
   void navmesh_pathing_cell::use_info_state::commit_to(form_stub_use_info_builder& uib) {
      uib.add_outbound_reference(this->parent_world);
      uib.add_outbound_reference(this->interior_cell);
   }
   #pragma endregion
}