#pragma once
#include "../_common.h"
#include <unordered_map>
#include "./cell_grid_dword.h"

namespace dovah {
   namespace loaded_forms {
      class Form;
   }
   class form_stub;
}

namespace dovah::loaded_forms::structs {
   //
   // WRLD/RNAM[]
   //
   class large_ref_index {
      public:
         struct form_specific_use_info_data {
            std::unordered_map<cell_grid_dword, std::vector<form_id_t>> cells_to_refs;
         };

         struct ref_info {
            form_reference_t form;
            cell_grid_dword  parent_cell_id;
         };

      public:
         std::unordered_map<cell_grid_dword, std::vector<ref_info>> cells_to_refs;

      public:
         void load(tes_subrecord_reader&, load_order_interfaces::form_load&); // load a single cell's entry
         void save(tes_record_writer&, load_order_interfaces::form_save&) const;
         static void generate_use_info(tes_subrecord_reader&, form_stub_use_info_builder&);
         void clone_from(const large_ref_index& original, loaded_forms::Form& my_containing_form) noexcept;
         void sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_containing_form) noexcept;
         void clear(loaded_forms::Form& my_containing_form);

         bool empty() const noexcept;
   };
}