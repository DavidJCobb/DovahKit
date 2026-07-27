#include "./needs_to_be_saved.h"
#include "../../files/file_load_order.h"
#include "../../form_stub.h"
#include "../../form_stub_addenda.h"
#include "./for_each_persistent_ref_in_world.h"

namespace dovah::form_stub_helpers {
   static bool needs_to_be_saved_impl(const file_load_order& flo, const form_stub& stub);
   
   //

   static constexpr bool can_have_children(const form_stub& stub) {
      return form_type_info::lookup(stub.form_type).flags & form_type_info::flag::can_have_children;
   }

   static constexpr bool is_persistent_cell_of(const form_stub& cell, const form_stub& world) {
      return world.addenda && world.addenda->persistent_cell == &cell;
   }

   static constexpr bool is_persistent_ref(const form_stub& stub) {
      return form_type_is_reference(stub.form_type) && stub.test_record_flags(0x400);
   }

   //

   static bool does_any_descendant_need_save(const file_load_order& flo, const form_stub& parent) {
      if (!can_have_children(parent))
         return false;

      for (auto& pair : parent.inbound) {
         auto& entry = pair.second;
         if (!(entry.flags & use_info::entry_flag_to_mask(use_info::entry_flags::base::parent)))
            continue;
         const auto* stub = entry.other;
         if (needs_to_be_saved_impl(flo, *stub))
            return true;
      }

      return false;
   }

   static bool does_non_persistent_cell_descendant_need_save(const file_load_order& flo, const form_stub& parent) {
      for (auto& pair : parent.inbound) {
         auto& entry = pair.second;
         if (!(entry.flags & use_info::entry_flag_to_mask(use_info::entry_flags::base::parent)))
            continue;
         const auto* stub = entry.other;
         if (is_persistent_ref(*stub))
            continue;
         if (needs_to_be_saved_impl(flo, *stub))
            return true;
      }

      return false;
   }

   static bool worldspace_has_persistent_ref_needing_save(const dovah::form_stub& world) {
      bool needs_save = false;
      form_stub_helpers::for_each_persistent_ref_in_world(world, [&needs_save](form_stub& child) {
         if (needs_to_be_saved(child)) {
            needs_save = true;
            return true; // break
         }
         return false; // continue
      });
      return needs_save;
   }

   //

   static bool needs_to_be_saved_impl(const file_load_order& flo, const form_stub& stub) {
      if (stub.is_edited() || flo.is_defined_or_overridden_in_active_file(stub))
         return true;

      // Check if child/descendant records will need saving. Requires special-case 
      // handling for persistent refs.
      if (stub.form_type == dovah::form_type::cell) {
         auto* world_stub = stub.get_parent_form();
         if (world_stub && is_persistent_cell_of(stub, *world_stub)) {
            //
            // The worldspace's persistent cell needs to be saved if any persistent 
            // refs in the worldspace need to be saved. (We re-parent persistent refs 
            // on load, placing them in whatever cell their coordinates would place 
            // them in. Thus the cell won't test as having any descendant forms that 
            // need to be saved; we need this special-case handling here.)
            //
            if (worldspace_has_persistent_ref_needing_save(*world_stub))
               return true;
         } else {
            //
            // The aforementioned re-parenting of persistent refs means that when we 
            // check whether a non-persistent cell needs saving, we need to ignore 
            // any persistent refs that were re-parented under it.
            //
            if (does_non_persistent_cell_descendant_need_save(flo, stub))
               return true;
         }
      } else {
         if (does_any_descendant_need_save(flo, stub))
            return true;
      }

      // Special-case: reordering INFOs within a DIAL, without otherwise editing any 
      // data within the INFOs or the DIAL.
      if (stub.form_type == form_type::topic) {
         if (auto* addenda = stub.addenda) {
            auto& list_d = addenda->ordered_children.get_master_list();
            auto& list_a = addenda->ordered_children.get_active_list();
            if (list_d != list_a) {
               //
               // No child forms were added nor reparented into these lists (else they 
               // would've tripped the `does_descendant_form_need_save` check for the 
               // "edited" flag), but child forms may have been reordered without ever 
               // actually being modified.
               //
               return true;
            }
         }
      }

      return false;
   }

   extern bool needs_to_be_saved(const form_stub& stub) {
      return needs_to_be_saved_impl(stub.get_owning_load_order(), stub);
   }
}