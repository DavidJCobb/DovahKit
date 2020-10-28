#pragma once
#include "../extra_data.h"

namespace dovah::loaded_forms::components::extra {
   class ragdoll_data : public basic_extra_data {
      public:
         static constexpr uint32_t signature_base  = 'XRGD';
         static constexpr uint32_t signature_biped = 'XRGB';
         //
         bool has_rgd = false;
         bool has_rgb = false;
         //
         std::vector<uint8_t>     data_rgd;
         std::array<uint8_t, 0xC> data_rgb;
         //
         virtual extra_data_type get_type() const noexcept { return extra_data_type::ragdoll_data; };
         virtual load_result load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc) override;
         virtual void save(tes_record_writer&) override;
         //
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&) {}
         //
         virtual basic_extra_data* clone(form_stub& clone_owner) const noexcept override;
   };
}