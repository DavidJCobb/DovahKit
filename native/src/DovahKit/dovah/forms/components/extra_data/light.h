#pragma once
#include "../extra_data.h"

namespace dovah::loaded_forms::components::extra {
   class light : public basic_extra_data {
      public:
         static constexpr uint32_t signature = 'XLIG';
         //
         float    fov;
         float    fade;
         uint32_t unk08;
         float    shadow_depth_bias = 1.0F;
         uint32_t unk10;
         //
         virtual extra_data_type get_type() const noexcept { return extra_data_type::lock; };
         virtual load_result load(tes_subrecord_reader&) override;
         virtual void save(tes_record_writer&) override;
         //
         static void generate_use_info(tes_record_reader&, form_stub*) {}
   };
}