#pragma once
#include <cstdint>
#include <vector>
#include "../../extra_data.h"
#include "../../../../../form_reference_t.h"

namespace dovah::loaded_forms::components::extra_data_types {
   class cell_region_list : public extra_data {
      public:
         static constexpr uint32_t signature = 'XCLR';

      public:
         cell_region_list() : extra_data(all_extra_data_types::index_of_type<cell_region_list>) {}

      public:
         std::vector<form_reference_t> regions; // XCLR -> REGN[]

      public:
         virtual subrecord_load_result load(tes_file_reading::subrecord&, load_interface_t&) override;
         virtual record_load_result load(tes_file_reading::record&, load_interface_t&) override;
         virtual void save(tes_file_writing::record&, save_interface_t&) override;
         
         static void generate_use_info(tes_file_reading::record&, form_stub_use_info_builder&, extra_data_use_info_state&);
         virtual void clear_contained_formIDs(loaded_forms::Form& my_owner) override;
         virtual void sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) override;
         
         virtual extra_data* clone(loaded_forms::Form& clone_owner) const noexcept override;
   };
}