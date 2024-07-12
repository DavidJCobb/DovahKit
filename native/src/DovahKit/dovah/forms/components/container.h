#pragma once
#include <vector>
#include "../_common.h"
#include "../structs/container_object_extra_data.h"

namespace dovah::loaded_forms::components {
   struct container_entry {
      form_reference_t item;      // CNTO+0
      int32_t          count = 0; // CNTO+4
      structs::container_object_extra_data extra_data; // COED
   };
   struct container_data {
      std::vector<container_entry> entries;
      //
      void load(tes_subrecord_reader&, load_order_interfaces::form_load& intfc);
      bool save(tes_record_writer&, load_order_interfaces::form_save&);
      static void generate_use_info(tes_subrecord_reader&, form_stub_use_info_builder&);
      void clone_from(const container_data& original, loaded_forms::Form& my_owner) noexcept;
      void sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) noexcept;
      void clear(loaded_forms::Form& my_owner);
   };
}
