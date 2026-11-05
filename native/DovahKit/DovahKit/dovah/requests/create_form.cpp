#include "./create_form.h"
#include <cassert>
#include "../active_load_order.h"
#include "../form_data/can_load_form_data_of_type.h"
#include "../form_data_function_tables/create_blank_instance.h"
#include "../form_stub.h"
#include "../form_stub_addenda.h"
#include "../form_stub_collection.h"
#include "../tes/file.h"
#include "../tes/record_flags.h"
#include "../utils/form_stubs/can_load_form_data.h"
#include "../utils/form_stubs/is_exterior_cell.h"
#include "../utils/form_stubs/is_interior_cell.h"
#include "../utils/form_types/is_cell_child.h"
#include "../utils/form_types/is_ref.h"

namespace dovah::requests {
   create_form::create_form(::dovah::active_load_order& alo) :
      active_load_order(alo),
      form_id(alo.get_form_stub_collection()._reserve_form_id({}))
   {
   }
   create_form::~create_form() {
      if (!this->committed)
         this->active_load_order.get_form_stub_collection()._abandon_form_id_reservation({}, this->form_id);
   }

   std::optional<create_form::error_code> create_form::would_fail() const {
      const auto& alo = this->active_load_order;
      const auto& fsc = alo.get_form_stub_collection();
      if (!is_valid_form_type(this->form_type))
         return error_code::invalid_form_type;
      if (!alo.get_active_file())
         return error_code::no_active_file;
      if (!this->form_id)
         return error_code::no_form_id_available;
      if (!can_load_form_data_of_type(this->form_type))
         return error_code::unimplemented_form_type;

      {
         const auto& info = form_type_info::lookup(this->form_type);
         if (info.flags & form_type_info::flag::is_skyrim_special) {
            if (alo.get_current_game() != game::skyrim_special)
               return error_code::form_type_unavailable_in_current_game;
         }
      }

      // Can we destroy any occupying none-stub?
      if (form_stub* occupier = fsc.get_form_by_id(this->form_id, false)) {
         assert(occupier->is_none_stub() && "We should have reserved this form ID when the request was initialized. How did it end up taken?");
         if (!fsc._can_destroy_none_stub(*occupier))
            return error_code::cannot_sever_references_to_none_stub;
      }

      if (this->child_of) {
         auto parent_type = this->child_of->form_type;
         auto child_type  = this->form_type;

         bool error = false;
         if (utils::form_types::is_cell_child(child_type)) { // validate parent/child relationships
            error = parent_type != form_type::cell;
         } else if (child_type == form_type::cell) {
            error = parent_type != form_type::worldspace;
         } else if (child_type == form_type::topic_info) {
            error = parent_type != form_type::topic;
         } else {
            error = true;
         }
         if (error)
            return error_code::invalid_parent_child_relationship;

         if (utils::form_types::is_cell_child(child_type)) {
            if (form_stub_helpers::is_worldspace_persistent_cell(*this->child_of)) {
               return error_code::do_not_use_worldspace_persistent_cell_as_parent;
            }
         }
         
         if (child_type == form_type::cell) { // validate worldspace grid coordinates
            auto& grid_opt = this->grid_position;
            if (!grid_opt.has_value())
               return error_code::exterior_cell_must_have_grid_coordinates;
            auto& grid_coords = grid_opt.value();

            auto* existing = form_stub_helpers::get_worldspace_cell_by_grid(*this->child_of, grid_coords.x, grid_coords.y);
            if (existing)
               return error_code::exterior_grid_coordinates_already_taken;
         }
      } else {
         if (utils::form_types::is_cell_child(this->form_type))
            return error_code::cannot_create_reference_with_no_parent_cell;
      }
      if (this->clone_of && this->form_type == form_type::cell) {
         if (this->child_of) {
            if (utils::form_stubs::is_interior_cell(*this->clone_of))
               return error_code::interior_cell_clone_cannot_have_parent;
         } else {
            if (utils::form_stubs::is_exterior_cell(*this->clone_of))
               return error_code::exterior_cell_clone_must_have_parent;
         }
      }

      return {};
   }

