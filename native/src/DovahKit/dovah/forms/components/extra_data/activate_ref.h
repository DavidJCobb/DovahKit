#pragma once
#include "../extra_data.h"

namespace dovah::loaded_forms::components::extra {
   class activate_ref : public basic_extra_data {
      //
      // TODO: IDENTIFY WHICH OF THESE MEMBERS ARE FORM IDs.
      //
      public:
         static constexpr uint32_t signature = 'XACR';
         //
         uint32_t unk00; // form ID?
         uint32_t unk04;
         uint8_t  unk08;
         uint8_t  pad09[3];
         //
         virtual extra_data_type get_type() const noexcept { return extra_data_type::activate_ref; };
         virtual load_result load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc) override;
         virtual void save(tes_record_writer&) override;
         //
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&) {}
         //
         virtual basic_extra_data* clone(form_stub& clone_owner) const noexcept override;
   };
}