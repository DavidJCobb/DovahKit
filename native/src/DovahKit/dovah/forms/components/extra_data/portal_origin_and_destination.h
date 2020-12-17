#pragma once
#include "../extra_data.h"

namespace dovah::loaded_forms::components::extra {
   class portal_origin_and_destination : public basic_extra_data {
      public:
         static constexpr uint32_t signature = 'XPOD';
         //
         form_reference_t origin;      // REFR
         form_reference_t destination; // REFR
         //
         virtual extra_data_type get_type() const noexcept { return extra_data_type::portal_origin_and_destination; };
         virtual extra_data_load_result load(tes_subrecord_reader&, load_interface_t&) override;
         virtual void save(tes_record_writer&, save_interface_t&) override;
         //
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);
         //
         virtual basic_extra_data* clone(form_stub& clone_owner) const noexcept override;
         virtual void sever_outbound_references_to(form_stub& target, form_stub& my_owner) override;
   };
}