#include "./create_form.h"

#define OBJECT_REFERENCE_FORM_DATA_IMPLEMENTED 0 // TODO

#include <cassert>
#include "../_passkeys/add_form_stub_source_file.h"
#include "../active_load_order.h"
#include "../exceptions/form_creation_failed.h"
#include "../facts/cells/exterior_cell_side_length.h"
#include "../form_data/can_load_form_data_of_type.h"
#if OBJECT_REFERENCE_FORM_DATA_IMPLEMENTED
   #include "../form_data/types/r/object_reference.h"
#endif
#include "../form_data_function_tables/assign.h"
#include "../form_data_function_tables/create_blank_instance.h"
#include "../form_data_function_tables/initialize.h"
#include "../form_stub.h"
#include "../form_stub_addenda.h"
#include "../form_stub_collection.h"
#include "../tes/file.h"
#include "../tes/record_flags.h"
#include "../types/file_prefix.h"
#include "../utils/form_ids/normalize_next_record_id.h"
#include "../utils/form_stubs/can_load_form_data.h"
#include "../utils/form_stubs/get_worldspace_cell_by_grid.h"
#include "../utils/form_stubs/is_exterior_cell.h"
#include "../utils/form_stubs/is_interior_cell.h"
#include "../utils/form_stubs/is_worldspace_persistent_cell.h"
#include "../utils/form_types/actually_is_a_form.h"
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
      if (!utils::form_types::actually_is_a_form(this->form_type))
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

      #if _DEBUG
         //
         // Verify that the target form ID is either unoccupied, or occupied only by a none-stub 
         // that we can delete.
         //
         if (const form_stub* occupier = fsc.get_form_by_id(this->form_id, false)) {
            assert(occupier->is_none_stub() && "We should have reserved this form ID when the request was initialized. How did it end up taken?");
            for (auto& [_, entry] : occupier->inbound) {
               assert(!!entry.other && "Use info entry with no form-stub pointer?");
               //
               // Verify that we can actually delete the none-stub. This requires being able to 
               // load all of the user-forms, so they can be told to sever the use.
               // 
               // None-stubs should only be created when we build the user-forms' use info, and 
               // in general, when we implement support for a form type, we should be starting 
               // with *both* the "build use info" and "load" functions. The only way we can have 
               // a none-stub that we can't delete is if only the former function was implemented, 
               // and I don't feel that should be considered a normal scenario.
               //
               assert(utils::form_stubs::can_load_form_data(*entry.other) && "How did we end up with a none-stub that we can't delete?");
            }
         }
      #endif

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
            if (utils::form_stubs::is_worldspace_persistent_cell(*this->child_of)) {
               return error_code::do_not_use_worldspace_persistent_cell_as_parent;
            }
         }
         
         if (child_type == form_type::cell) { // validate worldspace grid coordinates
            auto& grid_opt = this->grid_position;
            if (!grid_opt.has_value())
               return error_code::exterior_cell_must_have_grid_coordinates;
            auto& grid_coords = grid_opt.value();

            auto* existing = utils::form_stubs::get_worldspace_cell_by_grid(*this->child_of, grid_coords.x, grid_coords.y);
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
         fsc._destroy_none_stub({}, *occupier);
      }

      base_managed_form_data* loaded = nullptr;
      uint32_t record_flags = 0;
      
      auto* stub = new form_stub;
      stub->form_type = this->form_type;
      {
         auto factory = form_data_function_tables::create_blank_instance::lookup(this->form_type);
         assert(!!factory);
         loaded = &((factory)(*stub));
         assert(loaded != nullptr && "Previous checks already established that we can construct this form type. Why did we receive nullptr when trying?");
         stub->_on_newly_created_with_data({}, *loaded);
      }
      stub->_add_source_file({}, *active_file, 0);
      stub->form_id   = this->form_id;
      stub->editor_id = this->editor_id;
      stub->set_edited(true);
      //
      if (this->grid_position.has_value()) {
         const auto& src = this->grid_position.value();
         stub->get_or_create_addenda().grid_position = this->grid_position.value();
      }
      //
      fsc._store_newly_created_form({}, *stub);
      //
      if (this->child_of) {
         stub->set_parent_form(this->child_of);
         #if OBJECT_REFERENCE_FORM_DATA_IMPLEMENTED
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
            assert(loaded != nullptr && "Previous checks already established that we can construct this form type. Why did we receive nullptr when trying?");
            if (this->child_of->addenda && this->child_of->addenda->grid_position.has_value()) {
               auto [gx, gy] = this->child_of->addenda->grid_position.value();
               //
               // cell grid coordinates -> world units of point centered within the cell
               //
               float wx = gx * facts::cells::exterior_cell_side_length;
               float wy = gy * facts::cells::exterior_cell_side_length;
               wx += (facts::cells::exterior_cell_side_length / 2.0F);
               wy += (facts::cells::exterior_cell_side_length / 2.0F);
               
               auto* casted = (form_data::object_reference<managed_form_params>*)loaded;
               casted->position.x = wx;
               casted->position.y = wy;
               casted->position.z = 0;
            }
         }
         #endif
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
            auto assign_handler = form_data_function_tables::assign::managed_to_managed::lookup(stub->form_type);
            assert(!!assign_handler);
            (assign_handler)(*loaded, *original);
         }
      } else {
         auto init_handler = form_data_function_tables::initialize::lookup(stub->form_type);
         if (init_handler)
            (init_handler)(*loaded);
      }
      //
      if (active_file) {
         auto next_id = fsc.find_first_free_form_id_in_active_file(this->form_id + 1);
         if (next_id) {
            active_file->header().nextFormID = utils::form_ids::normalize_next_record_id(next_id, alo.file_prefix_for(*active_file));
         } else {
            active_file->header().nextFormID = 0; // not sure what the CK would do in this case
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