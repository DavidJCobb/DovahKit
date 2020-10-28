#pragma once
#include <vector>
#include "../_common.h"

namespace dovah::loaded_forms::components {
   struct container_entry {
      // CNTO:
      form_reference_t item;
      int32_t count = 0;
      // COED:
      form_reference_t owner;
      form_reference_t global; // for NPC_ owners
      int32_t factionRank = 0; // for FACT owners
      float   condition; // item health
      //
      container_entry() : factionRank(0) {};
   };
   struct container_data {
      std::vector<container_entry> entries;
      //
      void load(tes_subrecord_reader&, load_order_interfaces::form_load& intfc);
      static void generate_use_info(tes_subrecord_reader&, form_stub_use_info_builder&);
      void clone_from(const container_data& original, form_stub& my_owner) noexcept;
      void sever_outbound_references_to(form_stub& target, form_stub& my_owner) noexcept;
   };
}
