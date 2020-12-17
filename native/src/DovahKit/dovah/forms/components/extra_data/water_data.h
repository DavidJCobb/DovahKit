#pragma once
#include "../extra_data.h"

namespace dovah::loaded_forms::components::extra {
   class water_data : public basic_extra_data {
      public:
         static constexpr uint32_t signature_base = 'XWCN';
         static constexpr uint32_t signature_vel  = 'XWCU';
         //
         struct _vector4 {
            cobb::vector3<float> velocity;
            float unk0C;
         };
         //
         uint32_t count; // XWCN
         std::vector<_vector4> data; // XWCU
         //
         virtual extra_data_type get_type() const noexcept { return extra_data_type::water_data; };
         virtual load_result load(tes_subrecord_reader&, load_interface_t&) override;
         virtual bool        load(tes_record_reader&,    load_interface_t&) override;
         virtual void        save(tes_record_writer&, save_interface_t&) override;
         //
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&) {}
         //
         virtual basic_extra_data* clone(form_stub& clone_owner) const noexcept override;
   };
}