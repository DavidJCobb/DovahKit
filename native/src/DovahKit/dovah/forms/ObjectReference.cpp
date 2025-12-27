#include "ObjectReference.h"
#include "_common_cpp.h"
#include "../data/hardcoded_form_ids.h"
#include "../form_stubs/helpers/get_worldspace_cell_by_grid.h"
#include "components/extra_data/types/e/enable_state_parent.h"
#include "components/extra_data/types/s/scale.h"
#include "components/extra_data/use_info_state.h"

// persistence checks
#include "./components/extra_data/types/l/location.h"
#include "./Activator.h"
#include "./DefaultObjectManager.h"
#include "./Door.h"

#include "../exceptions/form_creation_failed.h"
#include "../exceptions/object_reference_move_failed.h"
#include "../load_order_requests/form_creation_request.h"

namespace dovah::loaded_forms {
   void ObjectReference::set_position(cobb::vector3<float> position) {
      using exception  = exceptions::object_reference_move_failed;
      using error_code = exception::error_code;

      if (this->is_working_copy) {
         throw exception(error_code::operation_not_allowed_on_form_working_copy, this->stub);
      }
      auto* cell = this->stub.get_parent_form();
      if (!cell) { // technically possible with PlayerRef
         throw exception(error_code::reference_is_orphaned, this->stub);
      }
      if (!cell->is_exterior_cell()) {
         if (!this->is_working_copy)
            this->stub.set_edited(true);
         this->position = position;
         return;
      }
      auto* world = cell->get_parent_form();
      assert(world && "How is an exterior cell not in a worldspace?");
      return this->set_position_and_world(position, *world);
   }
   void ObjectReference::set_position_and_parent(cobb::vector3<float> position, form_stub& world_or_cell) {
      using exception  = exceptions::object_reference_move_failed;
      using error_code = exception::error_code;

      if (this->is_working_copy) {
         throw exception(error_code::operation_not_allowed_on_form_working_copy, this->stub);
      }
      assert(world_or_cell.form_type == form_type::cell || world_or_cell.form_type == form_type::worldspace);
      if (world_or_cell.form_type == dovah::form_type::worldspace) {
         return this->set_position_and_world(position, world_or_cell);
      }
      if (world_or_cell.is_exterior_cell()) {
         auto* world = world_or_cell.get_parent_form();
         assert(world != nullptr);
         return this->set_position_and_world(position, *world);
      }
      return this->set_position_and_cell(position, world_or_cell);
   }
   void ObjectReference::set_position_and_cell(cobb::vector3<float> position, form_stub& parent_cell) {
      using exception  = exceptions::object_reference_move_failed;
      using error_code = exception::error_code;

      if (this->is_working_copy) {
         throw exception(error_code::operation_not_allowed_on_form_working_copy, this->stub);
      }
      if (this->stub.is_hardcoded()) {
         throw exception(error_code::reference_is_hardcoded, this->stub);
      }
      auto* world = parent_cell.get_parent_form();
      if (world) {
         assert(world->form_type == form_type::worldspace && "How is a cell a child of a non-worldspace form?");
         assert(parent_cell.is_exterior_cell() && "How is an interior cell a child of a worldspace?");
         int32_t cx;
         int32_t cy;
         parent_cell.get_grid_coordinates(cx, cy);
         int32_t gx = position.x / 4096;
         int32_t gy = position.y / 4096;
         if (cx != gx || cy != gy) {
            auto ex = exception(error_code::desired_position_is_outside_of_desired_cell, this->stub);
            ex.details.cell     = &parent_cell;
            ex.details.position = position;
            throw ex;
         }
      }
      this->stub.set_edited(true);
      this->stub.set_parent_form(&parent_cell);
      this->position = position;
   }
   void ObjectReference::set_position_and_world(cobb::vector3<float> position, form_stub& world) {
      using exception  = exceptions::object_reference_move_failed;
      using error_code = exception::error_code;

      if (this->is_working_copy) {
         throw exception(error_code::operation_not_allowed_on_form_working_copy, this->stub);
      }
      if (this->stub.is_hardcoded()) {
         throw exception(error_code::reference_is_hardcoded, this->stub);
      }
      assert(world.form_type == form_type::worldspace);
      int32_t gx = position.x / 4096;
      int32_t gy = position.y / 4096;
      auto* move_to_cell = form_stub_helpers::get_worldspace_cell_by_grid(&world, gx, gy);
      if (move_to_cell) {
         this->stub.set_edited(true);
         this->stub.set_parent_form(move_to_cell);
         this->position = position;
         return;
      }
      //
      // There's no existing cell bounding the desired REFR coordinates, so we'll 
      // have to try and create a new cell.
      //
      auto& lo = this->stub.get_owning_load_order();
      try {
         auto request = lo.request_form_creation(dovah::form_type::cell);
         request.set_parent_form(&world);
         request.cell_grid_coordinates = { .x = gx, .y = gy };
         move_to_cell = request.commit();
      } catch (const exceptions::form_creation_failed&) {
         throw exception(error_code::failed_to_create_destination_cell, this->stub);
      }
      if (!move_to_cell) {
         throw exception(error_code::failed_to_create_destination_cell, this->stub);
      }
      this->stub.set_edited(true);
      this->stub.set_parent_form(move_to_cell);
      this->position = position;
   }
   
