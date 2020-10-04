#pragma once
#include "../extra_data.h"

namespace dovah::loaded_forms::components::extra {
   class linked_ref : public basic_extra_data {
      public:
         static constexpr uint32_t signature = 'XLKR';
         //
         form_id_t keyword; // optional; struct can be 8 bytes (KYWD, ref) or 4 bytes (ref)
         form_id_t ref;
         //
         virtual extra_data_type get_type() const noexcept { return extra_data_type::linked_ref; };
         virtual extra_data_load_result load(tes_subrecord_reader&) override;
         virtual void save(tes_record_writer&) override;
         //
         static void generate_use_info(tes_record_reader&, form_stub*);
         //
         virtual basic_extra_data* clone(form_stub& clone_owner) const noexcept override;
         virtual void sever_outbound_references_to(form_stub& target, form_stub& my_owner) override;
   };
}