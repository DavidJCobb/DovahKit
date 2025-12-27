#pragma once
#include <cstdint>
#include <vector>
#include "../../extra_data.h"
#include "../../../../../form_reference_t.h"

namespace dovah::loaded_forms::components::extra_data_types {
   class room_ref_data : public extra_data {
      public:
         static constexpr uint32_t signature = 'XRMR';
         
         struct flag {
            flag() = delete;
            enum : uint8_t {
               has_imagespace        = 0x04, // only used during load
               has_lighting_template = 0x08, // only used during load
            };
         };

         static constexpr int max_linked_room_count = std::numeric_limits<uint8_t>::max();

      public:
         room_ref_data() : extra_data(all_extra_data_types::index_of_type<room_ref_data>) {}

      public:
         uint8_t flags     = 0;
         bool    is_master = false;
         form_reference_t lighting_template;
         form_reference_t imagespace;
         std::vector<form_reference_t> linked_rooms;

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