#pragma once
#include <cstdint>
#include "../../extra_data.h"
#include "../../../../../form_reference_t.h"

namespace dovah::loaded_forms::components::extra_data_types {
   class lock : public extra_data {
      public:
         static constexpr uint32_t signature = 'XLOC';
         
         struct flag {
            flag() = delete;
            enum : uint8_t {
               leveled = 0x04,
            };
         };

      public:
         lock() : extra_data(all_extra_data_types::index_of_type<lock>) {}

      public:
         uint8_t  level = 0; // from 0 to 255; thresholds are: novice = 1; apprentice = 25; adept = 50; expert = 75; master = 100; requires key = 255
         form_reference_t key;
         uint8_t  flags = 0;
         uint32_t unk0C = 0;
         uint32_t unk10 = 0;

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