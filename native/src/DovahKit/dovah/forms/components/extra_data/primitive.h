#pragma once
#include "../extra_data.h"
#include "../../../helpers/vector3.h"

namespace dovah::loaded_forms::components::extra {
   class primitive : public basic_extra_data {
      public:
         static constexpr uint32_t signature = 'XPRM';
         enum class shape : uint32_t {
            none,
            box,
            sphere,
            portal_box,
            unknown,
         };
         //
         cobb::vector3<float> bounds;
         struct {
            float r;
            float g;
            float b;
         } color;
         float unknown;
         shape type = shape::box;
         //
         virtual extra_data_type get_type() const noexcept { return extra_data_type::lock; };
         virtual load_result load(tes_subrecord_reader& subrecord, load_interface_t& intfc) override;
         virtual void save(tes_record_writer&, save_interface_t&) override;
         //
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&) {}
         //
         virtual basic_extra_data* clone(form_stub& clone_owner) const noexcept override;
   };
}