   float ObjectReference::get_scale() const {
      float scale = 1.0F;
      if (auto* extra = this->extra_data.get<dovah::loaded_forms::components::extra_data_types::scale>()) {
         scale = extra->value;
         if (scale < 0.01F)
            scale = 0.01F;
         else if (scale > 100.0F)
            scale = 100.0F;
         else
            scale -= std::fmod(scale, 0.01F);
         return scale;
      }
      return scale;
   }
   float ObjectReference::get_raw_scale() const {
      if (auto* extra = this->extra_data.get<dovah::loaded_forms::components::extra_data_types::scale>()) {
         return extra->value;
      }
      return 1.0F;
   }
   void ObjectReference::set_scale(float v, bool with_limits) {
      if (with_limits) {
         v = std::roundf(v * 100.0F);
         if (v < 1)
            v = 1;
         else if (v > 100)
            v = 100;
         v /= 100.0F;
      }
      auto* extra = this->extra_data.get_or_create<dovah::loaded_forms::components::extra_data_types::scale>();
      if (extra->value == v)
         return;
      extra->value = v;
      this->stub.set_edited(true);
   }

   void ObjectReference::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      //
      if (!intfc.is_winning_record)
         return;
      //
      form_id_t formID;
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'DATA':
               subrecord.read(this->position.x);
               subrecord.read(this->position.y);
               subrecord.read(this->position.z);
               subrecord.read(this->rotation.x);
               subrecord.read(this->rotation.y);
               subrecord.read(this->rotation.z);
               break;
            case 'ONAM':
               this->is_open = true;
               break;
            case 'NAME': // base form (subrecord signature is vestigial from Morrowind, which used editor IDs instead of form IDs)
               subrecord.read(this->base_form);
               break;
            case 'VMAD':
               this->script_data.load(subrecord, intfc);
               break;
            default:
               if (this->extra_data.load(record, intfc) == components::extra_data_list::load_result::unrecognized) {
                  //
                  // Subrecord is not extra-data.
                  //
                  intfc.warn_on_unrecognized_subrecord(subrecord);
               }
               break;
         }
      }
   }
   /*static*/ void ObjectReference::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files. (TODO: CONFIRM THIS)
         //
         return;
      //
      components::extra_data_use_info_state eduis;
      form_id_t base_form;
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'VMAD':
               decltype(script_data)::generate_use_info(subrecord, uib);
               break;
            case 'NAME': // base form (subrecord signature is vestigial from Morrowind, which used editor IDs instead of form IDs)
               subrecord.read(base_form);
               break;
            case 'EDID': // editor ID
            case 'ONAM':
            case 'DATA':
               break;
            default:
               if (!components::extra_data_list::generate_use_info(record, uib, eduis)) {
                  //
                  // If execution reaches this spot, then the subrecord is not extra-data.
                  //
               }
               break;
         }
      }
      uib.add_outbound_reference(base_form, use_info_entry::flag::object_reference);
      eduis.commit_to(uib);
   }
   void ObjectReference::_clone_impl(Form* out) const noexcept {
      assert(out->type == this->type);
      auto copy = (ObjectReference*)out;
      
      copy->extra_data.clone_from(this->extra_data, *copy);
      copy->script_data.clone_from(this->script_data, *copy);
      copy->base_form.set(*copy, this->base_form);
      copy->is_open  = this->is_open;
      copy->position = this->position;
      copy->rotation = this->rotation;
   }
   void ObjectReference::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      //
      auto& NAME = record.open_next_subrecord('NAME');
      NAME.write(this->base_form);
      NAME.close();
      //
      this->extra_data.save(record, intfc);
      //
      if (this->is_open) {
         record.open_next_subrecord('ONAM').close();
      }
      auto& DATA = record.open_next_subrecord('DATA');
      DATA.write(this->position.x);
      DATA.write(this->position.y);
      DATA.write(this->position.z);
      DATA.write(this->rotation.x);
      DATA.write(this->rotation.y);
      DATA.write(this->rotation.z);
      DATA.close();
   }
   void ObjectReference::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this);
      this->extra_data.sever_outbound_references_to(other, *this);
      //
      this->base_form.clear_if(*this, other);
   }
   void ObjectReference::_clear_impl() noexcept {
      this->extra_data.clear(*this);
      this->script_data.clear(*this);
      this->base_form.set(*this, nullptr);
      this->is_open = false;
      //this->position = { 0, 0, 0 }; // don't reset this as that might change the parent cell
      this->rotation = { 0, 0, 0 };
   }
   bool ObjectReference::_friendly_delete_impl(const file_load_order& load_order) noexcept {
      if (!this->is_working_copy)
         this->stub.edit_record_flags(form_flag::disabled, true);
      //
      // Make the reference an opposite enable state child of the PlayerRef.
      //
      auto* player_ref = load_order.get_form(hardcoded_form_ids::PlayerRef);
      assert(player_ref && "ObjectReference::_friendly_delete_impl: Why is the PlayerRef form not reachable by ID?");
      auto* extra = this->extra_data.get_or_create<components::extra_data_types::enable_state_parent>();
      extra->flags = components::extra_data_types::enable_state_parent::flag::opposite;
      extra->ref.set(*this, player_ref);

      //
      // TODO: xEdit uses -30000 as its preferred Z-coordinate, and it makes any actors that 
      // this procedure is applied to persistent. Why the latter?
      //

      //
      this->position.z = -9999.0F;
      //
      return true;
   }
}