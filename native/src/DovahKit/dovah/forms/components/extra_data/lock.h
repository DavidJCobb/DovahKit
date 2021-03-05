#pragma once
#include "../extra_data.h"

namespace dovah::loaded_forms::components::extra {
   class lock : public basic_extra_data {
      public:
         static constexpr uint8_t scalar_use_count = 1;
      public:
         static constexpr uint32_t signature = 'XLOC';
         struct flag {
            flag() = delete;
            enum : uint8_t {
               leveled = 0x04,
            };
         };
         //
         uint8_t  level; // from 0 to 255; thresholds are: novice = 1; apprentice = 25; adept = 50; expert = 75; master = 100; requires key = 255
         uint8_t  pad01[3];
         struct_form_reference_t key;
         uint8_t  flags = 0;
         uint8_t  pad09[3];
         uint32_t unk0C;
         uint32_t unk10;
         //
         virtual extra_data_type get_type() const noexcept { return extra_data_type::lock; };
         virtual load_result load(tes_subrecord_reader& subrecord, load_interface_t& intfc) override;
         virtual void save(tes_record_writer&, save_interface_t&) override;
         //
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);
         //
         virtual basic_extra_data* clone(loaded_forms::Form& clone_owner) const noexcept override;
         virtual void sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) override;
   };
}