#pragma once
#include "../extra_data.h"

namespace dovah::loaded_forms::components::extra {
   class distant_data : public basic_extra_data {
      public:
         static constexpr uint32_t signature = 'XLOD';
         //
         uint32_t unk00; // these are loaded as a struct; we can tell they're separate fields due to BSWAP instructions
         uint32_t unk04;
         uint32_t unk08;
         //
         virtual extra_data_type get_type() const noexcept { return extra_data_type::distant_data; };
         virtual load_result load(tes_subrecord_reader& subrecord, load_interface_t& intfc) override;
         virtual void save(tes_record_writer&, save_interface_t&) override;
         //
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&) {}
         //
         virtual basic_extra_data* clone(loaded_forms::Form& clone_owner) const noexcept override;
   };
}