   form_stub& create_form::commit() {
      assert(!this->committed);
      using exception  = exceptions::form_creation_failed;
      
      auto& alo         = this->active_load_order;
      auto& fsc         = alo.get_form_stub_collection();
      auto* active_file = alo.get_active_file();

      auto error = this->would_fail();
      if (error.has_value()) {
         throw exception(error.value(), *this);
      }

      if (form_stub* occupier = fsc.get_form_by_id(this->form_id, false)) {
         assert(occupier->is_none_stub() && "We should have reserved this form ID when the request was initialized. How did it end up taken?");
         assert(this->_can_destroy_none_stub(*occupier));
         fsc._destroy_none_stub({}, *occupier);
      }

      base_managed_form_data* loaded = nullptr;
      uint32_t record_flags = 0;
      
      auto* stub = new form_stub;
      stub->form_type = this->form_type;
      if (!this->clone_of) {
         auto factory = form_data_function_tables::create_blank_instance::lookup(this->form_type);
         assert(!!factory);
         loaded = &((factory)(*stub));
         assert(loaded != nullptr && "Previous checks already established that we can construct this form type. Why did we receive nullptr when trying?");
      }
      stub->loaded    = loaded;
      stub->_add_file(*active_file, 0);
      stub->form_id   = this->form_id;
      stub->editor_id = this->editor_id;
      stub->set_edited(true);
      //
      if (this->grid_position.has_value()) {
         const auto& src = this->grid_position.value();
         stub->get_or_create_addenda().grid_position = this->grid_position.value();
      }
      //
      {
         auto guard = std::lock_guard(fsc.forms_lock);
         fsc.forms[this->form_id] = stub;
         fsc.forms_by_type[stub->form_type][this->form_id] = stub;
         fsc.active_file_forms[this->form_id] = stub;
         fsc.active_file_forms_by_type[stub->form_type][this->form_id] = stub;
      }
      //
      if (this->child_of) {
         stub->set_parent_form(this->child_of);
         if (
            this->child_of->form_type == dovah::form_type::cell
         && utils::form_types::is_ref(stub->form_type)
         && utils::form_stubs::is_exterior_cell(*this->child_of)
         ) {
            //
            // If placing the ref in an exterior cell, set some reasonable default coordinates, 
            // so that the ref doesn't have coordinates that place it oustide of its intended 
            // parent cell.
            //
            assert(stub->loaded != nullptr && "Previous checks already established that we can construct this form type. Why did we receive nullptr when trying?");
            int32_t gx = 0;
            int32_t gy = 0;
            if (this->child_of->get_grid_coordinates(gx, gy)) {
               //
               // cell grid coordinates -> world units of point centered within the cell
               //
               float wx = gx * core_constants::exterior_cell_side_length;
               float wy = gy * core_constants::exterior_cell_side_length;
               wx += (core_constants::exterior_cell_side_length / 2.0F);
               wy += (core_constants::exterior_cell_side_length / 2.0F);
               
               auto* casted = (loaded_forms::ObjectReference*)stub->form;
               casted->position.x = wx;
               casted->position.y = wy;
               casted->position.z = 0;
            }
         }
      }
      //
      if (this->clone_of) {
         auto flags = this->clone_of->get_record_flags();
         flags &= ~((uint32_t)1 << (size_t)tes::record_flag::deleted);
         flags &= ~((uint32_t)1 << (size_t)tes::record_flag::partial);
         flags &= ~((uint32_t)1 << (size_t)tes::record_flag::compressed);
         stub->edit_record_flags(flags, true); // clone record flags
         //
         auto original = this->clone_of->load();
         if (original) {
            loaded = original->clone(*stub);
            assert(loaded != nullptr && "Previous checks already established that we can construct this form type. Why did we receive nullptr when trying?");
         }
      } else {
         loaded->setup(*this);
      }
      //
      if (active_file) {
         auto next_id = fsc.find_first_free_form_id_in_active_file(this->form_id + 1);
         if (next_id) {
            active_file->header.nextFormID = utils::form_ids::normalize_next_record_id(next_id, alo.file_prefix_for(*active_file));
         } else {
            active_file->header.nextFormID = 0; // not sure what the CK would do in this case
         }
      }
      //
      fsc._abandon_form_id_reservation({}, this->form_id);
      this->committed = true;
      if (alo.post_load_callbacks.on_form_created)
         (alo.post_load_callbacks.on_form_created)(*stub);
      return *stub;
   }
}