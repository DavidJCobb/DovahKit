#pragma once
#include "../extra_data.h"
#include "../../../helpers/vector3.h"

namespace dovah::loaded_forms::components::extra {
   class portal : public basic_extra_data {
      //
      // NOTE: Skyrim Classic doesn't use this record type. It'll load the data in full, 
      // but then just discard it.
      //
      public:
         static constexpr uint32_t signature = 'XPTL';
         //
         float width;
         float height;
         cobb::vector3<float> position;
         struct { // quaternion?
            float a;
            float b;
            float c;
            float d;
         } rotation;
         //
         virtual extra_data_type get_type() const noexcept { return extra_data_type::portal; };
         virtual load_result load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc) override;
         virtual void save(tes_record_writer&) override;
         //
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&) {}
         //
         virtual basic_extra_data* clone(form_stub& clone_owner) const noexcept override;
   };
}