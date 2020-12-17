#pragma once
#include "../extra_data.h"

namespace dovah::loaded_forms::components::extra {
   class package_start_location : public basic_extra_data {
      public:
         static constexpr uint32_t signature = 'XPSL';
         //
         uint32_t unk00;
         uint32_t unk04;
         uint32_t unk08;
         uint32_t unk0C;
         uint32_t unk10; // this may be a float, and is the only field that can't be BSWAPped
         //
         virtual extra_data_type get_type() const noexcept { return extra_data_type::package_start_location; };
         virtual load_result load(tes_subrecord_reader& subrecord, load_interface_t& intfc) override;
         virtual void save(tes_record_writer&, save_interface_t&) override;
         //
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&) {}
         //
         virtual basic_extra_data* clone(form_stub& clone_owner) const noexcept override;
   };
}