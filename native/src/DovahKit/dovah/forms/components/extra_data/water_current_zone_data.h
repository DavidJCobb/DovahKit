#pragma once
#include "../extra_data.h"
#include "../../../helpers/vector3.h"

namespace dovah::loaded_forms::components::extra {
   class water_current_zone_data : public basic_extra_data {
      public:
         static constexpr uint32_t signature_vel_linear     = 'XCVL';
         static constexpr uint32_t signature_vel_rotational = 'XCVR';
         //
         struct {
            cobb::vector3<float> linear;
            cobb::vector3<float> angular;
         } velocity;
         //
         virtual extra_data_type get_type() const noexcept { return extra_data_type::water_current_zone_data; };
         virtual load_result load(tes_subrecord_reader& subrecord, load_interface_t& intfc) override;
         virtual void save(tes_record_writer&, save_interface_t&) override;
         //
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&, extra_data_use_info_state&) {}
         //
         virtual basic_extra_data* clone(loaded_forms::Form& clone_owner) const noexcept override;
   };
}