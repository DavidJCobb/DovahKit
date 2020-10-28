#pragma once
#include "../extra_data.h"

namespace dovah::loaded_forms::components::extra {
   class collision_data : public basic_extra_data {
      public:
         static constexpr uint32_t signature = 'XTRI';
         //
         uint32_t layer;
         //
         virtual extra_data_type get_type() const noexcept { return extra_data_type::collision_data; };
         virtual load_result load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc) override;
         virtual void save(tes_record_writer&) override;
         //
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&) {}
         //
         virtual basic_extra_data* clone(form_stub& clone_owner) const noexcept override;
   };
}