#pragma once
#include <vector>
#include "../_common.h"

namespace dovah::loaded_forms::components {
   struct container_entry {
      // CNTO:
      form_reference_t item;
      int32_t count = 0;
      // COED:
      struct {
         form_reference_t owner;
         form_reference_t global; // for NPC_ owners
         int32_t          faction_rank = 0; // for FACT owners
      } ownership;
      struct {
         float value;
         bool  present = false;
      } condition; // item health
   };
   struct container_data {
      std::vector<container_entry> entries;
      //
      void load(tes_subrecord_reader&, load_order_interfaces::form_load& intfc);
      bool save(tes_record_writer&, load_order_interfaces::form_save&);
      static void generate_use_info(tes_subrecord_reader&, form_stub_use_info_builder&);
      void clone_from(const container_data& original, form_stub& my_owner) noexcept;
      void sever_outbound_references_to(form_stub& target, form_stub& my_owner) noexcept;
      void clear(form_stub& my_owner);
   };
}
