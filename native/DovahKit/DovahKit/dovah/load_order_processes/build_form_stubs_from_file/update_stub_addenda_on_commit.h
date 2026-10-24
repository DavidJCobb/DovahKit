#pragma once
#include "dovah/form_stub.h"
#include "dovah/form_stub_addenda.h"
#include "dovah/form_type.h"

namespace dovah::load_order_processes::build_form_stubs_from_file {
   inline void update_stub_addenda_on_commit(
      form_stub& stub,
      form_stub* stub_parent
   ) {
      if (stub.form_type == form_type::cell) {
         //
         // Manage the persistent cell for this cell's parent world:
         //
         if (stub.test_record_flags((uint32_t)1 << (size_t)tes::record_flag::persistent)) {
            //
            // Ensure that worldspaces are aware of their persistent cells.
            //
            if (stub_parent && stub_parent->form_type == form_type::worldspace) {
               auto& addenda = stub_parent->get_or_create_addenda();
               if (!addenda.persistent_cell) // Only the first persistent-flagged cell loaded by a worldspace will be THE persistent cell.
                  addenda.persistent_cell = &stub;
            }
         }
      } else if (stub.form_type == form_type::land) {
         //
         // Manage the canonical landscape for this landscape's parent cell:
         //
         if (stub_parent && stub_parent->form_type == form_type::cell) {
            stub_parent->get_or_create_addenda().canonical_landscape = &stub;
         }
      }
   }
}