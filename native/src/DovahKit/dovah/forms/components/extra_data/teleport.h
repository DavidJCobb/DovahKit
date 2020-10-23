#pragma once
#include "../extra_data.h"
#include "../../../helpers/vector3.h"

namespace dovah::loaded_forms::components::extra {
   class teleport : public basic_extra_data {
      public:
         static constexpr uint32_t signature = 'XTEL';
         struct flag {
            flag() = delete;
            enum : uint32_t {
               no_alarm = 0x01,
            };
         };
         //
         form_reference_t     target_door;
         cobb::vector3<float> position;
         cobb::vector3<float> rotation;  // radians
         uint32_t             flags = 0; // the game only keeps the low byte
         //
         virtual extra_data_type get_type() const noexcept { return extra_data_type::teleport; };
         virtual load_result load(tes_subrecord_reader&) override;
         virtual void save(tes_record_writer&) override;
         //
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);
         //
         virtual basic_extra_data* clone(form_stub& clone_owner) const noexcept override;
         virtual void sever_outbound_references_to(form_stub& target, form_stub& my_owner) override;
   };
}