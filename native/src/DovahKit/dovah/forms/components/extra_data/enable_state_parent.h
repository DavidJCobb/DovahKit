#pragma once
#include "../extra_data.h"

namespace dovah::loaded_forms::components::extra {
   class enable_state_parent : public basic_extra_data {
      public:
         static constexpr uint32_t signature = 'XESP';
         struct flag {
            flag() = delete;
            enum : uint8_t {
               opposite = 0x01,
               pop_in   = 0x02,
            };
         };
         //
         form_reference_t ref;
         uint8_t flags = 0;
         uint8_t pad05[3];
         //
         virtual extra_data_type get_type() const noexcept { return extra_data_type::enable_state_parent; };
         virtual load_result load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc) override;
         virtual void save(tes_record_writer&) override;
         //
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);
         //
         virtual basic_extra_data* clone(form_stub& clone_owner) const noexcept override;
         virtual void sever_outbound_references_to(form_stub& target, form_stub& my_owner) override;
   };
}